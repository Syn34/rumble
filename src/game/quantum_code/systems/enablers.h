// enablers.h - systems that switch other systems on and off as their component appears
// and disappears. The listed systems start disabled; the enabler turns one on when an
// entity gets the component, the disabler turns it off when the last one goes.
#pragma once

#include "quantum_code/systems/all_systems.h"

namespace Quantum {

class KillZoneSystemEnabler final : public GenericSystemEnabler<KillZone, KillZoneSystem> {
  public:
    // 0x7b7bd98
    KillZoneSystemEnabler() = default;
};
class KillZoneSystemDisabler final : public GenericSystemDisabler<KillZone, KillZoneSystem> {
  public:
    // 0x7b7bde0
    KillZoneSystemDisabler() = default;
};

class ShapeOverlappingEnabler final : public GenericSystemEnabler<ShapeOverlapper, ShapeOverlappingSystem> {
  public:
    // 0x7be0c74
    ShapeOverlappingEnabler() = default;
};
class ShapeOverlappingDisabler final : public GenericSystemDisabler<ShapeOverlapper, ShapeOverlappingSystem> {
  public:
    // 0x7be0cbc
    ShapeOverlappingDisabler() = default;
};

class GloveOfDoomTargetEnabler final : public GenericSystemEnabler<AgentOfDoomGroup, GloveOfDoomTargetSystem> {
  public:
    // 0x7bf7a98
    GloveOfDoomTargetEnabler() = default;
};
class GloveOfDoomTargetDisabler final : public GenericSystemDisabler<AgentOfDoomGroup, GloveOfDoomTargetSystem> {
  public:
    // 0x7bf7ae0
    GloveOfDoomTargetDisabler() = default;
};

class GloveOfDoomWeaponEnabler final : public GenericSystemEnabler<GloveOfDoomWeapon, GloveOfDoomWeaponSystem> {
  public:
    // 0x7bf7878
    GloveOfDoomWeaponEnabler() = default;
};
class GloveOfDoomWeaponDisabler final : public GenericSystemDisabler<GloveOfDoomWeapon, GloveOfDoomWeaponSystem> {
  public:
    // 0x7bf78c0
    GloveOfDoomWeaponDisabler() = default;
};

class MoveToSystemEnabler final : public GenericSystemEnabler<MoveTo, MoveToSystem> {
  public:
    // 0x7bd0978
    MoveToSystemEnabler() = default;
};
class MoveToSystemDisabler final : public GenericSystemDisabler<MoveTo, MoveToSystem> {
  public:
    // 0x7bd09c0
    MoveToSystemDisabler() = default;
};

class TurnToSystemEnabler final : public GenericSystemEnabler<TurnTo, TurnToSystem> {
  public:
    // 0x7bd257c
    TurnToSystemEnabler() = default;
};
class TurnToSystemDisabler final : public GenericSystemDisabler<TurnTo, TurnToSystem> {
  public:
    // 0x7bd25c4
    TurnToSystemDisabler() = default;
};

class AutoJumperActorEnablerSystem final : public GenericSystemEnabler<AutoJumperActor, AutoJumperActorSystem> {
  public:
    // 0x7bd5878
    AutoJumperActorEnablerSystem() = default;
};
class AutoJumperActorDisablerSystem final : public GenericSystemDisabler<AutoJumperActor, AutoJumperActorSystem> {
  public:
    // 0x7bd58c0
    AutoJumperActorDisablerSystem() = default;
};

class RollBouncingEnablerSystem final : public GenericSystemEnabler<Roll, RollBouncingSystem> {
  public:
    // 0x7bdf480
    RollBouncingEnablerSystem() = default;
};
class RollBouncingDisablerSystem final : public GenericSystemDisabler<Roll, RollBouncingSystem> {
  public:
    // 0x7bdf4c8
    RollBouncingDisablerSystem() = default;
};

class MeleeAttackEnablerSystem final : public GenericSystemEnabler<MeleeAttack, MeleeAttackSystem> {
  public:
    // 0x7cc8368
    MeleeAttackEnablerSystem() = default;
};
class MeleeAttackDisablerSystem final : public GenericSystemDisabler<MeleeAttack, MeleeAttackSystem> {
  public:
    // 0x7cc83b0
    MeleeAttackDisablerSystem() = default;
};

class DamageToImpulseConverterSystemEnabler final : public GenericSystemEnabler<DamageToImpulseConverter, DamageToImpulseConverterSystem> {
  public:
    // 0x7bbb5b4
    DamageToImpulseConverterSystemEnabler() = default;
};
class DamageToImpulseConverterSystemDisabler final : public GenericSystemDisabler<DamageToImpulseConverter, DamageToImpulseConverterSystem> {
  public:
    // 0x7bbb5fc
    DamageToImpulseConverterSystemDisabler() = default;
};

class ParabolaFollowerEnablerSystem final : public GenericSystemEnabler<ParabolaFollower, ParabolaFollowerSystem> {
  public:
    // 0x7bd0ee8
    ParabolaFollowerEnablerSystem() = default;
};
class ParabolaFollowerDisablerSystem final : public GenericSystemDisabler<ParabolaFollower, ParabolaFollowerSystem> {
  public:
    // 0x7bd0f30
    ParabolaFollowerDisablerSystem() = default;
};

class BlasterSpecialShotEnhancementEnablerSystem final : public GenericSystemEnabler<BlasterSpecialShotEnhancement, BlasterSpecialShotEnhancementSystem> {
  public:
    // 0x7bf0278
    BlasterSpecialShotEnhancementEnablerSystem() = default;
};
class BlasterSpecialShotEnhancementDisablerSystem final : public GenericSystemDisabler<BlasterSpecialShotEnhancement, BlasterSpecialShotEnhancementSystem> {
  public:
    // 0x7bf02c0
    BlasterSpecialShotEnhancementDisablerSystem() = default;
};

class ArenaCannonSystemEnabler final : public GenericSystemEnabler<ArenaCannon, ArenaCannonSystem> {
  public:
    // 0x7bd7a3c
    ArenaCannonSystemEnabler() = default;
};
class ArenaCannonSystemDisabler final : public GenericSystemDisabler<ArenaCannon, ArenaCannonSystem> {
  public:
    // 0x7bd7a84
    ArenaCannonSystemDisabler() = default;
};

class ShieldedActorSystemEnabler final : public GenericSystemEnabler<ShieldedActor, ShieldedActorSystem> {
  public:
    // 0x7cb75ac
    ShieldedActorSystemEnabler() = default;
};
class ShieldedActorSystemDisabler final : public GenericSystemDisabler<ShieldedActor, ShieldedActorSystem> {
  public:
    // 0x7cb75f4
    ShieldedActorSystemDisabler() = default;
};

class AmoeboidLauncherEnhancementPuddleSystemEnabler final : public GenericSystemEnabler<AmoeboidLauncherEnhancementPuddle, AmoeboidLauncherEnhancementPuddleSystem> {
  public:
    // 0x7beee90
    AmoeboidLauncherEnhancementPuddleSystemEnabler() = default;
};
class AmoeboidLauncherEnhancementPuddleSystemDisabler final : public GenericSystemDisabler<AmoeboidLauncherEnhancementPuddle, AmoeboidLauncherEnhancementPuddleSystem> {
  public:
    // 0x7beeed8
    AmoeboidLauncherEnhancementPuddleSystemDisabler() = default;
};

// Enables the projectile switcher systems when a weapon gets the enhancement: the main
// system always, the update system only if the enhancement switches ammunition after a
// use time.
class WeaponProjectileSwitcherEnhancementSystemEnabler final
    : public SystemSignalsOnly,
      public ISignalOnComponentAdded<WeaponProjectileSwitcherEnhancement> {
  public:
    // 0x7bee128
    WeaponProjectileSwitcherEnhancementSystemEnabler() = default;
    // 0x7bee044 - Nothing is enabled when the enhancement's spec asset is missing.
    void OnAdded(Frame* f, EntityRef entity, WeaponProjectileSwitcherEnhancement* component) override;
};

class WeaponProjectileSwitcherEnhancementSystemDisabler final
    : public SystemSignalsOnly,
      public ISignalOnComponentRemoved<WeaponProjectileSwitcherEnhancement> {
  public:
    // 0x7bee1d4
    WeaponProjectileSwitcherEnhancementSystemDisabler() = default;
    // 0x7bee130 - Once no enhancement is left both systems are disabled (update system
    // first).
    void OnRemoved(Frame* f, EntityRef entity, WeaponProjectileSwitcherEnhancement* component) override;
};

} // namespace Quantum
