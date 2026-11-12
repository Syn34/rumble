// deterministic_interfaces.h - the abstract seams of the PhotonDeterministic session runtime.
//
// IDeterministicGame is how the session drives the actual game (Quantum's QuantumGame):
// it creates frames, decodes input and runs one simulation step per OnSimulate call.
// IDeterministicNetwork connects the session to a server (DeterministicNetwork) or loops
// back locally (DeterministicNetworkLocal). The input and rpc providers produce the local
// players' input for each frame (LocalInputProvider offline, a replay provider when
// replaying). ICommunicator is the Photon realtime transport used by the online network.
//
// Interface method order matches the original interface slot order.
#pragma once

#include "deterministic_containers.h"
#include "deterministic_types.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace Photon::Deterministic {

class DeterministicFrame;
class DeterministicSession;
class DeterministicSessionConfig;
class DeterministicTickInput;
struct DeterministicFrameInputTemp;
struct DeterministicTickChecksumError;

namespace Protocol {
class Message;
}

// Photon.Deterministic.IDeterministicGame
class IDeterministicGame {
  public:
    virtual ~IDeterministicGame() = default;

    virtual DeterministicSession* Session() const = 0; // slot 0
    virtual int GetInputSerializedFixedSize() = 0;     // slot 1
    virtual int GetInputInMemorySize() = 0;            // slot 2
    // Decodes a player's serialised input (may be null) into `buffer`, which holds
    // GetInputInMemorySize() bytes.
    virtual void DeserializeInputInto(int player, const ByteArray& data,
                                      std::uint8_t* buffer) = 0; // slot 3
    // Polls the local player's input for `frame`.
    virtual DeterministicFrameInputTemp OnLocalInput(int frame, int player) = 0; // slot 4
    virtual std::shared_ptr<IDisposable> CreateFrameContext() = 0;               // slot 5
    virtual std::shared_ptr<DeterministicFrame>
    CreateFrame(const std::shared_ptr<IDisposable>& context) = 0;                // slot 6
    virtual std::shared_ptr<DeterministicFrame> GetVerifiedFrame(int tick) = 0;  // slot 7
    virtual ByteArray GetExtraErrorFrameDumpData(DeterministicFrame& frame) = 0; // slot 8
    virtual void OnDestroy() = 0;                                                // slot 9
    virtual void AssignSession(DeterministicSession* session) = 0;               // slot 10
    virtual void OnGameStart(DeterministicFrame& state) = 0;                     // slot 11
    virtual void OnGameResync() = 0;                                             // slot 12
    virtual void OnSimulate(DeterministicFrame& state) = 0;                      // slot 13
    virtual void OnSimulateFinished(DeterministicFrame& state) = 0;              // slot 14
    virtual void OnUpdateDone() = 0;                                             // slot 15
    virtual void OnChecksumError(const DeterministicTickChecksumError& error,
                                 std::span<const std::shared_ptr<DeterministicFrame>> frames) = 0;
    virtual void
    OnChecksumErrorFrameDump(int actorId, int frameNumber,
                             const std::shared_ptr<DeterministicSessionConfig>& sessionConfig,
                             const ByteArray& runtimeConfig, const ByteArray& frameData,
                             const ByteArray& extraData) = 0;                    // slot 17
    virtual void OnInputConfirmed(const DeterministicFrameInputTemp& input) = 0; // slot 18
    virtual void OnChecksumComputed(int frame, std::uint64_t checksum) = 0;      // slot 19
    virtual void OnSimulationEnd() = 0;                                          // slot 20
    virtual void OnSimulationBegin() = 0;                                        // slot 21
    virtual void OnPluginDisconnect(const std::string& reason) = 0;              // slot 22
};

// Photon.Deterministic.IDeterministicRpcProvider
class IDeterministicRpcProvider {
  public:
    virtual ~IDeterministicRpcProvider() = default;
    // The next queued rpc of `player` (Item0, may be null) and whether it is a command.
    virtual QTuple<ByteArray, bool> GetRpc(int frame, int player) = 0;
    virtual void AddRpc(int player, ByteArray data, bool command) = 0;
};

// Photon.Deterministic.IDeterministicInputProvider
class IDeterministicInputProvider {
  public:
    virtual ~IDeterministicInputProvider() = default;
    virtual bool CanSimulate(int frame) = 0;
    virtual DeterministicFrameInputTemp GetInput(int frame, int player) = 0;
};

// Photon.Deterministic.IDeterministicReplayProvider: supplies both input and rpcs when a
// recorded match is replayed. Declares nothing of its own.
class IDeterministicReplayProvider : public IDeterministicInputProvider,
                                     public IDeterministicRpcProvider {};

// Photon.Deterministic.IDeterministicNetwork
class IDeterministicNetwork {
  public:
    virtual ~IDeterministicNetwork() = default;
    virtual int RoundTripTime() const = 0; // slot 0
    // Confirmed inputs received from the network, consumed by the session.
    virtual NetworkQueue<std::shared_ptr<DeterministicTickInput>>& InputReceiveQueue() = 0;
    virtual void Poll() = 0;                                                      // slot 2
    virtual void Destroy() = 0;                                                   // slot 3
    virtual void SendLocalRtt(int rtt) = 0;                                       // slot 4
    virtual void SendLocalChecksum(int tick, std::uint64_t checksum) = 0;         // slot 5
    virtual void SendLocalInputs() = 0;                                           // slot 6
    virtual void QueueLocalInput(const DeterministicFrameInputTemp& input) = 0;   // slot 7
    virtual void ResetInputState(int frame) = 0;                                  // slot 8
    virtual void SendProtocolMessage(std::shared_ptr<Protocol::Message> msg) = 0; // slot 9
};

// Photon.Deterministic.OnEventReceived (delegate; Invoke 0x7a83744).
using OnEventReceived = std::function<void(std::uint8_t eventCode, const ByteArray& data,
                                           int dataLength, void* dataContainer)>;

// Photon.Deterministic.ICommunicator: the realtime transport. Online-only; the offline PC
// build never creates one.
class ICommunicator {
  public:
    virtual ~ICommunicator() = default;
    virtual int RoundTripTime() const = 0;
    virtual void Service() = 0;
    virtual void OnDestroy() = 0;
    virtual void DisposeEventObject(void* obj) = 0;
    virtual void RaiseEvent(std::uint8_t eventCode, const ByteArray& message, int messageLength,
                            bool reliable, std::span<const int> toPlayers) = 0;
    virtual void AddEventListener(OnEventReceived onEventReceived) = 0;
};

// Photon.Deterministic.IDeterministicPlatformTaskRunner: runs simulation jobs on worker
// threads.
class IDeterministicPlatformTaskRunner {
  public:
    virtual ~IDeterministicPlatformTaskRunner() = default;
    virtual void Schedule(std::span<const std::function<void()>> delegates) = 0;
    virtual bool PollForComplete() = 0;
};

} // namespace Photon::Deterministic
