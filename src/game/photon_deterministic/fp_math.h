// fp_math.h - scalar helpers on FP used by gameplay code.
#pragma once

#include "fp.h"

namespace Photon::Deterministic::FPMath {

constexpr FP Min(FP a, FP b) { return a < b ? a : b; }
constexpr FP Max(FP a, FP b) { return a > b ? a : b; }

constexpr FP Abs(FP v) { return v < FP::Zero() ? -v : v; }

constexpr FP Clamp(FP v, FP lo, FP hi) { return v < lo ? lo : (v > hi ? hi : v); }
constexpr FP Clamp01(FP v) { return Clamp(v, FP::Zero(), FP::One()); }

} // namespace Photon::Deterministic::FPMath
