// value_support.h - shared building blocks of the simulation's data types: hashing and
// serialisation of individual fields, and typed collection helpers.
//
// The original's generated code spells every field's handling out inline; here the
// per-type knowledge (how an int, an FP, an enum, a nested struct... is hashed and
// serialised) lives in these overloads so that each type's GetHashCode / Serialize reads
// as a plain list of its fields.
#pragma once

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "photon_deterministic/rng_session.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_ref_entity_prototype.h"
#include "quantum_core/button.h"
#include "quantum_core/component_prototype_ref.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/hash_code_utils.h"
#include "quantum_core/physics3d/hit3d.h"
#include "quantum_core/player_ref.h"
#include "quantum_core/ptr.h"
#include "quantum_core/q_boolean.h"
#include "quantum_core/q_dictionary.h"
#include "quantum_core/q_list.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/transform.h"

#include <array>
#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::RNGSession;

// --- hashing ---------------------------------------------------------------------------

// .NET GetHashCode of the built-in value types, as the original runtime computes them.
constexpr std::int32_t FieldHash(std::int32_t v) { return v; }
constexpr std::int32_t FieldHash(std::uint32_t v) { return static_cast<std::int32_t>(v); }
constexpr std::int32_t FieldHash(std::uint8_t v) { return v; }
constexpr std::int32_t FieldHash(std::uint16_t v) { return v; }
constexpr std::int32_t FieldHash(std::int8_t v) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(v) ^ (static_cast<std::uint32_t>(v) << 8));
}
constexpr std::int32_t FieldHash(std::int16_t v) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::uint16_t>(v)) |
                                     (static_cast<std::uint32_t>(v) << 16));
}
constexpr std::int32_t FieldHash(std::int64_t v) {
    return HashCodeUtils::UInt64HashCode(static_cast<std::uint64_t>(v));
}
constexpr std::int32_t FieldHash(std::uint64_t v) { return HashCodeUtils::UInt64HashCode(v); }
constexpr std::int32_t FieldHash(Ptr v) { return v.GetHashCode(); }

// Enumerations hash as their numeric value.
template <class E>
    requires std::is_enum_v<E>
constexpr std::int32_t FieldHash(E v) {
    return FieldHash(static_cast<std::underlying_type_t<E>>(v));
}

// Everything else has a GetHashCode() of its own.
template <class T>
    requires requires(const T& t) {
        { t.GetHashCode() } -> std::convertible_to<std::int32_t>;
    }
std::int32_t FieldHash(const T& v) {
    return static_cast<std::int32_t>(v.GetHashCode());
}

// Fixed-size buffers hash like HashCodeUtils.GetArrayHashCode: h = h * 31 + element,
// starting from 0.
template <class T, std::size_t N> std::int32_t FieldHash(const std::array<T, N>& items) {
    return HashCodeUtils::GetArrayHashCode(items.data(), static_cast<int>(N),
                                           [](const T& item) { return FieldHash(item); });
}

// The generated hash pattern: hash = seed, then hash = hash * 31 + field for each field
// (32-bit wrap-around).
class HashBuilder {
  public:
    constexpr explicit HashBuilder(std::uint32_t seed) : _hash(seed) {}

    template <class T> constexpr HashBuilder& Add(const T& value) {
        _hash = _hash * 31u + static_cast<std::uint32_t>(FieldHash(value));
        return *this;
    }

    constexpr std::int32_t Value() const { return static_cast<std::int32_t>(_hash); }

  private:
    std::uint32_t _hash;
};

// --- serialisation ---------------------------------------------------------------------

inline void SerializeField(std::int32_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::uint32_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::int64_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::uint64_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::int16_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::uint16_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::uint8_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(std::int8_t* v, FrameSerializer* s) { s->Stream().Serialize(v); }
inline void SerializeField(FP* v, FrameSerializer* s) { FP::Serialize(v, *s); }
inline void SerializeField(RNGSession* v, FrameSerializer* s) { RNGSession::Serialize(v, *s); }

// Enumerations travel as their underlying integer.
template <class E>
    requires std::is_enum_v<E>
void SerializeField(E* v, FrameSerializer* s) {
    auto raw = static_cast<std::underlying_type_t<E>>(*v);
    SerializeField(&raw, s);
    *v = static_cast<E>(raw);
}

// Types with a static Serialize(void*, serializer) of their own.
template <class T>
    requires requires(T* t, FrameSerializer* s) { T::Serialize(t, s); }
void SerializeField(T* v, FrameSerializer* s) {
    T::Serialize(v, s);
}

// Fixed arrays exposed through a FixedArray property: element by element (FixedArray.
// Serialize with the element type's serializer).
template <class T, std::size_t N> void SerializeField(std::array<T, N>* items, FrameSerializer* s) {
    for (T& item : *items) {
        SerializeField(&item, s);
    }
}

// Raw fixed buffers of a primitive type: one SerializeBuffer call.
template <class T, std::size_t N> void SerializeBuffer(std::array<T, N>& items, FrameSerializer* s) {
    s->Stream().SerializeBuffer(items.data(), static_cast<int>(N));
}

// Physics query results and shapes are written by quantum.core's own serialisers, which
// are not reimplemented yet; any type containing one has a TODO Serialize.
inline void SerializeField(Physics3D::Hit3D*, FrameSerializer*) {
    throw std::logic_error("Hit3D serialisation is not implemented yet");
}
inline void SerializeField(Shape3D*, FrameSerializer*) {
    throw std::logic_error("Shape3D serialisation is not implemented yet");
}

// A collection item serializer (StaticDelegates.SerializeXxx in the original).
template <class T> void SerializeItem(void* item, FrameSerializer* s) {
    SerializeField(static_cast<T*>(item), s);
}

// QList.Serialize<T>: the list behind `ptr`, items through T's serializer.
template <class T> void SerializeList(Ptr* ptr, FrameSerializer* s) {
    Collections::QListInternal::Serialize(ptr, s, static_cast<int>(sizeof(T)), &SerializeItem<T>);
}

// QDictionary.Serialize<K, V>.
template <class K, class V> void SerializeDictionary(Ptr* ptr, FrameSerializer* s) {
    Collections::QDictionaryInternal::Serialize(ptr, s, static_cast<int>(sizeof(K)),
                                                static_cast<int>(sizeof(V)), &SerializeItem<K>,
                                                &SerializeItem<V>);
}

// --- collection allocation -------------------------------------------------------------

template <class T> Ptr AllocateListPtr(Core::FrameBase* f, int capacity) {
    Collections::QListPtr<T> list;
    f->AllocateList<T>(list, capacity);
    return list.Ptr;
}

template <class K, class V> Ptr AllocateDictionaryPtr(Core::FrameBase* f, int capacity) {
    Collections::QDictionaryPtr<K, V> dictionary;
    f->AllocateDictionary<K, V>(dictionary, capacity);
    return dictionary.Ptr;
}

template <class K, class V> Ptr AllocateEnumDictionaryPtr(Core::FrameBase* f, int capacity) {
    Collections::QDictionaryPtr<K, V> dictionary;
    f->AllocateEnumDictionary<K, V>(dictionary, capacity);
    return dictionary.Ptr;
}

} // namespace Quantum
