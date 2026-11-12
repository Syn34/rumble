// deterministic_network_local.h - Photon.Deterministic.DeterministicNetworkLocal: the network
// layer of offline and replay sessions.
//
// It plays the server's part in memory: messages the session sends are queued and handled
// on the next Poll. The only request it understands is Join, which it answers with Joined
// (granting every requested player slot to this client) and SimulationStart (carrying the
// session's own local configs). Input never travels anywhere, so the input queue stays
// empty and all send methods are no-ops.
#pragma once

#include "deterministic_containers.h"
#include "deterministic_interfaces.h"
#include "protocol.h"

#include <deque>
#include <memory>

namespace Photon::Deterministic {

class DeterministicSession;

class DeterministicNetworkLocal final : public IDeterministicNetwork {
  public:
    // 0x7a80720
    explicit DeterministicNetworkLocal(DeterministicSession& session);
    ~DeterministicNetworkLocal() override;

    // 0x7a89b88 - always 0.
    int RoundTripTime() const override;
    // 0x7a89b90
    NetworkQueue<std::shared_ptr<DeterministicTickInput>>& InputReceiveQueue() override;
    // 0x7a89b98 - publishes the (zero) round trip, then handles all queued messages: first
    // those for the session, then those sent by the session.
    void Poll() override;
    // 0x7a89cac
    void Destroy() override;
    // 0x7a89cb8
    void SendLocalRtt(int rtt) override;
    // 0x7a89cbc
    void SendLocalChecksum(int tick, std::uint64_t checksum) override;
    // 0x7a89cb0
    void SendLocalInputs() override;
    // 0x7a89cb4
    void QueueLocalInput(const DeterministicFrameInputTemp& input) override;
    // 0x7a89c50
    void ResetInputState(int frame) override;
    // 0x7a89c54 - queues a message from the session.
    void SendProtocolMessage(std::shared_ptr<Protocol::Message> msg) override;

    // 0x7a89cc0 - answers a join request.
    void OnJoin(Protocol::Join& join);

  private:
    std::shared_ptr<Protocol::RttUpdate> rttUpdate_;
    DeterministicSession& session_;
    // Delivers "server" messages to the session.
    std::unique_ptr<Dispatcher<Protocol::Message>> outDispatcher_;
    // Handles messages sent by the session.
    std::unique_ptr<Dispatcher<Protocol::Message>> inDispatcher_;
    NetworkQueue<std::shared_ptr<DeterministicTickInput>> inputReceiveQueue_;
    std::deque<std::shared_ptr<Protocol::Message>> inQueue_;
    std::deque<std::shared_ptr<Protocol::Message>> outQueue_;
};

} // namespace Photon::Deterministic
