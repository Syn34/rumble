// asset_guid.h - identifiers for simulation assets.
//
// Original: Quantum.AssetGuid and Quantum.AssetRef (quantum.core.dll).
// Every simulation asset (weapon specs, game mode configs, maps...) has a 64-bit
// GUID. The top bit marks assets created at runtime ("dynamic"); the bit below
// it is reserved. Value 0 means "no asset".
#pragma once

#include <compare>
#include <cstdint>
#include <string>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum {

class AssetObject;

struct AssetGuid {
    static constexpr std::uint64_t DynamicBit = std::uint64_t{1} << 63;
    static constexpr std::int64_t ReservedBits = std::int64_t{1} << 62;
    static constexpr std::int64_t Invalid = 0;

    std::int64_t Value = Invalid;

    constexpr AssetGuid() = default;
    // 0x7ce0500
    constexpr explicit AssetGuid(std::int64_t value) : Value(value) {}

    // 0x7ce04e4
    constexpr bool IsValid() const { return Value != Invalid; }
    // 0x7ce04f4
    constexpr bool IsDynamic() const { return (static_cast<std::uint64_t>(Value) >> 63) != 0; }

    // Random GUID for an asset created outside the asset database.
    static AssetGuid NewGuid();

    // 0x7ce0744 (CompareTo), 0x7ce0734 / 0x7ce074c (Equals), 0x7ce07cc / 0x7ce07d8 (==, !=)
    friend constexpr auto operator<=>(AssetGuid, AssetGuid) = default;

    // 0x7ce07c4 - same as the hash of a 64-bit integer: low word xor high word.
    std::int32_t GetHashCode() const;

    // 0x7ce07e4 / 0x7ce07ec - 16 upper-case hex digits, optionally in brackets.
    std::string ToString(bool brackets = true) const;

    // 0x7ce0610 - Writes or reads Value as a 64-bit integer through the serializer's
    // stream.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer);
};

// A reference to an asset by GUID. Original: Quantum.AssetRef.
struct AssetRef {
    AssetGuid Id;

    friend constexpr auto operator<=>(AssetRef, AssetRef) = default; // 0x7ce9f6c, 0x7ce9f78

    // 0x7ce9f64
    std::int32_t GetHashCode() const { return Id.GetHashCode(); }
    // 0x7ce9f5c
    std::string ToString() const { return ToString(Id); }
    // 0x7ce0990 - the text every typed reference uses: "Id: " and the bracketed GUID.
    static std::string ToString(AssetGuid id);

    // 0x7ce9f40 (op_Implicit(AssetObject)) - the asset's GUID; empty for null. Defined in
    // asset_object.cpp.
    static AssetRef FromAsset(const AssetObject* asset);
    // 0x7ce9f4c - Same as AssetGuid::Serialize on Id.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRef*>(ptr)->Id, serializer);
    }
    // 0x7ce9f88 (Equals(object)) is the boxed form of operator==.
};

// The typed references the code generator emits per asset class (AssetRefMap,
// AssetRefNavMesh, ...). They all have the same shape - one GUID - and the same members,
// so one template stands for all of them; each asset header names its own alias and lists
// the original addresses of that type's members next to it.
//
// Members (original names): op_Implicit(T) -> FromAsset, Serialize, ToString, Equals /
// op_Equality / op_Inequality -> operator==, GetHashCode, and the generated
// AssetRef<T>Ext.FindAsset(frame) extension -> FindAsset.
template <class T> struct TypedAssetRef {
    static constexpr int SIZE = 8;

    AssetGuid Id;

    // The asset's GUID; empty for null.
    static TypedAssetRef FromAsset(const T* asset) {
        return TypedAssetRef{asset != nullptr ? asset->Guid : AssetGuid{}};
    }
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(ptr, serializer);
    }
    std::string ToString() const { return AssetRef::ToString(Id); }
    std::int32_t GetHashCode() const { return Id.GetHashCode(); }
    friend constexpr bool operator==(TypedAssetRef, TypedAssetRef) = default;

    operator AssetRef() const { return AssetRef{Id}; }

    // AssetRef<T>Ext.FindAsset: the frame's FindAsset<T> (main-thread lookup, logs when
    // missing or of another type). Works with FrameBase and FrameThreadSafe.
    template <class TFrame> T* FindAsset(TFrame* frame) const {
        return frame->template FindAsset<T>(Id);
    }
};

} // namespace Quantum
