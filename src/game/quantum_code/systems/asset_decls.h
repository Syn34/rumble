// asset_decls.h - TEMPORARY declarations of the quantum.code asset classes the systems
// read.
//
// The asset classes themselves are being reimplemented separately (quantum_code/assets/).
// Until they land, the systems declare here only the classes and fields they use, with
// the original's names, types, field order and base classes. When the real classes
// exist, this header is replaced by includes of theirs (a declaration here then has to
// go: the names clash on purpose, so a mismatch cannot go unnoticed).
#pragma once

#include "quantum_code/asset_refs.h"
#include "quantum_code/enums.h"
#include "quantum_code/components.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/component_prototype.h"

#include "photon_deterministic/fp.h"

#include <vector>

namespace Quantum {

using Photon::Deterministic::FP;

class EnhancementSpec : public AssetObject {};

class GameModeSpec : public AssetObject {
  public:
    GAME_MODE id{};
    FP gameDuration;
    FP overTimeDuration;
};

class HealthSpec : public AssetObject {
  public:
    HealthSpecMode Mode{};
    std::vector<FP> LeveledMaxHP;
    FP MinHP;
    FP MaxHP;
    FP BaseHP;
    FP FixedHP;
    std::vector<FP> AmountPerSelfLevel;
    FP SpawnImmunityDuration;
    bool KillWhenNoMoreHP = false;
    bool HasDamageThreshold = false;
    FP DamageThreshold;
    bool HasPushThreshold = false;
    FP PushPercentageThreshold;
    bool CanRegenerate = false;
    FP TimeWithoutDamageBeforeStartRegen;
    FP TimeToRegenerate;
    bool OnlyRegenerateIfShieldIsUp = false;
    bool TakeDamageOutsideShrinkingZone = false;
    FP PercentDamageOutsideShrinkingZone;
    FP DamageCooldownOutsideShrinkingZone;
    FP HealthRegenPerPickup;
};

class ImprovementSpec : public AssetObject {};

class ReplaceHealthSpecHeroImprovementSpec : public ImprovementSpec {
  public:
    AssetRefHealthSpec HealthSpec;
};

class HeroSpec : public AssetObject {
  public:
    std::vector<FP> LeveledPower;
    std::vector<FP> LeveledHP;
    bool HasShield = false;
    std::vector<FP> LeveledShield;
    AssetRefHealthSpec HealthSpec;
    std::vector<AssetRefImprovementSpec> HeroImprovementSpecs;
};

class SimulationHeroesTable : public AssetObject {
  public:
    struct Entry {
        GENERATED_CHARACTER_ID uniqueID{};
        AssetRefHeroSpec spec;
    };
    AssetRefHeroSpec defaultSpec;
    std::vector<Entry> entries;
};

class LifetimeSpec : public AssetObject {
  public:
    bool StartLifetimeOnSpawn = false;
    bool UseOwnerWeaponLevelForLifetime = false;
    FP LifetimeDuration;
    bool ReduceDurationIfHitDamageable = false;
    FP ReducedLifetimeDuration;
    std::vector<FP> LeveledLifetimeDurations;
    bool SpawnEntityOnDeath = false;
    std::vector<AssetRefEntityPrototype> EntitiesToSpawnOnDeath;
};

class EntityGroupSpec : public AssetObject {
  public:
    std::int32_t GroupMaxMembers = 0;
};

// Something to run on an entity (subclasses implement it).
class CallbackAction : public AssetObject {
  public:
    virtual void Execute(Frame* frame, EntityRef entity) = 0;
};

class BoltKingGameModeSpec : public GameModeSpec {
  public:
    FP EndGameDelay;
    std::int32_t ScoreToBecomeKing = 0;
};

class EnergyCellManagerSpec : public AssetObject {
  public:
    std::int32_t LowCharge = 0;
    std::int32_t MediumCharge = 0;
    std::int32_t HighCharge = 0;
};

class UltimateAbilitySpec : public AssetObject {};

class DamageableSpec : public AssetObject {
  public:
    FP LastHitSaveDuration;
};

namespace Prototypes {
// The prototype of a Killable: what happens when the entity is killed. (Abstract here:
// the prototype classes' materialisation is not part of the systems.)
class Killable_Prototype : public TypedComponentPrototype<Killable> {
  public:
    AssetRefDeathAnimationSpec deathAnimationSpec;
    KillableOnKillBehaviour OnKillBehaviour{};
    FP DelayBeforeDestruction;
    bool OnKillDisablePhysics = false;
    bool OnKillDisableCollider = false;
    AssetRefEntityPrototype EntityToSpawnOnDeath;
};
} // namespace Prototypes

class WeaponEnhancementSpec : public EnhancementSpec {};

class WeaponProjectileSwitcherEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype AlternateAmmunitionAsset;
    std::uint32_t AmmunitionChangeModulo = 0;
    bool DestroyPreviousAmmoInstanceOnSwitch = false;
    bool SwitchAmmoAfterUseTime = false;
    FP SwitchAmmoAfterUseTimeDelay;
};

} // namespace Quantum
