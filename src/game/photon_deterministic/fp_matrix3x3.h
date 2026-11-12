// fp_matrix3x3.h - Photon.Deterministic.FPMatrix3x3, a 3x3 fixed-point matrix.
//
// Field names are row/column (M12 = row 1, column 2); storage is column-major
// (M00, M10, M20, M01, ...), as in the original. Products shift each term before
// summing, like the vector types.
#pragma once

#include "fp.h"
#include "fp_internal.h"
#include "fp_vector3.h"

#include <cstdint>
#include <initializer_list>
#include <string>

namespace Photon::Deterministic {

struct FPMatrix3x3 {
    static constexpr int SIZE = 72; // serialised size in bytes

    FP M00;
    FP M10;
    FP M20;
    FP M01;
    FP M11;
    FP M21;
    FP M02;
    FP M12;
    FP M22;

    // 0x7a71074 get_Zero
    static constexpr FPMatrix3x3 Zero() { return {}; }

    // 0x7a71088 - arguments listed row by row.
    static constexpr FPMatrix3x3 FromRows(FP m00, FP m01, FP m02, FP m10, FP m11, FP m12, FP m20,
                                          FP m21, FP m22) {
        return {m00, m10, m20, m01, m11, m21, m02, m12, m22};
    }

    // 0x7a710a4 - arguments listed column by column.
    static constexpr FPMatrix3x3 FromColumns(FP m00, FP m10, FP m20, FP m01, FP m11, FP m21, FP m02,
                                             FP m12, FP m22) {
        return {m00, m10, m20, m01, m11, m21, m02, m12, m22};
    }

    friend constexpr bool operator==(const FPMatrix3x3& a, const FPMatrix3x3& b) = default;

    // 0x7a710c0 get_Transposed
    constexpr FPMatrix3x3 Transposed() const {
        return FromRows(M00, M10, M20, M01, M11, M21, M02, M12, M22);
    }

    // 0x7a71564
    friend constexpr FPMatrix3x3 operator+(const FPMatrix3x3& a, const FPMatrix3x3& b) {
        return {a.M00 + b.M00, a.M10 + b.M10, a.M20 + b.M20, a.M01 + b.M01, a.M11 + b.M11,
                a.M21 + b.M21, a.M02 + b.M02, a.M12 + b.M12, a.M22 + b.M22};
    }

    // 0x7a715f8 - matrix product a * b.
    friend constexpr FPMatrix3x3 operator*(const FPMatrix3x3& a, const FPMatrix3x3& b) {
        return FromRows(a.M00 * b.M00 + a.M01 * b.M10 + a.M02 * b.M20,
                        a.M00 * b.M01 + a.M01 * b.M11 + a.M02 * b.M21,
                        a.M00 * b.M02 + a.M01 * b.M12 + a.M02 * b.M22,
                        a.M10 * b.M00 + a.M11 * b.M10 + a.M12 * b.M20,
                        a.M10 * b.M01 + a.M11 * b.M11 + a.M12 * b.M21,
                        a.M10 * b.M02 + a.M11 * b.M12 + a.M12 * b.M22,
                        a.M20 * b.M00 + a.M21 * b.M10 + a.M22 * b.M20,
                        a.M20 * b.M01 + a.M21 * b.M11 + a.M22 * b.M21,
                        a.M20 * b.M02 + a.M21 * b.M12 + a.M22 * b.M22);
    }

    // 0x7a71738 - matrix * column vector.
    friend constexpr FPVector3 operator*(const FPMatrix3x3& m, const FPVector3& vector) {
        return {m.M00 * vector.X + m.M01 * vector.Y + m.M02 * vector.Z,
                m.M10 * vector.X + m.M11 * vector.Y + m.M12 * vector.Z,
                m.M20 * vector.X + m.M21 * vector.Y + m.M22 * vector.Z};
    }

    // 0x7a71494 - fields hashed in storage order.
    constexpr std::int32_t GetHashCode() const {
        std::int32_t hash = detail::HashSeed;
        for (const FP value : {M00, M10, M20, M01, M11, M21, M02, M12, M22}) {
            hash = detail::HashCombine(hash, detail::HashRaw(value));
        }
        return hash;
    }

    // 0x7a710ec - "((m00, m01, m02), (m10, m11, m12), (m20, m21, m22))", row by row.
    std::string ToString() const;
};

static_assert(sizeof(FPMatrix3x3) == FPMatrix3x3::SIZE);

} // namespace Photon::Deterministic
