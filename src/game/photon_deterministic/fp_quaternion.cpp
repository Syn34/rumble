#include "fp_quaternion.h"

#include "dotnet_number_format.h"
#include "fp_math_pending.h"

#include <cassert>
#include <cstdint>

namespace Photon::Deterministic {

namespace {

constexpr FP Dot(const FPQuaternion& a, const FPQuaternion& b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
}

// Halves an angle. Written as "angle * FP._0_50" in spirit, but the original build
// compiles it to an arithmetic shift of the raw value, which this reproduces exactly.
constexpr FP HalfAngle(FP angle) {
    return FP::FromRaw(angle.RawValue() >> 1);
}

constexpr FPQuaternion Negated(const FPQuaternion& q) {
    return {-q.X, -q.Y, -q.Z, -q.W};
}

constexpr FPQuaternion Scaled(const FPQuaternion& q, FP s) {
    return {q.X * s, q.Y * s, q.Z * s, q.W * s};
}

constexpr FPQuaternion Sum(const FPQuaternion& a, const FPQuaternion& b) {
    return {a.X + b.X, a.Y + b.Y, a.Z + b.Z, a.W + b.W};
}

// Spherical blend of two quaternions `angle` radians apart (already on the same
// hemisphere): (from * sin((1-t)a) + to * sin(t a)) / sin(a). Shared by SlerpUnclamped
// and RotateTowards; neither the inputs nor the result are normalised here.
FPQuaternion SphericalBlend(const FPQuaternion& from, const FPQuaternion& to, FP angle, FP t) {
    const FP fromWeight = FPMath::Sin(angle * (FP::One() - t));
    const FP toWeight = FPMath::Sin(angle * t);
    const FP inverseSin = FP::One() / FPMath::Sin(angle);
    return Scaled(Sum(Scaled(from, fromWeight), Scaled(to, toWeight)), inverseSin);
}

} // namespace

// 0x7a76cc4
FPQuaternion FPQuaternion::Normalize(const FPQuaternion& value) {
    const FP lengthSqr = Dot(value, value);
    if (lengthSqr == FP::Zero()) {
        return Identity();
    }
    return Scaled(value, FP::One() / FPMath::Sqrt(lengthSqr));
}

// 0x7a77ec4
FPQuaternion FPQuaternion::Normalized() const {
    return Normalize(*this);
}

// 0x7a77b38
FPQuaternion FPQuaternion::Inverse(const FPQuaternion& value) {
    const FP lengthSqr = Dot(value, value);
    if (lengthSqr == FP::Zero()) {
        return value;
    }
    return Scaled(Conjugate(value), FP::One() / FPMath::Sqrt(lengthSqr));
}

// 0x7a77f24
FPQuaternion FPQuaternion::Inverted() const {
    return Inverse(*this);
}

// 0x7a76b00
FPQuaternion FPQuaternion::Lerp(const FPQuaternion& a, const FPQuaternion& b, FP t) {
    return LerpUnclamped(a, b, FPMath::Clamp01(t));
}

// 0x7a76b7c
FPQuaternion FPQuaternion::LerpUnclamped(const FPQuaternion& a, const FPQuaternion& b, FP t) {
    // q and -q are the same rotation; flip b so the blend takes the shorter arc.
    const FPQuaternion target = Dot(a, b) < FP::Zero() ? Negated(b) : b;
    return Normalize(Sum(Scaled(a, FP::One() - t), Scaled(target, t)));
}

// 0x7a77284
FPQuaternion FPQuaternion::Slerp(const FPQuaternion& from, const FPQuaternion& to, FP t) {
    return SlerpUnclamped(from, to, FPMath::Clamp01(t));
}

// 0x7a77300
FPQuaternion FPQuaternion::SlerpUnclamped(const FPQuaternion& from, const FPQuaternion& to, FP t) {
    FPQuaternion target = to;
    FP cosine = Dot(from, to);
    if (cosine < FP::Zero()) {
        target = Negated(to);
        cosine = -cosine;
    }

    // Close to parallel (cos >= ~0.99, raw 64881) sin(angle) gets too small to divide
    // by, so blend linearly instead. The comparison is unsigned in the original.
    constexpr std::uint64_t NearlyParallelRaw = 64881;
    if (static_cast<std::uint64_t>(cosine.RawValue()) >= NearlyParallelRaw) {
        return LerpUnclamped(from, target, t);
    }
    return Normalize(SphericalBlend(from, target, FPMath::Acos(cosine), t));
}

// 0x7a77618
FPQuaternion FPQuaternion::RotateTowards(const FPQuaternion& from, const FPQuaternion& to,
                                         FP maxDegreesDelta) {
    FPQuaternion target = to;
    FP cosine = Dot(from, to);
    if (cosine < FP::Zero()) {
        target = Negated(to);
        cosine = -cosine;
    }

    // The half-angle from Acos is doubled ("FP * 2") to get the rotation angle.
    const FP halfAngle = FPMath::Acos(cosine);
    const FP angle = detail::MulInt(halfAngle, 2);
    const FP maxRadians = maxDegreesDelta * FP::Deg2Rad();
    if (angle <= maxRadians) {
        return target;
    }
    // Unlike SlerpUnclamped the result is not re-normalised.
    return SphericalBlend(from, target, halfAngle, maxRadians / angle);
}

// 0x7a76d5c
FPQuaternion FPQuaternion::CreateFromYawPitchRoll(FP yaw, FP pitch, FP roll) {
    FP sinRoll;
    FP cosRoll;
    FPMath::SinCos(HalfAngle(roll), sinRoll, cosRoll);
    FP sinPitch;
    FP cosPitch;
    FPMath::SinCos(HalfAngle(pitch), sinPitch, cosPitch);
    FP sinYaw;
    FP cosYaw;
    FPMath::SinCos(HalfAngle(yaw), sinYaw, cosYaw);

    // Same grouping as System.Numerics: the yaw/pitch products are formed first.
    const FP cyCp = cosYaw * cosPitch;
    const FP cySp = cosYaw * sinPitch;
    const FP syCp = sinYaw * cosPitch;
    const FP sySp = sinYaw * sinPitch;
    return {
        cySp * cosRoll + syCp * sinRoll,
        syCp * cosRoll - cySp * sinRoll,
        cyCp * sinRoll - sySp * cosRoll,
        cyCp * cosRoll + sySp * sinRoll,
    };
}

// 0x7a778d4
FPQuaternion FPQuaternion::Euler(FP x, FP y, FP z) {
    return CreateFromYawPitchRoll(y * FP::Deg2Rad(), x * FP::Deg2Rad(), z * FP::Deg2Rad());
}

// 0x7a77944
FPQuaternion FPQuaternion::Euler(const FPVector3& eulerAngles) {
    return Euler(eulerAngles.X, eulerAngles.Y, eulerAngles.Z);
}

// 0x7a779c8
FPQuaternion FPQuaternion::AngleAxis(FP angle, const FPVector3& axis) {
    const FPVector3 unitAxis = FPVector3::Normalize(axis);
    FP sinHalf;
    FP cosHalf;
    FPMath::SinCos(HalfAngle(angle * FP::Deg2Rad()), sinHalf, cosHalf);
    return {sinHalf * unitAxis.X, sinHalf * unitAxis.Y, sinHalf * unitAxis.Z, cosHalf};
}

// 0x7a76f98
FPQuaternion FPQuaternion::LookRotation(const FPVector3& forward, const FPVector3& up,
                                        bool orthoNormalize) {
    const FPVector3 f = FPVector3::Normalize(forward);
    FPVector3 u = up;
    if (orthoNormalize) {
        u = FPVector3::Normalize(u - f * FPVector3::Dot(f, u));
    }
    const FPVector3 r = FPVector3::Normalize(FPVector3::Cross(u, f));
    if (!orthoNormalize) {
        u = FPVector3::Cross(f, r);
    }

    // Rotation matrix (columns r, u, f) to quaternion, branching on the largest
    // diagonal term for precision. Each branch: s = sqrt(1 + combination of the
    // diagonal), the dominant component is s/2, the others are differences of
    // off-diagonal terms times 0.5/s.
    const FP trace = r.X + u.Y + f.Z;
    if (trace > FP::Zero()) {
        const FP s = FPMath::Sqrt(trace + FP::One());
        const FP k = FP::Half() / s;
        return {(u.Z - f.Y) * k, (f.X - r.Z) * k, (r.Y - u.X) * k, s * FP::Half()};
    }
    if (r.X >= u.Y && r.X >= f.Z) {
        const FP s = FPMath::Sqrt(r.X - u.Y - f.Z + FP::One());
        const FP k = FP::Half() / s;
        return {s * FP::Half(), (u.X + r.Y) * k, (f.X + r.Z) * k, (u.Z - f.Y) * k};
    }
    if (u.Y > f.Z) {
        const FP s = FPMath::Sqrt(u.Y - r.X - f.Z + FP::One());
        const FP k = FP::Half() / s;
        return {(u.X + r.Y) * k, s * FP::Half(), (u.Z + f.Y) * k, (f.X - r.Z) * k};
    }
    const FP s = FPMath::Sqrt(f.Z - (u.Y + r.X) + FP::One());
    const FP k = FP::Half() / s;
    return {(f.X + r.Z) * k, (u.Z + f.Y) * k, s * FP::Half(), (r.Y - u.X) * k};
}

// 0x7a76f18
FPQuaternion FPQuaternion::LookRotation(const FPVector3& forward, bool orthoNormalize) {
    (void)orthoNormalize; // ignored by the original, see the header
    return LookRotation(forward, FPVector3::Up(), false);
}

// 0x7a77c04
FPVector3 FPQuaternion::ToEulerZXY(const FPQuaternion& value) {
    const FP x = value.X;
    const FP y = value.Y;
    const FP z = value.Z;
    const FP w = value.W;
    // The "2 * a * b" terms double each product before the shift (see TwiceProduct).
    using detail::TwiceProduct;

    const FP sinPitch =
        FPMath::Clamp(TwiceProduct(w, x) - TwiceProduct(z, y), -FP::One(), FP::One());
    const FP pitch = FPMath::Asin(sinPitch);

    FP yaw;
    FP roll;
    if (FPMath::Abs(sinPitch) < FP::One()) {
        yaw = FPMath::Atan2(TwiceProduct(w, y) + TwiceProduct(z, x),
                            FP::One() - TwiceProduct(x, x) - TwiceProduct(y, y));
        roll = FPMath::Atan2(TwiceProduct(w, z) + TwiceProduct(y, x),
                             FP::One() - TwiceProduct(x, x) - TwiceProduct(z, z));
    } else {
        // Gimbal lock (pitch at +-90 degrees): fold all of the remaining rotation into
        // yaw and report zero roll.
        yaw = FPMath::Atan2(TwiceProduct(w, y) - TwiceProduct(z, x),
                            FP::One() - (TwiceProduct(y, y) + TwiceProduct(z, z)));
        roll = FP::Zero();
    }
    return {pitch * FP::Rad2Deg(), yaw * FP::Rad2Deg(), roll * FP::Rad2Deg()};
}

// 0x7a77fa0
FPVector3 FPQuaternion::AsEuler() const {
    return ToEulerZXY(*this);
}

// 0x7a78004
std::string FPQuaternion::ToString() const {
    return "(" + detail::FormatSingleFixed(X.AsFloat(), 1) + ", " +
           detail::FormatSingleFixed(Y.AsFloat(), 1) + ", " +
           detail::FormatSingleFixed(Z.AsFloat(), 1) + ", " +
           detail::FormatSingleFixed(W.AsFloat(), 1) + ")";
}

// TODO(0x7a77e7c, Photon.Deterministic.FPQuaternion.Serialize): serialises X, Y, Z, W
// through FP.Serialize (0x7a726d0) with the frame serializer; needs
// IDeterministicFrameSerializer and FP.Serialize, which belong to the serialisation work.
void FPQuaternion::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    (void)ptr;
    (void)serializer;
    assert(false && "FPQuaternion::Serialize is not implemented yet");
}

} // namespace Photon::Deterministic
