// q_string.h - Quantum.QString: helpers shared by the fixed-capacity simulation strings.
//
// QString34/50/64/256 store a string inside frame memory as a 16-bit byte count followed
// by that many bytes of UTF-16 code units (little-endian, as on every supported target);
// the number is the payload capacity in bytes. All comparisons are ordinal, code unit by
// code unit. The helpers here work on any struct with that layout (QStringLayout).
//
// C# strings are passed as std::u16string_view; a null C# string is an empty view.
#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Quantum {

// Common layout of the QStringN types (QStringCommonLayout in the original).
template <class Q>
concept QStringLayout = requires(Q q) {
    { q.byteCount } -> std::convertible_to<std::uint16_t>;
    { q.bytes[0] } -> std::convertible_to<std::uint8_t>;
    std::size(q.bytes);
};

struct QString {
    // 0x7cef10c - True if the string's UTF-16 bytes fit in `maxByteCount`.
    static bool CanHold(std::u16string_view str, int maxByteCount) {
        return GetByteCount(str) <= maxByteCount;
    }

    // 0x7cef12c - Ordinal comparison of stored bytes with a string. Returns -1/0/1, except
    // that an empty side yields the raw length difference (-str.size() for no bytes,
    // byteCount for an empty string), as in the original.
    static int CompareOrdinal(const std::uint8_t* bytes, std::uint16_t byteCount,
                              std::u16string_view s);
    // 0x7cef234 - Ordinal comparison of two stored strings. For equal character counts the
    // result is the byte-count difference (0 unless a count is odd).
    static int CompareOrdinal(const std::uint8_t* a, std::uint16_t aCount, const std::uint8_t* b,
                              std::uint16_t bCount);
    // 0x7cef1dc - Ordinal comparison of two code unit arrays: -1, 0 or 1.
    static int CompareOridinalNot4ByteAligned(const char16_t* a, int aa, const char16_t* b, int bb);
    // 0x7cef29c
    static int GetByteCount(std::u16string_view str) { return static_cast<int>(str.size()) * 2; }
    // 0x7cef2ac - C# integer division.
    static int GetCharCount(int byteCount) { return byteCount / 2; }

    // 0x5118378 (instance for QString50)
    template <QStringLayout Q> static int CompareOrdinal(const Q& qstr, std::u16string_view str) {
        return CompareOrdinal(qstr.bytes, qstr.byteCount, str);
    }
    // 0x511840c (QString50)
    template <QStringLayout Q>
    static int CompareOrdinal(const Q& qstr, const std::uint8_t* bytes, std::uint16_t byteCount) {
        return CompareOrdinal(qstr.bytes, qstr.byteCount, bytes, byteCount);
    }

    // 0x5118004 (QString50) - Equality with a string, or with another QString (compared
    // from the other string's side, as the original asks the other object). Comparing with
    // anything else is false in the original; C++ callers simply have no such overload.
    template <QStringLayout Q> static bool AreEqual(const Q& qstr, std::u16string_view other) {
        return CompareOrdinal(qstr, other) == 0;
    }
    template <QStringLayout Q, QStringLayout Other>
    static bool AreEqual(const Q& qstr, const Other& other) {
        return CompareOrdinal(other, qstr.bytes, qstr.byteCount) == 0;
    }

    // 0x511851c (QString50) - Stores `str`; std::invalid_argument if it needs more than
    // `maxByteCount` bytes.
    template <QStringLayout Q>
    static void ConstructFrom(std::u16string_view str, int maxByteCount, Q& qstr) {
        if (!CanHold(str, maxByteCount)) {
            throw std::invalid_argument("QString: string too long for this QString type");
        }
        ConstructNoByteCountCheck(str, 0, maxByteCount, qstr);
    }

    // 0x5118828 (QString50) - Stores `str` without the capacity check. An empty string
    // zeroes the whole struct; otherwise only the count and the used bytes are written
    // (later bytes keep their old contents). characterCount and maxByteCount are unused, as
    // in the original. A string that would overrun the struct throws std::length_error
    // (the original does not check; the caller guarantees it fits).
    template <QStringLayout Q>
    static void ConstructNoByteCountCheck(std::u16string_view str, int /*characterCount*/,
                                          int /*maxByteCount*/, Q& qstr) {
        if (str.empty()) {
            qstr = Q{};
            return;
        }
        const int byteCount = GetByteCount(str);
        if (byteCount >= 0x10000) {
            throw std::overflow_error("QString: byte count does not fit 16 bits");
        }
        if (static_cast<std::size_t>(byteCount) > std::size(qstr.bytes)) {
            throw std::length_error("QString: string does not fit the struct");
        }
        qstr.byteCount = static_cast<std::uint16_t>(byteCount);
        std::memcpy(qstr.bytes, str.data(), static_cast<std::size_t>(byteCount));
    }

    // 0x5118b38 (QString50) - The stored string.
    template <QStringLayout Q> static std::u16string GetString(const Q& qstr) {
        std::u16string result(static_cast<std::size_t>(qstr.byteCount >> 1), u'\0');
        std::memcpy(result.data(), qstr.bytes, result.size() * sizeof(char16_t));
        return result;
    }
};

} // namespace Quantum
