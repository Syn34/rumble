// damageable_systems.h - damage resolution: DamageableSystem (queued hits through elemental
// multipliers, shield and health, invulnerability windows), ShieldedActorLib and
// ShieldedActorSystem (shield points, breaking and regeneration).
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

class DamageableSpec;
class ElementalDamageTable;
class ShieldedActorSpec;

struct DamageableSystemFilter {
    EntityRef entity;
    Damageable* damageable = nullptr;
    static constexpr auto FilterEntity = &DamageableSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&DamageableSystemFilter::damageable);
};

class DamageableSystem final : public SystemMainThreadFilter<DamageableSystemFilter>,
                               public ISignalKillable_OnRevive,
                               public ISignalGameMode_OnMainLoopStart,
                               public ISignalShield_OnBreaks,
                               public ISignalOnComponentAdded<Damageable>,
                               public ISignalDamageable_OnBeforeHit {
  public:
    // 0x7bb9b10
    DamageableSystem() = default;
    // 0x7bb820c - Living damageables with a spec, when the session has an elemental table:
    // resolve the queued hits, then let stale electric and ice build-ups lapse.
    void Update(Frame* frame, Filter& filter) override;
    // 0x7bb895c - The electric build-up resets once the table's reset delay has passed
    // since the last electric hit.
    static void ResetElectricalDamageGuage(Frame* frame, Damageable* damageable, const ElementalDamageTable* elementalDamageTable);
    // 0x7bb89d0 - Same for ice.
    static void ResetIceBuildUp(Frame* frame, Damageable* damageable, const ElementalDamageTable* elementalDamageTable);
    // 0x7bb831c - Killable and no longer alive.
    static bool IsDead(Frame* frame, EntityRef entity);
    // 0x7bb8398 - Each queued hit (none while invulnerable to damage, and none after the
    // entity dies): elemental multiplier, shield then health, remembered as the last hit,
    // reported, and counted in the attacker's weapon statistics. The queue is emptied.
    void ResolveHits(Frame* frame, EntityRef hittenEntity, Damageable* damageable, DamageableSpec* damageableSpec,
                     const ElementalDamageTable* elementalDamageTable);
    // 0x7bb8a5c - The table's multiplier for the hit against the damageable's element; a
    // spec that dies to buffed damage types turns any multiplier above 1 into a kill.
    FP GetElementalDamageMultiplier(Frame* frame, const ElementalDamageTable* elementalDamageTable, HitData* hit,
                                    DamageableSpec* damageableSpec);
    // 0x7bb8ad8 - A shield that takes damage keeps it from health.
    ShieldHitResult ResolveShieldDamages(Frame* frame, EntityRef entity, HitData* hitData);
    // 0x7bb8c04
    HealthHitResult ResolveHealthDamages(Frame* frame, EntityRef entity, HitData* hitData);
    // 0x7bb910c - Revived entities get the respawn invulnerability.
    void Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7bb9238 - Every damageable gets the initial invulnerability when the main loop starts.
    void GameMode_OnMainLoopStart(Frame* frame, EntityRef gameMode) override;
    // 0x7bb9418 - A broken shield grants the shield-break invulnerability.
    void Shield_OnBreaks(Frame* frame, EntityRef entity, EntityRef sourceEntity, const FPVector3& direction) override;
    // 0x7bb9544 - New damageables get the spawn invulnerability.
    void OnAdded(Frame* frame, EntityRef entity, Damageable* damageable) override;
    // TODO(0x7bb9614, Quantum.DamageableSystem.Damageable_OnBeforeHit): a projectile headshot
    //   first clears the source's critical-hit multiplier and rebuilds the hit with
    //   DamageSpec.CreateHitData (not reimplemented yet, so that step is skipped); the
    //   attacker's boss, shieldless-target and more-bolts damage multipliers are applied.
    void Damageable_OnBeforeHit(Frame* f, EntityRef targetEntity, HitData* hitData) override;
};

class ShieldedActorLib {
  public:
    // 0x7cb6fbc - A hit that does not ignore shields takes shield points (all of them for
    // shield-breaking hits, or when a broken shield is regenerating); the regeneration timer
    // restarts. A shield left with points reports the hit (and may slow the actor); an
    // emptied one breaks. Nothing is reported as taken while the shield is broken.
    static ShieldHitResult ApplyHit(Frame* frame, EntityRef entity, const HitData& hitData);
    // 0x7cb7294 - Entering a state: waiting restarts the regeneration timer, regenerating
    // is signalled, full refills the shield.
    static void SetState(Frame* frame, EntityRef entity, ShieldState state);
    // 0x7cb7410 - Regeneration may start after the spec's delay from now.
    static void StartOrRestartRegenTimers(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                          ShieldedActorSpec* spec);
    // TODO(0x7cb7464, Quantum.ShieldedActorLib.ApplyMoveSpeedModifier): slows the actor's KCC
    //   (hit slow at most once per tick interval, or the break slow); waits for
    //   KccExtension.AddModifier_MoveSpeed. Does nothing for now.
    static void ApplyMoveSpeedModifier(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                       ShieldedActorSpec* spec);
};

struct ShieldedActorSystemFilter {
    EntityRef entity;
    ShieldedActor* shieldedActor = nullptr;
    static constexpr auto FilterEntity = &ShieldedActorSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&ShieldedActorSystemFilter::shieldedActor);
};

class ShieldedActorSystem final : public SystemMainThreadFilter<ShieldedActorSystemFilter>,
                                  public ISignalKillable_OnRevive,
                                  public ISignalOnComponentAdded<ShieldedActor> {
  public:
    // 0x7cb7dc4
    ShieldedActorSystem() = default;
    // 0x7cb763c
    bool get_StartEnabled() const override { return false; }
    // 0x7cb7644 - Revived actors come back with a full shield.
    void Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7cb774c - First time: the shield size comes from the hero's level (heroes without
    // a shield lose the component) or from the human players' average weapon level; the
    // average-level mode waits until that is known. Throws std::runtime_error for a
    // hero-mode shield on a non-player.
    static void InitActorIfNeeded(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor, ShieldedActorSpec* spec);
    // 0x7cb7a00 - Living actors: initialise, clear the per-frame flags, wait, regenerate.
    void Update(Frame* frame, Filter& filter) override;
    // 0x7cb7b08 - A waiting shield that may regenerate starts once its timer has run out.
    static void WaitForRegen(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor);
    // 0x7cb7b74 - Regenerating shields fill at max / regeneration time per second, and
    // become full at the maximum.
    static void Regenerate(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor, ShieldedActorSpec* spec);
    // 0x7cb7c20 - Disabling the system removes every shield.
    void OnDisabled(Frame* f) override;
    // 0x7cb7d9c - New shields may regenerate.
    void OnAdded(Frame* f, EntityRef entity, ShieldedActor* component) override;
};

} // namespace Quantum
