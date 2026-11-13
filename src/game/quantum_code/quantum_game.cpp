// quantum_game.cpp - Quantum.QuantumGame.
#include "quantum_code/quantum_game.h"

#include "quantum_code/systems/core_systems.h"
#include "quantum_code/systems/system_setup.h"
#include "quantum_core/asset_db.h"
#include "quantum_core/dynamic_asset_db.h"
#include "quantum_core/events.h"
#include "quantum_core/resource_manager.h"

#include "photon_deterministic/deterministic_session.h"
#include "photon_deterministic/log.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace Quantum {

using Photon::Deterministic::ByteArray;
using Photon::Deterministic::DeterministicFrame;
using Photon::Deterministic::DeterministicFrameInputTemp;
using Photon::Deterministic::DeterministicFrameSerializeMode;
using Photon::Deterministic::IDisposable;
using Photon::Deterministic::Log;

namespace {

// The frame context as the session sees it: something it disposes at the end. (The
// original's context class, FrameContextUser, is itself disposable.)
class DisposableFrameContext final : public IDisposable {
  public:
    explicit DisposableFrameContext(std::shared_ptr<Core::FrameContext> context) : Context(std::move(context)) {}
    void Dispose() override {
        if (Context) {
            Context->Dispose();
        }
    }
    std::shared_ptr<Core::FrameContext> Context;
};

// Input buffers hold one serialised Input; 1 KiB as in the original.
constexpr std::size_t InputBufferSize = 1024;

template <class T> bool HasSystem(const std::vector<SystemBase*>& systems) {
    return std::any_of(systems.begin(), systems.end(),
                       [](SystemBase* system) { return dynamic_cast<T*>(system) != nullptr; });
}

} // namespace

QuantumGame::QuantumGame(const QuantumGameStartParameters& startParameters)
    : _heapExtraCount(startParameters.HeapExtraCount), _resourceManager(startParameters.ResourceManager),
      _assetSerializer(startParameters.AssetSerializer), _flags(startParameters.GameFlags) {
    // The original copies the initial dynamic assets into a database of its own.
    if (startParameters.InitialDynamicAssets) {
        _initialDynamicAssets = std::make_shared<DynamicAssetDB>();
        _initialDynamicAssets->CopyFrom(*startParameters.InitialDynamicAssets);
    }
    InitCallbacks();
}

QuantumGame::~QuantumGame() {
    // The frames hold the context's memory; release them before the context goes.
    _frames = {};
}

const std::vector<int>& QuantumGame::GetLocalPlayers() const {
    static const std::vector<int> none;
    return _session != nullptr ? _session->LocalPlayerIndices() : none;
}

bool QuantumGame::PlayerIsLocal(int player) const { return _session != nullptr && _session->IsLocalPlayer(player); }

int QuantumGame::GetInputSerializedFixedSize() {
    std::vector<std::uint8_t> buffer(InputBufferSize);
    FrameSerializer serializer(DeterministicFrameSerializeMode::Serialize, nullptr, std::span<std::uint8_t>(buffer));
    serializer.SetWriting(true);
    serializer.SetInputMode(true);
    ::Quantum::Input::Write(&serializer, ::Quantum::Input{});
    return static_cast<int>(serializer.ToArray().size());
}

void QuantumGame::DeserializeInputInto(int player, const ByteArray& data, std::uint8_t* buffer) {
    if (!_inputSerializerRead) {
        _inputStreamReadZeroArray.assign(InputBufferSize, 0);
        _inputReadBuffer.assign(InputBufferSize, 0);
        _inputSerializerRead = std::make_unique<FrameSerializer>(DeterministicFrameSerializeMode::Serialize, nullptr,
                                                                 std::span<std::uint8_t>(_inputReadBuffer));
    }
    try {
        _inputSerializerRead->Reset();
        _inputSerializerRead->set_Frame(nullptr);
        _inputSerializerRead->SetReading(true);
        _inputSerializerRead->SetInputMode(true);
        const bool hasData = data && !data->empty();
        _inputSerializerRead->CopyFromArray(hasData ? std::span<const std::uint8_t>(*data)
                                                    : std::span<const std::uint8_t>(_inputStreamReadZeroArray));
        const ::Quantum::Input input = ::Quantum::Input::Read(_inputSerializerRead.get());
        std::memcpy(buffer, &input, sizeof(input));
    } catch (const std::exception& e) {
        Log::Error("Received invalid input data from player {0}, could not deserialize.", player);
        Log::Exception(e);
        std::memset(buffer, 0, sizeof(::Quantum::Input));
    }
}

DeterministicFrameInputTemp QuantumGame::OnLocalInput(int frame, int player) {
    const bool isFirst = !_polledInputInThisSimulation;
    _polledInputInThisSimulation = true;
    try {
        PolledInput polled;
        if (PollInput) {
            if (std::optional<PolledInput> result = PollInput(frame, player, isFirst)) {
                polled = *result;
            }
        }
        if (!_inputSerializerWrite) {
            _inputWriteBuffer.assign(InputBufferSize, 0);
            _inputSerializerWrite = std::make_unique<FrameSerializer>(DeterministicFrameSerializeMode::Serialize, nullptr,
                                                                      std::span<std::uint8_t>(_inputWriteBuffer));
        }
        _inputSerializerWrite->Reset();
        _inputSerializerWrite->SetWriting(true);
        _inputSerializerWrite->SetInputMode(true);
        ::Quantum::Input::Write(_inputSerializerWrite.get(), polled.Input);
        std::vector<std::uint8_t> bytes = _inputSerializerWrite->ToArray();
        const int length = static_cast<int>(bytes.size());
        return DeterministicFrameInputTemp::Predicted(frame, player, Photon::Deterministic::MakeByteArray(std::move(bytes)),
                                                      length, polled.Flags);
    } catch (const std::exception& e) {
        Log::Error("## Input Code Threw Exception ##");
        Log::Exception(e);
        throw;
    }
}

std::shared_ptr<IDisposable> QuantumGame::CreateFrameContext() {
    if (_session == nullptr || _resourceManager == nullptr) {
        throw std::logic_error("QuantumGame::CreateFrameContext: no session or resource manager");
    }
    std::shared_ptr<AssetDB> assetDatabase = _resourceManager->CreateAssetDatabase();

    _configurations.Runtime = RuntimeConfig::FromByteArray(_session->RuntimeConfig() ? *_session->RuntimeConfig()
                                                                                     : std::vector<std::uint8_t>{});
    _configurations.Simulation =
        assetDatabase->FindAsset<SimulationConfig>(_configurations.Runtime.SimulationConfig.Id, true);
    if (_configurations.Simulation == nullptr) {
        throw std::runtime_error("QuantumGame: the runtime config's simulation config is not in the asset database");
    }
    // TODO(0x7b8a26c, Quantum.DeterministicCommandSetup.GetCommandFactories): the game's
    // command factories are not registered with the session's command serializer yet
    // (offline matches send no commands).

    // Root systems without the empty slots, then every system of their hierarchies.
    _systemsRoot = SystemSetup::CreateSystems(&_configurations.Runtime, _configurations.Simulation);
    _systemsRoot.erase(std::remove(_systemsRoot.begin(), _systemsRoot.end(), nullptr), _systemsRoot.end());
    _systemsRootPointers.clear();
    _systemsAll.clear();
    for (const auto& root : _systemsRoot) {
        _systemsRootPointers.push_back(root.get());
        for (SystemBase* system : root->get_Hierarchy()) {
            _systemsAll.push_back(system);
        }
    }

    // Heaps: one per simulation thread, one per frame the session keeps (verified,
    // predicted, and two more when interpolating), the extra ones asked for, and those of
    // the snapshot buffers.
    // TODO(0x7ca5044, Quantum.QuantumGame.SnapshotsCreateBuffers): the checksum and
    // instant-replay snapshot buffers (online desync checks, replays) are not created, so
    // they add no heaps.
    int frameHeaps = 1;
    frameHeaps += _session->IsPredicted() ? 1 : 0;
    frameHeaps += _session->IsInterpolatable() ? 1 : 0;
    frameHeaps += (_session->IsPredicted() && _session->IsInterpolatable()) ? 1 : 0;
    const int snapshotHeaps = 0;
    const int heapCount = std::max(0, _configurations.Simulation->ThreadCount) + frameHeaps +
                          std::max(0, _heapExtraCount) + snapshotHeaps;

    for (std::size_t i = 0; i < _systemsAll.size(); ++i) {
        _systemsAll[i]->set_RuntimeIndex(static_cast<int>(i));
    }

    Core::FrameContext::Args args;
    args.AssetDatabase = assetDatabase;
    // The session's platform info outlives the context; the context only reads it.
    args.PlatformInfo = const_cast<Photon::Deterministic::DeterministicPlatformInfo*>(&_session->PlatformInfo());
    args.IsServer = (_flags & 1) != 0;
    Photon::Deterministic::DeterministicSession* session = _session;
    args.IsLocalPlayer = [session](int player) { return session->IsLocalPlayer(player); };
    args.CommandSerializer = &_session->CommandSerializer();
    args.AssetSerializer = _assetSerializer;
    args.InitialDynamicAssets = _initialDynamicAssets;
    args.HeapConfig = Allocator::Heap::Config(_configurations.Simulation->HeapPageShift,
                                              _configurations.Simulation->HeapPageCount, heapCount);
    args.HeapTrackingMode = _configurations.Simulation->HeapTrackingMode;
    args.UseCullingArea = HasSystem<Core::CullingSystem2D>(_systemsAll) || HasSystem<Core::CullingSystem3D>(_systemsAll);
    args.UseNavigation = HasSystem<Core::NavigationSystem>(_systemsAll);
    args.UsePhysics2D = HasSystem<Core::PhysicsSystem2D>(_systemsAll);
    args.UsePhysics3D = HasSystem<Core::PhysicsSystem3D>(_systemsAll);
    _context = std::make_shared<Core::FrameContext>(args);

    _stats.clear();
    for (SystemBase* system : _systemsAll) {
        _stats[system];
    }
    return std::make_shared<DisposableFrameContext>(_context);
}

std::shared_ptr<DeterministicFrame> QuantumGame::CreateFrame(const std::shared_ptr<IDisposable>& context) {
    if (_session == nullptr) {
        throw std::logic_error("QuantumGame::CreateFrame: no session");
    }
    Core::FrameContext* frameContext = _context.get();
    if (auto* disposable = dynamic_cast<DisposableFrameContext*>(context.get())) {
        frameContext = disposable->Context.get();
    }
    return std::make_shared<Frame>(frameContext, _systemsAll, _systemsRootPointers, _session->SessionConfig().get(),
                                   &_configurations.Runtime, _configurations.Simulation, _session->DeltaTime());
}

std::shared_ptr<DeterministicFrame> QuantumGame::GetVerifiedFrame(int tick) {
    Log::Warn("Unable to find verified frame for tick {0}, increase {1} or increase {2}.", tick,
              "ChecksumSnapshotHistoryLengthSeconds", "ChecksumInterval");
    return nullptr;
}

ByteArray QuantumGame::GetExtraErrorFrameDumpData(DeterministicFrame& frame) {
    (void)frame;
    return nullptr;
}

void QuantumGame::OnDestroy() {
    if (GameDestroyed) {
        GameDestroyed(*this);
    }
}

void QuantumGame::AssignSession(Photon::Deterministic::DeterministicSession* session) {
    _session = session;
    if (_session == nullptr) {
        return;
    }
    std::shared_ptr<Photon::Deterministic::DeterministicSessionConfig> sessionConfig;
    ByteArray runtimeConfig;
    _session->GetLocalConfigs(sessionConfig, runtimeConfig);
    if (sessionConfig && (sessionConfig->PlayerCount < 1 || sessionConfig->PlayerCount > ::Quantum::Input::MAX_COUNT)) {
        throw std::runtime_error("Invalid player count " + std::to_string(sessionConfig->PlayerCount) +
                                 " (needs to be in 1-" + std::to_string(::Quantum::Input::MAX_COUNT) + " range)");
    }
}

void QuantumGame::OnGameStart(DeterministicFrame& state) {
    InitEventInvoker(_session->RollbackWindow());
    auto* frame = dynamic_cast<Frame*>(&state);
    if (frame == nullptr) {
        throw std::logic_error("QuantumGame::OnGameStart: not a Quantum frame");
    }
    // The session owns the frame; the container only refers to it.
    std::shared_ptr<Frame> initial(std::shared_ptr<Frame>{}, frame);
    _frames.Predicted = initial;
    _frames.PredictedPrevious = initial;
    _frames.Verified = initial;
    _frames.PreviousUpdatePredicted = initial;
    _frames.Verified->get_Context()->HookHeapAllocTrackerToFrame(_frames.Verified.get());
    if (GameStarted) {
        GameStarted(*this);
    }
    InitSystems(state);
}

void QuantumGame::OnGameResync() {}

void QuantumGame::InitSystems(DeterministicFrame& df) {
    auto& f = dynamic_cast<Frame&>(df);
    f.get_Context()->OnFrameSimulationBegin(&f);
    const auto commitBetweenSystems = [&f]() {
        if (f.get_CommitCommandsMode() == CommitCommandsModes::InBetweenSystems) {
            f.CommitAllCommands();
        }
    };
    try {
        for (SystemBase* system : _systemsAll) {
            try {
                system->OnInit(&f);
                commitBetweenSystems();
            } catch (const std::exception& e) {
                LogSimulationException(system, "OnInit", e);
            }
        }
        for (SystemBase* system : _systemsAll) {
            if (!system->get_StartEnabled()) {
                continue;
            }
            try {
                system->OnEnabled(&f);
                commitBetweenSystems();
            } catch (const std::exception& e) {
                LogSimulationException(system, "OnEnabled", e);
            }
        }
    } catch (...) {
        f.get_Context()->OnFrameSimulationEnd();
        throw;
    }
    f.get_Context()->OnFrameSimulationEnd();
}

void QuantumGame::ApplyInputs(Frame& f) {
    const int playerCount = _session->PlayerCount();
    for (int player = 0; player < playerCount; ++player) {
        const std::uint8_t* raw = f.GetRawInput(player);
        if (raw == nullptr) {
            Log::Error("Got null input for player {0}", player);
            continue;
        }
        ::Quantum::Input input;
        std::memcpy(&input, raw, sizeof(input));
        f.SetPlayerInput(player, input);
    }
}

void QuantumGame::LogSimulationException(const SystemBase* system, const char* phase, const std::exception& e) {
    auto& stats = _stats[system];
    ++stats.Exceptions;
    if (stats.LastException.empty() || stats.Exceptions <= 1) {
        stats.LastException = std::string(phase) + ": " + e.what();
        Log::Error("{0} {1}: {2}", system != nullptr ? system->ScheduleSample() : std::string("(system)"), phase, e.what());
    }
}

void QuantumGame::OnSimulate(DeterministicFrame& state) {
    auto& f = dynamic_cast<Frame&>(state);
    try {
        ApplyInputs(f);
        f.get_Context()->OnFrameSimulationBegin(&f);
        f.OnFrameSimulateBegin();
        // TODO(0x7c735f0, Quantum.Frame.UpdatePlayerData): player data from the players'
        // rpcs (RuntimePlayer) is not reimplemented yet.
        for (SystemBase* system : _systemsRootPointers) {
            if (!f.SystemIsEnabledSelf(system)) {
                continue;
            }
            ++_stats[system].Scheduled;
            try {
                system->OnSchedule(&f);
            } catch (const std::exception& e) {
                LogSimulationException(system, "OnSchedule", e);
            }
        }
        f.OnFrameSimulateEnd();
        f.get_Context()->OnFrameSimulationEnd();
        if (f.IsVerified()) {
            ++_verifiedTicks;
        }
    } catch (const std::exception& e) {
        Log::Exception(e);
    }
}

void QuantumGame::OnSimulateFinished(DeterministicFrame& state) {
    if (SimulateFinished) {
        SimulateFinished(*this, dynamic_cast<Frame&>(state));
    }
}

void QuantumGame::OnUpdateDone() {
    const auto asFrame = [](const std::shared_ptr<DeterministicFrame>& frame) {
        return std::dynamic_pointer_cast<Frame>(frame);
    };
    _frames.Predicted = asFrame(_session->FramePredicted());
    _frames.PredictedPrevious = asFrame(_session->FramePredictedPrevious());
    _frames.Verified = asFrame(_session->FrameVerified());
    _frames.PreviousUpdatePredicted = asFrame(_session->PreviousUpdateFramePredicted());
    if (_session->IsStalling()) {
        _interpolationFactor = 1.0f;
    } else {
        const double factor = _session->AccumulatedTime() / _session->DeltaTimeDouble();
        _interpolationFactor = static_cast<float>(std::clamp(factor, 0.0, 1.0));
    }
    if (UpdateView) {
        UpdateView(*this);
    }
    InvokeEvents();
}

void QuantumGame::OnChecksumError(const Photon::Deterministic::DeterministicTickChecksumError& error,
                                  std::span<const std::shared_ptr<DeterministicFrame>> frames) {
    (void)error;
    (void)frames;
}

void QuantumGame::OnChecksumErrorFrameDump(int actorId, int frameNumber,
                                           const std::shared_ptr<Photon::Deterministic::DeterministicSessionConfig>& sessionConfig,
                                           const ByteArray& runtimeConfig, const ByteArray& frameData,
                                           const ByteArray& extraData) {
    (void)actorId;
    (void)frameNumber;
    (void)sessionConfig;
    (void)runtimeConfig;
    (void)frameData;
    (void)extraData;
}

void QuantumGame::OnInputConfirmed(const DeterministicFrameInputTemp& input) { (void)input; }

void QuantumGame::OnChecksumComputed(int frame, std::uint64_t checksum) {
    if (ChecksumComputed) {
        ChecksumComputed(*this, frame, checksum);
    }
}

void QuantumGame::OnSimulationEnd() {
    if (_context) {
        _context->OnSimulationEnd();
    }
}

void QuantumGame::OnSimulationBegin() {
    _polledInputInThisSimulation = false;
    if (_context) {
        _context->OnSimulationBegin();
    }
}

void QuantumGame::OnPluginDisconnect(const std::string& reason) { (void)reason; }

void QuantumGame::InitEventInvoker(int size) { (void)size; }

void QuantumGame::InvokeEvents() {
    if (!_context) {
        return;
    }
    EventList* events = _context->get_Events();
    if (events == nullptr) {
        return;
    }
    while (events->get_Count() > 0) {
        EventBase* ev = events->PopHead();
        ++_eventCounts[ev->Id];
        _context->ReleaseEvent(ev);
    }
}

} // namespace Quantum
