// settings_reader.h - typed reading of decoded asset data (PC-side glue; new code).
//
// A simulation asset is stored as a script object whose `Settings` field holds the asset's
// serialised fields under their original names. The decoder turns that into a tree of
// rr::assets::Value; these helpers copy the tree into the C++ asset types:
//
//   FP            { RawValue: SInt64 }
//   FPVector2/3   { X: FP, Y: FP[, Z: FP] },  FPQuaternion { X, Y, Z, W }
//   AssetRef*     { Id: { Value: SInt64 } }   (every typed reference has an AssetGuid `Id`)
//   EntityRef     { Index, Version }
//   LayerMask     { BitMask }
//   enums / bool / integers stored as integers, strings as strings, arrays and lists as
//   arrays.
//
// Structured types are read by a ReadFields(const Value&, T&, ReaderTag) overload, found
// through the ReaderTag argument; the generated per-class readers live in
// quantum_code_factories.cpp. A field missing from the data (older data, or a field the
// editor didn't serialise) keeps the C++ default, as the original deserialiser does.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/fp_animation_curve.h"
#include "quantum_core/layer_mask.h"
#include "quantum_core/shape3d_config.h"
#include "rr/assets/type_tree_value.h"

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Quantum {
struct QBoolean;
struct EntityPrototypeRef;
struct StaticColliderData;
struct MapStaticCollider3D;
struct AutoJumperStartParams;
struct ChargingStats;
struct GOAPState;
struct UTMomentumData;
struct WeaponLoadoutItemData;
} // namespace Quantum

namespace rr::content {

using rr::assets::Value;

// Selects the ReadFields overloads of this namespace through argument-dependent lookup.
struct ReaderTag {};

// The child `name` of a structured value, or null.
inline const Value* Child(const Value& parent, std::string_view name) {
    const auto* fields = parent.AsStruct();
    if (!fields) return nullptr;
    for (const auto& field : *fields)
        if (field.name == name) return field.value.get();
    return nullptr;
}

template <class T>
concept HasGuidId = requires(T t) {
    { t.Id } -> std::convertible_to<Quantum::AssetGuid>;
};

template <class T>
concept HasFieldReader = requires(const Value& v, T& out) { ReadFields(v, out, ReaderTag{}); };

template <class T> void ReadValue(const Value& value, T& out);

inline void ReadValue(const Value& value, bool& out) { out = value.AsInt(out ? 1 : 0) != 0; }
inline void ReadValue(const Value& value, std::string& out) {
    if (const auto* text = value.AsString()) out = *text;
}
inline void ReadValue(const Value& value, Photon::Deterministic::FP& out) {
    if (const auto* raw = Child(value, "RawValue")) out = Photon::Deterministic::FP::FromRaw(raw->AsInt());
}
inline void ReadValue(const Value& value, Photon::Deterministic::FPVector2& out) {
    if (const auto* x = Child(value, "X")) ReadValue(*x, out.X);
    if (const auto* y = Child(value, "Y")) ReadValue(*y, out.Y);
}
inline void ReadValue(const Value& value, Photon::Deterministic::FPVector3& out) {
    if (const auto* x = Child(value, "X")) ReadValue(*x, out.X);
    if (const auto* y = Child(value, "Y")) ReadValue(*y, out.Y);
    if (const auto* z = Child(value, "Z")) ReadValue(*z, out.Z);
}
inline void ReadValue(const Value& value, Photon::Deterministic::FPQuaternion& out) {
    if (const auto* x = Child(value, "X")) ReadValue(*x, out.X);
    if (const auto* y = Child(value, "Y")) ReadValue(*y, out.Y);
    if (const auto* z = Child(value, "Z")) ReadValue(*z, out.Z);
    if (const auto* w = Child(value, "W")) ReadValue(*w, out.W);
}
inline void ReadValue(const Value& value, Quantum::AssetGuid& out) {
    if (const auto* raw = Child(value, "Value")) out = Quantum::AssetGuid(raw->AsInt());
}
inline void ReadValue(const Value& value, Quantum::EntityRef& out) {
    if (const auto* index = Child(value, "Index")) out.Index = static_cast<int>(index->AsInt());
    if (const auto* version = Child(value, "Version")) out.Version = static_cast<int>(version->AsInt());
}
inline void ReadValue(const Value& value, Quantum::LayerMask& out) {
    if (const auto* mask = Child(value, "BitMask")) out.BitMask = static_cast<std::int32_t>(mask->AsInt());
}

// Byte arrays may be decoded as one blob or as an array of numbers.
inline void ReadValue(const Value& value, std::vector<std::uint8_t>& out) {
    if (const auto* bytes = std::get_if<Value::Bytes>(&value.data)) {
        out.assign(bytes->begin(), bytes->end());
        return;
    }
    if (const auto* items = value.AsArray()) {
        out.clear();
        out.reserve(items->size());
        for (const auto& item : *items) out.push_back(static_cast<std::uint8_t>(item->AsInt()));
    }
}

template <class T> void ReadValue(const Value& value, std::vector<T>& out) {
    const auto* items = value.AsArray();
    if (!items) return;
    out.clear();
    out.resize(items->size());
    for (std::size_t i = 0; i < items->size(); ++i) ReadValue(*(*items)[i], out[i]);
}

template <class T> void ReadValue(const Value& value, T& out) {
    if constexpr (std::is_enum_v<T>) {
        out = static_cast<T>(value.AsInt(static_cast<std::int64_t>(out)));
    } else if constexpr (std::is_floating_point_v<T>) {
        out = static_cast<T>(value.AsDouble(out));
    } else if constexpr (std::is_integral_v<T>) {
        out = static_cast<T>(value.AsInt(static_cast<std::int64_t>(out)));
    } else if constexpr (HasGuidId<T>) {
        if (const auto* id = Child(value, "Id")) ReadValue(*id, out.Id);
    } else {
        static_assert(HasFieldReader<T>, "no reader for this type");
        ReadFields(value, out, ReaderTag{});
    }
}

// Reads the field `name` of `parent` into `out` if it is present.
template <class T> void Read(const Value& parent, std::string_view name, T& out) {
    if (const auto* value = Child(parent, name)) ReadValue(*value, out);
}

// Hand-written readers for quantum.core types and for quantum.code value types defined
// outside the asset headers (builtin_asset_factories.cpp).
void ReadFields(const Value& value, Quantum::FPAnimationCurve::Keyframe& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FPAnimationCurve& out, ReaderTag);
void ReadFields(const Value& value, Quantum::QBoolean& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityPrototypeRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Shape3DConfig::CompoundShapeData3D& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Shape3DConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StaticColliderData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapStaticCollider3D& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AutoJumperStartParams& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargingStats& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPState& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UTMomentumData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponLoadoutItemData& out, ReaderTag);

} // namespace rr::content
