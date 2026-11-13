// gameplay_specs.h - gameplay spec assets: damage, projectiles, areas of effect, targeting, pushing, drops (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/ai_behaviours.h"

namespace Quantum {

class AmmunitionDamageSpec : public AssetObject {
  public:
    bool UseRangeToDamageMultCurve{};
    FPAnimationCurve RangeToDamageMultCurve;

    // 0x7c8fd28
    AmmunitionDamageSpec() = default;
};

class AoeTrailSpec : public AssetObject {
  public:
    FP AoeTrailSegmentMinimalLength = FP::FromRaw(32768); // 0.5
    FP AoeTrailLifetime = FP::FromInt(5);
    FPVector3 localSpawnOffset;
    bool OnlyGenerateOnGround = true;
    FP MaxGroundDistance = FP::FromRaw(32768); // 0.5
    LayerMask GroundMask;
    FP GroundYOffset{};
    bool GeneratePointOnCollision = true;
    bool OnlyVerticalCollision = true;
    bool GeneratePointAboveAngleEnabled = true;
    FP GeneratePointAboveAngle = FP::FromInt(5);

    // 0x7c8dda0
    AoeTrailSpec() = default;
};

class AreaOfEffectIgnitionSpec : public AssetObject {
  public:
    AssetRefEntityPrototype IgnitionAoeEntityPrototype;

    // 0x7c8df24
    AreaOfEffectIgnitionSpec() = default;
};

class AreaOfEffectLevelUpSpec : public AssetObject {
  public:
    AssetRefEntityPrototype EntityPrototypeRef;
    std::int32_t MaxLevelUpOccurrences = 1;

    // 0x7c8df2c
    AreaOfEffectLevelUpSpec() = default;
};

class AreaOfEffectSpec : public AssetObject {
  public:
    enum class ForceDirectionType : std::int32_t {
        FromCenter = 0,
        FromCenterWithLateralCoef = 1,
        AlongAxis = 2,
        AlwaysProjectileVelocity = 3,
        AwayFromShooter = 4,
    };

    Shape3DConfig ShapeConfig;
    bool StartsEnabled = true;
    FP StartDelay{};
    bool IsContinuous{};
    bool IgnoreAlreadyEffectedEntities{};
    FP IgnoreEffectedEntitiesDuration = FP::FromInt(1);
    bool AutoRemoveEffectedEntityIfOutOfArea{};
    FPVector3 InitialScale = FPVector3::One();
    bool IsInfinite{};
    FP Duration = FP::FromInt(1);
    bool GrowsOverDuration{};
    FP MinGrowthRange{};
    FP MaxGrowthRange = FP::FromInt(1);
    bool CanAffectEnemies = true;
    bool CanAffectTeam{};
    bool CanAffectInstigator{};
    bool NeedsLineOfSightCheck = true;
    QueryOptions LineOfSightQueryOptions = QueryOptions::HitAll;
    bool LimitScaleToOne = true;
    bool ScaleRequiresRaycast{};
    LayerMask ScaleRaycastLayerMask = LayerMask{-1};
    bool ConstrainToForwardAngle{};
    FP ForwardAngleConstraintDegrees = FP::FromInt(30);
    bool ForwardAngleConstraintIs2D{};
    bool ConstrainToHeightOffset{};
    FP MaxHeightDifference = FP::FromInt(1);
    bool ConstrainToInnerRadiusOffset{};
    FP InnerRadiusOffset = FP::FromInt(1);
    bool ProjectileForceDirectionIsFlat{};
    AreaOfEffectSpec::ForceDirectionType DirectionType{};
    FP LateralForceCoef = FP::FromRaw(6553); // 0.0999908
    FPVector3 AlongAxisDirection = FPVector3::Forward();
    FP AwayFromShooterMaxRange = FP::FromInt(1);
    bool NotifyViewOnHit{};
    bool NotifyViewOnMiss{};
    bool NotifyInstigatorOnHit{};
    bool SetupEntityPhysicsCollider3D{};

    // 0x7c8de38
    AreaOfEffectSpec() = default;
};

class BigBoomMineSpec : public AssetObject {
  public:
    LayerMask CollisionLayerMask;
    FP FallingSpeed = FP::FromInt(10);
    FP ExplosionDelay{};

    // 0x7c8df3c
    BigBoomMineSpec() = default;
};

class BlastBallSpec : public AssetObject {
  public:
    FP maxSpeed = FP::FromInt(10);
    FP meleeWeaponImpulseMultiplier = FP::FromInt(1);
    FP dragWhenShot = FP::FromRaw(32768); // 0.5
    FP dragRecoveryDelay{};
    FP dragRecoveryDuration = FP::FromInt(1);
    AssetRefDamageSpec constantDashDamageSpec;
    FP speedModifierOnDashEnded = FP::FromRaw(-49152); // -0.75
    FP speedModifierOnDashEndedDuration = FP::FromInt(1);
    FP dashInfluenceDistance = FP::FromInt(5);
    FPAnimationCurve meleeCorrection;
    FP meleeHitRotationLockTime{};
    FP settledSpeedThreshold = FP::FromInt(1);
    AssetRefPhysicsMaterial bouncyPhysicsMaterial;
    AssetRefPhysicsMaterial settledPhysicsMaterial;
    FP wallBounceForce = FP::FromInt(10);

    // 0x7c8df70
    BlastBallSpec() = default;

    // 0x7c8df50
    FP maxSpeedSqr() const { return maxSpeed * maxSpeed; }
    // 0x7c8df60
    FP SettledSpeedThresholdSqr() const { return settledSpeedThreshold * settledSpeedThreshold; }
};

class BombardierDroneSpec : public AssetObject {
  public:
    FP MoveSpeed = FP::FromInt(2);
    FP TakeOffDistance = FP::FromInt(2);
    FP TakeOffHeight = FP::FromInt(5);
    FPAnimationCurve TakeOffCurve;
    FP FlightDistance = FP::FromInt(10);
    LayerMask CollisionLayerMask;
    bool CanSteer{};
    FP SteeringMaxAnglePerSecond = FP::FromInt(90);
    AssetRefEntityPrototype BombAsset;
    BombDropSide BombFirstDropSide = BombDropSide::Right;
    FP BombDropSideOffset = FP::FromInt(1);
    std::int32_t BombDropCount{};
    FP BombDropMaxHeight = FP::FromInt(20);
    bool BombDropOnFlightStart{};
    bool BombDropOnFlightEnd{};
    bool DroppingOnBeingDestroyed{};
    FP DroppingGravity = FP::FromInt(10);
    FP DroppingMaxDuration = FP::FromInt(10);
    AssetRefEntityPrototype DroppingExplosionAsset;

    // 0x7c8e04c
    BombardierDroneSpec() = default;

    // 0x7c8dfe4 - time to climb TakeOffHeight at MoveSpeed.
    FP TakeOffDuration() const;
    // 0x7c8dff4 - time to cover FlightDistance at MoveSpeed.
    FP FlightDuration() const;
    // 0x7c8e008 - take-off, flight and the longest bombing run.
    FP MaxLifetime() const;
    // 0x7c8e030 - how far the drone can get: take-off height and flight distance plus the bombing run at MoveSpeed.
    FP TotalRange() const;
};

class BombardierWeaponAutoTargetSpec : public AssetObject {
  public:
    FP BombRadiusToAreaFactor = FP::FromInt(1);
    bool ShowGizmos{};

    // 0x7c8e098
    BombardierWeaponAutoTargetSpec() = default;
};

class BuzzbladesRicochetSpec : public AssetObject {
  public:
    FP Radius = FP::FromInt(10);
    std::int32_t MaxHitCountIncrementPerNewEnemy = 1;
    std::int32_t ExtendedMaxHitCount = 6;

    // 0x7c8e1e0
    BuzzbladesRicochetSpec() = default;
};

class CallbackAction : public AssetObject {
  public:

    // 0x7c8e1fc
    CallbackAction() = default;
};

class CollisionForceSpec : public AssetObject {
  public:
    bool collideIfMovingTowardsTargetOnly{};
    bool excludeDashingPlayers{};
    bool useHitAndDamageDataFromComponentsSpecs{};
    bool damageAllies{};
    bool fixedDamage{};
    FP maxDamage = FP::FromInt(10);
    FPAnimationCurve speedToDamageCurve;
    FP maxSpeed = FP::FromInt(10);
    bool pushAllies{};
    FP pushDirectionVerticalBonus{};
    bool fixedPush{};
    FP maxPush = FP::FromInt(10);
    FPAnimationCurve speedToPushCurve;

    // 0x7c8e240
    CollisionForceSpec() = default;
};

class CryosliderSpec : public AssetObject {
  public:
    bool killWhenWeaponRangeTravelled{};
    bool killAboveMaxFallHeight{};
    FP maxFallHeight = FP::FromInt(5);
    bool UseHoming{};
    FP HomingAngularSpeed = FP::FromInt(90);

    // 0x7c8e260
    CryosliderSpec() = default;
};

class CryosliderWeaponAutoTargetSpec : public AssetObject {
  public:
    FP maxYForValidTarget = FP::FromInt(5);
    FP minYForValidTarget = FP::FromInt(-1);
    FP minZForValidTarget = FP::FromInt(2);
    bool useWidthFromAoeTrail = true;
    FP autoTargetWidth = FP::FromInt(5);
    bool ShowGizmos{};

    // 0x7c8e278
    CryosliderWeaponAutoTargetSpec() = default;
};

class DamageChargeableSpec : public AssetObject {
  public:
    FP MaxChargeValue = FP::FromInt(100);
    FP InactivityResetDelay = FP::FromInt(5);
    AssetRefLevelButtonGameplayElementConfig onFullyCharged;
    FP ActivationDuration = FP::FromInt(5);
    AssetRefLevelButtonGameplayElementConfig onTimerElapsed;

    // 0x7c8fd30
    DamageChargeableSpec() = default;
};

class DamageMultiplierSpec : public AssetObject {
  public:
    enum class DamageFilterType : std::int32_t {
        ALL = 0,
        ALL_WEAPONS = 1,
        SPECIFIC_WEAPON = 2,
        WEAPON_CATEGORY = 3,
    };

    FP Multiplier{};
    DamageMultiplierSpec::DamageFilterType DamageFilter{};
    GENERATED_WEAPON_ID WeaponID{};
    bool UseDamageType{};
    ::Quantum::DamageType DamageType{};

    // 0x7c8fd4c
    DamageMultiplierSpec() = default;
};

class DamageSpec : public AssetObject {
  public:
    enum class DamageAmountMode : std::int32_t {
        FIXED = 0,
        USE_AVERAGE_HUMAN_HEROES_LEVEL = 1,
        USE_WEAPON_LEVEL = 2,
        USE_HERO_LEVEL = 3,
        USE_PVE_SCALING = 4,
        USE_SELF_LEVEL = 5,
    };

    ENTITY_INFO_FILTER EntityFilter = ENTITY_INFO_FILTER::All;
    ::Quantum::DamageType DamageType = ::Quantum::DamageType::Normal;
    FP ElectricalBuildUpPerHit = FP::FromInt(10);
    FP IceBuildUpPerHit = FP::FromInt(10);
    FP BaseUnscaledAmount{};
    DamageSpec::DamageAmountMode Mode{};
    FP Amount{};
    FP BaseAmount{};
    FP MinAmount{};
    FP MaxAmount{};
    std::vector<FP> AmountPerLevel = std::vector<FP>(10);
    std::vector<FP> AmountPerSelfLevel;
    bool DamageShield = true;
    bool BreakShield{};
    FP ShieldDamageMultiplier = FP::FromInt(1);
    bool ApplyMoveSpeedModifier = true;
    bool DamageHealth = true;
    FP HealthDamageMultiplier = FP::FromInt(1);
    bool CanKill = true;
    bool UseReticleFeedback = true;
    bool SlowDownPlayerActor = true;

    // 0x7c8f920
    DamageSpec() = default;

    // TODO(0x7c8e2a4, Quantum.DamageSpec.CreateHitData): HitData CreateHitData(Frame frame,
    //   FPVector3 position, FPVector3 direction, EntityRef sourceEntity, EntityRef targetEntity);
    //   uses DamageLib.TryGetDamageMultiplier, DamageSpec.CalculateDamages,
    //   DamageSpec.GetDamageModifier, Ownership.GetOwnershipOrDefault, QBoolean.op_Implicit
    // TODO(0x7c8e70c, Quantum.DamageSpec.CalculateDamages): FP CalculateDamages(Frame frame,
    //   HitData hitData, EntityRef targetEntity); uses FPMath.Min, FPVector3.Distance,
    //   DamageLib.GetTotalDamageMultiplier, DamageSpec.CalculateRawDamages,
    //   FPAnimationCurve.Evaluate, PowerUpUser.get_ActivePowerUps,
    //   ProjectileLib.ComputeProjectileDamageMultiplier, Unsafe.TryGetWeaponUser
    // TODO(0x7c8f030, Quantum.DamageSpec.CalculateRawDamages): FP CalculateRawDamages(Frame frame,
    //   HitData hitData); uses FPMath.Clamp, FPMath.Lerp, BalancingLib.TryGetHumanHeroesLevelRatio,
    //   WeaponLib.TryGetWeaponLevel
    // TODO(0x7c8edc8, Quantum.DamageSpec.GetDamageModifier): void GetDamageModifier(Frame frame,
    //   Ownership sourceOwnership, DamageModifier> resultDamageModifier); uses
    //   AssetGuid.get_IsValid, BalancingLib.TryGetHeroSpec,
    //   DamageSpec.GetDamageModifiersFromHeroImprovements,
    //   DamageSpec.GetDamageModifiersFromWeaponEnhancement
    // TODO(0x7c8f4e8, Quantum.DamageSpec.GetDamageModifiersFromWeaponEnhancement): void
    //   GetDamageModifiersFromWeaponEnhancement(Frame frame, WeaponEnhancementSpec enhancementSpec,
    //   EntityRef sourceWeapon, DamageModifier> damageModifierList); uses AssetGuid.op_Inequality,
    //   AssetObject.get_Guid, SimulationWeaponTable.TryGetWeaponId
    // TODO(0x7c8f2cc, Quantum.DamageSpec.GetDamageModifiersFromHeroImprovements): void
    //   GetDamageModifiersFromHeroImprovements(Frame frame, ImprovementSpec improvementSpec,
    //   DamageModifier> damageModifierList); uses AssetGuid.op_Inequality, AssetObject.get_Guid
};

class DamageToImpulseConverterSpec : public AssetObject {
  public:
    enum class ConversionMode : std::int32_t {
        None = 0,
        Constant = 1,
        Curve = 2,
    };

    struct AoEMultiplier {
        AssetRefAreaOfEffectSpec AoE;
        FP Multiplier{};
        bool UseLateralImpactDirectionOnly{};

        // TODO(0x7c8fe88, Quantum.DamageToImpulseConverterSpec.AoEMultiplier.Equals): bool
        //   Equals(object obj)
        // TODO(0x7c8ff04, Quantum.DamageToImpulseConverterSpec.AoEMultiplier.GetHashCode): int
        //   GetHashCode()
    };

    struct WeaponMultiplier {
        GENERATED_WEAPON_ID WeaponID{};
        FP Multiplier{};
        bool UseLateralImpactDirectionOnly{};

        // TODO(0x7c8fd90, Quantum.DamageToImpulseConverterSpec.WeaponMultiplier.Equals): bool
        //   Equals(object obj)
        // TODO(0x7c8fe10, Quantum.DamageToImpulseConverterSpec.WeaponMultiplier.GetHashCode): int
        //   GetHashCode()
    };

    DamageToImpulseConverterSpec::ConversionMode mode{};
    FP constantMinDamage = FP::FromInt(1);
    FP constantMaxDamage = FP::FromInt(3);
    FP curveMinDamage = FP::FromInt(100);
    FP curveMaxDamage = FP::FromInt(1000);
    FPAnimationCurve damageToImpulseCurve;
    FP curveMinValue = FP::FromInt(2);
    FP curveMaxValue = FP::FromRaw(13107); // 0.199997
    std::vector<DamageToImpulseConverterSpec::WeaponMultiplier> weaponMultipliers;
    std::vector<DamageToImpulseConverterSpec::AoEMultiplier> aoeMultipliers;
    bool useImpactDirection{};
    FP impactDirectionInterpolation = FP::FromInt(1);
    FP impactVerticalDirectionMult = FP::FromInt(1);
    FP impactHorizontalDirectionMult = FP::FromInt(1);

    // 0x7c8fd54
    DamageToImpulseConverterSpec() = default;
};

class DroppableManagerSpec : public AssetObject {
  public:
    struct DroppableSpec {
        struct CollectSpec {
            FP DelayBeforeCollection = FP::FromRaw(32768); // 0.5
            FP Range = FP::FromInt(8);
            FP ShortRange = FP::FromInt(3);
            FP Duration = FP::FromRaw(49152); // 0.75
            FP DelayForDropper = FP::FromInt(2);
            std::int32_t DetectionFrequency = 10;
            FP TimeWindowDuration = FP::FromRaw(21845); // 0.333328

            // 0x7c903fc
            CollectSpec() = default;
        };

        struct DropSpec {
            FP Duration = FP::FromInt(1);
            EasingType XZEasing = EasingType::OutQuad;
            EasingType YEasing = EasingType::OutBounce;
            FP YAmplitude = FP::FromInt(1);
            FP MinRange = FP::FromRaw(98304); // 1.5
            FP MaxRange = FP::FromInt(3);

            // 0x7c903cc
            DropSpec() = default;
        };

        struct ShrinkingZoneSpec {
            FP Margin = FP::FromInt(2);
            FP Delay = FP::FromInt(2);

            // 0x7c90430
            ShrinkingZoneSpec() = default;
        };

        DroppableManagerSpec::DroppableSpec::DropSpec Drop;
        DroppableManagerSpec::DroppableSpec::CollectSpec Collect;
        DroppableManagerSpec::DroppableSpec::ShrinkingZoneSpec ShrinkingZone;

        // 0x7c9028c
        DroppableSpec() = default;
    };

    struct DroppableOverride {
        std::vector<DroppableType> Types;
        DroppableManagerSpec::DroppableSpec Spec;
    };

    DroppableManagerSpec::DroppableSpec DefaultDroppableSpec;
    std::vector<DroppableManagerSpec::DroppableOverride> DroppableSpecOverrides;
    std::unordered_map<DroppableType, DroppableManagerSpec::DroppableSpec> droppableSpecsByType; // not stored
    AssetRefPowerUpSpec PowerUpOverhealthAssetRef;
    AssetRefPowerUpSpec PowerUpExtraDamageAssetRef;
    AssetRefPowerUpSpec PowerUpExtraDamagePVEAssetRef;
    AssetRefPowerUpSpec PowerUpSpeedBoostAssetRef;
    AssetRefPowerUpSpec PowerUpPermanentOverhealthAssetRef;

    // 0x7c90204
    DroppableManagerSpec() = default;

    // TODO(0x7c8ff78, Quantum.DroppableManagerSpec.GetDroppableSpec): DroppableSpec
    //   GetDroppableSpec(DroppableType type)
    // TODO(0x7c900d8, Quantum.DroppableManagerSpec.TryGetPowerUpSpec): bool TryGetPowerUpSpec(Frame
    //   f, DroppableType type, PowerUpSpec spec); uses AssetGuid.get_IsValid
};

class ElementalDamageTable : public AssetObject {
  public:
    std::vector<DamageTypeToElementMapping> mappings;
    AssetRefEntityPrototype fireDamagePrototype;
    ENTITY_INFO_FILTER fireDamageEntityFilter = ENTITY_INFO_FILTER::All;
    AssetRefAreaOfEffectIgnitionSpec AreaOfEffectIgnitionSpecRef;
    ElectricalDamageSettings electricalDamageSettings;
    IceDamageSettings iceDamageSettings;

    // 0x7c8fc58
    ElementalDamageTable() = default;

    // TODO(0x7c8f9ec, Quantum.ElementalDamageTable.TryGetTable): bool TryGetTable(Frame frame,
    //   ElementalDamageTable table)
    // TODO(0x7c8fa78, Quantum.ElementalDamageTable.GetDamageMultiplier): FP
    //   GetDamageMultiplier(Frame frame, ElementalDamageTable table, DamageType damageType,
    //   ElementType elementType)
};

class EnergyCellManagerSpec : public AssetObject {
  public:
    std::int32_t LowCharge = 10;
    std::int32_t MediumCharge = 50;
    std::int32_t HighCharge = 100;

    // 0x7c904cc
    EnergyCellManagerSpec() = default;
};

class EntityDropperSpec : public AssetObject {
  public:
    DroppableType Type{};
    FPVector3 DropOffset;
    std::uint16_t OverallEntityDropCountPercent = 100;
    bool IfAffectedByGameModeMultiplier{};
    std::uint16_t DropProbabilityPercent = 100;
    bool TryDropOnGround{};
    FP FindGroundMaxDistance = FP::FromInt(10);
    FPVector3 OffsetFromGround = FPVector3(FP::FromRaw(0), FP::FromRaw(32768), FP::FromRaw(0));
    FP Lifetime{};
    bool PreventOwnerToCollect{};
    bool DropsOnDeath{};
    KillType KillTypes = static_cast<KillType>(-1);
    bool DropAllOnDeath{};
    std::uint16_t MinCounterOnDeath{};
    std::uint16_t DropCountOnDeath{};
    bool DropsOnHealthDecreased{};
    FP DropsEveryPercentOnHealthDecreased = FP::FromInt(10);
    std::uint16_t DropCountOnHealthDecreased{};

    // 0x7c905d8
    EntityDropperSpec() = default;

    // TODO(0x7c904e8, Quantum.EntityDropperSpec.GetDropCountOnDeath): ushort
    //   GetDropCountOnDeath(Frame frame, EntityRef entity)
    // 0x7c905cc - nothing by default.
    virtual void DroppedOnDeath(Frame* frame, EntityRef entity, FPVector3 position, std::uint16_t dropCount) const { (void)frame, (void)entity, (void)position, (void)dropCount; }
    // 0x7c905d0
    virtual std::uint16_t GetDropCountOnHealthDecreased() const { return DropCountOnHealthDecreased; }
};

class EntityGroupSpec : public AssetObject {
  public:
    std::int32_t GroupMaxMembers{};

    // 0x7c90674
    EntityGroupSpec() = default;
};

class FireReactorSpec : public AssetObject {
  public:
    FP DisabledDuration = FP::FromInt(20);
    FP EnabledDuration = FP::FromInt(10);

    // 0x7c9067c
    FireReactorSpec() = default;
};

class HealSourceSpec : public AssetObject {
  public:
    enum class HealingType : std::int32_t {
        VALUE = 0,
        MAX_HEALTH_PERCENTAGE = 1,
        MISSING_HEALTH_PERCENTAGE = 2,
    };

    HealSourceSpec::HealingType Type{};
    FP HealPerSecond{};
    FP PercentGainedPerSecond{};

    // 0x7c9210c
    HealSourceSpec() = default;
};

class LifetimeSpec : public AssetObject {
  public:
    bool StartLifetimeOnSpawn = true;
    bool UseOwnerWeaponLevelForLifetime{};
    FP LifetimeDuration = FP::FromInt(1);
    bool ReduceDurationIfHitDamageable{};
    FP ReducedLifetimeDuration = FP::FromRaw(32768); // 0.5
    std::vector<FP> LeveledLifetimeDurations = std::vector<FP>(10);
    bool SpawnEntityOnDeath{};
    std::vector<AssetRefEntityPrototype> EntitiesToSpawnOnDeath;
    FPVector3 SpawnOnDeathOffset;
    bool UseRaycastToSpawnOnDeath{};
    LayerMask UseRaycastToSpawnOnDeathLayerMask = LayerMask{-1};
    FP SpawnOnDeathRaycastDistance = FP::FromInt(5);
    bool SpawnOnDeathOrientOnNormal = true;

    // 0x7c92af8
    LifetimeSpec() = default;
};

class MapCustomData : public AssetObject {
  public:
    struct BakedAutoJumper {
        std::int32_t staticColliderIndex{};
        AutoJumperStartParams autoJumperStartParams;
    };

    struct BakedRailColliderData {
        std::int32_t staticColliderIndex{};
        FP distance{};
        std::int32_t nearbyRailColIndexLeft{};
        std::int32_t nearbyRailColIndexRight{};
    };

    struct BakedSplineSample {
        FPVector3 posWorld;
        FP distance{};
    };

    struct BakedSpline {
        std::vector<MapCustomData::BakedSplineSample> samples;
        FP length{};

        // TODO(0x7c92f48, Quantum.MapCustomData.BakedSpline.GetTangent): FPVector3 GetTangent(int
        //   i)
        // TODO(0x7c93104, Quantum.MapCustomData.BakedSpline.GetSampleAtDistance): void
        //   GetSampleAtDistance(FP d, FPVector3 pos, FPVector3 tangent)
    };

    struct BakedGrindRail {
        MapCustomData::BakedSpline spline;
        std::vector<MapCustomData::BakedRailColliderData> collidersData;
    };

    struct BakedTravolator {
        std::int32_t staticColliderIndex{};
        FPVector3 direction;
    };

    struct BlastBallData {
        FPVector3 ballPosition;
    };

    struct ClosingPointData {
        FPVector3 targetPosition;
        FP targetRadius{};
        FP startTime{};
        FP endTime{};
    };

    struct ClosingZoneData {
        FPVector3 startPosition;
        FP startRadius{};
        std::vector<MapCustomData::ClosingPointData> closingPoints;
    };

    struct ControlZoneData {
        FPVector3 position;
        FP radius{};
        std::int32_t id{};

        // TODO(0x7c92f08, Quantum.MapCustomData.ControlZoneData.GetIdAsString): string
        //   GetIdAsString()
    };

    struct SpawnPointData {
        FPVector3 position;
        FP rotY{};
        SpawnPointOptions options{};
        TeamFlag teamFlag{};
        std::uint16_t introTrackId{};
        FP availabilityDuration{};
    };

    struct StaticColliderGrindRailPair {
        std::int32_t staticColliderIndex{};
        std::int32_t grindRailIndex{};
    };

    struct StaticDroppableData {
        DroppableType type{};
        FPVector3 position;
    };

    struct TutorialData {
        FPVector3 tutoShootPosition;
        FPQuaternion tutoShootRotation;
        FPVector3 tutoMeleePosition;
        FPQuaternion tutoMeleeRotation;
        FPVector3 arenaPosition;
        FPQuaternion arenaRotation;
        FPVector3 ultimatePosition;
        FPQuaternion ultimateRotation;
    };

    std::vector<MapCustomData::SpawnPointData> SpawnPoints;
    MapCustomData::BlastBallData blastBallData;
    std::vector<MapCustomData::ControlZoneData> ControlZones;
    MapCustomData::TutorialData tutorialData;
    std::vector<MapCustomData::ClosingZoneData> ClosingZones;
    std::vector<FPVector3> GoldenBoltSpawnPoints;
    std::vector<MapCustomData::StaticDroppableData> StaticDroppables;
    std::vector<MapCustomData::BakedAutoJumper> staticJumpers;
    std::vector<MapCustomData::BakedTravolator> staticTravolator;
    std::vector<MapCustomData::BakedGrindRail> bakedGrindRails;
    std::vector<MapCustomData::StaticColliderGrindRailPair> staticColliderGrindRailPairs;
    std::unordered_map<std::int32_t, std::int32_t> _staticColliderToGrindRailIndex; // not stored; filled by Loaded

    // 0x7c92e2c
    MapCustomData() = default;

    // 0x7c92c34 - the grind rail the static collider belongs to.
    bool TryGetStaticColliderToGrindRailIndex(std::int32_t staticColliderIndex, std::int32_t& grindRailIndex) const;
    // 0x7c92c9c - rebuilds the static collider -> grind rail index table from staticColliderGrindRailPairs (a duplicate collider throws).
    void Loaded(IResourceManager* resourceManager, Allocator* allocator) override;
};

class MoveToSpec : public AssetObject {
  public:
    MoveToSpecMoveModes moveMode{};
    MoveOffsetOrientation offsetOrientation{};
    FPVector3 position;
    bool followEntity{};
    FPVector3 offset;
    FP delay{};
    FP duration{};
    EasingType easing{};

    // 0x7c937c0
    MoveToSpec() = default;
};

class ParabolaFollowerSpec : public AssetObject {
  public:
    FP Speed = FP::FromInt(1);
    FP MinDuration = FP::FromRaw(49152); // 0.75
    bool ConstantSpeed = true;
    FPAnimationCurve SpeedCurve;
    FP Height = FP::FromInt(5);
    bool CanGoBeyondEnd{};
    bool AutoRemoveOnReachedEnd{};
    bool AutoRemoveOnDeath = true;
    FP RemoveDelay{};
    bool ForceClampPosition{};
    bool UseMidPointCurve{};
    bool HomingEnabled{};
    FP HomingPower = FP::FromInt(10);
    bool UseTargetFinder = true;
    bool SnapOnNewTarget = true;
    FP TimeToFindNewTarget = FP::FromRaw(32768); // 0.5
    bool AirControlEnabled{};
    FP AirControlSpeed = FP::FromInt(10);
    bool OrientToEndPos{};
    bool OnlyOrientYAxis{};
    FPVector3 OrientationOffset;

    // 0x7c937e0
    ParabolaFollowerSpec() = default;
};

class PowerUpSpec : public AssetObject {
  public:
    bool HasDuration{};
    FP Duration{};

    // 0x7c93b5c
    PowerUpSpec() = default;

    // Abstract in the original.
    virtual PowerUpType GetPowerUpType() const = 0;
    // Abstract in the original.
    virtual void OnStarted(Frame* f, EntityRef userEntity) const {
        (void)f, (void)userEntity;
        NotReimplemented("Quantum.PowerUpSpec.OnStarted");
    }
    // Abstract in the original.
    virtual void OnFinished(Frame* f, EntityRef userEntity) const {
        (void)f, (void)userEntity;
        NotReimplemented("Quantum.PowerUpSpec.OnFinished");
    }
    // TODO(0x7c9388c, Quantum.PowerUpSpec.Start): void Start(Frame f, EntityRef userEntity); uses
    //   FrameEvents.PowerUp_OnStarted, AssetRefPowerUpSpec.op_Implicit, PowerUpSpec.Finish,
    //   PowerUpUser.get_ActivePowerUps
    // TODO(0x7c93b08, Quantum.PowerUpSpec.Finish): void Finish(Frame f, EntityRef userEntity, int
    //   stackIndex); uses FrameEvents.PowerUp_OnFinished
};

class PrioritisedTargetHandlerSpec : public AssetObject {
  public:
    FP detectionRadius_GroupTarget = FP::FromInt(25);
    FP detectionRadius_VisibleThreat = FP::FromInt(20);
    FP detectionRadius_AttackedByThreat = FP::FromInt(15);
    FP acquireIfAttackedRecentlySeconds = FP::FromInt(2);
    FP reacquireVisibleThreatAfterSeconds = FP::FromInt(2);
    FP forgetRecentThreatDuration = FP::FromInt(5);

    // 0x7c93b80
    PrioritisedTargetHandlerSpec() = default;
};

class ProjectileSpawnerSpec : public AssetObject {
  public:
    AssetRefEntityPrototype Projectile;
    AssetRefProjectileSpec ProjectileSpec;
    std::int32_t ProjectileCount = 3;
    FP Range = FP::FromInt(5);
    FP Height = FP::FromInt(10);
    FP Delay = FP::FromRaw(16384); // 0.25
    FP Interval = FP::FromRaw(6553); // 0.0999908

    // 0x7c94058
    ProjectileSpawnerSpec() = default;
};

class ProjectileSpec : public AssetObject {
  public:
    ProjectileType Type{};
    FP Speed = FP::FromInt(5);
    FP MaxLifeTime = FP::FromInt(5);
    bool OverrideMaxTravelDistance{};
    FP MaxTravelDistance = FP::FromInt(30);
    bool ReturnsToOwner{};
    ProjectileTargetedPart ForcedTargetedPart{};
    FP Gravity{};
    bool OverrideGravityAfterDistance{};
    FP OverrideGravityDistance{};
    FP OverrideGravityValue{};
    bool Elevates{};
    FP ElevationHeight = FP::FromInt(1);
    bool Accelerates{};
    ::Quantum::AccelerationType AccelerationType{};
    FP LinearAcceleration = FP::FromInt(10);
    FP CurveAccelerationMaxSpeed = FP::FromInt(50);
    FP CurveAccelerationDuration = FP::FromInt(2);
    FPAnimationCurve AccelerationCurve;
    FP LobOvershootMinHeight = FP::FromInt(-10);
    FP LobOvershootMaxRangePercent = FP::FromInt(200);
    std::vector<TrajectoryCurveData> LobCurves;
    FPAnimationCurve LobCurve;
    FP LobFlightTimeDurationSeconds = FP::FromInt(1);
    bool LobScaleFlightHeightWithTargetDistance = true;
    bool LobFlightInverseHeightCoef = true;
    FP LobAcquireTargetMinDistance = FP::FromInt(3);
    FP LobLoseTargetMaxDistance = FP::FromInt(4);
    bool SetAutoTargetFromAOE{};
    bool AimLowerOnEntityTarget{};
    FPAnimationCurve AnticipationForwardCurve;
    FPAnimationCurve HorizontalOffsetCurve;
    FPAnimationCurve VerticalOffsetCurve;
    LayerMask CollisionLayerMask = LayerMask{-1};
    ProjectileImpactType ImpactType = ProjectileImpactType::Die;
    bool SpawnEntityOnDeath{};
    AssetRefEntityPrototype AreaOfEffectAsset;
    bool AssignSpawnedEntityToOwnerTeam = true;
    bool AttachSpawnedEntityToImpactPoint{};
    FP SpawnNormalOffset{};
    bool OrientSpawnedEntityOnNormal{};
    bool TryFindBestOrientationNormal{};
    bool ResetSpawnedEntityUpVector{};
    bool Rebound{};
    FP GroundNormalAngleTolerance = FP::FromInt(25);
    FP LeadingFactor = FP::FromInt(1);
    bool IsHoming{};
    FP InitialHomingLerpFactor = FP::FromInt(10);
    FP ProgressiveHomingLerpIncreaseFactor = FP::FromInt(1);
    FP HomingMaxAngle = FP::FromInt(360);

    // 0x7c93f9c
    ProjectileSpec() = default;

    // 0x7c93ba8 - only linear projectiles accelerate.
    bool HasAcceleration() const;
    // TODO(0x7c93bc8, Quantum.ProjectileSpec.GetSurroundingTrajectories): void
    //   GetSurroundingTrajectories(FP angle, TrajectoryCurveData> lowerTrajectory,
    //   TrajectoryCurveData> higherTrajectory)
    // TODO(0x7c93d5c, Quantum.ProjectileSpec.GetLobTrajectoryData): void GetLobTrajectoryData(FP
    //   angle, TrajectoryCurveData> lowerTrajectory, TrajectoryCurveData> higherTrajectory, FP
    //   rangeRatio, FP height, FP trajectoryLerp); uses FPMath.InverseLerp, FPMath.Lerp,
    //   ProjectileSpec.GetSurroundingTrajectories
};

class PusherSpec : public AssetObject {
  public:
    PUSH_FILTERS PushFilters{};
    bool Is2D{};
    FP PushForce{};
    FP PushUpwardForce{};
    std::vector<FP> LeveledPushForce = std::vector<FP>(10);
    bool UseCurve{};
    FPAnimationCurve ForceCurve;
    bool IsInverse{};
    bool IsContinuous{};
    PushOnShieldBehavior OnShieldBehaviour{};
    bool BypassPushStop{};
    bool ForcePushStop{};
    FP ForcePushStopStrength = FP::FromInt(10);
    bool CanBump{};
    ::Quantum::BumpType BumpType{};
    FP BumpReductionPercentage = FP::FromRaw(32768); // 0.5
    FP FlatBumpStrength = FP::FromRaw(229376); // 3.5

    // 0x7c94438
    PusherSpec() = default;

    // 0x7c940c4 - copies the push settings into a push request.
    void SetupPushData(PushData* pushData) const;
    // TODO(0x7c94160, Quantum.PusherSpec.CalculateImpulse): FPVector3 CalculateImpulse(Frame f,
    //   FPVector3 forceDirection, FPVector3 hitPoint, EntityRef pusherEntity); uses FPMath.SqrtRaw,
    //   FPVector3.Normalize, FPVector3.get_Zero, FPAnimationCurve.Evaluate, Transform3D.get_Forward
};

class RewardSpec : public AssetObject {
  public:
    enum class AmountType : std::int32_t {
        Flat = 0,
        Percentage = 1,
    };

    DroppableType type = DroppableType::Bolt;
    RewardSpec::AmountType amountType{};
    std::uint16_t amount = 15;

    // 0x7c944b4
    RewardSpec() = default;
};

class SeekNewTargetPriorityList : public AssetObject {
  public:
    LayerMask DetectionLayers = LayerMask{1};
    std::vector<ENTITY_INFO_FILTER> PrioritisedEntityFilters;

    // 0x7c8e114
    SeekNewTargetPriorityList() = default;
};

class ShapeOverlapperSpec : public AssetObject {
  public:
    Shape3DConfig shape;
    LayerMask layerMask;
    bool bypassCaster{};
    bool bypassOwner{};
    bool useCastRate{};
    FP castRate{};
    bool NeedsLineOfSightCheck = true;
    QueryOptions LineOfSightQueryOptions = QueryOptions::HitAll;

    // 0x7c94510
    ShapeOverlapperSpec() = default;
};

class SpeederSpec : public AssetObject {
  public:
    FP Cooldown{};

    // 0x7c94578
    SpeederSpec() = default;
};

class StreamEffectSpawnerSpec : public AssetObject {
  public:
    AssetRefEntityPrototype StreamEffectAsset;

    // 0x7c9459c
    StreamEffectSpawnerSpec() = default;
};

class StreamEffectSpec : public AssetObject {
  public:
    FP StartRadius = FP::FromRaw(655); // 0.00999451
    FP EndRadius = FP::FromRaw(39321); // 0.599991
    FP TimeToMaxRadius = FP::FromRaw(32768); // 0.5

    // 0x7c94580
    StreamEffectSpec() = default;
};

class TargetFinderSpec : public AssetObject {
  public:
    ENTITY_INFO_FILTER TargetEntityFilter = ENTITY_INFO_FILTER::All;
    FP RangeNoWeapon{};
    bool RequireGrounded{};
    bool FindOnUpdate = true;
    bool FindOnWeaponUse{};
    bool NeedBeInFront = true;
    FP InFrontAngleLimitDegrees = FP::FromInt(104);
    bool BypassIfVeryClose = true;
    FP VeryCloseDistance = FP::FromInt(2);
    bool OverrideWeaponRange{};
    FP RangeOverride{};
    FP RangeExtensionPercent{};
    bool HasMinRange{};
    FP MinRange{};
    bool HasMaxHeightDiff{};
    FP MaxHeightDiff{};
    bool NeedLineOfSight = true;
    FPVector3 LineOfSightStartOffset;
    bool CheckLinearProjectileElevation{};
    std::int32_t LinearProjectileElevatedCurveRaycastCount = 2;
    bool UseCamSightCheck = true;
    FP MinCamSightAngleDegreesFar = FP::FromInt(9);
    FP MinCamSightAngleDegreesNear = FP::FromInt(17);
    FP MaxCamSightAngleDegreesFar = FP::FromInt(19);
    FP MaxCamSightAngleDegreesNear = FP::FromInt(35);
    FP CamAngleCloseDistance = FP::FromInt(4);
    FP CamDotValueForOnReticle = FP::FromString("0.999");
    FP ScoreOnReticle = FP::FromInt(200);
    FP ScoreDistance = FP::FromInt(25);
    FP ScoreInFront = FP::FromInt(50);
    FP ScorePreviousTarget = FP::FromInt(30);
    std::vector<FP> EntityTypeScores;
    bool SetAutoTarget = true;
    bool DisableOnPushed = true;

    // 0x7c945c0
    TargetFinderSpec() = default;
};

class TargetedEffectSpec : public AssetObject {
  public:
    bool InstantFirstTick{};
    FP TickRate = FP::FromInt(1);
    std::int32_t MaxTickCount = 1;

    // 0x7c946f0
    TargetedEffectSpec() = default;
};

class TrailSpawnerSpec : public AssetObject {
  public:
    AssetRefEntityPrototype TrailAsset;
    FP AoeTrailWidth = FP::FromInt(1);
    FP AoeTrailSegmentLength = FP::FromInt(1);

    // 0x7c94708
    TrailSpawnerSpec() = default;
};

class Action_AddModifierMoveSpeed : public CallbackAction {
  public:
    FP MoveSpeedModifier{};
    FP MoveSpeedDuration{};
    MOD_ORIGIN MoveSpeedModOrigin{};

    // 0x7cb12d4
    Action_AddModifierMoveSpeed() = default;

    // TODO(0x7cb120c, Quantum.Action_AddModifierMoveSpeed.Execute): void Execute(Frame frame,
    //   EntityRef entity); uses KccExtension.AddModifier_MoveSpeed
    // TODO(0x7cb12c8, Quantum.Action_AddModifierMoveSpeed.Execute): void Execute(Frame frame,
    //   EntityRef entity, EntityRef targetEntity)
};

class Action_KillSelf : public CallbackAction {
  public:

    // 0x7cb1300
    Action_KillSelf() = default;

    // TODO(0x7cb12dc, Quantum.Action_KillSelf.Execute): void Execute(Frame frame, EntityRef
    //   entity); uses KillableLib.Kill
    // TODO(0x7cb12f4, Quantum.Action_KillSelf.Execute): void Execute(Frame frame, EntityRef entity,
    //   EntityRef targetEntity)
};

class Action_MoveTo_StartMoving : public CallbackAction {
  public:

    // 0x7cb1468
    Action_MoveTo_StartMoving() = default;

    // TODO(0x7cb1308, Quantum.Action_MoveTo_StartMoving.Execute): void Execute(Frame frame,
    //   EntityRef entity); uses MoveTo.StartMoving
    // TODO(0x7cb13b4, Quantum.Action_MoveTo_StartMoving.Execute): void Execute(Frame frame,
    //   EntityRef entity, EntityRef targetEntity); uses MoveTo.StartMoving
};

class Action_SpawnEntities : public CallbackAction {
  public:
    std::vector<AssetRefEntityPrototype> EntitiesToSpawn;
    FPVector3 Offset;
    bool UseRaycast{};
    LayerMask RaycastLayerMask = LayerMask{-1};
    FP RaycastDistance = FP::FromInt(5);
    bool OrientOnNormal = true;

    // 0x7cb1998
    Action_SpawnEntities() = default;

    // TODO(0x7cb1470, Quantum.Action_SpawnEntities.Execute): void Execute(Frame frame, EntityRef
    //   entity); uses FrameSignals.Lifetime_OnSpawnEntityOnDeath, FPMath.SqrtRaw,
    //   FPVector3.get_Down, AssetGuid.op_Equality, AssetGuid.op_Implicit, EntityRef.get_None,
    //   FPMathUtility.OrientOnNormal, Frame.GetPersistentHitCollection...
    // TODO(0x7cb198c, Quantum.Action_SpawnEntities.Execute): void Execute(Frame frame, EntityRef
    //   entity, EntityRef targetEntity)
};

class BoltDropperSpec : public EntityDropperSpec {
  public:

    // 0x7bb6f40
    BoltDropperSpec() = default;
};

class EnergyCellDropperSpec : public EntityDropperSpec {
  public:

    // 0x7bbd614
    EnergyCellDropperSpec() = default;
};

class EnergyCellPowerUpSpec : public PowerUpSpec {
  public:
    std::int32_t ChargeValue{};

    // 0x7bc798c
    EnergyCellPowerUpSpec() = default;

    // 0x7bc7968
    PowerUpType GetPowerUpType() const override { return PowerUpType::EnergyCell; }
    // TODO(0x7bc7970, Quantum.EnergyCellPowerUpSpec.OnStarted): void OnStarted(Frame f, EntityRef
    //   userEntity); uses EnergyCellManager.TryGiveEnergyCell
    // 0x7bc7988 - nothing.
    void OnFinished(Frame* f, EntityRef userEntity) const override { (void)f, (void)userEntity; }
};

class ExtraDamagePowerUpSpec : public PowerUpSpec {
  public:
    FP DamageBoostPercent{};

    // 0x7bc79a4
    ExtraDamagePowerUpSpec() = default;

    // 0x7bc7994
    PowerUpType GetPowerUpType() const override { return PowerUpType::ExtraDamage; }
    // 0x7bc799c - nothing (the extra damage is applied where damage is computed).
    void OnStarted(Frame* f, EntityRef userEntity) const override { (void)f, (void)userEntity; }
    // 0x7bc79a0 - nothing.
    void OnFinished(Frame* f, EntityRef userEntity) const override { (void)f, (void)userEntity; }
};

class OverhealthPowerUpSpec : public PowerUpSpec {
  public:
    enum class HealthBoostAmountMode : std::int32_t {
        FIXED = 0,
        USE_AVERAGE_HUMAN_HEROES_LEVEL = 1,
        USE_HERO_LEVEL = 2,
        PERCENTAGE = 3,
    };

    FP DecreaseDuration{};
    FP DecreaseTickRate{};
    OverhealthPowerUpSpec::HealthBoostAmountMode Mode{};
    FP FixedHealthBoost{};
    FP MinHealthBoost{};
    FP MaxHealthBoost{};
    std::vector<FP> HealthBoostPerLevel = std::vector<FP>(10);
    FP Percentage{};

    // 0x7bc7d68
    OverhealthPowerUpSpec() = default;

    // 0x7bc79ac
    PowerUpType GetPowerUpType() const override { return PowerUpType::Overhealth; }
    // TODO(0x7bc79b4, Quantum.OverhealthPowerUpSpec.OnStarted): void OnStarted(Frame f, EntityRef
    //   userEntity); uses Health.AddOverhealth, OverhealthPowerUpSpec.GetHealthBoost
    // TODO(0x7bc7cac, Quantum.OverhealthPowerUpSpec.OnFinished): void OnFinished(Frame f, EntityRef
    //   userEntity); uses Health.DecreaseOverhealth, OverhealthPowerUpSpec.GetHealthBoost
    // TODO(0x7bc7a90, Quantum.OverhealthPowerUpSpec.GetHealthBoost): FP GetHealthBoost(Frame f,
    //   EntityRef userEntity); uses FPMath.Lerp, BalancingLib.TryGetHumanHeroesLevelRatio
};

class PowerUpDropperSpec : public EntityDropperSpec {
  public:

    // 0x7bc7f58
    PowerUpDropperSpec() = default;
};

class SpeedBoostPowerUpSpec : public PowerUpSpec {
  public:
    FP SpeedModifier{};

    // 0x7bc7f50
    SpeedBoostPowerUpSpec() = default;

    // 0x7bc7dcc
    PowerUpType GetPowerUpType() const override { return PowerUpType::SpeedBoost; }
    // TODO(0x7bc7dd4, Quantum.SpeedBoostPowerUpSpec.OnStarted): void OnStarted(Frame f, EntityRef
    //   userEntity); uses KccExtension.AddModifier_MoveSpeed
    // TODO(0x7bc7ea4, Quantum.SpeedBoostPowerUpSpec.OnFinished): void OnFinished(Frame f, EntityRef
    //   userEntity); uses KccExtension.ClearModifiersOfType
};

} // namespace Quantum
