// narrow_checks.cpp - Quantum.Physics2D.NarrowChecks.
#include "narrow_checks.h"
#include "narrow_checks_detail.h"

#include "quantum_core/gjk_settings.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <cassert>
#include <cstdint>
#include <limits>

namespace Quantum::Physics2D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

using detail::EdgeHalf;
using detail::NearestFaceNormal;
using detail::ToLocal;

// 0x7d1381c
bool LineBoxLocalSpace(const FPVector2& localStart, const FPVector2& localEnd,
                       const FPVector2& min, const FPVector2& max, FPVector2& contact,
                       FPVector2& normal, FP& normalizedDist) {
    contact = FPVector2::Zero();
    normal = FPVector2::Zero();
    normalizedDist = FP::Zero();

    const FPVector2 delta = localEnd - localStart;
    // For each face (-X, +X, -Y, +Y): direction term and distance term of the slab test.
    const FP directions[4] = {-delta.X, delta.X, -delta.Y, delta.Y};
    const FP distances[4] = {localStart.X - min.X, max.X - localStart.X, localStart.Y - min.Y,
                             max.Y - localStart.Y};
    const FPVector2 faceNormals[4] = {FPVector2::Left(), FPVector2::Right(), FPVector2::Down(),
                                      FPVector2::Up()};

    FP enter = FP::MinValue();
    FP exit = FP::MaxValue();
    for (int face = 0; face < 4; ++face) {
        const FP direction = directions[face];
        if (direction == 0) {
            if (distances[face] < 0) {
                return false; // parallel to this slab and outside it
            }
            continue;
        }
        const FP t = distances[face] / direction;
        if (direction < 0) {
            if (enter < t) {
                enter = t;
                normal = faceNormals[face];
            }
        } else if (exit > t) {
            exit = t;
        }
    }

    if (enter > exit || enter < 0 || enter > 1) {
        return false;
    }
    contact = {localStart.X + enter * delta.X, localStart.Y + enter * delta.Y};
    normalizedDist = enter;
    return true;
}

// The shape-pair dispatch of NarrowChecks.Check (0x7d139d4). Pairs are keyed by
// (type A, type B); pairs whose check is written for the other order are inverted first.
bool CheckShapes(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    assert(mInfo != nullptr && mInfo->EntryA != nullptr && mInfo->EntryB != nullptr);
    using T = Shape2DType;
    const T a = mInfo->EntryA->Shape->Type();
    const T b = mInfo->EntryB->Shape->Type();
    auto is = [&](T first, T second) { return a == first && b == second; };
    auto inverted = [&]() {
        mInfo->InvertEntries();
        return mInfo;
    };

    if (is(T::Circle, T::Circle)) {
        return CircleCircle::Check(mInfo, detailedManifold);
    }
    if (is(T::Circle, T::Polygon)) {
        return CirclePolygon::Check(tls, mInfo, detailedManifold);
    }
    if (is(T::Polygon, T::Circle)) {
        return CirclePolygon::Check(tls, inverted(), detailedManifold);
    }
    if (is(T::Box, T::Circle)) {
        return BoxCircle::Check(mInfo, detailedManifold);
    }
    if (is(T::Circle, T::Box)) {
        return BoxCircle::Check(inverted(), detailedManifold);
    }
    if (is(T::Circle, T::Edge)) {
        return CircleEdge::Check(mInfo, detailedManifold);
    }
    if (is(T::Edge, T::Circle)) {
        return CircleEdge::Check(inverted(), detailedManifold);
    }
    // Polygon-box pairs run the polygon clipper with the box as A.
    if (is(T::Polygon, T::Polygon) || is(T::Box, T::Polygon)) {
        return PolygonPolygon::Check(tls, mInfo, detailedManifold);
    }
    if (is(T::Polygon, T::Box)) {
        return PolygonPolygon::Check(tls, inverted(), detailedManifold);
    }
    if (is(T::Polygon, T::Edge)) {
        return PolygonEdge::Check(tls, mInfo, detailedManifold);
    }
    if (is(T::Edge, T::Polygon)) {
        return PolygonEdge::Check(tls, inverted(), detailedManifold);
    }
    if (is(T::Box, T::Box)) {
        return BoxBox::Check(mInfo, detailedManifold);
    }
    if (is(T::Box, T::Edge)) {
        return BoxEdge::Check(mInfo, detailedManifold);
    }
    if (is(T::Edge, T::Box)) {
        return BoxEdge::Check(inverted(), detailedManifold);
    }
    if (is(T::Edge, T::Edge)) {
        return EdgeEdge::Check(mInfo, detailedManifold);
    }
    return false; // compounds and unknown types never collide here
}

bool CheckShapes(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    NarrowTls2D tls;
    return CheckShapes(tls, mInfo, detailedManifold);
}

// 0x7d139d4
bool Check(NarrowTls2D& tls, const CCDSettings& settings, FP deltaTime,
           CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const bool overlap = CheckShapes(tls, mInfo, detailedManifold);
    if (overlap || !mInfo->Manifold->_m.GetUsesCcd()) {
        return overlap;
    }
    FP timeOfImpact;
    return Gjk2D::ComputeTimeOfImpact(tls, mInfo, settings, detailedManifold, deltaTime,
                                      timeOfImpact) == TimeOfImpactResult::Collision;
}

// 0x7d13e40
bool CircleCircle::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const NarrowEntry2D& entryA = *mInfo->EntryA;
    const NarrowEntry2D& entryB = *mInfo->EntryB;
    const FP radiusA = entryA.Shape->Circle.Radius;
    const FP radii = entryB.Shape->Circle.Radius + radiusA;
    const FPVector2 posA = entryA.Narrow.Position;
    const FPVector2 delta = entryB.Narrow.Position - posA;
    const FP distanceSquared = FPVector2::Dot(delta, delta);
    const bool overlap = distanceSquared < radii * radii;

    if (overlap && detailedManifold) {
        CollisionManifold2D* manifold = mInfo->Manifold;
        const FP distance = FPMath::Sqrt(distanceSquared);
        FPVector2 contact;
        if (distance > fpdetail::Epsilon) {
            const FPVector2 normal = {delta.X / distance, delta.Y / distance};
            manifold->Penetration = radii - distance;
            manifold->Normal = normal;
            contact = {posA.X + normal.X * radiusA, posA.Y + normal.Y * radiusA};
        } else {
            // Concentric: any direction will do.
            manifold->Penetration = radiusA;
            manifold->Normal = FPVector2::Right();
            contact = posA;
        }
        CollisionManifold2D::AddContactPoint(manifold, contact);
    }
    return overlap;
}

// 0x7d13fa4
bool BoxCircle::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const NarrowEntry2D& box = *mInfo->EntryA;
    const NarrowEntry2D& circle = *mInfo->EntryB;
    const FPVector2 boxPos = box.Narrow.Position;
    const FPVector2 circlePos = circle.Narrow.Position;
    const FP radius = circle.Shape->Circle.Radius;

    // Bounding-circle rejection first.
    const FP radii = radius + box.Shape->Box._broadRadius;
    const FPVector2 delta = circlePos - boxPos;
    if (!(FPVector2::Dot(delta, delta) < radii * radii)) {
        return false;
    }

    const FP sin = box.Narrow.RotSin;
    const FP cos = box.Narrow.RotCos;
    const FPVector2 extents = box.Shape->Box._extents;
    const FPVector2 local = ToLocal(delta, sin, cos);
    CollisionManifold2D* manifold = mInfo->Manifold;

    const bool centreInside = local.X >= -extents.X && local.X <= extents.X &&
                              local.Y >= -extents.Y && local.Y <= extents.Y;
    if (centreInside) {
        if (detailedManifold) {
            // Push out through the nearest face; the contact is the circle centre nudged
            // 1/256 against the normal.
            const FPVector2 face = NearestFaceNormal(extents, local.X, local.Y);
            manifold->Penetration = radius;
            const FPVector2 normal = -FPVector2::Rotate(-face, sin, cos);
            manifold->Normal = normal;
            const FPVector2 contact = {
                circlePos.X - FP::FromRaw(normal.X.RawValue() >> 8),
                circlePos.Y - FP::FromRaw(normal.Y.RawValue() >> 8)};
            CollisionManifold2D::AddContactPoint(manifold, contact);
        }
        return true;
    }

    const FPVector2 closest = FPVector2::Clamp(local, -extents, extents);
    if (FPVector2::DistanceSquared(closest, local) > radius * radius) {
        return false;
    }
    FP distance;
    const FPVector2 direction = FPVector2::Normalize(closest - local, distance);
    manifold->Penetration = radius - distance; // written even when it turns out negative
    if (manifold->Penetration < 0) {
        return false;
    }
    const FPVector2 worldDirection = FPVector2::Rotate(direction, sin, cos);
    if (!detailedManifold) {
        return true;
    }
    CollisionManifold2D::AddContactPoint(manifold, FPVector2::Rotate(closest, sin, cos) + boxPos);
    manifold->Normal = -worldDirection;
    return true;
}

// 0x7d14220
bool CircleEdge::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const NarrowEntry2D& circle = *mInfo->EntryA;
    const NarrowEntry2D& edge = *mInfo->EntryB;
    const FPVector2 centre = circle.Narrow.Position;
    const FP radius = circle.Shape->Circle.Radius;
    const FP extent = edge.Shape->Edge.Extent;
    const FP sin = edge.Narrow.RotSin;
    const FP cos = edge.Narrow.RotCos;

    const FPVector2 half = EdgeHalf(edge);
    const FPVector2 start = half + edge.Narrow.Position;
    const FPVector2 end = edge.Narrow.Position - half;

    // Project the centre onto the edge line (direction (cos, sin) from `start`).
    const FPVector2 toCentre = centre - start;
    const FP along = toCentre.Y * sin + toCentre.X * cos;
    const FPVector2 closest = {start.X + along * cos, start.Y + along * sin};
    const FPVector2 closestToCentre = FPVector2::Normalize(centre - closest);

    CollisionManifold2D* manifold = mInfo->Manifold;
    auto touchVertex = [&](const FPVector2& vertex) {
        const FP distance = FPVector2::Distance(vertex, centre);
        if (distance > radius) {
            return false;
        }
        if (detailedManifold) {
            CollisionManifold2D::AddContactPoint(manifold, vertex);
            manifold->Normal = FPVector2::Normalize(vertex - centre);
            manifold->Penetration = radius - distance;
        }
        return true;
    };

    if (along < 0) {
        return touchVertex(start);
    }
    if (along > fpdetail::MulInt(extent, 2)) {
        return touchVertex(end);
    }
    const FP distance = FPVector2::Distance(closest, centre);
    if (distance > radius) {
        return false;
    }
    if (detailedManifold) {
        CollisionManifold2D::AddContactPoint(manifold, closest);
        manifold->Normal = -closestToCentre;
        manifold->Penetration = radius - distance;
    }
    return true;
}

// 0x7d1450c
bool BoxEdge::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const NarrowEntry2D& box = *mInfo->EntryA;
    const NarrowEntry2D& edge = *mInfo->EntryB;
    const FPVector2 boxPos = box.Narrow.Position;
    const FP sin = box.Narrow.RotSin;
    const FP cos = box.Narrow.RotCos;
    const FP ex = box.Shape->Box._extents.X;
    const FP ey = box.Shape->Box._extents.Y;

    // Edge endpoints in the box's local frame.
    const FPVector2 half = EdgeHalf(edge);
    const FPVector2 offset = edge.Narrow.Position - boxPos;
    const FPVector2 p0 = ToLocal(offset + half, sin, cos);
    const FPVector2 p1 = ToLocal(offset - half, sin, cos);
    const FPVector2 d = p1 - p0;

    // Slab clipping of p0 + t*d against [-ex, ex] x [-ey, ey].
    const FP exitX = ex - p0.X;
    const FP leftGapX = p0.X + ex;
    const FP exitY = ey - p0.Y;
    const FP bottomGapY = p0.Y + ey;
    if (d.X == 0 && (-leftGapX > 0 || exitX < 0)) {
        return false;
    }
    if (d.Y == 0 && (-bottomGapY > 0 || exitY < 0)) {
        return false;
    }

    FP tMin = FP::FromRaw(std::numeric_limits<std::int32_t>::min());
    FP tMax = FP::FromRaw(std::numeric_limits<std::int32_t>::max());
    if (d.X != 0) {
        const FP a = -leftGapX / d.X;
        const FP b = exitX / d.X;
        tMax = FPMath::Max(a, b);
        tMin = FPMath::Min(a, b);
        if (tMax < fpdetail::Epsilon) {
            return false;
        }
    }
    if (d.Y != 0) {
        const FP a = -bottomGapY / d.Y;
        const FP b = exitY / d.Y;
        tMin = FPMath::Max(tMin, FPMath::Min(a, b));
        tMax = FPMath::Min(tMax, FPMath::Max(a, b));
        if (tMin > tMax || tMax < fpdetail::Epsilon) {
            return false;
        }
    }
    if (tMax > 1 && tMin > 1) {
        return false;
    }
    if (!detailedManifold) {
        return true;
    }

    // The clipped part of the edge gives the two contact points.
    const FPVector2 contact0 = tMin < 0 ? p0 : FPVector2{p0.X + tMin * d.X, p0.Y + tMin * d.Y};
    const FPVector2 contact1 = tMax > 1 ? p1 : FPVector2{p0.X + tMax * d.X, p0.Y + tMax * d.Y};

    // Default normal: from the box centre towards the nearest point of the edge's line,
    // with the box's support depth along it as penetration.
    const FP length = d.Magnitude();
    const FPVector2 direction = {d.X / length, d.Y / length};
    const FP along = (-p0.Y) * direction.Y + (-p0.X) * direction.X;
    FPVector2 normal = FPVector2::Normalize({p0.X + along * direction.X, p0.Y + along * direction.Y});
    const FP depthX = normal.X >= 0 ? normal.X * exitX : (-normal.X) * leftGapX;
    const FP depthY = normal.Y >= 0 ? (-normal.Y) * exitY : normal.Y * bottomGapY;
    FP penetration = depthX - depthY;

    // If an endpoint pokes out of the box, a face may give a shallower way out.
    const bool clipped = tMax > 1 || tMin < 0;
    if (clipped) {
        auto consider = [&](FP depth, FPVector2 faceNormal) {
            if (depth < penetration) {
                penetration = depth;
                normal = faceNormal;
            }
        };
        if (d.X > 0) {
            if (tMin < 0) {
                consider(exitX, FPVector2::Right());
            }
            if (tMax > 1) {
                consider(p1.X + ex, FPVector2::Left());
            }
        } else {
            if (tMin < 0) {
                consider(leftGapX, FPVector2::Left());
            }
            if (tMax > 1) {
                consider(exitX - d.X, FPVector2::Right());
            }
        }
        if (d.Y > 0) {
            if (tMin < 0) {
                consider(exitY, FPVector2::Up());
            }
            if (tMax > 1) {
                consider(p1.Y + ey, FPVector2::Down());
            }
        } else {
            if (tMin < 0) {
                consider(bottomGapY, FPVector2::Down());
            }
            if (tMax > 1) {
                consider(exitY - d.Y, FPVector2::Up());
            }
        }
    }

    if (penetration < 0) {
        return false;
    }
    CollisionManifold2D* manifold = mInfo->Manifold;
    manifold->Penetration = penetration;
    manifold->Normal = FPVector2::Rotate(normal, sin, cos);
    CollisionManifold2D::AddContactPoint(manifold, FPVector2::Rotate(contact0, sin, cos) + boxPos);
    CollisionManifold2D::AddContactPoint(manifold, FPVector2::Rotate(contact1, sin, cos) + boxPos);
    return true;
}

// 0x7d15970
bool EdgeEdge::Check(CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    const NarrowEntry2D& a = *mInfo->EntryA;
    const NarrowEntry2D& b = *mInfo->EntryB;
    const FP extentA = a.Shape->Edge.Extent;
    const FP extentB = b.Shape->Edge.Extent;
    const FP sinA = a.Narrow.RotSin;
    const FP cosA = a.Narrow.RotCos;

    // B in A's frame: its first endpoint, its unit direction towards that endpoint (the
    // rotated (-1, 0)) and the vector from the first to the second endpoint (-span).
    const FPVector2 start = ToLocal(b.Narrow.Position + EdgeHalf(b) - a.Narrow.Position, sinA, cosA);
    const FPVector2 direction = ToLocal({-b.Narrow.RotCos, -b.Narrow.RotSin}, sinA, cosA);
    const FPVector2 span = {fpdetail::TwiceProduct(direction.X, extentB),
                            fpdetail::TwiceProduct(direction.Y, extentB)};
    const FPVector2 end = start - span;

    // Both endpoints beyond the same end of A, or on the same side of A's line, or both
    // on the line (collinear): no crossing.
    const FP minX = span.X < 0 ? start.X : end.X;
    const FP maxX = span.X < 0 ? end.X : start.X;
    if (minX > extentA || maxX < -extentA) {
        return false;
    }
    if (span.Y == 0 && start.Y == 0) {
        return false;
    }
    if (fpdetail::WrapMul(end.Y.RawValue(), start.Y.RawValue()) > 0) {
        return false;
    }
    // Where B crosses A's line (raw product over raw divisor, as the original computes it).
    assert(span.Y != 0);
    const FP crossing =
        FP::FromRaw(fpdetail::WrapMul(start.Y.RawValue(), span.X.RawValue()) / -span.Y.RawValue()) +
        start.X;
    const FP insideA = extentA - FPMath::Abs(crossing);
    if (insideA < 0) {
        return false;
    }
    if (!detailedManifold) {
        return true;
    }

    // Push B off A's line by the shorter way, or along A's line if that is shorter still.
    CollisionManifold2D* manifold = mInfo->Manifold;
    if (start.Y > 0) {
        if (start.Y < -end.Y) {
            manifold->Penetration = start.Y;
            manifold->Normal = FPVector2::Down();
        } else {
            manifold->Penetration = -end.Y;
            manifold->Normal = FPVector2::Up();
        }
    } else if (end.Y <= -start.Y) {
        manifold->Penetration = FPMath::Max(end.Y, FP::Zero());
        manifold->Normal = FPVector2::Down();
    } else {
        manifold->Penetration = -start.Y;
        manifold->Normal = FPVector2::Up();
    }
    const FP alongA = insideA * FPMath::Abs(direction.Y);
    if (alongA < manifold->Penetration) {
        manifold->Penetration = alongA;
        const bool sameSide = fpdetail::WrapMul(crossing.RawValue(), start.Y.RawValue()) > 0;
        manifold->Normal = sameSide ? FPVector2{direction.Y, -direction.X}
                                    : FPVector2{-direction.Y, direction.X};
    }
    CollisionManifold2D::AddContactPoint(manifold,
                                         a.Narrow.Position + FPVector2{crossing * cosA, crossing * sinA});
    manifold->Normal = FPVector2::Rotate(manifold->Normal, sinA, cosA);
    return true;
}

} // namespace Quantum::Physics2D::NarrowChecks
