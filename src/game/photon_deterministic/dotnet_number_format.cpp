#include "dotnet_number_format.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace Photon::Deterministic::detail {

namespace {

// A decimal number as .NET's formatter holds it: value = 0.d1d2d3... * 10^scale.
struct DecimalNumber {
    bool negative = false;
    std::string digits; // significant digits, no trailing zeros; empty means zero
    int scale = 0;
};

constexpr int SinglePrecisionDigits = 7;

DecimalNumber ToDecimal(float value) {
    DecimalNumber number;
    const double v = static_cast<double>(value);
    if (v == 0.0) {
        return number;
    }
    number.negative = v < 0.0;

    // "%.6e" gives the 7 significant digits correctly rounded: "d.dddddde+XX".
    char text[64];
    std::snprintf(text, sizeof text, "%.*e", SinglePrecisionDigits - 1, std::fabs(v));
    number.digits.push_back(text[0]);
    const char* p = text + 2;
    while (*p != 'e') {
        number.digits.push_back(*p++);
    }
    number.scale = std::atoi(p + 1) + 1;
    while (!number.digits.empty() && number.digits.back() == '0') {
        number.digits.pop_back();
    }
    if (number.digits.empty()) {
        number = DecimalNumber{};
    }
    return number;
}

// Keeps the first `position` digits, rounding half up on the next one; a number that
// rounds to nothing becomes positive zero (so -0.04 formats as "0.0", not "-0.0").
void RoundTo(DecimalNumber& number, int position) {
    std::string& d = number.digits;
    int kept = 0;
    while (kept < position && kept < static_cast<int>(d.size())) {
        ++kept;
    }
    if (kept == position && kept < static_cast<int>(d.size()) && d[kept] >= '5') {
        while (kept > 0 && d[kept - 1] == '9') {
            --kept;
        }
        if (kept > 0) {
            ++d[kept - 1];
        } else {
            ++number.scale;
            d[0] = '1';
            kept = 1;
        }
    } else {
        while (kept > 0 && d[kept - 1] == '0') {
            --kept;
        }
    }
    d.resize(static_cast<std::size_t>(kept > 0 ? kept : 0));
    if (kept <= 0) {
        number.scale = 0;
        number.negative = false;
    }
}

} // namespace

std::string FormatSingle(float value) {
    DecimalNumber number = ToDecimal(value);
    RoundTo(number, SinglePrecisionDigits);

    std::string out;
    if (number.negative) {
        out += '-';
    }

    // Scientific notation once the decimal point would sit outside the 7 digits, or
    // more than three zeros after it.
    int pointPosition = number.scale;
    const bool scientific = pointPosition > SinglePrecisionDigits || pointPosition < -3;
    if (scientific) {
        pointPosition = 1;
    }

    std::size_t next = 0;
    if (pointPosition > 0) {
        for (int i = 0; i < pointPosition; ++i) {
            out += next < number.digits.size() ? number.digits[next++] : '0';
        }
    } else {
        out += '0';
    }
    if (next < number.digits.size() || pointPosition < 0) {
        out += '.';
        for (int i = pointPosition; i < 0; ++i) {
            out += '0';
        }
        out.append(number.digits, next, std::string::npos);
    }

    if (scientific) {
        const int exponent = number.scale - 1;
        char text[16];
        std::snprintf(text, sizeof text, "E%c%02d", exponent < 0 ? '-' : '+', std::abs(exponent));
        out += text;
    }
    return out;
}

std::string FormatSingleFixed(float value, int decimals) {
    DecimalNumber number = ToDecimal(value);
    RoundTo(number, number.scale + decimals);

    std::string out;
    if (number.negative) {
        out += '-';
    }

    std::size_t next = 0;
    if (number.scale > 0) {
        for (int i = 0; i < number.scale; ++i) {
            out += next < number.digits.size() ? number.digits[next++] : '0';
        }
    } else {
        out += '0';
    }
    if (decimals > 0) {
        out += '.';
        int remaining = decimals;
        for (int i = number.scale; i < 0 && remaining > 0; ++i, --remaining) {
            out += '0';
        }
        for (; remaining > 0; --remaining) {
            out += next < number.digits.size() ? number.digits[next++] : '0';
        }
    }
    return out;
}

} // namespace Photon::Deterministic::detail
