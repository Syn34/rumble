// gjk3d.cpp - Quantum.Physics3D.Gjk3D, its simplex and separation function, and the support
// mapping of 3D entries (Entry3D.GetSupportOffset).
#include "gjk3d.h"

#include "quantum_core/physics_components.h"

#include "quantum_core/gjk_settings.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/fp_quaternion.h"

#include <algorithm>
#include <cassert>
#include <utility>

namespace Quantum::Physics3D {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {

constexpr FP::Raw MinDistanceToleranceRaw = 6;
constexpr FP ProgressThreshold = FP::FromRaw(65470); // about 0.999
constexpr std::int32_t TimeShift = 14;
constexpr std::int32_t PushBackIterations = 16;
constexpr FP::Raw CoincidentRaw = 9;
constexpr FP::Raw CoplanarRaw = 8;

FP ScaleByTime(FP value, FP time, std::int32_t dtShift) {
    return FP::FromRaw(fpdetail::WrapMul(value.RawValue(), time.RawValue()) >> ((dtShift + 16) & 63));
}

FPVector3 ScaleByTime(const FPVector3& value, FP time, std::int32_t dtShift) {
    return {ScaleByTime(value.X, time, dtShift), ScaleByTime(value.Y, time, dtShift),
            ScaleByTime(value.Z, time, dtShift)};
}

// Rotation by the angular velocity over `time`, as yaw (Y), pitch (X) and roll (Z).
FPQuaternion AngularStep(const FPVector3& angularVelocity, FP time, std::int32_t dtShift) {
    return FPQuaternion::CreateFromYawPitchRoll(ScaleByTime(angularVelocity.Y, time, dtShift),
                                                ScaleByTime(angularVelocity.X, time, dtShift),
                                                ScaleByTime(angularVelocity.Z, time, dtShift));
}

FP ShiftLeft(FP value, std::int32_t bits) {
    return FP::FromRaw(static_cast<FP::Raw>(static_cast<std::uint64_t>(value.RawValue()) << (bits & 63)));
}

FPVector3 ShiftLeft(const FPVector3& value, std::int32_t bits) {
    return {ShiftLeft(value.X, bits), ShiftLeft(value.Y, bits), ShiftLeft(value.Z, bits)};
}

FP ShiftedSqrMagnitude(const FPVector3& point, std::int32_t shift) {
    const auto square = [&](FP v) {
        return fpdetail::WrapMul(v.RawValue(), v.RawValue()) >> (shift & 63);
    };
    return FP::FromRaw(square(point.X) + square(point.Y) + square(point.Z));
}

FP SphereRadius(const Entry3D& entry) {
    return entry.Shape->Type() == Shape3DType::Sphere ? entry.Shape->Sphere.Radius : FP::Zero();
}

FPVector3 CentreOfMass(const Entry3D& entry) {
    assert(entry.Transform3D != nullptr);
    return entry.Transform3D->Position + entry.RotatedCenterOfMass;
}

const PhysicsBody3D& BodyOf(const Entry3D& entry) {
    assert(entry.Body3D != nullptr);
    return *entry.Body3D;
}

// Angular motion that turns the shape's support points: none for a sphere spinning about
// its own centre.
bool TurnsSupport(const Entry3D& entry) {
    if (entry.Shape->Type() == Shape3DType::Sphere && entry.RotatedCenterOfMass == FPVector3::Zero()) {
        return false;
    }
    return BodyOf(entry).AngularVelocity != FPVector3::Zero();
}

} // namespace

// 0x7d79e54
FPVector3 GetSupportOffset(const Entry3D& entry, FPVector3 worldDir, std::optional<FP> deltaTime,
                           std::int32_t dtShift) {
    const Shape3D& shape = *entry.Shape;
    FPVector3 offset;
    switch (shape.Type()) {
    case Shape3DType::Sphere:
        if (deltaTime.has_value() && entry.HasDynamicBody &&
            entry.RotatedCenterOfMass != FPVector3::Zero()) {
            const FPQuaternion step = AngularStep(BodyOf(entry).AngularVelocity, *deltaTime, dtShift);
            offset = entry.RotatedCenterOfMass - step * entry.RotatedCenterOfMass;
        }
        break;
    case Shape3DType::Box: {
        bool rotates = false;
        FPQuaternion step;
        FPVector3 centroidOffset;
        if (deltaTime.has_value() && *deltaTime != 0 && entry.HasDynamicBody &&
            BodyOf(entry).AngularVelocity != FPVector3::Zero()) {
            step = AngularStep(BodyOf(entry).AngularVelocity, *deltaTime, dtShift);
            worldDir = FPQuaternion::Inverse(step) * worldDir;
            const FPVector3 pivot = (entry.Transform3D->Position - entry.Narrow.Position) + entry.RotatedCenterOfMass;
            if (pivot != FPVector3::Zero()) {
                centroidOffset = pivot - step * pivot;
            }
            rotates = true;
        }
        const FPVector3 extents = shape.Box._extents;
        const NarrowData3D& narrow = entry.Narrow;
        const FP x = FPVector3::Dot(worldDir, narrow.RotatedRight) < 0 ? -extents.X : extents.X;
        const FP y = FPVector3::Dot(worldDir, narrow.RotatedUp) < 0 ? -extents.Y : extents.Y;
        const FP z = FPVector3::Dot(worldDir, narrow.RotatedForward) < 0 ? -extents.Z : extents.Z;
        offset = narrow.RotatedRight * x + narrow.RotatedUp * y + narrow.RotatedForward * z;
        if (rotates) {
            offset = step * offset + centroidOffset;
        }
        break;
    }
    default:
        assert(false && "Entry3D.GetSupportOffset: unsupported shape type");
        return {};
    }
    if (deltaTime.has_value() && entry.HasDynamicBody) {
        offset = offset + ScaleByTime(BodyOf(entry).Velocity, *deltaTime, dtShift);
    }
    return offset;
}

// 0x7d50fe4
void SimplexDataPointers3D::ResetPointers(SimplexDataPointers3D* ptrs, SimplexData3D* simplex) {
    ptrs->Data = simplex;
    ptrs->UsedCount = 0;
    ptrs->VertexPointers[0] = &simplex->Vertex0;
    ptrs->VertexPointers[1] = &simplex->Vertex1;
    ptrs->VertexPointers[2] = &simplex->Vertex2;
    ptrs->VertexPointers[3] = &simplex->Vertex3;
    ptrs->Lambdas = &simplex->Lambda0;
}

// 0x7d4f360
void Gjk3D::GetSimplexVertex(const Entry3D* entryA, const Entry3D* entryB,
                             const TriangleCCW* tri, FPVector3& worldDir, SimplexVertex3D* vertex,
                             std::int32_t bitShift, std::optional<FP> deltaTime, std::int32_t dtShift) {
    auto support = [&](const Entry3D& entry, FPVector3& base, FPVector3& offset) {
        if (entry.Shape->Type() == Shape3DType::Mesh) {
            assert(tri != nullptr);
            base = tri->Center;
            tri->GetSupportOffset(worldDir, offset);
        } else {
            base = entry.Narrow.Position;
            offset = GetSupportOffset(entry, worldDir, deltaTime, dtShift);
        }
    };
    FPVector3 offsetA;
    support(*entryA, vertex->SupportA, offsetA);
    worldDir = -worldDir;
    FPVector3 offsetB;
    support(*entryB, vertex->SupportB, offsetB);
    vertex->SupportA = offsetA + vertex->SupportA;
    vertex->SupportB = offsetB + vertex->SupportB;
    vertex->Point = ShiftLeft(vertex->SupportA - vertex->SupportB, bitShift);
}

// 0x7d4eff8
void Gjk3D::ComputeDistance(SimplexDataPointers3D* ptrs, const Entry3D* entryA,
                            const Entry3D* entryB, const TriangleCCW* tri, FP& distance,
                            FPVector3& separationAxis, std::int32_t iterations,
                            std::optional<FP> deltaTime, std::int32_t dtShift) {
    ptrs->UsedCount = 0;
    const FP radii = SphereRadius(*entryA) + SphereRadius(*entryB);
    ptrs->UsedCount = 1;
    FPVector3 direction = entryB->Narrow.Position - entryA->Narrow.Position;
    GetSimplexVertex(entryA, entryB, tri, direction, ptrs->VertexPointers[0], 0, deltaTime, dtShift);
    FPVector3 closest = ptrs->VertexPointers[0]->Point;
    FP distanceSquared = closest.SqrMagnitude();
    const FP grown = radii + fpdetail::Epsilon;
    const FP tolerance = FP::FromRaw(std::max(MinDistanceToleranceRaw, (grown * grown).RawValue()));
    bool degenerate = false;

    while (iterations-- > 0 && distanceSquared > tolerance) {
        const std::int32_t index = ptrs->UsedCount;
        ptrs->UsedCount = index + 1;
        FPVector3 search = -closest;
        GetSimplexVertex(entryA, entryB, tri, search, ptrs->VertexPointers[index], 0, deltaTime, dtShift);
        if (IsAffinelyDependent(ptrs)) {
            --ptrs->UsedCount;
            break;
        }
        const FPVector3 newest = ptrs->VertexPointers[ptrs->UsedCount - 1]->Point;
        const FP progress = FPVector3::Dot(closest, newest) / distanceSquared;
        ComputeClosestPoint(ptrs, 0, closest, degenerate);
        distanceSquared = closest.SqrMagnitude();
        if (progress > ProgressThreshold || degenerate || ptrs->UsedCount == 4) {
            break;
        }
    }

    const FP gap = FPMath::Sqrt(distanceSquared) - radii;
    distance = gap;
    if (gap > 0) {
        separationAxis = FPVector3::Normalize(closest);
    } else {
        distance = FP::Zero();
        separationAxis = FPVector3::Zero();
    }
}

namespace {

template <FPVector3 SimplexVertex3D::*Support>
FPVector3 WitnessPoint(const SimplexDataPointers3D* ptrs) {
    const FP* lambdas = ptrs->Lambdas;
    const std::int32_t count = ptrs->UsedCount;
    if (count == 1) {
        return ptrs->VertexPointers[0]->*Support;
    }
    if (count < 1 || count > 4) {
        return FPVector3::Zero();
    }
    FPVector3 point = ptrs->VertexPointers[0]->*Support * lambdas[0];
    for (std::int32_t i = 1; i < count; ++i) {
        point = point + ptrs->VertexPointers[i]->*Support * lambdas[i];
    }
    return point;
}

} // namespace

// 0x7d4f7e0
void Gjk3D::GetWitnessPoints(const SimplexDataPointers3D* ptrs, FPVector3& pointA, FPVector3& pointB) {
    pointA = WitnessPoint<&SimplexVertex3D::SupportA>(ptrs);
    pointB = WitnessPoint<&SimplexVertex3D::SupportB>(ptrs);
}

// 0x7d4fb50
void Gjk3D::GetWitnessPointA(const SimplexDataPointers3D* ptrs, FPVector3& pointA) {
    pointA = WitnessPoint<&SimplexVertex3D::SupportA>(ptrs);
}

// 0x7d4fd68
void Gjk3D::GetWitnessPointB(const SimplexDataPointers3D* ptrs, FPVector3& pointB) {
    pointB = WitnessPoint<&SimplexVertex3D::SupportB>(ptrs);
}

// 0x7d4f5ac
bool Gjk3D::IsAffinelyDependent(const SimplexDataPointers3D* ptrs) {
    const auto point = [&](int i) { return ptrs->VertexPointers[i]->Point; };
    switch (ptrs->UsedCount) {
    case 2:
        return (point(1) - point(0)).SqrMagnitude().RawValue() < CoincidentRaw;
    case 3:
        return FPVector3::Cross(point(1) - point(0), point(2) - point(0)).SqrMagnitude().RawValue() <
               CoincidentRaw;
    case 4: {
        const FPVector3 a = point(0);
        const FP::Raw volume =
            FPVector3::Dot(FPVector3::Cross(point(2) - a, point(3) - a), point(1) - a).RawValue();
        return volume >= -CoplanarRaw && volume <= CoplanarRaw;
    }
    default:
        return false;
    }
}

// 0x7d4f744
void Gjk3D::ComputeClosestPoint(SimplexDataPointers3D* ptrs, std::int32_t curBitShift,
                                FPVector3& closestPoint, bool& isDegenerate) {
    switch (ptrs->UsedCount) {
    case 1:
        isDegenerate = false;
        closestPoint = ptrs->VertexPointers[0]->Point;
        break;
    case 2:
        isDegenerate = false;
        SolveLine(ptrs, ptrs->Lambdas, closestPoint);
        break;
    case 3:
        isDegenerate = false;
        SolveTriangle(ptrs, ptrs->Lambdas, curBitShift, closestPoint);
        break;
    case 4:
        SolveTetrahedron(ptrs, ptrs->Lambdas, curBitShift, closestPoint, isDegenerate);
        break;
    default:
        isDegenerate = false;
        closestPoint = FPVector3::Zero();
        break;
    }
}

// 0x7d503dc
void Gjk3D::SolveLine(SimplexDataPointers3D* ptrs, FP* lambdas, FPVector3& closestPoint) {
    SimplexVertex3D* const vertexA = ptrs->VertexPointers[0];
    SimplexVertex3D* const vertexB = ptrs->VertexPointers[1];
    const FPVector3 a = vertexA->Point;
    const FPVector3 b = vertexB->Point;
    const FPVector3 ba = a - b;
    const FP t = FPVector3::Dot(ba, a);
    if (t <= 0) {
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = a;
        return;
    }
    const FP lengthSquared = ba.SqrMagnitude();
    if (t >= lengthSquared) {
        ptrs->VertexPointers[0] = vertexB;
        ptrs->VertexPointers[1] = vertexA;
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = b;
        return;
    }
    ptrs->UsedCount = 2;
    const FP u = t / lengthSquared;
    const FP w = FP::One() - u;
    lambdas[0] = w;
    lambdas[1] = u;
    closestPoint = a * w + b * u;
}

// 0x7d4ff80 - as the 2D solver (Voronoi regions, newest vertex C first); the products of
// the region determinants drop 2 * curBitShift bits so shifted points don't overflow.
void Gjk3D::SolveTriangle(SimplexDataPointers3D* ptrs, FP* lambdas, std::int32_t curBitShift,
                          FPVector3& closestPoint) {
    SimplexVertex3D* const pa = ptrs->VertexPointers[0];
    SimplexVertex3D* const pb = ptrs->VertexPointers[1];
    SimplexVertex3D* const pc = ptrs->VertexPointers[2];
    const FPVector3 a = pa->Point;
    const FPVector3 b = pb->Point;
    const FPVector3 c = pc->Point;
    const FPVector3 ab = b - a;
    const FPVector3 ac = c - a;
    const FP half = FP::Half();
    const FP third = FP::FromRaw(0x5555);
    const std::int32_t drop = (2 * curBitShift) & 0x3e;
    const auto reduced = [&](FP v) { return FP::FromRaw(v.RawValue() >> drop); };

    const FP acDotC = FPVector3::Dot(ac, c);
    const FP abDotC = FPVector3::Dot(ab, c);
    if (acDotC <= 0 && acDotC <= abDotC) {
        ptrs->VertexPointers[2] = pa;
        ptrs->VertexPointers[0] = pc;
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = c;
        return;
    }
    const FP abDotA = FPVector3::Dot(ab, a);
    const FP acDotA = FPVector3::Dot(ac, a);
    if (acDotA >= 0 && abDotA >= 0) {
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = a;
        return;
    }
    const FP abDotCr = reduced(abDotC);
    const FP acDotAr = reduced(acDotA);
    const FP abDotAr = reduced(abDotA);
    const FP acDotCr = reduced(acDotC);
    const FP vb = acDotAr * abDotCr - acDotCr * abDotAr;
    if (acDotC >= 0 && acDotA <= 0 && vb <= 0) {
        ptrs->VertexPointers[1] = pc;
        ptrs->VertexPointers[2] = pb;
        ptrs->UsedCount = 2;
        const FP denominator = acDotA - acDotC;
        const FP w = denominator == 0 ? half : acDotA / denominator;
        const FP u = denominator == 0 ? half : FP::One() - w;
        closestPoint = a * u + c * w;
        lambdas[0] = u;
        lambdas[1] = w;
        return;
    }
    const FP abDotB = FPVector3::Dot(ab, b);
    const FP acDotB = FPVector3::Dot(ac, b);
    if (abDotB <= 0 && abDotB <= acDotB) {
        ptrs->VertexPointers[0] = pb;
        ptrs->VertexPointers[1] = pa;
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = b;
        return;
    }
    const FP abDotBr = reduced(abDotB);
    const FP acDotBr = reduced(acDotB);
    const FP va = acDotCr * abDotBr - acDotBr * abDotCr;
    const FP d1 = acDotC - abDotC;
    const FP d2 = abDotB - acDotB;
    if (d1 >= 0 && d2 >= 0 && va <= 0) {
        ptrs->VertexPointers[2] = pa;
        ptrs->VertexPointers[0] = pc;
        ptrs->UsedCount = 2;
        const FP denominator = d1 + d2;
        const FP w = denominator == 0 ? half : d2 / denominator;
        const FP u = denominator == 0 ? half : FP::One() - w;
        closestPoint = b * u + c * w;
        lambdas[0] = w;
        lambdas[1] = u;
        return;
    }
    const FP vc = acDotBr * abDotAr - acDotAr * abDotBr;
    if (abDotB >= 0 && abDotA <= 0 && vc <= 0) {
        ptrs->UsedCount = 2;
        const FP denominator = abDotA - abDotB;
        const FP v = denominator == 0 ? half : abDotA / denominator;
        const FP u = denominator == 0 ? half : FP::One() - v;
        closestPoint = a * u + b * v;
        lambdas[0] = u;
        lambdas[1] = v;
        return;
    }
    ptrs->UsedCount = 3;
    const FP sum = vb + va + vc;
    FP v = third;
    FP w = third;
    FP u = third;
    if (sum != 0) {
        v = vb / sum;
        w = vc / sum;
        u = FP::One() - (v + w);
    }
    closestPoint = b * v + c * w + a * u;
    lambdas[0] = u;
    lambdas[1] = v;
    lambdas[2] = w;
}

// 0x7d50a88
bool Gjk3D::IsOriginOutsidePlane(const FPVector3& vA, const FPVector3& vB, const FPVector3& vC,
                                 const FPVector3& oppositeV, bool& isDegenerate,
                                 FP& oppositeBarycentricCoord) {
    const FPVector3 normal = FPVector3::Cross(vB - vA, vC - vA);
    const FP oppositeSide = FPVector3::Dot(normal, oppositeV - vA);
    if (oppositeSide == 0) {
        isDegenerate = true;
        oppositeBarycentricCoord = FP::Zero();
        return false;
    }
    const FP planeOffset = FPVector3::Dot(normal, vA);
    if ((planeOffset * oppositeSide).RawValue() >= 1) {
        oppositeBarycentricCoord = FP::Zero();
        return true;
    }
    oppositeBarycentricCoord = planeOffset / -oppositeSide;
    return false;
}

// 0x7d504f4 - tests the four faces; the closest point comes from the nearest face the
// origin lies outside of, or the tetrahedron contains the origin.
void Gjk3D::SolveTetrahedron(SimplexDataPointers3D* ptrs, FP* lambdas, std::int32_t curBitShift,
                             FPVector3& closestPoint, bool& isDegenerate) {
    SimplexVertex3D* const a = ptrs->VertexPointers[0];
    SimplexVertex3D* const b = ptrs->VertexPointers[1];
    SimplexVertex3D* const c = ptrs->VertexPointers[2];
    SimplexVertex3D* const d = ptrs->VertexPointers[3];
    closestPoint = FPVector3::Zero();
    isDegenerate = false;
    FP coords[4] = {}; // barycentric weights of a, b, c, d
    SimplexDataPointers3D best = *ptrs;
    FP bestDistanceSquared = FP::MaxValue();
    bool inside = true;

    struct Face {
        SimplexVertex3D* p;
        SimplexVertex3D* q;
        SimplexVertex3D* r;
        SimplexVertex3D* opposite;
        int coord;
    };
    const Face faces[4] = {{a, b, c, d, 3}, {a, c, d, b, 1}, {a, d, b, c, 2}, {b, d, c, a, 0}};
    // The face triangles are solved with these vertex orders.
    SimplexVertex3D* const solveOrder[4][3] = {{a, b, c}, {a, c, d}, {a, b, d}, {c, b, d}};
    for (int f = 0; f < 4; ++f) {
        const Face& face = faces[f];
        const bool outside = IsOriginOutsidePlane(face.p->Point, face.q->Point, face.r->Point,
                                                  face.opposite->Point, isDegenerate,
                                                  coords[face.coord]);
        if (outside) {
            inside = false;
            SimplexDataPointers3D candidate = *ptrs;
            candidate.UsedCount = 3;
            candidate.VertexPointers[0] = solveOrder[f][0];
            candidate.VertexPointers[1] = solveOrder[f][1];
            candidate.VertexPointers[2] = solveOrder[f][2];
            FP faceLambdas[3] = {};
            FPVector3 point;
            SolveTriangle(&candidate, faceLambdas, curBitShift, point);
            const FP distanceSquared = point.SqrMagnitude();
            if (f == 0 || distanceSquared < bestDistanceSquared) {
                closestPoint = point;
                best = candidate;
                best.VertexPointers[3] = face.opposite;
                std::copy(faceLambdas, faceLambdas + 3, lambdas);
                bestDistanceSquared = distanceSquared;
            }
        }
        if (isDegenerate) {
            ptrs->UsedCount = 3;
            return;
        }
    }
    if (inside) {
        ptrs->UsedCount = 4;
        std::copy(coords, coords + 4, lambdas);
        return;
    }
    *ptrs = best;
}

// 0x7d51968
void SeparationFunction3D::Initialize(const SimplexDataPointers3D* ptrs,
                                      const CollisionManifoldInfo3D* mInfo, FP deltaTime,
                                      std::int32_t dtBitshift) {
    FPVector3 pointA;
    FPVector3 pointB;
    Gjk3D::GetWitnessPoints(ptrs, pointA, pointB);
    _dynamicState = 0;
    _mInfo = *mInfo;

    if (ptrs->UsedCount == 1) {
        _closestFeatures = PointPoint;
        _entry = _mInfo.EntryA;
        _faceEntry = _mInfo.EntryB;
        _comPoint = CentreOfMass(*_entry);
        _faceComPoint = CentreOfMass(*_faceEntry);
        _separationAxis = pointA - pointB;
        if (_entry->HasDynamicBody) {
            const PhysicsBody3D& body = BodyOf(*_entry);
            _dynamicState = static_cast<std::uint8_t>(
                _dynamicState | (body.Velocity != FPVector3::Zero() ? PointEntryHasLinearMotion : 0) |
                (body.AngularVelocity != FPVector3::Zero() ? PointEntryHasAngularMotion : 0));
        }
        if (_faceEntry->HasDynamicBody) {
            const PhysicsBody3D& body = BodyOf(*_faceEntry);
            _dynamicState = static_cast<std::uint8_t>(
                _dynamicState | (body.Velocity != FPVector3::Zero() ? FaceEntryHasLinearMotion : 0) |
                (body.AngularVelocity != FPVector3::Zero() ? FaceEntryHasAngularMotion : 0));
        }
        _separationAxis = FPVector3::Normalize(_separationAxis);
        return;
    }

    // Edge (two vertices) or face (three): if B's support points coincide, B provides the
    // point and A the edge or face; otherwise the other way round.
    _closestFeatures = ptrs->UsedCount == 2 ? PointEdge : PointFace;
    const auto supportB = [&](int i) { return ptrs->VertexPointers[i]->SupportB; };
    FP::Raw spread;
    if (ptrs->UsedCount == 2) {
        spread = (supportB(1) - supportB(0)).SqrMagnitude().RawValue();
    } else {
        spread = FPVector3::Cross(supportB(1) - supportB(0), supportB(2) - supportB(0)).SqrMagnitude().RawValue();
    }
    const bool pointOnB = spread >= -CoplanarRaw && spread <= CoplanarRaw;
    _entry = pointOnB ? _mInfo.EntryB : _mInfo.EntryA;
    _faceEntry = pointOnB ? _mInfo.EntryA : _mInfo.EntryB;
    const FPVector3 point = pointOnB ? pointB : pointA;
    const FPVector3 facePoint = pointOnB ? pointA : pointB;
    _separationAxis = point - facePoint;

    _comPoint = CentreOfMass(*_entry);
    if (_entry->HasDynamicBody) {
        // The original keeps the "has a body" flag in the same register, so angular motion
        // marks the linear flag too.
        const PhysicsBody3D& body = BodyOf(*_entry);
        if (body.Velocity != FPVector3::Zero()) {
            _dynamicState = PointEntryHasLinearMotion;
        }
        if (TurnsSupport(*_entry)) {
            _dynamicState = PointEntryHasLinearMotion | PointEntryHasAngularMotion;
        }
    }
    const std::uint8_t pointFlags = _entry->HasDynamicBody ? std::uint8_t{PointEntryHasLinearMotion} : 0;

    _faceComPoint = CentreOfMass(*_faceEntry);
    _faceComOffsetPoint = facePoint - _faceComPoint;
    if (_faceEntry->HasDynamicBody) {
        const PhysicsBody3D& body = BodyOf(*_faceEntry);
        // Same register reuse: the face flags are or'ed onto the "has a body" value.
        std::uint8_t flags = _dynamicState != 0 ? _dynamicState : pointFlags;
        if (body.Velocity != FPVector3::Zero()) {
            flags |= FaceEntryHasLinearMotion;
            _dynamicState = flags;
            _faceComOffsetPoint = _faceComOffsetPoint - ScaleByTime(body.Velocity, deltaTime, dtBitshift);
        }
        if (TurnsSupport(*_faceEntry)) {
            flags |= FaceEntryHasAngularMotion;
            _dynamicState = flags;
            if (deltaTime != 0) {
                // Back to the face entry's starting pose.
                const FPQuaternion undo = AngularStep(body.AngularVelocity, deltaTime, dtBitshift).Inverted();
                _faceComOffsetPoint = undo * _faceComOffsetPoint;
                _separationAxis = undo * _separationAxis;
            }
        }
    }
    _separationAxis = FPVector3::Normalize(_separationAxis);
}

// 0x7d51f70
FP SeparationFunction3D::GetMinimumSeparation(FP deltaTime, std::int32_t dtBitshift) {
    // Offset of a point of `entry` from its centre of mass in the entry's starting pose.
    auto startPoseOffset = [&](const Entry3D& entry, FPVector3 offset, std::uint8_t linear,
                               std::uint8_t angular) {
        if ((_dynamicState & linear) != 0) {
            offset = offset - ScaleByTime(BodyOf(entry).Velocity, deltaTime, dtBitshift);
        }
        if ((_dynamicState & angular) != 0) {
            offset = AngularStep(BodyOf(entry).AngularVelocity, deltaTime, dtBitshift).Inverted() * offset;
        }
        return offset;
    };

    if (_closestFeatures == PointPoint) {
        FPVector3 direction = -_separationAxis;
        SimplexVertex3D vertex;
        Gjk3D::GetSimplexVertex(_entry, _faceEntry, _mInfo.Tri, direction, &vertex, 0, deltaTime,
                                dtBitshift);
        _comOffsetPoint = vertex.SupportA - _comPoint;
        _faceComOffsetPoint = vertex.SupportB - _faceComPoint;
        _comOffsetPoint = startPoseOffset(*_entry, _comOffsetPoint, PointEntryHasLinearMotion,
                                          PointEntryHasAngularMotion);
        _faceComOffsetPoint = startPoseOffset(*_faceEntry, _faceComOffsetPoint, FaceEntryHasLinearMotion,
                                              FaceEntryHasAngularMotion);
        return FPVector3::Dot(_separationAxis, vertex.Point);
    }
    assert(_closestFeatures == PointEdge || _closestFeatures == PointFace);

    // Move the edge/face (and the axis) to `deltaTime`.
    FPVector3 axis = _separationAxis;
    FPVector3 faceOffset = _faceComOffsetPoint;
    if ((_dynamicState & FaceEntryHasAngularMotion) != 0) {
        const FPQuaternion step = AngularStep(BodyOf(*_faceEntry).AngularVelocity, deltaTime, dtBitshift);
        axis = step * axis;
        faceOffset = step * faceOffset;
    }
    if ((_dynamicState & FaceEntryHasLinearMotion) != 0) {
        faceOffset = faceOffset + ScaleByTime(BodyOf(*_faceEntry).Velocity, deltaTime, dtBitshift);
    }
    const FPVector3 direction = -axis;
    FPVector3 support;
    if (_entry->Shape->Type() == Shape3DType::Mesh) {
        FPVector3 offset;
        _mInfo.Tri->GetSupportOffset(direction, offset);
        support = offset + _mInfo.Tri->Center;
    } else {
        support = GetSupportOffset(*_entry, direction, deltaTime, dtBitshift) + _entry->Narrow.Position;
    }
    _comOffsetPoint = startPoseOffset(*_entry, support - _comPoint, PointEntryHasLinearMotion,
                                      PointEntryHasAngularMotion);
    return FPVector3::Dot(support - _faceComPoint - faceOffset, axis);
}

// 0x7d526c4
FP SeparationFunction3D::GetSeparation(FP deltaTime, std::int32_t dtBitshift) const {
    FPVector3 axis = _separationAxis;
    FPVector3 comPoint = _comPoint;
    FPVector3 comOffset = _comOffsetPoint;
    if ((_dynamicState & PointEntryHasLinearMotion) != 0) {
        comPoint = comPoint + ScaleByTime(BodyOf(*_entry).Velocity, deltaTime, dtBitshift);
    }
    if ((_dynamicState & PointEntryHasAngularMotion) != 0) {
        comOffset = AngularStep(BodyOf(*_entry).AngularVelocity, deltaTime, dtBitshift) * comOffset;
    }
    FPVector3 faceComPoint = _faceComPoint;
    FPVector3 faceOffset = _faceComOffsetPoint;
    if ((_dynamicState & FaceEntryHasLinearMotion) != 0) {
        faceComPoint = faceComPoint + ScaleByTime(BodyOf(*_faceEntry).Velocity, deltaTime, dtBitshift);
    }
    if ((_dynamicState & FaceEntryHasAngularMotion) != 0) {
        const FPQuaternion step = AngularStep(BodyOf(*_faceEntry).AngularVelocity, deltaTime, dtBitshift);
        faceOffset = step * faceOffset;
        if (_closestFeatures != PointPoint) {
            axis = step * axis;
        }
    }
    return FPVector3::Dot(axis, (comOffset + comPoint) - (faceOffset + faceComPoint));
}

// 0x7d51020
TimeOfImpactResult Gjk3D::ComputeTimeOfImpactDetailedInfo(SimplexDataPointers3D* ptrs,
                                                          CollisionManifold3D* m,
                                                          const Entry3D* entryA,
                                                          const Entry3D* entryB,
                                                          const TriangleCCW* tri, FP distance) {
    FPVector3 contact;
    GetWitnessPointB(ptrs, contact);
    if (m->Normal == FPVector3::Zero()) {
        FPVector3 motion = m->Normal;
        if (entryB->HasDynamicBody) {
            motion = BodyOf(*entryB).Velocity;
        }
        if (entryA->HasDynamicBody) {
            motion = motion - BodyOf(*entryA).Velocity;
        }
        m->Normal = FPVector3::Normalize(motion);
    }
    if (entryB->Shape->Type() == Shape3DType::Sphere) {
        contact = contact + m->Normal * entryB->Shape->Sphere.Radius;
    }
    CollisionManifold3D::AddContactPoint(m, contact);
    if (tri != nullptr) {
        if (FPVector3::Dot(tri->Normal, m->Normal) < 0) {
            return TimeOfImpactResult::Ignore; // approaching the back of the triangle
        }
        m->Normal = tri->Normal;
    }
    m->Normal = -m->Normal;
    if (distance < 0) {
        m->Penetration = -distance;
    }
    return TimeOfImpactResult::Collision;
}

// 0x7d50b74 - conservative advancement as in Gjk2D::ComputeTimeOfImpact, with a fixed
// push-back budget.
TimeOfImpactResult Gjk3D::ComputeTimeOfImpact(NarrowTls3D& tls, CollisionManifoldInfo3D* mInfo,
                                              const CCDSettings& settings, bool detailedInfo,
                                              FP deltaTime, FP& timeOfImpact) {
    SimplexDataPointers3D* ptrs = &tls.SimplexPointers;
    SimplexDataPointers3D::ResetPointers(ptrs, &tls.SimplexData);
    const Entry3D* entryA = mInfo->EntryA;
    const Entry3D* entryB = mInfo->EntryB;
    const TriangleCCW* tri = mInfo->Tri;
    CollisionManifold3D* manifold = mInfo->Manifold;
    const FP tolerance = settings.CCDDistanceTolerance;
    const FP radii = SphereRadius(*entryA) + SphereRadius(*entryB);
    const FP endTime = FP::FromRaw(deltaTime.RawValue() << TimeShift);

    auto hit = [&](FP t, FP distance) {
        timeOfImpact = FP::FromRaw(t.RawValue() >> TimeShift);
        manifold->_m.TimeOfImpact = timeOfImpact;
        if (detailedInfo) {
            return ComputeTimeOfImpactDetailedInfo(ptrs, manifold, entryA, entryB, tri, distance);
        }
        return TimeOfImpactResult::Collision;
    };

    FP t1;
    for (std::int32_t outer = settings.MaxTimeOfImpactIterations; outer >= 1; --outer) {
        FP distance;
        FPVector3 axis;
        ComputeDistance(ptrs, entryA, entryB, tri, distance, axis, settings.MaxTimeOfImpactIterations,
                        t1, TimeShift);
        if (distance != 0) {
            manifold->Normal = axis;
        } else if (t1 == 0) {
            timeOfImpact = FP::Zero();
            return TimeOfImpactResult::Overlapping;
        }
        if (distance < tolerance) {
            return hit(t1, distance);
        }

        SeparationFunction3D separation{};
        separation.Initialize(ptrs, mInfo, t1, TimeShift);
        FP t2 = endTime;
        for (std::int32_t pushBack = PushBackIterations; pushBack >= 0; --pushBack) {
            const FP s2 = separation.GetMinimumSeparation(t2, TimeShift) - radii;
            if (s2 > tolerance) {
                timeOfImpact = FP::FromRaw(endTime.RawValue() >> TimeShift);
                return TimeOfImpactResult::Ignore;
            }
            if (s2 > -tolerance) {
                t1 = t2;
                break;
            }
            const FP s1 = separation.GetSeparation(t1, TimeShift) - radii;
            if (s1 <= tolerance) {
                return hit(t1, distance);
            }
            FP a1 = t1;
            FP a2 = t2;
            FP sA = s1;
            FP sB = s2;
            for (std::int32_t root = settings.MaxRootFindingIterations; root > 0; --root) {
                const FP span = a2 - a1;
                if (span == 0) {
                    timeOfImpact = FP::Zero();
                    return TimeOfImpactResult::Failed;
                }
                FP t;
                bool secant = false;
                if ((root & 1) == 0) {
                    const FP::Raw divisor = (sA - sB).RawValue();
                    const FP::Raw step =
                        divisor == 0 ? 0 : fpdetail::WrapMul(span.RawValue(), sA.RawValue()) / divisor;
                    if (step >= 1 && step < span.RawValue()) {
                        t = FP::FromRaw(step) + a1;
                        secant = true;
                    }
                }
                if (!secant) {
                    t = FP::FromRaw((a1 + a2).RawValue() >> 1);
                }
                const FP s = separation.GetSeparation(t, TimeShift) - radii;
                if (s < tolerance && s > -tolerance) {
                    t2 = t;
                    break;
                }
                if (s > 0) {
                    a1 = t;
                    sA = s;
                } else {
                    a2 = t;
                    sB = s;
                }
            }
        }
    }
    timeOfImpact = FP::Zero();
    return TimeOfImpactResult::Failed;
}

// 0x7d5122c
bool Gjk3D::ShapeCastManifold(NarrowTls3D& tls, const Entry3D* entryA, const Entry3D* entryB,
                              const TriangleCCW* tri, const GJKSettings& settings,
                              const FPVector3& castDistance, bool computeDetailedInfo, Hit3D& hit) {
    if (tri != nullptr && FPVector3::Dot(tri->Normal, castDistance) > 0) {
        hit = Hit3D{};
        return false;
    }
    SimplexDataPointers3D* ptrs = &tls.SimplexPointers;
    SimplexDataPointers3D::ResetPointers(ptrs, &tls.SimplexData);
    const FP radiusB = SphereRadius(*entryB);
    const FP radii = SphereRadius(*entryA) + radiusB;
    std::int16_t shift = settings._simplexMinBitShift;
    std::int32_t iterations = settings._shapeCastMaxIterations;
    std::int32_t squareShift = 2 * shift + 16;
    hit = Hit3D{};
    const FP hardRange = settings._shapeCastHardTolerance + radii;
    const FP hardToleranceSquared = hardRange * hardRange;

    FPVector3 seedDirection = -castDistance;
    GetSimplexVertex(entryA, entryB, tri, seedDirection, ptrs->VertexPointers[0], shift, std::nullopt, 0);
    FPVector3 v = ptrs->VertexPointers[0]->Point;
    FP bestDistanceSquared = ShiftedSqrMagnitude(v, squareShift);
    FPVector3 travelled;
    bool degenerate = false;
    bool moved = false; // the simplex was moved with B (or precision was added)

    std::int32_t remaining = iterations;
    if (bestDistanceSquared > hardToleranceSquared && iterations >= 1) {
        for (;;) {
            SimplexVertex3D support;
            FPVector3 search = -v;
            GetSimplexVertex(entryA, entryB, tri, search, &support, shift, std::nullopt, 0);
            v = FPVector3::Normalize(v);
            const FP vDotCast = FPVector3::Dot(castDistance, v);
            const auto shifted = [&](FP a, FP b) {
                return fpdetail::WrapMul(a.RawValue(), b.RawValue()) >> ((shift + 16) & 63);
            };
            const FP vDotSupport =
                FP::FromRaw(shifted(v.X, support.Point.X) - radii.RawValue() +
                            (shifted(v.Y, support.Point.Y) + shifted(v.Z, support.Point.Z)));

            moved = false;
            if (vDotSupport > vDotCast * hit.CastDistanceNormalized) {
                if (vDotCast <= 0) {
                    return false;
                }
                const FP fraction = vDotSupport / vDotCast;
                const FP step = fraction - hit.CastDistanceNormalized;
                hit.CastDistanceNormalized = fraction;
                if (fraction > FP::One()) {
                    return false;
                }
                hit.Normal = v;
                const FPVector3 move = castDistance * step;
                travelled = travelled + move;
                if (ptrs->UsedCount >= 1) {
                    for (std::int32_t i = 0; i < ptrs->UsedCount; ++i) {
                        SimplexVertex3D* vertex = ptrs->VertexPointers[i];
                        vertex->SupportA = vertex->SupportA + move;
                        vertex->Point = vertex->Point - ShiftLeft(move, shift);
                    }
                    moved = true;
                }
            }

            // SupportA holds B's moved support, SupportB A's (Point = SupportB - SupportA).
            SimplexVertex3D* vertex = ptrs->VertexPointers[ptrs->UsedCount++];
            vertex->SupportA = support.SupportB + travelled;
            vertex->SupportB = support.SupportA;
            vertex->Point = ShiftLeft(vertex->SupportB - vertex->SupportA, shift);
            remaining = iterations - 1;

            if (IsAffinelyDependent(ptrs)) {
                if (settings._simplexMaxBitShift > shift) {
                    ++shift;
                    squareShift += 2;
                    --ptrs->UsedCount;
                    for (std::int32_t i = 0; i < ptrs->UsedCount; ++i) {
                        SimplexVertex3D* kept = ptrs->VertexPointers[i];
                        kept->Point = kept->Point + kept->Point;
                    }
                    moved = true;
                } else if (!moved) {
                    --ptrs->UsedCount;
                    break;
                } else {
                    const std::int32_t last = ptrs->UsedCount - 1;
                    std::swap(ptrs->VertexPointers[0], ptrs->VertexPointers[last]);
                    ptrs->UsedCount = 1;
                    moved = true;
                }
            }

            ComputeClosestPoint(ptrs, shift, v, degenerate);
            if (ptrs->UsedCount == 4 || degenerate) {
                // Fall back to the last triangle.
                ptrs->UsedCount = 3;
                if (moved) {
                    bool unused = false;
                    ComputeClosestPoint(ptrs, shift, v, unused);
                }
                break;
            }
            const FP distanceSquared = ShiftedSqrMagnitude(v, squareShift);
            if (distanceSquared < bestDistanceSquared) {
                bestDistanceSquared = distanceSquared;
            }
            const bool more = distanceSquared > hardToleranceSquared && iterations > 1;
            iterations = remaining;
            if (!more) {
                break;
            }
        }
    }

    if (hit.CastDistanceNormalized == 0) {
        return false;
    }
    if (remaining == 0) {
        const FP softRange = settings._shapeCastSoftTolerance + radii;
        if (bestDistanceSquared > softRange * softRange) {
            return false;
        }
    }
    hit.Normal = -hit.Normal;
    if (computeDetailedInfo) {
        GetWitnessPointA(ptrs, hit.Point);
        if (tri != nullptr) {
            hit.Normal = tri->Normal;
        }
        hit.Point = hit.Point - hit.Normal * radiusB;
    }
    return true;
}

} // namespace Quantum::Physics3D
