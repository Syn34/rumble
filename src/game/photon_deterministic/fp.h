// fp.h - Photon.Deterministic.FP, the deterministic fixed-point number used by
// the gameplay simulation.
//
// The game's simulation is lockstep-deterministic: every client must compute
// bit-identical results, so it never uses floating point. All gameplay values
// (positions, speeds, timers, damage multipliers) are fixed-point numbers with
// 16 fractional bits stored in a signed 64-bit integer (Q47.16).
//
// Game data serialises these values as their raw integer, so the representation
// here must stay exactly 16 fractional bits for loaded data to mean the same
// thing.
//
// Arithmetic rules that matter for matching the original results:
//   * all arithmetic is unchecked: overflow wraps around
//   * multiply = full 64-bit product, then arithmetic shift right by 16
//     (no rounding; the original's MulRound constant is 0)
//   * divide   = (a << 16) / b, truncating toward zero
//   * integer division by zero yields 0 and MinValue / -1 wraps: the original
//     ran on arm64, whose divide instruction behaves that way, and the compiled
//     code had no divide-by-zero checks. We reproduce that on every platform.
//   * vector dot products shift each term separately before summing
//
// Original addresses (in comments) refer to libil2cpp.so of Android v1.10.4.
#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace Photon::Deterministic {

class FP {
  public:
    using Raw = std::int64_t;

    // Original compile-time constants (FP.SIZE, FP.Precision, ...).
    static constexpr int SIZE = 8;
    static constexpr int FRACTIONS_COUNT = 5;
    static constexpr Raw RAW_ONE = Raw{1} << 16;
    static constexpr Raw RAW_ZERO = 0;
    static constexpr int Precision = 16;
    static constexpr int Bits = 64;
    static constexpr Raw MulRound = 0;
    static constexpr int MulShift = 16;
    static constexpr int MulShiftTrunc = 16;
    static constexpr bool UsesRoundedConstants = false;

    // Names used by the rest of this codebase.
    static constexpr int FractionBits = Precision;
    static constexpr Raw OneRaw = RAW_ONE;

    constexpr FP() = default;

    // 0x7a73c74 / 0x7a73c7c / 0x7a73c84 / 0x7a73c8c / 0x7a73c98
    // FP.op_Implicit(int | uint | short | ushort | byte): the original allows
    // these (and only these) integer types to convert implicitly.
    template <typename Int>
        requires std::same_as<Int, std::int32_t> || std::same_as<Int, std::uint32_t> ||
                 std::same_as<Int, std::int16_t> || std::same_as<Int, std::uint16_t> ||
                 std::same_as<Int, std::uint8_t>
    constexpr FP(Int value) // NOLINT(google-explicit-constructor): mirrors C# implicit operator
        : raw_(static_cast<Raw>(value) << Precision) {}

    // 0x7a73e80 FP.FromRaw. (0x7a73cf0, the private FP(long) constructor, does
    // the same thing.) Use at call sites so it's obvious the integer is scaled.
    static constexpr FP FromRaw(Raw raw) {
        FP v;
        v.raw_ = raw;
        return v;
    }

    // Helper for this codebase (not an original method): any 64-bit integer.
    static constexpr FP FromInt(std::int64_t value) { return FromRaw(value << Precision); }

    // 0x7a73e04 FP.FromFloat_UNSAFE: scale by 2^16 and truncate toward zero.
    // Throws std::overflow_error when the scaled value is above 2^63 (the
    // original's checked conversion raised OverflowException). Below -2^63 the
    // result is MinValue and NaN gives 0 - what the original's unchecked arm64
    // conversion produced. "Unsafe" because a float is not deterministic input.
    static FP FromFloat_UNSAFE(float value);
    // Older spelling kept for existing callers.
    static FP FromFloatUnsafe(float value) { return FromFloat_UNSAFE(value); }

    // 0x7a73e84 FP.FromString: parses "[-]int[.fraction]" without using
    // floating point. Throws std::invalid_argument (FormatException) for
    // malformed text and std::overflow_error when the value doesn't fit.
    static FP FromString(std::string_view value);
    // 0x7a74188 FP.ParseInteger: an integer string, scaled to raw.
    static Raw ParseInteger(std::string_view format);
    // 0x7a74070 FP.ParseFractions: the digits after the decimal point, as raw.
    static Raw ParseFractions(std::string_view format);

    // The raw value (the original's public RawValue field).
    constexpr Raw RawValue() const { return raw_; }

    // 0x7a73cc4 FP.AsInt: integer part (floor), truncated to 32 bits.
    constexpr std::int32_t AsInt() const { return static_cast<std::int32_t>(raw_ >> Precision); }
    // 0x7a73ccc FP.AsFloat / 0x7a73cd8 FP.AsDouble
    float AsFloat() const { return static_cast<float>(raw_) / static_cast<float>(RAW_ONE); }
    double AsDouble() const {
        return static_cast<double>(raw_) * (1.0 / static_cast<double>(RAW_ONE));
    }

    // 0x7a73ca4 / 0x7a73cac / 0x7a73cb4 / 0x7a73cbc FP.op_Explicit(FP) to
    // int, long, float and double. Integer conversions floor.
    constexpr explicit operator std::int32_t() const { return AsInt(); }
    constexpr explicit operator std::int64_t() const { return raw_ >> Precision; }
    explicit operator float() const { return AsFloat(); }
    explicit operator double() const { return AsDouble(); }

    // --- Constants (original static properties) ---
    static constexpr FP MinValue() { return FromRaw(std::numeric_limits<Raw>::min()); } // 0x7a73978
    static constexpr FP MaxValue() { return FromRaw(std::numeric_limits<Raw>::max()); } // 0x7a73980
    // Range in which products of two values still fit: +-32768.
    // 0x7a73988
    static constexpr FP UseableMin() { return FromRaw(std::numeric_limits<std::int32_t>::min()); }
    // 0x7a73990
    static constexpr FP UseableMax() { return FromRaw(std::numeric_limits<std::int32_t>::max()); }
    // Angle constants: pi etc. truncated to 16 fractional bits.
    static constexpr FP Pi() { return FromRaw(205887); }       // 0x7a73998
    static constexpr FP PiTimes2() { return FromRaw(411774); } // 0x7a739a4
    static constexpr FP PiOver2() { return FromRaw(102943); }  // 0x7a739b0
    static constexpr FP Deg2Rad() { return FromRaw(1143); }    // 0x7a739bc (pi/180)
    static constexpr FP Rad2Deg() { return FromRaw(3754936); } // 0x7a739c4 (180/pi)
    static constexpr FP _0() { return FromRaw(0); }            // 0x7a739d0
    static constexpr FP _1() { return FromRaw(RAW_ONE); }      // 0x7a739d8
    static constexpr FP _2() { return FromInt(2); }            // 0x7a739e0
    static constexpr FP _3() { return FromInt(3); }            // 0x7a739e8
    static constexpr FP _4() { return FromInt(4); }            // 0x7a739f0
    static constexpr FP _5() { return FromInt(5); }            // 0x7a739f8
    static constexpr FP _6() { return FromInt(6); }            // 0x7a73a00
    static constexpr FP _7() { return FromInt(7); }            // 0x7a73a08
    static constexpr FP _8() { return FromInt(8); }            // 0x7a73a10
    static constexpr FP _10() { return FromInt(10); }          // 0x7a73a18
    static constexpr FP _100() { return FromInt(100); }        // 0x7a73a20
    static constexpr FP _1000() { return FromInt(1000); }      // 0x7a73a28
    // Fraction encodings exactly as the original defines them. Most are the
    // exact value truncated to 16 bits, but _0_03 and _0_05 are one below that
    // and _0_99 is rounded up; they are kept as is because results depend on them.
    static constexpr FP _0_01() { return FromRaw(655); }    // 0x7a73a30
    static constexpr FP _0_02() { return FromRaw(1310); }   // 0x7a73a38
    static constexpr FP _0_10() { return FromRaw(6553); }   // 0x7a73a40
    static constexpr FP _0_20() { return FromRaw(13107); }  // 0x7a73a48
    static constexpr FP _0_25() { return FromRaw(16384); }  // 0x7a73a50
    static constexpr FP _0_50() { return FromRaw(32768); }  // 0x7a73a58
    static constexpr FP _0_75() { return FromRaw(49152); }  // 0x7a73a60
    static constexpr FP _0_03() { return FromRaw(1965); }   // 0x7a73a68
    static constexpr FP _0_05() { return FromRaw(3275); }   // 0x7a73a70
    static constexpr FP _0_33() { return FromRaw(21845); }  // 0x7a73a78 (one third)
    static constexpr FP _0_99() { return FromRaw(64881); }  // 0x7a73a80
    static constexpr FP Minus_1() { return FromInt(-1); }   // 0x7a73a88
    static constexpr FP _1_20() { return FromRaw(78643); }  // 0x7a73a90
    static constexpr FP _1_50() { return FromRaw(98304); }  // 0x7a73a9c
    static constexpr FP _1_75() { return FromRaw(114688); } // 0x7a73aa4
    static constexpr FP _1_33() { return FromRaw(87381); }  // 0x7a73aac (four thirds)
    static constexpr FP EN1() { return FromRaw(6553); }     // 0x7a73ab8 (1e-1)
    static constexpr FP EN2() { return FromRaw(655); }      // 0x7a73ac0 (1e-2)
    static constexpr FP EN3() { return FromRaw(65); }       // 0x7a73ac8 (1e-3)
    static constexpr FP EN4() { return FromRaw(6); }        // 0x7a73ad0 (1e-4)
    static constexpr FP Epsilon() { return FromRaw(65); }   // 0x7a73ad8 (same as EN3)

    // Names used by the rest of this codebase.
    static constexpr FP Zero() { return _0(); }
    static constexpr FP One() { return _1(); }
    static constexpr FP Half() { return _0_50(); }

    // --- Arithmetic (all wrap on overflow) ---
    friend constexpr FP operator-(FP a) { return FromRaw(WrapSub(0, a.raw_)); } // 0x7a73ae0

    // 0x7a73ae8
    friend constexpr FP operator+(FP a, FP b) { return FromRaw(WrapAdd(a.raw_, b.raw_)); }
    // 0x7a73af0
    friend constexpr FP operator+(FP a, std::int32_t b) {
        return FromRaw(WrapAdd(a.raw_, IntToRaw(b)));
    }
    // 0x7a73afc
    friend constexpr FP operator+(std::int32_t a, FP b) {
        return FromRaw(WrapAdd(IntToRaw(a), b.raw_));
    }

    // 0x7a73b08
    friend constexpr FP operator-(FP a, FP b) { return FromRaw(WrapSub(a.raw_, b.raw_)); }
    // 0x7a73b10
    friend constexpr FP operator-(FP a, std::int32_t b) {
        return FromRaw(WrapSub(a.raw_, IntToRaw(b)));
    }
    // 0x7a73b1c
    friend constexpr FP operator-(std::int32_t a, FP b) {
        return FromRaw(WrapSub(IntToRaw(a), b.raw_));
    }

    // 0x7a73b28
    friend constexpr FP operator*(FP a, FP b) { return FromRaw(MulRaw(a.raw_, b.raw_)); }
    // Scaling by an integer needs no shift.
    // 0x7a73b34
    friend constexpr FP operator*(FP a, std::int32_t b) { return FromRaw(WrapMul(a.raw_, b)); }
    // 0x7a73b40
    friend constexpr FP operator*(std::int32_t a, FP b) { return FromRaw(WrapMul(a, b.raw_)); }

    // 0x7a73b4c
    friend constexpr FP operator/(FP a, FP b) { return FromRaw(DivRaw(a.raw_, b.raw_)); }
    // 0x7a73b58
    friend constexpr FP operator/(FP a, std::int32_t b) {
        return FromRaw(DivideTruncating(a.raw_, b));
    }
    // int / FP: the integer is scaled twice (once to FP, once for the division).
    // 0x7a73b64
    friend constexpr FP operator/(std::int32_t a, FP b) {
        return FromRaw(DivideTruncating(static_cast<Raw>(a) << (2 * Precision), b.raw_));
    }

    // Remainder takes the sign of the dividend (like C# and C++ %).
    // 0x7a73b70
    friend constexpr FP operator%(FP a, FP b) { return FromRaw(Remainder(a.raw_, b.raw_)); }
    // 0x7a73b7c
    friend constexpr FP operator%(FP a, std::int32_t b) {
        return FromRaw(Remainder(a.raw_, IntToRaw(b)));
    }

    constexpr FP& operator+=(FP o) { return *this = *this + o; }
    constexpr FP& operator-=(FP o) { return *this = *this - o; }
    constexpr FP& operator*=(FP o) { return *this = *this * o; }
    constexpr FP& operator/=(FP o) { return *this = *this / o; }
    constexpr FP& operator%=(FP o) { return *this = *this % o; }

    // --- Comparisons ---
    // FP-FP: 0x7a73b8c (<), 0x7a73bb8 (<=), 0x7a73bd4 (>), 0x7a73c00 (>=),
    // 0x7a73c2c (==), 0x7a73c58 (!=). FP-int: 0x7a73b98, 0x7a73bc4, 0x7a73be0,
    // 0x7a73c0c, 0x7a73c38, 0x7a73c64. int-FP: 0x7a73ba8, 0x7a73bf0, 0x7a73c1c,
    // 0x7a73c48. The int is scaled to raw and compared exactly.
    friend constexpr bool operator==(FP a, FP b) = default;
    friend constexpr auto operator<=>(FP a, FP b) = default;
    friend constexpr bool operator==(FP a, std::int32_t b) { return a.raw_ == IntToRaw(b); }
    friend constexpr auto operator<=>(FP a, std::int32_t b) { return a.raw_ <=> IntToRaw(b); }

    // 0x7a741a0 FP.MulTruncate: same as operator* (truncating multiply).
    static constexpr FP MulTruncate(FP x, FP y) { return x * y; }

    // 0x7a73cf8 FP.CompareTo: -1, 0 or 1.
    constexpr std::int32_t CompareTo(FP other) const {
        return raw_ < other.raw_ ? -1 : (raw_ > other.raw_ ? 1 : 0);
    }
    // 0x7a73d00 FP.Equals(FP). (0x7a73d10 Equals(object) has no C++ counterpart.)
    constexpr bool Equals(FP other) const { return raw_ == other.raw_; }
    // 0x7a7106c FP.GetHashCode: the hash of the raw long, i.e. low ^ high word.
    constexpr std::int32_t GetHashCode() const {
        return static_cast<std::int32_t>(static_cast<std::uint32_t>(raw_) ^
                                         static_cast<std::uint32_t>(raw_ >> 32));
    }
    // 0x7a73d88 FP.ToString: the value as a float, printed with the invariant
    // culture's default ("G", 7 significant digits) float format.
    std::string ToString() const;

    // TODO(0x7a726d0, Photon.Deterministic.FP.Serialize): static
    // Serialize(void* ptr, IDeterministicFrameSerializer serializer) writes or
    // reads RawValue through serializer.Stream (BitStream.Serialize(long*)).
    // Needs IDeterministicFrameSerializer/BitStream.

    // --- Raw helpers, exposed so vector maths can apply the same rules ---
    static constexpr Raw MulRaw(Raw a, Raw b) {
        // Multiply in unsigned space so overflow wraps instead of being UB,
        // then shift arithmetically (well-defined since C++20).
        return WrapMul(a, b) >> Precision;
    }

    static constexpr Raw DivRaw(Raw a, Raw b) {
        return DivideTruncating(static_cast<Raw>(static_cast<std::uint64_t>(a) << Precision), b);
    }

    // Integer division with the original's (arm64) edge cases: x / 0 == 0 and
    // MinValue / -1 == MinValue. Otherwise truncates toward zero.
    static constexpr Raw DivideTruncating(Raw a, Raw b) {
        if (b == 0) {
            return 0;
        }
        if (b == -1) {
            return WrapSub(0, a);
        }
        return a / b;
    }

    // Matching remainder: x % 0 == x, x % -1 == 0.
    static constexpr Raw Remainder(Raw a, Raw b) {
        if (b == 0) {
            return a;
        }
        if (b == -1) {
            return 0;
        }
        return a % b;
    }

  private:
    static constexpr Raw IntToRaw(std::int32_t value) {
        return static_cast<Raw>(value) << Precision;
    }

    static constexpr Raw WrapAdd(Raw a, Raw b) {
        return static_cast<Raw>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
    }
    static constexpr Raw WrapSub(Raw a, Raw b) {
        return static_cast<Raw>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
    }
    static constexpr Raw WrapMul(Raw a, Raw b) {
        return static_cast<Raw>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
    }

    Raw raw_ = 0;
};

static_assert(sizeof(FP) == FP::SIZE, "FP must stay a plain 64-bit value (it is serialised raw)");

} // namespace Photon::Deterministic
