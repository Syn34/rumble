// fp_quaternion.h - Photon.Deterministic.FPQuaternion, fixed-point rotation quaternion.
//
// Mirrors Unity's Quaternion API (left-handed, angles in degrees for Euler/AngleAxis/
// RotateTowards) on top of FP arithmetic. Products are shifted one by one before they
// are summed, exactly like the vector types.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_math.h"
#include "fp_vector3.h"

#include <cstdint>
#include <string>

namespace Photon::Deterministic {

class IDeterministicFrameSerializer;

struct FPQuaternion {
    static constexpr FP::Raw RAW_180 = FP::Raw{180} << FP::FractionBits;
    static constexpr FP::Raw RAW_360 = FP::Raw{360} << FP::FractionBits;
    static constexpr int SIZE = 32; // serialised size in bytes

    FP X;
    FP Y;
    FP Z;
    FP W;

    constexpr FPQuaternion() = default;
    constexpr FPQuaternion(FP x, FP y, FP z, FP w) : X(x), Y(y), Z(z), W(w) {}

    // 0x7a77bf4 get_Identity
    static constexpr FPQuaternion Identity() {
        return {FP::Zero(), FP::Zero(), FP::Zero(), FP::One()};
    }

    friend constexpr bool operator==(const FPQuaternion& a, const FPQuaternion& b) = default;

    // --- basic algebra ------------------------------------------------------------

    // 0x7a769d8 - Hamilton product: applying the result rotates by `right`, then `left`.
    static constexpr FPQuaternion Product(const FPQuaternion& left, const FPQuaternion& right) {
        return {
            left.X * right.W + left.Y * right.Z - left.Z * right.Y + left.W * right.X,
            left.Y * right.W + left.Z * right.X - left.X * right.Z + left.W * right.Y,
            left.Z * right.W + left.X * right.Y - left.Y * right.X + left.W * right.Z,
            left.W * right.W - (left.X * right.X + left.Y * right.Y + left.Z * right.Z),
        };
    }

    // 0x7a77d28
    friend constexpr FPQuaternion operator*(const FPQuaternion& left, const FPQuaternion& right) {
        return Product(left, right);
    }

    // 0x7a77d94 - rotates a point (Unity's Quaternion * Vector3 formula).
    friend constexpr FPVector3 operator*(const FPQuaternion& quat, const FPVector3& point) {
        // The doubled components are raw doublings ("FP * 2"), not FP multiplies.
        const FP x2 = detail::MulInt(quat.X, 2);
        const FP y2 = detail::MulInt(quat.Y, 2);
        const FP z2 = detail::MulInt(quat.Z, 2);
        const FP xx = quat.X * x2;
        const FP yy = quat.Y * y2;
        const FP zz = quat.Z * z2;
        const FP xy = quat.X * y2;
        const FP xz = quat.X * z2;
        const FP yz = quat.Y * z2;
        const FP wx = quat.W * x2;
        const FP wy = quat.W * y2;
        const FP wz = quat.W * z2;
        return {
            (FP::One() - (yy + zz)) * point.X + (xy - wz) * point.Y + (xz + wy) * point.Z,
            (xy + wz) * point.X + (FP::One() - (xx + zz)) * point.Y + (yz - wx) * point.Z,
            (xz - wy) * point.X + (yz + wx) * point.Y + (FP::One() - (xx + yy)) * point.Z,
        };
    }

    // 0x7a76a7c
    static constexpr FPQuaternion Conjugate(const FPQuaternion& value) {
        return {-value.X, -value.Y, -value.Z, value.W};
    }

    // 0x7a77f84 get_Conjugated
    constexpr FPQuaternion Conjugated() const { return Conjugate(*this); }

    // 0x7a76aa0 - exact comparison with (0, 0, 0, 1).
    static constexpr bool IsIdentity(const FPQuaternion& value) { return value == Identity(); }

    // 0x7a76ad0
    static constexpr bool IsZero(const FPQuaternion& value) { return value == FPQuaternion{}; }

    // 0x7a77bc8 get_MagnitudeSqrRaw - squared length (per-term shifted), as a raw value.
    constexpr FP::Raw MagnitudeSqrRaw() const { return (X * X + Y * Y + Z * Z + W * W).RawValue(); }

    // 0x7a76cc4 - unit-length copy; a zero quaternion becomes the identity.
    static FPQuaternion Normalize(const FPQuaternion& value);
    // 0x7a77ec4 get_Normalized
    FPQuaternion Normalized() const;

    // 0x7a77b38 - conjugate divided by the length (so only a true inverse for unit
    // quaternions, which is all the game uses); a zero quaternion is returned unchanged.
    static FPQuaternion Inverse(const FPQuaternion& value);
    // 0x7a77f24 get_Inverted
    FPQuaternion Inverted() const;

    // --- interpolation ----------------------------------------------------------

    // 0x7a76b00 - t clamped to [0, 1].
    static FPQuaternion Lerp(const FPQuaternion& a, const FPQuaternion& b, FP t);
    // 0x7a76b7c - normalised linear blend along the shorter arc.
    static FPQuaternion LerpUnclamped(const FPQuaternion& a, const FPQuaternion& b, FP t);
    // 0x7a77284 - t clamped to [0, 1].
    static FPQuaternion Slerp(const FPQuaternion& from, const FPQuaternion& to, FP t);
    // 0x7a77300 - spherical blend along the shorter arc; nearly parallel inputs fall
    // back to LerpUnclamped.
    static FPQuaternion SlerpUnclamped(const FPQuaternion& from, const FPQuaternion& to, FP t);
    // 0x7a77618 - rotates `from` towards `to` by at most maxDegreesDelta degrees.
    static FPQuaternion RotateTowards(const FPQuaternion& from, const FPQuaternion& to,
                                      FP maxDegreesDelta);

    // --- construction from angles/directions -------------------------------------

    // 0x7a76d5c - angles in radians: yaw about Y, pitch about X, roll about Z.
    static FPQuaternion CreateFromYawPitchRoll(FP yaw, FP pitch, FP roll);
    // 0x7a778d4 - Euler angles in degrees (applied Z, then X, then Y).
    static FPQuaternion Euler(FP x, FP y, FP z);
    // 0x7a77944
    static FPQuaternion Euler(const FPVector3& eulerAngles);
    // 0x7a779c8 - rotation of `angle` degrees about `axis` (normalised internally).
    static FPQuaternion AngleAxis(FP angle, const FPVector3& axis);
    // 0x7a76f98 - rotation whose forward (+Z) is `forward` and whose up is as close to
    // `up` as possible. With orthoNormalize, `up` is first made perpendicular to forward
    // (Gram-Schmidt) and used as is instead of being rebuilt from the cross products.
    static FPQuaternion LookRotation(const FPVector3& forward, const FPVector3& up,
                                     bool orthoNormalize);
    // 0x7a76f18 - LookRotation(forward, Up, false). Note: the original ignores its
    // orthoNormalize argument and always passes false.
    static FPQuaternion LookRotation(const FPVector3& forward, bool orthoNormalize);

    // --- conversion to angles --------------------------------------------------

    // 0x7a77c04 - Euler angles in degrees for the Z-X-Y rotation order (Unity's).
    static FPVector3 ToEulerZXY(const FPQuaternion& value);
    // 0x7a77fa0 get_AsEuler
    FPVector3 AsEuler() const;

    // --- object methods ----------------------------------------------------------

    // 0x7a7821c
    constexpr std::int32_t GetHashCode() const {
        std::int32_t hash = detail::HashSeed;
        hash = detail::HashCombine(hash, detail::HashRaw(X));
        hash = detail::HashCombine(hash, detail::HashRaw(Y));
        hash = detail::HashCombine(hash, detail::HashRaw(Z));
        return detail::HashCombine(hash, detail::HashRaw(W));
    }

    // 0x7a78004 - "(x, y, z, w)" with one decimal per component.
    std::string ToString() const;

    // 0x7a77e7c
    static void Serialize(void* ptr, IDeterministicFrameSerializer* serializer);
};

static_assert(sizeof(FPQuaternion) == FPQuaternion::SIZE);

} // namespace Photon::Deterministic
