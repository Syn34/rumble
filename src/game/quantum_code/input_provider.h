// input_provider.h - Quantum.InputProvider (quantum.code.dll): a store of confirmed
// player input by frame, used to record a match and to replay it.
//
// One DeterministicTickInputSet per frame from the start frame on, each with one input
// per player. The store grows as confirmed input arrives (by a fixed step, or doubling).
#pragma once

#include "photon_deterministic/deterministic_frame.h"
#include "photon_deterministic/deterministic_interfaces.h"
#include "photon_deterministic/deterministic_session_config.h"
#include "photon_deterministic/deterministic_tick_input.h"

#include <vector>

namespace Quantum {

class QuantumGame;

// Quantum.InputProviderExtensions: helpers on DeterministicTickInput.
namespace InputProviderExtensions {
// 0x7cafaac - Empties the input (tick, data, rpc, flags of state).
void Clear(Photon::Deterministic::DeterministicTickInput& input);
// 0x7cafaf4 - Copies a confirmed input (the data is cloned).
void Set(Photon::Deterministic::DeterministicTickInput& input,
         const Photon::Deterministic::DeterministicFrameInputTemp& value);
// 0x7cafba0 - True when every player's input of the set has a tick.
bool IsComplete(const Photon::Deterministic::DeterministicTickInputSet& set);
} // namespace InputProviderExtensions

class InputProvider final : public Photon::Deterministic::IDeterministicReplayProvider {
  public:
    using DeterministicFrameInputTemp = Photon::Deterministic::DeterministicFrameInputTemp;
    using DeterministicTickInputSet = Photon::Deterministic::DeterministicTickInputSet;

    // 0x7cab508 - Players from the session config; recording starts at its rollback window
    // (the first frame that takes input).
    InputProvider(const Photon::Deterministic::DeterministicSessionConfig& config, int capacity,
                  int growSize);
    // 0x7caf7cc - Replays a recorded list.
    explicit InputProvider(std::vector<DeterministicTickInputSet> inputList);
    // 0x7cab4a8
    InputProvider(int playerCount, int startFrame, int capacity, int growSize);

    // 0x7caf7ac - One past the last frame the store has room for.
    int get_MaxFrame() const { return _startFrame + static_cast<int>(_inputs.size()); }

    // 0x7caaee0 - Restarts at `startFrame` with every input emptied (storage is kept).
    void Clear(int startFrame);
    // 0x7caf7f8 - Takes over a recorded list; its inputs count as sent.
    void ImportFromList(std::vector<DeterministicTickInputSet> inputList);
    // 0x7caadcc - Stores a confirmed input, growing the store when needed. Input for
    // frames before the start frame is ignored.
    void OnInputConfirmed(QuantumGame* game, const DeterministicFrameInputTemp& input);

    // 0x7cafb60 - Recorded matches carry rpcs in their inputs; nothing to add.
    void AddRpc(int player, Photon::Deterministic::ByteArray data, bool command) override {
        (void)player;
        (void)data;
        (void)command;
    }
    // 0x7cafb64 - Whether every player's input of `frame` is there.
    bool CanSimulate(int frame) override;
    // 0x7cafc04 - The rpc stored with a player's input and whether it is a command.
    Photon::Deterministic::QTuple<Photon::Deterministic::ByteArray, bool> GetRpc(int frame, int player) override;
    // 0x7cafcc8 - The stored input as verified input (empty past the end).
    DeterministicFrameInputTemp GetInput(int frame, int player) override;
    // 0x7cafae8
    int ToIndex(int frame) const { return frame - _startFrame; }

    const std::vector<DeterministicTickInputSet>& Inputs() const { return _inputs; }

  private:
    // 0x7caf8c8 - Grows (or shrinks) the store to `size` frames; new frames get empty
    // inputs for every player.
    void Allocate(int size);

    int _playerCount = 0;
    int _growSize = 0;
    int _startFrame = 0;
    std::vector<DeterministicTickInputSet> _inputs;
};

} // namespace Quantum
