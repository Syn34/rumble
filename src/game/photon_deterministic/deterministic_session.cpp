#include "deterministic_session.h"

#include "deterministic_containers.h"
#include "deterministic_frame.h"
#include "deterministic_interfaces.h"
#include "deterministic_network.h"
#include "deterministic_network_local.h"
#include "deterministic_simulator.h"
#include "local_input_provider.h"
#include "log.h"
#include "protocol.h"

#include <algorithm>
#include <exception>
#include <numeric>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

// Protocol version reported in join requests (DeterministicProtocolVersionAttribute of the
// library; the latest DeterministicProtocolVersions value is V2_2_0_0).
constexpr const char* ProtocolVersion = "2.2.0.0";

template <typename F> void LogExceptions(F&& step) {
    try {
        step();
    } catch (const std::exception& e) {
        Log::Exception(e);
    }
}

} // namespace

// --- DeltaClock -----------------------------------------------------------------------

double DeltaClock::GetDelta() {
    const double now = clock_.ElapsedSeconds();
    const double delta = now - clockPrevious_;
    clockPrevious_ = now;
    return delta < 0.0 ? 0.0 : delta;
}

// --- construction ---------------------------------------------------------------------

DeterministicSession::DeterministicSession(const DeterministicSessionArgs& args) {
    Log::Info("Platform Info: {0}", args.PlatformInfo.ToString());
    platformInfo_ = args.PlatformInfo;
    replayProvider_ = args.Replay;
    mode_ = args.Mode;
    game_ = args.Game;
    sessionConfigLocal_ = args.SessionConfig;
    runtimeConfigLocal_ = args.RuntimeConfig;
    initialTick_ = args.FrameData ? args.InitialTick : 0;
    frameData_ = args.FrameData;
    commandSerializer_ = std::make_unique<DeterministicCommandSerializer>();
    disableInterpolatableStates_ = args.DisableInterpolatableStates;

    if (!sessionConfigLocal_) {
        throw std::invalid_argument("DeterministicSession: no session config");
    }
    if (game_ == nullptr) {
        throw std::invalid_argument("DeterministicSession: no game");
    }
    if (sessionConfigLocal_->InputFixedSizeEnabled) {
        sessionConfigLocal_->InputFixedSize = game_->GetInputSerializedFixedSize();
    }
    state_ = DeterministicSessionState::Idle;
    simulatorInputOffset_ = sessionConfigLocal_->InputDelayMin;
    simulatorInputOffsetPrevious_ = sessionConfigLocal_->InputDelayMin;
    statsUpdateTimer_ = detail::Stopwatch::StartNew();

    switch (mode_) {
    case DeterministicGameMode::Local: {
        network_ = std::make_unique<DeterministicNetworkLocal>(*this);
        auto local = std::make_unique<LocalInputProvider>(game_);
        rpcProvider_ = local.get();
        inputProvider_ = local.get();
        ownedInputProvider_ = std::move(local);
        break;
    }
    case DeterministicGameMode::Replay:
        network_ = std::make_unique<DeterministicNetworkLocal>(*this);
        inputProvider_ = replayProvider_;
        rpcProvider_ = replayProvider_;
        break;
    case DeterministicGameMode::Multiplayer:
    case DeterministicGameMode::Spectating: {
        // Online-only: DeterministicNetwork is not reimplemented and throws here.
        auto network = std::make_unique<DeterministicNetwork>(*this, args.Communicator);
        rpcProvider_ = network.get();
        network_ = std::move(network);
        ownedInputProvider_ = std::make_unique<NetworkInputProvider>(game_);
        inputProvider_ = ownedInputProvider_.get();
        break;
    }
    }

    stats_ = DeterministicStats();
    simulator_ = std::make_unique<DeterministicSimulator>(*this);
    game_->AssignSession(this);
}

DeterministicSession::~DeterministicSession() = default;

// --- protocol handlers ----------------------------------------------------------------

void DeterministicSession::OnProtocolRttUpdate(int rtt) {
    maxClientRtt_ = rtt;
}

void DeterministicSession::OnProtocolRuntimeConfig(Protocol::RuntimeConfig& msg) {
    if (!msg.Requested) {
        return;
    }
    auto reply = std::make_shared<Protocol::RuntimeConfig>();
    reply->Config = runtimeConfigLocal_;
    network_->SendProtocolMessage(std::move(reply));
}

void DeterministicSession::OnProtocolSessionConfig(Protocol::SessionConfig& msg) {
    if (!msg.Requested) {
        return;
    }
    auto reply = std::make_shared<Protocol::SessionConfig>();
    reply->Config = sessionConfigLocal_;
    network_->SendProtocolMessage(std::move(reply));
}

void DeterministicSession::OnProtocolSimulationStart(Protocol::SimulationStart& msg) {
    state_ = DeterministicSessionState::Running;
    reconnect_ = msg.Reconnect;
    runtimeConfig_ = msg.RuntimeConfig;
    sessionConfig_ = msg.SessionConfig;
    if (!sessionConfig_) {
        throw std::logic_error("DeterministicSession: SimulationStart without a session config");
    }
    deltaClock_ = std::make_unique<DeltaClock>();
    deltaClock_->Start();
    rttSyncTimer_ = detail::Stopwatch::StartNew();
    stats_ = DeterministicStats();
    statsUpdateTimer_ = detail::Stopwatch::StartNew();
    lockstep_ = sessionConfig_->LockstepSimulation || mode_ == DeterministicGameMode::Replay;

    if (IsOnline()) {
        // Start the clock where the server is: its time minus the ticks before the first
        // one, plus half a round trip.
        syncClockTimer_ = detail::Stopwatch::StartNew();
        const int rtt = network_->RoundTripTime();
        const int startFrames = initialTick_ != 0 ? initialTick_ : sessionConfig_->RollbackWindow;
        const double spectating =
            mode_ == DeterministicGameMode::Spectating ? spectatingOffsetSec_ : 0.0;
        simulator_->AccumulateTime(MillisecondsAsSeconds(rtt) * 0.5 +
                                   (msg.ServerTime - startFrames * DeltaTimeDouble()) + spectating);
    }

    try {
        frameContext_ = game_->CreateFrameContext();
        simulator_->Initialize(frameContext_);
        game_->OnGameStart(*simulator_->FrameVerified());
        simulator_->OnGameStart();
    } catch (const std::exception& e) {
        Log::Exception(e);
    }

    if (initialTick_ != 0 && frameData_) {
        Log::Info("Resetting to tick {0}", initialTick_);
        Reset(*frameData_, initialTick_, true);
    }
}

void DeterministicSession::OnProtocolDisconnect(Protocol::Disconnect& msg) {
    if (game_ == nullptr) {
        throw std::logic_error("DeterministicSession: disconnect without a game");
    }
    game_->OnPluginDisconnect(msg.Reason);
    Destroy();
}

void DeterministicSession::OnProtocolChecksumError(Protocol::TickChecksumError&) {
    // Online-only (see header); intentionally empty.
}

void DeterministicSession::OnProtocolFrameSnapshotRequest(Protocol::FrameSnapshotRequest& msg) {
    if (state_ != DeterministicSessionState::Running) {
        return;
    }
    SendSnapshot(msg.ReferenceTick);
}

void DeterministicSession::OnProtocolFrameSnapshot(Protocol::FrameSnapshot&) {
    // Online-only (see header); intentionally empty.
}

void DeterministicSession::SendSnapshot(int) {
    // Online-only (see header); intentionally empty.
}

void DeterministicSession::OnTickChecksumErrorFrameDump(Protocol::TickChecksumErrorFrameDump&) {
    // Online-only (see header); intentionally empty.
}

void DeterministicSession::OnProtocolClockCorrect(double serverSeconds, double serverTimeScale) {
    if (state_ != DeterministicSessionState::Running) {
        return;
    }
    timeScale_ = serverTimeScale;
    // Outside a reconnect, samples are ignored for the first second.
    if (!reconnect_ && syncClockTimer_.ElapsedSeconds() <= 1.0) {
        return;
    }
    syncClockTimer_.Stop();
    const double spectating =
        mode_ == DeterministicGameMode::Spectating ? spectatingOffsetSec_ : 0.0;
    serverTimeReceived_ = spectating + std::max(serverTimeReceived_, serverSeconds);
}

void DeterministicSession::OnProtocolJoined(Protocol::Joined& msg) {
    if (!msg.IsConfirmed()) {
        Log::Error("Joining the session failed");
        return;
    }
    if (state_ != DeterministicSessionState::Idle) {
        Log::Error("The session was already joined");
        return;
    }
    state_ = DeterministicSessionState::Joined;
    localPlayerIndices_ = msg.PlayerSlots;
    localPlayersSet_.clear();
    localPlayersOrder_.clear();
    for (int player : localPlayerIndices_) {
        // HashSet<int> enumerates in insertion order; keep that order for polling.
        if (localPlayersSet_.insert(player).second) {
            localPlayersOrder_.push_back(player);
        }
    }
    if (msg.Status == Protocol::JoinStatus::WaitForSnapshot) {
        Log::Info("Waiting for a frame snapshot; the clock is paused");
        simulator_->SetPaused(true);
    }
}

// --- properties -----------------------------------------------------------------------

const std::shared_ptr<DeterministicFrame>& DeterministicSession::FramePredicted() const {
    return simulator_->FramePredicted();
}

const std::shared_ptr<DeterministicFrame>& DeterministicSession::FramePredictedPrevious() const {
    return simulator_->FramePredictedPrevious();
}

const std::shared_ptr<DeterministicFrame>& DeterministicSession::FrameVerified() const {
    return simulator_->FrameVerified();
}

const std::shared_ptr<DeterministicFrame>&
DeterministicSession::PreviousUpdateFramePredicted() const {
    return simulator_->FramePreviousUpdatePredicted();
}

double DeterministicSession::AccumulatedTime() const {
    return simulator_ ? simulator_->Accumulated() : 0.0;
}

int DeterministicSession::PredictedFrames() const {
    return simulator_ ? simulator_->FramesPredicted() : 0;
}

FP DeterministicSession::DeltaTime() const {
    if (!sessionConfig_) {
        return FP::FromRaw(0);
    }
    return FP::FromRaw(static_cast<FP::Raw>(FP::OneRaw / sessionConfig_->UpdateFPS));
}

double DeterministicSession::DeltaTimeDouble() const {
    return sessionConfig_ ? 1.0 / static_cast<double>(sessionConfig_->UpdateFPS) : 0.0;
}

bool DeterministicSession::IsStalling() const {
    return simulator_ && simulator_->IsStalling();
}

bool DeterministicSession::IsPaused() const {
    return simulator_ && simulator_->Paused();
}

int DeterministicSession::ClockAdjustmentAccumulationMax() const {
    if (!sessionConfig_) {
        throw std::logic_error("DeterministicSession: no session config");
    }
    return sessionConfig_->TimeCorrectionRate * 4;
}

int DeterministicSession::NextFrame() const {
    return simulator_ ? simulator_->FrameCounter() + 1 : 0;
}

// --- operations -----------------------------------------------------------------------

void DeterministicSession::GetLocalConfigs(
    std::shared_ptr<DeterministicSessionConfig>& sessionConfig, ByteArray& runtimeConfig) const {
    sessionConfig = sessionConfigLocal_;
    runtimeConfig = runtimeConfigLocal_;
}

std::unique_ptr<Dispatcher<Protocol::Message>>
DeterministicSession::CreateNetworkMessageDispatcher() {
    auto dispatcher = std::make_unique<Dispatcher<Protocol::Message>>();
    dispatcher->Bind<Protocol::RuntimeConfig>([this](auto& m) { OnProtocolRuntimeConfig(m); });
    dispatcher->Bind<Protocol::SessionConfig>([this](auto& m) { OnProtocolSessionConfig(m); });
    dispatcher->Bind<Protocol::SimulationStart>([this](auto& m) { OnProtocolSimulationStart(m); });
    dispatcher->Bind<Protocol::Disconnect>([this](auto& m) { OnProtocolDisconnect(m); });
    dispatcher->Bind<Protocol::TickChecksumError>([this](auto& m) { OnProtocolChecksumError(m); });
    dispatcher->Bind<Protocol::FrameSnapshotRequest>(
        [this](auto& m) { OnProtocolFrameSnapshotRequest(m); });
    dispatcher->Bind<Protocol::FrameSnapshot>([this](auto& m) { OnProtocolFrameSnapshot(m); });
    dispatcher->Bind<Protocol::TickChecksumErrorFrameDump>(
        [this](auto& m) { OnTickChecksumErrorFrameDump(m); });
    dispatcher->Bind<Protocol::Joined>([this](auto& m) { OnProtocolJoined(m); });
    return dispatcher;
}

bool DeterministicSession::IsFrameVerified(int frame) const {
    return simulator_ && simulator_->IsFrameVerified(frame);
}

bool DeterministicSession::IsLocalPlayer(int player) const {
    return localPlayersSet_.contains(player);
}

void DeterministicSession::SendCommand(int player, DeterministicCommand& command) {
    if (state_ != DeterministicSessionState::Running) {
        Log::Error("Commands can only be sent while the simulation is running");
        return;
    }
    if (localPlayerIndices_.empty()) {
        Log::Error("Commands cannot be sent while spectating");
        return;
    }
    BitStream& stream = commandSerializer_->CommandSerializerStreamWrite;
    stream.Reset();
    stream.SetWriting(true);
    if (!commandSerializer_->PackNext(stream, command)) {
        Log::Error("Command {0} does not fit the command buffer", command.CommandType().name());
        return;
    }
    rpcProvider_->AddRpc(player, MakeByteArray(stream.ToArray()), true);
}

void DeterministicSession::SetPlayerData(int player, ByteArray data) {
    if (state_ != DeterministicSessionState::Running) {
        Log::Error("Player data can only be set while the simulation is running");
        return;
    }
    if (localPlayerIndices_.empty()) {
        Log::Error("Player data cannot be set while spectating");
        return;
    }
    rpcProvider_->AddRpc(player, std::move(data), false);
}

void DeterministicSession::Join(const std::string& id, int playerSlots) {
    auto join = std::make_shared<Protocol::Join>();
    join->Id = id;
    join->PlayerSlots = mode_ == DeterministicGameMode::Spectating ? 0 : playerSlots;
    join->InitialTick = initialTick_;
    join->PlayerCount = sessionConfigLocal_->PlayerCount;
    join->ProtocolVersion = ProtocolVersion;
    network_->SendProtocolMessage(std::move(join));
}

void DeterministicSession::Destroy() {
    LogExceptions([&] {
        if (game_ != nullptr) {
            game_->OnDestroy();
            game_ = nullptr;
        }
    });
    LogExceptions([&] {
        if (simulator_) {
            simulator_->Destroy();
            simulator_.reset();
        }
    });
    LogExceptions([&] {
        if (network_) {
            network_->Destroy();
            network_.reset();
        }
    });
    LogExceptions([&] {
        if (frameContext_) {
            frameContext_->Dispose();
            frameContext_.reset();
        }
    });
    LogExceptions([&] {
        if (runner_) {
            runner_->Dispose();
            runner_.reset();
        }
    });
    state_ = DeterministicSessionState::Destroyed;
}

void DeterministicSession::Update(std::optional<double> deltaTime) {
    UpdateSimulation(deltaTime);
}

double DeterministicSession::FramesAsSeconds(int frames) const {
    return DeltaTimeDouble() * static_cast<double>(frames);
}

double DeterministicSession::SumList(const std::vector<double>& list) {
    double sum = 0.0;
    for (double value : list) {
        sum += value;
    }
    return sum;
}

void DeterministicSession::PerformClockAdjustment() {
    if (!(serverTimeReceived_ > serverTimeAdjusted_) || !IsOnline()) {
        return;
    }
    serverTimeAdjusted_ = serverTimeReceived_;

    const auto window = static_cast<std::size_t>(sessionConfig_->TimeCorrectionRate * 4);
    clientRttAvg_.push_back(static_cast<double>(network_->RoundTripTime()));
    if (clientRttAvg_.size() > window) {
        clientRttAvg_.erase(clientRttAvg_.begin());
    }

    // How far the local clock trails the server, using the averaged round trip.
    const double localTime =
        DeltaTimeDouble() * simulator_->FrameCounter() + simulator_->Accumulated();
    const double averageRtt = SumList(clientRttAvg_) / static_cast<double>(clientRttAvg_.size());
    serverTimeAvgDiff_.push_back(serverTimeAdjusted_ + MillisecondsAsSeconds(averageRtt) * 0.5 -
                                 localTime);
    if (serverTimeAvgDiff_.size() > window) {
        serverTimeAvgDiff_.erase(serverTimeAvgDiff_.begin());
    }
    if (serverTimeAvgDiff_.size() != window) {
        return;
    }

    // Once the window is full, correct by a share of the average difference if it is large
    // enough (MinTimeCorrectionFrames * 17 ms).
    const double count = static_cast<double>(serverTimeAvgDiff_.size());
    const double averageDiff = SumList(serverTimeAvgDiff_) / count;
    if (std::abs(averageDiff) >= sessionConfig_->MinTimeCorrectionFrames * 0.017) {
        simulator_->AdjustClock(averageDiff / count);
    }
}

double DeterministicSession::GetDeltaTime(std::optional<double> deltaTime) {
    double dt = 0.0;
    if (deltaTime) {
        dt = *deltaTime;
    } else {
        if (!deltaClock_) {
            throw std::logic_error("DeterministicSession: no delta clock before the start");
        }
        dt = deltaClock_->GetDelta();
    }
    return dt * timeScale_;
}

void DeterministicSession::Reset(std::span<const std::uint8_t> frameData, int frameNumber,
                                 bool resetInputFrame) {
    simulator_->Reset(frameData, frameNumber, resetInputFrame);
    OnReset();
}

void DeterministicSession::Reset(const DeterministicFrame& frame, bool resetInputFrame) {
    simulator_->Reset(frame, resetInputFrame);
    OnReset();
}

void DeterministicSession::OnReset() {
    simulator_->SetPaused(false);
    const int frame = simulator_->FrameVerified()->Number;
    if (network_) {
        network_->ResetInputState(frame);
    }
    game_->OnGameResync();
    if (mode_ == DeterministicGameMode::Local) {
        ConfirmInputDelayFrames(frame);
    }
}

void DeterministicSession::ConfirmInputDelayFrames(int frame) {
    for (int delay = 1; delay <= sessionConfig_->InputDelayMin; ++delay) {
        for (int player = 0; player < sessionConfig_->PlayerCount; ++player) {
            DeterministicFrameInputTemp empty;
            empty.Frame = frame + delay;
            empty.Player = player;
            empty.IsVerified = true;
            game_->OnInputConfirmed(empty);
        }
    }
}

void DeterministicSession::PushReceivedInputsIntoSimulator() {
    while (network_) {
        std::shared_ptr<DeterministicTickInput> input;
        if (!network_->InputReceiveQueue().TryPop(input)) {
            return;
        }
        DeterministicFrameInputTemp temp(input->Tick, input->PlayerIndex, input->Rpc,
                                         input->DataArray, input->DataLength, input->Flags, true);
        simulator_->InsertInput(temp);
        LogExceptions([&] { game_->OnInputConfirmed(temp); });
        inputPool_.Release(input);
    }
}

void DeterministicSession::UpdateSimulation(std::optional<double> deltaTime) {
    if (simulator_) {
        simulator_->SetIsStalling(false);
    }
    if (state_ == DeterministicSessionState::Destroyed) {
        return;
    }
    if (network_) {
        network_->Poll(); // may start (or destroy) the session
    }
    if (state_ != DeterministicSessionState::Running || checksumError_ || simulator_->Paused()) {
        return;
    }
    PushReceivedInputsIntoSimulator();
    const double dt = GetDeltaTime(deltaTime);
    UpdateFrameOffset();
    simulator_->AccumulateTime(dt);
    PerformClockAdjustment();
    simulator_->CheckIsStalling(maxVerifiedTicksPerUpdate_);
    UpdateSimulationInner();
    UpdateStats();
    CallOnUpdateDone();
}

void DeterministicSession::UpdateSimulationInner() {
    statsUpdateTimer_.Restart();
    game_->OnSimulationBegin();

    // A change of input delay since the last produced input is absorbed gradually: a
    // shorter delay skips input ticks, a longer one produces extra input for one tick.
    int offsetChange = simulatorInputOffset_ - simulatorInputOffsetPrevious_;
    const int counterAtStart = simulator_->FrameCounter();
    bool queuedInput = false;

    int frame = 0;
    while (simulator_->StepFrameNumber(frame)) {
        if (!inputProvider_->CanSimulate(frame)) {
            simulator_->StepFrameNumberDown(1);
            break;
        }
        if (mode_ != DeterministicGameMode::Replay) {
            if (offsetChange < 0) {
                ++offsetChange;
                continue;
            }
            if (offsetChange > 0) {
                simulator_->StepFrameNumberDown(1);
                frame -= offsetChange;
                --offsetChange;
            }
            frame += simulatorInputOffset_;
            if (IsOnline()) {
                // Too far ahead of the server: produce no input this tick.
                const int ahead =
                    detail::DoubleToInt32(simulator_->Accumulated() / simulator_->DeltaTime()) -
                    simulatorInputOffset_;
                if (ahead > sessionConfig_->InputHardTolerance) {
                    continue;
                }
            }
        }

        for (int player : localPlayersOrder_) {
            QTuple<ByteArray, bool> rpc = rpcProvider_->GetRpc(frame, player);
            DeterministicFrameInputTemp input = inputProvider_->GetInput(frame, player);
            if (input.Data) {
                if (input.DataLength >= 256) {
                    Log::Error("Input of {0} bytes is too large (at most 255)", input.DataLength);
                    input.Data = MakeByteArray();
                    input.DataLength = 0;
                }
            } else {
                input.Data = MakeByteArray();
                input.DataLength = 0;
            }
            if (rpc.Item0) {
                input.Rpc = rpc.Item0;
                if (rpc.Item1) {
                    input.Flags = input.Flags | DeterministicInputFlags::Command;
                }
            }
            simulator_->InsertInput(input);
            if (network_) {
                network_->QueueLocalInput(input);
                queuedInput = true;
            }
            if (mode_ == DeterministicGameMode::Local) {
                game_->OnInputConfirmed(input);
            }
        }
    }

    if (simulator_->FrameCounter() > counterAtStart) {
        simulatorInputOffsetPrevious_ = simulatorInputOffset_;
    }
    if (queuedInput) {
        network_->SendLocalInputs();
    }
    simulator_->Simulate(maxVerifiedTicksPerUpdate_);
    game_->OnSimulationEnd();
    statsUpdateTimer_.Stop();
}

void DeterministicSession::CallOnUpdateDone() {
    LogExceptions([&] { game_->OnUpdateDone(); });
}

void DeterministicSession::UpdateStats() {
    stats_.SetPing(network_->RoundTripTime());
    stats_.SetFrame(simulator_->FramePredicted()->Number);
    stats_.SetOffset(simulatorInputOffset_);
    stats_.SetPredicted(simulator_->FramesPredicted());
    stats_.SetUpdateTime(statsUpdateTimer_.ElapsedSeconds());
}

int DeterministicSession::CalculateInputOffset() const {
    const int rtt = network_->RoundTripTime();
    const double extraMs = std::max(0.0, (rtt - sessionConfigLocal_->InputDelayPingStart) / 2.0);
    const double frameMs = 1000.0 / static_cast<double>(sessionConfig_->UpdateFPS);
    return detail::CeilingToInt32(timeScale_ * (extraMs / frameMs)) +
           sessionConfigLocal_->InputDelayMin;
}

void DeterministicSession::UpdateFrameOffset() {
    const double elapsed = rttSyncTimer_.ElapsedSeconds();
    if (elapsed > lastRttSync_ + 0.25) {
        lastRttSync_ = elapsed;
        network_->SendLocalRtt(network_->RoundTripTime());
    }
    simulatorInputOffset_ = std::min(CalculateInputOffset(), sessionConfigLocal_->InputDelayMax);
}

} // namespace Photon::Deterministic
