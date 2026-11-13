// entity_prototype_loader.h - builds entity prototype and map assets together with their
// component prototypes (PC-side glue; new code).
#pragma once

namespace rr::content {

// Registers the loaders for EntityPrototypeAsset (component prototypes from the prefab the
// asset belongs to) and MapAsset (baked map entities). Called by
// RegisterBuiltInAssetFactories.
void RegisterPrefabAssetFactories();

} // namespace rr::content
