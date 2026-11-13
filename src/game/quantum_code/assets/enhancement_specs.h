// enhancement_specs.h - weapon enhancements and hero improvements (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/weapon_specs.h"

namespace Quantum {

class EnhancementSpec : public AssetObject {
  public:

    // 0x7c9349c
    EnhancementSpec() = default;
};

class ImprovementSpec : public AssetObject {
  public:
    std::vector<DamageModifier> DamageModifiers;

    // 0x7c92704
    ImprovementSpec() = default;

    // TODO(0x7c92700, Quantum.ImprovementSpec.OnInit): void OnInit(Frame f, EntityRef heroEntity,
    //   EntityRef weaponEntity)
};

class CompoundHeroImprovementSpec : public ImprovementSpec {
  public:
    std::vector<AssetRefImprovementSpec> Improvements;

    // 0x7bd3c24
    CompoundHeroImprovementSpec() = default;

    // TODO(0x7bd3a1c, Quantum.CompoundHeroImprovementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
    // TODO(0x7bd3b18, Quantum.CompoundHeroImprovementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   heroEntity, EntityRef weaponEntity)
};

class DamageMultiplierHeroImprovementSpec : public ImprovementSpec {
  public:
    AssetRefDamageMultiplierSpec DamageMultiplierSpec;

    // 0x7bd3c94
    DamageMultiplierHeroImprovementSpec() = default;

    // TODO(0x7bd3c2c, Quantum.DamageMultiplierHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class DashCooldownReductionHeroImprovementSpec : public ImprovementSpec {
  public:
    FP DashCooldownReduction = FP::FromRaw(32768); // 0.5

    // 0x7bd3d3c
    DashCooldownReductionHeroImprovementSpec() = default;

    // TODO(0x7bd3c9c, Quantum.DashCooldownReductionHeroImprovementSpec.OnInit): void OnInit(Frame
    //   f, EntityRef heroEntity)
};

class ElementalBuildUpMultiplierHeroImprovementSpec : public ImprovementSpec {
  public:
    bool AddElectricalBuildUpMultiplier{};
    FP ElectricalBuildUpMultiplier = FP::FromInt(1);
    bool AddIceBuildUpMultiplier{};
    FP IceBuildUpMultiplier = FP::FromInt(1);

    // 0x7bd3df8
    ElementalBuildUpMultiplierHeroImprovementSpec() = default;

    // TODO(0x7bd3d4c, Quantum.ElementalBuildUpMultiplierHeroImprovementSpec.OnInit): void
    //   OnInit(Frame f, EntityRef heroEntity)
};

class HeroEnhancementSpec : public EnhancementSpec {
  public:

    // 0x7beee10
    HeroEnhancementSpec() = default;
};

class JumpAttackHeroImprovementSpec : public ImprovementSpec {
  public:
    AssetRefMeleeJumpAttackSpec MeleeJumpAttackSpec;

    // 0x7bd4008
    JumpAttackHeroImprovementSpec() = default;

    // TODO(0x7bd3e0c, Quantum.JumpAttackHeroImprovementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   heroEntity)
    // TODO(0x7bd3e10, Quantum.JumpAttackHeroImprovementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   heroEntity, EntityRef weaponEntity)
};

class JumpMultiplierHeroImprovementSpec : public ImprovementSpec {
  public:
    FP BaseJumpMultiplier{};

    // 0x7bd4090
    JumpMultiplierHeroImprovementSpec() = default;

    // TODO(0x7bd4010, Quantum.JumpMultiplierHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class OnKillCallbacksHeroImprovementSpec : public ImprovementSpec {
  public:
    std::vector<AssetRefCallbackAction> OnPlayerKilled;

    // 0x7bd41b0
    OnKillCallbacksHeroImprovementSpec() = default;

    // TODO(0x7bd4098, Quantum.OnKillCallbacksHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class PlaceholderHeroImprovementSpec : public ImprovementSpec {
  public:

    // 0x7bd41bc
    PlaceholderHeroImprovementSpec() = default;

    // TODO(0x7bd41b8, Quantum.PlaceholderHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class ReplaceHealthSpecHeroImprovementSpec : public ImprovementSpec {
  public:
    AssetRefHealthSpec HealthSpec;

    // 0x7bd41c8
    ReplaceHealthSpecHeroImprovementSpec() = default;

    // TODO(0x7bd41c4, Quantum.ReplaceHealthSpecHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class WeaponEnhancementSpec : public EnhancementSpec {
  public:
    enum class APPLY_TO : std::int32_t {
        SPECIFIC_WEAPON = 0,
        SPECIFIC_SLOT = 1,
    };

    std::vector<DamageModifier> DamageModifiers;
    WeaponEnhancementSpec::APPLY_TO ApplyTo{};
    GENERATED_WEAPON_ID Weapon_ID{};
    WeaponLoadoutSlot Slot{};

    // 0x7beed88
    WeaponEnhancementSpec() = default;
};

class WeaponStatsModifierHeroImprovementSpec : public ImprovementSpec {
  public:
    std::vector<WeaponStatModifier> WeaponStatsModifiers;
    bool UseFilter{};
    std::vector<GENERATED_WEAPON_ID> WeaponFilter;
    bool IncludeUltimates{};

    // 0x7bd449c
    WeaponStatsModifierHeroImprovementSpec() = default;

    // TODO(0x7bd41d0, Quantum.WeaponStatsModifierHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity, EntityRef weaponEntity)
    // TODO(0x7bd4498, Quantum.WeaponStatsModifierHeroImprovementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef heroEntity)
};

class AddDamageIncreaseOnTargetedEffectSpec : public WeaponEnhancementSpec {
  public:

    // 0x7bed538
    AddDamageIncreaseOnTargetedEffectSpec() = default;

    // TODO(0x7bed4dc, Quantum.AddDamageIncreaseOnTargetedEffectSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class AddEntitiesToSpawnOnProjectileDeathSpec : public WeaponEnhancementSpec {
  public:
    std::vector<AssetRefEntityPrototype> EntityPrototypesToSpawn;

    // 0x7b7cff4
    AddEntitiesToSpawnOnProjectileDeathSpec() = default;

    // TODO(0x7b7cf64, AddEntitiesToSpawnOnProjectileDeathSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class AmmoMultiplierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP BonusMultiplier = FP::FromRaw(16384); // 0.25

    // 0x7b7d0e0
    AmmoMultiplierEnhancementSpec() = default;

    // TODO(0x7b7cffc, AmmoMultiplierEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class AmoeboidLauncherEnhancementPuddleSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype AcidPuddleEntityPrototype;

    // 0x7beee8c
    AmoeboidLauncherEnhancementPuddleSpec() = default;

    // TODO(0x7beee18, Quantum.AmoeboidLauncherEnhancementPuddleSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class BlasterMoveSpeedEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP MoveSpeedModifier{};

    // 0x7befc20
    BlasterMoveSpeedEnhancementSpec() = default;

    // TODO(0x7befba8, Quantum.BlasterMoveSpeedEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class BlasterSpecialShotEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype SpecialAmmoAsset;
    std::uint32_t SpecialShotFrequency = 5u;

    // 0x7bf026c
    BlasterSpecialShotEnhancementSpec() = default;

    // TODO(0x7bf01b4, Quantum.BlasterSpecialShotEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class CompoundEnhancementSpec : public WeaponEnhancementSpec {
  public:
    std::vector<AssetRefEnhancementSpec> Enhancements;

    // 0x7becf30
    CompoundEnhancementSpec() = default;

    // TODO(0x7bece34, Quantum.CompoundEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class CooldownMultiplierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP Multiplier = FP::FromRaw(49152); // 0.75

    // 0x7b7d1b4
    CooldownMultiplierEnhancementSpec() = default;

    // TODO(0x7b7d0f0, CooldownMultiplierEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class DamageIncreasedOnTargetsEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP Duration{};
    FP DamagePercentIncrease{};

    // 0x7becfb0
    DamageIncreasedOnTargetsEnhancementSpec() = default;

    // TODO(0x7becf38, Quantum.DamageIncreasedOnTargetsEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class EnforcerFirstShotBonusEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP MoveSpeedBonusValue{};
    FP MoveSpeedBonusDuration{};

    // 0x7bf6aac
    EnforcerFirstShotBonusEnhancementSpec() = default;

    // TODO(0x7bf6a38, Quantum.EnforcerFirstShotBonusEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class FirerateMultiplierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP BonusMultiplier = FP::FromRaw(16384); // 0.25

    // 0x7b7d290
    FirerateMultiplierEnhancementSpec() = default;

    // TODO(0x7b7d1c4, FirerateMultiplierEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class GatlingEnhancementResetAmmoOnGadgetUseSpec : public WeaponEnhancementSpec {
  public:

    // 0x7bf6efc
    GatlingEnhancementResetAmmoOnGadgetUseSpec() = default;

    // TODO(0x7bf6e30, Quantum.GatlingEnhancementResetAmmoOnGadgetUseSpec.OnInit): void OnInit(Frame
    //   f, EntityRef weaponEntity)
};

class GatlingEnhancementSpec : public WeaponEnhancementSpec {
  public:
    GatlingWeaponSpec::GatlingLevel AdditionalLevel;

    // 0x7bf6e2c
    GatlingEnhancementSpec() = default;

    // TODO(0x7bf6d5c, Quantum.GatlingEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class GrenadeLauncherAftershockEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefProjectileSpawnerSpec ProjectileSpawnerSpec;

    // 0x7bf8058
    GrenadeLauncherAftershockEnhancementSpec() = default;

    // TODO(0x7bf7f10, Quantum.GrenadeLauncherAftershockEnhancementSpec.OnInit): void OnInit(Frame
    //   f, EntityRef weaponEntity)
};

class GrenadeLauncherStunGrenadeEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP PushModifierStatusEffect{};
    FP PushModifierStatusEffectDuration{};
    bool PushModifierStatusEffectRevertAfterPush{};

    // 0x7bf80d0
    GrenadeLauncherStunGrenadeEnhancementSpec() = default;

    // TODO(0x7bf805c, Quantum.GrenadeLauncherStunGrenadeEnhancementSpec.OnInit): void OnInit(Frame
    //   f, EntityRef weaponEntity)
};

class HeadHunterWallHackEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP ScreenRadiusDetection = FP::FromInt(1);

    // 0x7bf86e0
    HeadHunterWallHackEnhancementSpec() = default;

    // TODO(0x7bf866c, Quantum.HeadHunterWallHackEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class HealthMultiplierEnhancementSpec : public HeroEnhancementSpec {
  public:
    FP HealthMultiplier = FP::FromRaw(78643); // 1.2

    // 0x7b7d2f4
    HealthMultiplierEnhancementSpec() = default;

    // TODO(0x7b7d2a0, HealthMultiplierEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   heroEntity)
};

class HoloshieldGloveHealZoneEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype HealZoneEntityPrototype;

    // 0x7bf8760
    HoloshieldGloveHealZoneEnhancementSpec() = default;

    // TODO(0x7bf86ec, Quantum.HoloshieldGloveHealZoneEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class HoverBootsEnhancementCollisionSpec : public WeaponEnhancementSpec {
  public:
    AssetRefDamageMultiplierSpec DamageMultiplierSpec;

    // 0x7bf9740
    HoverBootsEnhancementCollisionSpec() = default;

    // TODO(0x7bf96d8, Quantum.HoverBootsEnhancementCollisionSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class IgnoreEntitiesMultiplierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP Multiplier = FP::FromRaw(49152); // 0.75

    // 0x7b7d3c0
    IgnoreEntitiesMultiplierEnhancementSpec() = default;

    // TODO(0x7b7d308, IgnoreEntitiesMultiplierEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class MineLauncherEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP MergeDistanceThreshold{};
    AssetRefCallbackAction SpawnEntitiesActionOverride;

    // 0x7bfaa30
    MineLauncherEnhancementSpec() = default;

    // TODO(0x7bfa9bc, Quantum.MineLauncherEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class MineLauncherProjectileOverrideEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype ProjectileOverride;

    // 0x7bfaaa8
    MineLauncherProjectileOverrideEnhancementSpec() = default;

    // TODO(0x7bfaa34, Quantum.MineLauncherProjectileOverrideEnhancementSpec.OnInit): void
    //   OnInit(Frame f, EntityRef weaponEntity)
};

class RecoverAmmoOnDashEnhancementSpec : public WeaponEnhancementSpec {
  public:
    std::int32_t AmmoRecoverAmount = 3;

    // 0x7b7d444
    RecoverAmmoOnDashEnhancementSpec() = default;

    // TODO(0x7b7d3d0, RecoverAmmoOnDashEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class ReplaceMultiShotConfigsEnhancementSpec : public WeaponEnhancementSpec {
  public:
    std::vector<WeaponSpec::MultiShotConfig> MultiShotConfigs;
    bool consumeOneAmmoPerMultiShot{};

    // 0x7bede34
    ReplaceMultiShotConfigsEnhancementSpec() = default;

    // TODO(0x7beda08, Quantum.ReplaceMultiShotConfigsEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class ShotgunBerserkerEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype EnhancedAmmoAsset;

    // 0x7bfc078
    ShotgunBerserkerEnhancementSpec() = default;

    // TODO(0x7bfc000, Quantum.ShotgunBerserkerEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class ShotgunEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype EnhancedAmmoAsset;

    // 0x7bfbffc
    ShotgunEnhancementSpec() = default;

    // TODO(0x7bfbf84, Quantum.ShotgunEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

class TeslaClawMoreBeamEnhancementSpec : public WeaponEnhancementSpec {
  public:
    std::int32_t AdditionalBeams = 1;
    AssetRefDamageSpec BeamDamageSpec;

    // 0x7bfc938
    TeslaClawMoreBeamEnhancementSpec() = default;

    // TODO(0x7bfc8c4, Quantum.TeslaClawMoreBeamEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class VoidRepulserBulldozerEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP MoveSpeedBonusDelay = FP::FromInt(1);
    FP MoveSpeedModifier = FP::FromRaw(19659); // 0.299973

    // 0x7bfd544
    VoidRepulserBulldozerEnhancementSpec() = default;

    // TODO(0x7bfd4cc, Quantum.VoidRepulserBulldozerEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class VoidRepulserSpeedBoostEnhancementSpec : public WeaponEnhancementSpec {
  public:
    FP MoveSpeedModifier{};
    FP MoveSpeedDuration{};

    // 0x7bfe280
    VoidRepulserSpeedBoostEnhancementSpec() = default;

    // TODO(0x7bfe20c, Quantum.VoidRepulserSpeedBoostEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class VoidRepulserVoidWaveEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype OnReleaseAmmunitionAsset;

    // 0x7bfe53c
    VoidRepulserVoidWaveEnhancementSpec() = default;

    // TODO(0x7bfe4c4, Quantum.VoidRepulserVoidWaveEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class WarmongerMiniRocketsEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype EnhancedAmmoAsset;
    std::int32_t AdditionalAmmoCount{};

    // 0x7bfe91c
    WarmongerMiniRocketsEnhancementSpec() = default;

    // TODO(0x7bfe8a8, Quantum.WarmongerMiniRocketsEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class WeaponChargingStatsModifierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    ChargingStats AdditionalChargingStats;

    // 0x7bedf74
    WeaponChargingStatsModifierEnhancementSpec() = default;

    // TODO(0x7bedebc, Quantum.WeaponChargingStatsModifierEnhancementSpec.OnInit): void OnInit(Frame
    //   f, EntityRef weaponEntity)
};

class WeaponProjectileSwitcherEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype AlternateAmmunitionAsset;
    std::uint32_t AmmunitionChangeModulo = 1u;
    bool DestroyPreviousAmmoInstanceOnSwitch{};
    bool SwitchAmmoAfterUseTime{};
    FP SwitchAmmoAfterUseTimeDelay{};

    // 0x7bee034
    WeaponProjectileSwitcherEnhancementSpec() = default;

    // TODO(0x7bedf7c, Quantum.WeaponProjectileSwitcherEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class WeaponStatsModifierEnhancementSpec : public WeaponEnhancementSpec {
  public:
    std::vector<WeaponStatModifier> WeaponStatsModifiers;

    // 0x7beed84
    WeaponStatsModifierEnhancementSpec() = default;

    // TODO(0x7beebf4, Quantum.WeaponStatsModifierEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class WrenchGroundSlamEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefEntityPrototype GroundAoePrototype;

    // 0x7c001a8
    WrenchGroundSlamEnhancementSpec() = default;

    // TODO(0x7c00134, Quantum.WrenchGroundSlamEnhancementSpec.OnInit): void OnInit(Frame f,
    //   EntityRef weaponEntity)
};

class WrenchThrowEnhancementSpec : public WeaponEnhancementSpec {
  public:
    AssetRefTargetFinderSpec TargetFinderSpec;
    AssetRefEntityPrototype ThrowableAmmoAsset;
    AssetRefEntityPrototype DefaultAmmoAsset;
    FP HoldInputDelay = FP::FromInt(1);

    // 0x7c00128
    WrenchThrowEnhancementSpec() = default;

    // TODO(0x7c000b0, Quantum.WrenchThrowEnhancementSpec.OnInit): void OnInit(Frame f, EntityRef
    //   weaponEntity)
};

} // namespace Quantum
