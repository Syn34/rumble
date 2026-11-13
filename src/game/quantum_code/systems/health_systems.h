// health_systems.h - hit points: HealthLib (initialisation per spec mode, max-HP
// multipliers) and HealthSystem (regeneration, overhealth decay, pickups).
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

class HealthSpec;
class HeroSpec;
class ReplaceHealthSpecHeroImprovementSpec;

class HealthLib {
  public:
    // 0x7bc25e4 - Looks up the spec and, the first time, works out the entity's maximum HP
    // from the spec's mode (hero level, owner's weapon level, PvE scaling, fixed value,
    // own level, or the human players' average weapon level), then fills the stats and
    // applies the multipliers. The average-level mode waits (stays uninitialised) until
    // that average is known.
    static void InitialiseStatsIfNeeded(Frame* f, EntityRef healthEntity, Health* health, HealthSpec*& healthSpec);
    // 0x7bc2960 - Hero mode: the hero's health spec (possibly replaced by an improvement)
    // and the HP of the hero's level (capped at level 10). Throws std::runtime_error for
    // an entity without a PlayerActor.
    static void InitHeroHealth(Frame* f, EntityRef healthEntity, Health* health, HealthSpec*& healthSpec, FP& hp);
    // 0x7bc2c6c - The hero's first improvement that replaces the health spec.
    static bool TryGetReplaceHealthHeroImprovementSpec(Frame* f, HeroSpec* heroSpec,
                                                       ReplaceHealthSpecHeroImprovementSpec*& replaceHealthSpec);
    // 0x7bc2ec8 - Current HP over maximum HP; zero without Health.
    static FP GetHPRatio(Frame* frame, EntityRef entity);
    // 0x7bb6da0 - Sets (or multiplies into) one source's max-HP multiplier: an unset (1)
    // multiplier or overrideExisting replaces it.
    static void SetMaxHpMultiplier(Frame* frame, EntityRef entity, HEALTH_MULTIPLIER_SOURCE source, FP multiplier,
                                   bool overrideExisting);
    // 0x7bc2f74 - 1 without Health.
    static FP GetMaxHpMultiplier(Frame* frame, EntityRef entity, HEALTH_MULTIPLIER_SOURCE source);
    // 0x7bc2bb8 - Maximum HP = initial maximum times every multiplier (in source order);
    // a full entity stays full.
    static void UpdateMaxHp(Frame* frame, Health* health);
    // TODO(0x7bb8d08, Quantum.HealthLib.ApplyHit): applies a hit to HP and overhealth
    // (thresholds, kill on zero HP, regeneration restart, events); not reimplemented yet.
    // 0x7bc3050
    static void FullRegen(Frame* frame, EntityRef entity);
};

struct HealthSystemFilter {
    EntityRef entity;
    Health* health = nullptr;
    static constexpr auto FilterEntity = &HealthSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&HealthSystemFilter::health);
};

class HealthSystem final : public SystemMainThreadFilter<HealthSystemFilter>,
                           public ISignalShield_OnFull,
                           public ISignalKillable_OnRevive,
                           public ISignalReward_OnCollected,
                           public ISignalOnComponentAdded<Health>,
                           public ISignalBlastBallGame_StateChanged {
  public:
    // 0x7bc3b68
    HealthSystem() = default;
    // 0x7bc30ec - Only during the main loop (or without a game mode), not for players
    // whose game mode has finished: initialises, regenerates and decays overhealth.
    void Update(Frame* f, Filter& filter) override;
    // 0x7bc33ac - Overhealth beyond what is left is dropped; expired overhealth decays a
    // tick at a time over its decrease duration, then goes.
    static void UpdateActiveOverhealths(Frame* f, Health* health);
    // 0x7bc322c - After the spec's delay without damage, HP climbs back towards the
    // maximum over a duration proportional to what was missing when regeneration began.
    static void Regenerate(Frame* frame, EntityRef healthEntity, Health* health, HealthSpec* spec);
    // 0x7bc363c - A recharged shield restarts the regeneration delay for specs that only
    // regenerate behind a shield.
    void Shield_OnFull(Frame* frame, EntityRef entity) override;
    // 0x7bc373c - Revived entities are initialised again.
    void Killable_OnRevive(Frame* frame, EntityRef toRevive, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7bc37e0 - HP pickups heal by the spec's amount per pickup.
    void Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type, std::uint16_t amount) override;
    // 0x7bc392c - Every max-HP multiplier starts at 1.
    void OnAdded(Frame* f, EntityRef entity, Health* component) override;
    // 0x7bc39b8 - Entering state 3 re-initialises every player's health.
    void BlastBallGame_StateChanged(Frame* f, BlastBallGameState newState) override;
};

} // namespace Quantum
