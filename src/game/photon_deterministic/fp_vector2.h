// fp_vector2.h - Photon.Deterministic.FPVector2, the simulation's 2D fixed-point vector.
//
// All arithmetic follows FP's rules: every product is shifted back to 16 fractional bits
// on its own before terms are summed (so a dot product is (ax*bx >> 16) + (ay*by >> 16)),
// and additions wrap. Several methods deliberately differ from the textbook formula in
// the low bits (noted where they do); those details are kept so results stay
// bit-identical with the original simulation.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_math.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace Photon::Deterministic {

struct FPVector3;
class IDeterministicFrameSerializer;

struct FPVector2 {
    static constexpr int SIZE = 16; // serialised size in bytes

    FP X;
    FP Y;

    constexpr FPVector2() = default;

    // 0x7a71c80
    constexpr FPVector2(FP x, FP y) : X(x), Y(y) {}

    // 0x7a72830 - from whole numbers.
    constexpr FPVector2(std::int32_t x, std::int32_t y) : X(FP::FromInt(x)), Y(FP::FromInt(y)) {}

    // --- constants -------------------------------------------------------------------

    // 0x7a72664
    static constexpr FPVector2 Zero() { return {}; }
    // 0x7a72670
    static constexpr FPVector2 Right() { return {FP::One(), FP::Zero()}; }
    // 0x7a7267c
    static constexpr FPVector2 Left() { return {-FP::One(), FP::Zero()}; }
    // 0x7a72688
    static constexpr FPVector2 Up() { return {FP::Zero(), FP::One()}; }
    // 0x7a72694
    static constexpr FPVector2 Down() { return {FP::Zero(), -FP::One()}; }

    // --- operators -------------------------------------------------------------------

    // 0x7a725e8 op_Equality / 0x7a725f8 op_Inequality
    friend constexpr bool operator==(FPVector2 a, FPVector2 b) = default;

    // 0x7a72608
    friend constexpr FPVector2 operator-(FPVector2 v) { return {-v.X, -v.Y}; }
    // 0x7a72614
    friend constexpr FPVector2 operator+(FPVector2 a, FPVector2 b) {
        return {a.X + b.X, a.Y + b.Y};
    }
    // 0x7a72620
    friend constexpr FPVector2 operator-(FPVector2 a, FPVector2 b) {
        return {a.X - b.X, a.Y - b.Y};
    }
    // 0x7a7262c
    friend constexpr FPVector2 operator*(FPVector2 v, FP s) { return {v.X * s, v.Y * s}; }
    // 0x7a72640
    friend constexpr FPVector2 operator*(FP s, FPVector2 v) { return {s * v.X, s * v.Y}; }
    // 0x7a72654 - scales the raw values by a whole number (no fixed-point shift).
    friend constexpr FPVector2 operator*(FPVector2 v, std::int32_t s) {
        return {detail::MulInt(v.X, s), detail::MulInt(v.Y, s)};
    }

    // --- swizzles (defined in fp_vector2.cpp, they need FPVector3) --------------------

    // 0x7a7186c get_XYY
    FPVector3 XYY() const;
    // 0x7a72840 get_XOY - lifts a ground-plane position into 3D (Y becomes Z).
    FPVector3 XOY() const;
    // 0x7a7285c get_XYO
    FPVector3 XYO() const;

    // --- length --------------------------------------------------------------------

    // 0x7a7280c get_SqrMagnitude
    constexpr FP SqrMagnitude() const { return Dot(*this, *this); }
    // 0x7a727f4 get_Magnitude
    FP Magnitude() const;
    // 0x7a72824 get_Normalized
    FPVector2 Normalized() const;

    // 0x7a717a8
    static FPVector2 Normalize(FPVector2 value);
    // 0x7a71800 - also returns the length that was divided out.
    static FPVector2 Normalize(FPVector2 value, FP& magnitude);

    // 0x7a71a74
    static FPVector2 ClampMagnitude(FPVector2 vector, FP maxLength);

    // --- products and distances -----------------------------------------------------

    // 0x7a71a60
    static constexpr FP Dot(FPVector2 a, FPVector2 b) { return a.X * b.X + a.Y * b.Y; }

    // 0x7a71a44
    static constexpr FP DistanceSquared(FPVector2 a, FPVector2 b) {
        const FPVector2 d = a - b;
        return Dot(d, d);
    }

    // 0x7a7187c
    static FP Distance(FPVector2 a, FPVector2 b);

    // 0x7a71bfc - z component of the 3D cross product (a.X*b.Y - a.Y*b.X).
    static constexpr FP Cross(FPVector2 a, FPVector2 b) { return a.X * b.Y - a.Y * b.X; }

    // 0x7a71c10 - identical to Cross in the original build.
    static constexpr FP CrossRaw(FPVector2 a, FPVector2 b) { return Cross(a, b); }

    // 0x7a71d24 - determinant of the 2x2 matrix with columns v1, v2 (same as Cross).
    static constexpr FP Determinant(FPVector2 v1, FPVector2 v2) {
        return v1.X * v2.Y - v1.Y * v2.X;
    }

    // --- rotation, reflection, interpolation ---------------------------------------

    // 0x7a71af0 - rotates counter-clockwise by `radians`.
    static FPVector2 Rotate(FPVector2 vector, FP radians);

    // 0x7a71bd8 - rotates counter-clockwise by the angle whose sine/cosine are given.
    static constexpr FPVector2 Rotate(FPVector2 vector, FP sin, FP cos) {
        return {vector.X * cos - vector.Y * sin, vector.Y * cos + vector.X * sin};
    }

    // 0x7a71c24 - mirrors `vector` about the line with unit normal `normal`.
    static constexpr FPVector2 Reflect(FPVector2 vector, FPVector2 normal) {
        // The original doubles each product of the dot before dropping the fractional
        // bits (shift by 15), rather than doubling the finished dot product.
        const FP twiceDot =
            detail::TwiceProduct(normal.X, vector.X) + detail::TwiceProduct(normal.Y, vector.Y);
        return {vector.X - twiceDot * normal.X, vector.Y - twiceDot * normal.Y};
    }

    // 0x7a71c48 - component-wise clamp.
    static constexpr FPVector2 Clamp(FPVector2 value, FPVector2 min, FPVector2 max) {
        return {FPMath::Clamp(value.X, min.X, max.X), FPMath::Clamp(value.Y, min.Y, max.Y)};
    }

    // 0x7a71c88 - t is clamped to [0, 1].
    static constexpr FPVector2 Lerp(FPVector2 start, FPVector2 end, FP t) {
        t = FPMath::Clamp01(t);
        return {start.X + t * (end.X - start.X), start.Y + t * (end.Y - start.Y)};
    }

    // 0x7a71cb4
    static constexpr FPVector2 Max(FPVector2 value1, FPVector2 value2) {
        return {FPMath::Max(value1.X, value2.X), FPMath::Max(value1.Y, value2.Y)};
    }

    // 0x7a71cd4
    static constexpr FPVector2 Min(FPVector2 value1, FPVector2 value2) {
        return {FPMath::Min(value1.X, value2.X), FPMath::Min(value1.Y, value2.Y)};
    }

    // 0x7a71cf4 - the vector rotated 90 degrees counter-clockwise.
    static constexpr FPVector2 CalculateLeft(FPVector2 vector) { return {-vector.Y, vector.X}; }

    // 0x7a71d04 - true if this vector points to the left of (counter-clockwise from)
    // `vector`. Computed as Dot(CalculateLeft(vector), this), which negates vector.Y
    // before the multiply.
    constexpr bool IsLeftOf(FPVector2 vector) const {
        return Dot(CalculateLeft(vector), *this) > FP::Zero();
    }

    // --- angles (radians) -----------------------------------------------------------

    // 0x7a71d38 - unsigned angle between two vectors of any length.
    static FP Radians(FPVector2 a, FPVector2 b);
    // 0x7a71e28 - as Radians, for vectors already of unit length.
    static FP RadiansSkipNormalize(FPVector2 a, FPVector2 b);
    // 0x7a71e54 - signed angle, positive when b is counter-clockwise from a.
    static FP RadiansSigned(FPVector2 a, FPVector2 b);
    // 0x7a71eb4 - as RadiansSigned, for unit vectors.
    static FP RadiansSignedSkipNormalize(FPVector2 a, FPVector2 b);

    // --- polygons ------------------------------------------------------------------

    // 0x7a71f28 - winding of a polygon, judged from its first three vertices only.
    // Throws std::out_of_range for fewer than three vertices (the original indexes
    // past the end and throws IndexOutOfRangeException).
    static bool IsClockWise(std::span<const FPVector2> vertices);
    // 0x7a71f84 - reverses the vertices in place if they wind clockwise.
    static void MakeCounterClockWise(std::span<FPVector2> vertices);
    // 0x7a71fac - reverses the vertex order in place.
    static void FlipWindingOrder(std::span<FPVector2> vertices);
    // 0x7a71fb4 - unit outward normal of each edge i -> i+1 (wrapping).
    static std::vector<FPVector2> CalculatePolygonNormals(std::span<const FPVector2> vertices);
    // 0x7a720b0 - copy of the vertices translated so the centroid is at the origin.
    static std::vector<FPVector2> RecenterPolygon(std::span<const FPVector2> vertices);
    // 0x7a722c4 - signed area (positive for counter-clockwise winding).
    static FP CalculatePolygonArea(std::span<const FPVector2> vertices);
    // 0x7a721a0 - area centroid. Throws std::invalid_argument("Not a valid polygon")
    // for zero area, as the original does.
    static FPVector2 CalculatePolygonCentroid(std::span<const FPVector2> vertices);
    // 0x7a72348 - moment-of-inertia factor about the origin (per unit mass).
    static FP CalculatePolygonInertiaFactor(std::span<const FPVector2> vertices);
    // 0x7a72418 - support point of a convex polygon in direction `localDir`, found by
    // hill-climbing from vertex 0. Expects verticesCount >= 2.
    static FPVector2 CalculatePolygonLocalSupport(const FPVector2* vertices, int verticesCount,
                                                  const FPVector2& localDir);
    // 0x7a72558 - distance from the origin to the farthest vertex.
    static FP CalculatePolygonRadius(std::span<const FPVector2> vertices);

    // --- object methods ------------------------------------------------------------

    // 0x7a728ec Equals(FPVector2). The boxed Equals(object) overload (0x7a72870) has no
    // C++ counterpart; it type-checks and then does this same comparison.
    constexpr bool Equals(FPVector2 other) const { return *this == other; }

    // 0x7a72900
    constexpr std::int32_t GetHashCode() const {
        return detail::HashCombine(detail::HashCombine(detail::HashSeed, detail::HashRaw(X)),
                                   detail::HashRaw(Y));
    }

    // 0x7a72940 - "(x, y)" with each component printed as a float.
    std::string ToString() const;

    // 0x7a726a0
    static void Serialize(void* ptr, IDeterministicFrameSerializer* serializer);
};

static_assert(sizeof(FPVector2) == FPVector2::SIZE);

} // namespace Photon::Deterministic
