// map.h - Quantum.Map and Quantum.AssetRefMap (quantum.core.dll).
//
// The map asset: everything the simulation knows about a level besides its entities'
// state - physics world size and broadphase bucketing, the static colliders, the baked
// static mesh triangles (in a BinaryData asset), the navmeshes (linked by reference and
// resolved by name once loaded), the navmesh region names and the map's entity
// prototypes.
#pragma once

#include "photon_deterministic/fp_vector2.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/triangle_ccw.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Quantum {

class Map final : public AssetObject, public IBuiltInAssetObject {
  public:
    // Map.BucketAxis / Map.SortAxis: broadphase bucketing and sorting directions.
    enum class BucketAxis : std::uint8_t { X = 0, Y = 1 };
    enum class SortAxis : std::uint8_t { X = 0, Y = 1 };

    AssetRef UserAsset;
    std::string Scene;
    std::string ScenePath;
    std::string SceneGuid;
    std::int32_t WorldSize = 256;
    std::int32_t BucketsCount = 16;
    std::int32_t BucketsSubdivisions = 8;
    BucketAxis BucketingAxis = BucketAxis::X;
    SortAxis SortingAxis = SortAxis::Y;
    std::int32_t TriangleMeshCellSize = 4;
    AssetRefBinaryData StaticColliders3DTrianglesData;
    bool OptionalTriangleMetadataSerializationEnabled = false;
    bool SerializeTrianglesMetadata = false;
    std::string StaticColliders3DTrianglesBinaryFile;
    std::int32_t GridSizeX = 0;
    std::int32_t GridSizeY = 0;
    std::int32_t GridNodeSize = 0;
    std::vector<AssetRefNavMesh> NavMeshLinks;
    std::vector<std::string> Regions;
    // Not reproduced yet: StaticColliders2D (MapStaticCollider2D[]), StaticColliders3D
    // (MapStaticCollider3D[]) and MapEntities (EntityPrototypeContainer[]); they belong to
    // the physics and prototype systems.

    // Filled by Loaded.
    std::map<std::int32_t, std::vector<TriangleCCW>> StaticColliders3DTriangles;
    std::unordered_map<std::string, std::shared_ptr<NavMesh>> NavMeshes;
    std::unordered_map<std::string, std::int32_t> RegionMap;

    // 0x7ce2630
    Map() = default;

    // 0x7ce1644
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }

    // 0x7ce16ec - half the navigation grid's width.
    std::int32_t WorldExtentX() const { return HalfProduct(GridSizeX, GridNodeSize); }
    // 0x7ce1708 - half the navigation grid's height.
    std::int32_t WorldExtentY() const { return HalfProduct(GridSizeY, GridNodeSize); }
    // 0x7ce1720 - the grid's lower-left corner (it is centred on the origin).
    FPVector2 WorldOffset() const;

    // TODO(0x7ce1790, Quantum.Map.SerializeStaticColliderTriangles): reads or writes
    // StaticColliders3DTriangles to the BinaryData stream: entry count, then per entry the
    // key, the triangle count and each TriangleCCW (TriangleCCW.Serialize, metadata if
    // SerializeTrianglesMetadata - when reading only if
    // OptionalTriangleMetadataSerializationEnabled, otherwise always). Writing also sets
    // OptionalTriangleMetadataSerializationEnabled. Needs TriangleCCW.Serialize.

    // TODO(0x7ce1b84, Quantum.Map.Disposed): frees the native buffers of the polygon
    // colliders inside StaticColliders2D; needs MapStaticCollider2D.

    // TODO(0x7ce1c70, Quantum.Map.Loaded): the StaticColliders2D polygon setup (radius and
    // native buffers per polygon collider) and reading the triangles
    // (SerializeStaticColliderTriangles) are missing. Done: world size and bucket values
    // made valid, the triangle data asset fetched (a warning when missing) and released
    // afterwards, the navmeshes and regions resolved (InitNavMeshes).
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;

    // 0x7ce22b0 - RegionMap from Regions (name -> index; more than 64 regions logs an
    // error) and NavMeshes from NavMeshLinks (by navmesh name). A duplicate name or a
    // missing navmesh throws std::invalid_argument.
    void InitNavMeshes(IResourceManager* resourceManager);

    // 0x7ce25b8 - the navmesh of that name, or null.
    NavMesh* GetNavMesh(const std::string& name) const;

    // 0x7ce218c - WorldSize at least 1 and below the limit of the broadphase: 0x3FFF when
    // bucketing and sorting use the same axis, 0x7FFF otherwise.
    void EnsurePhysicsWorldSizeInValidRange();

  private:
    // a * b / 2 with the original's 32-bit wrap-around multiplication.
    static std::int32_t HalfProduct(std::int32_t a, std::int32_t b) {
        return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) *
                                         static_cast<std::uint32_t>(b)) /
               2;
    }
};

// 0x7ce1584 op_Implicit, 0x7ce1590 Serialize, 0x7ce15a0 ToString, 0x7ce15a8 / 0x7ce15b8
// Equals, 0x7ce1630 GetHashCode, 0x7ce1638 ==
using AssetRefMap = TypedAssetRef<Map>;

} // namespace Quantum
