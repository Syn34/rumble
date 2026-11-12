// nullable_fp.cpp - NullableNonNegativeFP conversion and formatting.
#include "nullable_fp.h"

namespace Photon::Deterministic {

// 0x7a766b0
NullableNonNegativeFP NullableNonNegativeFP::FromFP(FP v) {
    if (v < 0) {
        throw std::out_of_range("Non negative values only allowed: " + v.ToString());
    }
    return NullableNonNegativeFP{static_cast<std::uint64_t>(v.RawValue()) | HasValueBit};
}

// 0x7a76858
std::string NullableNonNegativeFP::ToString() const {
    return HasValue() ? ValueOrDefault().ToString() : std::string("NULL");
}

} // namespace Photon::Deterministic
