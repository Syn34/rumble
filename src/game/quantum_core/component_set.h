// component_set.h - Quantum.ComponentSet: a 256-bit set of component type indices.
//
// Every entity has one (its "component mask": bit i set = the entity has the component
// with type index i), and filters use them to describe required, optional and excluded
// components.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct ComponentSet {
    static constexpr int SIZE = 32;
    static constexpr int BLOCK_COUNT = 4;
    static constexpr int BITS_PER_BLOCK = 64;
    static constexpr int MAX_COMPONENTS = 256;

    std::uint64_t _set[BLOCK_COUNT] = {};

    // 0x7ceba48
    bool IsEmpty() const { return (_set[0] | _set[1] | _set[2] | _set[3]) == 0; }

    // 0x7ceba78, 0x7cebaa0, 0x7cebac8
    void Add(int index) { _set[index / BITS_PER_BLOCK] |= Bit(index); }
    void Remove(int index) { _set[index / BITS_PER_BLOCK] &= ~Bit(index); }
    bool IsSet(int index) const { return (_set[index / BITS_PER_BLOCK] & Bit(index)) != 0; }

    // 0x4cb8dc0 (instance for Transform3D) - Adds the type index of component T.
    template <class T> void Add();

    // 0x7cebae8 (Equals(ComponentSet)); 0x7cebb34 is the boxed Equals(object).
    bool Equals(const ComponentSet& other) const {
        return _set[0] == other._set[0] && _set[1] == other._set[1] &&
               _set[2] == other._set[2] && _set[3] == other._set[3];
    }
    friend bool operator==(const ComponentSet& a, const ComponentSet& b) { return a.Equals(b); }

    // 0x7cebbe4 - h = H(block0); h = (h * 33) ^ H(block i) for the other blocks, where H
    // folds a 64-bit word to 32 bits (HashCodeUtils.UInt64HashCode: low ^ high).
    int GetHashCode() const;

    // 0x7cebc34 - The registered names of the contained component types (wording is this
    // project's own).
    std::string ToString() const;

    // 0x7cebe30 - True if every index in `other` is also in this set.
    bool IsSupersetOf(const ComponentSet& other) const {
        return (other._set[0] & ~_set[0]) == 0 && (other._set[1] & ~_set[1]) == 0 &&
               (other._set[2] & ~_set[2]) == 0 && (other._set[3] & ~_set[3]) == 0;
    }

    // True if the two sets share at least one index (inlined by the original filters).
    bool Overlaps(const ComponentSet& other) const {
        return (other._set[0] & _set[0]) != 0 || (other._set[1] & _set[1]) != 0 ||
               (other._set[2] & _set[2]) != 0 || (other._set[3] & _set[3]) != 0;
    }

    // 0x7cebe7c - The four blocks, as unsigned 64-bit values.
    static void Serialize(void* ptr, FrameSerializer* serializer);

    // 0x4cba0fc (instance for <Transform3D, Killable>) and the 1/3/4-type overloads - A
    // set holding the type indices of the given components.
    template <class... Ts> static ComponentSet Create() {
        ComponentSet set;
        (set.Add<Ts>(), ...);
        return set;
    }

  private:
    // The shift amount wraps like the original's 64-bit shift (index mod 64).
    static constexpr std::uint64_t Bit(int index) {
        return std::uint64_t{1} << (static_cast<unsigned>(index) & 63u);
    }
};

static_assert(sizeof(ComponentSet) == ComponentSet::SIZE);

} // namespace Quantum

#include "quantum_core/component_type_id.h"

namespace Quantum {

template <class T> void ComponentSet::Add() { Add(ComponentTypeId<T>::Id); }

} // namespace Quantum
