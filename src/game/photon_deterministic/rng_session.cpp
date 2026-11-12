#include "rng_session.h"

#include "bit_stream.h"
#include "deterministic_frame_serializer.h"

#include <algorithm>

namespace Photon::Deterministic {

namespace {

constexpr std::uint64_t PcgMultiplier = 6364136223846793005ull;

// Raw value one past FP 1.0, so NextUnbiased covers 0..1.0 inclusive.
constexpr std::uint32_t InclusiveUnitRange = static_cast<std::uint32_t>(FP::OneRaw) + 1u;

} // namespace

// 0x7a8dbf0
RNGSession::RNGSession(int seed) {
    // The seed is sign-extended to 64 bits, then two consecutive SplitMix64 outputs
    // become the state and the increment.
    auto sequence = static_cast<std::uint64_t>(static_cast<std::int64_t>(seed));
    state = NextSplitMix64(sequence);
    inc = NextSplitMix64(sequence);
}

// 0x7a8dc68
std::uint64_t RNGSession::NextSplitMix64(std::uint64_t& x) {
    x += 0x9E3779B97F4A7C15ull;
    std::uint64_t z = x;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

// 0x7a8db7c
std::uint32_t RNGSession::NextUInt32() {
    const std::uint64_t old = state;
    state = old * PcgMultiplier + (inc | 1u);

    const auto xorShifted = static_cast<std::uint32_t>(((old >> 18) ^ old) >> 27);
    const auto rotation = static_cast<std::uint32_t>(old >> 59);
    return (xorShifted >> rotation) | (xorShifted << ((32u - rotation) & 31u));
}

// 0x7a8d928
FP RNGSession::Next() {
    return FP::FromRaw(static_cast<FP::Raw>(NextUInt32() >> (32 - FP::FractionBits)));
}

// 0x7a8d974
FP RNGSession::NextFP() {
    return FP::FromRaw(static_cast<FP::Raw>(NextUInt32() >> (32 - FP::FractionBits)));
}

// 0x7a8d9c0
FP RNGSession::NextInclusive() {
    // The original inlines this call; for a bound of 65537 the rejection threshold is
    // 1, so only a raw draw of 0 is rejected before taking the remainder.
    return FP::FromRaw(static_cast<FP::Raw>(NextUnbiased(InclusiveUnitRange)));
}

// 0x7a8da20
std::uint32_t RNGSession::NextUnbiased(std::uint32_t max) {
    if (max == 0) {
        // The original performs an unchecked unsigned division by zero, which on the
        // target CPU yields 0: the threshold becomes 0 and the remainder leaves the
        // draw unchanged. Reproduce that so callers that pass a wrapped range of
        // 2^32 (see NextInclusive) still get the full 32-bit draw.
        return NextUInt32();
    }
    // Draws below 2^32 mod max would make the low results slightly more likely.
    const std::uint32_t threshold = (0u - max) % max;
    for (;;) {
        const std::uint32_t value = NextUInt32();
        if (value >= threshold) {
            return value % max;
        }
    }
}

// 0x7a8da84
FP RNGSession::Next(FP minInclusive, FP maxExclusive) {
    const FP low = std::min(minInclusive, maxExclusive);
    const FP high = std::max(minInclusive, maxExclusive);
    return low + (high - low) * Next();
}

// 0x7a8dae8
FP RNGSession::NextInclusive(FP minInclusive, FP maxInclusive) {
    const FP::Raw low = std::min(minInclusive, maxInclusive).RawValue();
    const FP::Raw high = std::max(minInclusive, maxInclusive).RawValue();
    // Width of the range in raw units; wraps for extreme bounds like the original.
    const auto span = static_cast<std::int64_t>(static_cast<std::uint64_t>(high) -
                                                static_cast<std::uint64_t>(low));

    std::uint64_t offset = 0;
    if (span <= static_cast<std::int64_t>(MAX) - 1) {
        // Fits in one draw: uniform over 0..span.
        offset = NextUnbiased(static_cast<std::uint32_t>(span) + 1u);
    } else {
        // Wider than 32 bits: a raw draw supplies the low word (drawn first), and an
        // unbiased draw over 0..(span >> 32) the high word. Note the result can exceed
        // span slightly; that is how the original behaves.
        const std::uint64_t lowWord = NextUInt32();
        const std::uint64_t highWord =
            NextUnbiased(static_cast<std::uint32_t>(static_cast<std::uint64_t>(span) >> 32) + 1u);
        offset = (highWord << 32) | lowWord;
    }
    return FP::FromRaw(static_cast<FP::Raw>(static_cast<std::uint64_t>(low) + offset));
}

// 0x7a8dbc4
int RNGSession::Next(int minInclusive, int maxExclusive) {
    if (minInclusive == maxExclusive) {
        return minInclusive;
    }
    const int low = std::min(minInclusive, maxExclusive);
    const int high = std::max(minInclusive, maxExclusive);
    const auto width = static_cast<std::uint32_t>(high) - static_cast<std::uint32_t>(low);
    return static_cast<int>(static_cast<std::uint32_t>(low) + NextUnbiased(width));
}

// 0x7a8dcc0
std::string RNGSession::ToString() const {
    // string.Format("{0}:{1}, {2}:{3}", "state", state, "inc", inc)
    return "state:" + std::to_string(state) + ", inc:" + std::to_string(inc);
}

// 0x7a8e078
int RNGSession::GetHashCode() const {
    // The usual (17 * 31 + a) * 31 + b combination of ulong.GetHashCode() values,
    // with 32-bit wrapping arithmetic.
    const auto hashOf = [](std::uint64_t v) {
        return static_cast<std::uint32_t>(v) ^ static_cast<std::uint32_t>(v >> 32);
    };
    std::uint32_t hash = 17u;
    hash = hash * 31u + hashOf(state);
    hash = hash * 31u + hashOf(inc);
    return static_cast<int>(hash);
}

// 0x7a8de88
void RNGSession::Serialize(void* ptr, IDeterministicFrameSerializer& serializer) {
    auto* session = static_cast<RNGSession*>(ptr);
    serializer.Stream().Serialize(&session->state);
    serializer.Stream().Serialize(&session->inc);
}

} // namespace Photon::Deterministic
