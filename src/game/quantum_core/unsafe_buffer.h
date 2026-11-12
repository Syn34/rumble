// unsafe_buffer.h - The two UnsafeBuffer types of quantum.core: arrays in memory from a
// native Allocator (not the frame heap), used for engine-side scratch data such as the
// physics solver's manifolds and constraints.
//
//  * Quantum.Collections.UnsafeBuffer: a fixed-length array that either owns its memory
//    ("dynamic") or wraps memory owned by someone else ("fixed").
//  * Quantum.PhysicsCommon.UnsafeBuffer: an append-only array that doubles when full and is
//    reset (not freed) between uses. PhysicsCommon is a class in the original; it is a
//    namespace here so its other members can be added separately.
#pragma once

#include "photon_deterministic/native.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Quantum::Collections {

struct UnsafeBuffer {
    void* Ptr = nullptr;
    int Length = 0;
    int Stride = 0;
    int Dynamic = 0; // 1 if Ptr was allocated by InitDynamic and must be freed

    // 0x7d216f4 - Frees a dynamic buffer and zeroes the struct; std::logic_error for a
    // fixed one.
    static void Free(Photon::Deterministic::Native::Allocator& allocator, UnsafeBuffer* buffer);
    // 0x7d21778 - Zeroes Length * Stride bytes.
    static void Clear(UnsafeBuffer* buffer);
    // 0x7d217e4 - Wraps memory owned elsewhere.
    static void InitFixed(UnsafeBuffer* buffer, void* ptr, int length, int stride);
    // 0x7d21800 - Allocates zeroed memory aligned for the element size.
    static void InitDynamic(Photon::Deterministic::Native::Allocator& allocator,
                            UnsafeBuffer* buffer, int length, int stride);
    // 0x7d21868 - Copies `count` elements; the source stride is used for both buffers.
    static void Copy(UnsafeBuffer source, int sourceIndex, UnsafeBuffer destination,
                     int destinationIndex, int count);
    // 0x7d21904
    static void* Element(void* bufferPtr, int index, int stride) {
        return static_cast<std::byte*>(bufferPtr) + static_cast<std::ptrdiff_t>(index) * stride;
    }
};

} // namespace Quantum::Collections

namespace Quantum::PhysicsCommon {

struct Utils {
    // 0x7d2fa80 - Smallest power of two >= value (for value >= 1).
    static int GetNextPowerOfTwo(int value);
};

class UnsafeBuffer {
  public:
    // Walks the entries forwards or backwards (Quantum.PhysicsCommon.UnsafeBuffer.Iterator`1);
    // defined below.
    template <class T> struct Iterator;

    UnsafeBuffer() = default;
    // 0x7d2f654
    explicit UnsafeBuffer(int stride) : _stride(stride) {}

    // 0x7d2f644 / 0x7d2f64c
    int get_Count() const { return _count; }
    void set_Count(int value) { _count = value; }

    // 0x7d2f664 - A buffer of `stride`-byte entries with room for at least `capacity`.
    static UnsafeBuffer InitBuffer(int stride, Photon::Deterministic::Native::Allocator& allocator,
                                   int capacity);
    // 0x52f2964 (instance for Physics2D.ConstraintData)
    template <class T>
    static UnsafeBuffer InitBuffer(Photon::Deterministic::Native::Allocator& allocator,
                                   int capacity) {
        return InitBuffer(static_cast<int>(sizeof(T)), allocator, capacity);
    }

    // 0x52f2c0c (instance for nint)
    template <class T> bool IsInitialized() const { return _buffer != nullptr; }
    // 0x52f2794 (nint)
    template <class T> T* GetBuffer() const { return static_cast<T*>(_buffer); }
    // 0x52f27ac (nint)
    template <class T> T* GetBuffer(int& count) const {
        count = _count;
        return static_cast<T*>(_buffer);
    }

    // 0x7d2f73c - Appends one zeroed entry and returns it; `index` receives its position.
    // A full buffer grows to max(8, 2 * count) entries (the new memory is not zeroed
    // beyond the copied entries; each entry is zeroed when handed out).
    void* GetNextEntry(Photon::Deterministic::Native::Allocator& allocator, int& index);
    // 0x52f2954 (instance for Physics3D.ManifoldData3D)
    template <class T>
    T* GetNextEntry(Photon::Deterministic::Native::Allocator& allocator, int& index) {
        return static_cast<T*>(GetNextEntry(allocator, index));
    }
    // 0x52f28e4 (nint)
    template <class T> T* GetNextEntry(Photon::Deterministic::Native::Allocator& allocator) {
        int index = 0;
        return static_cast<T*>(GetNextEntry(allocator, index));
    }

    // 0x52f27fc (nint)
    template <class T> Iterator<T> GetIterator(bool reverseOrder) const {
        return Iterator<T>(*this, reverseOrder);
    }

    // 0x7d2f888 - Forgets the entries.
    void Reset() { _count = 0; }
    // 0x7d2f890 - Zeroes the used entries (or the whole capacity) and forgets them.
    void ResetAndClear(bool clearUsedCountOnly);
    // 0x7d2f6a0 - Forgets the entries and makes room for `capacity`; a larger buffer
    // (rounded up to a power of two, zeroed) replaces the old one. Returns the storage.
    void* ResetAndEnsureCapacity(Photon::Deterministic::Native::Allocator& allocator, int capacity);
    // 0x52f2c70 (nint)
    template <class T>
    T* ResetAndEnsureCapacity(Photon::Deterministic::Native::Allocator& allocator, int capacity) {
        return static_cast<T*>(ResetAndEnsureCapacity(allocator, capacity));
    }
    // 0x7d2f924 - Frees the storage and zeroes the buffer (including its stride).
    void Dispose(Photon::Deterministic::Native::Allocator& allocator);

    int _count = 0;
    int _capacity = 0;
    int _stride = 0;
    void* _buffer = nullptr;
};

// Walks the entries forwards or backwards. Entries are addressed with sizeof(T), like the
// original.
template <class T> struct UnsafeBuffer::Iterator {
    UnsafeBuffer _ub;
    int _curIndex = -1;
    std::int8_t _inc = 1;

    // 0x5bce4cc (instance for nint)
    Iterator(const UnsafeBuffer& ub, bool reverseOrder)
        : _ub(ub), _curIndex(reverseOrder ? ub._count : -1),
          _inc(static_cast<std::int8_t>(reverseOrder ? -1 : 1)) {}

    // 0x5bce504
    bool MoveNext() {
        _curIndex += _inc;
        return _curIndex >= 0 && _curIndex < _ub._count;
    }
    // 0x5bce530
    void Reset() { _curIndex = _inc == 1 ? -1 : _ub._count; }
    // 0x5bce550
    T* get_CurrentUnsafe() const { return static_cast<T*>(_ub._buffer) + _curIndex; }
    // 0x5bce560; 0x5bce594 is the boxed IEnumerator.Current.
    T get_Current() const { return *get_CurrentUnsafe(); }
    // 0x5bce61c
    void Dispose() {}
};

} // namespace Quantum::PhysicsCommon
