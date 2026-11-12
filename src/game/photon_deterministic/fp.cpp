#include "fp.h"

#include <cmath>

namespace Photon::Deterministic {

FP FP::FromFloatUnsafe(float value) {
    const float scaled = value * static_cast<float>(OneRaw);

    // 2^63 is exactly representable as a float; anything at or beyond it cannot
    // fit in the raw integer. The original code raised an overflow error here;
    // we saturate instead so a bad input can't crash the simulation.
    constexpr float limit = 9223372036854775808.0f;
    if (std::isnan(scaled)) {
        return Zero();
    }
    if (scaled >= limit) {
        return MaxValue();
    }
    if (scaled < -limit) {
        return MinValue();
    }
    return FromRaw(static_cast<Raw>(scaled)); // truncates toward zero
}

} // namespace Photon::Deterministic
