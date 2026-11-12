// nullable_fp_vector3.h - Photon.Deterministic.NullableFPVector3, an optional FPVector3
// with a fixed blittable layout (a 64-bit "has value" flag followed by the vector) so it
// can live inside simulation component data.
#pragma once

#include "fp_vector3.h"

#include <cstdint>

namespace Photon::Deterministic {

struct NullableFPVector3 {
    static constexpr int SIZE = 32; // serialised size in bytes

    std::int64_t _hasValue = 0;
    FPVector3 _value;

    constexpr NullableFPVector3() = default;

    // 0x7a769a4 op_Implicit(FPVector3) - implicit, as in the original.
    constexpr NullableFPVector3(const FPVector3& v) : _hasValue(1), _value(v) {}

    // 0x7a76940 get_HasValue - true only for a flag of exactly 1.
    constexpr bool HasValue() const { return _hasValue == 1; }

    // 0x7a76950 get_Value - throws std::bad_optional_access when the flag is 0 (the
    // original throws InvalidOperationException). Note the asymmetry kept from the
    // original: any non-zero flag counts as "has a value" here.
    FPVector3 Value() const;

    // 0x7a769c0
    constexpr std::int32_t GetHashCode() const { return _hasValue != 0 ? _value.GetHashCode() : 0; }
};

static_assert(sizeof(NullableFPVector3) == NullableFPVector3::SIZE);

} // namespace Photon::Deterministic
