#include "test_framework.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/fp_vector.h"

#include <cstdint>
#include <stdexcept>
#include <string>

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {

template <typename Exception, typename Fn> bool Throws(Fn&& fn) {
    try {
        fn();
    } catch (const Exception&) {
        return true;
    }
    return false;
}

} // namespace

TEST_CASE("FP uses 16 fractional bits") {
    CHECK(FP::One().RawValue() == 65536);
    CHECK(FP::FromInt(3).RawValue() == 3 * 65536);
    CHECK(FP::FromInt(-2).AsInt() == -2);
    CHECK(FP::RAW_ONE == 65536 && FP::Precision == 16 && FP::SIZE == 8);
}

TEST_CASE("FP constants are the 16-bit encodings of their names") {
    // Truncations of the exact values, worked out by hand (x * 65536).
    CHECK(FP::Pi().RawValue() == 205887);       // 205887.4
    CHECK(FP::PiTimes2().RawValue() == 411774); // 411774.8
    CHECK(FP::PiOver2().RawValue() == 102943);  // 102943.7
    CHECK(FP::Deg2Rad().RawValue() == 1143);    // 1143.8
    CHECK(FP::Rad2Deg().RawValue() == 3754936); // 3754936.2
    CHECK(FP::_0_25().RawValue() == 16384);
    CHECK(FP::_1_50().RawValue() == 98304);
    CHECK(FP::_0_33().RawValue() == 65536 / 3);
    CHECK(FP::_1_33().RawValue() == 65536 + 65536 / 3);
    CHECK(FP::_1000() == FP::FromInt(1000));
    CHECK(FP::Minus_1() == -FP::_1());
    CHECK(FP::EN3().RawValue() == 65 && FP::Epsilon() == FP::EN3());
    CHECK(FP::UseableMax().RawValue() == 2147483647);
    CHECK(FP::UseableMin().RawValue() == -2147483647 - 1);
}

TEST_CASE("FP multiply and divide truncate like the simulation expects") {
    CHECK(FP::FromInt(3) * FP::FromInt(4) == FP::FromInt(12));
    CHECK(FP::FromInt(1) / FP::FromInt(4) == FP::FromRaw(16384));
    // 1/3 truncates toward zero in raw space
    CHECK((FP::One() / FP::FromInt(3)).RawValue() == 21845);
    CHECK((-FP::One() / FP::FromInt(3)).RawValue() == -21845);
    // multiply shifts arithmetically (floors toward negative infinity)
    CHECK((FP::FromRaw(-1) * FP::Half()).RawValue() == -1);
    CHECK(FP::MulTruncate(FP::FromRaw(-1), FP::Half()).RawValue() == -1);
}

TEST_CASE("FP division edge cases follow the original's arm64 behaviour") {
    CHECK(FP::FromInt(5) / FP::Zero() == FP::Zero());
    CHECK(FP::FromInt(5) / 0 == FP::Zero());
    CHECK(FP::FromInt(5) % FP::Zero() == FP::FromInt(5));
    CHECK(FP::MinValue() / -1 == FP::MinValue());
    CHECK(FP::MinValue() % FP::FromRaw(-1) == FP::Zero());
}

TEST_CASE("FP mixed int operators scale the int") {
    const FP x = FP::FromRaw(98304); // 1.5
    CHECK((x + 2).RawValue() == 98304 + 2 * 65536);
    CHECK((2 + x).RawValue() == 98304 + 2 * 65536);
    CHECK((x - 2).RawValue() == 98304 - 2 * 65536);
    CHECK((2 - x).RawValue() == 2 * 65536 - 98304);
    CHECK((x * 3).RawValue() == 3 * 98304);
    CHECK((3 * x).RawValue() == 3 * 98304);
    CHECK((x / 3).RawValue() == 98304 / 3);
    CHECK((3 / x).RawValue() == 2 * 65536); // 3 / 1.5
    CHECK((FP::FromInt(7) % 2) == FP::One());
    CHECK((FP::FromInt(-7) % FP::FromInt(2)) == -FP::One()); // sign of the dividend
    CHECK(x > 1 && x < 2 && x >= 1 && x <= 2 && 1 < x && 2 > x && x != 1);
    CHECK(FP::FromInt(4) == 4 && 4 == FP::FromInt(4));
}

TEST_CASE("FP implicit and explicit conversions") {
    const FP fromInt = std::int32_t{-3};
    const FP fromUnsigned = std::uint32_t{4000000000u};
    const FP fromShort = std::int16_t{-2};
    const FP fromUShort = std::uint16_t{65535};
    const FP fromByte = std::uint8_t{200};
    CHECK(fromInt.RawValue() == -3 * 65536);
    CHECK(fromUnsigned.RawValue() == std::int64_t{4000000000} * 65536);
    CHECK(fromShort.RawValue() == -2 * 65536);
    CHECK(fromUShort.RawValue() == std::int64_t{65535} * 65536);
    CHECK(fromByte.RawValue() == 200 * 65536);

    const FP minusOneAndAHalf = FP::FromRaw(-98304);
    CHECK(static_cast<std::int32_t>(minusOneAndAHalf) == -2); // floors
    CHECK(static_cast<std::int64_t>(minusOneAndAHalf) == -2);
    CHECK(static_cast<float>(minusOneAndAHalf) == -1.5f);
    CHECK(static_cast<double>(minusOneAndAHalf) == -1.5);
    CHECK(minusOneAndAHalf.AsDouble() == -1.5);
    // AsInt keeps only 32 bits of the integer part.
    CHECK(FP::FromInt(std::int64_t{1} << 32).AsInt() == 0);
}

TEST_CASE("FP float conversion truncates; positive overflow throws") {
    CHECK(FP::FromFloat_UNSAFE(1.5f) == FP::FromRaw(98304));
    CHECK(FP::FromFloat_UNSAFE(-0.5f) == FP::FromRaw(-32768));
    CHECK(FP::FromFloat_UNSAFE(1.0f / 65536.0f * 1.75f) == FP::FromRaw(1));
    CHECK(Throws<std::overflow_error>([] { FP::FromFloat_UNSAFE(1e30f); }));
    CHECK(FP::FromFloat_UNSAFE(-1e30f) == FP::MinValue());
}

TEST_CASE("FP CompareTo, Equals and GetHashCode") {
    CHECK(FP::One().CompareTo(FP::Half()) == 1);
    CHECK(FP::Half().CompareTo(FP::One()) == -1);
    CHECK(FP::One().CompareTo(FP::One()) == 0);
    CHECK(FP::One().Equals(FP::_1()));
    // Hash of a long: low word xor high word.
    CHECK(FP::FromRaw((std::int64_t{1} << 32) | 2).GetHashCode() == 3);
    CHECK(FP::FromRaw(-1).GetHashCode() == 0);
}

TEST_CASE("FP ToString prints the float value with 7 significant digits") {
    CHECK(FP::FromRaw(98304).ToString() == "1.5");
    CHECK(FP::FromInt(-42).ToString() == "-42");
    CHECK(FP::FromRaw(21845).ToString() == "0.3333282"); // 21845/65536 = 0.33332824...
    CHECK(FP::FromRaw(1).ToString() == "1.525879E-05");  // 2^-16
    CHECK(FP::FromInt(12345678).ToString() == "1.234568E+07");
}

TEST_CASE("FP ParseFractions rounds digits / 10^n to 16 bits") {
    CHECK(FP::ParseFractions("") == 0);
    CHECK(FP::ParseFractions("5") == 32768);
    CHECK(FP::ParseFractions("25") == 16384);
    // 333 * 65536 = 21823488; (21823488 + 500) / 1000 = 21823
    CHECK(FP::ParseFractions("333") == 21823);
    // 0.99999 * 65536 = 65535.34 -> 65535
    CHECK(FP::ParseFractions("99999") == 65535);
    // 0.999999 * 65536 = 65535.93 -> rounds to 65536
    CHECK(FP::ParseFractions("999999") == 65536);
    // Digits beyond 14 are ignored.
    CHECK(FP::ParseFractions("50000000000000999") == 32768);
    CHECK(FP::ParseInteger("12") == 12 * 65536);
}

TEST_CASE("FP FromString") {
    CHECK(FP::FromString("1.5") == FP::FromRaw(98304));
    CHECK(FP::FromString("  -2.25 ") == FP::FromRaw(-147456));
    CHECK(FP::FromString("7") == FP::FromInt(7));
    CHECK(FP::FromString("7.") == FP::FromInt(7));
    CHECK(FP::FromString(".5") == FP::Half());
    CHECK(FP::FromString("") == FP::Zero());
    CHECK(FP::FromString("   ") == FP::Zero());
    // Original quirk: the leading '.' test looks at the character before the
    // sign was removed, so "-.5" reads 5 as an integer.
    CHECK(FP::FromString("-.5") == FP::FromInt(-5));
    CHECK(Throws<std::invalid_argument>([] { FP::FromString("1.2.3"); }));
    CHECK(Throws<std::invalid_argument>([] { FP::FromString("."); }));
    CHECK(Throws<std::invalid_argument>([] { FP::FromString("abc"); }));
    CHECK(Throws<std::overflow_error>([] { FP::FromString("99999999999999999999"); }));
    // 2^47 - 1 is the largest integer part; adding a fraction that pushes the
    // sum past Int64 max must throw (checked addition in the original).
    CHECK(FP::FromString("140737488355327.5").RawValue() ==
          (((std::int64_t{1} << 47) - 1) << 16) + 32768);
    // .9999999 rounds to a whole 65536, which no longer fits.
    CHECK(Throws<std::overflow_error>([] { FP::FromString("140737488355327.9999999"); }));
}

TEST_CASE("FPVector3 dot shifts each term separately") {
    // Each component product of raw 1 * raw 1 shifts to 0 on its own, so the
    // sum is 0 rather than (3 >> 16).
    const FPVector3 tiny{FP::FromRaw(1), FP::FromRaw(1), FP::FromRaw(1)};
    CHECK(FPVector3::Dot(tiny, tiny) == FP::Zero());
    const FPVector3 v{FP::FromInt(1), FP::FromInt(2), FP::FromInt(3)};
    CHECK(v.SqrMagnitude() == FP::FromInt(14));
}
