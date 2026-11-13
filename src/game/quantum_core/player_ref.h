// player_ref.h - Quantum.PlayerRef: a player slot of the session.
//
// Stored one-based so that the zero value is "no player"; converting to and from int
// gives the zero-based slot index.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct PlayerRef {
    static constexpr int SIZE = 4;

    std::int32_t _index = 0;

    // 0x7d3e05c
    static constexpr PlayerRef None() { return PlayerRef{}; }
    // 0x7d3e190 - Zero-based slot -> PlayerRef.
    static constexpr PlayerRef FromIndex(int index) {
        PlayerRef player;
        player._index = index + 1;
        return player;
    }
    // 0x7d3e188 - PlayerRef -> zero-based slot (-1 for None).
    constexpr int ToIndex() const { return _index - 1; }
    constexpr explicit operator int() const { return ToIndex(); }

    // 0x7d3e064
    constexpr bool IsValid() const { return _index > 0; }

    // 0x7d3e198 (==), 0x7d3e1a4 (!=); 0x7d3e074 is the boxed Equals(object).
    friend constexpr bool operator==(PlayerRef a, PlayerRef b) { return a._index == b._index; }
    constexpr bool Equals(PlayerRef other) const { return _index == other._index; }

    // 0x7d3e0ec
    constexpr std::int32_t GetHashCode() const { return _index; }

    // 0x7d3e0f4 - Debug text: the slot index, or a marker for None (wording is this
    // project's own).
    std::string ToString() const;

    // 0x7d3e1b0 - The raw (one-based) value as a 32-bit integer.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(PlayerRef) == PlayerRef::SIZE);

} // namespace Quantum
