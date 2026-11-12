// FPMath tests. Lookup-table functions are tested with synthetic tables (the
// real ones are game data): "identity" tables whose entry i holds i reveal
// exactly which index a function reads, and small tables built from the
// defining formulas check the arithmetic around the lookup.
#include "test_framework.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_lut.h"
#include "photon_deterministic/fp_math.h"

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

using Photon::Deterministic::FP;
using Photon::Deterministic::FPLut;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {

template <typename Exception, typename Fn> bool Throws(Fn&& fn) {
    try {
        fn();
    } catch (const Exception&) {
        return true;
    }
    return false;
}

FP Raw(std::int64_t raw) {
    return FP::FromRaw(raw);
}

std::vector<std::int64_t> IdentityTable(std::size_t size) {
    std::vector<std::int64_t> table(size);
    for (std::size_t i = 0; i < size; ++i) {
        table[i] = static_cast<std::int64_t>(i);
    }
    return table;
}

// floor(sqrt(v)) for v < 2^62, exact.
std::int64_t IntegerSqrt(std::int64_t v) {
    auto r = static_cast<std::int64_t>(std::sqrt(static_cast<double>(v)));
    while (r * r > v) {
        --r;
    }
    while ((r + 1) * (r + 1) <= v) {
        ++r;
    }
    return r;
}

// sqrt(i) * 2^14 for i in [0, 65536]: the quantity the original table holds.
void InstallSqrtTable() {
    FPLut::sqrt_aprox_lut.resize(65537);
    for (std::int64_t i = 0; i <= 65536; ++i) {
        FPLut::sqrt_aprox_lut[static_cast<std::size_t>(i)] =
            static_cast<std::int32_t>(IntegerSqrt(i << 28));
    }
}

} // namespace

TEST_CASE("FPMath table functions throw before FPLut is loaded") {
    FPLut::sin_cos_lut.clear();
    FPLut::sqrt_aprox_lut.clear();
    CHECK(Throws<std::logic_error>([] { FPMath::Sin(FP::One()); }));
    CHECK(Throws<std::logic_error>([] { FPMath::Sqrt(FP::One()); }));
}

TEST_CASE("FPMath Sin/Cos read |rad| mod 2pi and restore sin's sign") {
    // Entry i: sin = i (low word), cos = -i (high word).
    constexpr std::int64_t entries = 411774; // one per raw angle in [0, 2pi)
    FPLut::sin_cos_lut.resize(entries);
    for (std::int64_t i = 0; i < entries; ++i) {
        const auto sinBits = static_cast<std::uint32_t>(i);
        const auto cosBits = static_cast<std::uint32_t>(-i);
        FPLut::sin_cos_lut[static_cast<std::size_t>(i)] =
            static_cast<std::int64_t>((static_cast<std::uint64_t>(cosBits) << 32) | sinBits);
    }
    CHECK(FPMath::Sin(Raw(5)) == Raw(5));
    CHECK(FPMath::Sin(Raw(-5)) == Raw(-5));
    CHECK(FPMath::Cos(Raw(-7)) == Raw(-7));
    CHECK(FPMath::Sin(Raw(entries + 9)) == Raw(9));
    CHECK(FPMath::Cos(FP::PiTimes2() * 3 + Raw(11)) == Raw(-11));

    FP s, c;
    FPMath::SinCos(Raw(-1000), s, c);
    CHECK(s == Raw(-1000) && c == Raw(-1000));
    std::int64_t sRaw = 0, cRaw = 0;
    FPMath::SinCosRaw(Raw(1234), sRaw, cRaw);
    CHECK(sRaw == 1234 && cRaw == -1234);

    // |MinValue| doesn't exist; the original failed its bounds check too.
    CHECK(Throws<std::out_of_range>([] { FPMath::Sin(FP::MinValue()); }));
}

TEST_CASE("FPMath Asin/Acos index value + 1 and reject |value| > 1") {
    FPLut::asin_lut = IdentityTable(131073);
    FPLut::acos_lut = IdentityTable(131073);
    CHECK(FPMath::Asin(-FP::One()) == Raw(0));
    CHECK(FPMath::Asin(FP::Zero()) == Raw(65536));
    CHECK(FPMath::Asin(FP::One()) == Raw(131072));
    CHECK(FPMath::Acos(FP::Half()) == Raw(65536 + 32768));
    CHECK(FPMath::Asin(FP::One() + Raw(1)) == FP::MinValue());
    CHECK(FPMath::Acos(-FP::One() - Raw(1)) == FP::MinValue());
    CHECK(FPMath::Asin(FP::MaxValue()) == FP::MinValue());
}

TEST_CASE("FPMath Atan uses three resolutions plus a final entry") {
    FPLut::atan_lut = IdentityTable(397731);
    const auto atanIndex = [](FP v) { return FPMath::Atan(v).RawValue(); };
    CHECK(atanIndex(Raw(12345)) == 12345);
    CHECK(atanIndex(FP::FromInt(6)) == 393216);             // end of the fine part
    CHECK(atanIndex(FP::FromInt(6) + Raw(4095)) == 393216); // 1/16 steps
    CHECK(atanIndex(FP::FromInt(6) + Raw(4096)) == 393217);
    CHECK(atanIndex(FP::FromInt(250)) == 393216 + 244 * 16);             // 397120
    CHECK(atanIndex(FP::FromInt(250 + 16)) == 397121);                   // 16.0 steps
    CHECK(atanIndex(FP::FromInt(10000)) == 397120 + (10000 - 250) / 16); // 397729
    CHECK(atanIndex(FP::FromInt(10000) + Raw(1)) == 397730);             // last entry
    CHECK(atanIndex(FP::FromInt(-6)) == -393216);                        // odd function
}

TEST_CASE("FPMath Atan2 picks the quadrant") {
    FPLut::atan_lut = IdentityTable(397731); // atan(x) "=" x for small x
    const FP one = FP::One();
    CHECK(FPMath::Atan2(one, one) == one);
    CHECK(FPMath::Atan2(one, -one) == -one + FP::Pi());
    CHECK(FPMath::Atan2(-one, -one) == one - FP::Pi());
    CHECK(FPMath::Atan2(FP::Zero(), -one) == FP::Pi()); // y == 0 counts as upper half
    CHECK(FPMath::Atan2(one, FP::Zero()) == FP::PiOver2());
    CHECK(FPMath::Atan2(-one, FP::Zero()) == -FP::PiOver2());
    CHECK(FPMath::Atan2(FP::Zero(), FP::Zero()) == FP::Zero());
}

TEST_CASE("FPMath Sqrt scales the table by even powers of two") {
    InstallSqrtTable();
    CHECK(FPMath::Sqrt(FP::Zero()) == FP::Zero());
    CHECK(FPMath::Sqrt(FP::_0_25()) == FP::Half());
    CHECK(FPMath::Sqrt(FP::One()) == FP::One());
    CHECK(FPMath::Sqrt(FP::FromInt(4)) == FP::FromInt(2));
    CHECK(FPMath::Sqrt(FP::FromInt(100)) == FP::FromInt(10));
    CHECK(FPMath::Sqrt(FP::FromInt(1 << 20)) == FP::FromInt(1 << 10));
    CHECK(FPMath::SqrtRaw(FP::FromInt(9).RawValue()) == FP::FromInt(3).RawValue());
    // sqrt(2) = 1.41421356 -> raw 92681.9; the table path truncates.
    const std::int64_t root2 = FPMath::Sqrt(FP::FromInt(2)).RawValue();
    CHECK(root2 >= 92680 && root2 <= 92682);
    CHECK(Throws<std::out_of_range>([] { FPMath::SqrtRaw(-1); }));

    const auto direct = FPMath::GetSqrtExponentMantissa(16384);
    CHECK(direct.Exponent == 0 && direct.Mantissa == (1 << 21));
    // 4.0 = 2^18 raw: shift 4 leaves 2^14, whose root is 2^7 * 2^14.
    const auto scaled = FPMath::GetSqrtExponentMantissa(std::uint64_t{1} << 18);
    CHECK(scaled.Exponent == 2 && scaled.Mantissa == (1 << 21));
    // Even huge unsigned inputs stay inside the table.
    const auto top = FPMath::GetSqrtExponentMantissa(~std::uint64_t{0});
    CHECK(top.Exponent == 24 && top.Mantissa == FPLut::sqrt_aprox_lut[65535]);
}

TEST_CASE("FPMath Exp combines e^n from the table with a polynomial for the rest") {
    // exp_integral_lut as described in fp_lut.h; only the entries the checks
    // below depend on need specific values.
    FPLut::exp_integral_lut.assign(63, 0);
    const auto entry = [](int n) -> std::int64_t& {
        return FPLut::exp_integral_lut[static_cast<std::size_t>(n + 30)];
    };
    entry(0) = 65536;  // e^0 * 2^16
    entry(1) = 178145; // e^1 * 2^16, truncated
    entry(-1) = static_cast<std::int64_t>(std::exp(-1.0) * 4398046511104.0); // e^-1 * 2^42
    entry(25) = static_cast<std::int64_t>(std::exp(25.0));                   // unscaled from n = 21

    CHECK(FPMath::Exp(FP::Zero()) == FP::One());
    // x = 0.25: n = 0, f = 16384. Terms: f^2/2 -> 2048, f^3/6 -> 170,
    // f^4/24 -> 10, so the polynomial is 65536 + 16384 + 2048 + 170 + 10.
    CHECK(FPMath::Exp(FP::_0_25()) == Raw(84148));
    // x = 0.75 rounds to n = 1 with f = -0.25: 65536 - 16384 + 2048 - 171 + 10
    // (the cubic term floors toward -inf when shifted) = 51039.
    CHECK(FPMath::Exp(FP::_0_75()) == Raw((std::int64_t{178145} * 51039) >> 16));
    // Negative n: the 2^42-scaled entry is rounded back to 16 bits.
    CHECK(FPMath::Exp(-FP::One()) == Raw((entry(-1) * 65536 + (std::int64_t{1} << 41)) >> 42));
    // n >= 21: the unscaled entry times the FP polynomial is already FP.
    CHECK(FPMath::Exp(FP::FromInt(25)) == Raw(entry(25) * 65536));
    // Range limits.
    CHECK(FPMath::Exp(FP::FromInt(-31)) == FP::Zero());
    CHECK(FPMath::Exp(FP::FromInt(33)) == FP::MaxValue());
}

TEST_CASE("FPMath rounding helpers") {
    CHECK(FPMath::RoundToInt(FP::FromRaw(163840)) == 3);   // 2.5 rounds up
    CHECK(FPMath::RoundToInt(FP::FromRaw(-163840)) == -2); // -2.5 rounds up too
    CHECK(FPMath::RoundToInt(FP::FromRaw(-163841)) == -3);
    CHECK(FPMath::FloorToInt(-FP::Half()) == -1);
    CHECK(FPMath::FloorToInt(FP::_1_75()) == 1);
    CHECK(FPMath::Ceiling(FP::FromRaw(81920)) == FP::FromInt(2));   // 1.25
    CHECK(FPMath::Ceiling(FP::FromRaw(-81920)) == FP::FromInt(-1)); // -1.25
    CHECK(FPMath::Ceiling(FP::FromInt(3)) == FP::FromInt(3));
    CHECK(FPMath::CeilToInt(FP::FromRaw(-81920)) == -1);
    CHECK(FPMath::CeilToInt(FP::FromRaw(81920)) == 2);
    CHECK(FPMath::CeilToInt(FP::FromInt(2)) == 2);
}

TEST_CASE("FPMath sign, abs, min/max and clamps") {
    CHECK(FPMath::Sign(FP::Zero()) == FP::One());
    CHECK(FPMath::Sign(FP::FromRaw(-1)) == FP::Minus_1());
    CHECK(FPMath::Abs(FP::FromInt(-5)) == FP::FromInt(5));
    CHECK(FPMath::Abs(FP::MinValue()) == FP::MinValue());
    CHECK(FPMath::Min(FP::_3(), FP::_1(), FP::_2()) == FP::_1());
    CHECK(FPMath::Max(FP::_3(), FP::_1(), FP::_2()) == FP::_3());
    CHECK(FPMath::Clamp01(FP::FromInt(2)) == FP::One());
    CHECK(FPMath::Clamp01(FP::FromInt(-2)) == FP::Zero());
    CHECK(FPMath::Clamp01(FP::Half()) == FP::Half());
    CHECK(FPMath::Clamp(FP::_5(), FP::_1(), FP::_3()) == FP::_3());
    CHECK(FPMath::Clamp(FP::_5(), FP::_3(), FP::_1()) ==
          FP::_1()); // min > max: max wins unless below min
    CHECK(FPMath::Clamp(FP::_0(), FP::_3(), FP::_1()) == FP::_3());
    CHECK(FPMath::Clamp(std::int32_t{7}, std::int32_t{0}, std::int32_t{5}) == 5);
    CHECK(FPMath::Clamp(std::int64_t{-7}, std::int64_t{0}, std::int64_t{5}) == 0);
    CHECK(FPMath::ClampUseable(FP::MaxValue()) == FP::UseableMax());
    CHECK(FPMath::ClampUseable(FP::MinValue()) == FP::UseableMin());
    CHECK(FPMath::ClampUseable(FP::_8()) == FP::_8());
}

TEST_CASE("FPMath Lerp and InverseLerp clamp t") {
    CHECK(FPMath::Lerp(FP::_2(), FP::_4(), FP::Half()) == FP::_3());
    CHECK(FPMath::Lerp(FP::_2(), FP::_4(), FP::_2()) == FP::_4());
    CHECK(FPMath::Lerp(FP::_2(), FP::_4(), -FP::_1()) == FP::_2());
    CHECK(FPMath::InverseLerp(FP::_2(), FP::_4(), FP::_3()) == FP::Half());
    CHECK(FPMath::InverseLerp(FP::_2(), FP::_4(), FP::_8()) == FP::One());
    CHECK(FPMath::InverseLerp(FP::_2(), FP::_2(), FP::_8()) == FP::Zero());
}

TEST_CASE("FPMath integer helpers") {
    CHECK(FPMath::NextPowerOfTwo(1) == 1);
    CHECK(FPMath::NextPowerOfTwo(5) == 8);
    CHECK(FPMath::NextPowerOfTwo(64) == 64);
    CHECK(Throws<std::logic_error>([] { FPMath::NextPowerOfTwo(0); }));
    CHECK(FPMath::ModuloClamped(-7, 3) == 2);
    CHECK(FPMath::ModuloClamped(7, 3) == 1);
    CHECK(FPMath::ModuloClamped(7, -3) == -2);
    CHECK(Throws<std::logic_error>([] { FPMath::ModuloClamped(1, std::int64_t{1} << 61); }));
    CHECK(Throws<std::logic_error>([] { FPMath::ModuloClamped(1, -(std::int64_t{1} << 61) - 1); }));
}

TEST_CASE("FPMath angle differences wrap into [-half, half)") {
    CHECK(FPMath::AngleBetweenDegrees(FP::FromInt(350), FP::FromInt(10)) == FP::FromInt(20));
    CHECK(FPMath::AngleBetweenDegrees(FP::FromInt(10), FP::FromInt(350)) == FP::FromInt(-20));
    CHECK(FPMath::AngleBetweenDegrees(FP::Zero(), FP::FromInt(180)) == FP::FromInt(-180));
    CHECK(FPMath::AngleBetweenDegrees(FP::FromInt(-720), FP::FromInt(90)) == FP::FromInt(90));
    CHECK(FPMath::AngleBetweenRadiansRaw(0, FP::Pi().RawValue()) == -FP::Pi().RawValue());
    CHECK(FPMath::AngleBetweenRadiansRaw(FP::Pi().RawValue(), 0) == -FP::Pi().RawValue());
    CHECK(FPMath::AngleBetweenRadiansRaw(0, 1000) == 1000);
}
