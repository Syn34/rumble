// system_allocator.h - PC implementations of the platform memory services.
//
// The original simulation runtime declares Native::Allocator and Native::Utility
// as abstract classes and leaves the implementation to the hosting platform. These
// are this project's implementations for desktop builds (new code, no original
// counterpart).
#pragma once

#include "native.h"

namespace Photon::Deterministic {

// Aligned operator new/delete. Each block remembers its alignment so Free can
// release it without being told.
class SystemAllocator final : public Native::Allocator {
  public:
    void Free(void* ptr) override;
    void* Alloc(int count) override;
    void* Alloc(int count, int alignment) override;
    void Clear(void* dest, int count) override;
};

// Raw memory operations via the C library. Object handles are plain addresses:
// C++ objects don't move, so there is nothing to pin.
class SystemUtility final : public Native::Utility {
  public:
    Native::ObjectHandle HandleAcquire(void* obj) override;
    void HandleRelease(Native::ObjectHandle handle) override;
    void* GetObjectForHandle(Native::ObjectHandle handle) override;
    void Copy(void* dest, const void* src, int count) override;
    void Clear(void* dest, int count) override;
    void Move(void* dest, const void* src, int count) override;
    int Compare(const void* ptr1, const void* ptr2, int count) override;

    // Installs a process-wide instance as Native::Utils (idempotent).
    static void Install();
};

} // namespace Photon::Deterministic
