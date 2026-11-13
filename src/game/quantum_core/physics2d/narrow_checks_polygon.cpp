// narrow_checks_polygon.cpp - Quantum.Physics2D.NarrowChecks: the checks involving convex
// polygons (boxes count as polygons here): Polygon, CirclePolygon, PolygonPolygon and
// PolygonEdge.
#include "narrow_checks.h"
#include "narrow_checks_detail.h"

#include "photon_deterministic/fp_collision.h"
#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <cassert>

namespace Quantum::Physics2D::NarrowChecks {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;
namespace FPCollision = Photon::Deterministic::FPCollision;
using detail::ToLocal;

namespace {

// Separation (FP.Epsilon, raw 65) below which a circle centre counts as inside a polygon.
constexpr FP InsideTolerance = fpdetail::Epsilon;

// Reference-face selection bias of the polygon clipper: B's face is preferred unless A's
// is clearly better (penA >= 0.95 * penB + 0.01 * penA).
constexpr FP BiasRelative = FP::FromRaw(62261); // 0.95
constexpr FP BiasAbsolute = FP::FromRaw(655);   // 0.01

// Raw 64 (just under FP.Epsilon): how far apart two polygon sides must be for the two
// contact halves of PolygonEdge to be measured separately.
constexpr FP SideTolerance = FP::FromRaw(64);

CollisionManifold2D& ManifoldOf(const CollisionManifoldInfo2D* mInfo) {
    assert(mInfo->Manifold != nullptr);
    return *mInfo->Manifold;
}

// The polygon vertex furthest along `direction`. Ties keep the highest index (the search
// runs from the last vertex down); an empty polygon gives the origin.
FPVector2 SupportPoint(const Polygon& polygon, FPVector2 direction) {
    FPVector2 support;
    FP best = FP::MinValue();
    for (std::int32_t i = polygon.VerticesCount - 1; i >= 0; --i) {
        const FP projection = FPVector2::Dot(direction, polygon.Vertices[i]);
        if (projection > best) {
            best = projection;
            support = polygon.Vertices[i];
        }
    }
    return support;
}

FPVector2 ToWorld(const Polygon& polygon, FPVector2 local) {
    return FPVector2::Rotate(local, polygon.RotSin, polygon.RotCos) + polygon.Position;
}

} // namespace

// 0x7d17380
void Polygon::Set(NarrowTls2D& tls, const NarrowEntry2D& entry) {
    const Shape2D& shape = *entry.Shape;
    switch (shape.Type()) {
    case Shape2DType::Polygon:
        Vertices = entry.Polygon.Vertices;
        Normals = entry.Polygon.Normals;
        Radius = shape.BroadRadius;
        VerticesCount = entry.Polygon.VerticesCount;
        NormalsCount = entry.Polygon.NormalsCount;
        break;
    case Shape2DType::Box: {
        Radius = shape.BroadRadius;
        VerticesCount = 4;
        NormalsCount = 4;
        OOBB box;
        box.Set(shape.Box._extents);
        tls.BoxVertices[0] = box.UL;
        tls.BoxVertices[1] = box.LL;
        tls.BoxVertices[2] = box.LR;
        tls.BoxVertices[3] = box.UR;
        tls.BoxNormals[0] = FPVector2::Left();
        tls.BoxNormals[1] = FPVector2::Down();
        tls.BoxNormals[2] = FPVector2::Right();
        tls.BoxNormals[3] = FPVector2::Up();
        Vertices = tls.BoxVertices;
        Normals = tls.BoxNormals;
        break;
    }
    default:
        return;
    }
    Position = entry.Narrow.Position;
    RotSin = entry.Narrow.RotSin;
    RotCos = entry.Narrow.RotCos;
}

// 0x7d14dd4
bool CirclePolygon::Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo,
                          bool detailedManifold) {
    const NarrowEntry2D& circle = *mInfo->EntryA;
    const FPVector2 centre = circle.Narrow.Position;
    const FP radius = circle.Shape->Circle.Radius;
    Polygon polygon;
    polygon.Set(tls, *mInfo->EntryB);
    const FP sin = polygon.RotSin;
    const FP cos = polygon.RotCos;
    const FPVector2 local = ToLocal(centre - polygon.Position, sin, cos);

    // Face of greatest separation; any face further than the radius separates them.
    FP separation = FP::MinValue();
    std::int32_t face = 0;
    for (std::int32_t i = 0; i < polygon.VerticesCount; ++i) {
        const FP s = FPVector2::Dot(polygon.Normals[i], local - polygon.Vertices[i]);
        if (s > radius) {
            return false;
        }
        if (s > separation) {
            separation = s;
            face = i;
        }
    }

    if (separation < InsideTolerance) {
        // Centre inside the polygon: push out through the nearest face.
        if (detailedManifold) {
            CollisionManifold2D& manifold = ManifoldOf(mInfo);
            const FPVector2 normal = FPVector2::Rotate(-polygon.Normals[face], sin, cos);
            manifold.Normal = normal;
            CollisionManifold2D::AddContactPoint(&manifold, centre + normal * radius);
            manifold.Penetration = radius;
        }
        return true;
    }

    const FPVector2 v1 = polygon.Vertices[face];
    const FPVector2 v2 = polygon.Vertices[(face + 1) % polygon.VerticesCount];
    const FP radiusSquared = radius * radius;

    // Beyond either end of the face: the nearest vertex is the contact.
    auto touchVertex = [&](FPVector2 vertex) {
        if (FPVector2::DistanceSquared(local, vertex) > radiusSquared) {
            return false;
        }
        if (detailedManifold) {
            CollisionManifold2D& manifold = ManifoldOf(mInfo);
            manifold.Normal = FPVector2::Normalize(FPVector2::Rotate(vertex - local, sin, cos));
            CollisionManifold2D::AddContactPoint(&manifold, ToWorld(polygon, vertex));
            manifold.Penetration = radius - separation;
        }
        return true;
    };
    if (FPVector2::Dot(local - v1, v2 - v1) <= 0) {
        return touchVertex(v1);
    }
    if (FPVector2::Dot(local - v2, v1 - v2) <= 0) {
        return touchVertex(v2);
    }

    const FPVector2 faceNormal = polygon.Normals[face];
    if (FPVector2::Dot(faceNormal, local - v1) > radius) {
        return false;
    }
    if (detailedManifold) {
        CollisionManifold2D& manifold = ManifoldOf(mInfo);
        const FPVector2 normal = FPVector2::Rotate(-faceNormal, sin, cos);
        manifold.Normal = normal;
        CollisionManifold2D::AddContactPoint(&manifold, centre + normal * separation);
        manifold.Penetration = radius - separation;
    }
    return true;
}

// 0x7d16db8
FP PolygonPolygon::FindAxisLeastPenetration(std::int32_t& faceIndex, const Polygon& a,
                                            const Polygon& b) {
    FP best = FP::MinValue();
    std::int32_t bestFace = 0;
    for (std::int32_t i = a.VerticesCount - 1; i >= 0; --i) {
        // a's face normal and vertex, in b's frame.
        const FPVector2 normal =
            ToLocal(FPVector2::Rotate(a.Normals[i], a.RotSin, a.RotCos), b.RotSin, b.RotCos);
        const FPVector2 vertex = ToLocal(ToWorld(a, a.Vertices[i]) - b.Position, b.RotSin, b.RotCos);
        const FPVector2 support = SupportPoint(b, -normal);
        const FP distance = FPVector2::Dot(normal, support - vertex);
        if (distance > best) {
            best = distance;
            bestFace = i;
        }
    }
    faceIndex = bestFace;
    return best;
}

// 0x7d16f64
void PolygonPolygon::FindIncidentFace(FPVector2 (&face)[2], const Polygon& refPoly,
                                      const Polygon& incPoly, std::int32_t referenceIndex) {
    const FPVector2 referenceNormal =
        ToLocal(FPVector2::Rotate(refPoly.Normals[referenceIndex], refPoly.RotSin, refPoly.RotCos),
                incPoly.RotSin, incPoly.RotCos);
    std::int32_t incident = 0;
    FP minDot = FP::MaxValue();
    for (std::int32_t i = incPoly.VerticesCount - 1; i >= 0; --i) {
        const FP dot = FPVector2::Dot(incPoly.Normals[i], referenceNormal);
        if (dot < minDot) {
            minDot = dot;
            incident = i;
        }
    }
    face[0] = ToWorld(incPoly, incPoly.Vertices[incident]);
    const std::int32_t next = incident + 1 >= incPoly.VerticesCount ? 0 : incident + 1;
    face[1] = ToWorld(incPoly, incPoly.Vertices[next]);
}

// 0x7d170bc
std::int32_t PolygonPolygon::Clip(const FPVector2& n, FP c, FPVector2 (&face)[2]) {
    FPVector2 out[2] = {face[0], face[1]};
    std::int32_t count = 0;
    const FP d1 = FPVector2::Dot(n, face[0]) - c;
    const FP d2 = FPVector2::Dot(n, face[1]) - c;
    if (d1 <= 0) {
        out[count++] = face[0];
    }
    if (d2 <= 0) {
        out[count++] = face[1];
    }
    if (d1 * d2 < 0) {
        // The segment crosses the plane: keep the crossing point.
        const FP alpha = d1 / (d1 - d2);
        out[count++] = face[0] + (face[1] - face[0]) * alpha;
    }
    face[0] = out[0];
    face[1] = out[1];
    return count;
}

// 0x7d14a4c
bool PolygonPolygon::Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo,
                           bool detailedManifold) {
    Polygon a;
    a.Set(tls, *mInfo->EntryA);
    Polygon b;
    b.Set(tls, *mInfo->EntryB);
    if (!FPCollision::CircleIntersectsCircle(a.Position, a.Radius, b.Position, b.Radius)) {
        return false;
    }
    std::int32_t faceA = 0;
    const FP penetrationA = FindAxisLeastPenetration(faceA, a, b);
    if (penetrationA >= 0) {
        return false;
    }
    std::int32_t faceB = 0;
    const FP penetrationB = FindAxisLeastPenetration(faceB, b, a);
    if (penetrationB >= 0) {
        return false;
    }

    const bool flip = !(penetrationA >= penetrationB * BiasRelative + penetrationA * BiasAbsolute);
    const Polygon& reference = flip ? b : a;
    const Polygon& incident = flip ? a : b;
    const std::int32_t referenceIndex = flip ? faceB : faceA;

    FPVector2 incidentFace[2];
    FindIncidentFace(incidentFace, reference, incident, referenceIndex);

    const std::int32_t nextIndex =
        referenceIndex + 1 == reference.VerticesCount ? 0 : referenceIndex + 1;
    const FPVector2 rotated1 =
        FPVector2::Rotate(reference.Vertices[referenceIndex], reference.RotSin, reference.RotCos);
    const FPVector2 rotated2 =
        FPVector2::Rotate(reference.Vertices[nextIndex], reference.RotSin, reference.RotCos);
    const FPVector2 sidePlaneNormal = FPVector2::Normalize(rotated2 - rotated1);
    const FPVector2 v1 = rotated1 + reference.Position;
    const FPVector2 v2 = rotated2 + reference.Position;

    // Clip the incident face to the reference face's extent.
    if (Clip(-sidePlaneNormal, -FPVector2::Dot(sidePlaneNormal, v1), incidentFace) < 2) {
        return false;
    }
    if (Clip(sidePlaneNormal, FPVector2::Dot(sidePlaneNormal, v2), incidentFace) < 2) {
        return false;
    }
    if (!detailedManifold) {
        return true;
    }

    CollisionManifold2D& manifold = ManifoldOf(mInfo);
    const FPVector2 referenceNormal = {sidePlaneNormal.Y, -sidePlaneNormal.X};
    manifold.Normal = flip ? -referenceNormal : referenceNormal;
    const FP referenceOffset = FPVector2::Dot(referenceNormal, v1);

    // Keep the clipped points behind the reference face; the penetration is their mean
    // depth (the second depth is added to whatever the manifold held if the first point
    // was dropped, as in the original).
    std::int32_t divisor = 1;
    const FP separation0 = referenceOffset - FPVector2::Dot(referenceNormal, incidentFace[0]);
    if (separation0 >= 0) {
        CollisionManifold2D::AddContactPoint(&manifold, incidentFace[0]);
        manifold.Penetration = separation0;
        divisor = 2;
    }
    const FP separation1 = referenceOffset - FPVector2::Dot(referenceNormal, incidentFace[1]);
    if (separation1 >= 0) {
        CollisionManifold2D::AddContactPoint(&manifold, incidentFace[1]);
        manifold.Penetration = fpdetail::DivInt(manifold.Penetration + separation1, divisor);
    }
    return manifold._contactPointsCount >= 1;
}

// 0x7d16c08
FP PolygonEdge::CalculatePenetrationOnPolygonHalf(const Polygon& polygon, std::int32_t initVertex,
                                                  std::int32_t endVertexUnclamped,
                                                  const FPVector2& cp1, const FPVector2& cp2,
                                                  const FPVector2& edgeNormal,
                                                  FPVector2& avgNormal) {
    const std::int32_t count = polygon.VerticesCount;
    const std::int32_t previousSide = (initVertex > 0 ? initVertex : count) - 1;
    const FP firstLength = FPVector2::Distance(polygon.Vertices[initVertex], cp1);
    avgNormal = polygon.Normals[previousSide] * firstLength;

    FP deepest = FP::Zero();
    for (std::int32_t i = initVertex; i <= endVertexUnclamped; ++i) {
        const std::int32_t index = i % count;
        const FPVector2 vertex = polygon.Vertices[index];
        const FPVector2 next =
            i == endVertexUnclamped ? cp2 : polygon.Vertices[(index + 1) % count];
        const FP length = FPVector2::Distance(vertex, next);
        avgNormal = avgNormal + polygon.Normals[index] * length;
        const FP depth = FPMath::Abs(FPVector2::Dot(vertex - cp1, edgeNormal));
        if (depth > deepest) {
            deepest = depth;
        }
    }
    avgNormal = FPVector2::Normalize(avgNormal);
    return deepest;
}

// 0x7d151cc
bool PolygonEdge::Check(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo, bool detailedManifold) {
    Polygon polygon;
    polygon.Set(tls, *mInfo->EntryA);
    const NarrowEntry2D& edge = *mInfo->EntryB;
    CollisionManifold2D& manifold = ManifoldOf(mInfo);
    const FP extent = edge.Shape->Edge.Extent;
    const FP sinP = polygon.RotSin;
    const FP cosP = polygon.RotCos;
    const FP sinE = edge.Narrow.RotSin;
    const FP cosE = edge.Narrow.RotCos;

    // Everything below is in the polygon's frame. `direction` points from the edge centre
    // towards its first endpoint.
    const FPVector2 direction = {(-cosE) * cosP - sinP * sinE, sinP * cosE - cosP * sinE};
    const FPVector2 centre = ToLocal(edge.Narrow.Position - polygon.Position, sinP, cosP);
    const FPVector2 half = {direction.X * extent, direction.Y * extent};
    const FPVector2 p1 = centre + half;
    const FPVector2 p2 = centre - half;
    const FPVector2 edgeNormal = {-direction.Y, direction.X};

    // Quick rejection against the polygon's bounding circle.
    if (FPMath::Sqrt(p1.SqrMagnitude()) > polygon.Radius &&
        FPMath::Sqrt(p2.SqrMagnitude()) > polygon.Radius &&
        !FPCollision::LineIntersectsCircle(p1, p2, FPVector2::Zero(), polygon.Radius, true)) {
        return false;
    }

    // Intersect the edge with every polygon side (prev -> cur). Until a real contact is
    // found the edge is extended by the polygon's diameter on both ends, so a side crossed
    // only by the extension still tells which side the edge lies on.
    FPVector2 contact1; // local contact points
    FPVector2 contact2;
    std::int32_t firstCur = 0; // sides of the first contact
    std::int32_t firstPrev = 0;
    std::int32_t cur = 0; // sides of the last intersection that ended the search
    std::int32_t prev = 0;
    const std::int32_t count = polygon.VerticesCount;
    if (count >= 1) {
        const FPVector2 extension = {fpdetail::TwiceProduct(direction.X, polygon.Radius),
                                     fpdetail::TwiceProduct(direction.Y, polygon.Radius)};
        bool crossedBehind = false; // an extension hit at or behind the edge centre
        bool crossedAhead = false;  // an extension hit beyond it, towards p1
        bool stopped = false;
        for (std::int32_t i = 1; i <= count && !stopped; ++i) {
            cur = i % count;
            prev = (cur > 0 ? cur : count) - 1;
            const FPVector2 grow = manifold._contactPointsCount > 0 ? FPVector2::Zero() : extension;
            FPVector2 point;
            FP distance;
            FP normalizedDist;
            if (!FPCollision::LineIntersectsLine(polygon.Vertices[cur], polygon.Vertices[prev],
                                                 p1 + grow, p2 - grow, point, distance,
                                                 normalizedDist)) {
                continue;
            }
            if (manifold._contactPointsCount <= 0 && FPVector2::Distance(point, centre) > extent) {
                // Only the extension reaches this side.
                if (!crossedBehind && !crossedAhead) {
                    firstCur = cur;
                    firstPrev = prev;
                    contact1 = point;
                    manifold.Normal = polygon.Normals[prev];
                }
                const FP along = FPVector2::Dot(direction, point - centre);
                crossedAhead = crossedAhead || along > 0;
                crossedBehind = crossedBehind || along <= 0;
                if (crossedAhead && crossedBehind) {
                    // The whole edge lies inside the polygon.
                    contact2 = point;
                    CollisionManifold2D::AddContactPoint(&manifold, ToWorld(polygon, p1));
                    CollisionManifold2D::AddContactPoint(&manifold, ToWorld(polygon, p2));
                    stopped = true;
                }
                continue;
            }
            if (!detailedManifold) {
                return true;
            }
            CollisionManifold2D::AddContactPoint(&manifold, ToWorld(polygon, point));
            if (manifold._contactPointsCount == 1) {
                firstCur = cur;
                firstPrev = prev;
                contact1 = point;
                manifold.Normal = polygon.Normals[prev];
            } else {
                contact2 = point;
                stopped = true;
            }
        }
        if (!stopped) {
            cur = 0;
            prev = 0;
        }
    }

    const std::int32_t contacts = manifold._contactPointsCount;
    if (contacts == 0) {
        return false;
    }

    if (contacts == 1) {
        // One side crossed: the edge endpoint inside the polygon is the second contact.
        const bool towardsP1 = FPVector2::Dot(direction, manifold.Normal) < 0;
        const FPVector2 endpoint = towardsP1 ? p1 : p2;
        contact2 = endpoint;
        const FPVector2 toEnd = endpoint - contact1;
        manifold.Penetration = FPVector2::Dot(-toEnd, manifold.Normal);
        const FP tangent = FPVector2::Dot(toEnd, {-manifold.Normal.Y, manifold.Normal.X});

        std::int32_t vertexIndex;
        std::int32_t halfIndex;
        FPVector2 halfStart;
        FPVector2 halfEnd;
        if (tangent > 0) {
            vertexIndex = (firstPrev > 0 ? firstPrev : count) - 1;
            halfIndex = firstCur;
            halfStart = contact1;
            halfEnd = contact2;
        } else {
            vertexIndex = firstCur;
            halfIndex = firstPrev;
            halfStart = contact2;
            halfEnd = contact1;
        }
        const FPVector2 sideNormal = polygon.Normals[vertexIndex];
        const FP vertexDepth = FPVector2::Dot(polygon.Vertices[vertexIndex] - endpoint, sideNormal);
        if (vertexDepth > 0 && vertexDepth < manifold.Penetration) {
            manifold.Penetration = vertexDepth;
            manifold.Normal = sideNormal;
        }
        FPVector2 averageNormal;
        const FP halfDepth = CalculatePenetrationOnPolygonHalf(polygon, halfIndex, halfIndex, halfStart,
                                                               halfEnd, edgeNormal, averageNormal);
        if (halfDepth > 0 && halfDepth < manifold.Penetration) {
            manifold.Penetration = halfDepth;
            manifold.Normal = averageNormal;
        } else if (manifold.Penetration < 0) {
            return false;
        }
        CollisionManifold2D::AddContactPoint(&manifold, ToWorld(polygon, endpoint));
    } else if (cur == firstPrev || prev == firstCur) {
        // Two contacts on neighbouring sides: one vertex of the polygon lies behind the edge.
        const bool sameVertex = cur == firstPrev;
        const FPVector2 start = sameVertex ? contact2 : contact1;
        const FPVector2 end = sameVertex ? contact1 : contact2;
        const std::int32_t vertex = sameVertex ? cur : firstCur;
        manifold.Penetration = CalculatePenetrationOnPolygonHalf(polygon, vertex, vertex, start, end,
                                                                 edgeNormal, manifold.Normal);
    } else {
        // Two contacts further apart: measure the polygon on both sides of the edge and
        // keep the shallower half.
        const FPVector2 meanNormal = FPVector2::Normalize(polygon.Normals[prev] + manifold.Normal);
        const FP spread = FPVector2::Dot(polygon.Vertices[cur] - polygon.Vertices[prev], meanNormal);
        manifold.Penetration = FP::MaxValue();
        bool done = false;
        if (spread >= -SideTolerance) {
            const std::int32_t end = firstPrev + (firstPrev < cur ? count : 0);
            FPVector2 averageNormal;
            const FP depth = CalculatePenetrationOnPolygonHalf(polygon, cur, end, contact2, contact1,
                                                               edgeNormal, averageNormal);
            if (depth < manifold.Penetration) {
                manifold.Penetration = depth;
                manifold.Normal = averageNormal;
            }
            done = spread > SideTolerance;
        }
        if (!done) {
            FPVector2 averageNormal;
            const FP depth = CalculatePenetrationOnPolygonHalf(polygon, (firstPrev + 1) % count, prev,
                                                               contact1, contact2, edgeNormal,
                                                               averageNormal);
            if (depth < manifold.Penetration) {
                manifold.Penetration = depth;
                manifold.Normal = averageNormal;
            }
        }
    }
    manifold.Normal = FPVector2::Rotate(manifold.Normal, sinP, cosP);
    return true;
}

// 0x7d15bf0
bool LinecastBody::Check(NarrowTls2D& tls, AssetGuid mapGuid, const NarrowEntry2D* b,
                         const FPVector2& start, const FPVector2& end, bool computeDetailedInfo,
                         Hit& hit) {
    hit = Hit{};
    auto identify = [&]() {
        hit._staticColliderIndex = b->StaticColliderIndex;
        if (b->StaticColliderIndex < 0) {
            hit._entity = b->Ref;
        } else {
            hit._mapGuid = mapGuid;
        }
        hit.ShapeUserTag = b->Shape->UserTag;
        hit._isTrigger = b->IsTrigger ? 1 : 0;
    };
    const Shape2D& shape = *b->Shape;
    const FPVector2 position = b->Narrow.Position;
    const FP sin = b->Narrow.RotSin;
    const FP cos = b->Narrow.RotCos;
    switch (shape.Type()) {
    case Shape2DType::Circle:
        if (!FPCollision::LineIntersectsCircle(start, end, position, shape.Circle.Radius, hit.Point,
                                               hit.CastDistanceNormalized, true)) {
            return false;
        }
        identify();
        if (computeDetailedInfo) {
            hit.Normal = FPVector2::Normalize(hit.Point - position);
        }
        return true;
    case Shape2DType::Edge: {
        const FPVector2 half = {cos * shape.Edge.Extent, shape.Edge.Extent * sin};
        FP distance;
        if (!FPCollision::LineIntersectsLine(start, end, position - half, position + half, hit.Point,
                                             distance, hit.CastDistanceNormalized)) {
            return false;
        }
        identify();
        if (computeDetailedInfo) {
            hit.Normal = {-sin, cos};
            if ((start.X - end.X) * sin + (end.Y - start.Y) * cos > 0) {
                hit.Normal = {sin, -cos}; // face the segment
            }
        }
        return true;
    }
    case Shape2DType::Box: {
        const FPVector2 extents = shape.Box._extents;
        FPVector2 contact;
        FPVector2 normal;
        if (!LineBoxLocalSpace(ToLocal(start - position, sin, cos), ToLocal(end - position, sin, cos),
                               -extents, extents, contact, normal, hit.CastDistanceNormalized)) {
            return false;
        }
        identify();
        hit.Point = FPVector2::Rotate(contact, sin, cos) + position;
        if (computeDetailedInfo) {
            hit.Normal = FPVector2::Rotate(normal, sin, cos);
        }
        return true;
    }
    case Shape2DType::Polygon: {
        Polygon polygon;
        polygon.Set(tls, *b);
        // Cyrus-Beck clipping of the segment (in the polygon's frame) against the sides.
        const FPVector2 localStart = ToLocal(start - position, sin, cos);
        const FPVector2 direction = ToLocal(end - position, sin, cos) - localStart;
        FP enter = FP::Zero();
        FP exit = FP::One();
        std::int32_t enterSide = -1;
        if (polygon.VerticesCount < 1) {
            return false;
        }
        for (std::int32_t i = 0; i < polygon.VerticesCount; ++i) {
            const FP numerator = FPVector2::Dot(polygon.Normals[i], polygon.Vertices[i] - localStart);
            const FP denominator = FPVector2::Dot(polygon.Normals[i], direction);
            if (denominator == 0) {
                if (numerator < 0) {
                    return false; // parallel to this side and outside it
                }
            } else if (denominator < 0) {
                if (numerator < enter * denominator) {
                    enter = numerator / denominator;
                    enterSide = i;
                }
            } else if (numerator < exit * denominator) {
                exit = numerator / denominator;
            }
            if (exit < enter) {
                return false;
            }
        }
        if (enterSide < 0 || enter > FP::One()) {
            return false;
        }
        identify();
        hit.Point = start + (end - start) * enter;
        hit.CastDistanceNormalized = enter;
        if (computeDetailedInfo) {
            hit.Normal = FPVector2::Rotate(polygon.Normals[enterSide], sin, cos);
        }
        return true;
    }
    default:
        return false;
    }
}

} // namespace Quantum::Physics2D::NarrowChecks
