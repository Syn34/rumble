// constants.h - Quantum.Constants (quantum.code.dll): fixed tuning values of the
// simulation. All are FP; the raw values are the originals' (FP literals rounded at
// compile time).
#pragma once

#include "photon_deterministic/fp.h"

namespace Quantum::Constants {

using Photon::Deterministic::FP;

// 0x7c96b44 - 0.1
constexpr FP DESTINATION_REACHED_TOLERANCE() { return FP::FromRaw(0x199a); }
// 0x7c96b4c - 0.5
constexpr FP DESTINATION_MIN_DISTANCE() { return FP::FromRaw(0x8000); }
// 0x7c96b54 - 30
constexpr FP BOTS_TARGET_FINDER_OVERRIDEN_RANGE() { return FP::FromRaw(0x1e0000); }
// 0x7c96b5c - 0.9
constexpr FP BOT_POWER_STATS_MIN_MULTIPLIER() { return FP::FromRaw(0xe666); }
// 0x7c96b64 - 1.1
constexpr FP BOT_POWER_STATS_MAX_MULTIPLIER() { return FP::FromRaw(0x1199a); }
// 0x7c96b70 - 0.48
constexpr FP BOT_POWER_STATS_WEAPON_0_MIN_FRACTION() { return FP::FromRaw(0x7ae1); }
// 0x7c96b78 - 0.52
constexpr FP BOT_POWER_STATS_WEAPON_0_MAX_FRACTION() { return FP::FromRaw(0x851f); }
// 0x7c96b80 - 2.6
constexpr FP BUMPER_PUSH_FORCE_MULTIPLIER() { return FP::FromRaw(0x2999a); }
// 0x7c96b8c - 1
constexpr FP GameMode_MinWaitConnectionTime() { return FP::FromRaw(0x10000); }
// 0x7c96b94 - 20
constexpr FP GameMode_WaitExpectedPlayersTime() { return FP::FromRaw(0x140000); }
// 0x7c96b9c - 0.5
constexpr FP GameMode_WaitUnexpectedPlayersTime() { return FP::FromRaw(0x8000); }

} // namespace Quantum::Constants
