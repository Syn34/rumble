// narrow_checks.h - Quantum.Physics2D.NarrowChecks and its nested per-shape-pair checks.
//
// Each Check(mInfo, detailedManifold) tests whether the two entries of the pair overlap.
// When they do and a detailed manifold is requested, it also fills the manifold's normal
// (pointing from entry A towards entry B), penetration depth and contact points. The pair
// order matters: a check written for (X, Y) expects A to be the X.
//
// The C# nested classes (NarrowChecks.CircleCircle, ...) are namespaces here.
#pragma once

#include "collision_manifold2d.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

namespace Quantum::Physics2D::NarrowChecks {

// 0x7d1381c - segment start->end against the axis-aligned box [min, max] (slab clipping).
// On a hit returns the entry point, the face normal there (-X, +X, -Y or +Y) and the
// entry parameter in [0, 1]. A segment starting inside the box does not hit. `contact`
// and `normalizedDist` are zeroed on a miss; `normal` keeps the last face that was
// tightened even then, like the original.
bool LineBoxLocalSpace(const FPVector2& localStart, const FPVector2& localEnd,
                       const FPVector2& min, const FPVector2& max, FPVector2& contact,
                       FPVector2& normal, FP& normalizedDist);

// Dispatches on the pair's shape types (inverting the pair where the check is written for
// the opposite order) and returns whether they overlap.
// The original NarrowChecks.Check (0x7d139d4) additionally falls back to a time-of-impact
// query for CCD manifolds that don't overlap now:
// TODO(0x7d139d4, Quantum.Physics2D.NarrowChecks.Check): CCD fallback through
// Gjk2D.ComputeTimeOfImpact needs the scene's thread-local data and settings.
bool CheckShapes(CollisionManifoldInfo2D* mInfo, bool detailedManifold);

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

// Not implemented yet (CheckShapes asserts on these pairs):
// TODO(0x7d15970, Quantum.Physics2D.NarrowChecks.EdgeEdge.Check)
// TODO(0x7d14dd4, Quantum.Physics2D.NarrowChecks.CirclePolygon.Check): A circle, B polygon
// (boxes count as polygons); needs Polygon.Set.
// TODO(0x7d14a4c, Quantum.Physics2D.NarrowChecks.PolygonPolygon.Check): SAT with
// FindAxisLeastPenetration (0x7d16db8), FindIncidentFace (0x7d16f64), Clip (0x7d170bc).
// TODO(0x7d151cc, Quantum.Physics2D.NarrowChecks.PolygonEdge.Check), with
// CalculatePenetrationOnPolygonHalf (0x7d16c08).
// TODO(0x7d13d10, Quantum.Physics2D.NarrowChecks.BoxBox.Check): oriented-box SAT through
// Collision (0x7d16188), CollideOrdered (0x7d16338), ChangeBase (0x7d16450), Project
// (0x7d165dc), FindContactPoint (0x7d16844), Inside (0x7d16a5c).
// TODO(0x7d17380, Quantum.Physics2D.NarrowChecks.Polygon.Set): world-space polygon view of
// an entry (box corners or the PolygonCollider's vertices); needs the frame for assets.

} // namespace Quantum::Physics2D::NarrowChecks
