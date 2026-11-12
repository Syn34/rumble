// fp_matrix2x2.h - Photon.Deterministic.FPMatrix2x2, a 2x2 fixed-point matrix.
//
// Field names are row/column (M10 = row 1, column 0); storage is column-major
// (M00, M10, M01, M11), as in the original.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_vector2.h"

#include <cstdint>
#include <initializer_list>
#include <string>

namespace Photon::Deterministic {

struct FPMatrix2x2 {
    FP M00;
    FP M10;
    FP M01;
    FP M11;

    // 0x7a70db4 get_Zero
    static constexpr FPMatrix2x2 Zero() { return {}; }

    friend constexpr bool operator==(const FPMatrix2x2& a, const FPMatrix2x2& b) = default;

    // 0x7a70d98 get_Determinant
    constexpr FP Determinant() const { return M00 * M11 - M01 * M10; }

    // 0x7a70dc0 - matrix * column vector.
    constexpr FPVector2 MultiplyVector(FPVector2 v) const {
        return {M00 * v.X + M01 * v.Y, M10 * v.X + M11 * v.Y};
    }

    // 0x7a71004
    constexpr std::int32_t GetHashCode() const {
        std::int32_t hash = detail::HashSeed;
        for (const FP value : {M00, M10, M01, M11}) {
            hash = detail::HashCombine(hash, detail::HashRaw(value));
        }
        return hash;
    }

    // 0x7a70dec - "((m00, m01), (m10, m11))", row by row, each printed as a float.
    std::string ToString() const;
};

} // namespace Photon::Deterministic
