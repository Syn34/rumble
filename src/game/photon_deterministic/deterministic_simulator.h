// deterministic_simulator.h - Photon.Deterministic.DeterministicSimulator: the rollback core.
//
// The simulator keeps a sorted list of per-tick input frames, starting with the newest tick
// whose input is fully confirmed ("verified"). Each update it
//   1. advances the verified state through every following tick whose input is now
//      complete (SimulateVerified), computing checksums on the configured interval, then
//   2. rolls the predicted state back to the verified state if anything new was verified
//      and re-simulates forward to the newest tick that has local input, filling missing
//      remote input by repeating each player's last confirmed repeatable input.
// Besides the verified and predicted states it keeps the previous predicted state (for
// interpolating between the last two predicted ticks) and the predicted state as it was at
// the end of the previous update. In lockstep mode prediction is disabled and the
// predicted states alias the verified one.
//
// Time is accumulated in seconds; every 1/UpdateFPS seconds a new input tick may begin
// (StepFrameNumber).
#pragma once

#include "deterministic_frame.h"
#include "deterministic_types.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace Photon::Deterministic {

class DeterministicCommandSerializer;
class DeterministicSession;
struct DeterministicTickChecksumError;

class DeterministicSimulator {
  public:
    // InputFrame (nested): the input of every player for one tick, with masks of which
    // players' input is confirmed (Verified) or locally predicted (Predicted).
    class InputFrame {
      public:
        InputFrame* Next = nullptr;
        int Number = 0;
        std::uint64_t Verified = 0;
        std::uint64_t Predicted = 0;
        std::unique_ptr<DeterministicFrameInput> Input;

        // 0x7a7c578
        InputFrame(int playerCount, int inputSize, Native::Allocator* allocator);
        // 0x7a7bba4
        void DeserializeCommands(DeterministicCommandSerializer& serializer);
        // 0x7a7c9ec
        std::string ToString() const;
    };

    // 0x7a7a44c
    explicit DeterministicSimulator(DeterministicSession& session);
    ~DeterministicSimulator();

    DeterministicSimulator(const DeterministicSimulator&) = delete;
    DeterministicSimulator& operator=(const DeterministicSimulator&) = delete;

    // 0x7a7a224 / 0x7a7a22c - a paused clock never starts new ticks.
    bool Paused() const { return clockPaused_; }
    void SetPaused(bool value) { clockPaused_ = value; }
    // 0x7a7a234 - whole ticks' worth of accumulated time.
    int FramesRemaining() const;
    // 0x7a7a29c - the newest tick that local input has been produced for.
    int FrameCounter() const { return inputFrameCounter_; }
    // 0x7a7a2a4 - ticks predicted during the last Simulate.
    int FramesPredicted() const { return predictedFramesLastUpdate_; }
    // 0x7a7a2ac - seconds not yet turned into ticks.
    double Accumulated() const { return timeAccumulated_; }
    // 0x7a7a26c - seconds per tick (1 / UpdateFPS).
    double DeltaTime() const;
    // 0x7a7a2b4 / 0x7a7a2bc
    bool IsStalling() const { return isStalling_; }
    void SetIsStalling(bool value) { isStalling_ = value; }
    // 0x7a7a2c4
    const std::shared_ptr<DeterministicFrame>& FramePredicted() const { return statePredicted_; }
    // 0x7a7a2cc
    const std::shared_ptr<DeterministicFrame>& FramePredictedPrevious() const {
        return statePredictedPrevious_;
    }
    // 0x7a7a2d4
    const std::shared_ptr<DeterministicFrame>& FramePreviousUpdatePredicted() const {
        return statePreviousUpdatePredicted_;
    }
    // 0x7a7a444
    const std::shared_ptr<DeterministicFrame>& FrameVerified() const { return stateVerified_; }
    // 0x7a7a2dc - forwarded to the game.
    std::shared_ptr<DeterministicFrame> GetVerifiedFrame(int tick);
    // 0x7a7a390 - forwarded to the game.
    ByteArray GetExtraErrorFrameDumpData(DeterministicFrame& frame);

    // 0x7a7a484 - whether input for `frame` is fully confirmed.
    bool IsFrameVerified(int frame) const;
    // 0x7a7a4a4 - adds `dt` seconds scaled by the clock's delta scale.
    void AccumulateTime(double dt);
    // 0x7a7a4b8 - adds `dt` seconds unscaled (clock correction).
    void AdjustClock(double dt);
    // 0x7a7a4c8 - starts the next tick if a whole tick of time has accumulated, the clock
    // runs and the simulator is not stalling past the verified range. `frame` receives the
    // new tick number, or -1.
    bool StepFrameNumber(int& frame);
    // 0x7a7a564 - takes `count` ticks back, returning their time to the accumulator.
    // Does nothing (and returns false) if fewer than `count` ticks were started.
    bool StepFrameNumberDown(int count);
    // 0x7a7a5c0 - a new game frame with initialised input storage.
    std::shared_ptr<DeterministicFrame>
    CreateStateFrame(const std::shared_ptr<IDisposable>& context, int number);
    // 0x7a7a7ec - records one player's input for a tick newer than the verified state.
    // Confirmed input is stored once; predicted input never overrides anything.
    void InsertInput(const DeterministicFrameInputTemp& input);
    // 0x7a7a9d8 - counts how many ticks after the verified state already have complete
    // input (up to `maxVerifiedTicks`) and decides whether the simulator must stall.
    void CheckIsStalling(int maxVerifiedTicks);
    // 0x7a7ab0c - snapshots `fromState` (default: the predicted state) as the previous
    // update's predicted state, when interpolation is enabled.
    void CopyPreviousUpdatePredicted(const std::shared_ptr<DeterministicFrame>& fromState);
    // 0x7a7abb8 - one simulator update (verify, then roll back and predict).
    void Simulate(int maxVerifiedTicks);
    // 0x7a7b054 - aligns all states with the initial verified state once the game started.
    void OnGameStart();
    // 0x7a7b27c - creates the states and the initial input frame. The first verified tick is
    // RollbackWindow - 1.
    void Initialize(const std::shared_ptr<IDisposable>& context);
    // 0x7a7b6cc - restarts from a copy of `frame`.
    void Reset(const DeterministicFrame& frame, bool resetInputFrame);
    // 0x7a7b89c - restarts from serialised frame data, optionally renumbering the frame.
    void Reset(std::span<const std::uint8_t> frameData, std::optional<int> frameNumber,
               bool resetInputFrame);
    // 0x7a7b820 - realigns predicted states and the input list with the verified state.
    void ResetFrames(bool resetInputFrame);
    // 0x7a7af70 - runs one tick of `f`'s input on `state`.
    void Simulate(InputFrame& f, DeterministicFrame& state, bool predicted);
    // 0x7a7b950 - fills missing remote input of `f` by repeating each player's latest
    // confirmed repeatable input within InputRepeatMaxDistance ticks.
    void PredictInput(InputFrame& f);
    // 0x7a7b5d4 - drops all input frames and starts a new list at `frame`, whose input
    // counts as complete. With `resetInputFrame` the local tick counter restarts there too
    // (and, offline, the input-delay ticks after it are pre-filled with empty input).
    void ResetInputFrameHead(int frame, bool resetInputFrame);
    // 0x7a7ad24 - advances the verified state by up to `maxTicks` complete ticks.
    bool SimulateVerified(int maxTicks);
    // 0x7a7bbb8
    void ApplyInputToStateFrame(InputFrame& inputFrame, DeterministicFrame& state, bool predicted);
    // 0x7a7b700 - makes `target` a verified copy of `source` with cleared input.
    void ResetStateFrame(DeterministicFrame& target, const DeterministicFrame& source);
    // 0x7a7a8a8 - the input frame of a tick, created and linked in order if needed.
    InputFrame& GetInputFrame(int frame);
    // 0x7a7c124 - a recycled or new input frame numbered `frame` (not yet linked).
    std::unique_ptr<InputFrame> AcquireInputFrame(int frame);
    // 0x7a7c290 - unlinks a frame from the lookup and recycles it.
    void ReleaseInputFrame(InputFrame& f);
    // 0x7a7beac - frees every input frame (linked, looked up and pooled).
    void DestroyAllInputFrames();
    // 0x7a7c608 - frees all frames and input memory.
    void Destroy();
    // 0x7a7c738 - reports a server-detected desync to the game with the verified state.
    void ChecksumError(const DeterministicTickChecksumError& error);

  private:
    DeterministicSession& session_;
    std::shared_ptr<DeterministicFrame> stateVerified_;
    std::shared_ptr<DeterministicFrame> statePredicted_;
    std::shared_ptr<DeterministicFrame> statePredictedPrevious_;
    std::shared_ptr<DeterministicFrame> statePreviousUpdatePredicted_;
    std::uint64_t playerMask_ = 0;
    std::uint64_t playerMaskLocal_ = 0;
    std::uint64_t playerMaskRemote_ = 0;
    InputFrame* inputFrameHead_ = nullptr;
    int inputFrameCounter_ = 0;
    int predictedFramesLastUpdate_ = 0;
    int minFramesToVerify_ = 0;
    bool isStalling_ = false;
    // Every live input frame is owned either by the lookup or by the pool.
    std::vector<std::unique_ptr<InputFrame>> inputFramePool_;
    std::unordered_map<int, std::unique_ptr<InputFrame>> inputFrameLookup_;
    double deltaScale_ = 1.0;
    double timeAccumulated_ = 0.0;
    bool clockPaused_ = false;
    std::vector<int> predictPlayers_;
    // Allocated and cleared like the original, which no longer reads them.
    std::vector<ByteArray> predictInput_;
    std::vector<DeterministicInputFlags> predictFlags_;
};

} // namespace Photon::Deterministic
