// actor_specs.h - actor assets: heroes, health, movement extensions, actor components (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/gameplay_specs.h"

namespace Quantum {

class AIMoveForwardBehaviourSpec : public AssetObject {
  public:
    bool BounceOnNavMesh = true;
    bool ConstrainToSpawner = true;
    bool DieOnWallHit{};
    bool BounceOnDynamicObjects = true;

    // 0x7c8dbb4
    AIMoveForwardBehaviourSpec() = default;
};

class AINavigationSpec : public AssetObject {
  public:
    std::vector<NAV_REGION> DisabledRegions;
    bool IsObstacle{};
    FP ObstacleRadius = FP::FromRaw(32768); // 0.5
    LayerMask ObstacleLayer = LayerMask{32768};
    bool UseAvoidance{};

    // 0x7c8dbcc
    AINavigationSpec() = default;
};

class ActorComponentSpecBase : public AssetObject {
  public:

    // 0x7c93884
    ActorComponentSpecBase() = default;

    // Abstract in the original.
    virtual void Shutdown(Frame* f, EntityRef e) const {
        (void)f, (void)e;
        NotReimplemented("Quantum.ActorComponentSpecBase.Shutdown");
    }
};

class ActorManagerConfig : public AssetObject {
  public:
    FP PendingRespawnDuration = FP::FromInt(1);
    bool BotsUseSpecificPendingRespawnDuration{};
    FP BotsPendingRespawnDuration = FP::FromInt(1);
    FP RespawnDuration = FP::FromInt(3);
    FP RespawnPointCalculationDelay = FP::FromInt(2);
    FP ShrinkingZoneRespawnMargin = FP::FromInt(10);
    FP ShrinkingZoneRespawnSafeDuration = FP::FromInt(5);
    FP RespawnPointCooldownDuration = FP::FromInt(5);
    FP RespawnInnerRadius = FP::FromInt(10);
    FP RespawnOuterRadius = FP::FromInt(25);

    // 0x7c8dcc8
    ActorManagerConfig() = default;

    // 0x7c8dca0 - pending duration (the bots' own one if they use it) plus the respawn itself.
    FP GetTotalRespawnDuration(bool isBot) const;
};

class ActorStatusModifierSpec : public AssetObject {
  public:
    ActorStatusType wantedType = ActorStatusType::Normal;
    FP durationSeconds = FP::FromInt(1);
    bool bypassShield{};

    // 0x7c8dd88
    ActorStatusModifierSpec() = default;
};

class ActorStatusSpec : public AssetObject {
  public:
    std::vector<ActorStatusTypeSettings> typeSettings;

    // 0x7c8dd00
    ActorStatusSpec() = default;
};

class BoltGathererActorSpec : public AssetObject {
  public:
    FP PreventBoltCollectionDefaultDuration = FP::FromInt(2);
    std::uint16_t InitialBoltCount = 10;
    std::uint16_t MaxBoltsCollectable{};

    // 0x7c8dfb4
    BoltGathererActorSpec() = default;
};

class BoltPowerActorSpec : public AssetObject {
  public:
    std::int32_t MaxBolts = 500;
    std::uint16_t MaxLevel = 5;
    FPAnimationCurve DamageMultiplier;
    FPAnimationCurve MaxHealthMultiplier;
    FPAnimationCurve MaxShieldMultiplier;
    FPAnimationCurve UltimateTimeMultiplier;
    FPAnimationCurve UltimatePerPushReceivedMultiplier;

    // 0x7c8dfcc
    BoltPowerActorSpec() = default;
};

class ChargeAttackSpec : public AssetObject {
  public:
    FP anticipationDelay = FP::FromInt(1);
    FP cooldownDelay = FP::FromInt(1);
    FP maxTravelDistance = FP::FromInt(10);
    FP maxSpeedModifier = FP::FromRaw(32768); // 0.5
    FP aoeRange = FP::FromRaw(98304); // 1.5

    // 0x7c8e214
    ChargeAttackSpec() = default;

    // 0x7c8e204
    FP MaxTravelDistSqr() const { return maxTravelDistance * maxTravelDistance; }
};

class ColliderResizeSpec : public AssetObject {
  public:
    FPVector3 TargetExtents;
    FPVector3 TargetCentroid;
    FP Duration{};

    // 0x7c8e238
    ColliderResizeSpec() = default;
};

class DamageableSpec : public AssetObject {
  public:
    DAMAGE_FILTER DamageFilter = DAMAGE_FILTER::All;
    ::Quantum::ElementType ElementType{};
    bool InstantKillByBuffedDamageTypes{};
    FP InitialInvulnerabilityDuration{};
    FP SpawnInvulnerabilityDuration{};
    FP RespawnInvulnerabilityDuration{};
    FP ShieldBreakInvulnerabilityDuration{};
    FP LastHitSaveDuration = FP::FromInt(10);

    // 0x7c8f9cc
    DamageableSpec() = default;
};

class DeathAnimationSpec : public AssetObject {
  public:
    AssetRefParabolaFollowerSpec ParabolaFollowerAsset;
    FP MaxDistance = FP::FromInt(3);
    FP MaxHeight = FP::FromInt(1);
    FP CastGroundOffset = FP::FromRaw(98304); // 1.5
    FP ShapeCastRadius = FP::FromInt(1);
    ::Quantum::LayerMask LayerMask = ::Quantum::LayerMask{1};
    ::Quantum::QueryOptions QueryOptions = ::Quantum::QueryOptions::HitAll;
    FP RayCastDistance = FP::FromInt(100);
    FP DefaultEndPointDownOffset = FP::FromInt(5);
    FP EndPointGroundOffset = FP::FromRaw(6553); // 0.0999908
    FP EndPointWallOffset = FP::FromRaw(32768); // 0.5
    FP KillSourceOffset = FP::FromRaw(16384); // 0.25

    // 0x7c92a1c
    DeathAnimationSpec() = default;
};

class DroppableCollectorSpec : public AssetObject {
  public:
    struct MaxCollectPerSecond {
        DroppableType droppableType{};
        std::uint16_t amount{};
    };

    FP RangeMultiplier = FP::FromInt(1);
    std::vector<DroppableCollectorSpec::MaxCollectPerSecond> MaxDroppablesCollectedPerSecond;

    // 0x7c904bc
    DroppableCollectorSpec() = default;

    // TODO(0x7c90444, Quantum.DroppableCollectorSpec.HasMaxCollectPerSecond): bool
    //   HasMaxCollectPerSecond(DroppableType droppableType, ushort maxAmount)
};

class HealthSpec : public AssetObject {
  public:
    HealthSpecMode Mode{};
    std::vector<FP> LeveledMaxHP = std::vector<FP>(10);
    FP MinHP = FP::FromInt(100);
    FP MaxHP = FP::FromInt(100);
    FP BaseHP = FP::FromInt(100);
    FP FixedHP = FP::FromInt(100);
    std::vector<FP> AmountPerSelfLevel;
    FP SpawnImmunityDuration{};
    bool KillWhenNoMoreHP{};
    bool HasDamageThreshold{};
    FP DamageThreshold{};
    bool HasPushThreshold{};
    FP PushPercentageThreshold = FP::FromInt(40);
    bool CanRegenerate{};
    FP TimeWithoutDamageBeforeStartRegen = FP::FromInt(3);
    FP TimeToRegenerate = FP::FromInt(3);
    bool OnlyRegenerateIfShieldIsUp = true;
    bool TakeDamageOutsideShrinkingZone{};
    FP PercentDamageOutsideShrinkingZone = FP::FromInt(1);
    FP DamageCooldownOutsideShrinkingZone = FP::FromInt(1);
    FP HealthRegenPerPickup = FP::FromInt(50);

    // 0x7c92114
    HealthSpec() = default;
};

class HeroSpec : public AssetObject {
  public:
    std::vector<FP> LeveledPower = std::vector<FP>(10);
    std::vector<FP> LeveledHP = std::vector<FP>(10);
    bool HasShield = true;
    std::vector<FP> LeveledShield = std::vector<FP>(10);
    AssetRefHealthSpec HealthSpec;
    AssetRefCharacterController3DConfig CharacterController3DConfig;
    GENERATED_WEAPON_ID DefaultWeapon{};
    Shape3DConfig ColliderShape;
    std::vector<AssetRefImprovementSpec> HeroImprovementSpecs;
    std::vector<AssetRef> TalentsInUnlockOrder;
    std::uint8_t AvailableInBucketIndex{};

    // 0x7c925cc
    HeroSpec() = default;

    // TODO(0x7c921bc, Quantum.HeroSpec.GetLevelAtPower): int GetLevelAtPower(Frame frame,
    //   GENERATED_CHARACTER_ID heroID, FP power)
    // TODO(0x7c92254, Quantum.HeroSpec.InitializeHeroImprovementsFromTalentLevel): void
    //   InitializeHeroImprovementsFromTalentLevel(Frame frame, EntityRef heroEntity, int
    //   heroTalentLevel)
    // TODO(0x7c9240c, Quantum.HeroSpec.InitializeHeroImprovementsFromTalentLevel): void
    //   InitializeHeroImprovementsFromTalentLevel(Frame frame, EntityRef heroEntity, EntityRef
    //   weaponEntity, int heroTalentLevel)
};

class JumpAttackSpec : public AssetObject {
  public:
    FP AttackDurationSeconds = FP::FromInt(2);
    FPAnimationCurve HorizontalCurve;
    FPAnimationCurve HeightCurve;
    FP IdealAttackRange = FP::FromInt(10);
    FP IdealAttackHeight = FP::FromInt(3);
    FP MaxAttackHeight = FP::FromInt(10);
    FP FinalDistanceToTarget{};
    AssetRefEntityPrototype EntityToSpawn;
    FP SlamTriggerRatio = FP::FromRaw(49152); // 0.75

    // 0x7c928a4
    JumpAttackSpec() = default;
};

class KccExtensionConfig : public AssetObject {
  public:
    bool CanMoveFromInputs = true;
    FP Impulse_Braking{};
    FP Impulse_Acceleration{};
    FP Airborne_Braking{};
    FP Airborne_Acceleration{};
    FP OrientationSharpness{};
    FP GoThroughWallCheckThreshold = FP::FromRaw(32768); // 0.5
    LayerMask WallCheckLayerMask = LayerMask{1088};
    bool limitVelocityBasedOnInput{};
    FP limitVelocityMinValue{};
    FP maxFallSpeed = FP::FromInt(100);
    bool CanJump = true;
    FP JumpRequiredGroundedTime = FP::FromRaw(16384); // 0.25
    FP CoyoteTime = FP::FromRaw(6553); // 0.0999908
    FP DoubleJumpImpulse{};
    bool CanBounce{};
    FP BounceRestitution = FP::FromInt(1);
    FP BounceMinSpeed = FP::FromInt(1);
    bool AutoJumpEnabled{};
    bool CanAutoJumpWhileAirborne{};
    FP AutoJumpVelocityFactor{};
    FP AutoJumpHeightCheckStart{};
    FP AutoJumpHeightCheckDistance{};
    FP AutoJumpMaxAngle{};
    FP AutoJumpDownTime{};
    FP AutoJumpDownTimeAfterManualJump{};
    bool AutoJumpOnBouncingBalloons = true;
    FP AutoJumpOnBouncingBalloonsDistance = FP::FromInt(4);
    FP AutoJumpOnBouncingBalloonsHeight = FP::FromInt(3);
    std::int32_t AutoJumpOnBouncingBalloonsFrameSlicing = 6;
    bool AutoJumpOnWallRun = true;
    FP AutoJumpOnWallRunDistance = FP::FromInt(3);
    FP AutoJumpOnWallRunHeight = FP::FromRaw(98304); // 1.5
    FP AutoJumpOnWallRunFacingThreshold = FP::FromRaw(32768); // 0.5
    std::int32_t AutoJumpOnWallRunFrameSlicing = 7;
    FP PushReductionSpeed{};
    FP PushReboundFactor = FP::FromRaw(32768); // 0.5
    FP PushKillThreshold = FP::FromRaw(655); // 0.00999451
    FP PushUpwardLerp = FP::FromRaw(6553); // 0.0999908
    FP PushUpwardAngle_MaxLerp = FP::FromInt(100);
    FP PushUpwardAngle_MinLerp = FP::FromInt(100);
    FP ReboundAfterPushMaxDelay = FP::FromInt(1);
    FP ReboundPushThreshold = FP::FromInt(20);
    std::vector<EntityMultiplierPair> WalkingPushMultiplierByEntity;
    FP pushGraceDuration{};
    FP pushGraceBreakForceFactorThreshold{};
    FP pushStopDuration{};
    bool LedgeSlowEnable{};
    FP LedgeSlowVelocityFactor{};
    FP LedgeSlowHeightThreshold{};
    FP LedgeSlowMaxSpeedReductionAmount{};
    FP LedgeSlowMaxSpeedReductionLimit{};
    FP LedgeSlowMaxAngle{};
    bool IsSlowedDownOnHit{};
    FP HitReceivedSlowDownMinDamage{};
    FP HitReceivedSlowDownMaxDamage{};
    FP HitReceivedSlowDownMinSlowdownStrength{};
    FP HitReceivedSlowDownMaxSlowdownStrength{};
    FP HitReceivedSlowDownMinDuration{};
    FP HitReceivedSlowDownMaxDuration{};
    FP HitReceivedSlowDownFalloffDuration{};

    // 0x7c92950
    KccExtensionConfig() = default;

    // TODO(0x7c928f0, Quantum.KccExtensionConfig.TryGetWalkingPushMultiplier): bool
    //   TryGetWalkingPushMultiplier(EntityInfoType type, FP multiplier)
};

class KccModifierSpec : public AssetObject {
  public:
    enum class Type : std::int32_t {
        Normal = 0,
        Surface = 1,
    };

    KccModifierSpec::Type ModifierType{};
    MOD_TYPE ModType{};
    FP Value = FP::FromInt(1);
    FP Duration = FP::FromInt(1);
    bool OverwriteExisting{};
    KccSurfaceType SurfaceType{};

    // 0x7c92a08
    KccModifierSpec() = default;
};

class LedgeGrabbingSpec : public AssetObject {
  public:
    std::vector<ActorStatusType> AuthorizedActorStatusTypes;
    FP ForwardDistance = FP::FromInt(2);
    FP DownwardDistance = FP::FromInt(2);
    FP HeightOffset = FP::FromRaw(98304); // 1.5
    ::Quantum::LayerMask LayerMask;
    FP ForwardMaxAngle = FP::FromInt(45);
    FP LedgeMinHeight = FP::FromInt(10);
    std::int32_t SchedulePeriod = 5;
    FP VoidMinHeight = FP::FromInt(2);
    FP LedgeMinThickness = FP::FromRaw(6553); // 0.0999908
    FP LedgeHitNormalMaxAngle = FP::FromInt(30);
    FP VoidTestOffsetY = FP::FromRaw(32768); // 0.5
    FPAnimationCurve snapPositionCurveXZ;
    FPAnimationCurve snapPositionCurveY;
    FPAnimationCurve snapRotationCurve;
    FP snapDuration = FP::FromInt(2);
    FP snapPosForwardOffset = FP::FromRaw(32768); // 0.5
    FP snapPosHeightOffset = FP::FromInt(2);
    FP cameraDistanceOffset = FP::FromInt(10);
    FP cameraVerticalAngle = FP::FromInt(15);
    bool debug{};

    // 0x7c92a7c
    LedgeGrabbingSpec() = default;
};

class LevelUpActorSpec : public AssetObject {
  public:

    // 0x7c92af0
    LevelUpActorSpec() = default;
};

class MeleeAttackSpec : public AssetObject {
  public:
    FP attackDuration = FP::FromInt(1);
    AssetRefEntityPrototype areaOfEffect;
    FPVector3 aoeSpawnOffset;

    // 0x7c933dc
    MeleeAttackSpec() = default;
};

class MeleeJumpAttackSpec : public AssetObject {
  public:
    AssetRefEntityPrototype GroundSlamAoEPrototypeAsset;
    FP CooldownDuration = FP::FromInt(2);
    FP InputHoldMinDuration = FP::FromRaw(13107); // 0.199997
    FP DownImpulseStrengh = FP::FromInt(400);
    FP MaxFallSpeedModifier = FP::FromInt(400);
    FP UpImpulseStrength = FP::FromInt(100);
    FP UpImpulseDuration = FP::FromInt(1);
    FP MoveSpeedModifier{};
    FP MoveSpeedModifierFallOf{};
    FP OnFinishMoveSpeedModifier{};
    FP OnFinishMoveSpeedModifierDuration = FP::FromInt(1);
    FP OnFinishMoveSpeedModifierFallOf = FP::FromInt(1);
    FP SwitchingBackToPreviousWeapon = FP::FromInt(1);
    FP RaycastLength = FP::FromInt(2);
    FP BallonBounceMultiplier = FP::FromRaw(32768); // 0.5

    // 0x7c9344c
    MeleeJumpAttackSpec() = default;
};

class PreciseHitReceiverSpec : public AssetObject {
  public:
    FP CriticalHitDistanceToTarget = FP::FromInt(1);
    FP HeadshotDistanceToTarget = FP::FromRaw(32768); // 0.5
    FP HeadshotBonusMultiplier = FP::FromInt(1);

    // 0x7c93b64
    PreciseHitReceiverSpec() = default;
};

class PushableSpec : public AssetObject {
  public:
    PUSH_FILTERS PushFilters{};
    FP PrePhysicsPushMultiplier = FP::FromInt(1);
    bool UsePushStop{};
    FP PushStopDurationSeconds = FP::FromInt(1);
    bool DisplayPushDamage = true;
    bool LockJumpOnPush{};
    FP LockJumpThreshold = FP::FromInt(5);
    FP LockJumpDuration = FP::FromInt(1);
    bool CustomPushOnShieldBreak{};
    FP PushOnShieldBreakForce = FP::FromInt(10);
    FP PushOnShieldBreakUpwardForce = FP::FromInt(10);
    bool PushOnDeath{};
    FP PushOnDeathForce = FP::FromInt(10);
    FP PushOnDeathUpwardForce{};
    bool PushStopOnDeath{};
    FP PushStopOnDeathDurationSeconds = FP::FromInt(1);

    // 0x7c94080
    PushableSpec() = default;
};

class RewardReceiverSpec : public AssetObject {
  public:
    FP rewardDelay = FP::FromRaw(98304); // 1.5
    std::uint16_t rivalMultiplier = 2;

    // 0x7c944cc
    RewardReceiverSpec() = default;
};

class RollSpec : public AssetObject {
  public:
    bool clampVerticalVelocity = true;
    FP verticalMaxVelocity = FP::FromRaw(13107); // 0.199997
    FP horizontalFixVelocity = FP::FromInt(18);
    FP angularFixVelocity = FP::FromInt(3);
    std::uint32_t maxBounce = 15u;
    bool disableBounceOnPlayers{};
    bool killOnPlayerCollision{};
    bool killOnCombatParticipantCollision{};
    bool alsoAdjustForwardOnBounce{};
    LayerMask depenetrationOverlapLayerMask;

    // 0x7c944e4
    RollSpec() = default;
};

class ShieldedActorSpec : public AssetObject {
  public:
    ShieldSpecMode Mode{};
    FP shieldPointsMin = FP::FromInt(60);
    FP shieldPointsMax = FP::FromInt(180);
    FP moveSpeedModifierWhenHit = FP::FromRaw(-16384); // -0.25
    FP moveSpeedModifierTickRate = FP::FromRaw(16384); // 0.25
    FP moveSpeedModifierWhenHitDuration = FP::FromRaw(32768); // 0.5
    FP moveSpeedModifierWhenBreak = FP::FromRaw(-32768); // -0.5
    FP moveSpeedModifierWhenBreakDuration = FP::FromRaw(32768); // 0.5
    FP timeWithoutDamageBeforeStartRegen = FP::FromInt(4);
    FP timeToRegenerate = FP::FromInt(3);

    // 0x7c94528
    ShieldedActorSpec() = default;
};

class SweepAttackSpec : public AssetObject {
  public:
    FP PrepareDurationSeconds = FP::FromInt(1);
    FP SweepDurationSeconds = FP::FromInt(1);
    FP SweepAngleDegrees = FP::FromInt(90);
    FP FinishedDurationSeconds = FP::FromInt(1);

    // 0x7c945a4
    SweepAttackSpec() = default;
};

class WallRunningSpec : public AssetObject {
  public:
    FP Acceleration{};
    FP Deceleration{};
    FP Speed{};
    FP PerpendicularThreshold{};
    FP JumpSpeed{};
    FP ExitImpulse{};
    FP HeightCorrectionCentroidOffset = FP::FromInt(-2);
    FP HeightCorrectionInterpolationSpeed{};
    FP SnapOffset = FP::FromRaw(32768); // 0.5

    // 0x7c94cb0
    WallRunningSpec() = default;
};

class ActorIcedSpec : public ActorComponentSpecBase {
  public:

    // 0x7cb9c08
    ActorIcedSpec() = default;

    // TODO(0x7cb9b24, Quantum.ActorIcedSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb9b7c, Quantum.ActorIcedSpec.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class ActorPhysicsSpec : public ActorComponentSpecBase {
  public:
    bool useHeroSpecCharacterController3DConfig{};
    AssetRefCharacterController3DConfig kccConfig;
    AssetRefKccExtensionConfig kccExtensionConfig;

    // 0x7cb8b78
    ActorPhysicsSpec() = default;

    // TODO(0x7cb88a8, Quantum.ActorPhysicsSpec.Init): void Init(Frame f, EntityRef e)
    // 0x7cb8b74 - nothing to undo.
    void Shutdown(Frame* f, EntityRef e) const override { (void)f, (void)e; }
};

class ActorSheepedSpec : public ActorComponentSpecBase {
  public:

    // 0x7cb9d84
    ActorSheepedSpec() = default;

    // TODO(0x7cb9ca0, Quantum.ActorSheepedSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb9cf8, Quantum.ActorSheepedSpec.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class ActorStunnedSpec : public ActorComponentSpecBase {
  public:

    // 0x7cb9f00
    ActorStunnedSpec() = default;

    // TODO(0x7cb9e1c, Quantum.ActorStunnedSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb9e74, Quantum.ActorStunnedSpec.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class AmoeboidLevelUpActorSpec : public LevelUpActorSpec {
  public:
    AssetRefEntityPrototype NextLevelGroupPrototype;

    // 0x7bef338
    AmoeboidLevelUpActorSpec() = default;

    // TODO(0x7bef160, Quantum.AmoeboidLevelUpActorSpec.LevelUp): void LevelUp(Frame f, EntityRef
    //   entity, LevelUpActor* levelUpActor)
};

class DashActorConfig : public ActorComponentSpecBase {
  public:
    FP validInputThreshold = FP::FromRaw(13107); // 0.199997
    FP frontThreshold = FP::FromRaw(9175); // 0.139999
    FP force = FP::FromInt(18);
    FP duration = FP::FromRaw(22937); // 0.349991
    FP inputFalloff = FP::FromRaw(6553); // 0.0999908
    FP cooldown = FP::FromRaw(163840); // 2.5
    FP initialYVelocity = FP::FromRaw(32768); // 0.5
    FP gravityModifier = FP::FromInt(-5);
    FP pushForceReductionFactor = FP::FromRaw(49152); // 0.75
    FP moveSpeedModifier = FP::FromRaw(-32768); // -0.5
    FP moveSpeedModifierDuration = FP::FromInt(1);
    FP moveSpeedModifierFalloffDuration = FP::FromRaw(6553); // 0.0999908
    bool stunHitPlayers{};
    FP stunDuration = FP::FromInt(2);
    AssetRefDamageSpec damageSpec;
    AssetRefPusherSpec pusherSpec;

    // 0x7cb40a8
    DashActorConfig() = default;

    // TODO(0x7cb3f58, Quantum.DashActorConfig.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb4000, Quantum.DashActorConfig.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class GrinderActorConfig : public ActorComponentSpecBase {
  public:
    FP grindSpeedMin = FP::FromRaw(98304); // 1.5
    FP grindSpeedMax = FP::FromInt(7);
    FP accelerationTime = FP::FromRaw(98304); // 1.5
    FP accelerationDecaySpeed = FP::FromInt(1);
    FP exitGrindImpulse = FP::FromInt(5);
    FP timeToSnap = FP::FromRaw(32768); // 0.5
    FP snapHeightFactor = FP::FromRaw(98304); // 1.5
    FP xInputThreshold = FP::FromRaw(16384); // 0.25
    FP perpendicularThreshold = FP::FromRaw(9828); // 0.149963
    FP switchCooldown = FP::FromInt(1);
    std::string SFX_Land = "Ratchet_LandRail";
    FPAnimationCurve snapHeightCurve;

    // 0x7cb53f0
    GrinderActorConfig() = default;

    // TODO(0x7cb5208, Quantum.GrinderActorConfig.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb52ac, Quantum.GrinderActorConfig.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class GroundDashActorConfig : public ActorComponentSpecBase {
  public:
    FP gravityModValue = FP::FromInt(30);
    FP maxFallSpeedModValue = FP::FromInt(3);
    FP startingDuration = FP::FromRaw(32768); // 0.5
    MOD_ORIGIN modifiersOrigin = MOD_ORIGIN::SELF_WEAPON;
    AssetRefEntityPrototype aoe;

    // 0x7cb55c4
    GroundDashActorConfig() = default;

    // TODO(0x7cb547c, Quantum.GroundDashActorConfig.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb5520, Quantum.GroundDashActorConfig.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class HelipackActorSpec : public ActorComponentSpecBase {
  public:
    FP minWorldHeight = FP::FromInt(-3);
    FP minGroundDistanceToStart = FP::FromInt(5);
    FP delayToActivate = FP::FromRaw(32768); // 0.5
    FP gravityModifier = FP::FromRaw(-32768); // -0.5
    FP maxFallSpeedModifier = FP::FromRaw(-32768); // -0.5
    FP accelerationModifier = FP::FromRaw(6553); // 0.0999908
    FP maxSpeedModifier = FP::FromRaw(-6553); // -0.0999908
    FP maxDuration = FP::FromInt(3);
    FPVector3 impulseOnActivation = FPVector3(FP::FromInt(1), FP::FromInt(3), FP::FromInt(1));
    FP modifierTransitionDuration = FP::FromRaw(32768); // 0.5

    // 0x7cb632c
    HelipackActorSpec() = default;

    // TODO(0x7cb618c, Quantum.HelipackActorSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7cb6230, Quantum.HelipackActorSpec.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class UltimateAbilityUserSpec : public ActorComponentSpecBase {
  public:

    // 0x7c0dfec
    UltimateAbilityUserSpec() = default;

    // TODO(0x7c0dea4, Quantum.UltimateAbilityUserSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7c0df48, Quantum.UltimateAbilityUserSpec.Shutdown): void Shutdown(Frame f, EntityRef
    //   e)
};

class WeaponLoadoutSpec : public ActorComponentSpecBase {
  public:
    WeaponLoadoutData fallbackLoadout;
    FP delayToSwitch = FP::FromRaw(16384); // 0.25
    FP delayOnEquipped = FP::FromRaw(6553); // 0.0999908

    // 0x7c020cc
    WeaponLoadoutSpec() = default;

    // TODO(0x7c01f84, Quantum.WeaponLoadoutSpec.Init): void Init(Frame f, EntityRef e)
    // TODO(0x7c02028, Quantum.WeaponLoadoutSpec.Shutdown): void Shutdown(Frame f, EntityRef e)
};

class WeaponUserSpec : public ActorComponentSpecBase {
  public:

    // 0x7c0e1d8
    WeaponUserSpec() = default;

    // TODO(0x7c0dff4, Quantum.WeaponUserSpec.Init): void Init(Frame frame, EntityRef entity)
    // TODO(0x7c0e0b4, Quantum.WeaponUserSpec.Shutdown): void Shutdown(Frame frame, EntityRef
    //   entity)
};

} // namespace Quantum
