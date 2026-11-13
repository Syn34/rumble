// narrow_checks.cpp - Quantum.Physics3D.NarrowChecks.
#include "narrow_checks.h"

#include "quantum_core/gjk_settings.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <cassert>

namespace Quantum::Physics3D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

// The shape-pair dispatch of NarrowChecks.Check (0x7d7071c).
bool CheckShapes(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    assert(mInfo != nullptr && mInfo->EntryA != nullptr && mInfo->EntryB != nullptr);
    using T = Shape3DType;
    const Entry3D* entryA = mInfo->EntryA;
    const T a = entryA->Shape->Type();
    const T b = mInfo->EntryB->Shape->Type();
    auto is = [&](T first, T second) { return a == first && b == second; };

    if (is(T::Sphere, T::Sphere)) {
        return SphereSphere::Check(mInfo, detailedManifold);
    }
    if (is(T::Sphere, T::Box)) {
        return SphereBox::Check(mInfo, detailedManifold);
    }
    if (is(T::Box, T::Sphere)) {
        mInfo->InvertEntries();
        return SphereBox::Check(mInfo, detailedManifold);
    }
    // The smooth-collision flag is read from the mesh entry.
    if (is(T::Sphere, T::Mesh)) {
        return SphereTriangle::Check(mInfo, detailedManifold, mInfo->EntryB->UseSmoothSphereMeshCollision);
    }
    if (is(T::Mesh, T::Sphere)) {
        mInfo->InvertEntries();
        return SphereTriangle::Check(mInfo, detailedManifold, entryA->UseSmoothSphereMeshCollision);
    }
    if (is(T::Box, T::Box)) {
        return BoxBox::Check(mInfo, detailedManifold);
    }
    if (is(T::Box, T::Mesh)) {
        return BoxTriangle::Check(mInfo, detailedManifold);
    }
    if (is(T::Mesh, T::Box)) {
        mInfo->InvertEntries();
        return BoxTriangle::Check(mInfo, detailedManifold);
    }
    return false;
}

// 0x7d7071c
bool Check(NarrowTls3D& tls, const CCDSettings& settings, FP deltaTime,
           CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    const bool overlap = CheckShapes(mInfo, detailedManifold);
    if (overlap || !mInfo->Manifold->_m.GetUsesCcd()) {
        return overlap;
    }
    FP timeOfImpact;
    return Gjk3D::ComputeTimeOfImpact(tls, mInfo, settings, detailedManifold, deltaTime,
                                      timeOfImpact) == TimeOfImpactResult::Collision;
}

// 0x7d790cc
bool SphereSphere::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    const Entry3D& entryA = *mInfo->EntryA;
    const Entry3D& entryB = *mInfo->EntryB;
    const FP radiusA = entryA.Shape->Sphere.Radius;
    const FP radii = entryB.Shape->Sphere.Radius + radiusA;
    const FPVector3 posA = entryA.Narrow.Position;
    const FPVector3 delta = entryB.Narrow.Position - posA;
    const FP distanceSquared = FPVector3::Dot(delta, delta);
    const bool overlap = distanceSquared < radii * radii;

    if (overlap && detailedManifold) {
        CollisionManifold3D* manifold = mInfo->Manifold;
        const FP distance = FPMath::Sqrt(distanceSquared);
        FPVector3 contact;
        if (distance > fpdetail::Epsilon) {
            const FPVector3 normal = {delta.X / distance, delta.Y / distance, delta.Z / distance};
            manifold->Penetration = radii - distance;
            manifold->Normal = normal;
            contact = {posA.X + normal.X * radiusA, posA.Y + normal.Y * radiusA,
                       posA.Z + normal.Z * radiusA};
        } else {
            // Concentric: any direction will do.
            manifold->Penetration = radiusA;
            manifold->Normal = FPVector3::Right();
            contact = posA;
        }
        CollisionManifold3D::AddContactPoint(manifold, contact);
    }
    return overlap;
}

// 0x7d78bec
bool SphereBox::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold) {
    const Entry3D& sphere = *mInfo->EntryA;
    const Entry3D& box = *mInfo->EntryB;
    const FP radius = sphere.Shape->Sphere.Radius;
    const NarrowData3D& frame = box.Narrow;
    const FPVector3 axes[3] = {frame.RotatedRight, frame.RotatedUp, frame.RotatedForward};
    const FPVector3 offset = sphere.Narrow.Position - frame.Position;
    const FP local[3] = {FPVector3::Dot(axes[0], offset), FPVector3::Dot(axes[1], offset),
                         FPVector3::Dot(axes[2], offset)};
    const FPVector3 e = box.Shape->Box._extents;
    const FP extents[3] = {e.X, e.Y, e.Z};

    // Clamp the centre into the box, axis by axis.
    FP closest[3];
    FP delta[3]; // closest - centre
    FP distanceSquared = FP::Zero();
    bool inside = true;
    for (int i = 0; i < 3; ++i) {
        if (-extents[i] - local[i] > 0) {
            closest[i] = -extents[i];
            delta[i] = -extents[i] - local[i];
            inside = false;
        } else if (extents[i] - local[i] < 0) {
            closest[i] = extents[i];
            delta[i] = extents[i] - local[i];
            inside = false;
        } else {
            closest[i] = local[i];
            delta[i] = FP::Zero();
        }
        distanceSquared += delta[i] * delta[i];
    }

    CollisionManifold3D* manifold = mInfo->Manifold;
    if (!inside) {
        const FP penetration = radius - FPMath::Sqrt(distanceSquared);
        if (penetration < 0) {
            return false;
        }
        if (!detailedManifold) {
            return true;
        }
        CollisionManifold3D::AddContactPoint(
            manifold, frame.Position + axes[0] * closest[0] + axes[1] * closest[1] + axes[2] * closest[2]);
        manifold->Penetration = penetration;
        const FPVector3 n = FPVector3::Normalize({delta[0], delta[1], delta[2]});
        manifold->Normal = axes[0] * n.X + axes[1] * n.Y + axes[2] * n.Z;
        return true;
    }
    if (!detailedManifold) {
        return true;
    }

    // Centre inside: leave through the nearest face (positive faces first; between +X and
    // +Y a tie picks +Y, any later tie keeps the earlier face).
    int axis = (extents[0] - local[0] < extents[1] - local[1]) ? 0 : 1;
    FP nearest = extents[axis] - local[axis];
    if (extents[2] - local[2] < nearest) {
        axis = 2;
        nearest = extents[2] - local[2];
    }
    bool positiveFace = true;
    for (int i = 0; i < 3; ++i) {
        if (local[i] + extents[i] < nearest) {
            axis = i;
            nearest = local[i] + extents[i];
            positiveFace = false;
        }
    }
    const FPVector3 normal = positiveFace ? -axes[axis] : axes[axis];
    manifold->Penetration = nearest + radius;
    manifold->Normal = normal;
    const FPVector3 position = sphere.Narrow.Position;
    CollisionManifold3D::AddContactPoint(
        manifold, {position.X + FP::FromRaw(normal.X.RawValue() >> 8),
                   position.Y + FP::FromRaw(normal.Y.RawValue() >> 8),
                   position.Z + FP::FromRaw(normal.Z.RawValue() >> 8)});
    return true;
}

// 0x7d79290
bool SphereTriangle::Check(CollisionManifoldInfo3D* mInfo, bool detailedManifold,
                           bool smoothSphereMeshCollision) {
    const Entry3D& sphere = *mInfo->EntryA;
    const FPVector3 center = sphere.Narrow.Position;
    const FP radius = sphere.Shape->Sphere.Radius;
    const TriangleCCW* tri = mInfo->Tri;
    FPVector3 nearest;
    FPVector3 onPlane;
    const bool touches = GetNearestPoint(tri, center, radius.RawValue(), nearest, onPlane);
    if (!touches || !detailedManifold) {
        return touches;
    }
    CollisionManifold3D* manifold = mInfo->Manifold;
    manifold->Penetration = radius - FPVector3::Distance(nearest, center);
    if (manifold->Penetration < 0) {
        return false;
    }
    CollisionManifold3D::AddContactPoint(manifold, smoothSphereMeshCollision ? onPlane : nearest);
    manifold->Normal = -tri->Normal;
    return true;
}

// 0x7d79418
bool SphereTriangle::GetNearestPoint(const TriangleCCW* tri, const FPVector3& center, FP::Raw radiusRaw,
                                     FPVector3& nearestPoint, FPVector3& pointOnPlane) {
    const FP radius = FP::FromRaw(radiusRaw);
    const FP height = FPVector3::Dot(tri->Normal, center - tri->Center);
    if (FPMath::Abs(height) > radius) {
        nearestPoint = FPVector3::Zero();
        pointOnPlane = FPVector3::Zero();
        return false;
    }
    pointOnPlane = center - tri->Normal * height;
    const FP radiusSquared = radius * radius;
    // Outside one of the edges: the nearest point is on that edge.
    struct Edge {
        const FPVector3& a;
        const FPVector3& b;
        const FPVector3& vector;
        FP length;
    };
    const Edge edges[3] = {{tri->A, tri->B, tri->EdgeA, tri->LengthEdgeA},
                           {tri->B, tri->C, tri->EdgeB, tri->LengthEdgeB},
                           {tri->C, tri->A, tri->EdgeC, tri->LengthEdgeC}};
    for (const Edge& edge : edges) {
        const FPVector3 direction = FPVector3::Normalize(edge.vector);
        const std::optional<bool> result = CheckForVertex(tri, pointOnPlane, edge.a, edge.b, direction,
                                                          edge.length, center,
                                                          radiusSquared.RawValue(), nearestPoint);
        if (result.has_value()) {
            return *result;
        }
    }
    nearestPoint = pointOnPlane;
    return true;
}

// 0x7d796d8
std::optional<bool> SphereTriangle::CheckForVertex(const TriangleCCW* tri, const FPVector3& projected,
                                                   const FPVector3& a, const FPVector3& b,
                                                   const FPVector3& direction, FP length,
                                                   const FPVector3& p, FP::Raw radiusSqr,
                                                   FPVector3& nearestPoint) {
    const FPVector3 side = FPVector3::Cross(direction, projected - b);
    if (FPVector3::Dot(tri->Normal, side) <= 0) {
        nearestPoint = FPVector3::Zero();
        return std::nullopt; // inside this edge
    }
    const FP along = FPVector3::Dot(projected - a, direction);
    if (along <= 0) {
        nearestPoint = a;
    } else if (along >= length) {
        nearestPoint = b;
    } else {
        nearestPoint = a + direction * along;
    }
    return FPVector3::DistanceSquared(nearestPoint, p).RawValue() <= radiusSqr;
}

// 0x7d3a898
FPVector3 TriangleCollisionIterator::AverageNormal() const {
    const CollisionManifold3D* tri = &_triManifolds[_meshManifold->_m.NextInMesh];
    FPVector3 sum = tri->Normal;
    std::int32_t next = tri->_m.NextInMesh;
    if (next != 0) {
        std::int32_t count = 1;
        while (next != 0) {
            tri = &_triManifolds[next];
            sum = sum + tri->Normal;
            ++count;
            next = tri->_m.NextInMesh;
        }
        sum = {fpdetail::DivInt(sum.X, count), fpdetail::DivInt(sum.Y, count),
               fpdetail::DivInt(sum.Z, count)};
    }
    return -sum;
}

// 0x7d78710
bool LinecastBody::RaySphereProjection(const FPVector3& center, FP radius, const FPVector3& start,
                                       const FPVector3& end, FPVector3& intersection,
                                       FP& normalizedDist) {
    auto miss = [&]() {
        intersection = FPVector3::Zero();
        normalizedDist = FP::Zero();
        return false;
    };
    const FPVector3 fromCenter = start - center;
    const FP radiusSquared = radius * radius;
    if (fromCenter.SqrMagnitude() < radiusSquared) {
        return miss();
    }
    const FPVector3 segment = end - start;
    const FP lengthSquared = segment.SqrMagnitude();
    if (lengthSquared == 0) {
        return miss();
    }
    const FP towards = FPVector3::Dot(-fromCenter, segment);
    if (towards < 0) {
        return miss();
    }
    const FPVector3 closest = start + segment * (towards / lengthSquared);
    const FP inside = radiusSquared - (center - closest).SqrMagnitude();
    if (inside < 0) {
        return miss();
    }
    normalizedDist = (towards - FPMath::Sqrt(inside * lengthSquared)) / lengthSquared;
    if (normalizedDist > FP::One()) {
        return miss();
    }
    intersection = start + segment * normalizedDist;
    return true;
}

// 0x7d7889c
bool LinecastBody::HitBoundingBox(const FPVector3& minB, const FPVector3& maxB, const FPVector3& origin,
                                  const FPVector3& dir, FPVector3& coord, FP::Raw maxDistanceRaw,
                                  FPVector3& normal, FP& normalizedDist) {
    auto miss = [&]() {
        coord = FPVector3::Zero();
        normal = FPVector3::Zero();
        normalizedDist = FP::Zero();
        return false;
    };
    const FP o[3] = {origin.X, origin.Y, origin.Z};
    const FP lo[3] = {minB.X, minB.Y, minB.Z};
    const FP hi[3] = {maxB.X, maxB.Y, maxB.Z};
    const FP d[3] = {dir.X, dir.Y, dir.Z};
    // Candidate entry plane per axis (none when the origin lies between the slabs).
    bool middle[3];
    FP candidate[3];
    for (int i = 0; i < 3; ++i) {
        if (o[i] < lo[i]) {
            middle[i] = false;
            candidate[i] = lo[i];
        } else if (o[i] > hi[i]) {
            middle[i] = false;
            candidate[i] = hi[i];
        } else {
            middle[i] = true;
            candidate[i] = FP::Zero();
        }
    }
    if (middle[0] && middle[1] && middle[2]) {
        miss();
        coord = origin; // starts inside
        return false;
    }
    FP maxT[3];
    for (int i = 0; i < 3; ++i) {
        maxT[i] = (!middle[i] && d[i] != 0) ? (candidate[i] - o[i]) / d[i] : -FP::One();
    }
    // The furthest candidate plane is the one hit (ties: x over y, the earlier of
    // the first two over z).
    const FP firstTwo = maxT[0] > maxT[1] ? maxT[0] : maxT[1];
    int plane = maxT[0] < maxT[1] ? 1 : 0;
    const FP t = firstTwo > maxT[2] ? firstTwo : maxT[2];
    if (!(firstTwo >= maxT[2])) {
        plane = 2;
    }
    if (t < 0) {
        return miss();
    }
    FP c[3];
    for (int i = 0; i < 3; ++i) {
        c[i] = i == plane ? candidate[i] : o[i] + d[i] * t;
        if (c[i] < lo[i] || c[i] > hi[i]) {
            return miss();
        }
    }
    coord = {c[0], c[1], c[2]};
    const FP maxDistance = FP::FromRaw(maxDistanceRaw);
    if ((origin - coord).SqrMagnitude() > maxDistance * maxDistance) {
        return miss();
    }
    switch (plane) {
    case 0:
        normal = o[0] > c[0] ? FPVector3::Right() : FPVector3::Left();
        break;
    case 1:
        normal = o[1] > c[1] ? FPVector3::Up() : FPVector3::Down();
        break;
    default:
        normal = o[2] > c[2] ? FPVector3::Forward() : FPVector3::Back();
        break;
    }
    normalizedDist = t / maxDistance;
    return true;
}

// 0x7d782c8
bool LinecastBody::Check(AssetGuid mapGuid, const Entry3D* b, const FPVector3& start,
                         const FPVector3& end, bool computeNormal, Hit3D& hit) {
    hit = Hit3D{};
    auto identify = [&]() {
        hit._staticColliderIndex = b->StaticColliderIndex;
        if (b->StaticColliderIndex < 0) {
            hit._entity = {b->Ref.Index, b->Ref.Version};
        } else {
            hit._mapGuid = mapGuid;
        }
        hit.ShapeUserTag = b->Shape->UserTag;
        hit._isTrigger = b->IsTriggerCollider() ? 1 : 0;
    };
    const NarrowData3D& frame = b->Narrow;
    switch (b->Shape->Type()) {
    case Shape3DType::Sphere:
        if (!RaySphereProjection(frame.Position, b->Shape->Sphere.Radius, start, end, hit.Point,
                                 hit.CastDistanceNormalized)) {
            return false;
        }
        identify();
        if (computeNormal) {
            hit.Normal = FPVector3::Normalize(hit.Point - frame.Position);
        }
        return true;
    case Shape3DType::Box: {
        // In the box's frame.
        const FPVector3 offset = start - frame.Position;
        const FPVector3 localStart = {FPVector3::Dot(frame.RotatedRight, offset),
                                      FPVector3::Dot(frame.RotatedUp, offset),
                                      FPVector3::Dot(frame.RotatedForward, offset)};
        const FPVector3 extents = b->Shape->Box._extents;
        FP length;
        const FPVector3 dir = FPVector3::Normalize(end - start, length);
        const FPVector3 localDir = {FPVector3::Dot(frame.RotatedRight, dir), FPVector3::Dot(frame.RotatedUp, dir),
                                    FPVector3::Dot(frame.RotatedForward, dir)};
        FPVector3 coord;
        FPVector3 normal;
        if (!HitBoundingBox(-extents, extents, localStart, localDir, coord, length.RawValue(), normal,
                            hit.CastDistanceNormalized)) {
            return false;
        }
        identify();
        hit.Point = frame.Position + frame.RotatedRight * coord.X + frame.RotatedUp * coord.Y +
                    frame.RotatedForward * coord.Z;
        if (computeNormal) {
            hit.Normal = frame.RotatedRight * normal.X + frame.RotatedUp * normal.Y +
                         frame.RotatedForward * normal.Z;
        }
        return true;
    }
    default:
        return false;
    }
}

} // namespace Quantum::Physics3D::NarrowChecks
