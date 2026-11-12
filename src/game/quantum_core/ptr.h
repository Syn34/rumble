// ptr.h - Quantum.Ptr, a reference into a frame heap.
//
// Everything the simulation allocates dynamically (list storage, dictionaries, buffers
// owned by components) lives in the frame's Heap. Instead of raw pointers the stored
// state holds a Ptr: the byte offset of the block from the start of the heap's memory.
// Offsets survive copying a heap to another frame (rollback/prediction copies frames
// wholesale), which raw pointers would not. Offset 0 is the null reference; the first
// pages of every heap hold allocator bookkeeping, so no block ever lives there.
#pragma once

#include <cstddef>
#include <functional>
#include <string>

namespace Quantum {

struct Ptr {
    static constexpr int SIZE = 4;

    int Offset = 0;

    // 0x7ce03bc
    static constexpr Ptr Null() { return Ptr{}; }

    // 0x7ce03c4 (Equals(Ptr)), 0x7ce04cc (==), 0x7ce04d8 (!=). Equals(object) at 0x7ce03d4
    // is the boxed form of the same comparison.
    friend constexpr bool operator==(Ptr, Ptr) = default;
    constexpr bool Equals(Ptr other) const { return Offset == other.Offset; }

    // 0x7ce044c
    constexpr int GetHashCode() const { return Offset; }

    // 0x7ce0454 - The offset in angle brackets, e.g. "<1024>".
    std::string ToString() const { return "<" + std::to_string(Offset) + ">"; }
};

static_assert(sizeof(Ptr) == Ptr::SIZE);

} // namespace Quantum

template <> struct std::hash<Quantum::Ptr> {
    std::size_t operator()(Quantum::Ptr p) const noexcept { return std::hash<int>{}(p.Offset); }
};
