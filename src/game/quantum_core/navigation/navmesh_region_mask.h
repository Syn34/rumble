// navmesh_region_mask.h - Quantum.NavMeshRegionMask (quantum.core.dll).
//
// Navmesh regions are areas that can be toggled at runtime (doors, bridges...). Each
// triangle, border and link carries a 64-bit mask of the regions it belongs to; the
// main area (always walkable) has an empty mask. An agent's mask lists the regions it
// may walk; an element is usable when its regions are a subset of the agent's.
#pragma once

#include <cstdint>
#include <string>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum {

class ByteStream;

struct NavMeshRegionMask {
    static constexpr int SIZE = 8;

    std::uint64_t _regions = 0;

    constexpr NavMeshRegionMask() = default;
    // 0x7d30d34
    constexpr explicit NavMeshRegionMask(std::uint64_t regions) : _regions(regions) {}

    // 0x7d30c70 - every region enabled.
    static constexpr NavMeshRegionMask Default() { return NavMeshRegionMask(~std::uint64_t{0}); }

    // 0x7d30c78 - intersection.
    static constexpr NavMeshRegionMask Combine(NavMeshRegionMask a, NavMeshRegionMask b) {
        return NavMeshRegionMask(a._regions & b._regions);
    }

    // 0x7d30d3c
    constexpr bool HasValidRegions() const { return _regions != 0; }
    // 0x7d30d4c - no region bits: the always-walkable main area.
    constexpr bool IsMainArea() const { return _regions == 0; }

    // 0x7d30d5c
    constexpr void ToggleRegion(int region, bool enabled) {
        const std::uint64_t bit = std::uint64_t{1} << (region & 63);
        _regions = enabled ? (_regions | bit) : (_regions & ~bit);
    }

    // 0x7d30d80
    constexpr bool IsRegionEnabled(int region) const { return ((_regions >> (region & 63)) & 1) != 0; }

    // 0x7d30d90 - every region of this mask is also in `other`.
    constexpr bool IsSubset(NavMeshRegionMask other) const { return (_regions & ~other._regions) == 0; }

    // 0x7d30da0 - note: "clear" re-enables every region.
    constexpr void Clear() { _regions = ~std::uint64_t{0}; }

    // 0x7d30dac (Equals(NavMeshRegionMask)); 0x7d30dbc Equals(object) boxes and compares.
    friend constexpr bool operator==(NavMeshRegionMask, NavMeshRegionMask) = default;

    // 0x7d30e2c - UInt64 hash plus a constant from the generated code.
    std::int32_t GetHashCode() const;

    // 0x7d30e4c - the mask as a decimal number.
    std::string ToString() const;

    // 0x7d30c80 - frame serialisation: the 64-bit mask through the BitStream.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer);

    // 0x7d30e54 - asset data serialisation (ulong).
    void Serialize(ByteStream& stream, bool write);
};
static_assert(sizeof(NavMeshRegionMask) == NavMeshRegionMask::SIZE);

} // namespace Quantum
