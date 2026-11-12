#include "local_input_provider.h"

#include "byte_utils.h"

#include <array>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

IDeterministicGame& Require(IDeterministicGame* game) {
    if (game == nullptr) {
        throw std::logic_error("input provider without a game");
    }
    return *game;
}

} // namespace

LocalInputProvider::LocalInputProvider(IDeterministicGame* game) : game_(game) {}

bool LocalInputProvider::CanSimulate(int) {
    return true;
}

DeterministicFrameInputTemp LocalInputProvider::GetInput(int frame, int player) {
    DeterministicFrameInputTemp input = Require(game_).OnLocalInput(frame, player);
    input.IsVerified = true;
    return input;
}

QTuple<ByteArray, bool> LocalInputProvider::GetRpc(int, int player) {
    auto& queue = GetRpcQueue(player);
    if (queue.empty()) {
        return {};
    }
    QTuple<ByteArray, bool> rpc = std::move(queue.front());
    queue.pop_front();
    return rpc;
}

void LocalInputProvider::AddRpc(int player, ByteArray data, bool command) {
    if (!command) {
        if (!data) {
            throw std::invalid_argument("LocalInputProvider::AddRpc: null player data");
        }
        const std::array<std::vector<std::uint8_t>, 2> blocks{*data, std::vector<std::uint8_t>(4)};
        data = MakeByteArray(ByteUtils::MergeByteBlocks(blocks));
    }
    GetRpcQueue(player).push_back(MakeQTuple(std::move(data), command));
}

std::deque<QTuple<ByteArray, bool>>& LocalInputProvider::GetRpcQueue(int player) {
    return rpcQueues_[player];
}

bool NetworkInputProvider::CanSimulate(int) {
    return true;
}

DeterministicFrameInputTemp NetworkInputProvider::GetInput(int frame, int player) {
    return Require(game_).OnLocalInput(frame, player);
}

} // namespace Photon::Deterministic
