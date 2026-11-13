// navmesh.h - Quantum.NavMesh and Quantum.AssetRefNavMesh (quantum.core.dll).
//
// A navmesh asset: triangles over the XZ plane plus three square grids that index them
// (triangles per cell, borders per cell, triangle centres per cell). All grids share
// the same layout: GridSizeX x GridSizeY cells of GridNodeSize units, centred on the
// origin; cell index = z * GridSizeX + x.
//
// Data format (what the asset pipeline must supply):
//   * The NavMesh asset object itself carries Name, DataAsset (a BinaryData asset
//     GUID), GridSizeX, GridSizeY, GridNodeSize, WorldOffset and MinAgentRadius. Those
//     come from the asset database (they are not in the binary blob).
//   * The BinaryData asset holds the bulk data, gzip-compressed when its IsCompressed
//     flag is set. After decompression it is a ByteStream (little-endian, no padding):
//       Triangles      int count, then per triangle  NavMeshTriangle::Serialize
//       TrianglesGrid  int count, then per cell      int count + int indices
//       Vertices       int count, then per vertex    NavMeshVertex::Serialize
//       BorderGrid     int count, then per cell      int count + int indices
//       TrianglesCenterGrid  int count, then per cell int count + int indices
//       Borders        int count, then per border    NavMeshBorder::Serialize
//       Links          int count (< 32768), then per link NavMeshLink::Serialize
//     FP values are raw 48.16 longs, vectors are X, Y, Z.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/asset_guid.h"
#include "quantum_core/navigation/navmesh_region_mask.h"
#include "quantum_core/navigation/navmesh_types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Photon::Deterministic {
struct RNGSession;
}

namespace Quantum {

using Photon::Deterministic::FPVector2;

class ByteStream;
class MailboxSet8;

// The part of a BinaryData asset NavMesh::Loaded uses.
struct NavMeshBinaryData {
    std::vector<std::uint8_t> Data;
    bool IsCompressed = false;
};

class NavMesh {
  public:
    // AssetObject part (Quantum.AssetObject.Identifier): the asset's GUID and path.
    AssetGuid Guid;
    std::string Path;

    std::string Name;
    AssetGuid DataAsset; // AssetRefBinaryData
    std::string DataFilepath;
    std::int32_t GridSizeX = 0;
    std::int32_t GridSizeY = 0;
    std::int32_t GridNodeSize = 0;
    FPVector2 WorldOffset;
    FP MinAgentRadius;
    std::vector<NavMeshTriangle> Triangles;
    std::vector<NavMeshTriangleNode> TrianglesGrid;
    std::vector<NavMeshVertex> Vertices;
    std::vector<NavMeshBorderNode> BorderGrid;
    std::vector<NavMeshTriangleCenterGridNode> TrianglesCenterGrid;
    std::vector<NavMeshBorder> Borders;
    std::vector<NavMeshLink> Links;

    // 0x7ce5014
    NavMesh() = default;

    // TODO(0x7ce2774, Quantum.NavMesh.Quantum.IBuiltInAssetObject.Dispatch): visitor
    // double dispatch of the asset system; needs IAssetObjectVisitor.

    // 0x7ce281c
    int BorderCount() const { return static_cast<int>(Borders.size()); }

    // 0x7ce2834 - called after the asset is loaded. The original looks the DataAsset GUID
    // up in the resource manager; the caller does that and passes the result (nullptr if
    // missing). Missing data logs a warning; a broken blob logs an error.
    void Loaded(const NavMeshBinaryData* dataAsset);

    // 0x7ce3164 - grid cell index of an XZ position (not clamped).
    int CalculateNodeGridIndex(FP positionX, FP positionZ) const;
    // 0x7ce31b4 - grid cell coordinates of an XZ position (not clamped).
    void CalculateNodeGridPosition(FP positionX, FP positionZ, int& x, int& z) const;
    // 0x7ce320c - clamps X/Z into the grid's extent.
    void ClampToGrid(FPVector3& position) const;

    // 0x7ce32bc - height (raw FP) of the triangle plane through v0 with `normal`, at p's
    // X/Z; p.Y when the plane is vertical.
    static std::int64_t GetHeightAtTriangleRaw(const FPVector3& p, const FPVector3& normal,
                                               const FPVector3& v0);

    // TODO(0x7ce331c, Quantum.NavMesh.FindClosestTriangle(FrameBase,...)): fetches the
    // navigation thread-local mailbox of thread 0 from the frame context, sizes it for this
    // navmesh and calls the overload below; needs FrameBase.

    // 0x7ce3474 - triangle under/near `sourcePosition`: first one containing the point (XZ)
    // whose height is within `radius` (any height when radius is 0), lowest height
    // difference wins; otherwise the nearest triangle within roughly `radius` (searched
    // ring by ring through the triangle grid). Returns false if none was found.
    bool FindClosestTriangle(const FPVector3& sourcePosition, FP radius,
                             NavMeshRegionMask regionMask, int& triangle,
                             FPVector3& closestPosition, MailboxSet8* mailbox) const;

    // 0x7ce3a78 - triangle containing the point (XZ) in its grid cell, or -1.
    int FindTriangle2D(const FPVector3& position, NavMeshRegionMask regionMask,
                       bool inclusive) const;

    // 0x7ce3dc0 - uniform random point in a triangle (false and zero for a bad index).
    bool FindRandomPointOnTriangle(int triangle, Photon::Deterministic::RNGSession* rngSession,
                                   FPVector3& result) const;

    // 0x7ce3f94 - random point in a disc around `position`, snapped onto the navmesh.
    bool FindRandomPointOnNavmesh(const FPVector3& position, FP radius,
                                  Photon::Deterministic::RNGSession* rngSession,
                                  NavMeshRegionMask regionMask, FPVector3& result) const;

    // 0x7ce41c0 - a border blocks agents of `regionMask` if they may use its triangle
    // and may not cross to the other side.
    bool IsBorderActive(int borderIndex, NavMeshRegionMask regionMask) const;

    // 0x7ce42c0
    bool Contains(const FPVector3& position, NavMeshRegionMask regionMask, bool inclusive) const;

    // 0x7ce4314 - walks the triangle strip from start to destination (searched with
    // FindClosestTriangle when an index is negative). With `considerCost` it only walks
    // through triangles of the start triangle's cost.
    bool LineOfSight(const FPVector3& positionStart, const FPVector3& positionDestination,
                     NavMeshRegionMask regionMask, int triangleStart, int triangleDestination,
                     bool considerCost) const;

    // 0x7ce48f4 - first active border hit by the ray (XZ, as FPVector2 X/Y).
    bool Raycast2D(FPVector2 origin, FPVector2 direction, FP distance, NavMeshRegionMask regionMask,
                   MailboxSet8* mailbox, FPVector2& hit) const;

    // 0x7ce4c18 - if `newPosition` is off the mesh, pulls it towards the closest active
    // border point (fully when correction >= 1, else lerped by `correction`).
    FPVector2 MovePositionIntoNavmesh(FPVector2 position, FPVector2 newPosition, FP correction,
                                      NavMeshRegionMask regionMask, MailboxSet8* mailbox) const;

    // 0x7ce2d30 - the binary blob (see the header comment).
    void Serialize(ByteStream& stream, bool write);
};

// Typed asset reference. Original: Quantum.AssetRefNavMesh.
struct AssetRefNavMesh {
    static constexpr int SIZE = 8;

    AssetGuid Id;

    // 0x7ce265c - the navmesh's GUID (invalid for null).
    static AssetRefNavMesh FromAsset(const NavMesh* value) {
        return AssetRefNavMesh{value != nullptr ? value->Guid : AssetGuid{}};
    }

    // 0x7ce2670 (Equals(AssetRefNavMesh)); 0x7ce2680 Equals(object) is the boxed form.
    friend constexpr bool operator==(AssetRefNavMesh, AssetRefNavMesh) = default;

    // 0x7ce26f8
    std::int32_t GetHashCode() const { return Id.GetHashCode(); }
    // 0x7ce2668
    std::string ToString() const { return AssetRef{Id}.ToString(); }

    // TODO(0x7ce2700, Quantum.AssetRefNavMeshExt.FindAsset): frame.FindAsset<NavMesh>(Id);
    // needs FrameBase and the asset database.
};
static_assert(sizeof(AssetRefNavMesh) == AssetRefNavMesh::SIZE);

} // namespace Quantum
