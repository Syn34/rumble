// narrow_checks_box.cpp - Quantum.Physics3D.NarrowChecks.BoxBox and BoxTriangle.
//
// Both checks work with the box caches of NarrowData3D (edges and face planes, local and
// rotated). Contact candidates are found by clipping edges against face planes, which is
// why the original runs these loops unrolled; here they are plain loops in the same order
// (plane by plane, edge by edge), since the order decides which contacts are kept.
#include "narrow_checks.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/fp_quaternion.h"

#include <cassert>
#include <limits>

namespace Quantum::Physics3D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {

constexpr FP::Raw RawMax = std::numeric_limits<FP::Raw>::max();

FP Component(const FPVector3& v, int axis) {
    return axis == 0 ? v.X : (axis == 1 ? v.Y : v.Z);
}

// A point in the box's local frame expressed in world space.
FPVector3 LocalToWorld(const NarrowData3D& narrow, const FPVector3& p) {
    return narrow.Position + narrow.RotatedRight * p.X + narrow.RotatedUp * p.Y +
           narrow.RotatedForward * p.Z;
}

} // namespace

// ---------------------------------------------------------------------------------------
// BoxBox
// ---------------------------------------------------------------------------------------

// 0x7d70eec
bool BoxBox::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    assert(mInfo != nullptr && mInfo->EntryA != nullptr && mInfo->EntryB != nullptr);
    const NarrowEntry3D& boxA = *mInfo->EntryA;
    const NarrowEntry3D& boxB = *mInfo->EntryB;
    const NarrowData3D& a = boxA.Narrow;
    const NarrowData3D& b = boxB.Narrow;
    const FPVector3 extentsA = boxA.Shape->Box._extents;
    const FPVector3 extentsB = boxB.Shape->Box._extents;
    const FP eA[3] = {extentsA.X, extentsA.Y, extentsA.Z};
    const FP eB[3] = {extentsB.X, extentsB.Y, extentsB.Z};
    const FPVector3 axesA[3] = {a.RotatedRight, a.RotatedUp, a.RotatedForward};
    const FPVector3 axesB[3] = {b.RotatedRight, b.RotatedUp, b.RotatedForward};

    // B's axes in A's frame; the absolute values get an epsilon so that nearly parallel
    // edges (cross product ~ 0) don't produce false separations.
    FP r[3][3];
    FP absR[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            r[i][j] = FPVector3::Dot(axesA[i], axesB[j]);
        }
    }
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            absR[i][j] = FPMath::Abs(r[i][j]) + fpdetail::Epsilon;
        }
    }

    // Centre offset, in world space and in A's frame.
    const FPVector3 offset = b.Position - a.Position;
    const FP t[3] = {FPVector3::Dot(offset, axesA[0]), FPVector3::Dot(offset, axesA[1]),
                     FPVector3::Dot(offset, axesA[2])};

    FP minPenetration = FP::MaxValue();
    FPVector3 bestAxis = FPVector3::Zero();

    // Face axes of A.
    for (int i = 0; i < 3; ++i) {
        const FP separation =
            eA[i] + absR[i][0] * eB[0] + absR[i][1] * eB[1] + absR[i][2] * eB[2] - FPMath::Abs(t[i]);
        if (separation <= 0) {
            return false;
        }
        if (separation < minPenetration && detailedManifold) {
            minPenetration = separation;
            bestAxis = axesA[i];
        }
    }
    // Face axes of B.
    for (int j = 0; j < 3; ++j) {
        const FP distance = t[1] * r[1][j] + t[0] * r[0][j] + t[2] * r[2][j];
        const FP separation = eB[j] + absR[0][j] * eA[0] + absR[1][j] * eA[1] + absR[2][j] * eA[2] -
                              FPMath::Abs(distance);
        if (separation <= 0) {
            return false;
        }
        if (separation < minPenetration && detailedManifold) {
            minPenetration = separation;
            bestAxis = axesB[j];
        }
    }
    // Edge pairs A_i x B_j, skipped when the edges are nearly parallel.
    for (int i = 0; i < 3; ++i) {
        const int i1 = (i + 1) % 3;
        const int i2 = (i + 2) % 3;
        for (int j = 0; j < 3; ++j) {
            if (absR[i][j] >= allowedDotDiff()) {
                continue;
            }
            const int j1 = (j + 1) % 3;
            const int j2 = (j + 2) % 3;
            const FP radiusA = eA[i1] * absR[i2][j] + eA[i2] * absR[i1][j];
            const FP radiusB = eB[j1] * absR[i][j2] + eB[j2] * absR[i][j1];
            const FP distance = t[i2] * r[i1][j] - t[i1] * r[i2][j];
            const FP separation = radiusA + radiusB - FPMath::Abs(distance);
            if (separation <= 0) {
                return false;
            }
            if (detailedManifold && separation < minPenetration) {
                // Unnormalised; degenerate crosses are not used as the normal.
                const FPVector3 axis = FPVector3::Cross(axesA[i], axesB[j]);
                if (axis.SqrMagnitude() > fpdetail::Epsilon) {
                    bestAxis = axis;
                    minPenetration = separation;
                }
            }
        }
    }

    // Overlapping: B's caches are made relative to A's centre, like A's own.
    const CustomPlanesBox planesB = GetPlanesBox(b.BoxPlanesRotated, offset);
    const CustomEdgesBox edgesB = GetEdgesBox(b.BoxEdgesRotated, offset);
    CollisionManifold3D* manifold = mInfo->Manifold;
    GetContactPoints(a.BoxPlanesRotated, planesB, a.BoxEdgesRotated, edgesB, &boxA, &boxB, offset,
                     manifold, detailedManifold);
    const bool hit = manifold->_contactPointsCount != 0;
    if (hit && detailedManifold) {
        if (FPVector3::Dot(offset, bestAxis) < 0) {
            bestAxis = -bestAxis;
        }
        manifold->Penetration = minPenetration;
        manifold->Normal = FPVector3::Normalize(bestAxis);
    }
    return hit;
}

// 0x7d72a28
CustomEdgesBox BoxBox::GetEdgesBox(CustomEdgesBox edges, const FPVector3& translation) {
    for (CustomLine& edge : edges.Edges) {
        edge.start = translation + edge.start;
        edge.end = translation + edge.end;
    }
    return edges;
}

// 0x7d72950
CustomPlanesBox BoxBox::GetPlanesBox(CustomPlanesBox planes, const FPVector3& translation) {
    for (CustomPlane& plane : planes.Planes) {
        plane.pointOnPlane = translation + plane.pointOnPlane;
    }
    return planes;
}

// 0x7d72d44
void BoxBox::GetContactPoints(const CustomPlanesBox& planesA, const CustomPlanesBox& planesB,
                              const CustomEdgesBox& edgesA, const CustomEdgesBox& edgesB,
                              const NarrowEntry3D* boxEntryA, const NarrowEntry3D* boxEntryB,
                              const FPVector3& boxAToBoxBOffset, CollisionManifold3D* manifold,
                              bool detailedManifold) {
    assert(boxEntryA != nullptr && manifold != nullptr);
    const FPVector3 noOffset = FPVector3::Zero();
    const FPVector3& positionA = boxEntryA->Narrow.Position;

    // B's edges through A's faces, then (if more contacts are wanted) A's edges through B's.
    GetContactPointPlaneEdge(planesA, edgesB, boxEntryA, noOffset, positionA, manifold,
                             detailedManifold);
    std::int32_t count = manifold->_contactPointsCount;
    if (count == 0 || (detailedManifold && count <= 3)) {
        GetContactPointPlaneEdge(planesB, edgesA, boxEntryB, boxAToBoxBOffset, positionA, manifold,
                                 detailedManifold);
        count = manifold->_contactPointsCount;
    }
    if (count > 0) {
        return;
    }
    // No edge crosses a face: one box may contain the other.
    if (BoxInAABB(edgesB, boxEntryA, noOffset)) {
        CollisionManifold3D::AddContactPoint(manifold, boxAToBoxBOffset + positionA);
    } else if (BoxInAABB(edgesA, boxEntryB, boxAToBoxBOffset)) {
        CollisionManifold3D::AddContactPoint(manifold, positionA);
    }
}

// 0x7d72ec4
void BoxBox::GetContactPointPlaneEdge(const CustomPlanesBox& planes, const CustomEdgesBox& edges,
                                      const NarrowEntry3D* boxEntry, const FPVector3& offset,
                                      const FPVector3& boxAPosition, CollisionManifold3D* manifold,
                                      bool detailedManifold) {
    assert(boxEntry != nullptr && manifold != nullptr);
    const FPVector3 grownExtents = boxEntry->Shape->Box._extents * extentsExpansionMultiplier();
    FPVector3 intersection = FPVector3::Zero();
    for (const CustomPlane& plane : planes.Planes) {
        for (const CustomLine& edge : edges.Edges) {
            if (!ClipToPlane(plane, edge, intersection)) {
                continue;
            }
            FPVector3 local = intersection;
            if (!PointInAABB(local, boxEntry, grownExtents, offset)) {
                continue;
            }
            CollisionManifold3D::AddContactPoint(manifold, boxAPosition + intersection);
            if (!detailedManifold || manifold->_contactPointsCount > 3) {
                return;
            }
        }
    }
}

// 0x7d75a6c
bool BoxBox::ClipToPlane(const CustomPlane& plane, const CustomLine& line, FPVector3& intersection) {
    const FPVector3 direction = line.end - line.start;
    const FP denominator = FPVector3::Dot(plane.normal, direction);
    if (FPMath::Abs(denominator).RawValue() <= fpdetail::Epsilon.RawValue()) {
        return false;
    }
    const FP fraction = FPVector3::Dot(plane.pointOnPlane - line.start, plane.normal) / denominator;
    if (fraction < 0 || fraction > FP::One()) {
        return false;
    }
    intersection = line.start + direction * fraction;
    return true;
}

// 0x7d75878
bool BoxBox::BoxInAABB(const CustomEdgesBox& edges, const NarrowEntry3D* boxEntry,
                       const FPVector3& offset) {
    assert(boxEntry != nullptr);
    const FPVector3 grownExtents = boxEntry->Shape->Box._extents * extentsExpansionMultiplier();
    // The eight corners: starts of the even edges of the top and bottom faces, then their
    // ends.
    const FPVector3 corners[8] = {edges.Edges[0].start, edges.Edges[2].start, edges.Edges[4].start,
                                  edges.Edges[6].start, edges.Edges[0].end,   edges.Edges[2].end,
                                  edges.Edges[4].end,   edges.Edges[6].end};
    for (FPVector3 corner : corners) {
        if (!PointInAABB(corner, boxEntry, grownExtents, offset)) {
            return false;
        }
    }
    return true;
}

// 0x7d75bac
bool BoxBox::PointInAABB(FPVector3& point, const NarrowEntry3D* box, const FPVector3& extents,
                         const FPVector3& offset) {
    assert(box != nullptr);
    point = box->Narrow.RotationInverse * (point - offset);
    return point.X >= -extents.X && point.Y >= -extents.Y && point.X <= extents.X &&
           point.Y <= extents.Y && point.Z >= -extents.Z && point.Z <= extents.Z;
}

// ---------------------------------------------------------------------------------------
// BoxTriangle
// ---------------------------------------------------------------------------------------

namespace {

// Shared tail of the axis tests: the overlap of the projected triangle [min, max] with the
// box's projection [-radius, radius].
bool AxisOverlap(FP projectionA, FP projectionB, FP radius, FP::Raw& penetrationRaw) {
    const FP max = projectionA > projectionB ? projectionA : projectionB;
    const FP min = projectionA < projectionB ? projectionA : projectionB;
    const FP penetration = radius + (min <= -max ? max : -min);
    if (penetration.RawValue() < penetrationRaw) {
        penetrationRaw = penetration.RawValue();
    }
    return penetration.RawValue() < -64;
}

// Axis (1,0,0) x edge: a = edge.z, b = edge.y.
FP ProjectX(const FPVector3& v, const FPVector3& edge) {
    return v.Y * edge.Z - v.Z * edge.Y;
}
// Axis (0,1,0) x edge: a = edge.z, b = edge.x.
FP ProjectY(const FPVector3& v, const FPVector3& edge) {
    return v.Z * edge.X + (-edge.Z) * v.X;
}
// Axis (0,0,1) x edge: a = edge.y, b = edge.x.
FP ProjectZ(const FPVector3& v, const FPVector3& edge) {
    return v.X * edge.Y - v.Y * edge.X;
}

} // namespace

// 0x7d75ca4
bool BoxTriangle::AxisTestX01(const CustomVertexTriangle& tri, const FPVector3& extents,
                              const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.Y == 0 && edge.Z == 0) {
        return false;
    }
    const FP radius = FPMath::Abs(extents.Z * fb + extents.Y * fa);
    return AxisOverlap(ProjectX(tri.v_0, edge), ProjectX(tri.v_2, edge), radius, penetrationRaw);
}

// 0x7d75edc
bool BoxTriangle::AxisTestX2(const CustomVertexTriangle& tri, const FPVector3& extents,
                             const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.Y == 0 && edge.Z == 0) {
        return false;
    }
    const FP radius = FPMath::Abs(extents.Z * fb + extents.Y * fa);
    return AxisOverlap(ProjectX(tri.v_0, edge), ProjectX(tri.v_1, edge), radius, penetrationRaw);
}

// 0x7d75d34
bool BoxTriangle::AxisTestY02(const CustomVertexTriangle& tri, const FPVector3& extents,
                              const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.Z == 0 && edge.X == 0) {
        return false;
    }
    const FP radius = extents.Z * fb + extents.X * fa;
    return AxisOverlap(ProjectY(tri.v_0, edge), ProjectY(tri.v_2, edge), radius, penetrationRaw);
}

// 0x7d75f6c
bool BoxTriangle::AxisTestY1(const CustomVertexTriangle& tri, const FPVector3& extents,
                             const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.Z == 0 && edge.X == 0) {
        return false;
    }
    const FP radius = extents.Z * fb + extents.X * fa;
    return AxisOverlap(ProjectY(tri.v_0, edge), ProjectY(tri.v_1, edge), radius, penetrationRaw);
}

// 0x7d75dcc
bool BoxTriangle::AxisTestZ12(const CustomVertexTriangle& tri, const FPVector3& extents,
                              const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.X == 0 && edge.Y == 0) {
        return false;
    }
    const FP radius = extents.Y * fb + extents.X * fa;
    return AxisOverlap(ProjectZ(tri.v_1, edge), ProjectZ(tri.v_2, edge), radius, penetrationRaw);
}

// 0x7d75e54
bool BoxTriangle::AxisTestZ0(const CustomVertexTriangle& tri, const FPVector3& extents,
                             const FPVector3& edge, FP fa, FP fb, FP::Raw& penetrationRaw) {
    if (edge.X == 0 && edge.Y == 0) {
        return false;
    }
    const FP radius = extents.Y * fb + extents.X * fa;
    return AxisOverlap(ProjectZ(tri.v_0, edge), ProjectZ(tri.v_1, edge), radius, penetrationRaw);
}

// 0x7d76000
bool BoxTriangle::PlaneBoxOverlaps(const FPVector3& triInverseNormal, const CustomVertexTriangle& tri,
                                   const FPVector3& extents, bool detailedManifold,
                                   FPVector3& deepestBoxVertex, FPVector3& deepestBoxVertexProj) {
    // The corner furthest along the normal (and, mirrored, the one furthest against it).
    deepestBoxVertex = extents;
    deepestBoxVertexProj = FPVector3::Zero();
    if (triInverseNormal.X < 0) {
        deepestBoxVertex.X = -deepestBoxVertex.X;
    }
    if (triInverseNormal.Y < 0) {
        deepestBoxVertex.Y = -deepestBoxVertex.Y;
    }
    if (triInverseNormal.Z < 0) {
        deepestBoxVertex.Z = -deepestBoxVertex.Z;
    }
    const FPVector3& v0 = tri.v_0;
    const FP maxDistance = FPVector3::Dot(triInverseNormal, deepestBoxVertex - v0);
    if (maxDistance < 0) {
        return false;
    }
    const FP minDistance = FPVector3::Dot(triInverseNormal, -(v0 + deepestBoxVertex));
    if (minDistance > 0) {
        return false;
    }
    if (detailedManifold) {
        deepestBoxVertexProj = deepestBoxVertex - triInverseNormal * maxDistance;
    }
    return true;
}

// 0x7d763d0
void BoxTriangle::ClipTriEdgesOnBoxPlanes(const NarrowEntry3D* boxEntry,
                                          const CustomVertexTriangle& tri,
                                          const FPVector3& boxExtents, CollisionManifold3D* m) {
    assert(boxEntry != nullptr && m != nullptr);
    const NarrowData3D& narrow = boxEntry->Narrow;
    const CustomLine triEdges[3] = {{tri.v_0, tri.v_1}, {tri.v_1, tri.v_2}, {tri.v_2, tri.v_0}};
    FPVector3 intersection = FPVector3::Zero();
    for (const CustomPlane& plane : narrow.BoxPlanes.Planes) {
        for (const CustomLine& edge : triEdges) {
            if (!ClipToPlane(plane, edge, intersection) || !PointInAabb(intersection, boxExtents)) {
                continue;
            }
            intersection = LocalToWorld(narrow, intersection);
            CollisionManifold3D::AddContactPoint(m, intersection);
            if (m->_contactPointsCount > 3) {
                return;
            }
        }
    }
}

// 0x7d77f18
bool BoxTriangle::ClipToPlane(const CustomPlane& plane, const CustomLine& line,
                              FPVector3& intersection) {
    const FPVector3 direction = line.end - line.start;
    const FP denominator = FPVector3::Dot(plane.normal, direction);
    // Parallel within FP.EN4 (raw 6).
    if (denominator.RawValue() >= -6 && denominator.RawValue() <= 6) {
        return false;
    }
    const FP fraction = FPVector3::Dot(plane.pointOnPlane - line.start, plane.normal) / denominator;
    if (fraction < 0 || fraction > FP::One()) {
        return false;
    }
    intersection = line.start + direction * fraction;
    return true;
}

// 0x7d76368
bool BoxTriangle::PointInAabb(const FPVector3& point, const FPVector3& extents) {
    return point.X <= extents.X && point.Y <= extents.Y && point.Y >= -extents.Y &&
           point.X >= -extents.X && point.Z <= extents.Z && point.Z >= -extents.Z;
}

// 0x7d77624
void BoxTriangle::ClipBoxEdgesOnTriPlane(const NarrowEntry3D* boxEntry, const FPVector3& triNormal,
                                         const CustomVertexTriangle& triVertex,
                                         CollisionManifold3D* m) {
    assert(boxEntry != nullptr && m != nullptr);
    const NarrowData3D& narrow = boxEntry->Narrow;
    const CustomPlane plane(triNormal, triVertex.v_0);
    FPVector3 intersection = FPVector3::Zero();
    for (const CustomLine& edge : narrow.BoxEdges.Edges) {
        if (!ClipToPlane(plane, edge, intersection) || !PointInTriangle(intersection, triVertex)) {
            continue;
        }
        intersection = LocalToWorld(narrow, intersection);
        CollisionManifold3D::AddContactPoint(m, intersection);
        if (m->_contactPointsCount > 3) {
            return;
        }
    }
}

// 0x7d77fd4
bool BoxTriangle::PointInTriangle(const FPVector3& p, const CustomVertexTriangle& tri) {
    const FPVector3 a = tri.v_0 - p;
    const FPVector3 b = tri.v_1 - p;
    const FPVector3 c = tri.v_2 - p;
    const FPVector3 u = FPVector3::Cross(b, c);
    const FPVector3 v = FPVector3::Cross(c, a);
    const FPVector3 w = FPVector3::Cross(a, b);
    if (FPVector3::Dot(u, v) < 0) {
        return false;
    }
    return FPVector3::Dot(u, w) >= 0;
}

// 0x7d7619c
FPVector3 BoxTriangle::ClosestPointOnTriangle(const TriangleCCW* tri, const FPVector3& pointOnPlane,
                                              const CustomVertexTriangle& customTri,
                                              const FPVector3& triInverseNormal,
                                              const Shape3D::BoxShape& box, bool& pointIsInsideTri) {
    (void)box;
    assert(tri != nullptr);
    pointIsInsideTri = false;
    FPVector3 closest = FPVector3::Zero();
    if (CheckForEdge(pointOnPlane, triInverseNormal, customTri.v_0, customTri.v_1, tri->LengthEdgeA,
                     closest) ||
        CheckForEdge(pointOnPlane, triInverseNormal, customTri.v_1, customTri.v_2, tri->LengthEdgeB,
                     closest) ||
        CheckForEdge(pointOnPlane, triInverseNormal, customTri.v_2, customTri.v_0, tri->LengthEdgeC,
                     closest)) {
        return closest;
    }
    pointIsInsideTri = true;
    return pointOnPlane;
}

// 0x7d78180
bool BoxTriangle::CheckForEdge(const FPVector3& pointOnPlane, const FPVector3& triInverseNormal,
                               const FPVector3& vertexA, const FPVector3& vertexB, FP edgeMagnitude,
                               FPVector3& closestPoint) {
    const FPVector3 edge = vertexB - vertexA;
    const FPVector3 side = FPVector3::Cross(pointOnPlane - vertexB, edge);
    if (FPVector3::Dot(side, triInverseNormal) <= 0) {
        closestPoint = FPVector3::Zero();
        return false; // inside this edge
    }
    const FPVector3 direction = edge * (FP::One() / edgeMagnitude);
    const FP along = FPVector3::Dot(direction, pointOnPlane - vertexA);
    if (along <= 0) {
        closestPoint = vertexA;
    } else if (along >= edgeMagnitude) {
        closestPoint = vertexB;
    } else {
        closestPoint = vertexA + direction * along;
    }
    return true;
}

// 0x7d762bc
FP::Raw BoxTriangle::PointPenetrationRaw(const FPVector3& point, const FPVector3& triInvertedNormal,
                                         const FPVector3& boxExtents) {
    FP::Raw result = RawMax;
    for (int axis = 0; axis < 3; ++axis) {
        const FP n = Component(triInvertedNormal, axis);
        if (n == 0) {
            continue;
        }
        const FP e = Component(boxExtents, axis);
        const FP p = Component(point, axis);
        // Distance to the face the normal points at, measured along the normal.
        const FP toFace = n > 0 ? e - p : -(e + p);
        const FP::Raw distance = (toFace / n).RawValue();
        result = distance < 0 ? 0 : (distance < result ? distance : result);
    }
    return result;
}

// 0x7d72058
bool BoxTriangle::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    assert(mInfo != nullptr && mInfo->EntryA != nullptr && mInfo->Tri != nullptr);
    const NarrowEntry3D& box = *mInfo->EntryA;
    const TriangleCCW* tri = mInfo->Tri;
    CollisionManifold3D* manifold = mInfo->Manifold;
    const Shape3D::BoxShape& boxShape = box.Shape->Box;
    const FPVector3 extents = boxShape._extents;
    const NarrowData3D& narrow = box.Narrow;

    // The triangle in the box's frame.
    const CustomVertexTriangle local = {narrow.RotationInverse * (tri->A - narrow.Position),
                                        narrow.RotationInverse * (tri->B - narrow.Position),
                                        narrow.RotationInverse * (tri->C - narrow.Position)};
    const FPVector3* corners[3] = {&local.v_0, &local.v_1, &local.v_2};

    // Box face axes: the triangle's bounds against the box.
    FP minimum[3];
    FP maximum[3];
    for (int axis = 0; axis < 3; ++axis) {
        minimum[axis] = maximum[axis] = Component(*corners[0], axis);
        for (int k = 1; k < 3; ++k) {
            const FP value = Component(*corners[k], axis);
            minimum[axis] = value < minimum[axis] ? value : minimum[axis];
            maximum[axis] = value > maximum[axis] ? value : maximum[axis];
        }
        const FP e = Component(extents, axis);
        if (minimum[axis] > e || maximum[axis] < -e) {
            return false;
        }
    }

    // Edge cross axes.
    FP::Raw penetrationRaw = RawMax;
    const FPVector3 edge0 = local.v_1 - local.v_0;
    const FPVector3 edge1 = local.v_2 - local.v_1;
    const FPVector3 edge2 = local.v_0 - local.v_2;
    {
        const FP fx = FPMath::Abs(edge0.X);
        const FP fy = FPMath::Abs(edge0.Y);
        const FP fz = FPMath::Abs(edge0.Z);
        if (AxisTestX01(local, extents, edge0, fz, fy, penetrationRaw) ||
            AxisTestY02(local, extents, edge0, fz, fx, penetrationRaw) ||
            AxisTestZ12(local, extents, edge0, fy, fx, penetrationRaw)) {
            return false;
        }
    }
    {
        const FP fx = FPMath::Abs(edge1.X);
        const FP fy = FPMath::Abs(edge1.Y);
        const FP fz = FPMath::Abs(edge1.Z);
        if (AxisTestX01(local, extents, edge1, fz, fy, penetrationRaw) ||
            AxisTestY02(local, extents, edge1, fz, fx, penetrationRaw) ||
            AxisTestZ0(local, extents, edge1, fy, fx, penetrationRaw)) {
            return false;
        }
    }
    {
        const FP fx = FPMath::Abs(edge2.X);
        const FP fy = FPMath::Abs(edge2.Y);
        const FP fz = FPMath::Abs(edge2.Z);
        if (AxisTestX2(local, extents, edge2, fz, fy, penetrationRaw) ||
            AxisTestY1(local, extents, edge2, fz, fx, penetrationRaw) ||
            AxisTestZ12(local, extents, edge2, fy, fx, penetrationRaw)) {
            return false;
        }
    }

    // The triangle's plane.
    const FPVector3 triInverseNormal = narrow.RotationInverse * tri->InverseNormal;
    FPVector3 deepestBoxVertex;
    FPVector3 deepestBoxVertexProj;
    if (!PlaneBoxOverlaps(triInverseNormal, local, extents, detailedManifold, deepestBoxVertex,
                          deepestBoxVertexProj)) {
        return false;
    }
    if (!detailedManifold) {
        return true;
    }

    // Penetration: the face-axis overlaps, the edge-axis overlaps and the depth of the
    // deepest corner (below the face, or how far it could move out of the box).
    for (int axis = 0; axis < 3; ++axis) {
        const FP e = Component(extents, axis);
        const FP lower = e - minimum[axis];
        const FP upper = e + maximum[axis];
        const FP::Raw overlap = (lower < upper ? lower : upper).RawValue();
        penetrationRaw = overlap < penetrationRaw ? overlap : penetrationRaw;
    }
    bool pointIsInsideTri = false;
    const FPVector3 closest = ClosestPointOnTriangle(tri, deepestBoxVertexProj, local, triInverseNormal,
                                                     boxShape, pointIsInsideTri);
    FP::Raw cornerPenetration;
    if (pointIsInsideTri) {
        const FPVector3 depth = deepestBoxVertex - deepestBoxVertexProj;
        cornerPenetration = FPMath::SqrtRaw(depth.SqrMagnitude().RawValue());
    } else {
        cornerPenetration = PointPenetrationRaw(closest, triInverseNormal, extents);
    }
    penetrationRaw = cornerPenetration < penetrationRaw ? cornerPenetration : penetrationRaw;
    if (penetrationRaw < 0) {
        return false;
    }
    assert(manifold != nullptr);
    manifold->Penetration = FP::FromRaw(penetrationRaw);

    // Contacts: the whole triangle when it fits in the (slightly grown) box.
    const FPVector3 grown = {extents.X + FP::_0_01(), extents.Y + FP::_0_01(),
                             extents.Z + FP::_0_01()};
    bool triangleInside = true;
    for (const FPVector3* corner : corners) {
        triangleInside = triangleInside && corner->X <= grown.X && corner->Y <= grown.Y &&
                         corner->Y >= -grown.Y && corner->X >= -grown.X && corner->Z <= grown.Z &&
                         corner->Z >= -grown.Z;
    }
    if (triangleInside) {
        CollisionManifold3D::AddContactPoint(manifold, tri->A);
        CollisionManifold3D::AddContactPoint(manifold, tri->B);
        CollisionManifold3D::AddContactPoint(manifold, tri->C);
    } else {
        ClipTriEdgesOnBoxPlanes(&box, local, grown, manifold);
        if (manifold->_contactPointsCount <= 3) {
            ClipBoxEdgesOnTriPlane(&box, triInverseNormal, local, manifold);
        }
    }
    if (manifold->_contactPointsCount == 0) {
        return false;
    }
    manifold->Normal = tri->InverseNormal;
    return true;
}

} // namespace Quantum::Physics3D::NarrowChecks
