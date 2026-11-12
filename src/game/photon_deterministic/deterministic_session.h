// deterministic_session.h - Photon.Deterministic.DeterministicSession: one running match.
//
// The session owns the simulator and a network layer and drives both from the host's frame
// loop. Lifecycle:
//   - construct with DeterministicSessionArgs (mode, configs, game, platform);
//   - Join(): sends a join request through the network layer. Offline,
//     DeterministicNetworkLocal answers it on the next Update with Joined (all requested
//     player slots become local) and SimulationStart (the local configs), which starts the
//     simulation;
//   - Update(deltaTime) every host frame: polls the network, turns elapsed time into input
//     ticks, collects local input (with the configured input delay), then lets the
//     simulator verify and predict;
//   - Destroy().
// Rpcs and commands of local players are queued with SetPlayerData / SendCommand and ride
// along with the next polled input.
#pragma once

#include "deterministic_command.h"
#include "deterministic_platform_info.h"
#include "deterministic_session_config.h"
#include "deterministic_stats.h"
#include "deterministic_tick_input.h"
#include "deterministic_types.h"
#include "fp.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Photon::Deterministic {

class DeterministicFrame;
class DeterministicSimulator;
class ICommunicator;
class IDeterministicGame;
class IDeterministicInputProvider;
class IDeterministicNetwork;
class IDeterministicReplayProvider;
class IDeterministicRpcProvider;
template <typename T> class Dispatcher;

namespace Protocol {
class Message;
class RuntimeConfig;
class SessionConfig;
class SimulationStart;
class Disconnect;
class TickChecksumError;
class FrameSnapshotRequest;
class FrameSnapshot;
class TickChecksumErrorFrameDump;
class Joined;
} // namespace Protocol

namespace detail {

// The parts of System.Diagnostics.Stopwatch the session uses.
class Stopwatch {
  public:
    static Stopwatch StartNew() {
        Stopwatch watch;
        watch.Start();
        return watch;
    }
    void Start() {
        if (!running_) {
            startedAt_ = Clock::now();
            running_ = true;
        }
    }
    void Stop() {
        if (running_) {
            elapsed_ += Clock::now() - startedAt_;
            running_ = false;
        }
    }
    void Restart() {
        elapsed_ = Clock::duration::zero();
        startedAt_ = Clock::now();
        running_ = true;
    }
    double ElapsedSeconds() const {
        Clock::duration total = elapsed_;
        if (running_) {
            total += Clock::now() - startedAt_;
        }
        return std::chrono::duration<double>(total).count();
    }

  private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point startedAt_{};
    Clock::duration elapsed_{};
    bool running_ = false;
};

} // namespace detail

// Photon.Deterministic.DeltaClock: wall-clock seconds between successive GetDelta calls.
class DeltaClock {
  public:
    // 0x7a7ddec
    DeltaClock() = default;
    // 0x7a7de58
    void Start() { clock_.Start(); }
    // 0x7a81ccc - seconds since the previous call (never negative).
    double GetDelta();

  private:
    detail::Stopwatch clock_;
    double clockPrevious_ = 0.0;
};

// Photon.Deterministic.DeterministicSessionArgs
struct DeterministicSessionArgs {
    DeterministicGameMode Mode = DeterministicGameMode::Local;
    std::shared_ptr<DeterministicSessionConfig> SessionConfig;
    // Not owned; must outlive the session (until Destroy).
    IDeterministicGame* Game = nullptr;
    // Online only; not owned.
    ICommunicator* Communicator = nullptr;
    // Replay mode only; not owned.
    IDeterministicReplayProvider* Replay = nullptr;
    DeterministicPlatformInfo PlatformInfo;
    // With FrameData: the tick the simulation restarts at after it started.
    int InitialTick = 0;
    bool DisableInterpolatableStates = false;
    ByteArray FrameData;
    ByteArray RuntimeConfig;
};

class DeterministicSession {
  public:
    // 0x7a7f998
    explicit DeterministicSession(const DeterministicSessionArgs& args);
    ~DeterministicSession();

    DeterministicSession(const DeterministicSession&) = delete;
    DeterministicSession& operator=(const DeterministicSession&) = delete;

    // --- protocol message handlers (bound by CreateNetworkMessageDispatcher) ----------
    // 0x7a7d6b0
    void OnProtocolRttUpdate(int rtt);
    // 0x7a7d6b8 - answers a config request with the local runtime config.
    void OnProtocolRuntimeConfig(Protocol::RuntimeConfig& msg);
    // 0x7a7d7b8 - answers a config request with the local session config.
    void OnProtocolSessionConfig(Protocol::SessionConfig& msg);
    // 0x7a7d8b8 - adopts the final configs, creates the frames and starts the clock.
    void OnProtocolSimulationStart(Protocol::SimulationStart& msg);
    // 0x7a7e198 - tells the game and destroys the session.
    void OnProtocolDisconnect(Protocol::Disconnect& msg);
    // TODO(0x7a7e718, Photon.Deterministic.DeterministicSession.OnProtocolChecksumError):
    // online-only; not used by the offline PC build. Stops on a server-reported desync and
    // uploads frame dumps.
    void OnProtocolChecksumError(Protocol::TickChecksumError& msg);
    // 0x7a7ec10 - only while running.
    void OnProtocolFrameSnapshotRequest(Protocol::FrameSnapshotRequest& msg);
    // TODO(0x7a7ed60, Photon.Deterministic.DeterministicSession.OnProtocolFrameSnapshot):
    // online-only; not used by the offline PC build. Collects snapshot blocks of a late
    // join and resets to the decoded frame.
    void OnProtocolFrameSnapshot(Protocol::FrameSnapshot& msg);
    // TODO(0x7a7ec30, Photon.Deterministic.DeterministicSession.SendSnapshot): online-only;
    // not used by the offline PC build. Serialises a verified frame and sends it in blocks.
    void SendSnapshot(int referenceTick);
    // TODO(0x7a7eebc, Photon.Deterministic.DeterministicSession.OnTickChecksumErrorFrameDump):
    // online-only; not used by the offline PC build. Reassembles another client's frame
    // dump and hands it to the game.
    void OnTickChecksumErrorFrameDump(Protocol::TickChecksumErrorFrameDump& msg);
    // 0x7a7f448 - server clock sample; adopts the server time scale.
    void OnProtocolClockCorrect(double serverSeconds, double serverTimeScale);
    // 0x7a7f56c - records the local player slots; pauses the clock if a snapshot follows.
    void OnProtocolJoined(Protocol::Joined& msg);

    // --- properties -------------------------------------------------------------------
    // 0x7a7f7d4
    int MaxVerifiedTicksPerUpdate() const { return maxVerifiedTicksPerUpdate_; }
    // 0x7a7f7dc
    const ByteArray& RuntimeConfig() const { return runtimeConfig_; }
    // 0x7a7f7e4
    const DeterministicStats& Stats() const { return stats_; }
    // 0x7a7f7ec
    IDeterministicGame* Game() const { return game_; }
    // 0x7a7f7f4
    const std::shared_ptr<DeterministicFrame>& FramePredicted() const;
    // 0x7a7f80c
    const std::shared_ptr<DeterministicFrame>& FramePredictedPrevious() const;
    // 0x7a7f824
    const std::shared_ptr<DeterministicFrame>& FrameVerified() const;
    // 0x7a7f83c
    const std::shared_ptr<DeterministicFrame>& PreviousUpdateFramePredicted() const;
    // 0x7a7f854 - the config the simulation runs with (null before SimulationStart).
    const std::shared_ptr<DeterministicSessionConfig>& SessionConfig() const {
        return sessionConfig_;
    }
    // 0x7a7f85c
    double AccumulatedTime() const;
    // 0x7a7f874
    int InitialTick() const { return initialTick_; }
    // 0x7a7f87c
    const std::vector<int>& LocalPlayerIndices() const { return localPlayerIndices_; }
    // 0x7a7a740
    int PlayerCount() const { return sessionConfig_ ? sessionConfig_->PlayerCount : 0; }
    // 0x7a7af58
    int RollbackWindow() const { return sessionConfig_ ? sessionConfig_->RollbackWindow : 0; }
    // 0x7a7f884
    int PredictedFrames() const;
    // 0x7a7f89c - one tick as FP (1/UpdateFPS, integer-divided in raw units).
    FP DeltaTime() const;
    // 0x7a7de70
    double DeltaTimeDouble() const;
    // 0x7a7f8c0
    DeterministicGameMode GameMode() const { return mode_; }
    // 0x7a7aaf8
    bool IsOnline() const {
        return mode_ == DeterministicGameMode::Multiplayer ||
               mode_ == DeterministicGameMode::Spectating;
    }
    // 0x7a7de94
    bool IsSpectating() const { return mode_ == DeterministicGameMode::Spectating; }
    // 0x7a7f8c8
    bool IsStalling() const;
    // 0x7a7f8e8
    bool IsPaused() const;
    // 0x7a7f908
    bool IsLockstep() const { return lockstep_; }
    // 0x7a7b26c
    bool IsPredicted() const { return !lockstep_; }
    // 0x7a7f910
    bool IsReplay() const { return mode_ == DeterministicGameMode::Replay; }
    // 0x7a7ab70
    bool IsInterpolatable() const { return !disableInterpolatableStates_; }
    // 0x7a7f920
    DeterministicSessionState State() const { return state_; }
    // 0x7a7f928
    IDeterministicReplayProvider* ReplayProvider() const { return replayProvider_; }
    // 0x7a7f930
    const DeterministicPlatformInfo& PlatformInfo() const { return platformInfo_; }
    // 0x7a7f938
    DeterministicCommandSerializer& CommandSerializer() const { return *commandSerializer_; }
    // 0x7a7f940 / 0x7a7f948 - an object disposed together with the session.
    const std::shared_ptr<IDisposable>& Runner() const { return runner_; }
    void SetRunner(std::shared_ptr<IDisposable> value) { runner_ = std::move(value); }
    // 0x7a7f958 - throws if there is no session config yet.
    int ClockAdjustmentAccumulationMax() const;
    // 0x7a7f974
    int NextFrame() const;
    // 0x7a7f990
    DeterministicTickInput::Pool& InputPool() { return inputPool_; }

    // --- operations -------------------------------------------------------------------
    // 0x7a809f4
    void GetLocalConfigs(std::shared_ptr<DeterministicSessionConfig>& sessionConfig,
                         ByteArray& runtimeConfig) const;
    // 0x7a80a30 - a dispatcher that routes the server's messages to the OnProtocol*
    // handlers of this session.
    std::unique_ptr<Dispatcher<Protocol::Message>> CreateNetworkMessageDispatcher();
    // 0x7a80e6c
    bool IsFrameVerified(int frame) const;
    // 0x7a80e94
    bool IsLocalPlayer(int player) const;
    // 0x7a80ef4 - packs the command and queues it as the player's next rpc.
    void SendCommand(int player, DeterministicCommand& command);
    // 0x7a81294 - queues raw player data as the player's next rpc.
    void SetPlayerData(int player, ByteArray data);
    // 0x7a81478 - asks for `playerSlots` local players (none when spectating).
    void Join(const std::string& id, int playerSlots);
    // 0x7a7e254 - tears everything down; each step's exceptions are logged and skipped.
    void Destroy();
    // 0x7a816b0 - one host frame; `deltaTime` in seconds, or measured by the session's
    // clock when absent.
    void Update(std::optional<double> deltaTime);
    // 0x7a8182c
    double FramesAsSeconds(int frames) const;
    // 0x7a81858
    static double MillisecondsAsSeconds(double milliseconds) { return milliseconds / 1000.0; }
    // 0x7a81868
    static double SumList(const std::vector<double>& list);
    // 0x7a81900 - nudges the local clock toward the server clock (online modes only).
    void PerformClockAdjustment();
    // 0x7a81c40
    double GetDeltaTime(std::optional<double> deltaTime);
    // 0x7a81d64
    void ResetReplay(const DeterministicFrame& frame) { Reset(frame, true); }
    // 0x7a7eeb4
    void Resync(std::span<const std::uint8_t> frameData, int frameNumber) {
        Reset(frameData, frameNumber, false);
    }
    // 0x7a7e100
    void Reset(std::span<const std::uint8_t> frameData, int frameNumber, bool resetInputFrame);
    // 0x7a81d6c
    void Reset(const DeterministicFrame& frame, bool resetInputFrame);

  private:
    friend class DeterministicSimulator;
    friend class DeterministicNetworkLocal;

    // 0x7a81db4 - resumes the clock and replays the input-delay notifications.
    void OnReset();
    // 0x7a82030 - moves confirmed inputs received from the network into the simulator.
    void PushReceivedInputsIntoSimulator();
    // 0x7a816b4
    void UpdateSimulation(std::optional<double> deltaTime);
    // 0x7a824b8 - produces local input for every new tick, then simulates.
    void UpdateSimulationInner();
    // 0x7a82fe4
    void CallOnUpdateDone();
    // 0x7a82eb0
    void UpdateStats();
    // 0x7a8310c - input delay in ticks for the current round trip.
    int CalculateInputOffset() const;
    // 0x7a822f4
    void UpdateFrameOffset();

    // Fires game.OnInputConfirmed with empty confirmed input for the input-delay ticks
    // following `frame` (offline only).
    void ConfirmInputDelayFrames(int frame);

    std::vector<std::shared_ptr<Protocol::FrameSnapshot>> snapshots_;
    ByteArray frameData_;
    int initialTick_ = 0;
    ByteArray runtimeConfig_;
    ByteArray runtimeConfigLocal_;
    std::shared_ptr<DeterministicSessionConfig> sessionConfig_;
    std::shared_ptr<DeterministicSessionConfig> sessionConfigLocal_;
    std::unordered_set<int> localPlayersSet_;
    std::vector<int> localPlayersOrder_;
    std::shared_ptr<IDisposable> frameContext_;
    std::unique_ptr<IDeterministicNetwork> network_;
    IDeterministicRpcProvider* rpcProvider_ = nullptr;
    IDeterministicInputProvider* inputProvider_ = nullptr;
    std::unique_ptr<IDeterministicInputProvider> ownedInputProvider_;
    int simulatorInputOffset_ = 0;
    int simulatorInputOffsetPrevious_ = 0;
    std::unique_ptr<DeterministicSimulator> simulator_;
    std::shared_ptr<DeterministicTickChecksumError> checksumError_;
    std::unique_ptr<DeterministicCommandSerializer> commandSerializer_;
    DeterministicGameMode mode_ = DeterministicGameMode::Local;
    DeterministicSessionState state_ = DeterministicSessionState::Idle;
    std::unique_ptr<DeltaClock> deltaClock_;
    detail::Stopwatch syncClockTimer_;
    detail::Stopwatch statsUpdateTimer_;
    detail::Stopwatch rttSyncTimer_;
    bool lockstep_ = false;
    bool disableInterpolatableStates_ = false;
    bool reconnect_ = false;
    double serverTimeReceived_ = 0.0;
    double serverTimeAdjusted_ = 0.0;
    double lastRttSync_ = 0.0;
    double timeScale_ = 1.0;
    std::vector<double> clientRttAvg_;
    std::vector<double> serverTimeAvgDiff_;
    int maxClientRtt_ = 0;
    std::vector<int> localPlayerIndices_;
    DeterministicStats stats_;
    IDeterministicGame* game_ = nullptr;
    IDeterministicReplayProvider* replayProvider_ = nullptr;
    DeterministicPlatformInfo platformInfo_;
    DeterministicTickInput::Pool inputPool_;
    std::unordered_map<int, std::shared_ptr<ChecksumErrorFrameDump>> checksumErrorFrameDumps_;
    int maxVerifiedTicksPerUpdate_ = 0x7fffffff;
    double spectatingOffsetSec_ = -1.0;
    std::shared_ptr<IDisposable> runner_;
};

} // namespace Photon::Deterministic
