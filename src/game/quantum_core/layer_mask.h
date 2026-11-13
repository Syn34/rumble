// layer_mask.h - Quantum.LayerMask (quantum.core.dll): a 32-bit set of physics layers.
#pragma once

#include <cstdint>

namespace Quantum {

struct LayerMask {
    static constexpr int SIZE = 4;

    std::int32_t BitMask = 0;

    // 0x7d3dd34 (op_Implicit(int)) and 0x7d3dd30 (op_Implicit(LayerMask)): plain
    // conversions to and from the bit mask.
    static constexpr LayerMask FromInt(std::int32_t mask) { return LayerMask{mask}; }
    constexpr operator std::int32_t() const { return BitMask; }

    // 0x7d3dd38
    constexpr std::int32_t GetHashCode() const { return BitMask + 527; }

    friend constexpr bool operator==(LayerMask, LayerMask) = default;
};

} // namespace Quantum
