// navmesh_types.h - the baked data a Quantum navmesh is made of (quantum.core.dll):
// NavMeshVertex, NavMeshPortal, NavMeshTriangle, NavMeshBorder, NavMeshLink and the
// three spatial grid node types.
//
// The data is produced by the editor-side baker and stored in a BinaryData asset;
// NavMesh::Serialize reads it back (see navmesh.h for the stream layout). All positions
// are in world space; the navmesh lives in the XZ plane with Y up.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/navigation/navmesh_region_mask.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;

class ByteStream;
class NavMesh;

// A connection from a triangle to a neighbour across one shared edge. Left/right are
// the edge's vertex indices as seen when walking from the triangle into the neighbour.
struct NavMeshPortal {
    std::int32_t LeftVertex = 0;
    std::int32_t RightVertex = 0;
    std::int32_t Neighbor = 0; // triangle index

    // 0x7d2b5d8 - LeftVertex, RightVertex, Neighbor.
    void Serialize(ByteStream& stream, bool write);
};

struct NavMeshVertex {
    FPVector3 Point;
    std::vector<std::int32_t> Borders; // borders that touch this vertex

    // 0x7d2b888 - Point, then the border list.
    void Serialize(ByteStream& stream, bool write);

    // 0x7d27548 - average normal of the vertex's active borders, normalised (left as is
    // when its squared length is already within FP.Epsilon of 1). Zero without borders.
    static FPVector3 CalculateNormal(int vertexIndex, const NavMesh& navmesh,
                                     NavMeshRegionMask regionMask);
};

struct NavMeshTriangle {
    std::int32_t Vertex0 = 0;
    std::int32_t Vertex1 = 0;
    std::int32_t Vertex2 = 0;
    FPVector3 Center;
    FPVector3 Normal;
    std::vector<NavMeshPortal> Neighbors;
    std::vector<std::int32_t> Links;   // off-mesh links starting in this triangle
    std::vector<std::int32_t> Borders; // borders on this triangle's edges
    NavMeshRegionMask Regions;
    FP Cost; // path cost multiplier

    // 0x7d2b640 - Vertex0-2, Center, Normal, Neighbors (length + portals), Links,
    // Borders, Regions, Cost.
    void Serialize(ByteStream& stream, bool write);
};

// An outer edge of the walkable area (agents are kept inside these).
struct NavMeshBorder {
    FPVector3 V0;
    FPVector3 V1;
    FPVector3 Normal;                 // points into the walkable area
    NavMeshRegionMask Regions;        // regions of the triangle the border belongs to
    NavMeshRegionMask RegionsOffmesh; // regions on the other side of the border

    // 0x7d2b934 - V0, V1, Normal, then RegionsOffmesh BEFORE Regions (field order is
    // swapped in the stream).
    void Serialize(ByteStream& stream, bool write);
};

// Borders overlapping one cell of the border grid.
struct NavMeshBorderNode {
    std::vector<std::int32_t> Borders;

    // 0x7d2ba54
    void Serialize(ByteStream& stream, bool write);
};

// Triangles overlapping one cell of the triangle grid.
struct NavMeshTriangleNode {
    std::vector<std::int32_t> Triangles;

    // 0x7d2ba88
    void Serialize(ByteStream& stream, bool write);
};

// Triangles whose centre lies in one cell (a second grid; unused by the code here).
struct NavMeshTriangleCenterGridNode {
    std::vector<std::int32_t> Triangles;

    // 0x7d2bbe4
    void Serialize(ByteStream& stream, bool write);
};

// An off-mesh connection (jump, ladder...) from Start in Triangle to End.
struct NavMeshLink {
    FPVector3 Start;
    FPVector3 End;
    std::int32_t Triangle = 0; // triangle End lies in (the A* target of the link)
    FP CostOverride;           // negative: use the triangle cost
    NavMeshRegionMask Region;
    std::string Name;

    // 0x7d2babc - Start, End, Triangle, CostOverride, Region, Name (UTF-8 bytes).
    void Serialize(ByteStream& stream, bool write);
};

} // namespace Quantum
