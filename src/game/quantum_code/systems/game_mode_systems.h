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

} // namespace Quantum
