#include "test_framework.h"

#include "photon_deterministic/nullable_fp.h"

#include <stdexcept>

using Photon::Deterministic::FP;
using Photon::Deterministic::NullableFP;
using Photon::Deterministic::NullableNonNegativeFP;

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

TEST_CASE("NullableFP") {
    const NullableFP empty;
    CHECK(!empty.HasValue());
    CHECK(empty.ValueOrDefault(FP::_3()) == FP::_3());
    CHECK(empty.GetHashCode() == 0);
    CHECK(Throws<std::logic_error>([&] { (void)empty.Value(); }));

    const NullableFP two = NullableFP::FromFP(FP::_2());
    CHECK(two.HasValue() && two.Value() == FP::_2());
    CHECK(two.ValueOrDefault(FP::_3()) == FP::_2());
    CHECK(two.GetHashCode() == FP::_2().GetHashCode());

    // Only exactly 1 counts for HasValue, but Value accepts any non-zero flag.
    const NullableFP odd{2, FP::_5()};
    CHECK(!odd.HasValue() && odd.Value() == FP::_5() && odd.ValueOrDefault(FP::_1()) == FP::_1());
}

TEST_CASE("NullableNonNegativeFP stores the flag in the sign bit") {
    const NullableNonNegativeFP empty;
    CHECK(!empty.HasValue());
    CHECK(empty.ValueOrDefault() == FP::Zero());
    CHECK(empty.ToString() == "NULL");
    CHECK(empty.GetHashCode() == 0);
    CHECK(Throws<std::logic_error>([&] { (void)empty.Value(); }));

    const NullableNonNegativeFP half = NullableNonNegativeFP::FromFP(FP::Half());
    CHECK(half._value == (NullableNonNegativeFP::HasValueBit | 32768));
    CHECK(half.HasValue() && half.Value() == FP::Half());
    CHECK(half.ToString() == "0.5");
    CHECK(half.GetHashCode() == FP::Half().GetHashCode());

    const NullableNonNegativeFP zero = NullableNonNegativeFP::FromFP(FP::Zero());
    CHECK(zero.HasValue() && zero.Value() == FP::Zero());

    CHECK(Throws<std::out_of_range>([] { NullableNonNegativeFP::FromFP(FP::FromRaw(-1)); }));
}
