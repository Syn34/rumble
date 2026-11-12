// Tests for FPQuaternion, FPMatrix2x2, FPMatrix3x3, FPBounds2, NullableFPVector3 and the
// .NET-style float formatting used by their ToString methods. Expected values are
// derived by hand from the fixed-point rules.
#include "test_framework.h"

#include "math_tables.h"

#include "photon_deterministic/dotnet_number_format.h"
#include "photon_deterministic/fp_bounds2.h"
#include "photon_deterministic/fp_matrix2x2.h"
#include "photon_deterministic/fp_matrix3x3.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/nullable_fp_vector3.h"

#include <optional>

using Photon::Deterministic::FP;
using Photon::Deterministic::FPBounds2;
using Photon::Deterministic::FPMatrix2x2;
using Photon::Deterministic::FPMatrix3x3;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::NullableFPVector3;
using Photon::Deterministic::detail::FormatSingle;
using Photon::Deterministic::detail::FormatSingleFixed;

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
FPQuaternion Q(int x, int y, int z, int w) {
    return {F(x), F(y), F(z), F(w)};
}
} // namespace

// --- FPQuaternion ------------------------------------------------------------------

TEST_CASE("FPQuaternion identity, zero, conjugate") {
    CHECK(FPQuaternion::Identity() == Q(0, 0, 0, 1));
    CHECK(FPQuaternion::IsIdentity(Q(0, 0, 0, 1)));
    CHECK(!FPQuaternion::IsIdentity(Q(0, 0, 0, -1)));
    CHECK(FPQuaternion::IsZero(Q(0, 0, 0, 0)));
    CHECK(!FPQuaternion::IsZero(FPQuaternion{R(1), F(0), F(0), F(0)}));
    CHECK(FPQuaternion::Conjugate(Q(1, 2, 3, 4)) == Q(-1, -2, -3, 4));
    CHECK(Q(1, 2, 3, 4).Conjugated() == Q(-1, -2, -3, 4));
    CHECK(Q(1, 2, 3, 4).MagnitudeSqrRaw() == F(30).RawValue());
}

TEST_CASE("FPQuaternion::Product follows Hamilton's rules") {
    const FPQuaternion i = Q(1, 0, 0, 0);
    const FPQuaternion j = Q(0, 1, 0, 0);
    const FPQuaternion k = Q(0, 0, 1, 0);
    CHECK(i * j == k);
    CHECK(j * i == Q(0, 0, -1, 0));
    CHECK(j * k == i);
    CHECK(k * i == j);
    CHECK(i * i == Q(0, 0, 0, -1));
    CHECK(FPQuaternion::Product(FPQuaternion::Identity(), Q(1, 2, 3, 4)) == Q(1, 2, 3, 4));
    // (1,2,3,4) * (5,6,7,8): standard result (24, 48, 48, -6).
    CHECK(Q(1, 2, 3, 4) * Q(5, 6, 7, 8) == Q(24, 48, 48, -6));
}

TEST_CASE("FPQuaternion rotates points") {
    // 180 degrees about Y: (0, 1, 0, 0).
    const FPQuaternion halfTurnY = Q(0, 1, 0, 0);
    CHECK(halfTurnY * V3(1, 0, 0) == V3(-1, 0, 0));
    CHECK(halfTurnY * V3(0, 0, 1) == V3(0, 0, -1));
    CHECK(halfTurnY * V3(0, 1, 0) == V3(0, 1, 0));
    CHECK(FPQuaternion::Identity() * V3(1, 2, 3) == V3(1, 2, 3));
    // 90 degrees about Z with sqrt(1/2) = raw 46341: Z * 2Z = (46341 * 92682) >> 16 = raw
    // 65536 exactly, so (1, 0, 0) lands exactly on (0, 1, 0).
    const FPQuaternion quarterTurnZ{F(0), F(0), R(46341), R(46341)};
    CHECK(quarterTurnZ * V3(1, 0, 0) == V3(0, 1, 0));
}

TEST_CASE("FPQuaternion hash and text") {
    CHECK(FPQuaternion::Identity().GetHashCode() == 15765393);
    CHECK(FPQuaternion::Identity().ToString() == "(0.0, 0.0, 0.0, 1.0)");
    // F1 rounds half away from zero; values rounding to zero lose their sign.
    const FPQuaternion q{F(1) / F(4), F(-3) / F(2), R(-2621), F(2)}; // -2621 raw ~ -0.04
    CHECK(q.ToString() == "(0.3, -1.5, 0.0, 2.0)");
    CHECK(FPQuaternion::RAW_180 == F(180).RawValue());
    CHECK(FPQuaternion::RAW_360 == F(360).RawValue());
}

// --- Matrices ----------------------------------------------------------------------

TEST_CASE("FPMatrix2x2") {
    const FPMatrix2x2 m{F(1), F(3), F(2), F(4)}; // rows (1, 2), (3, 4); column-major
    CHECK(m.Determinant() == F(-2));
    CHECK(m.MultiplyVector(V2(1, 1)) == V2(3, 7));
    CHECK(m.MultiplyVector(V2(1, 0)) == V2(1, 3));
    CHECK(FPMatrix2x2::Zero() == (FPMatrix2x2{}));
    CHECK(m.GetHashCode() == -2133618799);
    CHECK(m.ToString() == "((1, 2), (3, 4))");
}

TEST_CASE("FPMatrix3x3 construction and transpose") {
    const FPMatrix3x3 rows =
        FPMatrix3x3::FromRows(F(1), F(2), F(3), F(4), F(5), F(6), F(7), F(8), F(9));
    CHECK(rows.M01 == F(2));
    CHECK(rows.M10 == F(4));
    CHECK(rows.M21 == F(8));
    const FPMatrix3x3 columns =
        FPMatrix3x3::FromColumns(F(1), F(4), F(7), F(2), F(5), F(8), F(3), F(6), F(9));
    CHECK(rows == columns);
    const FPMatrix3x3 transposed = rows.Transposed();
    CHECK(transposed.M01 == F(4));
    CHECK(transposed.M10 == F(2));
    CHECK(transposed.M20 == F(3));
    CHECK(transposed.Transposed() == rows);
    CHECK(rows.ToString() == "((1, 2, 3), (4, 5, 6), (7, 8, 9))");
    // 17 * 31^9 mod 2^32, the constant the original folds in for all-zero fields.
    CHECK(FPMatrix3x3::Zero().GetHashCode() == 0x38e0830f);
}

TEST_CASE("FPMatrix3x3 arithmetic") {
    const FPMatrix3x3 a =
        FPMatrix3x3::FromRows(F(1), F(2), F(3), F(4), F(5), F(6), F(7), F(8), F(9));
    const FPMatrix3x3 identity =
        FPMatrix3x3::FromRows(F(1), F(0), F(0), F(0), F(1), F(0), F(0), F(0), F(1));
    CHECK(a * identity == a);
    CHECK(identity * a == a);
    CHECK(a + identity ==
          FPMatrix3x3::FromRows(F(2), F(2), F(3), F(4), F(6), F(6), F(7), F(8), F(10)));
    // a * a, row 0 = (1*1+2*4+3*7, 1*2+2*5+3*8, 1*3+2*6+3*9) = (30, 36, 42), etc.
    CHECK(a * a ==
          FPMatrix3x3::FromRows(F(30), F(36), F(42), F(66), F(81), F(96), F(102), F(126), F(150)));
    CHECK(a * V3(1, 0, -1) == V3(-2, -2, -2));
    CHECK(a * V3(1, 1, 1) == V3(6, 15, 24));
}

// --- FPBounds2 ---------------------------------------------------------------------

TEST_CASE("FPBounds2") {
    FPBounds2 b{V2(1, 1), V2(1, 2)};
    CHECK(b.Max() == V2(2, 3));
    CHECK(b.Min() == V2(0, -1));
    b.Expand(F(2)); // half the amount on each side
    CHECK(b.Extents == V2(2, 3));
    CHECK(b.Center == V2(1, 1));

    b.SetMinMax(V2(0, 0), V2(4, 2));
    CHECK(b.Center == V2(2, 1));
    CHECK(b.Extents == V2(2, 1));

    b.Encapsulate(V2(5, -1));
    CHECK(b.Min() == V2(0, -1));
    CHECK(b.Max() == V2(5, 2));
    CHECK(b.Extents == (FPVector2{F(5) / F(2), F(3) / F(2)}));

    b.Encapsulate(V2(1, 0)); // already inside: unchanged
    CHECK(b.Max() == V2(5, 2));

    // Odd raw sizes lose their last bit when halved: [0, raw 3] -> extents raw 1.
    FPBounds2 tiny;
    tiny.SetMinMax(V2(0, 0), FPVector2{R(3), R(3)});
    CHECK(tiny.Extents == (FPVector2{R(1), R(1)}));
    CHECK(tiny.Max() == (FPVector2{R(2), R(2)}));

    // hash = (17 * 31 + Center.GetHashCode()) * 31 + Extents.GetHashCode().
    const FPBounds2 h{V2(1, 2), V2(1, 2)};
    CHECK(h.GetHashCode() == (17 * 31 + 2179025) * 31 + 2179025);
}

// --- NullableFPVector3 -------------------------------------------------------------

TEST_CASE("NullableFPVector3") {
    const NullableFPVector3 empty;
    CHECK(!empty.HasValue());
    CHECK(empty.GetHashCode() == 0);
    bool threw = false;
    try {
        (void)empty.Value();
    } catch (const std::bad_optional_access&) {
        threw = true;
    }
    CHECK(threw);

    const NullableFPVector3 full = V3(1, 2, 3); // implicit, like the original
    CHECK(full.HasValue());
    CHECK(full.Value() == V3(1, 2, 3));
    CHECK(full.GetHashCode() == V3(1, 2, 3).GetHashCode());

    // A flag other than 0/1: HasValue says no, Value still returns the vector.
    NullableFPVector3 odd = V3(4, 5, 6);
    odd._hasValue = 2;
    CHECK(!odd.HasValue());
    CHECK(odd.Value() == V3(4, 5, 6));
}

// --- .NET float formatting ---------------------------------------------------------

TEST_CASE(".NET float formatting, general") {
    CHECK(FormatSingle(0.0f) == "0");
    CHECK(FormatSingle(1.0f) == "1");
    CHECK(FormatSingle(-2.25f) == "-2.25");
    CHECK(FormatSingle(1.0f / 3.0f) == "0.3333333"); // 7 significant digits
    CHECK(FormatSingle(1234567.0f) == "1234567");
    CHECK(FormatSingle(12345678.0f) == "1.234568E+07");
    CHECK(FormatSingle(10000000.0f) == "1E+07");
    CHECK(FormatSingle(0.0001f) == "0.0001");
    CHECK(FormatSingle(0.00001f) == "1E-05");
    CHECK(FormatSingle(FP::FromRaw(1).AsFloat()) == "1.525879E-05"); // 1/65536
    CHECK(FormatSingle(123456.7f) == "123456.7");                    // float is 123456.703125
}

TEST_CASE(".NET float formatting, fixed") {
    CHECK(FormatSingleFixed(1.0f, 1) == "1.0");
    CHECK(FormatSingleFixed(0.0f, 1) == "0.0");
    CHECK(FormatSingleFixed(0.25f, 1) == "0.3");
    CHECK(FormatSingleFixed(-0.04f, 1) == "0.0");
    CHECK(FormatSingleFixed(-0.06f, 1) == "-0.1");
    CHECK(FormatSingleFixed(9.96f, 1) == "10.0");
    CHECK(FormatSingleFixed(0.004f, 2) == "0.00");
    CHECK(FormatSingleFixed(1234.5f, 0) == "1235");
    // 123.45f is 123.4499969...; it is first rounded to 7 digits (123.4500) and then
    // to one decimal, so it prints 123.5.
    CHECK(FormatSingleFixed(123.45f, 1) == "123.5");
}

// These need the FPMath lookup tables (sqrt/trig), which are not in this build yet.
namespace {
bool Near(FP a, FP b, FP::Raw tolerance = 200) {
    const FP::Raw d = a.RawValue() - b.RawValue();
    return d <= tolerance && d >= -tolerance;
}
bool Near(const FPVector3& a, const FPVector3& b) {
    return Near(a.X, b.X) && Near(a.Y, b.Y) && Near(a.Z, b.Z);
}
} // namespace

TEST_CASE("FPQuaternion angle constructions") {
    rr::test::InstallMathDerivedTables();
    CHECK(Near(FPQuaternion::Euler(F(0), F(90), F(0)) * V3(0, 0, 1), V3(1, 0, 0)));
    CHECK(Near(FPQuaternion::AngleAxis(F(90), V3(0, 0, 1)) * V3(1, 0, 0), V3(0, 1, 0)));
    CHECK(Near(FPQuaternion::LookRotation(V3(1, 0, 0), false) * V3(0, 0, 1), V3(1, 0, 0)));
    CHECK(FPQuaternion::Normalize(Q(0, 0, 0, 0)) == FPQuaternion::Identity());
    CHECK(Near(FPQuaternion::Normalize(Q(0, 0, 0, 3)).W, F(1)));
    const FPVector3 euler = FPQuaternion::Euler(F(10), F(20), F(30)).AsEuler();
    // Several table lookups and fixed-point products accumulate ~0.025 degrees.
    CHECK(Near(euler.X, F(10), 2000) && Near(euler.Y, F(20), 2000) && Near(euler.Z, F(30), 2000));
    const FPQuaternion a = FPQuaternion::Identity();
    const FPQuaternion b = FPQuaternion::AngleAxis(F(90), V3(0, 1, 0));
    CHECK(Near(FPQuaternion::Slerp(a, b, FP::Half()) * V3(0, 0, 1),
               FPQuaternion::AngleAxis(F(45), V3(0, 1, 0)) * V3(0, 0, 1)));
    CHECK(FPQuaternion::RotateTowards(a, b, F(180)) == b);
}
