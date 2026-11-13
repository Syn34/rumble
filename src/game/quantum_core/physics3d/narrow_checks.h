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

namespace LinecastBody {
// 0x7d782c8 - segment start->end against a sphere or (oriented) box entry. On a hit fills
// the point, the fraction along the segment, the collider identity and, if asked, the
// surface normal.
bool Check(AssetGuid mapGuid, const NarrowEntry3D* b, const FPVector3& start, const FPVector3& end,
           bool computeNormal, Hit3D& hit);
// 0x7d78710 - first point where start->end enters the sphere (a segment starting inside
// misses); outputs zeroed on a miss.
bool RaySphereProjection(const FPVector3& center, FP radius, const FPVector3& start,
                         const FPVector3& end, FPVector3& intersection, FP& normalizedDist);
// 0x7d7889c - ray from `origin` along unit `dir` against the box [minB, maxB] (Woo's
// algorithm), within maxDistance (raw). A ray starting inside misses (coord = origin).
// The normal is the axis of the entry face pointing back towards the origin.
bool HitBoundingBox(const FPVector3& minB, const FPVector3& maxB, const FPVector3& origin,
                    const FPVector3& dir, FPVector3& coord, FP::Raw maxDistanceRaw,
                    FPVector3& normal, FP& normalizedDist);
} // namespace LinecastBody

// Not implemented yet (CheckShapes asserts on these pairs and reports no collision). Both
// need NarrowData3D's box edge/plane caches (NarrowData3D.Init).
// TODO(0x7d70eec, Quantum.Physics3D.NarrowChecks.BoxBox.Check): SAT over face and edge axes.
// TODO(0x7d7293c, Quantum.Physics3D.NarrowChecks.BoxBox.get_allowedDotDiff)
// TODO(0x7d72944, Quantum.Physics3D.NarrowChecks.BoxBox.get_extentsExpansionMultiplier)
// TODO(0x7d72a28, Quantum.Physics3D.NarrowChecks.BoxBox.GetEdgesBox)
// TODO(0x7d72950, Quantum.Physics3D.NarrowChecks.BoxBox.GetPlanesBox)
// TODO(0x7d72d44, Quantum.Physics3D.NarrowChecks.BoxBox.GetContactPoints)
// TODO(0x7d72ec4, Quantum.Physics3D.NarrowChecks.BoxBox.GetContactPointPlaneEdge)
// TODO(0x7d75a6c, Quantum.Physics3D.NarrowChecks.BoxBox.ClipToPlane)
// TODO(0x7d75878, Quantum.Physics3D.NarrowChecks.BoxBox.BoxInAABB)
// TODO(0x7d75bac, Quantum.Physics3D.NarrowChecks.BoxBox.PointInAABB)
// TODO(0x7d72058, Quantum.Physics3D.NarrowChecks.BoxTriangle.Check): Akenine-Moller
// box/triangle SAT with edge clipping for the contacts.
// TODO(0x7d75ca4, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestX01)
// TODO(0x7d75edc, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestX2)
// TODO(0x7d75d34, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestY02)
// TODO(0x7d75f6c, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestY1)
// TODO(0x7d75dcc, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestZ12)
// TODO(0x7d75e54, Quantum.Physics3D.NarrowChecks.BoxTriangle.AxisTestZ0)
// TODO(0x7d76000, Quantum.Physics3D.NarrowChecks.BoxTriangle.PlaneBoxOverlaps)
// TODO(0x7d763d0, Quantum.Physics3D.NarrowChecks.BoxTriangle.ClipTriEdgesOnBoxPlanes)
// TODO(0x7d77624, Quantum.Physics3D.NarrowChecks.BoxTriangle.ClipBoxEdgesOnTriPlane)
// TODO(0x7d77f18, Quantum.Physics3D.NarrowChecks.BoxTriangle.ClipToPlane)
// TODO(0x7d76368, Quantum.Physics3D.NarrowChecks.BoxTriangle.PointInAabb)
// TODO(0x7d77fd4, Quantum.Physics3D.NarrowChecks.BoxTriangle.PointInTriangle)
// TODO(0x7d7619c, Quantum.Physics3D.NarrowChecks.BoxTriangle.ClosestPointOnTriangle)
// TODO(0x7d78180, Quantum.Physics3D.NarrowChecks.BoxTriangle.CheckForEdge)
// TODO(0x7d762bc, Quantum.Physics3D.NarrowChecks.BoxTriangle.PointPenetrationRaw)

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
