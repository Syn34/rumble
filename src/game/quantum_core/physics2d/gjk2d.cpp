// gjk2d.cpp - Quantum.Physics2D.Gjk2D, its simplex and separation function, and the
// support mapping of 2D entries (Entry2D.GetSupportOffset).
#include "gjk2d.h"
#include "narrow_checks_detail.h"

#include "quantum_core/gjk_settings.h"

#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <algorithm>
#include <cassert>

namespace Quantum::Physics2D {

namespace fpdetail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;
using NarrowChecks::detail::ToLocal;

namespace {

// Distance tolerance of ComputeDistance: never below raw 6.
constexpr FP::Raw MinDistanceToleranceRaw = 6;
// ComputeDistance stops once a new support point improves the distance by less than this
// fraction (about 0.999).
constexpr FP ProgressThreshold = FP::FromRaw(65470);
// Precision bits of the time-of-impact search: times are raw FP values << 14.
constexpr std::int32_t TimeShift = 14;
// Collinearity / coincidence limits of IsAffinelyDependent (raw).
constexpr FP::Raw CoincidentRaw = 9;
constexpr FP::Raw CollinearRaw = 8;

// value * time, where the time carries `dtShift` extra fractional bits.
FP ScaleByTime(FP value, FP time, std::int32_t dtShift) {
    return FP::FromRaw(fpdetail::WrapMul(value.RawValue(), time.RawValue()) >> ((dtShift + 16) & 63));
}

FPVector2 ScaleByTime(FPVector2 value, FP time, std::int32_t dtShift) {
    return {ScaleByTime(value.X, time, dtShift), ScaleByTime(value.Y, time, dtShift)};
}

FP ShiftLeft(FP value, std::int32_t bits) {
    return FP::FromRaw(static_cast<FP::Raw>(static_cast<std::uint64_t>(value.RawValue()) << (bits & 63)));
}

FPVector2 ShiftLeft(FPVector2 value, std::int32_t bits) {
    return {ShiftLeft(value.X, bits), ShiftLeft(value.Y, bits)};
}

// Squared length of a simplex point that carries `bits` extra precision bits, in plain FP
// units.
FP ShiftedSqrMagnitude(FPVector2 point, std::int32_t shift) {
    const FP::Raw x = fpdetail::WrapMul(point.X.RawValue(), point.X.RawValue()) >> (shift & 63);
    const FP::Raw y = fpdetail::WrapMul(point.Y.RawValue(), point.Y.RawValue()) >> (shift & 63);
    return FP::FromRaw(x + y);
}

FP CircleRadius(const NarrowEntry2D& entry) {
    return entry.Shape->Type() == Shape2DType::Circle ? entry.Shape->Circle.Radius : FP::Zero();
}

// World position of the entry's centre of mass.
FPVector2 CentreOfMass(const NarrowEntry2D& entry) {
    return entry.TransformPosition + entry.RotatedCenterOfMass;
}

const NarrowBody2D& BodyOf(const NarrowEntry2D& entry) {
    assert(entry.Body != nullptr);
    return *entry.Body;
}

FPVector2 RotateByAngle(FPVector2 v, FP angle) {
    FP sin;
    FP cos;
    FPMath::SinCos(angle, sin, cos);
    return FPVector2::Rotate(v, sin, cos);
}

// Number of support vertices of a shape, as the time-of-impact search counts them.
std::int32_t SupportVertexCount(const NarrowEntry2D& entry) {
    switch (entry.Shape->Type()) {
    case Shape2DType::Circle:
        return 1;
    case Shape2DType::Edge:
        return 2;
    case Shape2DType::Box:
        return 4;
    case Shape2DType::Polygon:
        return entry.Polygon.VerticesCount;
    default:
        assert(false && "Gjk2D: unsupported shape type");
        return 0;
    }
}

} // namespace

// 0x7d17d70
void ComputeLocalDirectionAndCentroidOffset(const NarrowEntry2D& entry, FPVector2 worldDir,
                                            FP& sin, FP& cos, FPVector2& localDir,
                                            FPVector2& centroidOffset,
                                            std::optional<FP> deltaTime, std::int32_t dtShift) {
    centroidOffset = FPVector2::Zero();
    if (deltaTime.has_value() && entry.HasDynamicBody && BodyOf(entry).AngularVelocity != 0) {
        FP angleSin;
        FP angleCos;
        FPMath::SinCos(ScaleByTime(BodyOf(entry).AngularVelocity, *deltaTime, dtShift), angleSin,
                       angleCos);
        // The shape turns about the centre of mass, which moves its narrow position.
        const FPVector2 pivot = CentreOfMass(entry) - entry.Narrow.Position;
        if (pivot != FPVector2::Zero()) {
            centroidOffset = pivot - FPVector2::Rotate(pivot, angleSin, angleCos);
        }
        const FP rotatedSin = cos * angleSin + sin * angleCos;
        const FP rotatedCos = cos * angleCos - sin * angleSin;
        sin = rotatedSin;
        cos = rotatedCos;
    }
    localDir = ToLocal(worldDir, sin, cos);
}

// 0x7d17958
FPVector2 GetSupportOffset(const NarrowEntry2D& entry, FPVector2 worldDir,
                           std::optional<FP> deltaTime, std::int32_t dtShift) {
    const Shape2D& shape = *entry.Shape;
    FPVector2 offset;
    FP sin = entry.Narrow.RotSin;
    FP cos = entry.Narrow.RotCos;
    FPVector2 localDir;
    FPVector2 centroidOffset;
    switch (shape.Type()) {
    case Shape2DType::Polygon: {
        ComputeLocalDirectionAndCentroidOffset(entry, worldDir, sin, cos, localDir, centroidOffset,
                                               deltaTime, dtShift);
        const FPVector2 support = FPVector2::CalculatePolygonLocalSupport(
            entry.Polygon.Vertices, entry.Polygon.VerticesCount, localDir);
        offset = FPVector2::Rotate(support, sin, cos) + centroidOffset;
        break;
    }
    case Shape2DType::Edge: {
        ComputeLocalDirectionAndCentroidOffset(entry, worldDir, sin, cos, localDir, centroidOffset,
                                               deltaTime, dtShift);
        const FP extent = localDir.X < 0 ? -shape.Edge.Extent : shape.Edge.Extent;
        offset = FPVector2::Rotate({extent, FP::Zero()}, sin, cos) + centroidOffset;
        break;
    }
    case Shape2DType::Box: {
        ComputeLocalDirectionAndCentroidOffset(entry, worldDir, sin, cos, localDir, centroidOffset,
                                               deltaTime, dtShift);
        const FPVector2 extents = shape.Box._extents;
        const FPVector2 corner = {localDir.X < 0 ? -extents.X : extents.X,
                                  localDir.Y < 0 ? -extents.Y : extents.Y};
        offset = FPVector2::Rotate(corner, sin, cos) + centroidOffset;
        break;
    }
    case Shape2DType::Circle:
        // The radius is handled by the callers; only the centre moves (turning about the
        // centre of mass).
        if (deltaTime.has_value() && entry.HasDynamicBody &&
            entry.RotatedCenterOfMass != FPVector2::Zero()) {
            const FP angle = ScaleByTime(BodyOf(entry).AngularVelocity, *deltaTime, dtShift);
            offset = entry.RotatedCenterOfMass - RotateByAngle(entry.RotatedCenterOfMass, angle);
        }
        break;
    default:
        assert(false && "Entry2D.GetSupportOffset: unsupported shape type");
        return {};
    }
    if (deltaTime.has_value() && entry.HasDynamicBody) {
        offset = offset + ScaleByTime(BodyOf(entry).Velocity, *deltaTime, dtShift);
    }
    return offset;
}

// 0x7d82b0c
void SimplexDataPointers2D::ResetPointers(SimplexDataPointers2D* ptrs, SimplexData2D* simplex) {
    ptrs->Data = simplex;
    ptrs->UsedCount = 0;
    ptrs->VertexPointers[0] = &simplex->Vertex0;
    ptrs->VertexPointers[1] = &simplex->Vertex1;
    ptrs->VertexPointers[2] = &simplex->Vertex2;
    ptrs->Lambdas = &simplex->Lambda0;
}

// 0x7d823f4
void Gjk2D::GetSimplexVertex(const NarrowEntry2D* entryA, const NarrowEntry2D* entryB,
                             FPVector2 worldDir, SimplexVertex2D* vertex, std::int32_t bitShift,
                             std::optional<FP> deltaTime, std::int32_t dtShift) {
    vertex->SupportA = entryA->Narrow.Position + GetSupportOffset(*entryA, worldDir, deltaTime, dtShift);
    vertex->SupportB =
        entryB->Narrow.Position + GetSupportOffset(*entryB, -worldDir, deltaTime, dtShift);
    vertex->Point = ShiftLeft(vertex->SupportA - vertex->SupportB, bitShift);
}

// 0x7d82154
void Gjk2D::ComputeDistance(SimplexDataPointers2D* ptrs, const NarrowEntry2D* entryA,
                            const NarrowEntry2D* entryB, FP& distance, FPVector2& separationAxis,
                            std::int32_t iterations, std::optional<FP> deltaTime,
                            std::int32_t dtShift) {
    ptrs->UsedCount = 0;
    const FP radii = CircleRadius(*entryA) + CircleRadius(*entryB);
    ptrs->UsedCount = 1;
    GetSimplexVertex(entryA, entryB, entryB->Narrow.Position - entryA->Narrow.Position,
                     ptrs->VertexPointers[0], 0, deltaTime, dtShift);
    FPVector2 closest = ptrs->VertexPointers[0]->Point;
    FP distanceSquared = closest.SqrMagnitude();
    const FP grown = radii + fpdetail::Epsilon;
    const FP tolerance = FP::FromRaw(std::max(MinDistanceToleranceRaw, (grown * grown).RawValue()));

    while (iterations-- > 0 && distanceSquared > tolerance) {
        const std::int32_t index = ptrs->UsedCount;
        ptrs->UsedCount = index + 1;
        GetSimplexVertex(entryA, entryB, -closest, ptrs->VertexPointers[index], 0, deltaTime, dtShift);
        if (IsAffinelyDependent(ptrs)) {
            --ptrs->UsedCount;
            break;
        }
        const FPVector2 newest = ptrs->VertexPointers[ptrs->UsedCount - 1]->Point;
        const FP progress = FPVector2::Dot(closest, newest) / distanceSquared;
        ComputeClosestPoint(ptrs, closest);
        distanceSquared = closest.SqrMagnitude();
        if (progress > ProgressThreshold || ptrs->UsedCount == 3) {
            break;
        }
    }

    const FP gap = FPMath::Sqrt(distanceSquared) - radii;
    distance = gap;
    if (gap > 0) {
        separationAxis = FPVector2::Normalize(closest);
    } else {
        distance = FP::Zero();
        separationAxis = FPVector2::Zero();
    }
}

// 0x7d82600
bool Gjk2D::ShapeCastManifold(NarrowTls2D& tls, const NarrowEntry2D* entryA,
                              const NarrowEntry2D* entryB, const GJKSettings& settings,
                              FPVector2 castDistance, bool computeDetailedInfo, Hit& hit) {
    SimplexDataPointers2D* ptrs = &tls.SimplexPointers;
    SimplexDataPointers2D::ResetPointers(ptrs, &tls.SimplexData);
    const FP radiusB = CircleRadius(*entryB);
    const FP radii = CircleRadius(*entryA) + radiusB;
    std::int16_t shift = settings._simplexMinBitShift;
    std::int32_t iterations = settings._shapeCastMaxIterations;
    // Squared lengths of shifted points drop 16 + 2 * shift bits.
    std::int32_t squareShift = 2 * shift + 16;
    hit = Hit{};
    const FP hardRange = settings._shapeCastHardTolerance + radii;
    const FP hardToleranceSquared = hardRange * hardRange;

    // GJK ray cast (van den Bergen): v is the closest point of the Minkowski difference of
    // A and the cast-moved B to the origin. The first support point only seeds v; it is
    // not part of the simplex.
    GetSimplexVertex(entryA, entryB, -castDistance, ptrs->VertexPointers[0], shift, std::nullopt, 0);
    FPVector2 v = ptrs->VertexPointers[0]->Point;
    FP bestDistanceSquared = ShiftedSqrMagnitude(v, squareShift);
    FPVector2 travelled; // castDistance * fraction so far

    std::int32_t remaining = iterations;
    if (bestDistanceSquared > hardToleranceSquared && iterations >= 1) {
        for (;;) {
            SimplexVertex2D support;
            GetSimplexVertex(entryA, entryB, -v, &support, shift, std::nullopt, 0);
            v = FPVector2::Normalize(v);
            const FP vDotCast = FPVector2::Dot(v, castDistance);
            const FP::Raw supportX = fpdetail::WrapMul(v.X.RawValue(), support.Point.X.RawValue());
            const FP::Raw supportY = fpdetail::WrapMul(v.Y.RawValue(), support.Point.Y.RawValue());
            const FP vDotSupport = FP::FromRaw((supportX >> ((shift + 16) & 63)) - radii.RawValue() +
                                               (supportY >> ((shift + 16) & 63)));

            bool advanced = false;
            if (vDotSupport > vDotCast * hit.CastDistanceNormalized) {
                // The support plane separates: advance the cast to it.
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
                const FPVector2 move = castDistance * step;
                travelled = travelled + move;
                if (ptrs->UsedCount >= 1) {
                    // Move the simplex with B.
                    for (std::int32_t i = 0; i < ptrs->UsedCount; ++i) {
                        SimplexVertex2D* vertex = ptrs->VertexPointers[i];
                        vertex->SupportA = vertex->SupportA + move;
                        vertex->Point = vertex->Point - ShiftLeft(move, shift);
                    }
                    advanced = true;
                }
            }

            // Append the new point. In this simplex SupportA holds B's (moved) support and
            // SupportB holds A's, so Point = SupportB - SupportA.
            SimplexVertex2D* vertex = ptrs->VertexPointers[ptrs->UsedCount++];
            vertex->SupportA = support.SupportB + travelled;
            vertex->SupportB = support.SupportA;
            vertex->Point = ShiftLeft(vertex->SupportB - vertex->SupportA, shift);
            remaining = iterations - 1;

            if (IsAffinelyDependent(ptrs)) {
                if (settings._simplexMaxBitShift > shift) {
                    // Degenerate at this precision: drop the point, add a bit and retry.
                    ++shift;
                    squareShift += 2;
                    --ptrs->UsedCount;
                    for (std::int32_t i = 0; i < ptrs->UsedCount; ++i) {
                        ptrs->VertexPointers[i]->Point = ptrs->VertexPointers[i]->Point +
                                                         ptrs->VertexPointers[i]->Point;
                    }
                } else if (!advanced) {
                    --ptrs->UsedCount;
                    break;
                } else {
                    // Keep only the newest point.
                    const std::int32_t last = ptrs->UsedCount - 1;
                    std::swap(ptrs->VertexPointers[0], ptrs->VertexPointers[last]);
                    ptrs->UsedCount = 1;
                }
            }

            ComputeClosestPoint(ptrs, v);
            if (ptrs->UsedCount == 3) {
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
        return false; // overlapping at the start
    }
    if (remaining == 0) {
        // Out of iterations: accept only within the soft tolerance.
        const FP softRange = settings._shapeCastSoftTolerance + radii;
        if (bestDistanceSquared > softRange * softRange) {
            return false;
        }
    }
    hit.Normal = -hit.Normal;
    if (computeDetailedInfo) {
        GetWitnessPointA(ptrs, hit.Point);
        hit.Point = hit.Point - hit.Normal * radiusB;
    }
    return true;
}

// 0x7d82b40
void Gjk2D::GetWitnessPoints(const SimplexDataPointers2D* ptrs, FPVector2& pointA,
                             FPVector2& pointB) {
    GetWitnessPointA(ptrs, pointA);
    GetWitnessPointB(ptrs, pointB);
}

// 0x7d82d10
void Gjk2D::GetWitnessPointA(const SimplexDataPointers2D* ptrs, FPVector2& pointA) {
    const FP* lambdas = ptrs->Lambdas;
    switch (ptrs->UsedCount) {
    case 1:
        pointA = ptrs->VertexPointers[0]->SupportA;
        break;
    case 2:
        pointA = ptrs->VertexPointers[0]->SupportA * lambdas[0] +
                 ptrs->VertexPointers[1]->SupportA * lambdas[1];
        break;
    case 3:
        pointA = ptrs->VertexPointers[0]->SupportA * lambdas[0] +
                 ptrs->VertexPointers[1]->SupportA * lambdas[1] +
                 ptrs->VertexPointers[2]->SupportA * lambdas[2];
        break;
    default:
        pointA = FPVector2::Zero();
        break;
    }
}

// 0x7d82e30
void Gjk2D::GetWitnessPointB(const SimplexDataPointers2D* ptrs, FPVector2& pointB) {
    const FP* lambdas = ptrs->Lambdas;
    switch (ptrs->UsedCount) {
    case 1:
        pointB = ptrs->VertexPointers[0]->SupportB;
        break;
    case 2:
        pointB = ptrs->VertexPointers[0]->SupportB * lambdas[0] +
                 ptrs->VertexPointers[1]->SupportB * lambdas[1];
        break;
    case 3:
        pointB = ptrs->VertexPointers[0]->SupportB * lambdas[0] +
                 ptrs->VertexPointers[1]->SupportB * lambdas[1] +
                 ptrs->VertexPointers[2]->SupportB * lambdas[2];
        break;
    default:
        pointB = FPVector2::Zero();
        break;
    }
}

// 0x7d8250c
bool Gjk2D::IsAffinelyDependent(const SimplexDataPointers2D* ptrs) {
    switch (ptrs->UsedCount) {
    case 2: {
        const FPVector2 a = ptrs->VertexPointers[0]->Point;
        const FPVector2 b = ptrs->VertexPointers[1]->Point;
        return FPVector2::DistanceSquared(a, b).RawValue() < CoincidentRaw;
    }
    case 3: {
        const FPVector2 a = ptrs->VertexPointers[0]->Point;
        const FPVector2 b = ptrs->VertexPointers[1]->Point;
        const FPVector2 c = ptrs->VertexPointers[2]->Point;
        const FP::Raw area = FPVector2::Cross(b - a, c - a).RawValue();
        return area >= -CollinearRaw && area <= CollinearRaw;
    }
    default:
        return false;
    }
}

// 0x7d825a8
void Gjk2D::ComputeClosestPoint(SimplexDataPointers2D* ptrs, FPVector2& closestPoint) {
    switch (ptrs->UsedCount) {
    case 1:
        closestPoint = ptrs->VertexPointers[0]->Point;
        break;
    case 2:
        SolveLine(ptrs, ptrs->Lambdas, closestPoint);
        break;
    case 3:
        SolveTriangle(ptrs, ptrs->Lambdas, closestPoint);
        break;
    default:
        closestPoint = FPVector2::Zero();
        break;
    }
}

// 0x7d832b8
void Gjk2D::SolveLine(SimplexDataPointers2D* ptrs, FP* lambdas, FPVector2& closestPoint) {
    SimplexVertex2D* const vertexA = ptrs->VertexPointers[0];
    SimplexVertex2D* const vertexB = ptrs->VertexPointers[1];
    const FPVector2 a = vertexA->Point;
    const FPVector2 b = vertexB->Point;
    const FPVector2 ba = a - b;
    const FP t = FPVector2::Dot(ba, a); // origin beyond a?
    if (t <= 0) {
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = a;
        return;
    }
    const FP lengthSquared = ba.SqrMagnitude();
    if (t >= lengthSquared) {
        // Beyond b: keep b only.
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

// 0x7d82f50 - Voronoi regions of the triangle (Ericson's ClosestPtPointTriangle for the
// origin), testing the newest vertex C first.
void Gjk2D::SolveTriangle(SimplexDataPointers2D* ptrs, FP* lambdas, FPVector2& closestPoint) {
    SimplexVertex2D* const pa = ptrs->VertexPointers[0];
    SimplexVertex2D* const pb = ptrs->VertexPointers[1];
    SimplexVertex2D* const pc = ptrs->VertexPointers[2];
    const FPVector2 a = pa->Point;
    const FPVector2 b = pb->Point;
    const FPVector2 c = pc->Point;
    const FPVector2 ab = b - a;
    const FPVector2 ac = c - a;
    const FP half = FP::Half();
    const FP third = FP::FromRaw(0x5555);

    auto keepOne = [&](SimplexVertex2D* first, SimplexVertex2D* other, int otherSlot, FPVector2 point) {
        ptrs->VertexPointers[0] = first;
        ptrs->VertexPointers[otherSlot] = other;
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = point;
    };

    // Vertex C.
    const FP acDotC = FPVector2::Dot(ac, c);
    const FP abDotC = FPVector2::Dot(ab, c);
    if (acDotC <= 0 && acDotC <= abDotC) {
        keepOne(pc, pa, 2, c);
        return;
    }
    // Vertex A.
    const FP abDotA = FPVector2::Dot(ab, a);
    const FP acDotA = FPVector2::Dot(ac, a);
    if (acDotA >= 0 && abDotA >= 0) {
        ptrs->UsedCount = 1;
        lambdas[0] = FP::One();
        closestPoint = a;
        return;
    }
    // Edge AC.
    const FP vb = acDotA * abDotC - acDotC * abDotA;
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
    // Vertex B.
    const FP abDotB = FPVector2::Dot(ab, b);
    const FP acDotB = FPVector2::Dot(ac, b);
    if (abDotB <= 0 && abDotB <= acDotB) {
        keepOne(pb, pa, 1, b);
        return;
    }
    // Edge BC.
    const FP va = acDotC * abDotB - acDotB * abDotC;
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
        lambdas[0] = w; // weight of C (now first)
        lambdas[1] = u;
        return;
    }
    // Edge AB.
    const FP vc = abDotA * acDotB - abDotB * acDotA;
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
    // Inside: barycentric weights.
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

// 0x7d83b5c
void SeparationFunction2D::Initialize(const SimplexDataPointers2D* ptrs, const NarrowEntry2D* entryA,
                                      const NarrowEntry2D* entryB, FP deltaTime,
                                      std::int32_t dtBitshift) {
    if (ptrs->UsedCount == 1) {
        _closestFeatures = PointPoint;
        _entry = entryA;
        _edgeEntry = entryB;
        _comPoint = CentreOfMass(*entryA);
        _edgeComPoint = CentreOfMass(*entryB);
        FPVector2 pointA;
        FPVector2 pointB;
        Gjk2D::GetWitnessPoints(ptrs, pointA, pointB);
        _separationAxis = FPVector2::Normalize(pointA - pointB);
        return;
    }

    // Two vertices: one entry contributes the same point twice, the other an edge.
    _closestFeatures = PointEdge;
    const SimplexVertex2D* v0 = ptrs->VertexPointers[0];
    const SimplexVertex2D* v1 = ptrs->VertexPointers[1];
    FPVector2 point;
    FPVector2 edge0;
    FPVector2 edge1;
    if (v0->SupportA == v1->SupportA) {
        _entry = entryA;
        _edgeEntry = entryB;
        point = v0->SupportA;
        edge0 = v0->SupportB;
        edge1 = v1->SupportB;
    } else {
        _entry = entryB;
        _edgeEntry = entryA;
        point = v0->SupportB;
        edge0 = v0->SupportA;
        edge1 = v1->SupportA;
    }
    _comPoint = CentreOfMass(*_entry);
    _edgeComPoint = CentreOfMass(*_edgeEntry);

    const FPVector2 midpoint = {FP::FromRaw((edge0.X + edge1.X).RawValue() >> 1),
                                FP::FromRaw((edge0.Y + edge1.Y).RawValue() >> 1)};
    const FPVector2 along = edge1 - edge0;
    _separationAxis = {-along.Y, along.X};
    if (FPVector2::Dot(_separationAxis, point - midpoint) < 0) {
        _separationAxis = -_separationAxis; // face the point
    }
    _edgeComOffsetPoint = midpoint - _edgeComPoint;

    // Undo the edge entry's motion up to `deltaTime`, so the offset and axis are in its
    // starting pose.
    if (_edgeEntry->HasDynamicBody && deltaTime != 0) {
        const NarrowBody2D& body = BodyOf(*_edgeEntry);
        _edgeComOffsetPoint = _edgeComOffsetPoint - ScaleByTime(body.Velocity, deltaTime, dtBitshift);
        if (body.AngularVelocity != 0) {
            const FP angle = ScaleByTime(-body.AngularVelocity, deltaTime, dtBitshift);
            FP sin;
            FP cos;
            FPMath::SinCos(angle, sin, cos);
            _edgeComOffsetPoint = FPVector2::Rotate(_edgeComOffsetPoint, sin, cos);
            _separationAxis = FPVector2::Rotate(_separationAxis, sin, cos);
        }
    }
    _separationAxis = FPVector2::Normalize(_separationAxis);
}

// 0x7d83f04
FP SeparationFunction2D::GetMinimumSeparation(FP deltaTime, std::int32_t dtBitshift) {
    // Stores the point of `entry` at `support` as an offset from its centre of mass in the
    // entry's starting pose.
    auto rememberOffset = [&](const NarrowEntry2D& entry, FPVector2 support, FPVector2 comPoint) {
        FPVector2 offset = support - comPoint;
        if (entry.HasDynamicBody) {
            const NarrowBody2D& body = BodyOf(entry);
            offset = offset - ScaleByTime(body.Velocity, deltaTime, dtBitshift);
            if (body.AngularVelocity != 0) {
                offset = RotateByAngle(offset, ScaleByTime(-body.AngularVelocity, deltaTime, dtBitshift));
            }
        }
        return offset;
    };

    if (_closestFeatures == PointPoint) {
        const FPVector2 supportA =
            _entry->Narrow.Position + GetSupportOffset(*_entry, -_separationAxis, deltaTime, dtBitshift);
        const FPVector2 supportB = _edgeEntry->Narrow.Position +
                                   GetSupportOffset(*_edgeEntry, _separationAxis, deltaTime, dtBitshift);
        _comOffsetPoint = rememberOffset(*_entry, supportA, _comPoint);
        _edgeComOffsetPoint = rememberOffset(*_edgeEntry, supportB, _edgeComPoint);
        return FPVector2::Dot(_separationAxis, supportA - supportB);
    }

    // Point against edge: move the edge (and its axis) to `deltaTime`.
    FPVector2 direction = -_separationAxis;
    FPVector2 edgeOffset = _edgeComOffsetPoint;
    if (_edgeEntry->HasDynamicBody) {
        const NarrowBody2D& body = BodyOf(*_edgeEntry);
        if (body.AngularVelocity != 0) {
            FP sin;
            FP cos;
            FPMath::SinCos(ScaleByTime(body.AngularVelocity, deltaTime, dtBitshift), sin, cos);
            direction = FPVector2::Rotate(direction, sin, cos);
            edgeOffset = FPVector2::Rotate(edgeOffset, sin, cos);
        }
        edgeOffset = edgeOffset + ScaleByTime(body.Velocity, deltaTime, dtBitshift);
    }
    const FPVector2 support =
        _entry->Narrow.Position + GetSupportOffset(*_entry, direction, deltaTime, dtBitshift);
    _comOffsetPoint = rememberOffset(*_entry, support, _comPoint);
    return FPVector2::Dot(_edgeComPoint + edgeOffset - support, direction);
}

// 0x7d845fc
FP SeparationFunction2D::GetSeparation(FP deltaTime, std::int32_t dtBitshift) const {
    // Both remembered points (and the edge axis) moved to `deltaTime`.
    FPVector2 axis = _separationAxis;
    FPVector2 comPoint = _comPoint;
    FPVector2 comOffset = _comOffsetPoint;
    if (_entry->HasDynamicBody) {
        const NarrowBody2D& body = BodyOf(*_entry);
        if (body.AngularVelocity != 0) {
            comOffset = RotateByAngle(comOffset, ScaleByTime(body.AngularVelocity, deltaTime, dtBitshift));
        }
        comPoint = comPoint + ScaleByTime(body.Velocity, deltaTime, dtBitshift);
    }
    FPVector2 edgeComPoint = _edgeComPoint;
    FPVector2 edgeOffset = _edgeComOffsetPoint;
    if (_edgeEntry->HasDynamicBody) {
        const NarrowBody2D& body = BodyOf(*_edgeEntry);
        if (body.AngularVelocity != 0) {
            FP sin;
            FP cos;
            FPMath::SinCos(ScaleByTime(body.AngularVelocity, deltaTime, dtBitshift), sin, cos);
            edgeOffset = FPVector2::Rotate(edgeOffset, sin, cos);
            if (_closestFeatures == PointEdge) {
                axis = FPVector2::Rotate(axis, sin, cos);
            }
        }
        edgeComPoint = edgeComPoint + ScaleByTime(body.Velocity, deltaTime, dtBitshift);
    }
    return FPVector2::Dot(axis, (comOffset + comPoint) - (edgeOffset + edgeComPoint));
}

// 0x7d83a00
TimeOfImpactResult Gjk2D::ComputeTimeOfImpactDetailedInfo(SimplexDataPointers2D* ptrs,
                                                          CollisionManifold2D* m,
                                                          const NarrowEntry2D* entryA,
                                                          const NarrowEntry2D* entryB,
                                                          FP distance) {
    FPVector2 contact;
    GetWitnessPointA(ptrs, contact);
    if (m->Normal == FPVector2::Zero()) {
        // No separating axis (touching at the start of the step): use the relative motion.
        FPVector2 motion = m->Normal;
        if (entryA->HasDynamicBody) {
            motion = BodyOf(*entryA).Velocity;
        }
        if (entryB->HasDynamicBody) {
            motion = motion - BodyOf(*entryB).Velocity;
        }
        m->Normal = FPVector2::Normalize(motion);
    }
    if (entryA->Shape->Type() == Shape2DType::Circle) {
        contact = contact + m->Normal * entryA->Shape->Circle.Radius;
    }
    CollisionManifold2D::AddContactPoint(m, contact);
    m->Normal = -m->Normal;
    if (distance < 0) {
        m->Penetration = -distance;
    }
    return TimeOfImpactResult::Collision;
}

// 0x7d83394 - conservative advancement with a separation function and a mixed
// secant/bisection root finder (in the spirit of Box2D's b2TimeOfImpact).
TimeOfImpactResult Gjk2D::ComputeTimeOfImpact(NarrowTls2D& tls, CollisionManifoldInfo2D* mInfo,
                                              const CCDSettings& settings, bool detailedInfo,
                                              FP deltaTime, FP& timeOfImpact) {
    SimplexDataPointers2D* ptrs = &tls.SimplexPointers;
    SimplexDataPointers2D::ResetPointers(ptrs, &tls.SimplexData);
    const NarrowEntry2D* entryA = mInfo->EntryA;
    const NarrowEntry2D* entryB = mInfo->EntryB;
    CollisionManifold2D* manifold = mInfo->Manifold;
    const FP tolerance = settings.CCDDistanceTolerance;
    const FP radii = CircleRadius(*entryA) + CircleRadius(*entryB);
    // How often the far end of the interval may be pulled back per advancement step.
    const std::int32_t pushBackIterations = SupportVertexCount(*entryA) + SupportVertexCount(*entryB);
    const FP endTime = FP::FromRaw(deltaTime.RawValue() << TimeShift);

    FP t1; // start of the remaining interval
    for (std::int32_t outer = settings.MaxTimeOfImpactIterations; outer >= 1; --outer) {
        FP distance;
        FPVector2 axis;
        Gjk2D::ComputeDistance(ptrs, entryA, entryB, distance, axis, settings.MaxTimeOfImpactIterations,
                               t1, TimeShift);
        if (distance != 0) {
            manifold->Normal = axis;
        } else if (t1 == 0) {
            timeOfImpact = FP::Zero();
            return TimeOfImpactResult::Overlapping;
        }
        if (distance < tolerance) {
            timeOfImpact = FP::FromRaw(t1.RawValue() >> TimeShift);
            manifold->_m.TimeOfImpact = timeOfImpact;
            if (detailedInfo) {
                return ComputeTimeOfImpactDetailedInfo(ptrs, manifold, entryA, entryB, distance);
            }
            return TimeOfImpactResult::Collision;
        }

        SeparationFunction2D separation;
        separation.Initialize(ptrs, entryA, entryB, t1, TimeShift);
        FP t2 = endTime;
        for (std::int32_t pushBack = pushBackIterations; pushBack >= 0; --pushBack) {
            const FP s2 = separation.GetMinimumSeparation(t2, TimeShift) - radii;
            if (s2 > tolerance) {
                timeOfImpact = FP::FromRaw(endTime.RawValue() >> TimeShift);
                return TimeOfImpactResult::Ignore; // still apart at the end of the step
            }
            if (s2 > -tolerance) {
                t1 = t2; // touching at t2: advance and re-run GJK there
                break;
            }
            const FP s1 = separation.GetSeparation(t1, TimeShift) - radii;
            if (s1 <= tolerance) {
                timeOfImpact = FP::FromRaw(t1.RawValue() >> TimeShift);
                manifold->_m.TimeOfImpact = timeOfImpact;
                if (detailedInfo) {
                    return ComputeTimeOfImpactDetailedInfo(ptrs, manifold, entryA, entryB, distance);
                }
                return TimeOfImpactResult::Collision;
            }

            // Root of the separation between a1 (apart) and a2 (overlapping).
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
                    // Secant step (raw arithmetic on the shifted times).
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

} // namespace Quantum::Physics2D
