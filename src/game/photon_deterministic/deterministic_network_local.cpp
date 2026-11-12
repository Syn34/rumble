#include "deterministic_network_local.h"

#include "deterministic_session.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace Photon::Deterministic {

DeterministicNetworkLocal::DeterministicNetworkLocal(DeterministicSession& session)
    : rttUpdate_(std::make_shared<Protocol::RttUpdate>()), session_(session),
      outDispatcher_(session.CreateNetworkMessageDispatcher()),
      inDispatcher_(std::make_unique<Dispatcher<Protocol::Message>>()) {
    inDispatcher_->Bind<Protocol::Join>([this](Protocol::Join& join) { OnJoin(join); });
}

DeterministicNetworkLocal::~DeterministicNetworkLocal() = default;

int DeterministicNetworkLocal::RoundTripTime() const {
    return 0;
}

NetworkQueue<std::shared_ptr<DeterministicTickInput>>&
DeterministicNetworkLocal::InputReceiveQueue() {
    return inputReceiveQueue_;
}

void DeterministicNetworkLocal::Poll() {
    session_.maxClientRtt_ = rttUpdate_->Rtt;
    while (!outQueue_.empty()) {
        outDispatcher_->DispatchNext(outQueue_);
    }
    while (!inQueue_.empty()) {
        inDispatcher_->DispatchNext(inQueue_);
    }
}

void DeterministicNetworkLocal::ResetInputState(int) {}

void DeterministicNetworkLocal::SendProtocolMessage(std::shared_ptr<Protocol::Message> msg) {
    inQueue_.push_back(std::move(msg));
}

void DeterministicNetworkLocal::Destroy() {}

void DeterministicNetworkLocal::SendLocalInputs() {}

void DeterministicNetworkLocal::QueueLocalInput(const DeterministicFrameInputTemp&) {}

void DeterministicNetworkLocal::SendLocalRtt(int) {}

void DeterministicNetworkLocal::SendLocalChecksum(int, std::uint64_t) {}

void DeterministicNetworkLocal::OnJoin(Protocol::Join& join) {
    auto joined = std::make_shared<Protocol::Joined>();
    // A join that asks to start from a later tick has to wait for that state.
    joined->Status = join.InitialTick != 0 ? Protocol::JoinStatus::WaitForSnapshot
                                           : Protocol::JoinStatus::Confirmed;
    joined->PlayerSlots.resize(static_cast<std::size_t>(std::max(join.PlayerSlots, 0)));
    std::iota(joined->PlayerSlots.begin(), joined->PlayerSlots.end(), 0);
    outQueue_.push_back(joined);

    auto start = std::make_shared<Protocol::SimulationStart>();
    start->Reconnect = true;
    start->SessionConfig = session_.sessionConfigLocal_;
    start->RuntimeConfig = session_.runtimeConfigLocal_;
    start->ServerTime = 0.0;
    // The local config itself is updated to the number of joined players.
    if (!start->SessionConfig) {
        throw std::logic_error("DeterministicNetworkLocal: the session has no local config");
    }
    start->SessionConfig->PlayerCount = join.PlayerSlots;
    outQueue_.push_back(start);
}

} // namespace Photon::Deterministic
