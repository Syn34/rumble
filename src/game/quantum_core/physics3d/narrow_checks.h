// narrow_checks.h - Quantum.Physics3D.NarrowChecks and its nested shape-pair checks.
//
// Same conventions as the 2D checks: Check(mInfo, detailedManifold) returns whether the
// pair overlaps and, for a detailed manifold, fills normal (A towards B), penetration and
// contact points. The C# nested classes are namespaces here.
#pragma once

#include "collision_manifold3d.h"
#include "gjk3d.h"

namespace Quantum {
struct CCDSettings;
struct TriangleMesh; // the scene's static mesh (not reimplemented yet)
} // namespace Quantum

namespace Quantum::Physics3D::NarrowChecks {

// 0x7d7071c - dispatches on the pair's shape types (sphere, box, mesh triangle; inverting
// pairs written for the other order). When the shapes don't overlap now but the manifold
// uses CCD, a time-of-impact query over `deltaTime` decides instead. `settings` and
// `deltaTime` are the scene's CCD settings and time step; `tls` the thread's scratch data.
bool Check(NarrowTls3D& tls, const CCDSettings& settings, FP deltaTime,
           CollisionManifoldInfo3D* mInfo, bool detailedManifold);

// The shape dispatch of Check without the CCD fallback.
bool CheckShapes(CollisionManifoldInfo3D* mInfo, bool detailedManifold);

namespace SphereSphere {
// 0x7d790cc
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold);
} // namespace SphereSphere

namespace SphereBox {
// 0x7d78bec - A is the sphere, B the box. The centre is clamped into the box (in the box's
// axes); a centre inside the box is pushed out through the nearest face.
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold);
} // namespace SphereBox

namespace SphereTriangle {
// 0x7d79290 - A is the sphere, mInfo->Tri the triangle. With smooth collision the contact
// is the centre projected onto the triangle's plane instead of the nearest point.
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold, bool smoothSphereMeshCollision);
// 0x7d79418 - the point of the triangle nearest to `center` when the sphere (radius in
// raw units) reaches the triangle's plane; also the centre projected onto that plane.
bool GetNearestPoint(const TriangleCCW* tri, const FPVector3& center, FP::Raw radiusRaw,
                     FPVector3& nearestPoint, FPVector3& pointOnPlane);
// 0x7d796d8 - if `projected` lies outside the edge a->b (unit `direction`, `length`), the
// nearest point on that edge and whether it is within the radius; no value otherwise.
std::optional<bool> CheckForVertex(const TriangleCCW* tri, const FPVector3& projected,
                                   const FPVector3& a, const FPVector3& b,
                                   const FPVector3& direction, FP length, const FPVector3& p,
                                   FP::Raw radiusSqr, FPVector3& nearestPoint);
} // namespace SphereTriangle

// Not implemented yet (CheckShapes asserts on these pairs and reports no collision):
// TODO(0x7d70eec, Quantum.Physics3D.NarrowChecks.BoxBox.Check): SAT over face and edge axes
// with the box edge/plane caches of NarrowData3D; also BoxBox.get_allowedDotDiff (0x7d7293c),
// get_extentsExpansionMultiplier (0x7d72944), GetEdgesBox (0x7d72a28), GetPlanesBox
// (0x7d72950), GetContactPoints (0x7d72d44), GetContactPointPlaneEdge (0x7d72ec4),
// ClipToPlane (0x7d75a6c), BoxInAABB (0x7d75878), PointInAABB (0x7d75bac).
// TODO(0x7d72058, Quantum.Physics3D.NarrowChecks.BoxTriangle.Check): Akenine-Moller
// box/triangle SAT (AxisTestX01 0x7d75ca4, AxisTestX2 0x7d75edc, AxisTestY02 0x7d75d34,
// AxisTestY1 0x7d75f6c, AxisTestZ12 0x7d75dcc, AxisTestZ0 0x7d75e54, PlaneBoxOverlaps
// 0x7d76000) with edge clipping for contacts (ClipTriEdgesOnBoxPlanes 0x7d763d0,
// ClipBoxEdgesOnTriPlane 0x7d77624, ClipToPlane 0x7d77f18, PointInAabb 0x7d76368,
// PointInTriangle 0x7d77fd4, ClosestPointOnTriangle 0x7d7619c, CheckForEdge 0x7d78180,
// PointPenetrationRaw 0x7d762bc).

// Walks the per-triangle manifolds of a sphere/box-against-mesh collision.
struct TriangleCollisionIterator {
    CollisionManifold3D* _meshManifold = nullptr;
    CollisionManifold3D* _triManifolds = nullptr;
    CollisionManifold3D* _currentTri = nullptr;
    const TriangleMesh* _sceneMesh = nullptr;

    // 0x7d3a7a4
    TriangleCollisionIterator(const TriangleMesh* sceneMesh, CollisionManifold3D* meshManifold,
                              CollisionManifold3D* triManifolds)
        : _meshManifold(meshManifold), _triManifolds(triManifolds), _currentTri(meshManifold),
          _sceneMesh(sceneMesh) {}

    // 0x7d3a898 - the negated mean of the triangle manifolds' normals (chained through
    // NextInMesh starting at the mesh manifold; raw sums divided by the count).
    FPVector3 AverageNormal() const;
};

} // namespace Quantum::Physics3D::NarrowChecks
