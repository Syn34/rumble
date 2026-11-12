#include "fp_matrix3x3.h"

#include "dotnet_number_format.h"

namespace Photon::Deterministic {

// 0x7a710ec
std::string FPMatrix3x3::ToString() const {
    using detail::FormatSingle;
    const auto row = [](FP a, FP b, FP c) {
        return "(" + FormatSingle(a.AsFloat()) + ", " + FormatSingle(b.AsFloat()) + ", " +
               FormatSingle(c.AsFloat()) + ")";
    };
    return "(" + row(M00, M01, M02) + ", " + row(M10, M11, M12) + ", " + row(M20, M21, M22) + ")";
}

} // namespace Photon::Deterministic
