#include "native.h"

#include <stdexcept>
#include <string>

namespace Photon::Deterministic {

Native::Utility* Native::Utils = nullptr;

Native::Utility& Native::RequireUtils() {
    if (Utils == nullptr) {
        throw std::logic_error("Native::Utils has not been installed by the platform");
    }
    return *Utils;
}

void* Native::Allocator::AllocAndClear(int count) {
    void* block = Alloc(count);
    Clear(block, count);
    return block;
}

void* Native::Allocator::AllocAndClear(int count, int alignment) {
    void* block = Alloc(count, alignment);
    Clear(block, count);
    return block;
}

void* Native::Allocator::Expand(void* buffer, int currentSize, int newSize) {
    void* grown = Alloc(newSize);
    Clear(grown, newSize);
    RequireUtils().Copy(grown, buffer, currentSize);
    Free(buffer);
    return grown;
}

void Native::Utility::ClearFast(void* dest, int count) {
    RequireUtils().Clear(dest, count);
}

void Native::Utility::CopyFast(void* dest, const void* src, int count) {
    RequireUtils().Copy(dest, src, count);
}

int Native::RoundUpToAlignment(int size, int alignment) {
    switch (alignment) {
    case 1:
        return size;
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
        return (size + alignment - 1) & ~(alignment - 1);
    default:
        throw std::invalid_argument("Native::RoundUpToAlignment: invalid alignment " +
                                    std::to_string(alignment));
    }
}

int Native::GetAlignmentForArrayElement(int elementSize) {
    return (elementSize > 0 && elementSize % 8 == 0) ? 8 : 4;
}

} // namespace Photon::Deterministic
