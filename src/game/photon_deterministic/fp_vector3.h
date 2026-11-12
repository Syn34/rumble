// fp_vector3.h - Photon.Deterministic.FPVector3, the simulation's 3D fixed-point vector.
//
// Same arithmetic rules as FPVector2 (see fp_vector2.h): products are shifted one by one
// before they are summed, additions wrap. Angles returned by Angle/SignedAngle are in
// degrees, matching Unity's Vector3 API that this type mirrors.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_math.h"
#include "fp_vector2.h"

#include <cstdint>
#include <string>

namespace Photon::Deterministic {

class IDeterministicFrameSerializer;

struct FPVector3 {
    static constexpr int SIZE = 24; // serialised size in bytes

    FP X;
    FP Y;
    FP Z;

    constexpr FPVector3() = default;

    // 0x7a72850
    constexpr FPVector3(FP x, FP y, FP z) : X(x), Y(y), Z(z) {}

    // 0x7a73730 - from whole numbers.
    constexpr FPVector3(std::int32_t x, std::int32_t y, std::int32_t z)
        : X(FP::FromInt(x)), Y(FP::FromInt(y)), Z(FP::FromInt(z)) {}

    // --- constants (Unity's left-handed convention: +Z is forward) -------------------

    // 0x7a732e8
    static constexpr FPVector3 Zero() { return {}; }
    // 0x7a735c4
    static constexpr FPVector3 Left() { return {-FP::One(), FP::Zero(), FP::Zero()}; }
    // 0x7a735d4
    static constexpr FPVector3 Right() { return {FP::One(), FP::Zero(), FP::Zero()}; }
    // 0x7a735e4
    static constexpr FPVector3 Up() { return {FP::Zero(), FP::One(), FP::Zero()}; }
    // 0x7a735f8
    static constexpr FPVector3 Down() { return {FP::Zero(), -FP::One(), FP::Zero()}; }
    // 0x7a7360c
    static constexpr FPVector3 Back() { return {FP::Zero(), FP::Zero(), -FP::One()}; }
    // 0x7a7361c
    static constexpr FPVector3 Forward() { return {FP::Zero(), FP::Zero(), FP::One()}; }
    // 0x7a7362c
    static constexpr FPVector3 One() { return {FP::One(), FP::One(), FP::One()}; }

    // --- operators -------------------------------------------------------------------

    // 0x7a733d4 op_Equality / 0x7a73410 op_Inequality
    friend constexpr bool operator==(const FPVector3& a, const FPVector3& b) = default;

    // 0x7a7344c
    friend constexpr FPVector3 operator-(const FPVector3& v) { return {-v.X, -v.Y, -v.Z}; }
    // 0x7a73588
    friend constexpr FPVector3 operator+(const FPVector3& a, const FPVector3& b) {
        return {a.X + b.X, a.Y + b.Y, a.Z + b.Z};
    }
    // 0x7a7354c
    friend constexpr FPVector3 operator-(const FPVector3& a, const FPVector3& b) {
        return {a.X - b.X, a.Y - b.Y, a.Z - b.Z};
    }
    // 0x7a73474
    friend constexpr FPVector3 operator*(const FPVector3& v, FP s) {
        return {v.X * s, v.Y * s, v.Z * s};
    }
    // 0x7a734ac
    friend constexpr FPVector3 operator*(FP s, const FPVector3& v) {
        return {s * v.X, s * v.Y, s * v.Z};
    }
    // 0x7a734e4
    friend constexpr FPVector3 operator/(const FPVector3& v, FP s) {
        return {v.X / s, v.Y / s, v.Z / s};
    }
    // 0x7a7351c - divides the raw values by a whole number.
    friend constexpr FPVector3 operator/(const FPVector3& v, std::int32_t s) {
        return {detail::DivInt(v.X, s), detail::DivInt(v.Y, s), detail::DivInt(v.Z, s)};
    }

    // --- swizzles ------------------------------------------------------------------

    // 0x7a72b4c get_XY
    constexpr FPVector2 XY() const { return {X, Y}; }
    // 0x7a72b58 get_XZ - the ground-plane projection.
    constexpr FPVector2 XZ() const { return {X, Z}; }

    // --- length --------------------------------------------------------------------

    // 0x7a7367c get_SqrMagnitude
    constexpr FP SqrMagnitude() const { return Dot(*this, *this); }
    // 0x7a736a0 get_Magnitude
    FP Magnitude() const;
    // 0x7a736c4 get_Normalized
    FPVector3 Normalized() const;

    // 0x7a72a14
    static FPVector3 Normalize(const FPVector3& value);
    // 0x7a72aa4 - also returns the length that was divided out.
    static FPVector3 Normalize(const FPVector3& value, FP& magnitude);

    // 0x7a72c68
    static FPVector3 ClampMagnitude(const FPVector3& vector, FP maxLength);

    // --- products and distances -----------------------------------------------------

    // 0x7a72ec8 - each product is shifted before summing; this ordering affects the low
    // bits and must be kept for results to match the original simulation.
    static constexpr FP Dot(const FPVector3& a, const FPVector3& b) {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
    }

    // 0x7a72e7c
    static constexpr FPVector3 Cross(const FPVector3& a, const FPVector3& b) {
        return {a.Y * b.Z - a.Z * b.Y, a.Z * b.X - a.X * b.Z, a.X * b.Y - a.Y * b.X};
    }

    // 0x7a72e2c
    static constexpr FP DistanceSquared(const FPVector3& a, const FPVector3& b) {
        const FPVector3 d = a - b;
        return Dot(d, d);
    }

    // 0x7a72dd0
    static FP Distance(const FPVector3& a, const FPVector3& b);

    // 0x7a72c20 - component-wise product.
    static constexpr FPVector3 Scale(const FPVector3& a, const FPVector3& b) {
        return {a.X * b.X, a.Y * b.Y, a.Z * b.Z};
    }

    // 0x7a72d40
    static constexpr FPVector3 Min(const FPVector3& value1, const FPVector3& value2) {
        return {FPMath::Min(value1.X, value2.X), FPMath::Min(value1.Y, value2.Y),
                FPMath::Min(value1.Z, value2.Z)};
    }

    // 0x7a72d88
    static constexpr FPVector3 Max(const FPVector3& value1, const FPVector3& value2) {
        return {FPMath::Max(value1.X, value2.X), FPMath::Max(value1.Y, value2.Y),
                FPMath::Max(value1.Z, value2.Z)};
    }

    // --- interpolation, projection ------------------------------------------------

    // 0x7a72bcc
    static constexpr FPVector3 LerpUnclamped(const FPVector3& start, const FPVector3& end, FP t) {
        return {start.X + (end.X - start.X) * t, start.Y + (end.Y - start.Y) * t,
                start.Z + (end.Z - start.Z) * t};
    }

    // 0x7a72b68 - t is clamped to [0, 1].
    static constexpr FPVector3 Lerp(const FPVector3& start, const FPVector3& end, FP t) {
        return LerpUnclamped(start, end, FPMath::Clamp01(t));
    }

    // 0x7a73198 - steps from `from` towards `to` by at most maxDelta.
    static FPVector3 MoveTowards(const FPVector3& from, const FPVector3& to, FP maxDelta);

    // 0x7a73264 - projection of `vector` onto the line along `normal`; zero when normal
    // is shorter than sqrt(FP.Epsilon).
    static constexpr FPVector3 Project(const FPVector3& vector, const FPVector3& normal) {
        const FP normalSqr = Dot(normal, normal);
        if (normalSqr < detail::Epsilon) {
            return Zero();
        }
        const FP dot = Dot(vector, normal);
        return {dot * normal.X / normalSqr, dot * normal.Y / normalSqr, dot * normal.Z / normalSqr};
    }

    // 0x7a732f4
    static constexpr FPVector3 ProjectOnPlane(const FPVector3& vector,
                                              const FPVector3& planeNormal) {
        return vector - Project(vector, planeNormal);
    }

    // 0x7a73384 - mirrors `vector` about the plane with unit normal `normal`.
    static constexpr FPVector3 Reflect(const FPVector3& vector, const FPVector3& normal) {
        // -2 * Dot(...) is "FP times int": the raw dot is doubled without a shift.
        const FP factor = detail::MulInt(Dot(vector, normal), -2);
        return {vector.X + factor * normal.X, vector.Y + factor * normal.Y,
                vector.Z + factor * normal.Z};
    }

    // --- angles (degrees) ---------------------------------------------------------

    // 0x7a72fe4 - unsigned angle in degrees; zero if either vector is zero.
    static FP Angle(const FPVector3& a, const FPVector3& b);
    // 0x7a72efc - angle in degrees, negative when Cross(a, b) points against `axis`.
    static FP SignedAngle(const FPVector3& a, const FPVector3& b, const FPVector3& axis);

    // --- object methods ------------------------------------------------------------

    // 0x7a738e8 Equals(FPVector3). The boxed Equals(object) overload (0x7a73844) has no
    // C++ counterpart; it type-checks and then does this same comparison.
    constexpr bool Equals(const FPVector3& other) const { return *this == other; }

    // 0x7a73924
    constexpr std::int32_t GetHashCode() const {
        std::int32_t hash = detail::HashSeed;
        hash = detail::HashCombine(hash, detail::HashRaw(X));
        hash = detail::HashCombine(hash, detail::HashRaw(Y));
        return detail::HashCombine(hash, detail::HashRaw(Z));
    }

    // 0x7a73748 - "(x, y, z)" with each component printed as a float.
    std::string ToString() const;

    // 0x7a73640
    static void Serialize(void* ptr, IDeterministicFrameSerializer* serializer);
};

static_assert(sizeof(FPVector3) == FPVector3::SIZE);

} // namespace Photon::Deterministic
