// fp_math.cpp - Photon.Deterministic.FPMath functions backed by FPLut tables.
#include "fp_math.h"

#include "fp_lut.h"

#include <bit>
#include <stdexcept>
#include <string>

namespace Photon::Deterministic::FPMath {

namespace {

// |raw| with two's-complement wrap (|MinValue| stays negative, which then
// fails the table bounds check, as in the original).
constexpr std::int64_t Magnitude(std::int64_t raw) {
    return raw < 0 ? static_cast<std::int64_t>(0 - static_cast<std::uint64_t>(raw)) : raw;
}

constexpr std::int64_t ApplySign(std::int64_t magnitude, bool negative) {
    return negative ? static_cast<std::int64_t>(0 - static_cast<std::uint64_t>(magnitude))
                    : magnitude;
}

// sin_cos_lut has one entry per raw angle in [0, 2*pi): sin in the low
// 32 bits, cos in the high 32 bits. Angles wrap by the truncated 2*pi.
struct SinCosPair {
    std::int64_t sin;
    std::int64_t cos;
};

SinCosPair LookUpSinCos(FP rad) {
    const std::int64_t angle = Magnitude(rad.RawValue()) % FP::PiTimes2().RawValue();
    const auto packed = static_cast<std::uint64_t>(FPLut::Entry(FPLut::sin_cos_lut, angle));
    // sin is odd and cos is even in the angle, so only |rad| is looked up.
    const std::int64_t sinOfMagnitude = static_cast<std::int32_t>(packed & 0xFFFFFFFFu);
    const std::int64_t cos = static_cast<std::int32_t>(packed >> 32);
    return {ApplySign(sinOfMagnitude, rad < 0), cos};
}

// asin_lut / acos_lut cover raw -1.0 .. +1.0 at full resolution, offset by
// +1.0. Out-of-domain input returns MinValue rather than throwing.
FP LookUpInverseTrig(const std::vector<std::int64_t>& lut, FP value) {
    const std::uint64_t index = static_cast<std::uint64_t>(value.RawValue()) + FP::RAW_ONE;
    if (index > static_cast<std::uint64_t>(2 * FP::RAW_ONE)) {
        return FP::MinValue();
    }
    return FP::FromRaw(FPLut::Entry(lut, static_cast<std::int64_t>(index)));
}

} // namespace

// 0x7a742fc
FP Sin(FP rad) {
    return FP::FromRaw(LookUpSinCos(rad).sin);
}

// 0x7a743b0
FP Cos(FP rad) {
    return FP::FromRaw(LookUpSinCos(rad).cos);
}

// 0x7a74458
void SinCos(FP rad, FP& sin, FP& cos) {
    const SinCosPair pair = LookUpSinCos(rad);
    sin = FP::FromRaw(pair.sin);
    cos = FP::FromRaw(pair.cos);
}

// 0x7a74528
void SinCosRaw(FP rad, std::int64_t& sinRaw, std::int64_t& cosRaw) {
    const SinCosPair pair = LookUpSinCos(rad);
    sinRaw = pair.sin;
    cosRaw = pair.cos;
}

// 0x7a745f8
FP Asin(FP value) {
    return LookUpInverseTrig(FPLut::asin_lut, value);
}

// 0x7a71d9c
FP Acos(FP value) {
    return LookUpInverseTrig(FPLut::acos_lut, value);
}

// 0x7a74684
FP Atan(FP value) {
    // atan_lut samples atan(|x|) at three resolutions, finest near zero:
    //   [0, 6]        every raw step   (entries 0 .. 393216)
    //   (6, 250]      every 1/16       (entries 393216 .. 397120)
    //   (250, 10000]  every 16.0       (entries 397120 .. 397729)
    // and its last entry stands for anything larger. atan is odd, so the
    // sign is restored afterwards.
    constexpr std::int64_t fineEnd = 6 * FP::RAW_ONE;
    constexpr std::int64_t mediumEnd = 250 * FP::RAW_ONE;
    constexpr std::int64_t coarseEnd = 10000 * FP::RAW_ONE;
    constexpr int mediumStepShift = 12; // 1/16 = 2^12 raw
    constexpr int coarseStepShift = 20; // 16.0 = 2^20 raw
    constexpr std::int64_t mediumFirstEntry = fineEnd;
    constexpr std::int64_t coarseFirstEntry =
        mediumFirstEntry + ((mediumEnd - fineEnd) >> mediumStepShift);

    const std::int64_t magnitude = Magnitude(value.RawValue());
    std::int64_t index = 0;
    if (magnitude <= fineEnd) {
        index = magnitude; // includes |MinValue|, which fails the bounds check
    } else if (magnitude <= mediumEnd) {
        index = mediumFirstEntry + ((magnitude - fineEnd) >> mediumStepShift);
    } else if (magnitude <= coarseEnd) {
        index = coarseFirstEntry + ((magnitude - mediumEnd) >> coarseStepShift);
    } else {
        index = static_cast<std::int64_t>(FPLut::atan_lut.size()) - 1;
    }
    return FP::FromRaw(ApplySign(FPLut::Entry(FPLut::atan_lut, index), value < 0));
}

// 0x7a7479c
FP Atan2(FP y, FP x) {
    if (x > 0) {
        return Atan(y / x);
    }
    if (x < 0) {
        // Left half-plane: shift the principal value by pi toward y's side.
        const FP angle = Atan(y / x);
        return y < 0 ? angle - FP::Pi() : angle + FP::Pi();
    }
    if (y > 0) {
        return FP::PiOver2();
    }
    return y == 0 ? FP::_0() : -FP::PiOver2();
}

// 0x7a7499c
FP Sqrt(FP value) {
    return FP::FromRaw(SqrtRaw(value.RawValue()));
}

// 0x7a71898
std::int64_t SqrtRaw(std::int64_t x) {
    if (x < 0) {
        // ArgumentOutOfRangeException("x", ...) in the original.
        throw std::out_of_range("The number has to be positive: " + std::to_string(x));
    }
    // The table holds sqrt(i) * 2^14; sqrt of a raw value needs sqrt(i) * 2^8.
    constexpr int surplusBits = 6;
    const ExponentMantisaPair root = GetSqrtExponentMantissa(static_cast<std::uint64_t>(x));
    return (static_cast<std::int64_t>(root.Mantissa) << root.Exponent) >> surplusBits;
}

// 0x7a749a0
ExponentMantisaPair GetSqrtExponentMantissa(std::uint64_t x) {
    // Values up to 1.0 index the table directly.
    constexpr std::uint64_t directMax = FP::RAW_ONE;
    if (x <= directMax) {
        return {0, FPLut::Entry(FPLut::sqrt_aprox_lut, static_cast<std::int64_t>(x))};
    }
    // Larger values: shift right by an even amount so 14..16 significant bits
    // remain, look that up, and scale the root back by half the shift:
    // sqrt(m * 4^k) = sqrt(m) * 2^k.
    constexpr int keptBits = 14;
    const int evenLog2 = (static_cast<int>(std::bit_width(x)) - 1) & ~1;
    const int shift = evenLog2 - keptBits;
    const auto index = static_cast<std::int64_t>(x >> shift);
    return {shift / 2, FPLut::Entry(FPLut::sqrt_aprox_lut, index)};
}

// 0x7a74ac0
std::int64_t ModuloClamped(std::int64_t a, std::int64_t n) {
    // Limits keep the intermediate (a % n) + n from overflowing.
    constexpr std::int64_t limit = std::int64_t{1} << 61;
    if (n >= limit) {
        throw std::logic_error("N too big");
    }
    if (n <= -limit - 1) {
        throw std::logic_error("N too small");
    }
    return FP::Remainder(FP::Remainder(a, n) + n, n);
}

// 0x7a74810
std::int32_t NextPowerOfTwo(std::int32_t value) {
    if (value <= 0) {
        throw std::logic_error("Number must be positive");
    }
    return static_cast<std::int32_t>(std::bit_ceil(static_cast<std::uint32_t>(value)));
}

// 0x7a74bf8
FP Exp(FP x) {
    // e^x = e^n * e^f with n = x rounded to the nearest integer (halves up) and
    // f = x - n in [-0.5, 0.5). e^n comes from exp_integral_lut, e^f from a
    // 4th-order Taylor polynomial.
    constexpr std::int64_t minExponent = -30;
    constexpr std::int64_t maxExponent = 32;
    const std::int64_t raw = x.RawValue();
    const std::int64_t n = (raw >> FP::Precision) + ((raw >> (FP::Precision - 1)) & 1);
    if (n < minExponent) {
        return FP::_0();
    }
    if (n > maxExponent) {
        return FP::MaxValue();
    }

    // The low 16 bits read as a signed number are exactly x - n.
    const std::int64_t f = static_cast<std::int16_t>(raw & 0xFFFF);
    // Each power keeps its natural scale (f^k has 16k fractional bits) and is
    // brought back to 16 bits after dividing by k!, truncating.
    const std::int64_t f2 = f * f;
    const std::int64_t f3 = f2 * f;
    const std::int64_t f4 = f3 * f;
    const std::int64_t polynomial = FP::RAW_ONE + f + f2 / (2 << FP::Precision) +
                                    ((f3 / 6) >> (2 * FP::Precision)) +
                                    ((f4 / 24) >> (3 * FP::Precision));

    // The table stores e^n with 42 fractional bits for n < 0 (rounded when
    // scaling back), 16 for small n and none from n = 21 on, where e^n * 2^16
    // would leave no headroom for the multiplication.
    constexpr std::int64_t firstUnscaledExponent = 21;
    constexpr int negativeScaleBits = 42;
    const std::int64_t power = FPLut::Entry(FPLut::exp_integral_lut, n - minExponent);
    const auto product = static_cast<std::int64_t>(static_cast<std::uint64_t>(power) *
                                                   static_cast<std::uint64_t>(polynomial));
    if (n < 0) {
        const auto rounded =
            static_cast<std::uint64_t>(product) + (std::uint64_t{1} << (negativeScaleBits - 1));
        return FP::FromRaw(static_cast<std::int64_t>(rounded) >> negativeScaleBits);
    }
    return FP::FromRaw(n < firstUnscaledExponent ? product >> FP::Precision : product);
}

} // namespace Photon::Deterministic::FPMath
