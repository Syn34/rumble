#include "test_framework.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame_serializer.h"
#include "photon_deterministic/rng_session.h"

#include <cstdint>

using Photon::Deterministic::BitStream;
using Photon::Deterministic::FP;
using Photon::Deterministic::IBitStream;
using Photon::Deterministic::IDeterministicFrameSerializer;
using Photon::Deterministic::RNGSession;

namespace {

// Reference implementations written from the published algorithms (Vigna's
// SplitMix64 and O'Neill's pcg32_random_r), independent of rng_session.cpp.
std::uint64_t ReferenceSplitMix64(std::uint64_t& x) {
    std::uint64_t z = (x += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

struct ReferencePcg32 {
    std::uint64_t state;
    std::uint64_t inc;

    std::uint32_t Next() {
        const std::uint64_t oldstate = state;
        state = oldstate * 6364136223846793005ull + (inc | 1u);
        const auto xorshifted = static_cast<std::uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        const auto rot = static_cast<std::uint32_t>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((0u - rot) & 31u));
    }
};

ReferencePcg32 ReferenceFromSeed(int seed) {
    auto x = static_cast<std::uint64_t>(static_cast<std::int64_t>(seed));
    ReferencePcg32 r{};
    r.state = ReferenceSplitMix64(x);
    r.inc = ReferenceSplitMix64(x);
    return r;
}

std::uint32_t ReferenceUnbiased(ReferencePcg32& r, std::uint32_t bound) {
    const std::uint32_t threshold = (0u - bound) % bound;
    for (;;) {
        const std::uint32_t v = r.Next();
        if (v >= threshold) {
            return v % bound;
        }
    }
}

} // namespace

TEST_CASE("RNGSession layout is two raw 64-bit words") {
    CHECK(sizeof(RNGSession) == 16);
    CHECK(RNGSession::SIZE == 16);
    CHECK(RNGSession::MAX == 0xFFFFFFFFu);
}

TEST_CASE("RNGSession seeding uses SplitMix64") {
    // Known SplitMix64 outputs for a zero seed.
    const RNGSession zero(0);
    CHECK(zero.state == 0xE220A8397B1DCDAFull);
    CHECK(zero.inc == 0x6E789E6AA1B965F4ull);

    // Negative seeds are sign-extended.
    const RNGSession negative(-7);
    CHECK(negative.state == 0x6C1E186443822970ull);
    CHECK(negative.inc == 0x7A87F4DABCF192AAull);

    std::uint64_t x = 0;
    CHECK(RNGSession::NextSplitMix64(x) == 0xE220A8397B1DCDAFull);
    CHECK(x == 0x9E3779B97F4A7C15ull);
}

TEST_CASE("RNGSession matches the published PCG32 test vector") {
    // pcg32_srandom_r(&rng, 42, 54), then the first six outputs from the PCG demo.
    ReferencePcg32 setup{0, (54u << 1) | 1u};
    setup.Next();
    setup.state += 42;
    setup.Next();

    RNGSession rng;
    rng.state = setup.state;
    rng.inc = setup.inc;
    const std::uint32_t expected[] = {0xa15c02b7u, 0x7b47f409u, 0xba1d3330u,
                                      0x83d2f293u, 0xbfa4784bu, 0xcbed606eu};
    for (std::uint32_t e : expected) {
        CHECK(rng.NextUInt32() == e);
    }
}

TEST_CASE("RNGSession raw sequences from seeds") {
    // Values computed separately (Python) from the same algorithm.
    RNGSession a(0);
    CHECK(a.NextUInt32() == 0x412162a4u);
    CHECK(a.NextUInt32() == 0x01a0e8a6u);
    CHECK(a.NextUInt32() == 0xa3717e36u);
    CHECK(a.NextUInt32() == 0xff9fed17u);

    RNGSession b(12345);
    CHECK(b.NextUInt32() == 0x942315b9u);
    CHECK(b.NextUInt32() == 0xce625b6bu);
    CHECK(b.NextUInt32() == 0xfe847abfu);
    CHECK(b.NextUInt32() == 0x0440f8feu);

    for (int seed : {1, -1, 42, 2147483647, -2147483647 - 1}) {
        RNGSession rng(seed);
        ReferencePcg32 ref = ReferenceFromSeed(seed);
        for (int i = 0; i < 200; ++i) {
            CHECK(rng.NextUInt32() == ref.Next());
        }
        CHECK(rng.state == ref.state);
    }
}

TEST_CASE("RNGSession FP conversions") {
    for (int seed : {3, 99, -5}) {
        RNGSession rng(seed);
        ReferencePcg32 ref = ReferenceFromSeed(seed);
        for (int i = 0; i < 100; ++i) {
            // Next / NextFP: top 16 bits as the fraction, [0, 1).
            CHECK(rng.Next().RawValue() == static_cast<std::int64_t>(ref.Next() >> 16));
            CHECK(rng.NextFP().RawValue() == static_cast<std::int64_t>(ref.Next() >> 16));

            // NextInclusive: unbiased over 0..65536.
            const FP inc = rng.NextInclusive();
            CHECK(inc.RawValue() == ReferenceUnbiased(ref, 65537u));
            CHECK(inc >= FP::Zero());
            CHECK(inc <= FP::One());

            // Next(min, max): min + (max - min) * Next(), bounds in either order.
            const FP lo = FP::FromInt(-3);
            const FP hi = FP::FromRaw(5 * 65536 + 123);
            const FP r = rng.Next(hi, lo);
            const std::int64_t fraction = ref.Next() >> 16;
            CHECK(r.RawValue() ==
                  lo.RawValue() + ((hi.RawValue() - lo.RawValue()) * fraction >> 16));
            CHECK(r >= lo);
            CHECK(r < hi);
        }
    }
}

TEST_CASE("RNGSession NextInclusive(FP, FP) small and wide ranges") {
    RNGSession rng(777);
    ReferencePcg32 ref = ReferenceFromSeed(777);

    // Range fits in 32 bits: one unbiased draw over 0..span.
    const FP lo = FP::FromInt(10);
    const FP hi = FP::FromInt(20);
    for (int i = 0; i < 50; ++i) {
        const FP v = rng.NextInclusive(hi, lo);
        const std::uint32_t span = static_cast<std::uint32_t>(hi.RawValue() - lo.RawValue());
        CHECK(v.RawValue() == lo.RawValue() + ReferenceUnbiased(ref, span + 1));
        CHECK(v >= lo);
        CHECK(v <= hi);
    }

    // Equal bounds still consume one draw (unbiased over a single value).
    const FP same = FP::FromInt(4);
    CHECK(rng.NextInclusive(same, same) == same);
    ref.Next();

    // Wider than 32 bits: raw low word first, then the unbiased high word.
    const FP wideLo = FP::FromInt(-100000);
    const FP wideHi = FP::FromInt(100000);
    for (int i = 0; i < 50; ++i) {
        const FP v = rng.NextInclusive(wideLo, wideHi);
        const auto span = static_cast<std::uint64_t>(wideHi.RawValue() - wideLo.RawValue());
        const std::uint64_t lowWord = ref.Next();
        const std::uint64_t highWord =
            ReferenceUnbiased(ref, static_cast<std::uint32_t>(span >> 32) + 1u);
        CHECK(v.RawValue() ==
              wideLo.RawValue() + static_cast<std::int64_t>((highWord << 32) | lowWord));
    }
    CHECK(rng.state == ref.state);
}

TEST_CASE("RNGSession integer ranges") {
    RNGSession rng(2024);
    ReferencePcg32 ref = ReferenceFromSeed(2024);
    for (int i = 0; i < 100; ++i) {
        const int v = rng.Next(10, -5);
        CHECK(v == -5 + static_cast<int>(ReferenceUnbiased(ref, 15)));
        CHECK(v >= -5);
        CHECK(v < 10);
    }
    // Equal bounds return immediately without drawing.
    const std::uint64_t before = rng.state;
    CHECK(rng.Next(7, 7) == 7);
    CHECK(rng.state == before);

    // NextUnbiased(0) yields the raw draw (unchecked division by zero on the target).
    CHECK(rng.NextUnbiased(0) == ref.Next());
    CHECK(rng.NextUnbiased(1) == 0u);
    ref.Next();
    CHECK(rng.state == ref.state);
}

TEST_CASE("RNGSession ToString and GetHashCode") {
    RNGSession rng;
    rng.state = 5;
    rng.inc = 0x100000003ull;
    CHECK(rng.ToString() == "state:5, inc:4294967299");
    // (17 * 31 + 5) * 31 + (3 ^ 1)
    CHECK(rng.GetHashCode() == (17 * 31 + 5) * 31 + 2);
}

namespace {

class TestSerializer final : public IDeterministicFrameSerializer {
  public:
    explicit TestSerializer(BitStream& stream) : stream_(stream) {}
    bool Writing() const override { return stream_.Writing(); }
    bool Reading() const override { return stream_.Reading(); }
    IBitStream& Stream() override { return stream_; }

  private:
    BitStream& stream_;
};

} // namespace

TEST_CASE("RNGSession Serialize writes state then inc") {
    BitStream stream(32);
    stream.SetWriting(true);
    TestSerializer serializer(stream);

    RNGSession original(31337);
    original.NextUInt32();
    RNGSession::Serialize(&original, serializer);
    CHECK(stream.Position() == 128);
    const auto bytes = stream.ToArray();
    CHECK(bytes.size() == 16);
    CHECK(bytes[0] == static_cast<std::uint8_t>(original.state & 0xFF));
    CHECK(bytes[8] == static_cast<std::uint8_t>(original.inc & 0xFF));

    BitStream reader(stream.Data());
    TestSerializer readSerializer(reader);
    RNGSession copy;
    RNGSession::Serialize(&copy, readSerializer);
    CHECK(copy == original);
}
