// nullable_fp.cpp - NullableNonNegativeFP conversion and formatting.
#include "nullable_fp.h"

#include "bit_stream.h"
#include "deterministic_frame_serializer.h"

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

// 0x7a76734
void NullableNonNegativeFP::Serialize(void* ptr, IDeterministicFrameSerializer& serializer) {
    serializer.Stream().Serialize(&static_cast<NullableNonNegativeFP*>(ptr)->_value);
}

} // namespace Photon::Deterministic
