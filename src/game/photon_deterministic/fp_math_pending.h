// fp_math_pending.h - FPMath functions the vector/quaternion code needs but fp_math.h
// does not provide yet.
//
// ======================= PENDING FPMath WORK =======================
// FPMath (sqrt/trig lookup tables) is being written separately. This header holds ONLY
// declarations of the functions the vector types call, with the original's signatures.
// When the real FPMath lands in fp_math.h:
//   * delete the declarations below (and fp_math_pending.cpp, which only has stubs that
//     abort), keeping just the #include of fp_math.h, or include fp_math.h directly;
//   * re-enable the tests guarded by RR_HAVE_FPMATH in tests/test_fp_vectors.cpp.
// Nothing here may be given a "plausible" placeholder implementation.
// ====================================================================
#pragma once

#include "fp.h"
#include "fp_math.h"

#include <cstdint>

namespace Photon::Deterministic {

// ExponentMantisaPair (sic; global namespace in the original): result of
// FPMath.GetSqrtExponentMantissa, sqrt(x) ~= Mantissa * 2^Exponent in a scaled form.
struct ExponentMantisaPair {
    std::int32_t Exponent;
    std::int32_t Mantissa;
};

namespace FPMath {

// 0x7a7499c Photon.Deterministic.FPMath.Sqrt (branches straight to SqrtRaw 0x7a71898).
FP Sqrt(FP value);
// 0x7a749a0 Photon.Deterministic.FPMath.GetSqrtExponentMantissa(ulong x). The vector
// Normalize methods call an identical out-of-line copy at 0x7a78284.
ExponentMantisaPair GetSqrtExponentMantissa(std::uint64_t x);
// 0x7a742fc Photon.Deterministic.FPMath.Sin
FP Sin(FP rad);
// 0x7a74458 Photon.Deterministic.FPMath.SinCos(rad, out sin, out cos)
void SinCos(FP rad, FP& sin, FP& cos);
// 0x7a745f8 Photon.Deterministic.FPMath.Asin
FP Asin(FP value);
// 0x7a71d9c Photon.Deterministic.FPMath.Acos
FP Acos(FP value);
// 0x7a7479c Photon.Deterministic.FPMath.Atan2(y, x)
FP Atan2(FP y, FP x);

} // namespace FPMath
} // namespace Photon::Deterministic
