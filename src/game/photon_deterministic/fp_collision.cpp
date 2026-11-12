// fp_collision.cpp - Photon.Deterministic.FPCollision.
#include "fp_collision.h"

#include "fp_internal.h"
#include "fp_math.h"

#include <cstdint>

namespace Photon::Deterministic::FPCollision {

namespace {

// Which side of the directed line b->a the point p lies on, as the 2D cross product
// (a - b) x (p - b) written the way the original orders its terms. All point-in-triangle
// tests are built from three of these.
constexpr FP EdgeSide(FP px, FP py, FP ax, FP ay, FP bx, FP by) {
    return (ay - by) * (px - bx) - (py - by) * (ax - bx);
}

// Inclusive test shared by the 2D and ground-plane overloads: inside unless the point is
// strictly on the positive side of one edge and strictly on the negative side of another.
constexpr bool SidesAllowInside(FP d0, FP d1, FP d2) {
    const bool hasNegative = d0 < 0 || d1 < 0 || d2 < 0;
    const bool hasPositive = d0 > 0 || d1 > 0 || d2 > 0;
    return !(hasNegative && hasPositive);
}

// Weighted sum of points with weights in FP: the products are accumulated at full
// precision and shifted down once, which is how the original blends triangle vertices
// (it differs in the low bits from summing FP products).
FP::Raw BlendRaw(FP v0, FP w0, FP v1, FP w1) {
    return detail::WrapAdd(detail::WrapMul(v0.RawValue(), w0.RawValue()),
                           detail::WrapMul(v1.RawValue(), w1.RawValue()));
}

FPVector3 Blend(const FPVector3& a, FP wa, const FPVector3& b, FP wb) {
    return {FP::FromRaw(BlendRaw(a.X, wa, b.X, wb) >> 16),
            FP::FromRaw(BlendRaw(a.Y, wa, b.Y, wb) >> 16),
            FP::FromRaw(BlendRaw(a.Z, wa, b.Z, wb) >> 16)};
}

FPVector3 Blend(const FPVector3& a, FP wa, const FPVector3& b, FP wb, const FPVector3& c, FP wc) {
    auto component = [&](FP va, FP vb, FP vc) {
        const FP::Raw sum = detail::WrapAdd(BlendRaw(va, wa, vb, wb),
                                            detail::WrapMul(vc.RawValue(), wc.RawValue()));
        return FP::FromRaw(sum >> 16);
    };
    return {component(a.X, b.X, c.X), component(a.Y, b.Y, c.Y), component(a.Z, b.Z, c.Z)};
}

} // namespace

// 0x7a74d18
FPVector3 ClosestPointOnSegment(const FPVector3& point, const FPVector3& p1, const FPVector3& p2) {
    const FP lengthSquared = FPVector3::DistanceSquared(p1, p2);
    if (lengthSquared == 0) {
        return p1;
    }
    const FPVector3 segment = p2 - p1;
    const FP t = FPMath::Clamp01(FPVector3::Dot(point - p1, segment) / lengthSquared);
    return p1 + segment * t;
}

// 0x7a74e6c
FPVector2 ClosestPointOnSegment(FPVector2 point, FPVector2 p1, FPVector2 p2) {
    const FP lengthSquared = FPVector2::DistanceSquared(p1, p2);
    if (lengthSquared == 0) {
        return p1;
    }
    const FPVector2 segment = p2 - p1;
    const FP t = FPMath::Clamp01(FPVector2::Dot(point - p1, segment) / lengthSquared);
    return p1 + segment * t;
}

// 0x7a74f58
bool TriangleContainsPointExclusive(const FPVector3& pt, const FPVector3& v0, const FPVector3& v1,
                                    const FPVector3& v2) {
    const FP d0 = EdgeSide(pt.X, pt.Z, v0.X, v0.Z, v1.X, v1.Z);
    const FP d1 = EdgeSide(pt.X, pt.Z, v1.X, v1.Z, v2.X, v2.Z);
    const FP d2 = EdgeSide(pt.X, pt.Z, v2.X, v2.Z, v0.X, v0.Z);
    return (d0 > 0 && d1 > 0 && d2 > 0) || (d0 < 0 && d1 < 0 && d2 < 0);
}

// 0x7a74ff8
bool TriangleContainsPointInclusive(FPVector2 pt, FPVector2 v0, FPVector2 v1, FPVector2 v2) {
    return SidesAllowInside(EdgeSide(pt.X, pt.Y, v0.X, v0.Y, v1.X, v1.Y),
                            EdgeSide(pt.X, pt.Y, v1.X, v1.Y, v2.X, v2.Y),
                            EdgeSide(pt.X, pt.Y, v2.X, v2.Y, v0.X, v0.Y));
}

// 0x7a75078
bool TriangleContainsPointInclusive(const FPVector3& pt, const FPVector3& v0, const FPVector3& v1,
                                    const FPVector3& v2) {
    return SidesAllowInside(EdgeSide(pt.X, pt.Z, v0.X, v0.Z, v1.X, v1.Z),
                            EdgeSide(pt.X, pt.Z, v1.X, v1.Z, v2.X, v2.Z),
                            EdgeSide(pt.X, pt.Z, v2.X, v2.Z, v0.X, v0.Z));
}

// 0x7a75118
bool CircleIntersectsCircle(FPVector2 a_origin, FP a_radius, FPVector2 b_origin, FP b_radius) {
    const FP radii = a_radius + b_radius;
    return FPVector2::DistanceSquared(a_origin, b_origin) <= radii * radii;
}

// 0x7a75144
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2) {
    const FPVector2 r = p2 - p1;
    const FPVector2 s = q2 - q1;
    const FP denominator = FPVector2::Cross(r, s);
    if (denominator == 0) {
        return false; // parallel or collinear
    }
    const FPVector2 startOffset = q1 - p1;
    const FP t = FPVector2::Cross(startOffset, s) / denominator;
    if (t < 0 || t > 1) {
        return false;
    }
    const FP u = FPVector2::Cross(startOffset, r) / denominator;
    return u >= 0 && u <= 1;
}

// 0x7a751c8
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point,
                        FP& distance) {
    FP normalizedDist;
    return LineIntersectsLine(p1, p2, q1, q2, point, distance, normalizedDist);
}

// 0x7a751f4
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point,
                        FP& distance, FP& normalizedDist) {
    const FPVector2 r = p2 - p1;
    const FPVector2 s = q2 - q1;
    const FPVector2 startOffset = q1 - p1;
    const FP denominator = FPVector2::Cross(r, s);
    if (denominator != 0) {
        const FP t = FPVector2::Cross(startOffset, s) / denominator;
        const FP u = FPVector2::Cross(startOffset, r) / denominator;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            const FPVector2 along = {t * r.X, t * r.Y};
            normalizedDist = t;
            distance = along.Magnitude();
            point = p1 + along;
            return true;
        }
    }
    point = FPVector2::Zero();
    distance = FP::Zero();
    normalizedDist = FP::Zero();
    return false;
}

// 0x7a75304
bool LineIntersectsLine(FPVector2 p1, FPVector2 p2, FPVector2 q1, FPVector2 q2, FPVector2& point) {
    const FPVector2 r = p2 - p1;
    const FPVector2 s = q2 - q1;
    const FPVector2 startOffset = q1 - p1;
    const FP denominator = FPVector2::Cross(r, s);
    if (denominator != 0) {
        const FP t = FPVector2::Cross(startOffset, s) / denominator;
        const FP u = FPVector2::Cross(startOffset, r) / denominator;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            point = {p1.X + t * r.X, p1.Y + t * r.Y};
            return true;
        }
    }
    point = FPVector2::Zero();
    return false;
}

// 0x7a753bc
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          bool ignoreIfStartPointInside) {
    FPVector2 point;
    FP normalizedDist;
    return LineIntersectsCircle(p1, p2, position, radius, point, normalizedDist,
                                ignoreIfStartPointInside);
}

// 0x7a75590
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          FPVector2& point, bool ignoreIfStartPointInside) {
    FP normalizedDist;
    return LineIntersectsCircle(p1, p2, position, radius, point, normalizedDist,
                                ignoreIfStartPointInside);
}

// 0x7a75410
bool LineIntersectsCircle(FPVector2 p1, FPVector2 p2, FPVector2 position, FP radius,
                          FPVector2& point, FP& normalizedDist, bool ignoreIfStartPointInside) {
    // Works with angles rather than solving the quadratic: the distance from the centre
    // to the line follows from the angle between the segment and the centre->start
    // direction, then the half chord from Pythagoras.
    FP length;
    const FPVector2 direction = FPVector2::Normalize(p2 - p1, length);
    const FP radiusSquared = radius * radius;

    auto hitAt = [&](FP along) {
        normalizedDist = along / length;
        point = {p1.X + along * direction.X, p1.Y + along * direction.Y};
        return true;
    };

    if (radiusSquared != 0 && length != 0) {
        FP startDistance;
        const FPVector2 toStart = FPVector2::Normalize(p1 - position, startDistance);
        const FP cos = FPVector2::Dot(toStart, direction);
        const FP cosSquared = cos * cos;
        const FP lineDistance =
            cosSquared < 1 ? FPMath::Sqrt(FP::One() - cosSquared) * startDistance : FP::Zero();
        if (lineDistance <= radius) {
            const FP halfChord = FPMath::Sqrt(radiusSquared - lineDistance * lineDistance);
            // Distance along the segment to the point nearest the centre.
            const FP closestAlong = (-cos) * startDistance;
            const FP entry = closestAlong - halfChord;
            if (entry >= 0 && entry <= length) {
                return hitAt(entry);
            }
            if (!ignoreIfStartPointInside) {
                const FP exit = closestAlong + halfChord;
                if (exit >= 0 && exit <= length) {
                    return hitAt(exit);
                }
            }
        }
    }
    point = FPVector2::Zero();
    normalizedDist = FP::Zero();
    return false;
}

// 0x7a755bc
FP ClosestDistanceToTriangle(const FPVector3& p, const FPVector3& a, const FPVector3& b,
                             const FPVector3& c, FPVector3& closestPoint,
                             FPVector3& barycentricCoordinates) {
    // Region classification as in Ericson, "Real-Time Collision Detection" 5.1.5, but
    // carried out on the vertices relative to p (so every dot product has the opposite
    // sign to the book's). Before classifying, the relative coordinates are rescaled by
    // powers of two so the triangle's normal is neither tiny nor overflowing: the
    // region tests are then sign tests on well-conditioned numbers. The final point is
    // blended from the ORIGINAL vertices, so the scaling only affects which region and
    // which weights are chosen.
    barycentricCoordinates = FPVector3::Zero();

    FPVector3 relA = a - p;
    FPVector3 relB = b - p;
    FPVector3 relC = c - p;
    FPVector3 ab;
    FPVector3 ac;
    FPVector3 normal;
    FP normalSquared;
    auto update = [&] {
        ab = relB - relA;
        ac = relC - relA;
        normal = FPVector3::Cross(ab, ac);
        normalSquared = normal.SqrMagnitude();
    };
    auto rescale = [&](auto&& scale) {
        relA = {scale(relA.X), scale(relA.Y), scale(relA.Z)};
        relB = {scale(relB.X), scale(relB.Y), scale(relB.Z)};
        relC = {scale(relC.X), scale(relC.Y), scale(relC.Z)};
        update();
    };
    update();

    // The magnitude tests use the absolute value as an unsigned number, so Abs(MinValue)
    // counts as huge.
    auto tiny = [](FP v) { return static_cast<std::uint64_t>(FPMath::Abs(v).RawValue()) < 8; };
    auto huge = [](FP v) { return (static_cast<std::uint64_t>(FPMath::Abs(v).RawValue()) >> 31) != 0; };

    int exponent = 0; // log2 of the current scale factor
    while (exponent <= 3 &&
           (tiny(normal.X) || tiny(normal.Y) || tiny(normal.Z) || tiny(normalSquared))) {
        rescale([](FP v) { return FP::FromRaw(detail::WrapMul(v.RawValue(), 2)); });
        ++exponent;
    }
    while (exponent >= -7 &&
           (huge(normal.X) || huge(normal.Y) || huge(normal.Z) || huge(normalSquared))) {
        rescale([](FP v) { return FP::FromRaw(v.RawValue() >> 2); });
        exponent -= 2;
    }

    // Dot products with the book's names; each is the negation of Ericson's value.
    const FP d5 = FPVector3::Dot(ab, relC);
    const FP d6 = FPVector3::Dot(ac, relC);

    auto finish = [&](const FPVector3& point) {
        closestPoint = point;
        return FPVector3::DistanceSquared(p, closestPoint);
    };

    if (d6 <= 0 && d6 <= d5) {
        barycentricCoordinates.Z = FP::One();
        return finish(c);
    }

    const FP d1 = FPVector3::Dot(relA, ab);
    const FP d2 = FPVector3::Dot(relA, ac);
    if (d1 >= 0 && d2 >= 0) {
        barycentricCoordinates.X = FP::One();
        return finish(a);
    }

    const FP vb = d2 * d5 - d1 * d6;
    if (d6 >= 0 && d2 <= 0 && vb <= 0) {
        // Edge AC.
        const FP denominator = d2 - d6;
        const FP w = denominator == 0 ? FP::Half() : d2 / denominator;
        const FP oneMinusW = denominator == 0 ? FP::Half() : FP::One() - w;
        barycentricCoordinates.X = oneMinusW;
        barycentricCoordinates.Z = w;
        return finish(Blend(a, oneMinusW, c, w));
    }

    const FP d3 = FPVector3::Dot(relB, ab);
    const FP d4 = FPVector3::Dot(relB, ac);
    if (d3 <= 0 && d3 <= d4) {
        barycentricCoordinates.Y = FP::One();
        return finish(b);
    }

    const FP va = d3 * d6 - d4 * d5;
    // Ericson's (d4 - d3) and (d5 - d6), written with the negated dots.
    const FP edgeBcB = d3 - d4;
    const FP edgeBcC = d6 - d5;
    if (edgeBcC >= 0 && edgeBcB >= 0 && va <= 0) {
        // Edge BC.
        const FP denominator = edgeBcB + edgeBcC;
        const FP w = denominator == 0 ? FP::Half() : edgeBcB / denominator;
        const FP oneMinusW = denominator == 0 ? FP::Half() : FP::One() - w;
        barycentricCoordinates.Y = oneMinusW;
        barycentricCoordinates.Z = w;
        return finish(Blend(c, w, b, oneMinusW));
    }

    const FP vc = d1 * d4 - d2 * d3;
    if (d3 >= 0 && d1 <= 0 && vc <= 0) {
        // Edge AB.
        const FP denominator = d1 - d3;
        const FP v = denominator == 0 ? FP::Half() : d1 / denominator;
        const FP oneMinusV = denominator == 0 ? FP::Half() : FP::One() - v;
        barycentricCoordinates.X = oneMinusV;
        barycentricCoordinates.Y = v;
        return finish(Blend(a, oneMinusV, b, v));
    }

    // Inside the face.
    const FP denominator = vb + va + vc;
    FP u = FP::FromRaw(0x5555); // one third each for a degenerate triangle
    FP v = u;
    FP w = u;
    if (denominator != 0) {
        v = vb / denominator;
        w = vc / denominator;
        u = FP::One() - (v + w);
    }
    barycentricCoordinates = {u, v, w};
    return finish(Blend(a, u, b, v, c, w));
}

} // namespace Photon::Deterministic::FPCollision
