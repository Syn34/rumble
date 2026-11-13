#include "quantum_core/fp_animation_curve.h"

#include "photon_deterministic/fp_math.h"

namespace Quantum {

namespace {

using Photon::Deterministic::FP;
namespace FPMath = Photon::Deterministic::FPMath;

constexpr FP::Raw kOne = FP::RAW_ONE;
constexpr FP::Raw kFractionMask = kOne - 1;

// Before the curve's start (normalised time < 0). The original works on the fraction of
// the negated time; that is kept as is (it differs from the editor's own loop mode).
FP::Raw WrapBefore(std::int32_t mode, FP::Raw t) {
    const FP::Raw fraction = static_cast<std::uint32_t>(-static_cast<std::int32_t>(t)) & kFractionMask;
    switch (mode) {
    case 0: return 0;                                      // clamp
    case 1: return fraction;                               // repeat
    case 2: return fraction == 0 ? 0 : kOne - fraction;    // ping-pong
    default: return t;
    }
}

// After the curve's end (normalised time > 1).
FP::Raw WrapAfter(std::int32_t mode, FP::Raw t) {
    const FP::Raw fraction = t & kFractionMask;
    switch (mode) {
    case 0: return kOne;                                   // clamp
    case 1: return fraction == 0 ? kOne : fraction;        // repeat
    case 2: return kOne - fraction;                        // ping-pong
    default: return t;
    }
}

} // namespace

FP FPAnimationCurve::Evaluate(FP time) const {
    if (Samples.empty()) return FP::FromRaw(0);
    if (Samples.size() == 1) return Samples[0];

    time = FPMath::ClampUseable(time);
    // The original divides unguarded (a zero-length curve throws); answer the first sample.
    if (EndTime == StartTime) return Samples[0];
    FP::Raw t = (time - StartTime).RawValue() * kOne / (EndTime - StartTime).RawValue();
    if (t < 0) t = WrapBefore(PreWrapMode, t);
    else if (t > kOne) t = WrapAfter(PostWrapMode, t);

    const std::int32_t last = static_cast<std::int32_t>(Samples.size()) - 1;
    const FP::Raw scaled = t * last; // normalised time in sample units, 16.16
    const auto lower = static_cast<std::int32_t>(scaled >> 16);
    const auto upper = static_cast<std::int32_t>((scaled >> 16) + ((scaled & kFractionMask) != 0 ? 1 : 0));
    const FP from = Samples[static_cast<std::size_t>(FPMath::Clamp(lower, 0, last))];
    const FP to = Samples[static_cast<std::size_t>(FPMath::Clamp(upper, 0, last))];

    // Position between the two samples: the remainder of time / sample spacing, where the
    // spacing is 1 / last truncated to fixed point.
    const FP::Raw spacing = kOne / last;
    const FP blend = FPMath::Clamp01(FP::FromRaw((t * kOne / spacing) & kFractionMask));
    return FP::FromRaw(from.RawValue() + ((blend.RawValue() * (to - from).RawValue()) >> 16));
}

} // namespace Quantum
