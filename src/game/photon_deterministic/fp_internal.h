// fp_internal.h - small raw-integer helpers shared by the FP vector/quaternion/matrix types.
//
// Most of the deterministic maths is written with FP's own operators, which already
// reproduce the original's rules (wrapping add, multiply-then-shift per product). The
// helpers here cover the few places where the original deliberately works on the raw
// integers instead: unshifted products, "FP times int" scaling and .NET-style hashing.
// Not part of the public API.
#pragma once

#include "fp.h"

#include <cstdint>

namespace Photon::Deterministic::detail {

// Full 64-bit product of two raw values, wrapping on overflow, with no shift applied.
constexpr FP::Raw WrapMul(FP::Raw a, FP::Raw b) {
    return static_cast<FP::Raw>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
}

constexpr FP::Raw WrapAdd(FP::Raw a, FP::Raw b) {
    return static_cast<FP::Raw>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}

// Product of two FP values shifted by 15 instead of 16, i.e. twice the product with the
// doubling applied before the fractional bits are dropped. The original uses this form
// for "2 * a * b" terms; it differs from (a * b) * 2 in the lowest bit.
constexpr FP TwiceProduct(FP a, FP b) {
    return FP::FromRaw(WrapMul(a.RawValue(), b.RawValue()) >> 15);
}

// FP * int as the original defines it: the raw value scaled by the integer, no shift.
constexpr FP MulInt(FP value, std::int64_t factor) {
    return FP::FromRaw(WrapMul(value.RawValue(), factor));
}

// FP / int as the original defines it: raw value divided by the integer, truncating.
constexpr FP DivInt(FP value, std::int64_t divisor) {
    return FP::FromRaw(value.RawValue() / divisor);
}

// FP division that yields zero for a zero divisor. The original was compiled without
// divide-by-zero checks and runs on AArch64, whose integer divide returns 0 in that case;
// used where a zero divisor is reachable from ordinary (degenerate) input.
constexpr FP DivOrZero(FP numerator, FP divisor) {
    return divisor.RawValue() == 0 ? FP::Zero() : numerator / divisor;
}

// .NET Int64.GetHashCode, which the original's generated hash functions call on each
// FP's raw value: low 32 bits XOR high 32 bits.
constexpr std::int32_t HashRaw(FP value) {
    const auto bits = static_cast<std::uint64_t>(value.RawValue());
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(bits) ^
                                     static_cast<std::uint32_t>(bits >> 32));
}

// The original's struct hashes all follow "hash = hash * 31 + field" starting from 17,
// in 32-bit wrapping arithmetic.
constexpr std::int32_t HashSeed = 17;
constexpr std::int32_t HashCombine(std::int32_t hash, std::int32_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(hash) * 31u +
                                     static_cast<std::uint32_t>(value));
}

// FP.EN3 / FP.Epsilon as the original defines them (raw 65, i.e. 1/1000 truncated).
// Note: FP::EN3() in fp.h currently returns 66; the binary's FP.get_EN3 (0x7a73ac8) and
// FP.get_Epsilon (0x7a73ad8) both return 65, so the vector code uses this constant.
constexpr FP Epsilon = FP::FromRaw(65);

} // namespace Photon::Deterministic::detail
