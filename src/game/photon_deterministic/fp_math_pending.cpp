// fp_math_pending.cpp - temporary link stubs for fp_math_pending.h. See that header.
//
// Every function here aborts: they exist only so the library links while FPMath is
// being written elsewhere. Delete this file when the real FPMath definitions land.
#include "fp_math_pending.h"

#include <cstdio>
#include <cstdlib>

namespace Photon::Deterministic::FPMath {

namespace {
[[noreturn]] void NotYetAvailable(const char* name) {
    std::fprintf(stderr, "FPMath.%s is not implemented yet (pending FPMath work)\n", name);
    std::abort();
}
} // namespace

// TODO(0x7a7499c, Photon.Deterministic.FPMath.Sqrt): provided by the FPMath work.
FP Sqrt(FP) {
    NotYetAvailable("Sqrt");
}

// TODO(0x7a749a0, Photon.Deterministic.FPMath.GetSqrtExponentMantissa): provided by the
// FPMath work.
ExponentMantisaPair GetSqrtExponentMantissa(std::uint64_t) {
    NotYetAvailable("GetSqrtExponentMantissa");
}

// TODO(0x7a742fc, Photon.Deterministic.FPMath.Sin): provided by the FPMath work.
FP Sin(FP) {
    NotYetAvailable("Sin");
}

// TODO(0x7a74458, Photon.Deterministic.FPMath.SinCos): provided by the FPMath work.
void SinCos(FP, FP&, FP&) {
    NotYetAvailable("SinCos");
}

// TODO(0x7a745f8, Photon.Deterministic.FPMath.Asin): provided by the FPMath work.
FP Asin(FP) {
    NotYetAvailable("Asin");
}

// TODO(0x7a71d9c, Photon.Deterministic.FPMath.Acos): provided by the FPMath work.
FP Acos(FP) {
    NotYetAvailable("Acos");
}

// TODO(0x7a7479c, Photon.Deterministic.FPMath.Atan2): provided by the FPMath work.
FP Atan2(FP, FP) {
    NotYetAvailable("Atan2");
}

} // namespace Photon::Deterministic::FPMath
