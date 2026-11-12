#include "test_framework.h"

#include "rr/core/fp.h"
#include "rr/core/fp_math.h"
#include "rr/core/fp_vector.h"

using rr::det::FP;
using rr::det::FPVector3;
namespace FPMath = rr::det::FPMath;

TEST_CASE("FP uses 16 fractional bits") {
    CHECK(FP::One().RawValue() == 65536);
    CHECK(FP::FromInt(3).RawValue() == 3 * 65536);
    CHECK(FP::FromInt(-2).AsInt() == -2);
}

TEST_CASE("FP multiply and divide truncate like the simulation expects") {
    CHECK(FP::FromInt(3) * FP::FromInt(4) == FP::FromInt(12));
    CHECK(FP::FromInt(1) / FP::FromInt(4) == FP::FromRaw(16384));
    // 1/3 truncates toward zero in raw space
    CHECK((FP::One() / FP::FromInt(3)).RawValue() == 21845);
    CHECK((-FP::One() / FP::FromInt(3)).RawValue() == -21845);
    // multiply shifts arithmetically (floors toward negative infinity)
    CHECK((FP::FromRaw(-1) * FP::Half()).RawValue() == -1);
}

TEST_CASE("FP float conversion truncates and saturates") {
    CHECK(FP::FromFloatUnsafe(1.5f) == FP::FromRaw(98304));
    CHECK(FP::FromFloatUnsafe(-0.5f) == FP::FromRaw(-32768));
    CHECK(FP::FromFloatUnsafe(1e30f) == FP::MaxValue());
    CHECK(FP::FromFloatUnsafe(-1e30f) == FP::MinValue());
}

TEST_CASE("FPMath clamp helpers") {
    CHECK(FPMath::Clamp01(FP::FromInt(2)) == FP::One());
    CHECK(FPMath::Clamp01(FP::FromInt(-2)) == FP::Zero());
    CHECK(FPMath::Clamp01(FP::Half()) == FP::Half());
    CHECK(FPMath::Abs(FP::FromInt(-5)) == FP::FromInt(5));
}

TEST_CASE("FPVector3 dot shifts each term separately") {
    // Each component product of raw 1 * raw 1 shifts to 0 on its own, so the
    // sum is 0 rather than (3 >> 16).
    const FPVector3 tiny{FP::FromRaw(1), FP::FromRaw(1), FP::FromRaw(1)};
    CHECK(FPVector3::Dot(tiny, tiny) == FP::Zero());
    const FPVector3 v{FP::FromInt(1), FP::FromInt(2), FP::FromInt(3)};
    CHECK(v.SqrMagnitude() == FP::FromInt(14));
}
