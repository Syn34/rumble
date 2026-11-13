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
class TriangleMesh; // the scene's static mesh (triangle_mesh.h)
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
bool Check(AssetGuid mapGuid, const Entry3D* b, const FPVector3& start, const FPVector3& end,
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

namespace BoxBox {
// 0x7d7293c - largest |cos| between two box axes for which their cross product is still
// tested as a separating axis (raw 0xf851, about 0.97).
constexpr FP allowedDotDiff() { return FP::FromRaw(0xf851); }
// 0x7d72944 - box extents are grown by this factor (raw 0x1051e, about 1.02) when testing
// whether a contact candidate lies on the box.
constexpr FP extentsExpansionMultiplier() { return FP::FromRaw(0x1051e); }

// 0x7d70eec - separating-axis test over the 3 + 3 face axes and the 9 edge-pair axes
// (edge pairs skipped when nearly parallel). Contacts are where one box's edges cross the
// other's faces; failing that, the centre of a box lying entirely inside the other. With a
// detailed manifold the normal is the axis of least penetration, oriented from A to B.
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold);
// 0x7d72a28 - the edges moved by `translation`.
CustomEdgesBox GetEdgesBox(CustomEdgesBox edges, const FPVector3& translation);
// 0x7d72950 - the planes' points moved by `translation`.
CustomPlanesBox GetPlanesBox(CustomPlanesBox planes, const FPVector3& translation);
// 0x7d72d44 - contact generation; planes and edges are relative to box A's centre.
void GetContactPoints(const CustomPlanesBox& planesA, const CustomPlanesBox& planesB,
                      const CustomEdgesBox& edgesA, const CustomEdgesBox& edgesB,
                      const Entry3D* boxEntryA, const Entry3D* boxEntryB,
                      const FPVector3& boxAToBoxBOffset, CollisionManifold3D* manifold,
                      bool detailedManifold);
// 0x7d72ec4 - for every face plane of `boxEntry` (centre at `offset` from box A) and every
// edge of the other box: the crossing point, if it lies on `boxEntry` (slightly grown),
// becomes a contact (translated by `boxAPosition`). Stops after the first contact unless
// detailed, and once more than three are found.
void GetContactPointPlaneEdge(const CustomPlanesBox& planes, const CustomEdgesBox& edges,
                              const Entry3D* boxEntry, const FPVector3& offset,
                              const FPVector3& boxAPosition, CollisionManifold3D* manifold,
                              bool detailedManifold);
// 0x7d75a6c - where the segment crosses the plane; segments within raw 65 of parallel miss.
// `intersection` is only written on a hit.
bool ClipToPlane(const CustomPlane& plane, const CustomLine& line, FPVector3& intersection);
// 0x7d75878 - all eight corners (from the edges) inside the grown box.
bool BoxInAABB(const CustomEdgesBox& edges, const Entry3D* boxEntry, const FPVector3& offset);
// 0x7d75bac - `point` (relative to box A) inside the box with the given half-extents whose
// centre is at `offset`; `point` is replaced by its coordinates in the box's frame.
bool PointInAABB(FPVector3& point, const Entry3D* box, const FPVector3& extents,
                 const FPVector3& offset);
} // namespace BoxBox

namespace BoxTriangle {
// 0x7d72058 - A is the box, mInfo->Tri the mesh triangle. Akenine-Moller box/triangle
// overlap in the box's frame (box axes, the 9 edge cross axes, the triangle plane). With a
// detailed manifold the penetration is the smallest of the axis overlaps and the depth of
// the deepest box corner; contacts are the triangle's corners when it lies inside the
// (slightly grown) box, otherwise triangle edges clipped by box faces and box edges clipped
// by the triangle plane. The normal is the triangle's inverse normal.
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold);

// The edge cross-axis tests. Each projects two of the triangle's corners onto the axis
// edge x box-axis, compares with the box's projected radius and lowers `penetrationRaw` to
// the overlap. Returns true when the axis separates (overlap below raw -64); a zero axis
// never separates. `fa`/`fb` are the absolute edge components that weight the extents.
// 0x7d75ca4 - axis x, corners 0 and 2.
bool AxisTestX01(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                 FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d75edc - axis x, corners 0 and 1.
bool AxisTestX2(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d75d34 - axis y, corners 0 and 2.
bool AxisTestY02(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                 FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d75f6c - axis y, corners 0 and 1.
bool AxisTestY1(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d75dcc - axis z, corners 1 and 2.
bool AxisTestZ12(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                 FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d75e54 - axis z, corners 0 and 1.
bool AxisTestZ0(const CustomVertexTriangle& tri, const FPVector3& extents, const FPVector3& edge,
                FP fa, FP fb, FP::Raw& penetrationRaw);
// 0x7d76000 - whether the triangle's plane cuts the box. `deepestBoxVertex` is the box
// corner furthest along the normal; with a detailed manifold `deepestBoxVertexProj` is its
// projection onto the plane (zero otherwise).
bool PlaneBoxOverlaps(const FPVector3& triInverseNormal, const CustomVertexTriangle& tri,
                      const FPVector3& extents, bool detailedManifold, FPVector3& deepestBoxVertex,
                      FPVector3& deepestBoxVertexProj);
// 0x7d763d0 - triangle edges clipped by the box's local face planes; crossings inside
// `boxExtents` become (world space) contacts, at most four.
void ClipTriEdgesOnBoxPlanes(const Entry3D* boxEntry, const CustomVertexTriangle& tri,
                             const FPVector3& boxExtents, CollisionManifold3D* m);
// 0x7d77f18 - like BoxBox.ClipToPlane, but only segments within raw 6 of parallel miss.
bool ClipToPlane(const CustomPlane& plane, const CustomLine& line, FPVector3& intersection);
// 0x7d76368
bool PointInAabb(const FPVector3& point, const FPVector3& extents);
// 0x7d77624 - the box's local edges clipped by the triangle's plane; crossings inside the
// triangle become (world space) contacts, at most four.
void ClipBoxEdgesOnTriPlane(const Entry3D* boxEntry, const FPVector3& triNormal,
                            const CustomVertexTriangle& triVertex, CollisionManifold3D* m);
// 0x7d77fd4 - `p` (on the triangle's plane) inside the triangle, edges included.
bool PointInTriangle(const FPVector3& p, const CustomVertexTriangle& tri);
// 0x7d7619c - the point of the triangle nearest to `pointOnPlane`: on the first edge it
// lies outside of, or the point itself (pointIsInsideTri). `box` is unused.
FPVector3 ClosestPointOnTriangle(const TriangleCCW* tri, const FPVector3& pointOnPlane,
                                 const CustomVertexTriangle& customTri,
                                 const FPVector3& triInverseNormal, const Shape3D::BoxShape& box,
                                 bool& pointIsInsideTri);
// 0x7d78180 - if `pointOnPlane` is outside the edge a->b (of length edgeMagnitude), the
// nearest point of the edge; zero otherwise.
bool CheckForEdge(const FPVector3& pointOnPlane, const FPVector3& triInverseNormal,
                  const FPVector3& vertexA, const FPVector3& vertexB, FP edgeMagnitude,
                  FPVector3& closestPoint);
// 0x7d762bc - how far `point` can move along the normal before leaving the box: the
// smallest non-negative per-axis distance (raw; MaxValue for a zero normal).
FP::Raw PointPenetrationRaw(const FPVector3& point, const FPVector3& triInvertedNormal,
                            const FPVector3& boxExtents);
} // namespace BoxTriangle

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
