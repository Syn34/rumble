// asset_value_types.h - value types stored in the gameplay asset classes, and forward declarations of those classes (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/asset_refs.h"
#include "quantum_code/enums.h"
#include "quantum_code/components.h"
#include "quantum_core/asset_object.h"
#include "quantum_core/fp_animation_curve.h"
#include "quantum_core/layer_mask.h"
#include "quantum_core/shape3d_config.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/map.h"
#include "quantum_core/query_options.h"
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Quantum {

// The asset classes (defined in the other headers of this folder).
class AIAction;
class AIBlackboard;
class AIBlackboardInitializer;
class AIBotSDKConfig;
class AIConfig;
class AIFunction;
class AIFunctionAND;
class AIFunctionAssetRefList;
class AIFunctionBoolList;
class AIFunctionByteList;
class AIFunctionEntityRefList;
class AIFunctionFPList;
class AIFunctionFPVector2List;
class AIFunctionFPVector3List;
class AIFunctionIntegerList;
class AIFunctionNOT;
class AIFunctionOR;
class AIMoveForwardBehaviourSpec;
class AINavigationSpec;
class AISpawnerConfig;
class Action_AddModifierMoveSpeed;
class Action_KillSelf;
class Action_MoveTo_StartMoving;
class Action_SpawnEntities;
class ActorComponentSpecBase;
class ActorIcedSpec;
class ActorManagerConfig;
class ActorPhysicsSpec;
class ActorSheepedSpec;
class ActorStatusModifierSpec;
class ActorStatusSpec;
class ActorStunnedSpec;
class AddDamageIncreaseOnTargetedEffectSpec;
class AddEntitiesToSpawnOnProjectileDeathSpec;
class AddForceOnUltimateSpec;
class AddTargetToBlacklistAction;
class AimAtPassingTargetEvaluator;
class AirSaveEvaluator;
class AmmoMultiplierEnhancementSpec;
class AmmunitionDamageSpec;
class AmoeboidLauncherEnhancementPuddleSpec;
class AmoeboidLevelUpActorSpec;
class AoeTrailSpec;
class AreaOfEffectIgnitionSpec;
class AreaOfEffectLevelUpSpec;
class AreaOfEffectSpec;
class ArenaCannonConfig;
class ArithmeticFPFunction;
class AttackControlZoneEvaluator;
class AttackPlayerEvaluator;
class AutoJumperStaticTriggerConfig;
class BTBlackboardCompare;
class BTComposite;
class BTCooldown;
class BTDecorator;
class BTForceResult;
class BTLeaf;
class BTLoop;
class BTNode;
class BTRoot;
class BTSelector;
class BTSelectorRandom;
class BTSequence;
class BTService;
class BigBoomMineSpec;
class BlastBallGameModeSpec;
class BlastBallSpec;
class BlasterMoveSpeedEnhancementSpec;
class BlasterSpecialShotEnhancementSpec;
class BoltDropperSpec;
class BoltGathererActorSpec;
class BoltKingGameModeSpec;
class BoltPowerActorSpec;
class BombardierDroneSpec;
class BombardierWeaponAutoTargetSpec;
class BumperBarConfig;
class BumperConfig;
class BuzzbladesRicochetSpec;
class CallbackAction;
class ChargeAttackGetMaxDistanceFunction;
class ChargeAttackInProgressFunction;
class ChargeAttackSpec;
class ChargeAttackStartAction;
class ChargeWrenchAction;
class ChargeWrenchEvaluator;
class CheckBlackboardInt;
class ChoosePatrolDestinationAction;
class ColliderResizeSpec;
class CollisionForceSpec;
class CompoundEnhancementSpec;
class CompoundHFSMSpec;
class CompoundHeroImprovementSpec;
class Consideration;
class CooldownMultiplierEnhancementSpec;
class CryosliderSpec;
class CryosliderWeaponAutoTargetSpec;
class DamageChargeableSpec;
class DamageIncreasedOnTargetsEnhancementSpec;
class DamageMultiplierHeroImprovementSpec;
class DamageMultiplierSpec;
class DamageSpec;
class DamageToImpulseConverterSpec;
class DamageableSpec;
class DashActorConfig;
class DashBlastBallEvaluator;
class DashCooldownReductionHeroImprovementSpec;
class DashRoboBall;
class DeathAnimationSpec;
class DebugAction;
class DebugLeaf;
class DebugService;
class DefaultAIFunctionAssetRef;
class DefaultAIFunctionBool;
class DefaultAIFunctionByte;
class DefaultAIFunctionEntityRef;
class DefaultAIFunctionFP;
class DefaultAIFunctionFPVector2;
class DefaultAIFunctionFPVector3;
class DefaultAIFunctionInt;
class DefaultAIFunctionString;
class DefendControlZoneEvaluator;
class DestroyAction;
class DestroyElementConfig;
class DestroyMobGroupGameplayElement;
class DispatchMobSlamAttacksEventAction;
class DispatchMobsEventAction;
class DispatchShooterEventAction;
class DispatchSpinnerEventAction;
class DistanceFromPositionDecision;
class DrillDashGadgetSpec;
class DroppableCollectorSpec;
class DroppableManagerSpec;
class ElementalBuildUpMultiplierHeroImprovementSpec;
class ElementalDamageTable;
class EliminationGameModeSpec;
class EnergyCellDropperSpec;
class EnergyCellManagerSpec;
class EnergyCellPowerUpSpec;
class EnforcerFirstShotBonusEnhancementSpec;
class EnhancementSpec;
class EntityDropperSpec;
class EntityExistsFunction;
class EntityGroupSpec;
class EntityIsGroundedFunction;
class EntityRefsAreEqualFunction;
class EscapeShrinkingZoneEvaluator;
class ExtraDamagePowerUpSpec;
class FindTargetAction;
class FireReactorSpec;
class FirerateMultiplierEnhancementSpec;
class FrameTimeHasElapsedFunction;
class FreeForAllGameModeSpec;
class GOAPAction;
class GOAPBackValidation;
class GOAPDefaultAction;
class GOAPDefaultGoal;
class GOAPGoal;
class GOAPHeuristic;
class GOAPRoot;
class GOAPSingleBackValidation;
class GadgetSpec;
class GadgetWeaponSpec;
class GameIsStarted;
class GameManagerConfig;
class GameModeHandlerConfig;
class GameModeHandlerLevelConfig;
class GameModeSpec;
class GameModesTable;
class GameModifiersSpec;
class GatlingEnhancementResetAmmoOnGadgetUseSpec;
class GatlingEnhancementSpec;
class GatlingWeaponSpec;
class GetAttackInfoActiveFunction;
class GetAttackInfoTargetFunction;
class GetAttackInfoWasRunningFunction;
class GetEntityPositionFunction;
class GetFrameNumberFunction;
class GetGroupEntityFunction;
class GetGroupTargetFunction;
class GetHighestPriorityTargetEntityFunction;
class GetHighestPriorityTargetPositionFunction;
class GetRandomFPFunction;
class GetRecentPriorityTargetPositionFunction;
class GetTargetFinderRangeNoWeaponFunction;
class GetVec2Element;
class GloveOfDoomWeaponSpec;
class GoTo;
class GoToControlZone;
class GoToPlayerDebugPos;
class GoldenBoltConfig;
class GrenadeLauncherAftershockEnhancementSpec;
class GrenadeLauncherStunGrenadeEnhancementSpec;
class GrinderActorConfig;
class GroundDashAction;
class GroundDashActorConfig;
class GroupTeamEvaluator;
class HFSMAndDecision;
class HFSMDecision;
class HFSMLogicalDecision;
class HFSMNotDecision;
class HFSMOrDecision;
class HFSMRoot;
class HFSMState;
class HFSMTransitionSet;
class HasGroupTargetFunction;
class HasLineOfSightToTargetDecision;
class HasMoveBehaviourBouncedDecision;
class HasNavLineOfSightToTargetFunction;
class HasReachedTime;
class HasTarget;
class HasValidRecentPriorityTargetFunction;
class HeadHunterWallHackEnhancementSpec;
class HealSourceSpec;
class HealthMultiplierEnhancementSpec;
class HealthSpec;
class HelipackActorSpec;
class HeroEnhancementSpec;
class HeroSpec;
class HoloshieldGloveHealZoneEnhancementSpec;
class HoloshieldGloveWeaponSpec;
class HoverBootsEnhancementCollisionSpec;
class HoverBootsWeaponSpec;
class IdleAction;
class IgnoreEntitiesMultiplierEnhancementSpec;
class ImprovementSpec;
class ImpulseAction;
class ImpulseTowardTargetAction;
class InShootingZoneMovementEvaluator;
class IncreaseBlackboardInt;
class InputCheatFollowPlayer;
class IsDeadDecision;
class IsGroundedDecision;
class IsTargetValidDecision;
class IsTrue;
class ItemSpawnerConfig;
class JumpAction;
class JumpAttackAction;
class JumpAttackHeroImprovementSpec;
class JumpAttackSpec;
class JumpMultiplierHeroImprovementSpec;
class KccExtensionConfig;
class KccIsMovingFunction;
class KccModifierSpec;
class LedgeGrabbingSpec;
class LevelButtonConfig;
class LevelButtonGameplayElementConfig;
class LevelUpActorSpec;
class LifetimeSpec;
class MapCustomData;
class MeleeAttackIsActiveFunction;
class MeleeAttackSpec;
class MeleeJumpAttackSpec;
class MinMaxFPFunction;
class MineLauncherEnhancementSpec;
class MineLauncherProjectileOverrideEnhancementSpec;
class MobGroupConfig;
class MobInfoSpec;
class MoveAwayFromTeammatesEvaluator;
class MoveToClosestEnemyEvaluator;
class MoveToSpec;
class MoveTowardTargetAction;
class MoveTowardsBallEvaluator;
class MovingWallConfig;
class MovingWallGameplayElementConfig;
class NavigateToTargetAction;
class NegatronWeaponSpec;
class OnKillCallbacksHeroImprovementSpec;
class OverhealthPowerUpSpec;
class ParabolaFollowerSpec;
class PassingEvaluator;
class PathfindStatusEqualsFunction;
class PlaceholderHeroImprovementSpec;
class PowerUpDropperSpec;
class PowerUpSpec;
class PreciseHitReceiverSpec;
class PrepareShootingAction;
class PrioritisedTargetHandlerSpec;
class PriorityTargetTypeCompareFunction;
class ProjectileSpawnerSpec;
class ProjectileSpec;
class PushableSpec;
class PusherSpec;
class RailStaticTriggerConfig;
class RaiseAgentOfDoomViewEvent;
class ReachEnemyGoalEvaluator;
class ReachShootingZoneEvaluator;
class ReachStrikeZoneEvaluator;
class ReachedPointDecision;
class RecoverAmmoOnDashEnhancementSpec;
class ReplaceHealthSpecHeroImprovementSpec;
class ReplaceMultiShotConfigsEnhancementSpec;
class ResetMaxSpeedModifier;
class ResetTargetAction;
class ResetTimerAction;
class ResponseCurve;
class RewardReceiverSpec;
class RewardSpec;
class RollSpec;
class RynoWeaponSpec;
class SeekCrateEvaluator;
class SeekDroppableEvaluator;
class SeekGoldenBoltEvaluator;
class SeekNewTarget;
class SeekNewTargetPriorityList;
class SetAttackInfoActiveAction;
class SetBlackboardBool;
class SetBlackboardEntityRef;
class SetBlackboardFP;
class SetBlackboardFPVector3;
class SetBlackboardInt;
class SetCompoundHFSMSchedulePeriodAction;
class SetCurrentMaxSpeedModifier;
class SetGravityModifierAction;
class SetGroupTargetAction;
class SetOwnerInBlackboardAction;
class SetPositionInBlackboardAction;
class SetShootPrecision;
class SetSpawnerPositionInBlackboardAction;
class SetSpawnerWanderRadiusInBlackboardAction;
class SetupAoEAction;
class ShapeOverlapperSpec;
class ShieldedActorSpec;
class ShootBallAction;
class ShootBallEvaluator;
class ShootEvaluator;
class ShootTarget;
class ShotgunBerserkerEnhancementSpec;
class ShotgunEnhancementSpec;
class ShrinkingZoneSpec;
class SimulationGadgetTable;
class SimulationHeroesTable;
class SimulationMeleeWeaponsTable;
class SimulationUltimateTable;
class SimulationWeaponTable;
class SmokeScreenSpec;
class SpeedBoostPowerUpSpec;
class SpeederSpec;
class SquadSiegeConfig;
class SquadSiegeGameModeSpec;
class SquadSiegeLevelConfig;
class StartMeleeAttackAction;
class StartShootingAction;
class StaticColliderConfig;
class StaticTriggerConfig;
class StopNavigationAction;
class StopPathFinding;
class StopShootingAction;
class StopToShootEvaluator;
class StreamEffectSpawnerSpec;
class StreamEffectSpec;
class SweepAttackAction;
class SweepAttackSpec;
class SweepAttackStateEqualsFunction;
class SwitchWeapon;
class SwitchWeaponEvaluator;
class SwitchWeaponUltimate;
class SwitchWeaponUltimateEvaluator;
class TakeDefensivePositionEvaluator;
class TargetFinderSpec;
class TargetedEffectSpec;
class TeamTotalRumbleGameModeSpec;
class TeslaClawMoreBeamEnhancementSpec;
class TeslaClawWeaponSpec;
class TimerDecision;
class ToggleAoEAction;
class ToggleMovementBehaviourAction;
class ToggleMovementPackSlicingAction;
class ToggleTargetFinderAction;
class TrailSpawnerSpec;
class TrainingGameModeSpec;
class TravolatorStaticTriggerConfig;
class TrueDecision;
class TurnTowardTargetAction;
class TutorialGameModeSpec;
class UTRoot;
class UltimateAbilitySpec;
class UltimateAbilityUserSpec;
class UltimateEvolutionSpec;
class UltimateSlamSpec;
class UltimateTankSpec;
class UltimateWeaponSpec;
class UpdateStrikeZonePositionAction;
class UseGadget;
class UseGadgetEvaluator;
class UseWrench;
class UseWrenchEvaluator;
class VoidRepulserBulldozerEnhancementSpec;
class VoidRepulserSpeedBoostEnhancementSpec;
class VoidRepulserVoidWaveEnhancementSpec;
class VoidRepulserWeaponSpec;
class WaitLeaf;
class WallRunningSpec;
class Wander;
class WanderEvaluator;
class WarmongerMiniRocketsEnhancementSpec;
class WaveManagerSpec;
class WaveReferences;
class WeaponBehaviour_ChargingSpec;
class WeaponChargingStatsModifierEnhancementSpec;
class WeaponEnhancementSpec;
class WeaponLoadoutSpec;
class WeaponProjectileSwitcherEnhancementSpec;
class WeaponShowcaseGameModeSpec;
class WeaponSpec;
class WeaponSpecDefault;
class WeaponStatsModifierEnhancementSpec;
class WeaponStatsModifierHeroImprovementSpec;
class WeaponUserSpec;
class WithinRangeDecision;
class WrenchGroundSlamEnhancementSpec;
class WrenchThrowEnhancementSpec;
class WrenchWeaponSpec;
class ZoneControlGameModeSpec;

// Stands in for a method that isn't reimplemented yet where code must compile: an
// overridable base method whose overrides exist already. Throws std::logic_error.
[[noreturn]] void NotReimplemented(const char* method);

// Quantum.AIFunction<T> and Quantum.EnumEqualsFunction<T> (bot SDK generics; defined in
// bot_sdk_assets.h). The C++ name of AIFunction<T> carries a T suffix because a class and a
// class template can't share a name.
template <class T> class AIFunctionT;
template <class T> class EnumEqualsFunction;

// Quantum.AIParam<T> (bot SDK): a value read by an AI node - its own DefaultValue, an entry
// of the agent's AIConfig, a blackboard entry (both named by Key) or the result of an
// AIFunction, as chosen by Source. The original's generic members (Resolve, ResolveConfig,
// the constructor) are shared generic code without addresses of their own; the typed
// subclasses below carry the per-type parts.
template <class T> struct AIParam {
    AIParamSource Source = AIParamSource::Value;
    std::string Key;
    T DefaultValue{};
};

// Quantum.EnumAIParam<T>: an AIParam over an enum, with its function reference.
template <class T> struct EnumAIParam : AIParam<T> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<T>* _cachedFunction = nullptr; // not stored
};

struct AIBlackboardValueKey {
    std::string Key;
};

struct AIBlackboardEntry {
    AIBlackboardValueType Type{};
    AIBlackboardValueKey Key;
};

struct ActorPhysicsSurfaceSettings {
    KccSurfaceType surfaceType{};
    AssetRefActorPhysicsSpec physicsActorSpec;

    // 0x7cb92bc
    ActorPhysicsSurfaceSettings() = default;
};

struct ActorStatusTypeSettings {
    ActorStatusType type = ActorStatusType::Normal;
    std::vector<ActorStatusType> forbiddenNextTypes;
    std::vector<AssetRefActorComponentSpecBase> componentSpecs;
    FP prePhysicsPushModifier = FP::FromInt(1);
    std::vector<ActorPhysicsSurfaceSettings> physicsSurfaceSettings;

    // 0x7cb92c4
    ActorStatusTypeSettings() = default;
};

// Original: BATTLE_RESULT (GameClasses.dll, global namespace).
enum class BATTLE_RESULT : std::int32_t {
    UNKNOWN = 0,
    WIN = 1,
    LOSE = 2,
    DRAW = 3,
};

struct BTDataIndex {
    static constexpr std::int32_t SIZE = 4;
    static constexpr std::int32_t ALIGNMENT = 4;

    std::int32_t Index{};

    // TODO(0x7b7e16c, Quantum.BTDataIndex.GetHashCode): int GetHashCode()
};

struct DamageModifier {
    AssetRefDamageSpec DamageSpecRef;
    FP DamageModifierAmount{};

    // 0x7bbb5ac
    DamageModifier() = default;
};

struct ElementMultiplier {
    ElementType elementType{};
    FP multiplier = FP::FromInt(1);

    // 0x7bbc908
    ElementMultiplier() = default;
};

struct DamageTypeToElementMapping {
    DamageType damageType{};
    std::vector<ElementMultiplier> elementMultipliers;

    // 0x7bbc918
    DamageTypeToElementMapping() = default;
};

struct DifficultyIntervalInt {
    std::int32_t atMinDiff{};
    std::int32_t atMaxDiff = 100;

    // 0x7cbdb38
    DifficultyIntervalInt() = default;

    // 0x7cbdb0c - the value at a difficulty in [0, 1], interpolated and rounded up.
    std::int32_t Get(FP difficulty_0_1) const;
};

struct ElectricalDamageSettings {
    FP totalBuildUp = FP::FromInt(100);
    FP buildUpResetAfter = FP::FromInt(2);
    FP moveSpeedModifierWhenHit = FP::FromRaw(-16384); // -0.25
    FP moveSpeedModifierWhenHitDuration = FP::FromRaw(32768); // 0.5
    FP stunDurationSeconds = FP::FromInt(1);

    // 0x7bbc9a0
    ElectricalDamageSettings() = default;
};

struct EntityMultiplierPair {
    ::Quantum::EntityInfoType EntityInfoType{};
    FP Multiplier = FP::FromInt(1);

    // 0x7bdd104
    EntityMultiplierPair() = default;
};

// Original: FTUE_STEP_ID (GameClasses.dll, global namespace).
enum class FTUE_STEP_ID : std::int32_t {
    NONE = 0,
    GAMEPLAY_TUTORIAL = 1,
    META1 = 2,
    META2 = 3,
    META3 = 4,
    META4 = 5,
    META5 = 6,
    META6 = 7,
    META7 = 8,
    COMPLETE = 9,
};

struct GadgetSpecModifier {
    enum class OperationType : std::int32_t {
        Absolute = 0,
        Add = 1,
        Multiply = 2,
    };

    enum class Property : std::int32_t {
        None = 0,
        Cooldown = 1,
        Power = 2,
    };

    GadgetSpecModifier::Property prop{};
    GadgetSpecModifier::OperationType operation{};
    std::vector<FP> values = std::vector<FP>(10);

    // 0x7c00fd4
    GadgetSpecModifier() = default;
};

struct GameModesTableEntry {
    static constexpr std::int32_t SIZE = 24;
    static constexpr std::int32_t ALIGNMENT = 8;

    GAME_MODE id{};
    AssetRefEntityPrototype prototype;
    AssetRefGameModeSpec spec;
    GAME_MODE_STATUS status{};

    // TODO(0x7b980a4, Quantum.GameModesTableEntry.GetHashCode): int GetHashCode()
};

struct HFSMSetupData {
    AssetRefHFSMRoot hfsmRoot;
    AssetRefAIConfig aiConfig;
    AssetRefAIBlackboardInitializer blackboardInitializer;

    // 0x7cc9eb4
    HFSMSetupData() = default;
};

struct HFSMTransition {
    std::string Id;
    std::int32_t EventKey{};
    AssetRefHFSMDecision DecisionLink;
    AssetRefHFSMState StateLink;
    AssetRefHFSMTransitionSet TransitionSetLink;
    HFSMDecision* Decision = nullptr; // not stored
    HFSMState* State = nullptr; // not stored
    HFSMTransitionSet* TransitionSet = nullptr; // not stored

    // 0x7b84030
    HFSMTransition() = default;

    // 0x7b838a4 - resolves the decision, target state and transition set.
    void Setup(IResourceManager* resourceManager);
};

struct HeightRangePercentPair {
    FP height{};
    FP rangePercent{};
};

struct IceDamageSettings {
    FP totalBuildUp = FP::FromInt(100);
    FP buildUpResetAfter = FP::FromInt(2);
    FP moveSpeedModifierWhenHit = FP::FromRaw(-16384); // -0.25
    FP moveSpeedModifierWhenHitDuration = FP::FromRaw(32768); // 0.5
    FP icedDurationSeconds = FP::FromInt(5);

    // 0x7bbc9c4
    IceDamageSettings() = default;
};

struct MeleeComboAttackData {
    AssetRefEntityPrototype AmmunitionAsset;
    FP MaxTargetDistForAutoImpulse = FP::FromInt(5);
    FP ImpulseDelay{};
    FP ImpulseStrength = FP::FromInt(40);
    FP ImpulseDuration = FP::FromRaw(3275); // 0.0499725
    FP ForceInputDuration = FP::FromRaw(13107); // 0.199997
    FP TimeToRemainAlignedWithDirection = FP::FromRaw(32768); // 0.5
    FP MaxSpeedModifier = FP::FromRaw(-32768); // -0.5
    FP SpeedModifierDuration = FP::FromRaw(32768); // 0.5
    FP CooldownDuration = FP::FromRaw(32768); // 0.5
    FP NextHitTimeframe = FP::FromInt(1);
    FP ImpulseDirectionSnapToTargetStrength = FP::FromInt(2);

    // 0x7c00e5c
    MeleeComboAttackData() = default;
};

struct PveScalingLevelEntry {
    FP PveScaling{};
    std::uint16_t Level{};
};

struct MobLevelMappingData {
    ::Quantum::MobType MobType{};
    std::vector<PveScalingLevelEntry> PveScalingLevelTable;
};

struct MovingWallOffset {
    FPVector3 Offset;
    bool LockInPlaceOnArrival{};
    FP StopDuration = FP::FromInt(1);

    // 0x7bd8180
    MovingWallOffset() = default;
};

struct RandomWeaponsModifierSpec {
    FP randomTimeInterval = FP::FromInt(20);

    // 0x7bc6e9c
    RandomWeaponsModifierSpec() = default;
};

struct ResponseCurvePack {
    AssetRefAIFunction ResponseCurveRef;
    ::Quantum::ResponseCurve* ResponseCurve = nullptr; // not stored
};

struct TrajectoryCurveData {
    FP Angle{};
    FPAnimationCurve Curve;
    FP Height{};
    FP RangePercent{};
    std::vector<HeightRangePercentPair> HeightRangePercentPairs;

    // TODO(0x7c0599c, Quantum.TrajectoryCurveData.GetCurveEndDiffAndDiffRate): void
    //   GetCurveEndDiffAndDiffRate(FP step, FP endDiff, FP diffRate)
    // TODO(0x7c05a20, Quantum.TrajectoryCurveData.TryGetRangePercentForHeight): bool
    //   TryGetRangePercentForHeight(FP height, FP targetRangePercent, FP rangePercent)
};

struct UltimateSpecModifier {
    enum class OperationType : std::int32_t {
        Absolute = 0,
        Add = 1,
        Multiply = 2,
    };

    enum class Property : std::int32_t {
        None = 0,
        FullCharge = 1,
        Power = 2,
    };

    UltimateSpecModifier::Property prop{};
    UltimateSpecModifier::OperationType operation{};
    std::vector<FP> values = std::vector<FP>(10);

    // 0x7c0c8cc
    UltimateSpecModifier() = default;
};

struct WaveSpawnRequest {
    std::string MobID;
    FP SpawnTime{};
    std::uint16_t Amount{};
    std::uint16_t SpawnerIndex{};
    std::string DropperID;
    std::uint16_t DropperAmount{};
};

struct WaveData {
    FP WaveMaxDuration{};
    std::vector<WaveSpawnRequest> SpawnRequestArray;
};

struct WeaponLoadoutData {
    static constexpr std::int32_t SIZE = 32;
    static constexpr std::int32_t ALIGNMENT = 4;

    WeaponLoadoutItemData first;
    WeaponLoadoutItemData gadget;
    WeaponLoadoutItemData melee;
    WeaponLoadoutItemData ultimate;

    // TODO(0x7b9d680, Quantum.WeaponLoadoutData.GetHashCode): int GetHashCode()
};

struct WeaponSpecModifier {
    enum class OperationType : std::int32_t {
        Absolute = 0,
        Add = 1,
        Multiply = 2,
    };

    enum class Property : std::int32_t {
        None = 0,
        Range = 1,
        FireRate = 2,
        MoveSpeedModFactorOnUse = 3,
        MultiShotConfig = 4,
        ReloadTime = 5,
        CooldownTime = 6,
        AmmoPerBurst = 7,
        MaxAmmo = 8,
        Power = 9,
    };

    WeaponSpecModifier::Property prop{};
    WeaponSpecModifier::OperationType operation{};
    std::vector<FP> values = std::vector<FP>(10);

    // 0x7c11854
    WeaponSpecModifier() = default;
};

struct WeaponStatModifier {
    WeaponSpecModifier::Property Property{};
    WeaponSpecModifier::OperationType OperationType{};
    FP Value{};
};

struct WeaponVisualParams {
    GENERATED_WEAPON_ID weaponID{};
    FPVector3 weaponSpawnPosition;
    FPVector3 weaponPosition;
    FPVector3 weaponEulerAngles;
    FPVector3 weaponShootEulerAngles;
    FP triggerPressedDuration{};
    FPVector3 weaponLastRotationPosition;
    FPVector3 weaponLastRotationEulerAngles;
};

struct AIParamAssetRef : AIParam<AssetRef> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<AssetRef>* _cachedFunction = nullptr; // not stored

    // 0x7b80040
    AIParamAssetRef() = default;

    // TODO(0x7b7feb8, Quantum.AIParamAssetRef.GetBlackboardValue): AssetRef
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7ff54, Quantum.AIParamAssetRef.GetConfigValue): AssetRef GetConfigValue(KeyValuePair
    //   configPair)
    // TODO(0x7b7ff68, Quantum.AIParamAssetRef.GetFunctionValue): AssetRef
    //   GetFunctionValue(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
};

struct AIParamBool : AIParam<bool> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<bool>* _cachedFunction = nullptr; // not stored

    // 0x7b7f4c8
    AIParamBool() = default;

    // 0x7b7f464 - a parameter that uses `value` (Source stays Value).
    AIParamBool(bool value) { DefaultValue = value; }
    // TODO(0x7b7f510, Quantum.AIParamBool.GetBlackboardValue): bool
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7f5b0, Quantum.AIParamBool.GetConfigValue): bool GetConfigValue(KeyValuePair
    //   configPair)
    // TODO(0x7b7f5c4, Quantum.AIParamBool.GetFunctionValue): bool GetFunctionValue(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

struct AIParamEntityRef : AIParam<EntityRef> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<EntityRef>* _cachedFunction = nullptr; // not stored

    // 0x7b7fe70
    AIParamEntityRef() = default;

    // TODO(0x7b7fce8, Quantum.AIParamEntityRef.GetBlackboardValue): EntityRef
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7fd84, Quantum.AIParamEntityRef.GetConfigValue): EntityRef
    //   GetConfigValue(KeyValuePair configPair)
    // TODO(0x7b7fd98, Quantum.AIParamEntityRef.GetFunctionValue): EntityRef
    //   GetFunctionValue(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
};

struct AIParamFP : AIParam<FP> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<FP>* _cachedFunction = nullptr; // not stored

    // 0x7b7f6fc
    AIParamFP() = default;

    // 0x7b7f69c - a parameter that uses `value` (Source stays Value).
    AIParamFP(FP value) { DefaultValue = value; }
    // TODO(0x7b7f744, Quantum.AIParamFP.GetBlackboardValue): FP GetBlackboardValue(BlackboardValue
    //   value)
    // TODO(0x7b7f7e0, Quantum.AIParamFP.GetConfigValue): FP GetConfigValue(KeyValuePair configPair)
    // TODO(0x7b7f7f4, Quantum.AIParamFP.GetFunctionValue): FP GetFunctionValue(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

struct AIParamFPVector2 : AIParam<FPVector2> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<FPVector2>* _cachedFunction = nullptr; // not stored

    // 0x7b7fa58
    AIParamFPVector2() = default;

    // TODO(0x7b7f8cc, Quantum.AIParamFPVector2.GetBlackboardValue): FPVector2
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7f96c, Quantum.AIParamFPVector2.GetConfigValue): FPVector2
    //   GetConfigValue(KeyValuePair configPair)
    // TODO(0x7b7f980, Quantum.AIParamFPVector2.GetFunctionValue): FPVector2
    //   GetFunctionValue(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
};

struct AIParamFPVector3 : AIParam<FPVector3> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<FPVector3>* _cachedFunction = nullptr; // not stored

    // 0x7b7fca0
    AIParamFPVector3() = default;

    // TODO(0x7b7faa0, Quantum.AIParamFPVector3.GetBlackboardValue): FPVector3
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7fb4c, Quantum.AIParamFPVector3.GetConfigValue): FPVector3
    //   GetConfigValue(KeyValuePair configPair)
    // TODO(0x7b7fb6c, Quantum.AIParamFPVector3.GetFunctionValue): FPVector3
    //   GetFunctionValue(FrameThreadSafe frame, EntityRef entity, AIContext aiContext)
};

struct AIParamInt : AIParam<std::int32_t> {
    AssetRefAIFunction FunctionRef;
    AIFunctionT<std::int32_t>* _cachedFunction = nullptr; // not stored

    // 0x7b7f294
    AIParamInt() = default;

    // 0x7b7f234 - a parameter that uses `value` (Source stays Value).
    AIParamInt(std::int32_t value) { DefaultValue = value; }
    // TODO(0x7b7f2dc, Quantum.AIParamInt.GetBlackboardValue): int
    //   GetBlackboardValue(BlackboardValue value)
    // TODO(0x7b7f378, Quantum.AIParamInt.GetConfigValue): int GetConfigValue(KeyValuePair
    //   configPair)
    // TODO(0x7b7f38c, Quantum.AIParamInt.GetFunctionValue): int GetFunctionValue(FrameThreadSafe
    //   frame, EntityRef entity, AIContext aiContext)
};

struct AIParamGotoDestinationType : EnumAIParam<GOTO_DESTINATION> {

    // 0x7cbe880
    AIParamGotoDestinationType() = default;
};

struct AIParamMobSlamAttackEventType : EnumAIParam<MobSlamAttackState> {

    // 0x7cd2f94
    AIParamMobSlamAttackEventType() = default;
};

struct AIParamMobsEventType : EnumAIParam<MobsEventType> {

    // 0x7cd2eb8
    AIParamMobsEventType() = default;
};

struct AIParamPrioTargetType : EnumAIParam<PrioTargetType> {

    // 0x7cd44a4
    AIParamPrioTargetType() = default;
};

struct NavigateToTargetAction_NavType : EnumAIParam<AI_NAVIGATION_TYPE> {

    // 0x7cbb8b0
    NavigateToTargetAction_NavType() = default;
};

} // namespace Quantum
