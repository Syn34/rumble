// local_input_provider.h - Photon.Deterministic.LocalInputProvider and NetworkInputProvider.
//
// Both poll the game for the local players' input. The local provider (offline matches)
// marks that input as confirmed straight away and also queues the players' rpcs; online,
// local input stays unconfirmed until the server echoes it back.
#pragma once

#include "deterministic_frame.h"
#include "deterministic_interfaces.h"

#include <deque>
#include <unordered_map>

namespace Photon::Deterministic {

class LocalInputProvider final : public IDeterministicInputProvider,
                                 public IDeterministicRpcProvider {
  public:
    // 0x7a80958
    explicit LocalInputProvider(IDeterministicGame* game);

    // 0x7a84778 - local input is always available.
    bool CanSimulate(int frame) override;
    // 0x7a84780 - the game's input for the player, flagged as confirmed.
    DeterministicFrameInputTemp GetInput(int frame, int player) override;
    // 0x7a848dc - pops the player's oldest queued rpc (null, false when none).
    QTuple<ByteArray, bool> GetRpc(int frame, int player) override;
    // 0x7a84a44 - queues an rpc. Plain player data (not a command) gets four zero bytes
    // appended.
    void AddRpc(int player, ByteArray data, bool command) override;
    // 0x7a84960 - the player's queue, created on first use.
    std::deque<QTuple<ByteArray, bool>>& GetRpcQueue(int player);

  private:
    IDeterministicGame* game_;
    std::unordered_map<int, std::deque<QTuple<ByteArray, bool>>> rpcQueues_;
};

class NetworkInputProvider final : public IDeterministicInputProvider {
  public:
    // 0x7a806f0
    explicit NetworkInputProvider(IDeterministicGame* game) : game_(game) {}

    // 0x7a84d34
    bool CanSimulate(int frame) override;
    // 0x7a84d3c - the game's input for the player, as the game returned it.
    DeterministicFrameInputTemp GetInput(int frame, int player) override;

  private:
    IDeterministicGame* game_;
};

} // namespace Photon::Deterministic
