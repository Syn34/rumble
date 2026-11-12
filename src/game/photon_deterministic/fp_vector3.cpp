#include "fp_vector3.h"
#include "deterministic_frame_serializer.h"

#include "dotnet_number_format.h"
#include "fp_math.h"

#include <cassert>

namespace Photon::Deterministic {

namespace {

// Same scheme as FPVector2's Normalize: squared length from the unshifted products,
// sqrt as a mantissa/exponent pair from FPMath, components scaled by 2^44 / mantissa.
struct NormalizeResult {
    FPVector3 direction;
    FP magnitude;
};

NormalizeResult NormalizeWithMagnitude(const FPVector3& value) {
    const FP::Raw x = value.X.RawValue();
    const FP::Raw y = value.Y.RawValue();
    const FP::Raw z = value.Z.RawValue();
    const auto squaredLength = static_cast<std::uint64_t>(detail::WrapAdd(
        detail::WrapAdd(detail::WrapMul(x, x), detail::WrapMul(y, y)), detail::WrapMul(z, z)));
    if (squaredLength == 0) {
        return {FPVector3::Zero(), FP::Zero()};
    }

    const FPMath::ExponentMantisaPair root = FPMath::GetSqrtExponentMantissa(squaredLength);
    const FP::Raw mantissa = root.Mantissa;
    const FP::Raw reciprocal = (FP::Raw{1} << 44) / mantissa;
    const int shift = (root.Exponent + 14) & 63; // the hardware shift uses the low 6 bits

    NormalizeResult result;
    result.direction = {FP::FromRaw(detail::WrapMul(reciprocal, x) >> shift),
                        FP::FromRaw(detail::WrapMul(reciprocal, y) >> shift),
                        FP::FromRaw(detail::WrapMul(reciprocal, z) >> shift)};
    const auto scaledMantissa =
        static_cast<FP::Raw>(static_cast<std::uint64_t>(mantissa) << (root.Exponent & 63));
    result.magnitude = FP::FromRaw(scaledMantissa >> 14);
    return result;
}

} // namespace

// 0x7a736a0
FP FPVector3::Magnitude() const {
    return FPMath::Sqrt(SqrMagnitude());
}

// 0x7a736c4
FPVector3 FPVector3::Normalized() const {
    return Normalize(*this);
}

// 0x7a72a14
FPVector3 FPVector3::Normalize(const FPVector3& value) {
    return NormalizeWithMagnitude(value).direction;
}

// 0x7a72aa4
FPVector3 FPVector3::Normalize(const FPVector3& value, FP& magnitude) {
    const NormalizeResult result = NormalizeWithMagnitude(value);
    magnitude = result.magnitude;
    return result.direction;
}

// 0x7a72c68
FPVector3 FPVector3::ClampMagnitude(const FPVector3& vector, FP maxLength) {
    if (vector.SqrMagnitude() > maxLength * maxLength) {
        return Normalize(vector) * maxLength;
    }
    return vector;
}

// 0x7a72dd0
FP FPVector3::Distance(const FPVector3& a, const FPVector3& b) {
    return FPMath::Sqrt(DistanceSquared(a, b));
}

// 0x7a73198
FPVector3 FPVector3::MoveTowards(const FPVector3& from, const FPVector3& to, FP maxDelta) {
    const FPVector3 offset = to - from;
    const FP distance = FPMath::Sqrt(offset.SqrMagnitude());
    if (distance <= maxDelta || distance == FP::Zero()) {
        return to;
    }
    return from + offset / distance * maxDelta;
}

// 0x7a72fe4
FP FPVector3::Angle(const FPVector3& a, const FPVector3& b) {
    // Each vector is normalised by multiplying with 1/length (not via Normalize).
    const FP aSqr = a.SqrMagnitude();
    if (aSqr == FP::Zero()) {
        return FP::Zero();
    }
    const FPVector3 aUnit = a * (FP::One() / FPMath::Sqrt(aSqr));

    const FP bSqr = b.SqrMagnitude();
    if (bSqr == FP::Zero()) {
        return FP::Zero();
    }
    const FPVector3 bUnit = b * (FP::One() / FPMath::Sqrt(bSqr));

    const FP cosine = FPMath::Clamp(Dot(aUnit, bUnit), -FP::One(), FP::One());
    return FPMath::Acos(cosine) * FP::Rad2Deg();
}

// 0x7a72efc
FP FPVector3::SignedAngle(const FPVector3& a, const FPVector3& b, const FPVector3& axis) {
    const FP angle = Angle(a, b);
    return Dot(axis, Cross(a, b)) < FP::Zero() ? -angle : angle;
}

// 0x7a73748
std::string FPVector3::ToString() const {
    return "(" + detail::FormatSingle(X.AsFloat()) + ", " + detail::FormatSingle(Y.AsFloat()) +
           ", " + detail::FormatSingle(Z.AsFloat()) + ")";
}

// TODO(0x7a73640, Photon.Deterministic.FPVector3.Serialize): serialises X, Y, Z through
// FP.Serialize (0x7a726d0) with the frame serializer; needs IDeterministicFrameSerializer
// and FP.Serialize, which belong to the serialisation work.
void FPVector3::Serialize(void* ptr, IDeterministicFrameSerializer* serializer) {
    // Each component goes through the stream in declaration order.
    auto* value = static_cast<FPVector3*>(ptr);
    assert(value != nullptr && serializer != nullptr);
    FP::Serialize(&value->X, *serializer);
    FP::Serialize(&value->Y, *serializer);
    FP::Serialize(&value->Z, *serializer);
}

} // namespace Photon::Deterministic
