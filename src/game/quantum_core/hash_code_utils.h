// hash_code_utils.h - Quantum.HashCodeUtils: hash helpers used by generated code.
#pragma once

#include <cstddef>
#include <cstdint>

namespace Quantum::HashCodeUtils {

// 0x7d3d354 - h1 * 33 ^ h2.
constexpr std::int32_t CombineHashCodes(std::int32_t h1, std::int32_t h2) {
    return static_cast<std::int32_t>((static_cast<std::uint32_t>(h1) * 33u) ^
                                     static_cast<std::uint32_t>(h2));
}

// 0x7d3d360
constexpr std::int32_t CombineHashCodes(std::int32_t h1, std::int32_t h2, std::int32_t h3) {
    return CombineHashCodes(CombineHashCodes(h1, h2), h3);
}

// 0x7d3d374 - Low word xor high word (same as Int64.GetHashCode).
constexpr std::int32_t UInt64HashCode(std::uint64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value) ^
                                     static_cast<std::uint32_t>(value >> 32));
}

// GetArrayHashCode<T> (generic; instances e.g. 0x4f44ec0 for int, 0x4f44f30 for ulong):
// h = seed, then h = h * 31 + element hash for each element. `hashOf` returns the
// element's .NET hash.
template <class T, class HashOf>
std::int32_t GetArrayHashCode(const T* items, int length, HashOf hashOf, std::int32_t seed = 0) {
    std::uint32_t hash = static_cast<std::uint32_t>(seed);
    for (int i = 0; i < length; ++i) {
        hash = hash * 31u + static_cast<std::uint32_t>(hashOf(items[i]));
    }
    return static_cast<std::int32_t>(hash);
}

} // namespace Quantum::HashCodeUtils
