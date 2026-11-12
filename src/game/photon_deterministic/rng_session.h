// rng_session.h - Photon.Deterministic.RNGSession, the simulation's random number generator.
//
// The generator is PCG32 (O'Neill's "XSH RR" output on a 64-bit LCG): the 64-bit
// state advances by the standard PCG multiplier plus an odd increment, and each step
// outputs a 32-bit value made by xor-shifting the old state and rotating it by its top
// five bits. Every random decision in the gameplay simulation draws from one of these,
// so the exact sequence, the seeding and the conversions to FP ranges below must match
// the original bit for bit or replays and lockstep matches diverge.
//
// The struct lives inside simulation frame memory and is serialised as two raw
// 64-bit words, so its layout is part of the contract: state at offset 0, inc at 8.
#pragma once

#include "fp.h"

#include <cstdint>
#include <string>

namespace Photon::Deterministic {

class IDeterministicFrameSerializer;

struct RNGSession {
    // Size of the struct in frame memory, in bytes.
    static constexpr int SIZE = 16;
    static constexpr std::uint32_t MAX = 0xFFFFFFFFu;

    std::uint64_t state = 0;
    // Stream selector. Only its low bit is forced to 1 when used, so an even value
    // here still yields a valid (odd) increment.
    std::uint64_t inc = 0;

    RNGSession() = default;
    explicit RNGSession(int seed);

    // FP in [0, 1): the top 16 bits of a draw used as the fractional part.
    FP Next();
    // Same draw as Next(); kept as a separate name like the original.
    FP NextFP();
    // FP in [0, 1], i.e. raw value 0..65536 inclusive.
    FP NextInclusive();
    // FP in [min, max). The arguments may be given in either order.
    FP Next(FP minInclusive, FP maxExclusive);
    // FP in [min, max]. The arguments may be given in either order.
    FP NextInclusive(FP minInclusive, FP maxInclusive);
    // Integer in [min, max). Returns min when the bounds are equal; either order is accepted.
    int Next(int minInclusive, int maxExclusive);
    // Uniform integer in [0, max) by rejection sampling (no modulo bias).
    std::uint32_t NextUnbiased(std::uint32_t max);
    // One raw 32-bit PCG output.
    std::uint32_t NextUInt32();

    std::string ToString() const;
    int GetHashCode() const;

    // Serialises the session pointed to by `ptr` (frame memory) through the frame
    // serializer's bit stream: state, then inc, each as a 64-bit value.
    static void Serialize(void* ptr, IDeterministicFrameSerializer& serializer);

    // SplitMix64 step: advances `x` by the golden-ratio increment and returns the
    // mixed value. Used to expand a 32-bit seed into the PCG state and increment.
    static std::uint64_t NextSplitMix64(std::uint64_t& x);

    friend bool operator==(const RNGSession&, const RNGSession&) = default;
};

static_assert(sizeof(RNGSession) == RNGSession::SIZE, "RNGSession layout is serialised raw");

} // namespace Photon::Deterministic
