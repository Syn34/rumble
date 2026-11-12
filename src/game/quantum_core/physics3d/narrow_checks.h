// narrow_checks.h - Quantum.Physics3D.NarrowChecks and its nested shape-pair checks.
//
// Same conventions as the 2D checks: Check(mInfo, detailedManifold) returns whether the
// pair overlaps and, for a detailed manifold, fills normal (A towards B), penetration and
// contact points. The C# nested classes are namespaces here.
#pragma once

#include "collision_manifold3d.h"

namespace Quantum::Physics3D::NarrowChecks {

namespace SphereSphere {
// 0x7d790cc
bool Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold);
} // namespace SphereSphere

// Not implemented yet:
// TODO(0x7d7071c, Quantum.Physics3D.NarrowChecks.Check): type-pair dispatch (plus mesh
// triangle iteration and the CCD fallback); needs the scene.
// TODO(0x7d78bec, Quantum.Physics3D.NarrowChecks.SphereBox.Check): sphere centre into box
// space through NarrowData3D's rotated axes, clamp, push out; needs NarrowData3D.Init.
// TODO(0x7d79290, Quantum.Physics3D.NarrowChecks.SphereTriangle.Check), with GetNearestPoint
// (0x7d79418) and CheckForVertex (0x7d796d8); needs TriangleCCW.
// TODO(0x7d70eec, Quantum.Physics3D.NarrowChecks.BoxBox.Check): SAT over face and edge axes
// with GetEdgesBox (0x7d72a28), GetPlanesBox (0x7d72950), GetContactPoints (0x7d72d44),
// GetContactPointPlaneEdge (0x7d72ec4), ClipToPlane (0x7d75a6c), BoxInAABB (0x7d75878),
// PointInAABB (0x7d75bac).
// TODO(0x7d72058, Quantum.Physics3D.NarrowChecks.BoxTriangle.Check): Akenine-Moller
// box/triangle SAT (AxisTest* 0x7d75ca4..0x7d75f6c, PlaneBoxOverlaps 0x7d76000) with edge
// clipping for contacts (0x7d763d0, 0x7d77624, 0x7d77f18, 0x7d76368, 0x7d77fd4, 0x7d7619c,
// 0x7d78180, 0x7d762bc).

} // namespace Quantum::Physics3D::NarrowChecks
