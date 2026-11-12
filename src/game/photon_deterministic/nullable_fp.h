// nullable_fp.h - Photon.Deterministic.NullableFP and NullableNonNegativeFP:
// optional FP values with a fixed binary layout (they are part of
// deterministic simulation state, so the layout is kept as in the original).
#pragma once

#include "fp.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace Photon::Deterministic {

// 16 bytes: a 64-bit "has value" word followed by the value.
struct NullableFP {
    static constexpr int SIZE = 16;

    std::int64_t _hasValue = 0;
    FP _value;

    // 0x7a88604 NullableFP.HasValue. Note the original tests for exactly 1
    // here, but Value (below) accepts any non-zero flag.
    constexpr bool HasValue() const { return _hasValue == 1; }

    // 0x7a88614 NullableFP.Value. Throws std::logic_error when empty (the
    // original threw NullReferenceException).
    constexpr FP Value() const {
        if (_hasValue == 0) {
            throw std::logic_error("NullableFP has no value");
        }
        return _value;
    }

    // 0x7a8865c NullableFP.ValueOrDefault
    constexpr FP ValueOrDefault(FP v) const { return HasValue() ? Value() : v; }

    // 0x7a88674 NullableFP.op_Implicit(FP)
    static constexpr NullableFP FromFP(FP v) { return NullableFP{1, v}; }

    // 0x7a88680 NullableFP.GetHashCode
    constexpr std::int32_t GetHashCode() const { return _hasValue != 0 ? _value.GetHashCode() : 0; }
};

static_assert(sizeof(NullableFP) == NullableFP::SIZE, "NullableFP layout must stay 16 bytes");

// 8 bytes: a non-negative FP whose sign bit doubles as the "has value" flag.
struct NullableNonNegativeFP {
    static constexpr int SIZE = 8;
    static constexpr std::uint64_t HasValueBit = std::uint64_t{1} << 63;
    static constexpr std::uint64_t ValueMask = HasValueBit - 1;

    std::uint64_t _value = 0;

    // 0x7a76650 NullableNonNegativeFP.HasValue
    constexpr bool HasValue() const { return (_value & HasValueBit) != 0; }

    // 0x7a7665c NullableNonNegativeFP.Value. Throws std::logic_error when empty
    // (NullReferenceException in the original).
    constexpr FP Value() const {
        if (!HasValue()) {
            throw std::logic_error("NullableNonNegativeFP has no value");
        }
        return ValueOrDefault();
    }

    // 0x7a766a4 NullableNonNegativeFP.ValueOrDefault: the value, or 0.
    constexpr FP ValueOrDefault() const {
        return FP::FromRaw(static_cast<FP::Raw>(_value & ValueMask));
    }

    // 0x7a766b0 NullableNonNegativeFP.op_Implicit(FP). Throws
    // std::out_of_range for negative values (ArgumentOutOfRangeException).
    static NullableNonNegativeFP FromFP(FP v);

    // 0x7a76858 NullableNonNegativeFP.ToString: the value, or "NULL".
    std::string ToString() const;

    // 0x7a768bc NullableNonNegativeFP.GetHashCode
    constexpr std::int32_t GetHashCode() const {
        return HasValue() ? ValueOrDefault().GetHashCode() : 0;
    }

    // TODO(0x7a76734, Photon.Deterministic.NullableNonNegativeFP.Serialize):
    // static Serialize(void* ptr, IDeterministicFrameSerializer serializer)
    // passes _value to serializer.Stream (BitStream.Serialize(ulong*)). Needs
    // IDeterministicFrameSerializer/BitStream.
};

static_assert(sizeof(NullableNonNegativeFP) == NullableNonNegativeFP::SIZE,
              "NullableNonNegativeFP layout must stay 8 bytes");

} // namespace Photon::Deterministic
