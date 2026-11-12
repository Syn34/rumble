// polygon_collider.h - the data of Quantum.PolygonCollider, the asset that describes a
// convex polygon shape (vertices in local space, counter-clockwise, with precomputed
// edge normals, bounding radius and centroid).
//
// Only the fields the collision code reads are reproduced here. The asset-object side
// (Loaded/Disposed, native buffer management, visitor dispatch) belongs with the asset
// database and is not implemented yet:
// TODO(0x7ce5748, Quantum.PolygonCollider.Loaded): copies Vertices/Normals into native
// buffers via InitializeNativeBuffers (0x7ce21e8); needs the allocator and asset system.
#pragma once

#include "asset_guid.h"
#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

#include <vector>

namespace Quantum {

struct PolygonCollider {
    AssetGuid Guid; // AssetObject.Identifier.Guid
    std::vector<Photon::Deterministic::FPVector2> Vertices;
    std::vector<Photon::Deterministic::FPVector2> Normals;
    Photon::Deterministic::FP Radius;
    Photon::Deterministic::FPVector2 Centroid;
};

} // namespace Quantum
