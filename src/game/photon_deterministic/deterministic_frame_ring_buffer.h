// deterministic_frame_ring_buffer.h - Photon.Deterministic.DeterministicFrameRingBuffer:
// a fixed number of recent frame snapshots (for history lookups such as Quantum's
// "frame N ticks ago"). Frames in the buffer are allocated lazily from the game and
// reused; pushing copies the state of the given frame into the oldest slot.
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace Photon::Deterministic {

class DeterministicFrame;
class IDeterministicGame;
class IDisposable;

// Photon.Deterministic.DeterministicFrameSnapshotBufferFindMode
enum class DeterministicFrameSnapshotBufferFindMode : int {
    Equal = 0,
    ClosestLessThanOrEqual = 1,
    Closest = 2,
};

class DeterministicFrameRingBuffer {
  public:
    // 0x7a8d1fc - throws std::invalid_argument for a capacity below 1.
    explicit DeterministicFrameRingBuffer(int capacity);
    ~DeterministicFrameRingBuffer();

    DeterministicFrameRingBuffer(const DeterministicFrameRingBuffer&) = delete;
    DeterministicFrameRingBuffer& operator=(const DeterministicFrameRingBuffer&) = delete;

    // 0x7a8d2cc
    int Capacity() const { return capacity_; }
    // 0x7a8d2d4 / 0x7a8d2dc
    int Count() const { return count_; }
    void SetCount(int value) { count_ = value; }
    // 0x7a8d2e4 - the slots (null where no frame was created yet).
    const std::vector<std::shared_ptr<DeterministicFrame>>& Data() const { return data_; }

    // 0x7a8d2ec - frees and drops every stored frame.
    void Clear();
    // 0x7a8d38c - IDisposable.Dispose; same as Clear.
    void Dispose() { Clear(); }

    // 0x7a8d390 - an exact match always wins; otherwise, depending on the mode, the
    // closest frame below `frameNumber` or the closest frame overall (first found on ties).
    std::shared_ptr<DeterministicFrame>
    Find(int frameNumber, DeterministicFrameSnapshotBufferFindMode findMode) const;
    // 0x7a8d524 - copies `frame` into the next slot, creating the slot's frame with
    // game.CreateFrame(context) on first use.
    void PushBack(const DeterministicFrame& frame, IDeterministicGame& game,
                  const std::shared_ptr<IDisposable>& context);
    // 0x7a8d6bc - same, creating slot frames with `createFrame`.
    void PushBack(const DeterministicFrame& frame,
                  const std::function<std::shared_ptr<DeterministicFrame>()>& createFrame);
    // 0x7a8d7bc - offset 0 is the newest frame. Throws std::logic_error when empty and
    // std::out_of_range for an offset outside [0, Count).
    std::shared_ptr<DeterministicFrame> PeekBack(int offset) const;

    // 0x7a8d6ac - (i + count) modulo capacity.
    static int Increment(int i, int capacity, int count);
    // 0x7a8d894 - finds a history window/sampling that covers two requests: the larger
    // window wins if its sampling rate divides the other's. False (and zeros) otherwise.
    static bool TryGetCommonSamplingPattern(int windowA, int samplingA, int windowB, int samplingB,
                                            int& commonWindow, int& commonSampling);
    // 0x7a8d90c - frames needed to cover `window` ticks sampled every `samplingRate` ticks.
    static int GetSize(int window, int samplingRate);

  private:
    template <typename CreateFrame>
    void PushBackImpl(const DeterministicFrame& frame, CreateFrame&& create);

    std::vector<std::shared_ptr<DeterministicFrame>> data_;
    int tail_ = 0;
    int capacity_ = 0;
    int count_ = 0;
};

} // namespace Photon::Deterministic
