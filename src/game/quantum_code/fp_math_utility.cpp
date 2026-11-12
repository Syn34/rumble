// Original: Quantum.FPMathUtility (quantum.code.dll). Addresses refer to the
// arm64 libil2cpp.so of v1.10.4 and are for traceability only.
#include "fp_math_utility.h"

#include "photon_deterministic/fp_math.h"

#include <cmath>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {
constexpr int kByteMax = 255;
}

// 0x7be251c
bool FPMathUtility::Contains(std::int32_t flags, std::int32_t flag) {
    return (flags & flag) != 0;
}

// 0x7be2528
bool FPMathUtility::EqualWithEpsilon(const FPVector3& a, const FPVector3& b) {
    return FPVector3::DistanceSquared(a, b) < FP::EN3();
}

// 0x7be2568
bool FPMathUtility::EqualWithEpsilon(FP a, FP b) {
    return FPMath::Abs(a - b) < FP::EN3();
}

// 0x7be257c
FP FPMathUtility::SquareOf(FP value) {
    return value * value;
}

// 0x7be2588
FP FPMathUtility::Pow(FP base, FP exponent) {
    const double result = std::pow(base.AsDouble(), exponent.AsDouble());
    return FP::FromFloatUnsafe(static_cast<float>(result));
}

// 0x7be25fc
FP FPMathUtility::DegreesToRadians(FP degrees) {
    return degrees * FP::Deg2Rad();
}

// 0x7be260c
FP FPMathUtility::RadiansToDegrees(FP radians) {
    return radians * FP::Rad2Deg();
}

// 0x7be27cc
FP FPMathUtility::Map(FP inMin, FP inMax, FP outMin, FP outMax, FP value) {
    if (value <= inMin) {
        return outMin;
    }
    if (value >= inMax) {
        return outMax;
    }
    const FP t = (value - inMin) / (inMax - inMin);
    return outMin + t * (outMax - outMin);
}

// 0x7be2804
std::uint8_t FPMathUtility::QuantizeFP01ToByte(FP value01) {
    const FP t = FPMath::Clamp01(value01);
    // 255 * t, truncated. Done on the raw value so no precision is lost.
    return static_cast<std::uint8_t>((t.RawValue() * kByteMax) >> FP::FractionBits);
}

// 0x7be2824
std::uint8_t FPMathUtility::QuantizeFPToByte(FP value, FP min, FP max) {
    FP t;
    if (value <= min) {
        t = FP::Zero();
    } else if (value >= max) {
        t = FP::One();
    } else {
        t = (value - min) / (max - min);
    }
    return QuantizeFP01ToByte(t);
}

// 0x7be2874
FP FPMathUtility::UnQuantizeByteToFP(std::uint8_t quantized, FP min, FP max) {
    if (quantized == 0) {
        return min;
    }
    if (quantized == kByteMax) {
        return max;
    }
    const FP t = FP::FromRaw((FP::Raw{quantized} << FP::FractionBits) / kByteMax);
    return min + (max - min) * t;
}

// 0x7be28b8
FPVector3 FPMathUtility::ComponentMax(const FPVector3& a, const FPVector3& b) {
    return {FPMath::Max(a.X, b.X), FPMath::Max(a.Y, b.Y), FPMath::Max(a.Z, b.Z)};
}

// 0x7be28e4
FP FPMathUtility::GetMaxComponent(const FPVector3& v) {
    return FPMath::Max(v.X, FPMath::Max(v.Y, v.Z));
}

// 0x7be2910
FPVector3 FPMathUtility::ClosestPointOnLine(const FPVector3& point, const FPVector3& lineStart,
                                            const FPVector3& lineEnd) {
    const FPVector3 segment = lineEnd - lineStart;
    const FP lengthSqr = segment.SqrMagnitude();
    if (lengthSqr == FP::Zero()) {
        return lineStart;
    }
    const FP t = FPMath::Clamp01(FPVector3::Dot(point - lineStart, segment) / lengthSqr);
    return lineStart + segment * t;
}

// 0x7be2a2c
FP FPMathUtility::DistanceSqrToSegment(FPVector2 point, FPVector2 a, FPVector2 b) {
    const FP lengthSqr = FPVector2::DistanceSquared(a, b);
    if (lengthSqr == FP::Zero()) {
        return FPVector2::DistanceSquared(point, a);
    }
    const FPVector2 segment = b - a;
    const FP t = FPMath::Max(FP::Zero(),
                             FPMath::Min(FP::One(), FPVector2::Dot(point - a, segment) / lengthSqr));
    return FPVector2::DistanceSquared(point, a + segment * t);
}

} // namespace Quantum
