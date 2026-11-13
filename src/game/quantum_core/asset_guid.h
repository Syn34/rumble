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
    // 0x7ce9f5c / 0x7ce0990
    std::string ToString() const;

    // TODO(0x7ce9f40, Quantum.AssetRef.op_Implicit(AssetObject)): returns the
    // asset's identifier GUID, or an empty reference for null; needs AssetObject.
    // 0x7ce9f4c - Same as AssetGuid::Serialize on Id.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRef*>(ptr)->Id, serializer);
    }
    // TODO(0x7ce9f88, Quantum.AssetRef.Equals(object)): boxed comparison; not needed in C++.
};

} // namespace Quantum
