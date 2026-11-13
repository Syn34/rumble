// narrow_checks.h - Quantum.Physics2D.NarrowChecks and its nested per-shape-pair checks.
//
// Each Check(mInfo, detailedManifold) tests whether the two entries of the pair overlap.
// When they do and a detailed manifold is requested, it also fills the manifold's normal
// (pointing from entry A towards entry B), penetration depth and contact points. The pair
// order matters: a check written for (X, Y) expects A to be the X.
//
// The C# nested classes (NarrowChecks.CircleCircle, ...) are namespaces here. Checks that
// take the original's FrameThreadSafe only used it to reach the scene's thread-local data
// (box corners as a polygon), which is passed as a NarrowTls2D instead.
#pragma once

#include "collision_manifold2d.h"
#include "gjk2d.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

namespace Quantum {
struct CCDSettings;
}

namespace Quantum::Physics2D::NarrowChecks {

// 0x7d1381c - segment start->end against the axis-aligned box [min, max] (slab clipping).
// On a hit returns the entry point, the face normal there (-X, +X, -Y or +Y) and the
// entry parameter in [0, 1]. A segment starting inside the box does not hit. `contact`
// and `normalizedDist` are zeroed on a miss; `normal` keeps the last face that was
// tightened even then, like the original.
bool LineBoxLocalSpace(const FPVector2& localStart, const FPVector2& localEnd,
                       const FPVector2& min, const FPVector2& max, FPVector2& contact,
                       FPVector2& normal, FP& normalizedDist);

// 0x7d139d4 - dispatches on the pair's shape types (inverting the pair where the check is
// written for the opposite order). When the shapes don't overlap now but the manifold uses
// CCD, a time-of-impact query over `deltaTime` decides instead (a hit then counts as a
// collision). `settings` and `deltaTime` are the scene's GJK settings and time step.
bool Check(NarrowTls2D& tls, const CCDSettings& settings, FP deltaTime,
           CollisionManifoldInfo2D* mInfo, bool detailedManifold);

// The shape dispatch of Check without the CCD fallback.
bool CheckShapes(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold);
// Convenience overload with its own scratch data.
bool CheckShapes(CollisionManifoldInfo2D* mInfo, bool detailedManifold);

// A convex polygon view of a polygon or box entry, in world space (vertices and normals
// stay local; Position/RotSin/RotCos place them). Box corners live in the thread-local
// scratch buffers.
struct Polygon {
    FPVector2 Position;
    FP Radius; // broad radius
    const FPVector2* Vertices = nullptr;
    const FPVector2* Normals = nullptr;
    std::int32_t VerticesCount = 0;
    std::int32_t NormalsCount = 0;
    FP RotSin;
    FP RotCos;

    // 0x7d17380 - polygons use the entry's PolygonData; boxes become the four corners
    // UL, LL, LR, UR (counter-clockwise) with normals left, down, right, up. Other shape
    // types leave the polygon untouched.
    void Set(NarrowTls2D& tls, const NarrowEntry2D& entry);
};

// An oriented box as its four corners (in some local frame).
struct OOBB {
    FPVector2 UL; // upper left
    FPVector2 LL;
    FPVector2 UR;
    FPVector2 LR;

    // 0x7d16434 - axis-aligned box of half-size `extents` around the origin.
    void Set(FPVector2 extents);
    // 0x7d16a8c
    FPVector2 Min() const { return LL; }
    // 0x7d16a98
    FPVector2 Max() const { return UR; }
    // 0x7d16aa4 - inclusive containment, assuming the box is axis-aligned.
    bool InsideLocal(FPVector2 point) const;
    // 0x7d16ae0 - outward normal of the face nearest to `point` (ties: up, down, left,
    // right).
    FPVector2 NormalFromClosestFace(FPVector2 point) const;
};

namespace CircleCircle {
// 0x7d13e40
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace CircleCircle

namespace BoxCircle {
// 0x7d13fa4 - A is the box, B the circle.
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace BoxCircle

namespace CircleEdge {
// 0x7d14220 - A is the circle, B the edge.
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace CircleEdge

namespace BoxEdge {
// 0x7d1450c - A is the box, B the edge.
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace BoxEdge

namespace EdgeEdge {
// 0x7d15970 - the edges cross (collinear edges never collide). The contact is the crossing
// point; the normal is either A's normal or the perpendicular of B, whichever gives the
// shallower push-out.
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace EdgeEdge

namespace CirclePolygon {
// 0x7d14dd4 - A is the circle, B a polygon or box. Separating-axis test over the polygon's
// faces, then the nearest face or vertex region decides the contact.
bool Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold);
} // namespace CirclePolygon

namespace PolygonPolygon {
// 0x7d14a4c - A and B are polygons or boxes. SAT over both polygons' faces, then the
// incident face is clipped against the reference face's side planes (up to two contacts).
bool Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold);
// 0x7d16db8 - the face of `a` with the largest separation from `b` (negative when they
// overlap along every face normal of `a`) and its index.
FP FindAxisLeastPenetration(std::int32_t& faceIndex, const Polygon& a, const Polygon& b);
// 0x7d16f64 - the face of `incPoly` most anti-parallel to the reference face, in world
// space.
void FindIncidentFace(FPVector2 (&face)[2], const Polygon& refPoly, const Polygon& incPoly,
                      std::int32_t referenceIndex);
// 0x7d170bc - clips the segment `face` to the half-plane dot(n, p) <= c and returns
// how many points remain.
std::int32_t Clip(const FPVector2& n, FP c, FPVector2 (&face)[2]);
} // namespace PolygonPolygon

namespace PolygonEdge {
// 0x7d151cc - A is a polygon or box, B an edge. The edge (extended by the polygon's
// diameter while nothing has been found) is intersected with the polygon's sides; the
// penetration comes from the polygon vertices behind the edge.
bool Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold);
// 0x7d16c08 - walks the polygon outline from `initVertex` to `endVertexUnclamped`
// (indices wrap) up to the contact point cp2, starting at cp1. Returns the deepest vertex
// distance from the edge line and the length-weighted average of the outline normals.
FP CalculatePenetrationOnPolygonHalf(const Polygon& polygon, std::int32_t initVertex,
                                     std::int32_t endVertexUnclamped, const FPVector2& cp1,
                                     const FPVector2& cp2, const FPVector2& edgeNormal,
                                     FPVector2& avgNormal);
} // namespace PolygonEdge

namespace BoxBox {
struct LocalsContact {
    std::int32_t flipped = 0; // 1: the point is in B's frame
    FPVector2 point;
};

// Working state of one box-box test.
struct Locals {
    FP penetration;
    FPVector2 contactNormal;
    std::int32_t contactSize = 0;
    bool flipped = false; // the normal was found on B's axes
    LocalsContact* contacts = nullptr; // room for 4
};

// 0x7d13d10 - oriented boxes, SAT on both boxes' axes; contacts are corners of one box
// inside the other.
bool Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold);
// 0x7d16188 - runs both orderings and converts the result to world space.
bool Collision(Locals* locals, const NarrowEntry2D& a, const NarrowEntry2D& b);
// 0x7d16338 - SAT on a's two axes with b expressed in a's frame; collects b's corners
// inside a.
bool CollideOrdered(Locals* locals, const NarrowEntry2D& a, const NarrowEntry2D& b,
                    bool flipped);
// 0x7d16450 - the corners of `target`'s box expressed in `reference`'s local frame.
void ChangeBase(OOBB* box, const NarrowEntry2D& reference, const NarrowEntry2D& target);
// 0x7d165dc - overlap of `other` with the reference box along a local axis; keeps the
// smallest overlap (and its direction) in `locals`.
bool Project(Locals* locals, FPVector2 axis, const OOBB& other, bool flipped);
// 0x7d16844 - adds b's corners (order LL, UR, UL, LR) that lie inside a, up to two.
void FindContactPoint(Locals* locals, const OOBB& a, const OOBB& b, std::int32_t flipped);
// 0x7d16a5c
bool Inside(FPVector2 point, const OOBB& shape);
} // namespace BoxBox

} // namespace Quantum::Physics2D::NarrowChecks
