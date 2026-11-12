// math_tables.h - installs FPMath lookup tables computed from <cmath>.
//
// The real tables are game data loaded at runtime, so tests use stand-ins
// computed here from the functions they approximate. Values can differ from the
// original tables in the last bits; tests that use them compare with tolerances.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_lut.h"

#include <cmath>
#include <cstdint>

namespace rr::test {

inline void InstallMathDerivedTables() {
    using Photon::Deterministic::FP;
    using Photon::Deterministic::FPLut;
    const double one = static_cast<double>(FP::RAW_ONE);
    const auto raw = [one](double v) { return static_cast<std::int64_t>(std::llround(v * one)); };

    // One entry per raw angle in [0, 2*pi): sin in the low word, cos in the high word.
    const std::int64_t angles = FP::PiTimes2().RawValue();
    FPLut::sin_cos_lut.resize(static_cast<std::size_t>(angles));
    for (std::int64_t i = 0; i < angles; ++i) {
        const double a = static_cast<double>(i) / one;
        const auto s = static_cast<std::uint32_t>(static_cast<std::int32_t>(raw(std::sin(a))));
        const auto c = static_cast<std::uint32_t>(static_cast<std::int32_t>(raw(std::cos(a))));
        FPLut::sin_cos_lut[static_cast<std::size_t>(i)] =
            static_cast<std::int64_t>((std::uint64_t{c} << 32) | s);
    }

    // Inverse sine/cosine over raw -1.0 .. +1.0, offset by +1.0.
    const auto inverse = static_cast<std::size_t>(2 * FP::RAW_ONE + 1);
    FPLut::asin_lut.resize(inverse);
    FPLut::acos_lut.resize(inverse);
    for (std::size_t i = 0; i < inverse; ++i) {
        const double x = std::fmin(1.0, std::fmax(-1.0, (static_cast<double>(i) - one) / one));
        FPLut::asin_lut[i] = raw(std::asin(x));
        FPLut::acos_lut[i] = raw(std::acos(x));
    }

    // atan(|x|) sampled per raw step to 6, every 1/16 to 250, every 16 to 10000.
    FPLut::atan_lut.clear();
    for (std::int64_t r = 0; r <= 6 * FP::RAW_ONE; ++r) {
        FPLut::atan_lut.push_back(raw(std::atan(static_cast<double>(r) / one)));
    }
    for (double x = 6.0 + 1.0 / 16; x <= 250.0; x += 1.0 / 16) FPLut::atan_lut.push_back(raw(std::atan(x)));
    for (double x = 250.0 + 16; x <= 10000.0 + 16; x += 16) FPLut::atan_lut.push_back(raw(std::atan(x)));

    FPLut::tan_lut.assign(1, 0); // unused by FPMath; only needs to be non-empty

    FPLut::sqrt_aprox_lut.resize(65537);
    for (std::size_t i = 0; i < FPLut::sqrt_aprox_lut.size(); ++i) {
        FPLut::sqrt_aprox_lut[i] =
            static_cast<std::int32_t>(std::llround(std::sqrt(static_cast<double>(i)) * 16384.0));
    }

    FPLut::log2_approx_lut.resize(FPLut::Log2ApproxCount);
    for (std::size_t i = 0; i < FPLut::log2_approx_lut.size(); ++i) {
        FPLut::log2_approx_lut[i] = static_cast<std::uint32_t>(
            std::llround(std::log2(1.0 + static_cast<double>(i) / 64.0) * 2147483648.0));
    }

    FPLut::exp_integral_lut.resize(FPLut::ExpIntegralCount);
    for (int n = -30; n <= 32; ++n) {
        const double scale = n < 0 ? 4398046511104.0 : (n < 21 ? 65536.0 : 1.0);
        FPLut::exp_integral_lut[static_cast<std::size_t>(n + 30)] =
            static_cast<std::int64_t>(std::llround(std::exp(n) * scale));
    }
}

} // namespace rr::test
