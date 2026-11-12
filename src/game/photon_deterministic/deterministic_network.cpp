#include "deterministic_network.h"

#include <stdexcept>

namespace Photon::Deterministic {

namespace {

[[noreturn]] void OnlineOnly(const char* what) {
    throw std::logic_error(std::string(what) +
                           ": online-only; not implemented in the offline PC build");
}

} // namespace

DeterministicNetwork::DeterministicNetwork(DeterministicSession&, ICommunicator*) {
    OnlineOnly("DeterministicNetwork");
}

int DeterministicNetwork::RoundTripTime() const {
    OnlineOnly("DeterministicNetwork::RoundTripTime");
}
NetworkQueue<std::shared_ptr<DeterministicTickInput>>& DeterministicNetwork::InputReceiveQueue() {
    OnlineOnly("DeterministicNetwork::InputReceiveQueue");
}
void DeterministicNetwork::SendLocalRtt(int) {}
void DeterministicNetwork::SendLocalChecksum(int, std::uint64_t) {}
void DeterministicNetwork::SendProtocolMessages() {}
void DeterministicNetwork::HandleProtocolEvent() {}
void DeterministicNetwork::Poll() {}
void DeterministicNetwork::ResetInputState(int) {}
void DeterministicNetwork::SendLocalInputs() {}
void DeterministicNetwork::QueueLocalInput(const DeterministicFrameInputTemp&) {}
void DeterministicNetwork::OnEventDataReceived(std::uint8_t, const ByteArray&, int, void*) {}
void DeterministicNetwork::DequeueNetworkRecvQueue() {}
void DeterministicNetwork::HandleInputEvent() {}
void DeterministicNetwork::Destroy() {}
void DeterministicNetwork::SendProtocolMessage(std::shared_ptr<Protocol::Message>) {}
QTuple<ByteArray, bool> DeterministicNetwork::GetRpc(int, int) {
    OnlineOnly("DeterministicNetwork::GetRpc");
}
void DeterministicNetwork::AddRpc(int, ByteArray, bool) {}

DeterministicTickInputDecoder::DeterministicTickInputDecoder(Protocol::Serializer*,
                                                             DeterministicSession&, int) {
    OnlineOnly("DeterministicTickInputDecoder");
}
void DeterministicTickInputDecoder::ResetInputState(int) {}
std::shared_ptr<Protocol::InputMissing> DeterministicTickInputDecoder::CreateInputMissing(double,
                                                                                          int) {
    OnlineOnly("DeterministicTickInputDecoder::CreateInputMissing");
}
DeterministicTickInputEncodeHeader
DeterministicTickInputDecoder::Decode(BitStream&,
                                      std::vector<std::shared_ptr<DeterministicTickInput>>&) {
    OnlineOnly("DeterministicTickInputDecoder::Decode");
}
double DeterministicTickInputDecoder::GetTime() {
    OnlineOnly("DeterministicTickInputDecoder::GetTime");
}
void DeterministicTickInputDecoder::UpdateTrackingData(int, bool, std::uint64_t) {}
void DeterministicTickInputDecoder::CalculateTrackingData() {}
void DeterministicTickInputDecoder::CheckForCompletedTicks() {}
void DeterministicTickInputDecoder::CheckForMissingInputs() {}

} // namespace Photon::Deterministic
