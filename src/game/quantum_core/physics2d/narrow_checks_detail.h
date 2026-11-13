// narrow_checks_detail.h - small geometric helpers shared by the 2D narrow-phase checks.
// Not part of the public API.
#pragma once

#include "collision_manifold2d.h"

#include "photon_deterministic/fp_math.h"

namespace Quantum::Physics2D::NarrowChecks::detail {

// World offset -> local coordinates of a frame rotated by (sin, cos) (inverse rotation).
constexpr FPVector2 ToLocal(FPVector2 offset, FP sin, FP cos) {
    return {offset.Y * sin + offset.X * cos, offset.Y * cos - offset.X * sin};
}

// Outward normal of the box face nearest to a point inside the box. Ties keep the face
// tested first (top, bottom, left, right).
inline FPVector2 NearestFaceNormal(FPVector2 extents, FP x, FP y) {
    namespace FPMath = Photon::Deterministic::FPMath;
    FPVector2 normal = FPVector2::Up();
    FP best = FPMath::Abs(y - extents.Y);
    if (const FP d = FPMath::Abs(y - (-extents.Y)); d < best) {
        normal = FPVector2::Down();
        best = d;
    }
    if (const FP d = FPMath::Abs(x - (-extents.X)); d < best) {
        normal = FPVector2::Left();
        best = d;
    }
    if (FPMath::Abs(x - extents.X) < best) {
        normal = FPVector2::Right();
    }
    return normal;
}

// Half of an edge shape in world orientation: the rotated (-Extent, 0). The endpoints are
// centre + half and centre - half.
inline FPVector2 EdgeHalf(const NarrowEntry2D& edge) {
    return FPVector2::Rotate(FPVector2::Left() * edge.Shape->Edge.Extent, edge.Narrow.RotSin,
                             edge.Narrow.RotCos);
}

} // namespace Quantum::Physics2D::NarrowChecks::detail
