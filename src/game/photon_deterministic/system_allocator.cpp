#include "system_allocator.h"

#include <cstddef>
#include <cstring>
#include <new>
#include <stdexcept>

namespace Photon::Deterministic {

namespace {

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
    const std::size_t align = alignment > static_cast<int>(kMinAlignment)
                                  ? static_cast<std::size_t>(alignment)
                                  : kMinAlignment;
    // The header sits just in front of the returned block.
    auto* raw = static_cast<std::byte*>(
        ::operator new(static_cast<std::size_t>(count) + align, std::align_val_t{align}));
    std::byte* user = raw + align;
    const BlockHeader header{align};
    std::memcpy(user - sizeof(BlockHeader), &header, sizeof(BlockHeader));
    return user;
}

void SystemAllocator::Free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    auto* user = static_cast<std::byte*>(ptr);
    BlockHeader header{};
    std::memcpy(&header, user - sizeof(BlockHeader), sizeof(BlockHeader));
    ::operator delete(user - header.alignment, std::align_val_t{header.alignment});
}

void SystemAllocator::Clear(void* dest, int count) {
    std::memset(dest, 0, static_cast<std::size_t>(count));
}

Native::ObjectHandle SystemUtility::HandleAcquire(void* obj) {
    Native::ObjectHandle handle;
    handle.Address = reinterpret_cast<std::intptr_t>(obj);
    return handle;
}

void SystemUtility::HandleRelease(Native::ObjectHandle) {}

void* SystemUtility::GetObjectForHandle(Native::ObjectHandle handle) {
    return reinterpret_cast<void*>(handle.Address);
}

void SystemUtility::Copy(void* dest, const void* src, int count) {
    std::memcpy(dest, src, static_cast<std::size_t>(count));
}

void SystemUtility::Clear(void* dest, int count) {
    std::memset(dest, 0, static_cast<std::size_t>(count));
}

void SystemUtility::Move(void* dest, const void* src, int count) {
    std::memmove(dest, src, static_cast<std::size_t>(count));
}

int SystemUtility::Compare(const void* ptr1, const void* ptr2, int count) {
    return std::memcmp(ptr1, ptr2, static_cast<std::size_t>(count));
}

void SystemUtility::Install() {
    static SystemUtility instance;
    Native::Utils = &instance;
}

} // namespace Photon::Deterministic
