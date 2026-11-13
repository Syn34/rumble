// quantum_game.h - Quantum.QuantumGame (quantum.code.dll): the game object the deterministic
// session drives.
//
// QuantumGame implements IDeterministicGame. The session asks it to create the frame context
// (configs, systems, heaps), the frames, and to simulate one tick per OnSimulate call; it
// polls the local players' input through it and tells it when an update is done. The game
// keeps the latest frames of the session (verified, predicted...) for the view.
//
// On Android the Unity side talks to the game through a callback dispatcher and an event
// dispatcher. The PC build has neither; the callbacks it needs are plain std::function
// members (PollInput, GameStarted, UpdateView, SimulateFinished, ChecksumComputed).
#pragma once

#include "quantum_code/frame.h"
#include "quantum_code/input.h"
#include "quantum_code/runtime_config.h"
#include "quantum_code/simulation_config.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"

#include "photon_deterministic/deterministic_frame.h"
#include "photon_deterministic/deterministic_interfaces.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Quantum {

class IResourceManager;
class IAssetSerializer;
class DynamicAssetDB;

// Quantum.QuantumGameStartParameters (the parts the PC build uses).
struct QuantumGameStartParameters {
    // Not owned; must outlive the game.
    IResourceManager* ResourceManager = nullptr;
    IAssetSerializer* AssetSerializer = nullptr;
    int HeapExtraCount = 0;
    std::shared_ptr<DynamicAssetDB> InitialDynamicAssets;
    // QuantumGameFlags; bit 0 makes the frame context a server context.
    int GameFlags = 0;
};

// What the poll-input callback hands back (Quantum.CallbackPollInput: SetInput).
struct PolledInput {
    ::Quantum::Input Input{};
    Photon::Deterministic::DeterministicInputFlags Flags = Photon::Deterministic::DeterministicInputFlags::None;
};

class QuantumGame final : public Photon::Deterministic::IDeterministicGame {
  public:
    // QuantumGame.FramesContainer: the frames of the last session update.
    struct FramesContainer {
        std::shared_ptr<Frame> Verified;
        std::shared_ptr<Frame> Predicted;
        std::shared_ptr<Frame> PredictedPrevious;
        std::shared_ptr<Frame> PreviousUpdatePredicted;
    };
    // QuantumGame.ConfigurationsContainer.
    struct ConfigurationsContainer {
        RuntimeConfig Runtime;
        SimulationConfig* Simulation = nullptr;
    };

    // 0x7ca5304
    explicit QuantumGame(const QuantumGameStartParameters& startParameters);
    ~QuantumGame() override;

    QuantumGame(const QuantumGame&) = delete;
    QuantumGame& operator=(const QuantumGame&) = delete;

    // --- properties ----------------------------------------------------------------------
    // 0x7ca52b0
    const FramesContainer& Frames() const { return _frames; }
    // 0x7ca52b8
    const ConfigurationsContainer& Configurations() const { return _configurations; }
    // 0x7ca52c0
    Photon::Deterministic::DeterministicSession* Session() const override { return _session; }
    // 0x7ca52d0 - How far the view is between the previous and the current predicted frame.
    float InterpolationFactor() const { return _interpolationFactor; }
    // 0x7ca52fc
    int HeapExtraCount() const { return _heapExtraCount; }
    const std::vector<SystemBase*>& SystemsAll() const { return _systemsAll; }
    const std::vector<SystemBase*>& SystemsRoot() const { return _systemsRootPointers; }
    Core::FrameContext* Context() const { return _context.get(); }

    // 0x7ca58a0
    const std::vector<int>& GetLocalPlayers() const;
    // 0x7ca58b8
    bool PlayerIsLocal(int player) const;

    // --- IDeterministicGame --------------------------------------------------------------
    // 0x7caa194 - Bytes of a default input written in input mode.
    int GetInputSerializedFixedSize() override;
    // 0x7caa18c
    int GetInputInMemorySize() override { return ::Quantum::Input::SIZE; }
    // 0x7caa280 - Reads an Input (input mode) from the player's bytes, or from zeros when
    // there are none. Undecodable data is logged and leaves a zeroed input.
    void DeserializeInputInto(int player, const Photon::Deterministic::ByteArray& data,
                              std::uint8_t* buffer) override;
    // 0x7ca7750 - Asks the poll-input callback for the player's input and writes it in input
    // mode as a predicted (unconfirmed) input.
    Photon::Deterministic::DeterministicFrameInputTemp OnLocalInput(int frame, int player) override;
    // 0x7ca5ff4 - Runtime config from the session, simulation config from the asset
    // database, systems from SystemSetup, then the frame context sized for them.
    std::shared_ptr<Photon::Deterministic::IDisposable> CreateFrameContext() override;
    // 0x7ca5d98
    std::shared_ptr<Photon::Deterministic::DeterministicFrame>
    CreateFrame(const std::shared_ptr<Photon::Deterministic::IDisposable>& context) override;
    // TODO(0x7ca5eb0, Quantum.QuantumGame.GetVerifiedFrame): needs the checksum snapshot
    // ring buffer (SnapshotsCreateBuffers 0x7ca5044); only used for online desync dumps.
    std::shared_ptr<Photon::Deterministic::DeterministicFrame> GetVerifiedFrame(int tick) override;
    // TODO(0x7caa580, Quantum.QuantumGame.GetExtraErrorFrameDumpData): online desync dumps.
    Photon::Deterministic::ByteArray GetExtraErrorFrameDumpData(Photon::Deterministic::DeterministicFrame& frame) override;
    // 0x7ca5a90 - Snapshots, destroyed callback, heap tracking check (the latter two are
    // only the callback here).
    void OnDestroy() override;
    // 0x7ca91a8 - Also checks the session's player count (1..32). The memory layout
    // verification of the original is not needed: the PC structs are checked at compile
    // time.
    void AssignSession(Photon::Deterministic::DeterministicSession* session) override;
    // 0x7ca6bdc - Every frame slot starts as the initial frame, then the systems start.
    void OnGameStart(Photon::Deterministic::DeterministicFrame& state) override;
    // TODO(0x7ca7434, Quantum.QuantumGame.OnGameResync): online resync only.
    void OnGameResync() override;
    // 0x7ca7d8c - One tick: inputs, frame begin, player data, every enabled root system
    // (exceptions are logged per system and the tick goes on), frame end.
    void OnSimulate(Photon::Deterministic::DeterministicFrame& state) override;
    // 0x7ca86f0
    void OnSimulateFinished(Photon::Deterministic::DeterministicFrame& state) override;
    // 0x7ca8918 - Takes the session's frames, the interpolation factor, updates the view and
    // dispatches events.
    void OnUpdateDone() override;
    // TODO(0x7ca9464, Quantum.QuantumGame.OnChecksumError): online desync handling.
    void OnChecksumError(const Photon::Deterministic::DeterministicTickChecksumError& error,
                         std::span<const std::shared_ptr<Photon::Deterministic::DeterministicFrame>> frames) override;
    // TODO(0x7ca9b34, Quantum.QuantumGame.OnChecksumErrorFrameDump): online desync dumps.
    void OnChecksumErrorFrameDump(int actorId, int frameNumber,
                                  const std::shared_ptr<Photon::Deterministic::DeterministicSessionConfig>& sessionConfig,
                                  const Photon::Deterministic::ByteArray& runtimeConfig,
                                  const Photon::Deterministic::ByteArray& frameData,
                                  const Photon::Deterministic::ByteArray& extraData) override;
    // 0x7ca98c8 - Replay recording (not reimplemented) and the confirmed-input callback.
    void OnInputConfirmed(const Photon::Deterministic::DeterministicFrameInputTemp& input) override;
    // 0x7ca96b8 - Checksum recording (not reimplemented) and the callback.
    void OnChecksumComputed(int frame, std::uint64_t checksum) override;
    // 0x7ca9890
    void OnSimulationEnd() override;
    // 0x7ca98a8 - Input may be polled again for the first time in this simulation.
    void OnSimulationBegin() override;
    // 0x7ca9f60 - Only the callback (online only).
    void OnPluginDisconnect(const std::string& reason) override;

    // --- PC callbacks (stand-ins for the callback dispatcher) ----------------------------
    // CallbackPollInput: (frame, player, first poll in this simulation) -> input, or nothing
    // for a zeroed input.
    std::function<std::optional<PolledInput>(int frame, int player, bool isFirstInThisUpdate)> PollInput;
    std::function<void(QuantumGame&)> GameStarted;
    std::function<void(QuantumGame&)> UpdateView;
    std::function<void(QuantumGame&, Frame&)> SimulateFinished;
    std::function<void(QuantumGame&, int frame, std::uint64_t checksum)> ChecksumComputed;
    std::function<void(QuantumGame&)> GameDestroyed;

    // --- diagnostics (PC) ----------------------------------------------------------------
    struct SystemStats {
        std::uint64_t Scheduled = 0;   // OnSchedule calls (root systems)
        std::uint64_t Exceptions = 0;  // exceptions caught around OnInit/OnEnabled/OnSchedule
        std::string LastException;
    };
    // Per system (by pointer into SystemsAll()).
    const std::map<const SystemBase*, SystemStats>& Stats() const { return _stats; }
    // Events added by the simulation, by event id, since the start.
    const std::map<int, std::uint64_t>& EventCounts() const { return _eventCounts; }
    std::uint64_t VerifiedTicks() const { return _verifiedTicks; }

  private:
    // 0x7ca7054 - OnInit of every system, then OnEnabled of those that start enabled;
    // commands are committed between systems when the simulation config asks for it.
    void InitSystems(Photon::Deterministic::DeterministicFrame& df);
    // 0x7ca847c - Copies every player's decoded input into the frame's global block.
    void ApplyInputs(Frame& f);
    // 0x7ca8630 - The original logs the exception with the system's context.
    void LogSimulationException(const SystemBase* system, const char* phase, const std::exception& e);
    // 0x7ca6e44 - TODO(0x7ca6e44, Quantum.QuantumGame.InitEventInvoker): event confirmation
    // per verified tick is not reimplemented (see InvokeEvents).
    void InitEventInvoker(int size);
    // TODO(0x7ca8d5c, Quantum.QuantumGame.InvokeEvents): events are not confirmed, cancelled
    // or dispatched to a view yet; they are counted and returned to their pools.
    void InvokeEvents();
    // 0x7ca5518 - The callback objects of the original; nothing to create here.
    void InitCallbacks() {}

    FramesContainer _frames;
    ConfigurationsContainer _configurations;
    Photon::Deterministic::DeterministicSession* _session = nullptr;
    float _interpolationFactor = 0.0f;
    int _heapExtraCount = 0;
    IResourceManager* _resourceManager = nullptr;
    IAssetSerializer* _assetSerializer = nullptr;
    std::shared_ptr<DynamicAssetDB> _initialDynamicAssets;
    int _flags = 0;
    std::vector<std::uint8_t> _inputStreamReadZeroArray;
    std::vector<std::uint8_t> _inputReadBuffer;
    std::vector<std::uint8_t> _inputWriteBuffer;
    std::unique_ptr<FrameSerializer> _inputSerializerRead;
    std::unique_ptr<FrameSerializer> _inputSerializerWrite;
    std::vector<std::unique_ptr<SystemBase>> _systemsRoot;
    std::vector<SystemBase*> _systemsRootPointers;
    std::vector<SystemBase*> _systemsAll;
    std::shared_ptr<Core::FrameContext> _context;
    bool _polledInputInThisSimulation = false;

    std::map<const SystemBase*, SystemStats> _stats;
    std::map<int, std::uint64_t> _eventCounts;
    std::uint64_t _verifiedTicks = 0;
};

} // namespace Quantum
