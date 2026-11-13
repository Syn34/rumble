// game_mode_systems.h - game-mode systems: activators, score limits, timers.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

// Removes (kills next frame) entities meant only for, or never for, the session's game
// modifier.
class GameModeActivatorSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<GameModeActivator> {
  public:
    // 0x7ba359c
    GameModeActivatorSystem() = default;
    // 0x7ba351c - An activator with `enabled` set keeps its entity only when the session
    // runs its modificator; one without, only when the session runs another.
    void OnAdded(Frame* f, EntityRef entity, GameModeActivator* component) override;
};

// Bolt King: the mode ends once a collector reaches the score.
class GameModeBoltCountSystem final : public SystemSignalsOnly, public ISignalBolt_OnCollected {
  public:
    // 0x7ba377c
    GameModeBoltCountSystem() = default;
    // 0x7ba35a4
    bool get_StartEnabled() const override { return false; }
    // 0x7ba35ac
    void Bolt_OnCollected(Frame* f, EntityRef collector, std::int32_t amount) override;
};

class GameTimerLib {
  public:
    // 0x7ba7a5c - The game mode's duration; -1 without a game mode.
    static FP GetGameDuration(Frame* frame);
    // 0x7ba6f68 - Time spent in the main loop (zero outside it).
    static FP GetElapsedTime(Frame* frame);
    // 0x7ba7b6c - Time left in the main loop of a timed, unfinished game mode, never
    // below zero; once the normal time is up the overtime (if any) counts and
    // `isOverTime` is set. -1 when there is no running timer.
    static FP GetTimeLeft(Frame* frame, bool& isOverTime);
};

// Ends the game mode when its time (and overtime) runs out.
class GameTimerSystem final : public SystemMainThread {
  public:
    // 0x7ba79b4
    GameTimerSystem() = default;
    // 0x7ba7d00 - Only in the main loop of a game mode with a spec.
    void Update(Frame* frame) override;
    // 0x7ba7df4
    static void CheckGameTime(Frame* frame);
};

// Kill-count modes: the mode ends once a player (free-for-all) or a team (team total
// rumble) reaches the kills to win.
class GameModeKillCountSystem final : public SystemSignalsOnly, public ISignalKillable_OnKill {
  public:
    // 0x7ba73f0
    GameModeKillCountSystem() = default;
    // 0x7ba7158
    bool get_StartEnabled() const override { return false; }
    // 0x7ba7160 - Only kills of players by players count.
    void Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) override;
};

// Slow motion: a TimeScale singleton blends the frame's delta time between two scales.
class TimeScaleLib {
  public:
    // 0x7baafa4 - Blends from the current scale to `targetTimeScale` over `duration`.
    static void SetProgressiveTimeScale(Frame* frame, FP targetTimeScale, FP duration);
    // 0x7ba20d8 - Same over `halfDuration`, then back again.
    static void DoBackAndForthTimeScale(Frame* frame, FP targetTimeScale, FP halfDuration);
    // 0x7bab09c - Normal speed, no blend.
    static void ResetTimeScale(Frame* frame);
    // 0x7bab158
    static bool IsTimeScaleActive(Frame* frame);
};

struct TimeScaleSystemFilter {
    EntityRef entity;
    TimeScale* timeScale = nullptr;
    static constexpr auto FilterEntity = &TimeScaleSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&TimeScaleSystemFilter::timeScale);
};

class TimeScaleSystem final : public SystemMainThreadFilter<TimeScaleSystemFilter>,
                              public ISignalGameMode_OnStateChanged {
  public:
    // 0x7ba796c
    TimeScaleSystem() = default;
    // 0x7bab1f0 - Creates the TimeScale singleton.
    void OnInit(Frame* frame) override;
    // 0x7bab284 - While a blend runs, scales the delta time; at its end a back-and-forth
    // blend turns round, anything else stops (the last scale stays in effect).
    void Update(Frame* frame, Filter& filter) override;
    // 0x7bab3cc - The outro slows down per the game mode spec; the end restores speed.
    void GameMode_OnStateChanged(Frame* frame, GAME_MODE_HANDLER_STATE prevState,
                                 GAME_MODE_HANDLER_STATE newState) override;
};

} // namespace Quantum
