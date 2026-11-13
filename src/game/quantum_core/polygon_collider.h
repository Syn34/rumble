// polygon_collider.h - Quantum.PolygonCollider and Quantum.AssetRefPolygonCollider
// (quantum.core.dll): the asset that describes a convex polygon shape (vertices in local
// space, counter-clockwise, with precomputed edge normals, bounding radius and centroid).
//
// The collision code reads Vertices/Normals. The original also keeps copies in native
// memory (VerticesBuffer/NormalsBuffer) for its unmanaged shape code; they are made when a
// resource manager loads the asset and released when it disposes it.
#pragma once

#include "asset_guid.h"
#include "asset_object.h"
#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

#include <vector>

namespace Quantum {

class PolygonCollider : public AssetObject, public IBuiltInAssetObject {
  public:
    std::vector<Photon::Deterministic::FPVector2> Vertices;
    std::vector<Photon::Deterministic::FPVector2> Normals;
    Photon::Deterministic::FP Radius;
    Photon::Deterministic::FPVector2 Centroid;
    Photon::Deterministic::FPVector2* VerticesBuffer = nullptr;
    Photon::Deterministic::FPVector2* NormalsBuffer = nullptr;

    // 0x7ce57ec
    PolygonCollider() = default;
    // Copies share no native buffers.
    PolygonCollider(const PolygonCollider& other);
    PolygonCollider& operator=(const PolygonCollider& other);

    // 0x7ce56a0
    void Dispatch(IAssetObjectVisitor& visitor) override { visitor.Visit(*this); }

    // 0x7ce5748 - puts the vertices in counter-clockwise order, derives radius, normals and
    // centroid, and (when loaded by a resource manager) makes the native copies.
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
    // 0x7ce57dc - releases the native copies (when disposed by a resource manager).
    void Disposed(IResourceManager* resourceManager, Allocator* allocator) override;

    // 0x7ce21e8 - native copies of Vertices and Normals, unless they exist already.
    void InitializeNativeBuffers(Allocator* allocator);
    // 0x7ce1c04
    void FreeNativeBuffers(Allocator* allocator);
};

// 0x7ce5578 op_Implicit, 0x7ce5584 Serialize, 0x7ce5594 ToString, 0x7ce559c / 0x7ce55ac
// Equals, 0x7ce5624 GetHashCode, 0x7ce562c Ext.FindAsset
using AssetRefPolygonCollider = TypedAssetRef<PolygonCollider>;

} // namespace Quantum
