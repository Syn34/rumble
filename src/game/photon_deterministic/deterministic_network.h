// deterministic_network.h - Photon.Deterministic.DeterministicNetwork and
// DeterministicTickInputDecoder: the online network layer.
//
// Online-only; not used by the offline PC build. The class shapes and original addresses
// are kept so the session's Multiplayer/Spectating paths compile and the work list stays
// traceable, but nothing is reimplemented: constructing either type throws.
#pragma once

#include "deterministic_containers.h"
#include "deterministic_interfaces.h"
#include "deterministic_tick_input.h"

#include <memory>

namespace Photon::Deterministic {

class BitStream;
class DeterministicSession;

namespace Protocol {
class Serializer;
class InputMissing;
} // namespace Protocol

class DeterministicNetwork final : public IDeterministicNetwork, public IDeterministicRpcProvider {
  public:
    // TODO(0x7a80100, Photon.Deterministic.DeterministicNetwork..ctor): online-only; not used
    // by the offline PC build. Throws std::logic_error.
    DeterministicNetwork(DeterministicSession& session, ICommunicator* communicator);

    // TODO(0x7a887fc, Photon.Deterministic.DeterministicNetwork.get_RoundTripTime): online-only;
    // not used by the offline PC build.
    int RoundTripTime() const override;
    // TODO(0x7a8889c, Photon.Deterministic.DeterministicNetwork.get_InputReceiveQueue):
    // online-only; not used by the offline PC build.
    NetworkQueue<std::shared_ptr<DeterministicTickInput>>& InputReceiveQueue() override;
    // TODO(0x7a888a4, Photon.Deterministic.DeterministicNetwork.SendLocalRtt): online-only;
    // not used by the offline PC build.
    void SendLocalRtt(int rtt) override;
    // TODO(0x7a88938, Photon.Deterministic.DeterministicNetwork.SendLocalChecksum): online-only;
    // not used by the offline PC build.
    void SendLocalChecksum(int tick, std::uint64_t hash) override;
    // TODO(0x7a889d4, Photon.Deterministic.DeterministicNetwork.SendProtocolMessages):
    // online-only; not used by the offline PC build.
    void SendProtocolMessages();
    // TODO(0x7a88bb4, Photon.Deterministic.DeterministicNetwork.HandleProtocolEvent):
    // online-only; not used by the offline PC build.
    void HandleProtocolEvent();
    // TODO(0x7a88c78, Photon.Deterministic.DeterministicNetwork.Poll): online-only; not used
    // by the offline PC build.
    void Poll() override;
    // TODO(0x7a89054, Photon.Deterministic.DeterministicNetwork.ResetInputState): online-only;
    // not used by the offline PC build.
    void ResetInputState(int frame) override;
    // TODO(0x7a890f8, Photon.Deterministic.DeterministicNetwork.SendLocalInputs): online-only;
    // not used by the offline PC build.
    void SendLocalInputs() override;
    // TODO(0x7a89494, Photon.Deterministic.DeterministicNetwork.QueueLocalInput): online-only;
    // not used by the offline PC build.
    void QueueLocalInput(const DeterministicFrameInputTemp& input) override;
    // TODO(0x7a896e0, Photon.Deterministic.DeterministicNetwork.OnEventDataReceived):
    // online-only; not used by the offline PC build.
    void OnEventDataReceived(std::uint8_t eventCode, const ByteArray& data, int dataLength,
                             void* eventObject);
    // TODO(0x7a88e30, Photon.Deterministic.DeterministicNetwork.DequeueNetworkRecvQueue):
    // online-only; not used by the offline PC build.
    void DequeueNetworkRecvQueue();
    // TODO(0x7a897b0, Photon.Deterministic.DeterministicNetwork.HandleInputEvent): online-only;
    // not used by the offline PC build.
    void HandleInputEvent();
    // TODO(0x7a899bc, Photon.Deterministic.DeterministicNetwork.Destroy): online-only; not used
    // by the offline PC build.
    void Destroy() override;
    // TODO(0x7a89a68, Photon.Deterministic.DeterministicNetwork.SendProtocolMessage):
    // online-only; not used by the offline PC build.
    void SendProtocolMessage(std::shared_ptr<Protocol::Message> msg) override;
    // TODO(0x7a89ac0, Photon.Deterministic.DeterministicNetwork.GetRpc): online-only; not used
    // by the offline PC build.
    QTuple<ByteArray, bool> GetRpc(int frame, int player) override;
    // TODO(0x7a89acc, Photon.Deterministic.DeterministicNetwork.AddRpc): online-only; not used
    // by the offline PC build.
    void AddRpc(int player, ByteArray data, bool command) override;
};

class DeterministicTickInputDecoder {
  public:
    // TODO(0x7a83760, Photon.Deterministic.DeterministicTickInputDecoder..ctor): online-only;
    // not used by the offline PC build. Throws std::logic_error.
    DeterministicTickInputDecoder(Protocol::Serializer* serializer, DeterministicSession& session,
                                  int finishedTick);
    // TODO(0x7a83908, Photon.Deterministic.DeterministicTickInputDecoder.ResetInputState):
    // online-only; not used by the offline PC build.
    void ResetInputState(int tick);
    // TODO(0x7a839a4, Photon.Deterministic.DeterministicTickInputDecoder.CreateInputMissing):
    // online-only; not used by the offline PC build.
    std::shared_ptr<Protocol::InputMissing> CreateInputMissing(double rtt, int maxRequestFrame);
    // TODO(0x7a841dc, Photon.Deterministic.DeterministicTickInputDecoder.Decode): online-only;
    // not used by the offline PC build.
    DeterministicTickInputEncodeHeader
    Decode(BitStream& stream, std::vector<std::shared_ptr<DeterministicTickInput>>& result);
    // TODO(0x7a84160, Photon.Deterministic.DeterministicTickInputDecoder.GetTime): online-only;
    // not used by the offline PC build.
    double GetTime();
    // TODO(0x7a8463c, Photon.Deterministic.DeterministicTickInputDecoder.UpdateTrackingData):
    // online-only; not used by the offline PC build.
    void UpdateTrackingData(int tick, bool complete, std::uint64_t received);
    // TODO(0x7a84750, Photon.Deterministic.DeterministicTickInputDecoder.CalculateTrackingData):
    // online-only; not used by the offline PC build.
    void CalculateTrackingData();
    // TODO(0x7a83d4c, Photon.Deterministic.DeterministicTickInputDecoder.CheckForCompletedTicks):
    // online-only; not used by the offline PC build.
    void CheckForCompletedTicks();
    // TODO(0x7a83e9c, Photon.Deterministic.DeterministicTickInputDecoder.CheckForMissingInputs):
    // online-only; not used by the offline PC build.
    void CheckForMissingInputs();
};

} // namespace Photon::Deterministic
