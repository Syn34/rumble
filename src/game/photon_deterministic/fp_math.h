// fp_math.h - Photon.Deterministic.FPMath: scalar maths on FP.
//
// The simple helpers are inline here. Trigonometry, square root and Exp read
// the FPLut tables and live in fp_math.cpp; they throw if FPLut::Init hasn't
// been called (see fp_lut.h).
#pragma once

#include "fp.h"

#include <cstdint>

namespace Photon::Deterministic::FPMath {

// --- Lookup-table functions (fp_math.cpp) ---
FP Sin(FP rad);                                                     // 0x7a742fc
FP Cos(FP rad);                                                     // 0x7a743b0
void SinCos(FP rad, FP& sin, FP& cos);                              // 0x7a74458
void SinCosRaw(FP rad, std::int64_t& sinRaw, std::int64_t& cosRaw); // 0x7a74528
FP Asin(FP value);                                                  // 0x7a745f8
FP Acos(FP value);                                                  // 0x7a71d9c
FP Atan(FP value);                                                  // 0x7a74684
FP Atan2(FP y, FP x);                                               // 0x7a7479c
FP Sqrt(FP value);                                                  // 0x7a7499c
std::int64_t SqrtRaw(std::int64_t x);                               // 0x7a71898
FP Exp(FP x);                                                       // 0x7a74bf8

// FPMath.ExponentMantisaPair (spelling as in the original): a square root as
// Mantissa * 2^Exponent, with the mantissa taken from sqrt_aprox_lut.
struct ExponentMantisaPair {
    std::int32_t Exponent = 0;
    std::int32_t Mantissa = 0;
};
ExponentMantisaPair GetSqrtExponentMantissa(std::uint64_t x); // 0x7a749a0

// 0x7a74ac0 FPMath.ModuloClamped: a mod n with the sign of n ([0, n) for n > 0).
// Throws std::logic_error ("N too big" / "N too small"; InvalidOperationException
// in the original) when |n| >= 2^61.
std::int64_t ModuloClamped(std::int64_t a, std::int64_t n);

// 0x7a74810 FPMath.NextPowerOfTwo: smallest power of two >= value. Throws
// std::logic_error ("Number must be positive") when value <= 0.
std::int32_t NextPowerOfTwo(std::int32_t value);

// --- Inline helpers ---

// 0x7a71ea4 FPMath.Sign: 1 for zero and positive values, -1 for negative.
constexpr FP Sign(FP value) {
    return value < 0 ? FP::Minus_1() : FP::_1();
}

// 0x7a74884 FPMath.Abs (Abs(MinValue) wraps to MinValue).
constexpr FP Abs(FP value) {
    return value < 0 ? -value : value;
}

// 0x7a74890 FPMath.RoundToInt: nearest integer, halves round up.
constexpr std::int32_t RoundToInt(FP value) {
    const auto floor = static_cast<std::uint32_t>(value.RawValue() >> FP::Precision);
    const auto half = static_cast<std::uint32_t>((value.RawValue() >> (FP::Precision - 1)) & 1);
    return static_cast<std::int32_t>(floor + half);
}

// 0x7a748a0 FPMath.FloorToInt
constexpr std::int32_t FloorToInt(FP value) {
    return static_cast<std::int32_t>(value.RawValue() >> FP::Precision);
}

// 0x7a748a8 FPMath.Ceiling
constexpr FP Ceiling(FP value) {
    constexpr std::uint64_t fractionMask = FP::RAW_ONE - 1;
    const std::uint64_t raised = static_cast<std::uint64_t>(value.RawValue()) + fractionMask;
    return FP::FromRaw(static_cast<FP::Raw>(raised & ~fractionMask));
}

// 0x7a748b8 FPMath.CeilToInt
constexpr std::int32_t CeilToInt(FP value) {
    const auto floor = static_cast<std::uint32_t>(value.RawValue() >> FP::Precision);
    const bool hasFraction = (value.RawValue() & (FP::RAW_ONE - 1)) != 0;
    return static_cast<std::int32_t>(floor + (hasFraction ? 1u : 0u));
}

// 0x7a71cc8 FPMath.Max
constexpr FP Max(FP val1, FP val2) {
    return val1 > val2 ? val1 : val2;
}
// 0x7a71ce8 FPMath.Min
constexpr FP Min(FP val1, FP val2) {
    return val1 < val2 ? val1 : val2;
}
// 0x7a748c8 FPMath.Min(a, b, c)
constexpr FP Min(FP a, FP b, FP c) {
    return Min(Min(a, b), c);
}
// 0x7a748dc FPMath.Max(a, b, c)
constexpr FP Max(FP a, FP b, FP c) {
    return Max(Max(a, b), c);
}

// 0x7a71c6c FPMath.Clamp: min wins if min > max.
constexpr FP Clamp(FP value, FP min, FP max) {
    if (value < min) {
        return min;
    }
    return value < max ? value : max;
}

// 0x7a74908 FPMath.Clamp(int, int, int)
constexpr std::int32_t Clamp(std::int32_t value, std::int32_t min, std::int32_t max) {
    if (value < min) {
        return min;
    }
    return value < max ? value : max;
}

// 0x7a7491c FPMath.Clamp(long, long, long)
constexpr std::int64_t Clamp(std::int64_t value, std::int64_t min, std::int64_t max) {
    if (value < min) {
        return min;
    }
    return value < max ? value : max;
}

// 0x7a748f0 FPMath.Clamp01
constexpr FP Clamp01(FP value) {
    return Clamp(value, FP::_0(), FP::_1());
}

// 0x7a74930 FPMath.ClampUseable: into [UseableMin, UseableMax].
constexpr FP ClampUseable(FP value) {
    return Max(Min(value, FP::UseableMax()), FP::UseableMin());
}

// 0x7a7494c FPMath.Lerp: t is clamped to [0, 1].
constexpr FP Lerp(FP start, FP end, FP t) {
    return start + (end - start) * Clamp01(t);
}

// 0x7a7496c FPMath.InverseLerp: where value lies between start and end,
// clamped to [0, 1]; 0 when start == end.
constexpr FP InverseLerp(FP start, FP end, FP value) {
    if (end == start) {
        return FP::_0();
    }
    return Clamp01((value - start) / (end - start));
}

// 0x7a74b60 FPMath.AngleBetweenDegrees: signed shortest difference
// target - source, in [-180, 180).
constexpr FP AngleBetweenDegrees(FP source, FP target) {
    const FP fullTurn = FP::FromInt(360);
    const FP halfTurn = FP::FromInt(180);
    FP wrapped = (target - source + halfTurn) % fullTurn;
    if (wrapped < 0) {
        wrapped += fullTurn;
    }
    return wrapped - halfTurn;
}

// 0x7a74ba4 FPMath.AngleBetweenRadiansRaw: the same in raw radians, in
// [-Pi, Pi) using the truncated Pi constant.
constexpr std::int64_t AngleBetweenRadiansRaw(std::int64_t source, std::int64_t target) {
    const FP fullTurn = FP::PiTimes2();
    const FP halfTurn = FP::Pi();
    FP wrapped = (FP::FromRaw(target) - FP::FromRaw(source) + halfTurn) % fullTurn;
    if (wrapped < 0) {
        wrapped += fullTurn;
    }
    return (wrapped - halfTurn).RawValue();
}

} // namespace Photon::Deterministic::FPMath
