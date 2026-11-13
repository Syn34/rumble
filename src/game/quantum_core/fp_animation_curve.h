// fp_animation_curve.h - Quantum.FPAnimationCurve (quantum.core.dll): an editor animation
// curve baked into evenly spaced fixed-point samples for the simulation.
//
// The editor keeps the original keyframes (Keys) and wrap modes for re-baking; the
// simulation only evaluates Samples, which cover [StartTime, EndTime] with Resolution
// steps. Wrap modes as stored here: 0 clamps, 1 repeats, 2 ping-pongs.
#pragma once

#include "photon_deterministic/fp.h"

#include <cstdint>
#include <vector>

namespace Quantum {

struct FPAnimationCurve {
    using FP = Photon::Deterministic::FP;

    struct Keyframe {
        FP Time;
        FP Value;
        FP InTangent;
        FP OutTangent;
        std::int32_t TangentMode = 0;
        std::uint8_t TangentModeLeft = 0;
        std::uint8_t TangentModeRight = 0;
        std::uint8_t WeightedMode = 0;
    };

    std::vector<FP> Samples;
    std::int32_t PreWrapMode = 0;
    std::int32_t PostWrapMode = 0;
    FP StartTime;
    FP EndTime;
    std::int32_t Resolution = 0;
    std::int32_t OriginalPreWrapMode = 0;
    std::int32_t OriginalPostWrapMode = 0;
    std::vector<Keyframe> Keys;

    // 0x7d3db88 - the curve's value at `time`: zero without samples, the only sample for a
    // one-sample curve, otherwise a linear interpolation between the two samples around
    // the (wrapped) normalised time.
    FP Evaluate(FP time) const;
};

} // namespace Quantum
