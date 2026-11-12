// fp.h - deterministic fixed-point number used by the gameplay simulation.
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
//   * multiply = full 64-bit product, then arithmetic shift right by 16
//     (wraps on overflow, no rounding)
//   * divide   = (a << 16) / b, truncating toward zero
//   * vector dot products shift each term separately before summing
#pragma once

#include <compare>
#include <cstdint>
#include <limits>

namespace Photon::Deterministic {

class FP {
public:
    using Raw = std::int64_t;

    static constexpr int FractionBits = 16;
    static constexpr Raw OneRaw = Raw{1} << FractionBits;

    constexpr FP() = default;

    // Construct from a raw fixed-point value. Use FromRaw at call sites so it's
    // obvious the integer is already scaled.
    static constexpr FP FromRaw(Raw raw) {
        FP v;
        v.raw_ = raw;
        return v;
    }

    static constexpr FP FromInt(std::int64_t value) { return FromRaw(value * OneRaw); }

    // Converts a float by truncating toward zero after scaling. Not deterministic
    // across platforms if the float itself came from a non-deterministic source,
    // hence the name. Out-of-range input saturates (the original threw).
    static FP FromFloatUnsafe(float value);

    constexpr Raw RawValue() const { return raw_; }
    float AsFloat() const { return static_cast<float>(raw_) / static_cast<float>(OneRaw); }
    double AsDouble() const { return static_cast<double>(raw_) / static_cast<double>(OneRaw); }
    constexpr std::int64_t AsInt() const { return raw_ >> FractionBits; }

    // Common constants. Values are the raw encodings used by the simulation.
    static constexpr FP Zero() { return FromRaw(0); }
    static constexpr FP One() { return FromRaw(OneRaw); }
    static constexpr FP Half() { return FromRaw(OneRaw / 2); }
    static constexpr FP MinValue() { return FromRaw(std::numeric_limits<Raw>::min()); }
    static constexpr FP MaxValue() { return FromRaw(std::numeric_limits<Raw>::max()); }
    // 1/1000, used throughout gameplay code as the "close enough" tolerance.
    static constexpr FP EN3() { return FromRaw(66); }
    static constexpr FP Pi() { return FromRaw(205887); }
    static constexpr FP PiTimes2() { return FromRaw(411774); }
    static constexpr FP Deg2Rad() { return FromRaw(1143); }
    static constexpr FP Rad2Deg() { return FromRaw(3755967); }

    friend constexpr FP operator+(FP a, FP b) { return FromRaw(WrapAdd(a.raw_, b.raw_)); }
    friend constexpr FP operator-(FP a, FP b) { return FromRaw(WrapSub(a.raw_, b.raw_)); }
    friend constexpr FP operator-(FP a) { return FromRaw(WrapSub(0, a.raw_)); }
    friend constexpr FP operator*(FP a, FP b) { return FromRaw(MulRaw(a.raw_, b.raw_)); }
    friend constexpr FP operator/(FP a, FP b) { return FromRaw(DivRaw(a.raw_, b.raw_)); }

    constexpr FP& operator+=(FP o) { return *this = *this + o; }
    constexpr FP& operator-=(FP o) { return *this = *this - o; }
    constexpr FP& operator*=(FP o) { return *this = *this * o; }
    constexpr FP& operator/=(FP o) { return *this = *this / o; }

    friend constexpr bool operator==(FP a, FP b) = default;
    friend constexpr auto operator<=>(FP a, FP b) = default;

    // Raw helpers, exposed so vector maths can apply the per-term shift rule.
    static constexpr Raw MulRaw(Raw a, Raw b) {
        // Multiply in unsigned space so overflow wraps instead of being UB,
        // then shift arithmetically (well-defined since C++20).
        const auto product = static_cast<Raw>(static_cast<std::uint64_t>(a) *
                                              static_cast<std::uint64_t>(b));
        return product >> FractionBits;
    }

    static constexpr Raw DivRaw(Raw a, Raw b) {
        const auto scaled = static_cast<Raw>(static_cast<std::uint64_t>(a) << FractionBits);
        return scaled / b;
    }

private:
    static constexpr Raw WrapAdd(Raw a, Raw b) {
        return static_cast<Raw>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
    }
    static constexpr Raw WrapSub(Raw a, Raw b) {
        return static_cast<Raw>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
    }

    Raw raw_ = 0;
};

} // namespace Photon::Deterministic
