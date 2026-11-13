// system_groups.h - the system groups of the game's system list. Each group owns its
// children and schedules the enabled ones in the order given here (the original's order).
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

// Area-of-effect life cycle: trails, start, overlap queries, hit filtering and the
// effects applied to what was hit, then lifetime.
class AreaOfEffectSystemGroup final : public SystemMainThreadGroup {
  public:
    // 0x7bb6468
    AreaOfEffectSystemGroup();
};

// Bots and mobs: spawning, targeting, state machines, navigation.
class AISystemsGroup final : public SystemMainThreadGroup {
  public:
    // 0x7cc6f74
    AISystemsGroup();
};

// Game mode flow, timers, mode-specific rules and rankings.
class GameModeSystemsGroup final : public SystemMainThreadGroup {
  public:
    // 0x7ba73f8
    GameModeSystemsGroup();
};

class UltimateSystemGroup final : public SystemMainThreadGroup {
  public:
    // 0x7be596c
    UltimateSystemGroup();
};

class GadgetSystemGroup final : public SystemMainThreadGroup {
  public:
    // 0x7cdda7c
    GadgetSystemGroup();
};

// Weapon loadout, inputs, triggering and firing, then the weapon-specific systems.
class WeaponsSystemsGroup final : public SystemMainThreadGroup {
  public:
    // 0x7c118b8
    WeaponsSystemsGroup();
};

} // namespace Quantum
