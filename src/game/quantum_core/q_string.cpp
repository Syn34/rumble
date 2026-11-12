#include "q_string.h"

#include <algorithm>

namespace Quantum {

namespace {

// Code unit `index` of a stored string (bytes are UTF-16LE; unaligned access is avoided).
char16_t CodeUnit(const std::uint8_t* bytes, int index) {
    char16_t unit;
    std::memcpy(&unit, bytes + static_cast<std::ptrdiff_t>(index) * 2, sizeof(unit));
    return unit;
}

} // namespace

int QString::CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount,
                            std::u16string_view s) {
    const int stringLength = static_cast<int>(s.size());
    if (byteCount == 0) {
        return -stringLength;
    }
    if (stringLength == 0) {
        return byteCount;
    }

    const int length = byteCount >> 1;
    const int common = std::min(length, stringLength);
    for (int i = 0; i < common; ++i) {
        const char16_t a = CodeUnit(bytes, i);
        const char16_t b = s[static_cast<std::size_t>(i)];
        if (a != b) {
            return a < b ? -1 : 1;
        }
    }
    if (stringLength > length) {
        return -1;
    }
    return stringLength < length ? 1 : 0;
}

int QString::CompareOrdinal(const std::uint8_t* a, std::uint16_t aCount, const std::uint8_t* b,
                            std::uint16_t bCount) {
    const int aLength = aCount >> 1;
    const int bLength = bCount >> 1;
    const int common = std::min(aLength, bLength);
    for (int i = 0; i < common; ++i) {
        const char16_t x = CodeUnit(a, i);
        const char16_t y = CodeUnit(b, i);
        if (x != y) {
            return x < y ? -1 : 1;
        }
    }
    if (aLength < bLength) {
        return -1;
    }
    if (aLength > bLength) {
        return 1;
    }
    return aCount - bCount;
}

int QString::CompareOridinalNot4ByteAligned(const char16_t* a, int aa, const char16_t* b, int bb) {
    const int common = std::min(aa, bb);
    for (int i = 0; i < common; ++i) {
        if (a[i] != b[i]) {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    if (aa < bb) {
        return -1;
    }
    return aa > bb ? 1 : 0;
}

} // namespace Quantum
