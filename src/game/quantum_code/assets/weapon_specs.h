// weapon_specs.h - weapon, gadget and ultimate ability assets and their lookup tables (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/actor_specs.h"

namespace Quantum {

class GadgetSpec : public AssetObject {
  public:
    PlayerActorInputActionFilter AllowedInputActionDuringUse = PlayerActorInputActionFilter::All;
    FP cooldown = FP::FromInt(2);
    FP minCooldown = FP::FromInt(1);
    bool useDuration{};
    FP duration = FP::FromInt(1);
    FP failSafeTimeOut = FP::FromInt(10);
    bool ignorePushWhileInUse = true;
    bool isToggleable{};
    bool resetReloadOnActivation = true;
    std::vector<GadgetSpecModifier> levelUpModifiers;

    // 0x7c90e24
    GadgetSpec() = default;

    // TODO(0x7c90690, Quantum.GadgetSpec.Activate): void Activate(Frame frame, EntityRef entity,
    //   GadgetUser* user)
    // TODO(0x7c908c4, Quantum.GadgetSpec.IsOnCooldown): bool IsOnCooldown(Frame frame, EntityRef
    //   entity, GadgetUser user)
    // TODO(0x7c9086c, Quantum.GadgetSpec.IsOnCooldown): bool IsOnCooldown(Frame frame, EntityRef
    //   entity, GadgetUser* user)
    // TODO(0x7c90918, Quantum.GadgetSpec.Update): void Update(Frame frame, EntityRef entity,
    //   GadgetUser* user)
    // TODO(0x7c907ac, Quantum.GadgetSpec.Deactivate): void Deactivate(Frame frame, EntityRef
    //   entity, GadgetUser* user, bool setCooldown)
    // TODO(0x7c909e8, Quantum.GadgetSpec.OnWeaponReload): void OnWeaponReload(Frame frame,
    //   EntityRef weaponEntity, Weapon weapon, EntityRef gagdetUserEntity, GadgetUser* gadgetUser)
    // TODO(0x7c90a04, Quantum.GadgetSpec.OnWeaponUnEquip): void OnWeaponUnEquip(Frame frame,
    //   EntityRef weaponEntity, EntityRef gagdetUserEntity, GadgetUser* gadgetUser)
    // TODO(0x7c90ad4, Quantum.GadgetSpec.GetActivationRange): FP GetActivationRange(Frame frame,
    //   EntityRef entity)
    // TODO(0x7c90adc, Quantum.GadgetSpec.OnPlayerActorStunned): void OnPlayerActorStunned(Frame
    //   frame, EntityRef entity, GadgetUser* gadgetUser)
    // TODO(0x7c90ae4, Quantum.GadgetSpec.GetPowerAtLevel): int GetPowerAtLevel(int level)
    // TODO(0x7c90c88, Quantum.GadgetSpec.GetCooldownAtLevel): FP GetCooldownAtLevel(int level)
};

class SimulationGadgetTable : public AssetObject {
  public:
    struct GadgetDefinition {
        GENERATED_GADGET_ID id{};
        GENERATED_WEAPON_ID weaponId{};
        GENERATED_CHARACTER_ID characterId{};
        AssetRefGadgetSpec gadgetSpec;
    };

    AssetRefGadgetSpec defaultGadgetSpec;
    std::vector<SimulationGadgetTable::GadgetDefinition> definitions;

    // 0x7c90fcc
    SimulationGadgetTable() = default;

    // TODO(0x7c90ed8, Quantum.SimulationGadgetTable.TryGetGadget): bool
    //   TryGetGadget(GENERATED_CHARACTER_ID characterID, GadgetDefinition gadgetDefinition)
    // TODO(0x7c90fc4, Quantum.SimulationGadgetTable.GetDefaultGadget): AssetRefGadgetSpec
    //   GetDefaultGadget()
};

class SimulationMeleeWeaponsTable : public AssetObject {
  public:
    struct MeleeWeaponDefinition {
        GENERATED_CHARACTER_ID characterId{};
        GENERATED_WEAPON_ID id{};
    };

    GENERATED_WEAPON_ID defaultMeleeWeapon{};
    std::vector<SimulationMeleeWeaponsTable::MeleeWeaponDefinition> definitions;

    // 0x7c95d10
    SimulationMeleeWeaponsTable() = default;

    // TODO(0x7c95c48, Quantum.SimulationMeleeWeaponsTable.TryGetMeleeWeapon): bool
    //   TryGetMeleeWeapon(GENERATED_CHARACTER_ID characterID, MeleeWeaponDefinition
    //   meleeWeaponDefinition)
};

class SimulationUltimateTable : public AssetObject {
  public:
    struct UltimateDefinition {
        GENERATED_ULTIMATE_ID id{};
        GENERATED_CHARACTER_ID characterId{};
        AssetRefUltimateAbilitySpec ultimateAbilitySpec;
    };

    AssetRefUltimateAbilitySpec defaultUltimateSpec;
    std::vector<SimulationUltimateTable::UltimateDefinition> definitions;

    // 0x7c94c28
    SimulationUltimateTable() = default;

    // TODO(0x7c94b54, Quantum.SimulationUltimateTable.TryGetUltimate): bool
    //   TryGetUltimate(GENERATED_CHARACTER_ID characterID, UltimateDefinition ultimateDefinition)
    // TODO(0x7c94c20, Quantum.SimulationUltimateTable.GetDefaultUltimate):
    //   AssetRefUltimateAbilitySpec GetDefaultUltimate()
};

class SimulationWeaponTable : public AssetObject {
  public:
    struct Entry {
        GENERATED_WEAPON_ID uniqueID{};
        AssetRefEntityPrototype entityPrototype;
        AssetRefWeaponSpec weaponSpecRef;

        // 0x7c95f84
        Entry() = default;
    };

    FPAnimationCurve botsBackpackToWeaponLevelCurve;
    FP botsWeaponLevelRandomRangePercent{};
    std::vector<GENERATED_WEAPON_ID> botBanlist;
    std::vector<SimulationWeaponTable::Entry> entries;

    // 0x7c95ea8
    SimulationWeaponTable() = default;

    // TODO(0x7c8f820, Quantum.SimulationWeaponTable.TryGetWeaponId): bool TryGetWeaponId(Weapon
    //   weapon, GENERATED_WEAPON_ID weaponId)
    // TODO(0x7c95d98, Quantum.SimulationWeaponTable.GetWeaponSpec): WeaponSpec GetWeaponSpec(Frame
    //   frame, GENERATED_WEAPON_ID weaponId)
};

class UltimateAbilitySpec : public AssetObject {
  public:
    bool hasDuration = true;
    FP duration = FP::FromInt(5);
    bool finishUltimateOnRelease{};
    bool alwaysDisplayProgressLabel{};
    PlayerActorInputActionFilter AllowedInputActionDuringUse = PlayerActorInputActionFilter::All;
    bool canDashDuringUse = true;
    bool canJumpDuringUse = true;
    bool canUseGadgetDuringUse{};
    bool canRecast{};
    FP recastDelay = FP::FromInt(1);
    std::int32_t fullCharge = 100;
    bool resetChargeOnFinishUltimate{};
    FP chargeRatePerBolt{};
    FP chargeRatePerSecond{};
    FP chargeRatePerHit{};
    FP hitChargeCooldown = FP::FromRaw(16384); // 0.25
    FP chargeRatePerPushReceived{};
    FP chargeRatePerDamageDealt{};
    FP chargeRatePerDamageReceived{};
    FP chargeRatePerRespawn{};
    bool mustBeFullyCharged = true;
    std::int32_t minChargeToActivate{};
    bool toggleMode{};
    bool startInvulnerable{};
    FP invulnerabilityDuration{};
    INVULNERABILITY_FILTER invulnerableFilter = INVULNERABILITY_FILTER::ALL;
    bool addMoveSpeedModifier{};
    FP moveSpeedModifierOnUse = FP::FromInt(1);
    bool addOverhealth{};
    AssetRefOverhealthPowerUpSpec overhealthSpec;
    std::vector<UltimateSpecModifier> levelUpModifiers;
    FP TimeBetweenTwoUsesInSecs = FP::FromInt(1);
    bool useUltimateStacks{};
    std::int32_t maxStacks = 2;
    bool spawnEntityOnActivate{};
    AssetRefEntityPrototype entityToSpawnOnActivate;
    bool disableShootByDuration{};
    FP disableShootNormedDuration{};

    // 0x7c94a70
    UltimateAbilitySpec() = default;

    // TODO(0x7c9471c, Quantum.UltimateAbilitySpec.Init): void Init(Frame f, EntityRef entity)
    // TODO(0x7c94720, Quantum.UltimateAbilitySpec.CanUseWeapon): bool CanUseWeapon(Frame f,
    //   EntityRef entity)
    // TODO(0x7c94728, Quantum.UltimateAbilitySpec.GetPowerAtLevel): int GetPowerAtLevel(int level)
    // TODO(0x7c948cc, Quantum.UltimateAbilitySpec.GetFullChargeAtLevel): int
    //   GetFullChargeAtLevel(int level)
};

class WeaponBehaviour_ChargingSpec : public AssetObject {
  public:
    bool UpdateCurrentBurstWithCharge = true;
    FP FullChargeDuration = FP::FromRaw(98304); // 1.5
    std::int32_t NumberOfChargeLevel = 3;
    bool AutoReleaseOnFullCharge{};

    // 0x7c95f8c
    WeaponBehaviour_ChargingSpec() = default;
};

class WeaponSpec : public AssetObject {
  public:
    struct MultiShotConfig {
        std::vector<FPVector3> spawnOffsets;

        // 0x7c95bc0
        MultiShotConfig() = default;
    };

    ::Quantum::DamageType DamageType = ::Quantum::DamageType::Normal;
    WeaponLoadoutSlotType LoadoutSlotType{};
    bool DisableFireButtonDuringUltimate = true;
    bool ConsumesUltimateCharges{};
    FP ConsumptionPerShot{};
    ::Quantum::WeaponType WeaponType{};
    WeaponStartUseType StartUseType{};
    bool HasInputBuffering{};
    FP InputBufferingDuration = FP::FromRaw(16384); // 0.25
    bool HasMinDelayBeforeRelease{};
    FP MinFireDelayBeforeRelease = FP::FromInt(1);
    bool AutoReleaseAfterDelay{};
    bool DisplayFireButton{};
    WeaponStopUseType StopUseType{};
    WeaponTag Tags{};
    WeaponFilterType weaponFilterType = WeaponFilterType::Versatility;
    bool UseMeleeInput{};
    bool CanUseTriangleAimAssist{};
    bool CanHaveMultipleAmmoInstance = true;
    bool IsFiringWhileAmmoIsAlive{};
    FPVector3 EffectOffset;
    FPVector3 EffectScale = FPVector3::One();
    bool EffectHasDuration{};
    AssetRefEntityPrototype AmmunitionAsset;
    FPVector3 DefaultSpawnOffset;
    bool AutoCycleMultiShotConfigs = true;
    std::vector<WeaponSpec::MultiShotConfig> MultiShotConfigs;
    bool ConsumeOneAmmoPerMultiShot{};
    bool UseCustomDelayOnEquipped{};
    FP DelayOnEquipped{};
    FP FireRate = FP::FromInt(1);
    std::int32_t AmmoPerBurst = 1;
    FP BurstFireRate = FP::FromInt(1);
    bool BurstCanBeInterrupted{};
    FP BurstInterruptionWindowDuration = FP::FromRaw(655); // 0.00999451
    FP Range = FP::FromInt(20);
    FP OverShootDistance{};
    std::uint16_t BoltCost = 20;
    FP SpreadRadius{};
    bool DisableStickyFireInput{};
    bool CanDealCriticalHits{};
    bool UseCooldown{};
    FP CooldownDuration = FP::FromInt(1);
    bool CooldownFeedbackOnReticle = true;
    bool UseReload{};
    FP ReloadDurationSeconds = FP::FromInt(1);
    bool InfiniteAmmo{};
    bool UseAutoLoad = true;
    FP AutoLoadDelaySeconds = FP::FromRaw(98304); // 1.5
    bool OverrideDurationForAutoReload{};
    FP AutoLoadDurationSeconds = FP::FromInt(1);
    std::uint32_t MaxAmmo = 100u;
    std::uint32_t AmmoBurnPerShot = 1u;
    bool ConsumeAmmoOnDestroy{};
    std::uint32_t PickupMaxAmmo = 100u;
    bool SetKccLookTowardsCamera = true;
    bool SetKccLookTowardsCameraSight{};
    WeaponMoveSpeedModType MoveSpeedModType{};
    FP MoveSpeedModFactorOnUse{};
    bool MoveSpeedModHasCustomDuration{};
    FP MoveSpeedModDuration{};
    bool CanHeadshot{};
    bool UseOwnerTransformForDefaultLookDir{};
    bool UseRaycastForLookAtPoint = true;
    bool UseRangeForDefaultLookPoint = true;
    bool UseCurrentTargetForDefaultLookPoint{};
    bool OrientWeaponToTarget = true;
    bool UseBotCustomTargetOnly{};
    bool CanAutoShoot{};
    bool SnapLookAtPointToGround{};
    FP SnapRaycastDistance = FP::FromInt(2);
    bool OverrideTargetFinder{};
    AssetRefTargetFinderSpec TargetFinderSpecOverride;
    std::vector<WeaponSpecModifier> levelUpModifiers;
    std::vector<AssetRefEnhancementSpec> enhancementSpecs;
    std::int32_t AvailableInBucketIndex{};
    FP BotsMinLongPressTime = FP::FromRaw(98304); // 1.5
    FP BotsMaxLongPressTime = FP::FromInt(3);

    // 0x7c959c8
    WeaponSpec() = default;

    // TODO(0x7c94eb8, Quantum.WeaponSpec.CanUseWeapon): bool CanUseWeapon(Frame frame, EntityRef
    //   weaponEntity)
    // TODO(0x7c94ec0, Quantum.WeaponSpec.CanStopUseWeapon): bool CanStopUseWeapon(Frame frame,
    //   EntityRef weaponEntity, Weapon* weapon)
    // TODO(0x7c94ee0, Quantum.WeaponSpec.CanFireWeapon): bool CanFireWeapon(Frame frame, EntityRef
    //   weaponEntity)
    // TODO(0x7c94f08, Quantum.WeaponSpec.FireWeapon): void FireWeapon(Frame frame, Filter filter,
    //   WeaponType> overrideWeaponType)
    // TODO(0x7c955e4, Quantum.WeaponSpec.HasTag): bool HasTag(WeaponTag tag)
    // TODO(0x7c955f4, Quantum.WeaponSpec.GetWeaponPowerAtLevel): int GetWeaponPowerAtLevel(int
    //   level)
    // TODO(0x7c95798, Quantum.WeaponSpec.GetWeaponLevelAtPower): byte GetWeaponLevelAtPower(FP
    //   power, byte maxWeaponLevel)
};

class AddForceOnUltimateSpec : public UltimateAbilitySpec {
  public:
    enum class ForceType : std::int32_t {
        Impulse = 0,
        Accelerate = 1,
    };

    AddForceOnUltimateSpec::ForceType Type{};
    FP Strength{};
    FP ImpulseMaxSpeed = FP::FromInt(4);
    FP ImpulseMaxVerticalSpeed = FP::FromInt(100);
    FPVector3 Direction;
    bool DirectionIsLocal{};
    FP Delay{};
    bool LerpStrengthWithDuration{};
    std::uint32_t LerpPower = 1u;
    FP ImpulseDuration{};
    FP FallSpeedDampening{};

    // 0x7be5934
    AddForceOnUltimateSpec() = default;

    // TODO(0x7be58d4, Quantum.AddForceOnUltimateSpec.Init): void Init(Frame f, EntityRef entity)
    // TODO(0x7be592c, Quantum.AddForceOnUltimateSpec.CanUseWeapon): bool CanUseWeapon(Frame f,
    //   EntityRef entity)
};

class DrillDashGadgetSpec : public GadgetSpec {
  public:
    FP AnticipationDuration = FP::FromInt(1);
    FP AnticipationMoveSpeedModifier = FP::FromRaw(32768); // 0.5
    bool FreezeOnAnticipation = true;
    FP DashDuration = FP::FromInt(2);
    FP DashPower = FP::FromInt(30);
    FP DashInputFalloff = FP::FromRaw(21845); // 0.333328
    FP DashMaxDistance = FP::FromInt(10);
    Shape3DConfig DetectionShapeConfig;
    FPVector3 DetectionShapeOverlapOffset;
    LayerMask DetectionLayerMask;
    QueryOptions DetectionQueryOptions{};
    AssetRefDamageSpec DetectionDamageSpec;
    AssetRefPusherSpec DetectionPusherSpec;
    ENTITY_INFO_FILTER StopEntityInfoFilter{};
    FP AttackDuration = FP::FromInt(1);
    AssetRefEntityPrototype AttackAoePrototypeAsset;
    FP LastHitDuration = FP::FromInt(1);
    AssetRefEntityPrototype LastHitAoePrototypeAsset;
    FP RecoveryDuration = FP::FromInt(1);
    FP RecoveryMoveSpeedModifier = FP::FromRaw(32768); // 0.5
    FP GravityModifier = FP::FromInt(-5);
    AssetRefTargetFinderSpec TargetFinderAsset;

    // 0x7cdb7a0
    DrillDashGadgetSpec() = default;

    // TODO(0x7cdb010, Quantum.DrillDashGadgetSpec.get_TotalDuration): FP get_TotalDuration()
    // TODO(0x7cdb028, Quantum.DrillDashGadgetSpec.get_WeaponID): GENERATED_WEAPON_ID get_WeaponID()
    // TODO(0x7cdb030, Quantum.DrillDashGadgetSpec.Init): void Init(Frame frame, EntityRef
    //   actorEntity)
    // TODO(0x7cdb088, Quantum.DrillDashGadgetSpec.OnGadgetActivatedImpl): void
    //   OnGadgetActivatedImpl(Frame frame, EntityRef entity, GadgetUser* user)
    // TODO(0x7cdb1b8, Quantum.DrillDashGadgetSpec.OnGadgetDeactivated): void
    //   OnGadgetDeactivated(Frame frame, EntityRef entity, GadgetUser* user)
    // TODO(0x7cdb4c4, Quantum.DrillDashGadgetSpec.OnGadgetUpdated): void OnGadgetUpdated(Frame
    //   frame, EntityRef entity, GadgetUser* user)
    // TODO(0x7cdb4c8, Quantum.DrillDashGadgetSpec.GetActivationRange): FP GetActivationRange(Frame
    //   frame, EntityRef entity)
    // TODO(0x7cdb2f8, Quantum.DrillDashGadgetSpec.Cleanup): void Cleanup(Frame f, EntityRef entity,
    //   DrillDash* pDrillDash)
    // TODO(0x7cdb698, Quantum.DrillDashGadgetSpec.TryToOverrideTargetFinderSpec): bool
    //   TryToOverrideTargetFinderSpec(Frame f, EntityRef entity, AssetRefTargetFinderSpec
    //   targetFinderSpecAssetRef)
    // TODO(0x7cdb4d0, Quantum.DrillDashGadgetSpec.TryToResetTargetFinderSpec): bool
    //   TryToResetTargetFinderSpec(Frame f, EntityRef entity)
};

class GadgetWeaponSpec : public GadgetSpec {
  public:
    GENERATED_WEAPON_ID weaponID{};

    // 0x7c01f7c
    GadgetWeaponSpec() = default;

    // TODO(0x7c01b98, Quantum.GadgetWeaponSpec.get_WeaponID): GENERATED_WEAPON_ID get_WeaponID()
    // TODO(0x7c01ba0, Quantum.GadgetWeaponSpec.Init): void Init(Frame frame, EntityRef actorEntity)
    // TODO(0x7c01ba4, Quantum.GadgetWeaponSpec.OnGadgetActivatedImpl): void
    //   OnGadgetActivatedImpl(Frame frame, EntityRef entity, GadgetUser* user)
    // TODO(0x7c01db0, Quantum.GadgetWeaponSpec.OnGadgetDeactivated): void OnGadgetDeactivated(Frame
    //   frame, EntityRef entity, GadgetUser* user)
    // TODO(0x7c01e8c, Quantum.GadgetWeaponSpec.OnGadgetUpdated): void OnGadgetUpdated(Frame frame,
    //   EntityRef entity, GadgetUser* user)
};

class GatlingWeaponSpec : public WeaponSpec {
  public:
    struct GatlingLevel {
        FP SpinUpTime{};
        FP FireRateMultiplier{};
        bool IsProgressive{};
    };

    std::vector<GatlingWeaponSpec::GatlingLevel> Levels;

    // 0x7bf6d54
    GatlingWeaponSpec() = default;
};

class GloveOfDoomWeaponSpec : public WeaponSpec {
  public:
    std::int32_t DoomGroupMaxMembers{};
    FP AutoshootDelayForRespawningLastMember = FP::FromInt(1);

    // 0x7bf7868
    GloveOfDoomWeaponSpec() = default;
};

class HoloshieldGloveWeaponSpec : public WeaponSpec {
  public:
    std::int32_t ShieldMaxMembers{};
    std::int32_t AutoShootMaxMembers{};
    FP AutoshootDelayForRespawningLastMember = FP::FromInt(1);

    // 0x7bf9154
    HoloshieldGloveWeaponSpec() = default;
};

class HoverBootsWeaponSpec : public WeaponSpec {
  public:
    FP StopUseTransitionDuration = FP::FromInt(1);
    FP ForceInputDuration = FP::FromInt(1);
    FP TargetMaxSpeedModifier = FP::FromInt(1);
    FP ReachMaxSpeedDuration = FP::FromInt(1);
    FP MovementReactivity = FP::FromInt(1);
    FP GravityModifier{};
    LayerMask HitWallDynamicLayersToIgnore = LayerMask{0};
    FP HitWallStunDuration = FP::FromInt(3);
    FP HitWallMaxBounceForce = FP::FromInt(20);
    FP HitWallMaxAngleDegrees = FP::FromInt(45);
    FPVector2 ForwardRange = FPVector2(FP::FromRaw(32768), FP::FromRaw(98304));
    FP StraffStrength = FP::FromRaw(32768); // 0.5

    // 0x7bf988c
    HoverBootsWeaponSpec() = default;

    // TODO(0x7bf9744, Quantum.HoverBootsWeaponSpec.CanUseWeapon): bool CanUseWeapon(Frame frame,
    //   EntityRef weaponEntity)
};

class NegatronWeaponSpec : public WeaponSpec {
  public:
    std::vector<FP> LeveledMaxBeamDurationSeconds = std::vector<FP>(10);

    // 0x7bfb178
    NegatronWeaponSpec() = default;
};

class RynoWeaponSpec : public WeaponSpec {
  public:
    std::uint32_t MaxLockedTargets = 5u;
    FP TimeToAcquireTarget{};
    std::vector<std::uint32_t> LeveledMaxMissilesPerFire = std::vector<std::uint32_t>(10);
    FP MissileFireRate = FP::FromRaw(6553); // 0.0999908
    FP AutoFireDelaySeconds = FP::FromInt(2);

    // 0x7bfb374
    RynoWeaponSpec() = default;

    // TODO(0x7bfb1dc, Quantum.RynoWeaponSpec.CanStopUseWeapon): bool CanStopUseWeapon(Frame frame,
    //   EntityRef weaponEntity, Weapon* weapon)
    // TODO(0x7bfb1e8, Quantum.RynoWeaponSpec.CanFireWeapon): bool CanFireWeapon(Frame frame,
    //   EntityRef weaponEntity)
    // TODO(0x7bfb2d0, Quantum.RynoWeaponSpec.FireWeapon): void FireWeapon(Frame frame, Filter
    //   filter, WeaponType> overrideWeaponType)
};

class TeslaClawWeaponSpec : public WeaponSpec {
  public:
    AssetRefTargetFinderSpec TargetFinderSpecWhenShooting;
    LayerMask FireCastLayerMask;

    // 0x7bfd358
    TeslaClawWeaponSpec() = default;

    // TODO(0x7bfc944, Quantum.TeslaClawWeaponSpec.FireWeapon): void FireWeapon(Frame frame, Filter
    //   filter, WeaponType> overrideWeaponType)
    // TODO(0x7bfd018, Quantum.TeslaClawWeaponSpec.<FireWeapon>g__ShootTarget|2_0): void
    //   <FireWeapon>g__ShootTarget|2_0(PotentialTarget target, Filter filter, DamageSpec
    //   damageSpec, <>c__DisplayClass2_0 )
};

class UltimateEvolutionSpec : public UltimateAbilitySpec {
  public:
    struct EvolutionStat {
        FP Scale{};
        FP HealthPointMultiplier{};
    };

    FP AoESpawnDelay{};
    AssetRefEntityPrototype AoEPrototypeAssetRef;
    std::int32_t NumberOfEvolutionLevel = 4;
    std::vector<UltimateEvolutionSpec::EvolutionStat> EvolutionStats;

    // 0x7be3d6c
    UltimateEvolutionSpec() = default;

    // TODO(0x7be3d0c, Quantum.UltimateEvolutionSpec.Init): void Init(Frame f, EntityRef entity)
    // TODO(0x7be3d64, Quantum.UltimateEvolutionSpec.CanUseWeapon): bool CanUseWeapon(Frame f,
    //   EntityRef entity)
};

class UltimateSlamSpec : public UltimateAbilitySpec {
  public:
    AssetRefTargetFinderSpec TargetFinderAsset;
    AssetRefParabolaFollowerSpec ParabolaFollowerAsset;
    AssetRefEntityPrototype SlamAoEPrototypeAsset;
    REMOVE_REASON SpawnReasonSlamAoE{};
    FP HorizontalRange = FP::FromInt(7);
    FP MaxHeightDiff = FP::FromInt(5);
    ::Quantum::LayerMask LayerMask = ::Quantum::LayerMask{1};
    ::Quantum::QueryOptions QueryOptions = ::Quantum::QueryOptions::HitAll;
    FP SlamOffset = FP::FromRaw(114688); // 1.75
    std::int32_t MaxVerticalRaycastCount = 5;
    FP VerticalRaycastLength = FP::FromInt(100);
    bool Debug{};
    FP FallbackSphereRadius = FP::FromInt(1);

    // 0x7be42bc
    UltimateSlamSpec() = default;

    // TODO(0x7be425c, Quantum.UltimateSlamSpec.Init): void Init(Frame f, EntityRef entity)
    // TODO(0x7be42b4, Quantum.UltimateSlamSpec.CanUseWeapon): bool CanUseWeapon(Frame f, EntityRef
    //   entity)
};

class UltimateTankSpec : public UltimateAbilitySpec {
  public:
    AssetRefEntityPrototype tankPrototypeAssetRef;
    GENERATED_WEAPON_ID weaponID{};
    FPVector3 weaponPositionOffset = FPVector3(FP::FromRaw(0), FP::FromRaw(32768), FP::FromRaw(0));
    FP lifetime = FP::FromInt(10);
    FP healthDecreaseTickRate = FP::FromInt(1);

    // 0x7be5c58
    UltimateTankSpec() = default;

    // TODO(0x7be5c50, Quantum.UltimateTankSpec.get_WeaponID): GENERATED_WEAPON_ID get_WeaponID()
};

class UltimateWeaponSpec : public UltimateAbilitySpec {
  public:
    enum class ModificationType : std::int32_t {
        Replace = 0,
        Add = 1,
        Substract = 2,
        Multiply = 3,
    };

    enum class UpgradeType : std::int32_t {
        Damage = 0,
        Reload = 1,
        Ammunition = 2,
    };

    struct StatUpgrade {
        UltimateWeaponSpec::ModificationType modificationType{};
        UltimateWeaponSpec::UpgradeType type{};
        std::int16_t valuel{};
    };

    struct Upgrade {
        std::vector<UltimateWeaponSpec::StatUpgrade> statUpgrades;
    };

    GENERATED_WEAPON_ID weaponID{};
    bool finishUltimateOnWeaponUse{};
    FP finishUltimateOnWeaponUseDelay{};
    std::vector<UltimateWeaponSpec::Upgrade> UpgradePerLevel;

    // 0x7be6c64
    UltimateWeaponSpec() = default;

    // TODO(0x7be6c5c, Quantum.UltimateWeaponSpec.get_WeaponID): GENERATED_WEAPON_ID get_WeaponID()
};

class VoidRepulserWeaponSpec : public WeaponSpec {
  public:

    // 0x7bfdad8
    VoidRepulserWeaponSpec() = default;
};

class WeaponSpecDefault : public WeaponSpec {
  public:

    // 0x7c1184c
    WeaponSpecDefault() = default;
};

class WrenchWeaponSpec : public WeaponSpec {
  public:
    FP OrientationSharpness = FP::FromInt(10);
    FP MaxSpeedModifierHit = FP::FromRaw(-32768); // -0.5
    FP InputFalloff = FP::FromRaw(6553); // 0.0999908
    FP MinInputSqrMagForRegistration = FP::FromRaw(32768); // 0.5
    FP MaxTargetAngleCorrection = FP::FromInt(15);
    FPAnimationCurve ImpulseMultiplierFromDistanceToTarget;
    std::vector<MeleeComboAttackData> ComboAttacks;
    bool EnableCliffDetection = true;
    FP CliffDetectionDistance = FP::FromRaw(137625); // 2.09999
    FP CliffDetectionHeight = FP::FromInt(2);
    FP CliffDistanceMargin = FP::FromRaw(16384); // 0.25

    // 0x7c00f90
    WrenchWeaponSpec() = default;

    // TODO(0x7c00ea4, Quantum.WrenchWeaponSpec.CanFireWeapon): bool CanFireWeapon(Frame frame,
    //   EntityRef weaponEntity)
};

class SmokeScreenSpec : public AddForceOnUltimateSpec {
  public:

    // 0x7be5950
    SmokeScreenSpec() = default;
};

} // namespace Quantum
