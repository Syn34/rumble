// fp.cpp - Photon.Deterministic.FP: conversions from float and text, ToString.
#include "fp.h"

#include "bit_stream.h"
#include "deterministic_frame_serializer.h"

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace Photon::Deterministic {

namespace {

// Whitespace as .NET's Int64.Parse and String.Trim treat it (ASCII subset).
bool IsWhiteSpace(char c) {
    return c == ' ' || (c >= '\t' && c <= '\r');
}

std::string_view TrimWhiteSpace(std::string_view text) {
    while (!text.empty() && IsWhiteSpace(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && IsWhiteSpace(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

// Behaves like .NET Int64.Parse(string) with the default NumberStyles.Integer:
// optional surrounding whitespace, an optional leading sign, then decimal
// digits. Throws std::invalid_argument for anything else (FormatException) and
// std::overflow_error when the value doesn't fit (OverflowException).
std::int64_t ParseInt64(std::string_view text) {
    text = TrimWhiteSpace(text);
    bool negative = false;
    if (!text.empty() && (text.front() == '-' || text.front() == '+')) {
        negative = text.front() == '-';
        text.remove_prefix(1);
    }
    if (text.empty()) {
        throw std::invalid_argument("Input string was not in a correct format.");
    }

    // Accumulate the magnitude as unsigned so -2^63 parses.
    constexpr std::uint64_t limitPositive = std::numeric_limits<std::int64_t>::max();
    const std::uint64_t limit = negative ? limitPositive + 1 : limitPositive;
    std::uint64_t magnitude = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            throw std::invalid_argument("Input string was not in a correct format.");
        }
        const auto digit = static_cast<std::uint64_t>(c - '0');
        if (magnitude > (limit - digit) / 10) {
            throw std::overflow_error("Value was either too large or too small for an Int64.");
        }
        magnitude = magnitude * 10 + digit;
    }
    return negative ? static_cast<std::int64_t>(0 - magnitude)
                    : static_cast<std::int64_t>(magnitude);
}

// Splits on '.', dropping empty pieces (String.Split with RemoveEmptyEntries).
std::vector<std::string_view> SplitOnDots(std::string_view text) {
    std::vector<std::string_view> parts;
    while (!text.empty()) {
        const auto dot = text.find('.');
        const auto piece = text.substr(0, dot);
        if (!piece.empty()) {
            parts.push_back(piece);
        }
        if (dot == std::string_view::npos) {
            break;
        }
        text.remove_prefix(dot + 1);
    }
    return parts;
}

} // namespace

// 0x7a73e04
FP FP::FromFloat_UNSAFE(float value) {
    const float scaled = value * static_cast<float>(RAW_ONE);

    // 2^63 is exactly representable as a float.
    constexpr float twoPow63 = 9223372036854775808.0f;
    if (scaled > twoPow63) {
        throw std::overflow_error("Arithmetic operation resulted in an overflow.");
    }
    // The remaining out-of-range cases were not checked by the original; these
    // are the results its arm64 float-to-integer conversion gave.
    if (std::isnan(scaled)) {
        return Zero();
    }
    if (scaled == twoPow63) {
        return MaxValue();
    }
    if (scaled < -twoPow63) {
        return MinValue();
    }
    return FromRaw(static_cast<Raw>(scaled)); // truncates toward zero
}

// 0x7a74188
FP::Raw FP::ParseInteger(std::string_view format) {
    return static_cast<Raw>(static_cast<std::uint64_t>(ParseInt64(format)) << Precision);
}

// 0x7a74070
FP::Raw FP::ParseFractions(std::string_view format) {
    if (format.empty()) {
        return 0;
    }
    // Digits beyond the 14th can't change a 16-bit fraction (and wouldn't fit
    // once scaled), so they are dropped.
    constexpr std::size_t maxDigits = 14;
    if (format.size() > maxDigits) {
        format = format.substr(0, maxDigits);
    }
    Raw divisor = 1;
    for (std::size_t i = 0; i < format.size(); ++i) {
        divisor *= 10;
    }
    // digits / 10^n scaled to 16 bits, rounded half up.
    const auto scaled =
        static_cast<Raw>(static_cast<std::uint64_t>(ParseInt64(format)) << Precision);
    return DivideTruncating(scaled + divisor / 2, divisor);
}

// 0x7a73e84
FP FP::FromString(std::string_view value) {
    value = TrimWhiteSpace(value);
    if (value.empty()) {
        return Zero();
    }

    // Note: the original remembers the first character *before* stripping the
    // minus sign and later uses it to recognise a leading '.', so "-.5" parses
    // its digits as an integer (giving -5) rather than as a fraction. Kept for
    // fidelity.
    const char first = value.front();
    const bool negative = first == '-';
    if (negative) {
        value.remove_prefix(1);
    }

    const std::vector<std::string_view> parts = SplitOnDots(value);
    Raw result = 0;
    if (parts.size() == 2) {
        const Raw integer = ParseInteger(parts[0]);
        const Raw fraction = ParseFractions(parts[1]);
        // The original adds these with overflow checking.
        const bool overflows =
            (fraction > 0 && integer > std::numeric_limits<Raw>::max() - fraction) ||
            (fraction < 0 && integer < std::numeric_limits<Raw>::min() - fraction);
        if (overflows) {
            throw std::overflow_error("Arithmetic operation resulted in an overflow.");
        }
        result = integer + fraction;
    } else if (parts.size() == 1) {
        result = first == '.' ? ParseFractions(parts[0]) : ParseInteger(parts[0]);
    } else {
        throw std::invalid_argument(std::string(value));
    }

    return FromRaw(negative ? static_cast<Raw>(0 - static_cast<std::uint64_t>(result)) : result);
}

// 0x7a73d88
std::string FP::ToString() const {
    // .NET's float.ToString() on this runtime is the "G" format with 7
    // significant digits: fixed notation for exponents -5 < e < 7, otherwise
    // d.ddddddE+xx with at least two exponent digits and trailing zeros
    // removed - which is what printf's %.7G produces. Display only; never used
    // by the simulation.
    // Uncertainty: .NET rounds to 7 digits via its own routine; exact ties in
    // the 8th digit could in theory round differently from printf.
    char buffer[32];
    std::snprintf(buffer, sizeof buffer, "%.7G", static_cast<double>(AsFloat()));
    return buffer;
}

// 0x7a726d0
void FP::Serialize(void* ptr, IDeterministicFrameSerializer& serializer) {
    serializer.Stream().Serialize(&static_cast<FP*>(ptr)->raw_);
}

} // namespace Photon::Deterministic
