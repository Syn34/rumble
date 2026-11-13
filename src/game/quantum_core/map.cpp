// Original: Quantum.Map (quantum.core.dll).
#include "map.h"

#include "photon_deterministic/log.h"
#include "quantum_core/resource_manager.h"

#include <algorithm>
#include <stdexcept>
#include <typeinfo>

namespace Quantum {

using Photon::Deterministic::Log;

FPVector2 Map::WorldOffset() const { return FPVector2(-WorldExtentX(), -WorldExtentY()); }

void Map::EnsurePhysicsWorldSizeInValidRange() {
    if (WorldSize < 1) {
        WorldSize = 1;
        return;
    }
    const bool sameAxis = static_cast<std::uint8_t>(BucketingAxis) ==
                          static_cast<std::uint8_t>(SortingAxis);
    const std::int32_t limit = sameAxis ? 0x3FFF : 0x7FFF;
    if (WorldSize > limit) {
        WorldSize = limit;
    }
}

void Map::Loaded(IResourceManager* resourceManager, Allocator* /*allocator*/) {
    EnsurePhysicsWorldSizeInValidRange();
    BucketsCount = std::max(1, BucketsCount);
    BucketsSubdivisions = std::max(1, BucketsSubdivisions);
    TriangleMeshCellSize = std::max(1, TriangleMeshCellSize);

    // (StaticColliders2D: radius and native buffers of each polygon collider - missing.)

    const AssetGuid dataGuid = StaticColliders3DTrianglesData.Id;
    if (dataGuid.IsValid()) {
        const std::shared_ptr<AssetObject> asset = resourceManager->GetAsset(dataGuid, true);
        if (asset == nullptr) {
            Log::Warn("Map {0}: static triangle data {1} not found", Path, dataGuid.ToString(true));
        } else {
            if (dynamic_cast<const BinaryData*>(asset.get()) == nullptr) {
                throw std::bad_cast();
            }
            // (Reading the triangles - SerializeStaticColliderTriangles - is missing.)
            if (!dataGuid.IsDynamic()) {
                resourceManager->DisposeAsset(dataGuid);
            }
        }
    }
    InitNavMeshes(resourceManager);
}

void Map::InitNavMeshes(IResourceManager* resourceManager) {
    RegionMap.clear();
    // The original counts with a byte; maps have far fewer than 256 regions.
    for (std::size_t index = 0; index < Regions.size(); ++index) {
        if (!RegionMap.emplace(Regions[index], static_cast<std::int32_t>(index)).second) {
            throw std::invalid_argument("navmesh region name used twice: " + Regions[index]);
        }
        if (index >= 64) {
            Log::Error("Navmesh region {0} is past the 64 supported regions", Regions[index]);
        }
    }

    NavMeshes.clear();
    for (const AssetRefNavMesh& link : NavMeshLinks) {
        const std::shared_ptr<AssetObject> asset = resourceManager->GetAsset(link.Id, true);
        if (asset == nullptr) {
            throw std::invalid_argument("navmesh " + link.Id.ToString(true) + " not found");
        }
        auto navmesh = std::dynamic_pointer_cast<NavMesh>(asset);
        if (navmesh == nullptr) {
            throw std::bad_cast();
        }
        const std::string name = navmesh->Name;
        if (!NavMeshes.emplace(name, std::move(navmesh)).second) {
            throw std::invalid_argument("navmesh name used twice: " + name);
        }
    }
}

NavMesh* Map::GetNavMesh(const std::string& name) const {
    const auto found = NavMeshes.find(name);
    return found != NavMeshes.end() ? found->second.get() : nullptr;
}

} // namespace Quantum
