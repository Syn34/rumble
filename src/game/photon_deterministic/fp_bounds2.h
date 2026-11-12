// fp_bounds2.h - Photon.Deterministic.FPBounds2, an axis-aligned 2D box stored as
// centre and half-size (extents), like Unity's Bounds.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_vector2.h"

#include <cstdint>

namespace Photon::Deterministic {

struct FPBounds2 {
    FPVector2 Center;
    FPVector2 Extents;

    constexpr FPBounds2() = default;

    // 0x7a741d4
    constexpr FPBounds2(FPVector2 center, FPVector2 extents) : Center(center), Extents(extents) {}

    friend constexpr bool operator==(const FPBounds2& a, const FPBounds2& b) = default;

    // 0x7a741ac get_Max
    constexpr FPVector2 Max() const { return Center + Extents; }
    // 0x7a741c0 get_Min
    constexpr FPVector2 Min() const { return Center - Extents; }

    // 0x7a741e0 - grows the total size by `amount` on each axis.
    constexpr void Expand(FP amount) {
        const FP halfAmount = amount * FP::Half();
        Extents = {Extents.X + halfAmount, Extents.Y + halfAmount};
    }

    // 0x7a741f8
    constexpr void SetMinMax(FPVector2 min, FPVector2 max) {
        Extents = (max - min) * FP::Half();
        Center = min + Extents;
    }

    // 0x7a74224 - grows the box to contain `point`.
    constexpr void Encapsulate(FPVector2 point) {
        SetMinMax(FPVector2::Min(Min(), point), FPVector2::Max(Max(), point));
    }

    // 0x7a74260
    constexpr std::int32_t GetHashCode() const {
        return detail::HashCombine(detail::HashCombine(detail::HashSeed, Center.GetHashCode()),
                                   Extents.GetHashCode());
    }
};

} // namespace Photon::Deterministic
