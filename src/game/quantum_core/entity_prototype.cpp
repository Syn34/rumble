// entity_prototype.cpp - the small value types around entity prototypes:
// EntityPrototypeRef, AssetRefEntityPrototype, MapEntityId, MapEntityLink and View
// (quantum.core.dll).
#include "quantum_core/asset_ref_entity_prototype.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/entity_prototype_ref.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/map.h"
#include "quantum_core/map_entity_id.h"
#include "quantum_core/view.h"

#include <stdexcept>

namespace Quantum {

std::string EntityPrototypeRef::ToString() const {
    return "EntityPrototypeRef(asset " + Asset.Id.ToString(false) + ", inner id " +
           std::to_string(InnerAssetId) + ")";
}

EntityPrototypeRef EntityPrototypeRef::FromPrototypeAsset(const EntityPrototype* prototype) {
    if (prototype == nullptr) {
        throw std::invalid_argument("EntityPrototypeRef: no prototype asset");
    }
    return EntityPrototypeRef{AssetRef{prototype->Guid}, -1, 0};
}

EntityPrototypeRef EntityPrototypeRef::FromMasterAsset(const Map* asset, int internalAssetId) {
    if (asset == nullptr) {
        throw std::invalid_argument("EntityPrototypeRef: no master asset");
    }
    return EntityPrototypeRef{AssetRef{asset->Guid}, internalAssetId, 0};
}

AssetRefEntityPrototype AssetRefEntityPrototype::FromAsset(const EntityPrototype* asset) {
    return AssetRefEntityPrototype{asset != nullptr ? asset->Guid : AssetGuid{}};
}

void MapEntityId::Serialize(void* ptr, FrameSerializer* serializer) {
    serializer->Stream().Serialize(&static_cast<MapEntityId*>(ptr)->SceneIndexPlusOne);
}

void MapEntityLink::Serialize(void* ptr, FrameSerializer* serializer) {
    serializer->Stream().Serialize(&static_cast<MapEntityLink*>(ptr)->Index);
}

void View::Serialize(void* ptr, FrameSerializer* serializer) {
    AssetRefEntityView::Serialize(&static_cast<View*>(ptr)->Current, serializer);
}

} // namespace Quantum
