// native.h - Photon.Deterministic.Native: unmanaged memory services used by the simulation.
//
// Native.Allocator hands out raw blocks for frame and input memory; Native.Utility copies,
// clears and compares raw memory and pins managed objects behind handles. Both are
// abstract: the hosting platform supplies the implementations (the allocator through
// DeterministicPlatformInfo, the utility through the static Native::Utils).
#pragma once

#include <cstdint>

namespace Photon::Deterministic {

class Native final {
  public:
    Native() = delete;

    // Native.ObjectHandle: an opaque handle to a pinned managed object.
    struct ObjectHandle {
        std::intptr_t Address = 0;
        std::uint64_t Identifier = 0;

        ObjectHandle() = default;
        // 0x7a784cc
        explicit ObjectHandle(std::uint64_t identifier) : Identifier(identifier) {}
    };

    class Allocator {
      public:
        virtual ~Allocator() = default;

        // 0x7a784d4 - the base implementation has nothing to release.
        void Dispose() {}

        virtual void Free(void* ptr) = 0;
        virtual void* Alloc(int count) = 0;
        virtual void* Alloc(int count, int alignment) = 0;
        virtual void Clear(void* dest, int count) = 0;

        // 0x7a784d8
        void* AllocAndClear(int count);
        // 0x7a78520
        void* AllocAndClear(int count, int alignment);
        // Generic AllocAndClear<T>() (shared generic code, no own address): one zeroed T,
        // aligned for an array element of its size.
        template <typename T> T* AllocAndClear() {
            constexpr int size = static_cast<int>(sizeof(T));
            return static_cast<T*>(AllocAndClear(size, GetAlignmentForArrayElement(size)));
        }
        // 0x7a78568 - allocates `newSize` zeroed bytes, copies the first `currentSize` bytes
        // of `buffer` into them and frees the old block.
        void* Expand(void* buffer, int currentSize, int newSize);
        // Generic ExpandArray<T> (shared generic code): Expand measured in elements.
        template <typename T> T* ExpandArray(T* buffer, int currentSize, int newSize) {
            constexpr int size = static_cast<int>(sizeof(T));
            return static_cast<T*>(Expand(buffer, currentSize * size, newSize * size));
        }
    };

    class Utility {
      public:
        virtual ~Utility() = default;

        virtual ObjectHandle HandleAcquire(void* obj) = 0;
        virtual void HandleRelease(ObjectHandle handle) = 0;
        virtual void* GetObjectForHandle(ObjectHandle handle) = 0;
        virtual void Copy(void* dest, const void* src, int count) = 0;
        virtual void Clear(void* dest, int count) = 0;
        virtual void Move(void* dest, const void* src, int count) = 0;
        virtual int Compare(const void* ptr1, const void* ptr2, int count) = 0;

        // 0x7a78640 - forwards to Native::Utils (which may be another instance).
        static void ClearFast(void* dest, int count);
        // 0x7a786b0 - forwards to Native::Utils.
        static void CopyFast(void* dest, const void* src, int count);
    };

    // Native.Utils, installed by the platform before any simulation runs. Must not be
    // null when frame inputs are cleared or copied.
    static Utility* Utils;

    // 0x7a783ac - rounds `size` up to a multiple of `alignment` (1, 2, 4, 8, 16, 32 or 64);
    // any other alignment throws std::invalid_argument (InvalidOperationException).
    static int RoundUpToAlignment(int size, int alignment);
    // 0x7a784b4 - 8 for positive sizes that are a multiple of 8, otherwise 4.
    static int GetAlignmentForArrayElement(int elementSize);

    // The utility instance, throwing if the platform has not installed one.
    static Utility& RequireUtils();
};

} // namespace Photon::Deterministic
