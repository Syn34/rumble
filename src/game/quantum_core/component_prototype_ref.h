// component_prototype_ref.h - Quantum.ComponentPrototypeRef: names one component
// prototype inside an entity prototype asset (asset, entity inside the asset, component
// type index).
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/entity_prototype_ref.h"

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct ComponentPrototypeRef {
    static constexpr int SIZE = 16;

    AssetRef Asset;
    std::int32_t InnerAssetId = 0;
    std::int32_t ComponentTypeIndex = 0;

    // 0x7ceb178 get_EntityPrototypeRef - The entity prototype part (asset and inner id).
    EntityPrototypeRef get_EntityPrototypeRef() const {
        EntityPrototypeRef entity;
        entity.Asset = Asset;
        entity.InnerAssetId = InnerAssetId;
        return entity;
    }

    // 0x7ceb188 (Equals(ComponentPrototypeRef)); 0x7ceb1bc is the boxed Equals(object).
    bool Equals(const ComponentPrototypeRef& other) const {
        return Asset == other.Asset && InnerAssetId == other.InnerAssetId &&
               ComponentTypeIndex == other.ComponentTypeIndex;
    }
    friend bool operator==(const ComponentPrototypeRef& a, const ComponentPrototypeRef& b) {
        return a.Equals(b);
    }

    // 0x7ceb26c - Asset hash, then InnerAssetId and ComponentTypeIndex combined as
    // h = (h * 33) ^ value.
    std::int32_t GetHashCode() const;

    // 0x7ceb3cc - Debug text (wording is this project's own).
    std::string ToString() const;

    // 0x7ceb298 - The asset GUID (64-bit), then the two indices.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(ComponentPrototypeRef) == ComponentPrototypeRef::SIZE);

} // namespace Quantum
