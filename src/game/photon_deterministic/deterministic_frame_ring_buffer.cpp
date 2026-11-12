#include "deterministic_frame_ring_buffer.h"

#include "deterministic_frame.h"
#include "deterministic_interfaces.h"

#include <cstdlib>
#include <stdexcept>

namespace Photon::Deterministic {

DeterministicFrameRingBuffer::DeterministicFrameRingBuffer(int capacity) {
    if (capacity <= 0) {
        throw std::invalid_argument("DeterministicFrameRingBuffer: capacity must be positive");
    }
    capacity_ = capacity;
    data_.resize(static_cast<std::size_t>(capacity));
}

DeterministicFrameRingBuffer::~DeterministicFrameRingBuffer() = default;

void DeterministicFrameRingBuffer::Clear() {
    for (auto& frame : data_) {
        if (frame) {
            frame->Free();
        }
        frame.reset();
    }
    tail_ = 0;
    count_ = 0;
}

std::shared_ptr<DeterministicFrame>
DeterministicFrameRingBuffer::Find(int frameNumber,
                                   DeterministicFrameSnapshotBufferFindMode findMode) const {
    std::shared_ptr<DeterministicFrame> best;
    for (const auto& frame : data_) {
        if (!frame) {
            continue;
        }
        if (frame->Number == frameNumber) {
            return frame;
        }
        switch (findMode) {
        case DeterministicFrameSnapshotBufferFindMode::Closest:
            if (!best ||
                std::abs(frame->Number - frameNumber) < std::abs(best->Number - frameNumber)) {
                best = frame;
            }
            break;
        case DeterministicFrameSnapshotBufferFindMode::ClosestLessThanOrEqual:
            if (frame->Number < frameNumber && (!best || frame->Number > best->Number)) {
                best = frame;
            }
            break;
        case DeterministicFrameSnapshotBufferFindMode::Equal:
            break;
        }
    }
    return best;
}

template <typename CreateFrame>
void DeterministicFrameRingBuffer::PushBackImpl(const DeterministicFrame& frame,
                                                CreateFrame&& create) {
    auto& slot = data_.at(static_cast<std::size_t>(tail_));
    if (!slot) {
        slot = create();
        if (!slot) {
            throw std::logic_error("DeterministicFrameRingBuffer: frame factory returned null");
        }
    }
    slot->Copy(frame);
    slot->Number = frame.Number;
    tail_ = (tail_ + 1) % capacity_;
    if (count_ != capacity_) {
        ++count_;
    }
}

void DeterministicFrameRingBuffer::PushBack(const DeterministicFrame& frame,
                                            IDeterministicGame& game,
                                            const std::shared_ptr<IDisposable>& context) {
    PushBackImpl(frame, [&] { return game.CreateFrame(context); });
}

void DeterministicFrameRingBuffer::PushBack(
    const DeterministicFrame& frame,
    const std::function<std::shared_ptr<DeterministicFrame>()>& createFrame) {
    PushBackImpl(frame, [&] { return createFrame(); });
}

std::shared_ptr<DeterministicFrame> DeterministicFrameRingBuffer::PeekBack(int offset) const {
    if (count_ == 0) {
        throw std::logic_error("DeterministicFrameRingBuffer is empty");
    }
    if (offset < 0 || offset >= count_) {
        throw std::out_of_range("DeterministicFrameRingBuffer::PeekBack: offset out of range");
    }
    int index = tail_ - 1 - offset;
    if (index < 0) {
        index += capacity_;
    }
    return data_.at(static_cast<std::size_t>(index));
}

int DeterministicFrameRingBuffer::Increment(int i, int capacity, int count) {
    return (i + count) % capacity;
}

bool DeterministicFrameRingBuffer::TryGetCommonSamplingPattern(int windowA, int samplingA,
                                                               int windowB, int samplingB,
                                                               int& commonWindow,
                                                               int& commonSampling) {
    if (samplingA > 0 && samplingB > 0 && windowA >= 1 && windowB > 0) {
        // Both are positive here, so the unsigned comparisons of the original are plain ones.
        if (windowB >= windowA && samplingA % samplingB == 0) {
            commonWindow = windowB;
            commonSampling = samplingB;
            return true;
        }
        if (windowA >= windowB && samplingB % samplingA == 0) {
            commonWindow = windowA;
            commonSampling = samplingA;
            return true;
        }
    }
    commonWindow = 0;
    commonSampling = 0;
    return false;
}

int DeterministicFrameRingBuffer::GetSize(int window, int samplingRate) {
    return samplingRate >= 1 ? window / samplingRate + 1 : 0;
}

} // namespace Photon::Deterministic
