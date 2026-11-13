// q_buffer.h - Quantum.Collections.QBuffer and QCollectionsUtils: the storage primitive
// under the frame-heap collections.
//
// A QBuffer is a fixed-capacity array in a frame heap: element count, element size and
// the Ptr of the storage. It is itself stored inside heap-resident collection headers
// (QListInternal, QHashCollection), so its layout (12 bytes: Length, Stride, Ptr) is part
// of the frame memory format. The static operations take the heap explicitly; they are
// templates over FrameHeapLike like the generic originals (one instance address cited).
#pragma once

#include "quantum_core/frame_heap.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/ptr.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace Quantum::Collections {

// Typed view of `byteOffset` bytes into a block of frame memory. Heap blocks are 8-byte
// aligned and the collections round their strides to the element alignment, so the
// result is suitably aligned for the record types used here.
template <class T> T* ElementAt(void* base, int byteOffset) {
    static_assert(alignof(T) <= 8, "frame heap blocks are only 8-byte aligned");
    return reinterpret_cast<T*>(static_cast<std::byte*>(base) + byteOffset);
}

// Typed address of a Ptr in `heap`.
template <class T, Allocator::FrameHeapLike THeap> T* Resolve(THeap& heap, Quantum::Ptr p) {
    return static_cast<T*>(heap.Void(p));
}

struct QBuffer {
    int Length = 0;
    int Stride = 0;
    Quantum::Ptr Ptr;

    // 0x510fa20 (instance for object) - Allocates `length` zeroed elements of `stride`
    // bytes.
    template <Allocator::FrameHeapLike THeap>
    static void Init(THeap& heap, QBuffer* buffer, int length, int stride) {
        buffer->Ptr = heap.AllocateAndClearPtr(length * stride);
        buffer->Stride = stride;
        buffer->Length = length;
    }

    // 0x510f224 (object) - Zeroes all elements.
    template <Allocator::FrameHeapLike THeap> static void Clear(THeap& heap, QBuffer* buffer) {
        std::memset(heap.Void(buffer->Ptr), 0,
                    static_cast<std::size_t>(buffer->Length) *
                        static_cast<std::size_t>(buffer->Stride));
    }

    // 0x510f840 (object) - Releases the storage and resets the buffer to empty.
    template <Allocator::FrameHeapLike THeap> static void Free(THeap& heap, QBuffer* buffer) {
        heap.Free(buffer->Ptr);
        buffer->Ptr = Quantum::Ptr::Null();
        buffer->Length = 0;
        buffer->Stride = 0;
    }

    // 0x510f488 (object) - Copies `count` elements between two buffers (the source's
    // stride is used for both).
    template <Allocator::FrameHeapLike THeap>
    static void Copy(THeap& heap, QBuffer source, int sourceIndex, QBuffer destination,
                     int destinationIndex, int count) {
        std::byte* from = static_cast<std::byte*>(heap.Void(source.Ptr));
        std::byte* to = static_cast<std::byte*>(heap.Void(destination.Ptr));
        const int stride = source.Stride;
        if (count > 0) {
            std::memcpy(to + static_cast<std::ptrdiff_t>(stride) * destinationIndex,
                        from + static_cast<std::ptrdiff_t>(stride) * sourceIndex,
                        static_cast<std::size_t>(stride) * static_cast<std::size_t>(count));
        }
    }

    // 0x510fc30 (object) - Moves `count` elements within one buffer (ranges may overlap).
    template <Allocator::FrameHeapLike THeap>
    static void Move(THeap& heap, QBuffer source, int fromIndex, int toIndex, int count) {
        std::byte* base = static_cast<std::byte*>(heap.Void(source.Ptr));
        const int stride = source.Stride;
        if (count > 0) {
            std::memmove(base + static_cast<std::ptrdiff_t>(stride) * toIndex,
                         base + static_cast<std::ptrdiff_t>(stride) * fromIndex,
                         static_cast<std::size_t>(stride) * static_cast<std::size_t>(count));
        }
    }

    // 0x7d1fbf4 - Address of element `index`.
    static void* Element(void* bufferPtr, int index, int stride) {
        return static_cast<std::byte*>(bufferPtr) + static_cast<std::ptrdiff_t>(index) * stride;
    }

    // 0x7d1fba8 - ((17 * 31 + Length) * 31 + Stride) * 31 + Ptr hash.
    int GetHashCode() const;

    // Serialises one element (the original passes a delegate).
    using ItemSerializer = void (*)(void* item, FrameSerializer* serializer);

    // 0x7d1f978 - Length; when reading, Stride is set to `stride`.
    static void SerializeMeta(QBuffer* buffer, int stride, FrameSerializer* serializer);
    // 0x7d1faa8 - Meta, then the storage Ptr through the pointer table. True when the
    // caller has to serialise the storage contents (see FrameSerializer::PointerSerialize).
    static bool Serialize(QBuffer* buffer, int stride, FrameSerializer* serializer,
                          FrameSerializer::PtrKey& key);
    // 0x7d1faec - Meta, the Ptr, and (when needed) the first `count` elements.
    static void Serialize(QBuffer* buffer, int stride, FrameSerializer* serializer, int count,
                          ItemSerializer itemSerializer);
};

static_assert(sizeof(QBuffer) == 12);

struct QCollectionsUtils {
    // 0x7d1fc00 / 0x7d1fdc0 - The first table prime strictly greater than `value`.
    // Throws std::overflow_error past the largest table prime.
    static int GetNextPrime(int value);
    static std::uint32_t GetNextPrime(std::uint32_t value);
    // 0x7d1ff80 / 0x7d20140 - The first table prime greater than or equal to `value`.
    static int GetNextPrimeGreaterThanOrEqual(int value);
    static std::uint32_t GetNextPrimeGreaterThanOrEqual(std::uint32_t value);

    // 0x7d204bc (.cctor) builds the tables; here they are constants in the .cpp.
    // TODO(0x7d20300, Quantum.Collections.QCollectionsUtils.RegisterTypes): registers the
    // collection record types with the serializer's type registry.
};

} // namespace Quantum::Collections
