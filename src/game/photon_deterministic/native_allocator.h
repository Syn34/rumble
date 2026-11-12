// native_allocator.h - Photon.Deterministic.Native: unmanaged memory for the simulation.
//
// The simulation keeps almost all of its state in unmanaged memory obtained from an
// Allocator. The original declares Allocator as an abstract class whose platform
// subclass (supplied by the engine side) provides the raw Alloc/Free/Clear; the
// convenience operations (allocate-and-zero, grow a buffer) are written once on top of
// those. SystemAllocator below is this project's PC implementation of the platform part.
//
// Also here: the two alignment helpers of the Native class that the frame heap and the
// collections use to lay out their records.
#pragma once

#include <cstddef>
#include <cstring>

namespace Photon::Deterministic::Native {

// 0x7a783ac - Rounds `value` up to a multiple of `alignment`. Only the power-of-two
// alignments 1..64 are accepted; anything else throws (the original raises an
// exception naming the bad alignment).
int RoundUpToAlignment(int value, int alignment);

// 0x7a784b4 - Alignment used for one element of an unmanaged array: 8 for element sizes
// that are a positive multiple of 8 (so 64-bit fields stay aligned), otherwise 4.
constexpr int GetAlignmentForArrayElement(int elementSize) {
    return (elementSize % 8 == 0 && elementSize > 0) ? 8 : 4;
}

class Allocator {
  public:
    virtual ~Allocator() = default;

    // 0x7a784d4 - Nothing to release in the base class.
    virtual void Dispose() {}

    virtual void Free(void* ptr) = 0;
    virtual void* Alloc(int count) = 0;
    virtual void* Alloc(int count, int alignment) = 0;
    virtual void Clear(void* dest, int count) = 0;

    // 0x7a784d8 - Alloc followed by Clear of the same byte count.
    void* AllocAndClear(int count);
    // 0x7a78520 - Aligned variant.
    void* AllocAndClear(int count, int alignment);

    // 0x52f184c (instance for UnsafeHashMap) - one zeroed T.
    template <class T> T* AllocAndClear() {
        return static_cast<T*>(AllocAndClear(static_cast<int>(sizeof(T))));
    }

    // 0x7a78568 - Moves a buffer into a new, zeroed block of `newSize` bytes: the first
    // `currentSize` bytes are copied over and the old block is freed.
    void* Expand(void* buffer, int currentSize, int newSize);

    // 0x52f1948 (instance for Physics2D.Bucket) - Expand for arrays of T, sizes given in
    // elements.
    template <class T> T* ExpandArray(T* buffer, int currentSize, int newSize) {
        const int size = static_cast<int>(sizeof(T));
        return static_cast<T*>(Expand(buffer, currentSize * size, newSize * size));
    }
};

// PC implementation of the platform allocator (new code, no original counterpart):
// aligned operator new/delete. Blocks remember their alignment so Free can release them.
class SystemAllocator final : public Allocator {
  public:
    void Free(void* ptr) override;
    void* Alloc(int count) override;
    void* Alloc(int count, int alignment) override;
    void Clear(void* dest, int count) override;
};

} // namespace Photon::Deterministic::Native
