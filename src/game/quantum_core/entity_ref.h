// entity_ref.h - Quantum.EntityRef, Quantum.EntityFlags and Quantum.EntityInfo.
//
// An entity is a slot in the frame's entity table plus a version number. EntityRef is the
// handle the simulation passes around: the slot index and the version the slot had when
// the handle was made. A slot's version is bumped whenever its entity is destroyed, which
// turns every older handle stale; the top bit of the stored version marks the slot as
// currently holding a live entity. Index 0 is never handed out, so the all-zero handle
// (EntityRef::None) never refers to anything.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct EntityRef {
    static constexpr int SIZE = 8;

    int Index = 0;
    int Version = 0;

    // 0x7cf107c
    static constexpr EntityRef None() { return EntityRef{}; }

    // Both fields as one 64-bit value (the C# struct overlays them with a ulong field):
    // the index in the low half, the version in the high half.
    constexpr std::uint64_t Raw() const {
        return static_cast<std::uint32_t>(Index) |
               (static_cast<std::uint64_t>(static_cast<std::uint32_t>(Version)) << 32);
    }

    // 0x7cef688
    constexpr bool IsValid() const { return Raw() != 0; }

    // 0x7cf1a4c (==), 0x7cf1a58 (!=), 0x7ceb66c (Equals(EntityRef)). 0x7cf1a64 is the
    // boxed Equals(object).
    friend constexpr bool operator==(EntityRef a, EntityRef b) { return a.Raw() == b.Raw(); }
    constexpr bool Equals(EntityRef other) const { return Raw() == other.Raw(); }

    // 0x7cf1adc
    constexpr int GetHashCode() const {
        return static_cast<int>(static_cast<std::uint32_t>(Index) * 59209u +
                                static_cast<std::uint32_t>(Version));
    }

    // 0x7ceb7e4 - Writes or reads the handle at `ptr`. In input mode only a presence flag
    // is written for an empty handle (index 0), and reading an absent one yields None.
    static void Serialize(void* ptr, FrameSerializer* serializer);

    // 0x7cf1b90 - Debug text: index, version without the live bit, and a marker for
    // handles whose version lacks the live bit (wording is this project's own).
    std::string ToString() const;
};

static_assert(sizeof(EntityRef) == EntityRef::SIZE);

enum class EntityFlags : int {
    None = 0,
    NotCullable = 1,
    DestroyPending = 2,
};

constexpr EntityFlags operator|(EntityFlags a, EntityFlags b) {
    return static_cast<EntityFlags>(static_cast<int>(a) | static_cast<int>(b));
}
constexpr EntityFlags operator&(EntityFlags a, EntityFlags b) {
    return static_cast<EntityFlags>(static_cast<int>(a) & static_cast<int>(b));
}
constexpr EntityFlags operator~(EntityFlags a) {
    return static_cast<EntityFlags>(~static_cast<int>(a));
}

// One slot of the frame's entity table. For a free slot, Ref.Index holds the next free
// slot (the free list is threaded through the table).
struct alignas(8) EntityInfo {
    // Set in the stored version while the slot holds a live entity.
    static constexpr int ACTIVE_BIT = static_cast<int>(0x80000000u);
    static constexpr int VERSION_MASK = 0x7FFFFFFF;

    EntityRef Ref;
    EntityFlags Flags = EntityFlags::None;

    // 0x7cf1c68 - The handle, then the flags as a plain int.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

// Padded to 16 bytes like the original, which sizes and copies the table with that stride.
static_assert(sizeof(EntityInfo) == 16);

} // namespace Quantum

template <> struct std::hash<Quantum::EntityRef> {
    std::size_t operator()(Quantum::EntityRef e) const noexcept {
        return static_cast<std::size_t>(e.GetHashCode());
    }
};
