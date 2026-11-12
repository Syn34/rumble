#include "native_allocator.h"

#include <new>
#include <stdexcept>
#include <string>

namespace Photon::Deterministic::Native {

int RoundUpToAlignment(int value, int alignment) {
    switch (alignment) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
        return (value + alignment - 1) & ~(alignment - 1);
    default:
        throw std::invalid_argument("RoundUpToAlignment: unsupported alignment " +
                                    std::to_string(alignment));
    }
}

void* Allocator::AllocAndClear(int count) {
    void* block = Alloc(count);
    Clear(block, count);
    return block;
}

void* Allocator::AllocAndClear(int count, int alignment) {
    void* block = Alloc(count, alignment);
    Clear(block, count);
    return block;
}

void* Allocator::Expand(void* buffer, int currentSize, int newSize) {
    void* grown = Alloc(newSize);
    Clear(grown, newSize);
    std::memcpy(grown, buffer, static_cast<std::size_t>(currentSize));
    Free(buffer);
    return grown;
}

namespace {

// Each block starts with a header recording the alignment it was allocated with, so the
// matching aligned delete can be used. The header occupies one alignment unit (at least
// 16 bytes) to keep the returned pointer aligned.
constexpr std::size_t kMinAlignment = 16;

struct BlockHeader {
    std::size_t alignment;
};

} // namespace

void* SystemAllocator::Alloc(int count) {
    return Alloc(count, static_cast<int>(kMinAlignment));
}

void* SystemAllocator::Alloc(int count, int alignment) {
    if (count < 0) {
        throw std::invalid_argument("SystemAllocator::Alloc: negative size");
    }
    std::size_t align = alignment > static_cast<int>(kMinAlignment)
                            ? static_cast<std::size_t>(alignment)
                            : kMinAlignment;
    std::byte* raw = static_cast<std::byte*>(
        ::operator new(static_cast<std::size_t>(count) + align, std::align_val_t{align}));
    std::byte* user = raw + align;
    BlockHeader header{align};
    std::memcpy(user - sizeof(BlockHeader), &header, sizeof(BlockHeader));
    return user;
}

void SystemAllocator::Free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    std::byte* user = static_cast<std::byte*>(ptr);
    BlockHeader header{};
    std::memcpy(&header, user - sizeof(BlockHeader), sizeof(BlockHeader));
    ::operator delete(user - header.alignment, std::align_val_t{header.alignment});
}

void SystemAllocator::Clear(void* dest, int count) {
    std::memset(dest, 0, static_cast<std::size_t>(count));
}

} // namespace Photon::Deterministic::Native
