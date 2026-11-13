// asset_ref_entity_prototype.h - Quantum.AssetRefEntityPrototype: a typed reference to an
// entity prototype asset.
#pragma once

#include "quantum_core/asset_guid.h"

#include <cstdint>
#include <string>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum {

struct AssetRefEntityPrototype {
    static constexpr int SIZE = 8;

    AssetGuid Id;

    // 0x7ce1364 (==), 0x7ce1370 (!=), 0x7ce12d4 (Equals(AssetRefEntityPrototype)); 0x7ce12e4
    // is the boxed Equals(object).
    friend constexpr bool operator==(AssetRefEntityPrototype a, AssetRefEntityPrototype b) {
        return a.Id == b.Id;
    }
    constexpr bool Equals(AssetRefEntityPrototype other) const { return Id == other.Id; }

    // 0x7ce135c - The 64-bit hash of the GUID value.
    std::int32_t GetHashCode() const { return Id.GetHashCode(); }

    // 0x7ce12cc
    std::string ToString() const { return AssetRef{Id}.ToString(); }

    // 0x7ce12bc
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
        AssetGuid::Serialize(&static_cast<AssetRefEntityPrototype*>(ptr)->Id, serializer);
    }

    // TODO(0x7ce12b0, Quantum.AssetRefEntityPrototype.op_Implicit): EntityPrototype asset ->
    // reference (its GUID, or empty for null); needs the asset classes.
};

static_assert(sizeof(AssetRefEntityPrototype) == AssetRefEntityPrototype::SIZE);

} // namespace Quantum
