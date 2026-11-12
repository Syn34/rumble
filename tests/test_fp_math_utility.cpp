#include "test_framework.h"

#include "quantum_code/fp_math_utility.h"

using Quantum::FP;
using Quantum::FPMathUtility;
using Quantum::FPVector2;
using Quantum::FPVector3;

namespace {
FP F(int v) { return FP::FromInt(v); }
} // namespace

TEST_CASE("FPMathUtility::Contains") {
    CHECK(FPMathUtility::Contains(0b1010, 0b0010));
    CHECK(!FPMathUtility::Contains(0b1010, 0b0101));
}

TEST_CASE("FPMathUtility::EqualWithEpsilon") {
    CHECK(FPMathUtility::EqualWithEpsilon(F(1), F(1) + FP::FromRaw(65)));
    CHECK(!FPMathUtility::EqualWithEpsilon(F(1), F(1) + FP::FromRaw(66)));
    const FPVector3 a{F(1), F(2), F(3)};
    CHECK(FPMathUtility::EqualWithEpsilon(a, a));
    CHECK(!FPMathUtility::EqualWithEpsilon(a, FPVector3{F(1), F(2), F(4)}));
}

TEST_CASE("FPMathUtility angle conversion") {
    // Deg2Rad is stored to 16 bits, so 180 degrees lands slightly below Pi.
    CHECK(FPMathUtility::DegreesToRadians(F(180)).RawValue() == 180 * 1143);
    CHECK(FPMathUtility::RadiansToDegrees(F(1)) == FP::Rad2Deg());
    CHECK(FPMathUtility::RadiansToDegrees(F(0)) == F(0));
}

TEST_CASE("FPMathUtility::Map clamps and interpolates") {
    CHECK(FPMathUtility::Map(F(0), F(10), F(100), F(200), F(-5)) == F(100));
    CHECK(FPMathUtility::Map(F(0), F(10), F(100), F(200), F(15)) == F(200));
    CHECK(FPMathUtility::Map(F(0), F(10), F(100), F(200), F(5)) == F(150));
}

TEST_CASE("FPMathUtility byte quantisation round-trips the ends exactly") {
    CHECK(FPMathUtility::QuantizeFP01ToByte(FP::Zero()) == 0);
    CHECK(FPMathUtility::QuantizeFP01ToByte(FP::One()) == 255);
    CHECK(FPMathUtility::QuantizeFP01ToByte(F(5)) == 255);
    CHECK(FPMathUtility::QuantizeFP01ToByte(FP::Half()) == 127);
    CHECK(FPMathUtility::QuantizeFPToByte(F(20), F(10), F(20)) == 255);
    CHECK(FPMathUtility::UnQuantizeByteToFP(0, F(10), F(20)) == F(10));
    CHECK(FPMathUtility::UnQuantizeByteToFP(255, F(10), F(20)) == F(20));
    for (int b = 0; b < 256; ++b) {
        const auto q = static_cast<std::uint8_t>(b);
        const FP v = FPMathUtility::UnQuantizeByteToFP(q, F(-4), F(4));
        const auto back = FPMathUtility::QuantizeFPToByte(v, F(-4), F(4));
        CHECK(back == q || back + 1 == q); // truncation may lose one step
    }
}

TEST_CASE("FPMathUtility component helpers") {
    const FPVector3 a{F(1), F(5), F(-3)};
    const FPVector3 b{F(2), F(4), F(-1)};
    CHECK(FPMathUtility::ComponentMax(a, b) == (FPVector3{F(2), F(5), F(-1)}));
    CHECK(FPMathUtility::GetMaxComponent(a) == F(5));
}

TEST_CASE("FPMathUtility segment queries") {
    const FPVector3 start{F(0), F(0), F(0)};
    const FPVector3 end{F(10), F(0), F(0)};
    CHECK(FPMathUtility::ClosestPointOnLine({F(5), F(3), F(0)}, start, end) ==
          (FPVector3{F(5), F(0), F(0)}));
    CHECK(FPMathUtility::ClosestPointOnLine({F(-5), F(3), F(0)}, start, end) == start);
    CHECK(FPMathUtility::ClosestPointOnLine({F(1), F(1), F(1)}, start, start) == start);

    CHECK(FPMathUtility::DistanceSqrToSegment({F(5), F(3)}, {F(0), F(0)}, {F(10), F(0)}) == F(9));
    CHECK(FPMathUtility::DistanceSqrToSegment({F(13), F(4)}, {F(0), F(0)}, {F(10), F(0)}) == F(25));
    CHECK(FPMathUtility::DistanceSqrToSegment({F(3), F(4)}, {F(0), F(0)}, {F(0), F(0)}) == F(25));
}
