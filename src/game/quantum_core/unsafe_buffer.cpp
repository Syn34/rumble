#include "unsafe_buffer.h"

#include <stdexcept>

namespace Quantum::Collections {

void UnsafeBuffer::Free(Photon::Deterministic::Native::Allocator& allocator, UnsafeBuffer* buffer) {
    if (buffer->Dynamic == 0) {
        throw std::logic_error("UnsafeBuffer::Free: buffer does not own its memory");
    }
    allocator.Free(buffer->Ptr);
    *buffer = UnsafeBuffer{};
}

void UnsafeBuffer::Clear(UnsafeBuffer* buffer) {
    std::memset(buffer->Ptr, 0,
                static_cast<std::size_t>(buffer->Length) *
                    static_cast<std::size_t>(buffer->Stride));
}

void UnsafeBuffer::InitFixed(UnsafeBuffer* buffer, void* ptr, int length, int stride) {
    buffer->Ptr = ptr;
    buffer->Length = length;
    buffer->Stride = stride;
    buffer->Dynamic = 0;
}

void UnsafeBuffer::InitDynamic(Photon::Deterministic::Native::Allocator& allocator,
                               UnsafeBuffer* buffer, int length, int stride) {
    const int alignment = Photon::Deterministic::Native::GetAlignmentForArrayElement(stride);
    buffer->Ptr = allocator.AllocAndClear(stride * length, alignment);
    buffer->Length = length;
    buffer->Stride = stride;
    buffer->Dynamic = 1;
}

void UnsafeBuffer::Copy(UnsafeBuffer source, int sourceIndex, UnsafeBuffer destination,
                        int destinationIndex, int count) {
    const int stride = source.Stride;
    if (count > 0) {
        std::memcpy(Element(destination.Ptr, destinationIndex, stride),
                    Element(source.Ptr, sourceIndex, stride),
                    static_cast<std::size_t>(stride) * static_cast<std::size_t>(count));
    }
}

} // namespace Quantum::Collections

namespace Quantum::PhysicsCommon {

int Utils::GetNextPowerOfTwo(int value) {
    // Smear the highest set bit of value - 1 downwards, then step to the next power. The
    // shifts are on a signed int as in the original, so inputs <= 0 give 0.
    int v = static_cast<int>(static_cast<std::uint32_t>(value) - 1u);
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return static_cast<int>(static_cast<std::uint32_t>(v) + 1u);
}

UnsafeBuffer UnsafeBuffer::InitBuffer(int stride,
                                      Photon::Deterministic::Native::Allocator& allocator,
                                      int capacity) {
    UnsafeBuffer buffer(stride);
    buffer.ResetAndEnsureCapacity(allocator, capacity);
    return buffer;
}

void* UnsafeBuffer::ResetAndEnsureCapacity(Photon::Deterministic::Native::Allocator& allocator,
                                           int capacity) {
    // The capacity is compared as a 64-bit signed value against the request zero-extended,
    // so a negative request never fits.
    if (static_cast<std::int64_t>(_capacity) <
        static_cast<std::int64_t>(static_cast<std::uint32_t>(capacity))) {
        if (_buffer != nullptr) {
            allocator.Free(_buffer);
        }
        _capacity = Utils::GetNextPowerOfTwo(capacity);
        _buffer = allocator.AllocAndClear(_stride * _capacity);
    }
    _count = 0;
    return _buffer;
}

void* UnsafeBuffer::GetNextEntry(Photon::Deterministic::Native::Allocator& allocator, int& index) {
    if (_count == _capacity) {
        void* old = _buffer;
        _capacity = _count * 2 > 8 ? _count * 2 : 8;
        _buffer = allocator.Alloc(_stride * _capacity);
        if (old != nullptr) {
            std::memcpy(_buffer, old,
                        static_cast<std::size_t>(_stride) * static_cast<std::size_t>(_count));
            allocator.Free(old);
        }
    }
    index = _count++;
    void* entry = static_cast<std::byte*>(_buffer) + static_cast<std::ptrdiff_t>(_stride) * index;
    std::memset(entry, 0, static_cast<std::size_t>(_stride));
    return entry;
}

void UnsafeBuffer::ResetAndClear(bool clearUsedCountOnly) {
    if (_buffer != nullptr && _capacity >= 1) {
        const int entries = clearUsedCountOnly ? _count : _capacity;
        std::memset(_buffer, 0,
                    static_cast<std::size_t>(entries) * static_cast<std::size_t>(_stride));
    }
    _count = 0;
}

void UnsafeBuffer::Dispose(Photon::Deterministic::Native::Allocator& allocator) {
    if (_buffer != nullptr) {
        allocator.Free(_buffer);
    }
    _buffer = nullptr;
    _count = 0;
    _capacity = 0;
    _stride = 0;
}

} // namespace Quantum::PhysicsCommon
