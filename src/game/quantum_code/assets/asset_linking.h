// asset_linking.h - resolving the asset references stored in a gameplay asset while it is
// loaded (used by the Loaded overrides; new helper code).
//
// The original's Loaded overrides fetch each referenced asset with
// `(T)resourceManager.GetAsset(guid, true)`: a GUID naming no asset gives null, an asset of
// another type throws (InvalidCastException; std::bad_cast here), and a missing manager
// throws (NullReferenceException; std::invalid_argument here).
#pragma once

#include "quantum_core/asset_object.h"
#include "quantum_core/resource_manager.h"

#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace Quantum {

template <class T> T* LinkAsset(IResourceManager* resourceManager, AssetGuid guid) {
    if (resourceManager == nullptr) throw std::invalid_argument("asset references need a resource manager");
    const auto asset = resourceManager->GetAsset(guid, true);
    if (!asset) return nullptr;
    return &dynamic_cast<T&>(*asset);
}

// One resolved asset per reference, in order.
template <class T, class Ref>
std::vector<T*> LinkAssets(IResourceManager* resourceManager, const std::vector<Ref>& refs) {
    std::vector<T*> assets;
    assets.reserve(refs.size());
    for (const Ref& ref : refs) assets.push_back(LinkAsset<T>(resourceManager, ref.Id));
    return assets;
}

} // namespace Quantum
