// Tests for Photon.Deterministic.FPVector2 / FPVector3. Expected values are worked out by
// hand from the fixed-point rules (16 fractional bits, each product shifted on its own).
#include "test_framework.h"

#include "photon_deterministic/fp_vector.h"

#include <stdexcept>
#include <vector>

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

namespace {
FP F(int v) {
    return FP::FromInt(v);
}
FP R(FP::Raw raw) {
    return FP::FromRaw(raw);
}
FPVector2 V2(int x, int y) {
    return {F(x), F(y)};
}
FPVector3 V3(int x, int y, int z) {
    return {F(x), F(y), F(z)};
}

// A counter-clockwise 2x2 square with a corner at the origin.
std::vector<FPVector2> Square() {
    return {V2(0, 0), V2(2, 0), V2(2, 2), V2(0, 2)};
}
} // namespace

// --- FPVector2 ---------------------------------------------------------------------

TEST_CASE("FPVector2 constants and constructors") {
    CHECK(FPVector2::Zero() == V2(0, 0));
    CHECK(FPVector2::Right() == V2(1, 0));
    CHECK(FPVector2::Left() == V2(-1, 0));
    CHECK(FPVector2::Up() == V2(0, 1));
    CHECK(FPVector2::Down() == V2(0, -1));
    CHECK(FPVector2(3, -4) == V2(3, -4)); // int constructor scales to fixed point
    CHECK(FPVector2(3, -4).X.RawValue() == 3 * 65536);
}

TEST_CASE("FPVector2 operators") {
    CHECK(V2(1, 2) + V2(3, 5) == V2(4, 7));
    CHECK(V2(1, 2) - V2(3, 5) == V2(-2, -3));
    CHECK(-V2(1, -2) == V2(-1, 2));
    CHECK(V2(1, 2) * F(3) == V2(3, 6));
    CHECK(F(3) * V2(1, 2) == V2(3, 6));
    CHECK(V2(1, 2) != V2(1, 3));
    // Multiplying by an int scales the raw value: raw 3 * 2 = raw 6, no shift.
    const FPVector2 tiny{R(3), R(-5)};
    CHECK(tiny * 2 == (FPVector2{R(6), R(-10)}));
}

TEST_CASE("FPVector2::Dot shifts each product separately") {
    CHECK(FPVector2::Dot(V2(1, 2), V2(3, 4)) == F(11));
    // raw 1 * 0.5 = raw 0.5 per term, truncated to 0 each; a combined shift would give 1.
    const FPVector2 a{R(1), R(1)};
    const FPVector2 b{FP::Half(), FP::Half()};
    CHECK(FPVector2::Dot(a, b) == F(0));
    CHECK(V2(3, 4).SqrMagnitude() == F(25));
    CHECK(FPVector2::DistanceSquared(V2(1, 2), V2(4, 6)) == F(25));
}

TEST_CASE("FPVector2 cross products") {
    CHECK(FPVector2::Cross(V2(1, 0), V2(0, 1)) == F(1));
    CHECK(FPVector2::Cross(V2(0, 1), V2(1, 0)) == F(-1));
    CHECK(FPVector2::CrossRaw(V2(2, 3), V2(4, 5)) == F(2 * 5 - 3 * 4));
    CHECK(FPVector2::Determinant(V2(2, 3), V2(4, 5)) == F(-2));
}

TEST_CASE("FPVector2::Rotate with sine and cosine") {
    // 90 degrees counter-clockwise: sin = 1, cos = 0.
    CHECK(FPVector2::Rotate(V2(1, 0), F(1), F(0)) == V2(0, 1));
    CHECK(FPVector2::Rotate(V2(0, 1), F(1), F(0)) == V2(-1, 0));
    // 180 degrees.
    CHECK(FPVector2::Rotate(V2(2, 3), F(0), F(-1)) == V2(-2, -3));
}

TEST_CASE("FPVector2::Reflect") {
    CHECK(FPVector2::Reflect(V2(1, -1), V2(0, 1)) == V2(1, 1));
    CHECK(FPVector2::Reflect(V2(3, 0), V2(-1, 0)) == V2(-3, 0));
    // Each dot term is doubled before its shift: (raw 1 * 1.5) >> 15 = raw 3, whereas
    // 2 * Dot would give raw 2. X = 1 - (3 * 1.5) = raw 1 - raw 4 = raw -3.
    const FPVector2 reflected = FPVector2::Reflect({R(1), F(0)}, {R(0x18000), F(0)});
    CHECK(reflected.X == R(-3));
}

TEST_CASE("FPVector2 Clamp/Lerp/Min/Max") {
    CHECK(FPVector2::Clamp(V2(-5, 5), V2(0, 0), V2(2, 2)) == V2(0, 2));
    CHECK(FPVector2::Clamp(V2(1, 1), V2(0, 0), V2(2, 2)) == V2(1, 1));
    CHECK(FPVector2::Lerp(V2(0, 0), V2(4, -2), FP::Half()) == V2(2, -1));
    CHECK(FPVector2::Lerp(V2(0, 0), V2(4, -2), F(2)) == V2(4, -2)); // t clamped to 1
    CHECK(FPVector2::Lerp(V2(1, 1), V2(4, -2), F(-1)) == V2(1, 1)); // t clamped to 0
    CHECK(FPVector2::Max(V2(1, 5), V2(3, 2)) == V2(3, 5));
    CHECK(FPVector2::Min(V2(1, 5), V2(3, 2)) == V2(1, 2));
}

TEST_CASE("FPVector2 left/right tests") {
    CHECK(FPVector2::CalculateLeft(V2(1, 2)) == V2(-2, 1));
    CHECK(V2(0, 1).IsLeftOf(V2(1, 0)));
    CHECK(!V2(0, -1).IsLeftOf(V2(1, 0)));
    CHECK(!V2(2, 0).IsLeftOf(V2(1, 0))); // parallel is not "left"
}

TEST_CASE("FPVector2 swizzles") {
    CHECK(V2(1, 2).XOY() == V3(1, 0, 2));
    CHECK(V2(1, 2).XYO() == V3(1, 2, 0));
    CHECK(V2(1, 2).XYY() == V3(1, 2, 2));
}

TEST_CASE("FPVector2 GetHashCode matches the original formula") {
    // 17*31*31 + 31*hash(raw 65536) + hash(raw 131072), Int64 hash = lo ^ hi.
    CHECK(V2(1, 2).GetHashCode() == 2179025);
    // raw -65536 = 0xFFFFFFFF'FFFF0000 -> lo ^ hi = 0x0000FFFF.
    CHECK(V2(-1, 0).GetHashCode() == 2047922);
    // High words matter: hi(5 << 40) = 1280, hi(-(14 << 32)) = -14.
    CHECK((FPVector2{R(FP::Raw{5} << 40), R(-(FP::Raw{7} << 33))}.GetHashCode() == 56003));
    CHECK(V2(1, 2).Equals(V2(1, 2)));
}

TEST_CASE("FPVector2::ToString") {
    CHECK(V2(1, 2).ToString() == "(1, 2)");
    CHECK((FPVector2{F(-1) / F(2), F(5) / F(2)}).ToString() == "(-0.5, 2.5)");
}

TEST_CASE("FPVector2 polygon winding") {
    std::vector<FPVector2> square = Square();
    CHECK(!FPVector2::IsClockWise(square));
    FPVector2::FlipWindingOrder(square);
    CHECK(square.front() == V2(0, 2));
    CHECK(FPVector2::IsClockWise(square));
    FPVector2::MakeCounterClockWise(square);
    CHECK(square == Square());
    FPVector2::MakeCounterClockWise(square); // already counter-clockwise: unchanged
    CHECK(square == Square());

    bool threw = false;
    try {
        const std::vector<FPVector2> line{V2(0, 0), V2(1, 0)};
        (void)FPVector2::IsClockWise(line);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("FPVector2 polygon area and centroid") {
    CHECK(FPVector2::CalculatePolygonArea(Square()) == F(4));
    std::vector<FPVector2> clockwise = Square();
    FPVector2::FlipWindingOrder(clockwise);
    CHECK(FPVector2::CalculatePolygonArea(clockwise) == F(-4));
    CHECK(FPVector2::CalculatePolygonArea({}) == F(0));
    // Twice-area of raw -3 halves toward zero to raw -1 (not -2).
    const std::vector<FPVector2> sliver{V2(0, 0), FPVector2{F(0), R(3)}, V2(1, 0)};
    CHECK(FPVector2::CalculatePolygonArea(sliver) == R(-1));

    CHECK(FPVector2::CalculatePolygonCentroid(Square()) == V2(1, 1));
    const std::vector<FPVector2> triangle{V2(0, 0), V2(3, 0), V2(0, 3)};
    CHECK(FPVector2::CalculatePolygonCentroid(triangle) == V2(1, 1));

    bool threw = false;
    try {
        const std::vector<FPVector2> degenerate{V2(0, 0), V2(1, 1), V2(2, 2)};
        (void)FPVector2::CalculatePolygonCentroid(degenerate);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);

    const std::vector<FPVector2> recentred = FPVector2::RecenterPolygon(Square());
    CHECK(recentred == (std::vector<FPVector2>{V2(-1, -1), V2(1, -1), V2(1, 1), V2(-1, 1)}));
}

TEST_CASE("FPVector2::CalculatePolygonInertiaFactor") {
    // Square [0,2]^2 about the origin: (sum of (|a|^2 + a.b + |b|^2) * cross) / (6 * sum cross)
    // = 128 / 48 = 2.666..., i.e. raw (128 << 32) / (48 << 16) = 174762.
    CHECK(FPVector2::CalculatePolygonInertiaFactor(Square()) == R(174762));
    CHECK(FPVector2::CalculatePolygonInertiaFactor({}) == F(0));
}

TEST_CASE("FPVector2::CalculatePolygonLocalSupport") {
    const std::vector<FPVector2> square = Square();
    const auto support = [&](FPVector2 dir) {
        return FPVector2::CalculatePolygonLocalSupport(square.data(), 4, dir);
    };
    CHECK(support(V2(1, 1)) == V2(2, 2));   // climbs forwards 0 -> 1 -> 2
    CHECK(support(V2(1, -1)) == V2(2, 0));  // stops at 1
    CHECK(support(V2(-1, -1)) == V2(0, 0)); // neither neighbour is better
    CHECK(support(V2(-1, 1)) == V2(0, 2));  // climbs backwards to 3

    // Backward climb over several vertices of a hexagon-like fan.
    const std::vector<FPVector2> fan{V2(0, 0), V2(1, -1), V2(-3, 0), V2(-2, 0), V2(-1, 0)};
    CHECK(FPVector2::CalculatePolygonLocalSupport(fan.data(), 5, V2(-1, 0)) == V2(-3, 0));
}

// --- FPVector3 ---------------------------------------------------------------------

TEST_CASE("FPVector3 constants and constructors") {
    CHECK(FPVector3::Zero() == V3(0, 0, 0));
    CHECK(FPVector3::One() == V3(1, 1, 1));
    CHECK(FPVector3::Left() == V3(-1, 0, 0));
    CHECK(FPVector3::Right() == V3(1, 0, 0));
    CHECK(FPVector3::Up() == V3(0, 1, 0));
    CHECK(FPVector3::Down() == V3(0, -1, 0));
    CHECK(FPVector3::Back() == V3(0, 0, -1));
    CHECK(FPVector3::Forward() == V3(0, 0, 1));
    CHECK(FPVector3(1, 2, 3) == V3(1, 2, 3));
}

TEST_CASE("FPVector3 operators") {
    CHECK(V3(1, 2, 3) + V3(4, 5, 6) == V3(5, 7, 9));
    CHECK(V3(1, 2, 3) - V3(4, 5, 6) == V3(-3, -3, -3));
    CHECK(-V3(1, -2, 3) == V3(-1, 2, -3));
    CHECK(V3(1, 2, 3) * F(2) == V3(2, 4, 6));
    CHECK(F(2) * V3(1, 2, 3) == V3(2, 4, 6));
    CHECK(V3(2, 4, 6) / F(2) == V3(1, 2, 3));
    // Division by an int divides the raw value, truncating toward zero.
    CHECK((FPVector3{R(7), R(-7), F(4)} / 2 == FPVector3{R(3), R(-3), F(2)}));
    CHECK(V3(1, 2, 3) != V3(1, 2, 4));
}

TEST_CASE("FPVector3 products") {
    CHECK(FPVector3::Dot(V3(1, 2, 3), V3(4, 5, 6)) == F(32));
    const FPVector3 rawOnes{R(1), R(1), R(1)};
    const FPVector3 halves{FP::Half(), FP::Half(), FP::Half()};
    CHECK(FPVector3::Dot(rawOnes, halves) == F(0)); // per-term truncation
    CHECK(FPVector3::Cross(V3(1, 0, 0), V3(0, 1, 0)) == V3(0, 0, 1));
    CHECK(FPVector3::Cross(V3(0, 1, 0), V3(0, 0, 1)) == V3(1, 0, 0));
    CHECK(FPVector3::Cross(V3(1, 2, 3), V3(4, 5, 6)) == V3(-3, 6, -3));
    CHECK(FPVector3::Scale(V3(1, 2, 3), V3(4, -5, 6)) == V3(4, -10, 18));
    CHECK(V3(1, 2, 2).SqrMagnitude() == F(9));
    CHECK(FPVector3::DistanceSquared(V3(1, 1, 1), V3(2, 3, 3)) == F(9));
}

TEST_CASE("FPVector3 Min/Max/Lerp") {
    CHECK(FPVector3::Min(V3(1, 5, -2), V3(3, 2, -1)) == V3(1, 2, -2));
    CHECK(FPVector3::Max(V3(1, 5, -2), V3(3, 2, -1)) == V3(3, 5, -1));
    CHECK(FPVector3::Lerp(V3(0, 0, 0), V3(2, 4, 6), FP::Half()) == V3(1, 2, 3));
    CHECK(FPVector3::Lerp(V3(0, 0, 0), V3(2, 4, 6), F(3)) == V3(2, 4, 6));
    CHECK(FPVector3::LerpUnclamped(V3(0, 0, 0), V3(2, 4, 6), F(2)) == V3(4, 8, 12));
    CHECK(FPVector3::LerpUnclamped(V3(0, 0, 0), V3(2, 4, 6), F(-1)) == V3(-2, -4, -6));
}

TEST_CASE("FPVector3 Project/ProjectOnPlane/Reflect") {
    CHECK(FPVector3::Project(V3(3, 4, 0), V3(2, 0, 0)) == V3(3, 0, 0));
    // |normal|^2 below FP.Epsilon (raw 65) projects to zero: (raw 8)^2 >> 16 = 0.
    CHECK(FPVector3::Project(V3(3, 4, 0), FPVector3{R(8), F(0), F(0)}) == V3(0, 0, 0));
    CHECK(FPVector3::ProjectOnPlane(V3(3, 4, 5), V3(0, 1, 0)) == V3(3, 0, 5));
    CHECK(FPVector3::Reflect(V3(1, -1, 0), V3(0, 1, 0)) == V3(1, 1, 0));
    // Here the raw dot (raw 1) is doubled as an integer: X = 1 + (-2 * 1.5) = raw -2.
    // (FPVector2.Reflect's per-term doubling gives raw -3 for the same input.)
    const FPVector3 reflected = FPVector3::Reflect({R(1), F(0), F(0)}, {R(0x18000), F(0), F(0)});
    CHECK(reflected.X == R(-2));
}

TEST_CASE("FPVector3 swizzles, hash and text") {
    CHECK(V3(1, 2, 3).XY() == V2(1, 2));
    CHECK(V3(1, 2, 3).XZ() == V2(1, 3));
    CHECK(V3(1, 2, 3).GetHashCode() == 67746383);
    CHECK(V3(1, 2, 3).Equals(V3(1, 2, 3)));
    CHECK(V3(1, 2, 3).ToString() == "(1, 2, 3)");
    CHECK((FPVector3{F(1) / F(4), F(-3), F(0)}).ToString() == "(0.25, -3, 0)");
}

#ifdef RR_HAVE_FPMATH
// These need the FPMath lookup tables (sqrt/trig), which are not in this build yet.
namespace {
bool Near(FP a, FP b, FP::Raw tolerance = 66) {
    const FP::Raw d = a.RawValue() - b.RawValue();
    return d <= tolerance && d >= -tolerance;
}
} // namespace

TEST_CASE("FPVector length and normalisation (needs FPMath)") {
    CHECK(Near(V2(3, 4).Magnitude(), F(5)));
    CHECK(Near(FPVector2::Distance(V2(1, 1), V2(4, 5)), F(5)));
    const FPVector2 n = V2(3, 4).Normalized();
    CHECK(Near(n.X, F(3) / F(5)) && Near(n.Y, F(4) / F(5)));
    FP length;
    (void)FPVector2::Normalize(V2(0, 7), length);
    CHECK(Near(length, F(7)));
    CHECK(FPVector2::Normalize(V2(0, 0)) == V2(0, 0));
    CHECK(Near(FPVector2::ClampMagnitude(V2(0, 10), F(2)).Y, F(2)));
    CHECK(Near(V3(2, 3, 6).Magnitude(), F(7)));
    CHECK(Near(FPVector3::Normalize(V3(0, 0, 5)).Z, F(1)));
    CHECK(Near(FPVector3::Angle(V3(1, 0, 0), V3(0, 1, 0)), F(90), 700));
    CHECK(Near(FPVector3::SignedAngle(V3(0, 1, 0), V3(1, 0, 0), V3(0, 0, 1)), F(-90), 700));
    CHECK(FPVector3::MoveTowards(V3(0, 0, 0), V3(1, 0, 0), F(5)) == V3(1, 0, 0));
}
#endif
