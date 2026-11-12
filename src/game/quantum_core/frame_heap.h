// frame_heap.h - Quantum.Allocator.IFrameHeap / FrameHeap: the heap interface that
// simulation collections allocate through.
//
// A FrameHeap pairs a frame's Heap with the allocation tracker selected by the
// simulation's HeapTrackingMode. Collections (QList, QDictionary, ...) only need the six
// operations of IFrameHeap; their static helpers are templates over any type providing
// them (FrameHeapLike), which both IFrameHeap implementations and Heap itself satisfy.
#pragma once

#include "quantum_core/heap.h"
#include "quantum_core/ptr.h"

#include <concepts>
#include <memory>

namespace Quantum::Allocator {

class IFrameHeap {
  public:
    virtual ~IFrameHeap() = default;

    virtual void* AllocateAndClear(int size) = 0;
    virtual Ptr AllocateAndClearPtr(int size) = 0;
    virtual void Free(void* ptr) = 0;
    virtual void Free(Ptr ptr) = 0;
    virtual Ptr Void(const void* ptr) const = 0;
    virtual void* Void(Ptr p) const = 0;

    // Typed form of Void(Ptr) (generic interface method; no body of its own).
    template <class T> T* Void(Ptr p) const { return static_cast<T*>(Void(p)); }
};

// Anything collections can allocate through.
template <class H>
concept FrameHeapLike = requires(H& heap, Ptr p, void* block, int size) {
    { heap.AllocateAndClear(size) } -> std::convertible_to<void*>;
    { heap.AllocateAndClearPtr(size) } -> std::same_as<Ptr>;
    heap.Free(block);
    heap.Free(p);
    { heap.Void(p) } -> std::convertible_to<void*>;
    { heap.Void(static_cast<const void*>(block)) } -> std::same_as<Ptr>;
};

class FrameHeap final : public IFrameHeap {
  public:
    // 0x7d26210 - Takes ownership of the tracker and resets it.
    FrameHeap(Heap* heap, std::unique_ptr<IHeapAllocationTracker> tracker);

    // 0x7d26208
    IHeapAllocationTracker* get_Tracker() const { return _tracker.get(); }
    Heap* GetHeap() const { return _heap; }

    // 0x7d262e0
    void* AllocateAndClear(int size) override { return _heap->AllocateAndClear(size, *_tracker); }
    // 0x7d26338
    Ptr AllocateAndClearPtr(int size) override {
        return _heap->AllocateAndClearPtr(size, *_tracker);
    }
    // 0x4f0116c (instance for Ptr)
    template <class T> T* ExpandArray(T* buffer, int currentSize, int newSize) {
        return _heap->ExpandArray(buffer, currentSize, newSize, *_tracker);
    }
    // 0x7d263e8
    void Free(void* ptr) override { _heap->Free(ptr, *_tracker); }
    // 0x7d26390
    void Free(Ptr ptr) override { _heap->Free(ptr, *_tracker); }
    // 0x7d26440
    Ptr Void(const void* ptr) const override { return _heap->Void(ptr); }
    // 0x7d26450
    void* Void(Ptr p) const override { return _heap->Void(p); }
    // 0x4f0129c (instance for QHashCollection)
    template <class T> T* Void(Ptr p) const { return _heap->Void<T>(p); }

    // 0x7d26460 - A fresh tracker for the mode (Disabled, Simple or Traced), already reset.
    // Unknown modes throw std::out_of_range.
    static std::unique_ptr<IHeapAllocationTracker>
    GetAllocationTrackerForMode(HeapTrackingMode mode);

  private:
    Heap* _heap;
    std::unique_ptr<IHeapAllocationTracker> _tracker;
};

} // namespace Quantum::Allocator
