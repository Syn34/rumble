#include "frame_heap.h"

#include <stdexcept>
#include <string>

namespace Quantum::Allocator {

FrameHeap::FrameHeap(Heap* heap, std::shared_ptr<IHeapAllocationTracker> tracker)
    : _heap(heap), _tracker(std::move(tracker)) {
    if (_tracker == nullptr) {
        throw std::invalid_argument("FrameHeap: null tracker");
    }
    _tracker->Reset();
}

std::unique_ptr<IHeapAllocationTracker>
FrameHeap::GetAllocationTrackerForMode(HeapTrackingMode mode) {
    std::unique_ptr<IHeapAllocationTracker> tracker;
    switch (mode) {
    case HeapTrackingMode::Disabled:
        tracker = std::make_unique<DisabledHeapAllocTracker>();
        break;
    case HeapTrackingMode::DetectLeaks:
        tracker = std::make_unique<SimpleHeapAllocTracker>();
        break;
    case HeapTrackingMode::TraceAllocations:
        tracker = std::make_unique<TracedHeapAllocTracker>();
        break;
    default:
        throw std::out_of_range("FrameHeap: unknown heap tracking mode " +
                                std::to_string(static_cast<int>(mode)));
    }
    tracker->Reset();
    return tracker;
}

// ---------------------------------------------------------------------------------------
// Heap::AllocationTracker (it allocates through the frame heap, hence lives here)

Ptr* Heap::AllocationTracker::EnsureBufferCapacity(FrameHeap& heap) {
    if (_trackAllocCount == _trackAllocCapacity) {
        if (_trackAllocPtr.Offset == 0) {
            _trackAllocCapacity = DefaultCapacity;
            _trackAllocPtr = heap.AllocateAndClearPtr(DefaultCapacity * Ptr::SIZE);
        } else {
            Ptr* grown = heap.ExpandArray(heap.Void<Ptr>(_trackAllocPtr), _trackAllocCount,
                                          _trackAllocCount * 2);
            _trackAllocCapacity = static_cast<std::uint16_t>(_trackAllocCapacity * 2);
            _trackAllocPtr = heap.Void(grown);
        }
    }
    return heap.Void<Ptr>(_trackAllocPtr);
}

void Heap::AllocationTracker::TrackAllocation(FrameHeap& heap, Ptr ptr) {
    Ptr* entries = EnsureBufferCapacity(heap);
    entries[_trackAllocCount++] = ptr;
}

void Heap::AllocationTracker::FreeAllTracked(FrameHeap& heap) {
    if (_trackAllocCount == 0) {
        return;
    }
    const Ptr* entries = heap.Void<Ptr>(_trackAllocPtr);
    for (int i = 0; i < _trackAllocCount; ++i) {
        heap.Free(entries[i]);
    }
    _trackAllocCount = 0;
}

} // namespace Quantum::Allocator
