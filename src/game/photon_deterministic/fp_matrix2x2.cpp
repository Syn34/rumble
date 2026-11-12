#include "fp_matrix2x2.h"

#include "dotnet_number_format.h"

namespace Photon::Deterministic {

// 0x7a70dec
std::string FPMatrix2x2::ToString() const {
    using detail::FormatSingle;
    return "((" + FormatSingle(M00.AsFloat()) + ", " + FormatSingle(M01.AsFloat()) + "), (" +
           FormatSingle(M10.AsFloat()) + ", " + FormatSingle(M11.AsFloat()) + "))";
}

} // namespace Photon::Deterministic
