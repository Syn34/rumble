// Methods of the level assets (quantum.code.dll).
#include "quantum_code/assets/level_assets.h"

#include <stdexcept>
#include <string>

namespace Quantum {

void MapCustomData::Loaded(IResourceManager* resourceManager, Allocator* allocator) {
    AssetObject::Loaded(resourceManager, allocator);
    _staticColliderToGrindRailIndex.clear();
    for (const StaticColliderGrindRailPair& pair : staticColliderGrindRailPairs) {
        if (!_staticColliderToGrindRailIndex.emplace(pair.staticColliderIndex, pair.grindRailIndex).second)
            throw std::invalid_argument("static collider " + std::to_string(pair.staticColliderIndex) +
                                        " is listed for two grind rails");
    }
}

bool MapCustomData::TryGetStaticColliderToGrindRailIndex(std::int32_t staticColliderIndex,
                                                         std::int32_t& grindRailIndex) const {
    const auto it = _staticColliderToGrindRailIndex.find(staticColliderIndex);
    grindRailIndex = it != _staticColliderToGrindRailIndex.end() ? it->second : 0;
    return it != _staticColliderToGrindRailIndex.end();
}

} // namespace Quantum
