// entity_prototype_ref.h - Quantum.EntityPrototypeRef: names one entity prototype.
//
// An entity prototype lives either in its own EntityPrototype asset (InnerAssetId -1) or
// inside a map asset, as entry InnerAssetId of Map::MapEntities (the "master asset" form).
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_ref_entity_prototype.h"
#include "quantum_core/hash_code_utils.h"

#include <cstdint>
#include <string>

namespace Quantum {

class EntityPrototype;
class Map;

struct EntityPrototypeRef {
    static constexpr int SIZE = 16;

    AssetRef Asset;
    std::int32_t InnerAssetId = 0;
    std::int32_t _padding = 0;

    // 0x7cef6a0
    static constexpr EntityPrototypeRef Invalid() { return EntityPrototypeRef{}; }
    // 0x7cef698
    constexpr AssetGuid AssetId() const { return Asset.Id; }
    // 0x7cef6ac - only the asset decides validity.
    constexpr bool IsValid() const { return Asset.Id.IsValid(); }

    // 0x7cef6bc (Equals(EntityPrototypeRef)); 0x7cef6e0 is the boxed Equals(object). The
    // padding does not take part.
    constexpr bool Equals(const EntityPrototypeRef& other) const {
        return Asset == other.Asset && InnerAssetId == other.InnerAssetId;
    }
    friend constexpr bool operator==(const EntityPrototypeRef& a, const EntityPrototypeRef& b) {
        return a.Equals(b);
    }

    // 0x7cef790 - hash of the 64-bit GUID, then h * 33 ^ InnerAssetId.
    constexpr std::int32_t GetHashCode() const {
        return HashCodeUtils::CombineHashCodes(
            HashCodeUtils::UInt64HashCode(static_cast<std::uint64_t>(Asset.Id.Value)),
            InnerAssetId);
    }

    // 0x7cef7b4 - Debug text naming the asset and the inner id (wording is this project's
    // own).
    std::string ToString() const;

    // 0x7cef990 (FromPrototypeAsset(EntityPrototype)) - A stand-alone prototype asset. The
    // original throws NullReferenceException for null; this throws std::invalid_argument.
    static EntityPrototypeRef FromPrototypeAsset(const EntityPrototype* prototype);
    // 0x7cef9a8 (FromPrototypeAsset(AssetRefEntityPrototype))
    static constexpr EntityPrototypeRef FromPrototypeAsset(AssetRefEntityPrototype prototype) {
        return EntityPrototypeRef{AssetRef{prototype.Id}, -1, 0};
    }
    // 0x7cef9b0 - Entry `internalAssetId` of a map's entity prototypes. Null throws as
    // above.
    static EntityPrototypeRef FromMasterAsset(const Map* asset, int internalAssetId);
};

static_assert(sizeof(EntityPrototypeRef) == EntityPrototypeRef::SIZE);

} // namespace Quantum
