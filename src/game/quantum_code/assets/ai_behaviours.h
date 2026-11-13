// ai_behaviours.h - the game's AI actions, decisions and functions for the bot SDK graphs (quantum.code.dll).
//
// Asset classes are immutable simulation data authored in the editor and loaded from the
// game's content files (see src/game/content/quantum_code_factories.cpp). Fields are listed
// in the original declaration order; defaults are those of the original constructors.
// Fields marked "not stored" are runtime state, not part of the asset data; Loaded()
// resolves the stored references into the ones marked "filled by Loaded".
#pragma once

#include "quantum_code/assets/bot_sdk_assets.h"

namespace Quantum {

class AddTargetToBlacklistAction : public AIAction {
  public:
    AIParamEntityRef TargetEntity;

    // 0x7cba1ac
    AddTargetToBlacklistAction() = default;

    // TODO(0x7cb9f98, Quantum.AddTargetToBlacklistAction.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class ChargeAttackStartAction : public AIAction {
  public:

    // 0x7cc78f8
    ChargeAttackStartAction() = default;

    // TODO(0x7cc7850, Quantum.ChargeAttackStartAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class ChargeWrenchAction : public AIAction {
  public:
    FP minChargeTime = FP::FromInt(1);
    FP maxChargeTime = FP::FromInt(2);

    // 0x7ccf0d4
    ChargeWrenchAction() = default;

    // TODO(0x7ccf028, Quantum.ChargeWrenchAction.Update): void Update(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class ChoosePatrolDestinationAction : public AIAction {
  public:
    AIParamEntityRef PatrolCenter;
    AIParamFP PatrolRadius;
    AIBlackboardValueKey PatrolDestinationKey;
    AIBlackboardValueKey PatrolDestinationFoundKey;

    // 0x7cba830
    ChoosePatrolDestinationAction() = default;

    // TODO(0x7cba3f8, Quantum.ChoosePatrolDestinationAction.Update): void Update(Frame f, EntityRef
    //   entity, AIContext ctx)
};

class DashRoboBall : public AIAction {
  public:

    // 0x7ccf5fc
    DashRoboBall() = default;

    // TODO(0x7ccf5d4, Quantum.DashRoboBall.Update): void Update(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class DestroyAction : public AIAction {
  public:

    // 0x7cba850
    DestroyAction() = default;

    // TODO(0x7cba838, Quantum.DestroyAction.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class DispatchMobSlamAttacksEventAction : public AIAction {
  public:
    AIParamMobSlamAttackEventType EventType;

    // 0x7cd3068
    DispatchMobSlamAttacksEventAction() = default;

    // TODO(0x7cd2fdc, Quantum.DispatchMobSlamAttacksEventAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DispatchMobsEventAction : public AIAction {
  public:
    AIParamMobsEventType EventType;

    // 0x7cd2f8c
    DispatchMobsEventAction() = default;

    // TODO(0x7cd2f00, Quantum.DispatchMobsEventAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext ctx)
};

class DispatchShooterEventAction : public AIAction {
  public:
    EnumAIParam<ShootingEnemyEventType> EventType;

    // 0x7cd585c
    DispatchShooterEventAction() = default;

    // TODO(0x7cd57d0, Quantum.DispatchShooterEventAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DispatchSpinnerEventAction : public AIAction {
  public:
    EnumAIParam<SpinningEnemyEventType> EventType;

    // 0x7cd58f0
    DispatchSpinnerEventAction() = default;

    // TODO(0x7cd5864, Quantum.DispatchSpinnerEventAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DistanceFromPositionDecision : public HFSMDecision {
  public:
    enum class ComparisonType : std::int32_t {
        LessThan = 0,
        EqualTo = 1,
        GreaterThan = 2,
    };

    AIParamFPVector3 StartPosition;
    AIParamFPVector3 EndPosition;
    AIParamFP DistanceToCheck;
    DistanceFromPositionDecision::ComparisonType Comparison{};

    // 0x7cca6a4
    DistanceFromPositionDecision() = default;

    // TODO(0x7cca530, Quantum.DistanceFromPositionDecision.Decide): bool Decide(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class FindTargetAction : public AIAction {
  public:
    AIBlackboardValueKey Target;
    AIParamBool UseTargetFinder;
    AIParamFP TargetFinderRangeOverride;
    AIParamBool UsePushedBy;
    AIParamBool UseHitBy;

    // 0x7cbacc4
    FindTargetAction() = default;

    // TODO(0x7cba858, Quantum.FindTargetAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class GameIsStarted : public HFSMDecision {
  public:

    // 0x7cca744
    GameIsStarted() = default;

    // TODO(0x7cca6ac, Quantum.GameIsStarted.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class GoTo : public AIAction {
  public:
    AIParamGotoDestinationType destinationType;

    // 0x7cc5334
    GoTo() = default;

    // TODO(0x7cc4bbc, Quantum.GoTo.Update): void Update(Frame f, EntityRef agentEntity, AIContext
    //   aiContext)
};

class GoToControlZone : public AIAction {
  public:
    FP RepathMinInterval = FP::FromInt(3);
    FP RepathMaxInterval = FP::FromInt(6);

    // 0x7cd0e90
    GoToControlZone() = default;

    // TODO(0x7cd0b74, Quantum.GoToControlZone.Update): void Update(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
    // TODO(0x7cd0dac, Quantum.GoToControlZone.TryGetRandomPositionInZone): bool
    //   TryGetRandomPositionInZone(Frame frame, ControlZoneData zoneData, FPVector3 randomPosition)
};

class GoToPlayerDebugPos : public AIAction {
  public:

    // 0x7cc53b8
    GoToPlayerDebugPos() = default;

    // TODO(0x7cc533c, Quantum.GoToPlayerDebugPos.Update): void Update(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class GroundDashAction : public AIAction {
  public:

    // 0x7cbad70
    GroundDashAction() = default;

    // TODO(0x7cbaccc, Quantum.GroundDashAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class HasLineOfSightToTargetDecision : public HFSMDecision {
  public:
    AIBlackboardValueKey TargetKey;

    // 0x7cca950
    HasLineOfSightToTargetDecision() = default;

    // TODO(0x7cca74c, Quantum.HasLineOfSightToTargetDecision.Decide): bool Decide(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class HasMoveBehaviourBouncedDecision : public HFSMDecision {
  public:

    // 0x7ccabf8
    HasMoveBehaviourBouncedDecision() = default;

    // TODO(0x7ccab50, Quantum.HasMoveBehaviourBouncedDecision.Decide): bool Decide(Frame f,
    //   EntityRef entity, AIContext aiContext)
};

class HasReachedTime : public HFSMDecision {
  public:
    AIBlackboardValueKey TimerKey;
    AIParamFP TargetTime;

    // 0x7ccac98
    HasReachedTime() = default;

    // TODO(0x7ccac00, Quantum.HasReachedTime.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class HasTarget : public HFSMDecision {
  public:
    AIBlackboardValueKey TargetKey;

    // 0x7ccacd8
    HasTarget() = default;

    // TODO(0x7ccaca0, Quantum.HasTarget.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class IdleAction : public AIAction {
  public:

    // 0x7b89e88
    IdleAction() = default;

    // TODO(0x7b89e84, Quantum.IdleAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class ImpulseAction : public AIAction {
  public:
    AIParamFP ImpulseForceParam;
    AIParamFPVector3 ImpulseForwardOffsetParam;
    AIParamFP DurationParam;
    AIParamFP InputFalloffParam;

    // 0x7cbafb4
    ImpulseAction() = default;

    // TODO(0x7cbad78, Quantum.ImpulseAction.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class ImpulseTowardTargetAction : public AIAction {
  public:
    AIParamFP ImpulseForceParam;
    AIParamFPVector3 ImpulseForwardOffsetParam;
    AIParamFP DurationParam;
    AIParamFP InputFalloffParam;
    AIParamFP InputDurationParam;
    AIBlackboardValueKey TargetEntityKey;

    // 0x7cbb35c
    ImpulseTowardTargetAction() = default;

    // TODO(0x7cbafbc, Quantum.ImpulseTowardTargetAction.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class IsDeadDecision : public HFSMDecision {
  public:

    // 0x7ccad5c
    IsDeadDecision() = default;

    // TODO(0x7ccace0, Quantum.IsDeadDecision.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class IsGroundedDecision : public HFSMDecision {
  public:

    // 0x7ccae08
    IsGroundedDecision() = default;

    // TODO(0x7ccad64, Quantum.IsGroundedDecision.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class IsTargetValidDecision : public HFSMDecision {
  public:
    AIParamFP DetectionRadius;
    AIBlackboardValueKey TargetValueKey;

    // 0x7ccafb8
    IsTargetValidDecision() = default;

    // TODO(0x7ccae10, Quantum.IsTargetValidDecision.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class JumpAction : public AIAction {
  public:
    AIParamFP ImpulseForce = AIParamFP(FP::FromInt(1));

    // 0x7cbb4f0
    JumpAction() = default;

    // TODO(0x7cbb364, Quantum.JumpAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class JumpAttackAction : public AIAction {
  public:
    AIParamFPVector3 TargetPosition;

    // 0x7cc7e78
    JumpAttackAction() = default;

    // TODO(0x7cc7c80, Quantum.JumpAttackAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class MoveTowardTargetAction : public AIAction {
  public:
    AIParamEntityRef Target;
    AIParamFP CloseDistance = AIParamFP(FP::FromRaw(0));
    AIParamBool ResetActorInputDirectionWhenClose;
    AIParamBool PreserveForceLookInput;

    // 0x7cbb87c
    MoveTowardTargetAction() = default;

    // TODO(0x7cbb524, Quantum.MoveTowardTargetAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class NavigateToTargetAction : public AIAction {
  public:
    NavigateToTargetAction_NavType NavType;
    AIParamInt TickFrequency = AIParamInt(1);
    AIParamEntityRef TargetEntity;
    AIParamFPVector3 TargetPosition;
    AIParamFP NearbyTargetDistance = AIParamFP(FP::FromRaw(32768));
    AIBlackboardValueKey TargetKey;
    AIBlackboardValueKey TargetPositionKey;
    AIBlackboardValueKey PathfindStatusKey;
    bool MaintainInputDirectionWhenRepathing = true;

    // 0x7cbc0e8
    NavigateToTargetAction() = default;

    // TODO(0x7cbb8f8, Quantum.NavigateToTargetAction.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7cbc018, Quantum.NavigateToTargetAction.GetNearbyPointToTarget): FPVector3
    //   GetNearbyPointToTarget(Frame f, FPVector3 targetPos, FPVector3 agentPos, FP distance)
};

class PrepareShootingAction : public AIAction {
  public:

    // 0x7be73fc
    PrepareShootingAction() = default;

    // TODO(0x7be7330, Quantum.PrepareShootingAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class RaiseAgentOfDoomViewEvent : public AIAction {
  public:
    AgentOfDoomEventType EventType{};

    // 0x7cd2a80
    RaiseAgentOfDoomViewEvent() = default;

    // TODO(0x7cd2a5c, Quantum.RaiseAgentOfDoomViewEvent.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class ReachedPointDecision : public HFSMDecision {
  public:
    AIBlackboardValueKey TargetPositionKey;
    AIParamFPVector3 TargetPosition;
    AIParamFP Tolerance;

    // 0x7ccb1a8
    ReachedPointDecision() = default;

    // TODO(0x7ccb038, Quantum.ReachedPointDecision.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class ResetMaxSpeedModifier : public AIAction {
  public:

    // 0x7cbc1e8
    ResetMaxSpeedModifier() = default;

    // TODO(0x7cbc148, Quantum.ResetMaxSpeedModifier.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class ResetTargetAction : public AIAction {
  public:
    AIBlackboardValueKey Target;

    // 0x7cbc22c
    ResetTargetAction() = default;

    // TODO(0x7cbc1f0, Quantum.ResetTargetAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class ResetTimerAction : public AIAction {
  public:
    AIBlackboardValueKey TimerKey;

    // 0x7cbc24c
    ResetTimerAction() = default;

    // TODO(0x7cbc234, Quantum.ResetTimerAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class SeekNewTarget : public HFSMDecision {
  public:
    AIParamFP DetectionRadius;
    AIParamInt DetectionFrequency;
    AIBlackboardValueKey TargetKey;
    AIParamAssetRef TargetPriorityList;

    // 0x7ccba70
    SeekNewTarget() = default;

    // TODO(0x7ccb1b0, Quantum.SeekNewTarget.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7ccb2bc, Quantum.SeekNewTarget.FindTargetInRadius): EntityRef FindTargetInRadius(Frame
    //   f, EntityRef entity, AIContext aiContext)
};

class SetAttackInfoActiveAction : public AIAction {
  public:
    AIParamBool Active;
    AIParamEntityRef TargetEntity;
    AIParamBool WasRunningBeforeAttack;

    // 0x7cd2eb0
    SetAttackInfoActiveAction() = default;

    // TODO(0x7cd2d5c, Quantum.SetAttackInfoActiveAction.Update): void Update(Frame f, EntityRef
    //   entity, AIContext ctx)
};

class SetCompoundHFSMSchedulePeriodAction : public AIAction {
  public:
    enum class ScheduleMode : std::int32_t {
        Fast = 0,
        Medium = 1,
        Slow = 2,
    };

    SetCompoundHFSMSchedulePeriodAction::ScheduleMode mode{};

    // 0x7cca528
    SetCompoundHFSMSchedulePeriodAction() = default;

    // TODO(0x7cca450, Quantum.SetCompoundHFSMSchedulePeriodAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class SetCurrentMaxSpeedModifier : public AIAction {
  public:
    AIParamFP MaxSpeedModifier;

    // 0x7cbc340
    SetCurrentMaxSpeedModifier() = default;

    // TODO(0x7cbc254, Quantum.SetCurrentMaxSpeedModifier.Update): void Update(Frame f, EntityRef
    //   entity, AIContext aiContext)
};

class SetGravityModifierAction : public AIAction {
  public:
    AIParamFP Value = AIParamFP(FP::FromRaw(0));
    AIParamFP Duration = AIParamFP(FP::FromInt(-1));

    // 0x7cbc4cc
    SetGravityModifierAction() = default;

    // TODO(0x7cbc348, Quantum.SetGravityModifierAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class SetGroupTargetAction : public AIAction {
  public:
    AIParamEntityRef TargetEntity;

    // 0x7cd3ed8
    SetGroupTargetAction() = default;

    // TODO(0x7cd3d74, Quantum.SetGroupTargetAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class SetOwnerInBlackboardAction : public AIAction {
  public:
    AIBlackboardValueKey OwnerKey;

    // 0x7cbc650
    SetOwnerInBlackboardAction() = default;

    // TODO(0x7cbc51c, Quantum.SetOwnerInBlackboardAction.Update): void Update(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SetPositionInBlackboardAction : public AIAction {
  public:
    AIParamBool Condition = AIParamBool(true);
    AIBlackboardValueKey Position;
    AIParamEntityRef TransformEntity;

    // 0x7cbc7ec
    SetPositionInBlackboardAction() = default;

    // TODO(0x7cbc658, Quantum.SetPositionInBlackboardAction.Update): void Update(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SetShootPrecision : public AIAction {
  public:

    // 0x7cc5ac0
    SetShootPrecision() = default;

    // TODO(0x7cc5820, Quantum.SetShootPrecision.Update): void Update(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SetSpawnerPositionInBlackboardAction : public AIAction {
  public:
    AIBlackboardValueKey SpawnerPosition;

    // 0x7cbcbac
    SetSpawnerPositionInBlackboardAction() = default;

    // TODO(0x7cbc820, Quantum.SetSpawnerPositionInBlackboardAction.Update): void Update(Frame f,
    //   EntityRef entity, AIContext aiContext)
};

class SetSpawnerWanderRadiusInBlackboardAction : public AIAction {
  public:
    AIBlackboardValueKey SpawnerWanderRadius;

    // 0x7cbceec
    SetSpawnerWanderRadiusInBlackboardAction() = default;

    // TODO(0x7cbcbb4, Quantum.SetSpawnerWanderRadiusInBlackboardAction.Update): void Update(Frame
    //   f, EntityRef entity, AIContext aiContext)
};

class SetupAoEAction : public AIAction {
  public:
    AIParamBool Enabled;
    AIParamFP Range;

    // 0x7cbd084
    SetupAoEAction() = default;

    // TODO(0x7cbcef4, Quantum.SetupAoEAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class ShootBallAction : public AIAction {
  public:

    // 0x7cd09f8
    ShootBallAction() = default;

    // TODO(0x7cd0604, Quantum.ShootBallAction.Update): void Update(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class ShootTarget : public AIAction {
  public:

    // 0x7cc5ebc
    ShootTarget() = default;

    // TODO(0x7cc5ac8, Quantum.ShootTarget.Update): void Update(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class StartMeleeAttackAction : public AIAction {
  public:
    AIParamFP Delay;

    // 0x7cc89d0
    StartMeleeAttackAction() = default;

    // TODO(0x7cc8844, Quantum.StartMeleeAttackAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class StartShootingAction : public AIAction {
  public:
    AIParamEntityRef CustomShootTarget;

    // 0x7be7518
    StartShootingAction() = default;

    // TODO(0x7be7404, Quantum.StartShootingAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class StopNavigationAction : public AIAction {
  public:

    // 0x7cbd18c
    StopNavigationAction() = default;

    // TODO(0x7cbd08c, Quantum.StopNavigationAction.Update): void Update(Frame f, EntityRef entity,
    //   AIContext aiContext)
};

class StopPathFinding : public AIAction {
  public:

    // 0x7cc4bb4
    StopPathFinding() = default;

    // TODO(0x7cc4ba0, Quantum.StopPathFinding.Update): void Update(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class StopShootingAction : public AIAction {
  public:

    // 0x7be75d0
    StopShootingAction() = default;

    // TODO(0x7be7520, Quantum.StopShootingAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class SweepAttackAction : public AIAction {
  public:

    // 0x7cc9860
    SweepAttackAction() = default;

    // TODO(0x7cc96c4, Quantum.SweepAttackAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class SwitchWeapon : public AIAction {
  public:
    // static std::vector<std::int32_t> possibleSlots: see the .cpp file

    std::int32_t minCooldown_Seconds = 3;
    std::int32_t maxCooldown_Seconds = 10;

    // 0x7cc4ac0
    SwitchWeapon() = default;

    // TODO(0x7cc4abc, Quantum.SwitchWeapon.Update): void Update(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class SwitchWeaponUltimate : public AIAction {
  public:

    // 0x7cc4b98
    SwitchWeaponUltimate() = default;

    // TODO(0x7cc4b70, Quantum.SwitchWeaponUltimate.Update): void Update(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class TimerDecision : public HFSMDecision {
  public:
    AIParamFP TimeToTrueState = AIParamFP(FP::FromInt(3));

    // 0x7b8a224
    TimerDecision() = default;

    // TODO(0x7b8a198, Quantum.TimerDecision.Decide): bool Decide(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class ToggleAoEAction : public AIAction {
  public:
    AIParamBool Enabled;

    // 0x7cbd2ec
    ToggleAoEAction() = default;

    // TODO(0x7cbd194, Quantum.ToggleAoEAction.Update): void Update(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class ToggleMovementBehaviourAction : public AIAction {
  public:
    AIParamBool Value;
    AIParamBool ConstrainToSpawner = AIParamBool(true);

    // 0x7cbd488
    ToggleMovementBehaviourAction() = default;

    // TODO(0x7cbd2f4, Quantum.ToggleMovementBehaviourAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class ToggleMovementPackSlicingAction : public AIAction {
  public:
    AIParamBool Value;

    // 0x7cbd614
    ToggleMovementPackSlicingAction() = default;

    // TODO(0x7cbd4bc, Quantum.ToggleMovementPackSlicingAction.Update): void Update(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class ToggleTargetFinderAction : public AIAction {
  public:
    AIParamBool Value;

    // 0x7cbd778
    ToggleTargetFinderAction() = default;

    // TODO(0x7cbd61c, Quantum.ToggleTargetFinderAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class TurnTowardTargetAction : public AIAction {
  public:
    AIParamEntityRef Target;
    AIParamBool IsActive = AIParamBool(true);
    AIParamBool IsInstant;
    AIParamBool PreserveForceLookInput;

    // 0x7cbdad8
    TurnTowardTargetAction() = default;

    // TODO(0x7cbd780, Quantum.TurnTowardTargetAction.Update): void Update(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class UpdateStrikeZonePositionAction : public AIAction {
  public:
    AIBlackboardValueKey PreviousBallPosition;
    AIBlackboardValueKey StrikeZonePosition;
    FP OwnGoalDistanceThreshold = FP::FromInt(5);
    FP EnemyGoalDistanceThreshold = FP::FromInt(5);

    // 0x7cce7a8
    UpdateStrikeZonePositionAction() = default;

    // TODO(0x7cce480, Quantum.UpdateStrikeZonePositionAction.Update): void Update(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class UseGadget : public AIAction {
  public:
    FP minCooldown = FP::FromInt(7);
    FP maxCooldown = FP::FromInt(12);

    // 0x7cc580c
    UseGadget() = default;

    // TODO(0x7cc55a8, Quantum.UseGadget.Update): void Update(Frame frame, EntityRef agentEntity,
    //   AIContext aiContext)
};

class UseWrench : public AIAction {
  public:
    FP useDelay = FP::FromRaw(32768); // 0.5

    // 0x7cc5598
    UseWrench() = default;

    // TODO(0x7cc53c0, Quantum.UseWrench.Update): void Update(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class Wander : public AIAction {
  public:
    FP minWanderDist{};
    FP maxWanderDist{};
    FP minPauseDelayOnDestinationReached{};
    FP maxPauseDelayOnDestinationReached{};

    // 0x7cc639c
    Wander() = default;

    // TODO(0x7cc5ec4, Quantum.Wander.Update): void Update(Frame f, EntityRef agentEntity, AIContext
    //   aiContext)
    // TODO(0x7cc6178, Quantum.Wander.FindWanderPosition): bool FindWanderPosition(Frame f,
    //   AIContext aiContext, NavMesh navmesh, FPVector3 wanderPoint)
};

class WithinRangeDecision : public HFSMDecision {
  public:
    AIBlackboardValueKey TargetKey;
    AIParamEntityRef Target;
    AIParamFP AttackRangeParam;
    AIParamBool UseYTolerance;
    AIParamFP YTolerance;

    // 0x7ccc0b4
    WithinRangeDecision() = default;

    // TODO(0x7ccba8c, Quantum.WithinRangeDecision.Decide): bool Decide(Frame f, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7ccc004, Quantum.WithinRangeDecision.GetTargetBoundSize): FP GetTargetBoundSize(Frame
    //   frame, EntityRef target, FP baseSize)
    // TODO(0x7ccbea4, Quantum.WithinRangeDecision.GetSqrDistToClosestPointOnBox): FP
    //   GetSqrDistToClosestPointOnBox(Frame frame, EntityRef target, Transform3D* targetTransform,
    //   PhysicsCollider3D* targetCollider, FPVector3 entityPos)
};

class AIFunctionAND : public AIFunctionT<bool> {
  public:
    AIParamBool ValueA;
    AIParamBool ValueB;

    // 0x7b7ed9c
    AIFunctionAND() = default;

    // TODO(0x7b7ec48, Quantum.AIFunctionAND.Execute): bool Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b7ecec, Quantum.AIFunctionAND.Execute): bool Execute(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
};

class AIFunctionAssetRefList : public AIFunctionT<Collections::QList<AssetRef>> {
  public:
};

class AIFunctionBoolList : public AIFunctionT<Collections::QList<bool>> {
  public:
};

class AIFunctionByteList : public AIFunctionT<Collections::QList<std::uint8_t>> {
  public:
};

class AIFunctionEntityRefList : public AIFunctionT<Collections::QList<EntityRef>> {
  public:
};

class AIFunctionFPList : public AIFunctionT<Collections::QList<FP>> {
  public:
};

class AIFunctionFPVector2List : public AIFunctionT<Collections::QList<FPVector2>> {
  public:
};

class AIFunctionFPVector3List : public AIFunctionT<Collections::QList<FPVector3>> {
  public:
};

class AIFunctionIntegerList : public AIFunctionT<Collections::QList<std::int32_t>> {
  public:
};

class AIFunctionNOT : public AIFunctionT<bool> {
  public:
    AIParamBool Value;

    // 0x7b7eee4
    AIFunctionNOT() = default;

    // TODO(0x7b7ede4, Quantum.AIFunctionNOT.Execute): bool Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b7ee60, Quantum.AIFunctionNOT.Execute): bool Execute(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
};

class AIFunctionOR : public AIFunctionT<bool> {
  public:
    AIParamBool ValueA;
    AIParamBool ValueB;

    // 0x7b7f080
    AIFunctionOR() = default;

    // TODO(0x7b7ef2c, Quantum.AIFunctionOR.Execute): bool Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
    // TODO(0x7b7efd0, Quantum.AIFunctionOR.Execute): bool Execute(FrameThreadSafe frame, EntityRef
    //   entity, AIContext aiContext)
};

class AimAtPassingTargetEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;

    // 0x7cce8d4
    AimAtPassingTargetEvaluator() = default;

    // TODO(0x7cce868, Quantum.AimAtPassingTargetEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class AirSaveEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 80;
    std::int32_t dashDelayMin_Milliseconds = 264;
    std::int32_t dashDelayMax_Milliseconds = 924;
    std::int32_t minAffinityCooldown_Seconds = 10;
    std::int32_t maxAffinityCooldown_Seconds{};

    // 0x7cc3434
    AirSaveEvaluator() = default;

    // TODO(0x7cc2da4, Quantum.AirSaveEvaluator.ResetAirSave): void ResetAirSave(Frame f, EntityRef
    //   agentEntity, bool releaseJumpButton, FP cooldownToSet)
    // TODO(0x7cc2f38, Quantum.AirSaveEvaluator.Execute): FP Execute(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class ArithmeticFPFunction : public AIFunctionT<FP> {
  public:
    enum class Operation : std::int32_t {
        Add = 0,
        Subtract = 1,
        Multiply = 2,
        Divide = 3,
    };

    ArithmeticFPFunction::Operation operation{};
    AIParamFP first;
    AIParamFP second;

    // 0x7cd141c
    ArithmeticFPFunction() = default;

    // TODO(0x7cd12a4, Quantum.ArithmeticFPFunction.Execute): FP Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class AttackControlZoneEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 40;
    std::int32_t contestingUtility_0_100 = 80;

    // 0x7cd1250
    AttackControlZoneEvaluator() = default;

    // TODO(0x7cd0ea4, Quantum.AttackControlZoneEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class AttackPlayerEvaluator : public AIFunctionT<FP> {
  public:
    static constexpr std::int32_t ATTACK_POS_LOS_CANDIDATES = 9;

    std::int32_t utilityDefaultTarget = 10;
    std::int32_t utilityPlayerTarget = 45;
    std::int32_t utilityNPCTarget = 15;
    std::int32_t realPlayerSearchRadius = 10;
    std::int32_t durationMinSeconds = 10;
    std::int32_t durationMaxSeconds = 20;
    std::int32_t coolDownMinSeconds = 5;
    std::int32_t coolDownMaxSeconds = 30;
    std::vector<EntityRef> targetCandidates = std::vector<EntityRef>(32); // not stored

    // 0x7cc1d80
    AttackPlayerEvaluator() = default;

    // TODO(0x7cbfec8, Quantum.AttackPlayerEvaluator.StopAttackAndStartCooldown): void
    //   StopAttackAndStartCooldown(Frame f, Bot* pBot)
    // TODO(0x7cbff5c, Quantum.AttackPlayerEvaluator.ComputeAttackPosition): bool
    //   ComputeAttackPosition(Frame f, AIContext aiContext, EntityRef targetPlayer, FPVector3
    //   attackPos, bool needsLineOfSight, bool forceSuccess)
    // TODO(0x7cc052c, Quantum.AttackPlayerEvaluator.TrySnapToNavMesh): bool TrySnapToNavMesh(Frame
    //   f, AIContext aiContext, FPVector3 groundPos, FPVector3 navMeshPos)
    // TODO(0x7cc05f4, Quantum.AttackPlayerEvaluator.HasLineOfSight): bool HasLineOfSight(Frame f,
    //   FPVector3 fromGroundPos, FPVector3 targetPos)
    // TODO(0x7cc0770, Quantum.AttackPlayerEvaluator.CanAttackTarget): bool CanAttackTarget(Frame f,
    //   GAME_MODE gameMode, EntityRef botEntity, int botTeamID, EntityRef targetEntity, bool
    //   ignoreAggroBehaviour, EntityInfoType targetType)
    // TODO(0x7cc09b4, Quantum.AttackPlayerEvaluator.FindTargetToAttack): EntityRef
    //   FindTargetToAttack(Frame f, AIContext aiContext, FPVector3 botPosition, EntityRef
    //   botEntity, EntityInfoType targetType)
    // TODO(0x7cc12b4, Quantum.AttackPlayerEvaluator.IsTargetAlreadyAttacked): bool
    //   IsTargetAlreadyAttacked(Frame f, EntityRef self, EntityRef target, EntityRef
    //   otherAttackerForTarget)
    // TODO(0x7cc10a4, Quantum.AttackPlayerEvaluator.TryGetHumanAttacker): bool
    //   TryGetHumanAttacker(Frame f, EntityRef self, EntityRef entityRef)
    // TODO(0x7cc151c, Quantum.AttackPlayerEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
    // TODO(0x7cc1cf4, Quantum.AttackPlayerEvaluator.GetTargetUtilityValue): int
    //   GetTargetUtilityValue(EntityInfoType targetType)
};

class ChargeAttackGetMaxDistanceFunction : public AIFunctionT<FP> {
  public:

    // 0x7cc7718
    ChargeAttackGetMaxDistanceFunction() = default;

    // TODO(0x7cc7624, Quantum.ChargeAttackGetMaxDistanceFunction.Execute): FP Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class ChargeAttackInProgressFunction : public AIFunctionT<bool> {
  public:

    // 0x7cc7808
    ChargeAttackInProgressFunction() = default;

    // TODO(0x7cc7760, Quantum.ChargeAttackInProgressFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class ChargeWrenchEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;
    FP startChargingHealthThreshold = FP::FromInt(50);
    FP releaseChargeHealthThreshold = FP::FromInt(20);
    FP minShootDistance = FP::FromInt(20);
    FP maxShootDistance = FP::FromInt(40);

    // 0x7ccef14
    ChargeWrenchEvaluator() = default;

    // TODO(0x7cceeb8, Quantum.ChargeWrenchEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class DashBlastBallEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 40;
    FP maxDistanceToUse = FP::FromInt(15);
    std::int32_t minCooldown_Seconds{};
    std::int32_t maxCooldown_Seconds = 5;

    // 0x7ccf574
    DashBlastBallEvaluator() = default;

    // TODO(0x7ccf0e8, Quantum.DashBlastBallEvaluator.RefreshCurrentCustomTarget): bool
    //   RefreshCurrentCustomTarget(Frame f, AIContext aiContext)
    // TODO(0x7ccf364, Quantum.DashBlastBallEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class DefaultAIFunctionAssetRef : public AIFunctionT<AssetRef> {
  public:

    // 0x7b7e92c
    DefaultAIFunctionAssetRef() = default;

    // TODO(0x7b7e920, Quantum.DefaultAIFunctionAssetRef.Execute): AssetRef Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DefaultAIFunctionBool : public AIFunctionT<bool> {
  public:

    // 0x7b7e97c
    DefaultAIFunctionBool() = default;

    // TODO(0x7b7e974, Quantum.DefaultAIFunctionBool.Execute): bool Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class DefaultAIFunctionByte : public AIFunctionT<std::uint8_t> {
  public:

    // 0x7b7e9cc
    DefaultAIFunctionByte() = default;

    // TODO(0x7b7e9c4, Quantum.DefaultAIFunctionByte.Execute): byte Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class DefaultAIFunctionEntityRef : public AIFunctionT<EntityRef> {
  public:

    // 0x7b7ea1c
    DefaultAIFunctionEntityRef() = default;

    // TODO(0x7b7ea14, Quantum.DefaultAIFunctionEntityRef.Execute): EntityRef Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DefaultAIFunctionFP : public AIFunctionT<FP> {
  public:

    // 0x7b7ea6c
    DefaultAIFunctionFP() = default;

    // TODO(0x7b7ea64, Quantum.DefaultAIFunctionFP.Execute): FP Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class DefaultAIFunctionFPVector2 : public AIFunctionT<FPVector2> {
  public:

    // 0x7b7eabc
    DefaultAIFunctionFPVector2() = default;

    // TODO(0x7b7eab4, Quantum.DefaultAIFunctionFPVector2.Execute): FPVector2 Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DefaultAIFunctionFPVector3 : public AIFunctionT<FPVector3> {
  public:

    // 0x7b7eb60
    DefaultAIFunctionFPVector3() = default;

    // TODO(0x7b7eb04, Quantum.DefaultAIFunctionFPVector3.Execute): FPVector3 Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DefaultAIFunctionInt : public AIFunctionT<std::int32_t> {
  public:

    // 0x7b7ebb0
    DefaultAIFunctionInt() = default;

    // TODO(0x7b7eba8, Quantum.DefaultAIFunctionInt.Execute): int Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class DefaultAIFunctionString : public AIFunctionT<std::string> {
  public:

    // 0x7b7ec00
    DefaultAIFunctionString() = default;

    // TODO(0x7b7ebf8, Quantum.DefaultAIFunctionString.Execute): string Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class DefendControlZoneEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 85;

    // 0x7cd0b24
    DefendControlZoneEvaluator() = default;

    // TODO(0x7cd0a00, Quantum.DefendControlZoneEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class EntityExistsFunction : public AIFunctionT<bool> {
  public:
    AIParamEntityRef entityToTest;

    // 0x7cd1514
    EntityExistsFunction() = default;

    // TODO(0x7cd1464, Quantum.EntityExistsFunction.Execute): bool Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class EntityIsGroundedFunction : public AIFunctionT<bool> {
  public:
    AIParamEntityRef entityToTest;

    // 0x7cd1640
    EntityIsGroundedFunction() = default;

    // TODO(0x7cd155c, Quantum.EntityIsGroundedFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class EntityRefsAreEqualFunction : public AIFunctionT<bool> {
  public:
    AIParamEntityRef First;
    AIParamEntityRef Second;

    // 0x7cd1728
    EntityRefsAreEqualFunction() = default;

    // TODO(0x7cd1688, Quantum.EntityRefsAreEqualFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class EscapeShrinkingZoneEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 90;
    std::int32_t distanceToZoneBorder = 2;

    // 0x7cc2d50
    EscapeShrinkingZoneEvaluator() = default;

    // TODO(0x7cc2c20, Quantum.EscapeShrinkingZoneEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class FrameTimeHasElapsedFunction : public AIFunctionT<bool> {
  public:
    AIParamInt StartFrame;
    AIParamFP Duration;

    // 0x7cd1850
    FrameTimeHasElapsedFunction() = default;

    // TODO(0x7cd1770, Quantum.FrameTimeHasElapsedFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetAttackInfoActiveFunction : public AIFunctionT<bool> {
  public:

    // 0x7cd2b34
    GetAttackInfoActiveFunction() = default;

    // TODO(0x7cd2a88, Quantum.GetAttackInfoActiveFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetAttackInfoTargetFunction : public AIFunctionT<EntityRef> {
  public:

    // 0x7cd2c20
    GetAttackInfoTargetFunction() = default;

    // TODO(0x7cd2b7c, Quantum.GetAttackInfoTargetFunction.Execute): EntityRef Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetAttackInfoWasRunningFunction : public AIFunctionT<bool> {
  public:

    // 0x7cd2d14
    GetAttackInfoWasRunningFunction() = default;

    // TODO(0x7cd2c68, Quantum.GetAttackInfoWasRunningFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetEntityPositionFunction : public AIFunctionT<FPVector3> {
  public:
    AIParamEntityRef entityToTest;

    // 0x7cd19c0
    GetEntityPositionFunction() = default;

    // TODO(0x7cd1898, Quantum.GetEntityPositionFunction.Execute): FPVector3 Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetFrameNumberFunction : public AIFunctionT<std::int32_t> {
  public:

    // 0x7cd1a1c
    GetFrameNumberFunction() = default;

    // TODO(0x7cd1a08, Quantum.GetFrameNumberFunction.Execute): int Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class GetGroupEntityFunction : public AIFunctionT<EntityRef> {
  public:

    // 0x7cd31e4
    GetGroupEntityFunction() = default;

    // TODO(0x7cd3140, Quantum.GetGroupEntityFunction.Execute): EntityRef Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetGroupTargetFunction : public AIFunctionT<EntityRef> {
  public:

    // 0x7cd3360
    GetGroupTargetFunction() = default;

    // TODO(0x7cd322c, Quantum.GetGroupTargetFunction.Execute): EntityRef Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class GetHighestPriorityTargetEntityFunction : public AIFunctionT<EntityRef> {
  public:

    // 0x7cd3fc4
    GetHighestPriorityTargetEntityFunction() = default;

    // TODO(0x7cd3ee0, Quantum.GetHighestPriorityTargetEntityFunction.Execute): EntityRef
    //   Execute(Frame frame, EntityRef entity, AIContext aiContext)
};

class GetHighestPriorityTargetPositionFunction : public AIFunctionT<FPVector3> {
  public:
    AIParamBool UseIntermediateWaypoint = AIParamBool(true);

    // 0x7cd414c
    GetHighestPriorityTargetPositionFunction() = default;

    // TODO(0x7cd400c, Quantum.GetHighestPriorityTargetPositionFunction.Execute): FPVector3
    //   Execute(Frame frame, EntityRef entity, AIContext aiContext)
};

class GetRandomFPFunction : public AIFunctionT<FP> {
  public:
    AIParamFP Min;
    AIParamFP Max;

    // 0x7cd1b1c
    GetRandomFPFunction() = default;

    // TODO(0x7cd1a64, Quantum.GetRandomFPFunction.Execute): FP Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class GetRecentPriorityTargetPositionFunction : public AIFunctionT<FPVector3> {
  public:

    // 0x7cd445c
    GetRecentPriorityTargetPositionFunction() = default;

    // TODO(0x7cd430c, Quantum.GetRecentPriorityTargetPositionFunction.Execute): FPVector3
    //   Execute(Frame frame, EntityRef entity, AIContext aiContext)
};

class GetTargetFinderRangeNoWeaponFunction : public AIFunctionT<FP> {
  public:

    // 0x7cd1d14
    GetTargetFinderRangeNoWeaponFunction() = default;

    // TODO(0x7cd1b64, Quantum.GetTargetFinderRangeNoWeaponFunction.Execute): FP Execute(Frame
    //   frame, EntityRef entity, AIContext aiContext)
};

class GetVec2Element : public AIFunctionT<FP> {
  public:
    enum class Element : std::int32_t {
        X = 0,
        Y = 1,
    };

    AIParamFPVector2 Input;
    GetVec2Element::Element Elem{};

    // 0x7cd1de4
    GetVec2Element() = default;

    // TODO(0x7cd1d5c, Quantum.GetVec2Element.Execute): FP Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class GroupTeamEvaluator : public AIFunctionT<FP> {
  public:
    static constexpr bool LOG = 0;
    static constexpr bool LOG_HUMAN_TEAM = 1;

    AIParamInt utilityCloseParam;
    AIParamInt utilityFarParam;
    AIParamInt utilityHelpLeaderParam;
    AIParamFP closeDistanceToTargetParam;
    AIParamFP farDistanceToTargetParam;

    // 0x7cbfdd4
    GroupTeamEvaluator() = default;

    // TODO(0x7cbf698, Quantum.GroupTeamEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class HasGroupTargetFunction : public AIFunctionT<bool> {
  public:

    // 0x7cd3428
    HasGroupTargetFunction() = default;

    // TODO(0x7cd33a8, Quantum.HasGroupTargetFunction.Execute): bool Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class HasNavLineOfSightToTargetFunction : public AIFunctionT<bool> {
  public:
    AIParamEntityRef targetEntityRef;

    // 0x7cd1ff4
    HasNavLineOfSightToTargetFunction() = default;

    // TODO(0x7cd1e2c, Quantum.HasNavLineOfSightToTargetFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class HasValidRecentPriorityTargetFunction : public AIFunctionT<bool> {
  public:

    // 0x7cd42c4
    HasValidRecentPriorityTargetFunction() = default;

    // TODO(0x7cd41b0, Quantum.HasValidRecentPriorityTargetFunction.Execute): bool Execute(Frame
    //   frame, EntityRef entity, AIContext aiContext)
};

class InShootingZoneMovementEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;
    FP minWeaponRangeMultiplier = FP::FromRaw(32768); // 0.5
    FP maxWeaponRangeMultiplier = FP::FromRaw(98304); // 1.5
    FP minCooldownTimer = FP::FromRaw(32768); // 0.5
    FP maxCooldownTimer = FP::FromInt(2);

    // 0x7ccf9fc
    InShootingZoneMovementEvaluator() = default;

    // TODO(0x7ccf604, Quantum.InShootingZoneMovementEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class InputCheatFollowPlayer : public AIFunctionT<FP> {
  public:

    // 0x7cbe8fc
    InputCheatFollowPlayer() = default;

    // TODO(0x7cbe8c8, Quantum.InputCheatFollowPlayer.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class KccIsMovingFunction : public AIFunctionT<bool> {
  public:

    // 0x7cd2100
    KccIsMovingFunction() = default;

    // TODO(0x7cd203c, Quantum.KccIsMovingFunction.Execute): bool Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class MeleeAttackIsActiveFunction : public AIFunctionT<bool> {
  public:

    // 0x7cc8320
    MeleeAttackIsActiveFunction() = default;

    // TODO(0x7cc8274, Quantum.MeleeAttackIsActiveFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
};

class MinMaxFPFunction : public AIFunctionT<FP> {
  public:
    enum class MinMaxMode : std::int32_t {
        Min = 0,
        Max = 1,
    };

    MinMaxFPFunction::MinMaxMode mode{};
    AIParamFP first;
    AIParamFP second;

    // 0x7cd22b8
    MinMaxFPFunction() = default;

    // TODO(0x7cd2148, Quantum.MinMaxFPFunction.Execute): FP Execute(Frame frame, EntityRef entity,
    //   AIContext aiContext)
};

class MoveAwayFromTeammatesEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;
    FP distanceThreshold = FP::FromInt(2);

    // 0x7cd027c
    MoveAwayFromTeammatesEvaluator() = default;

    // TODO(0x7ccfeb8, Quantum.MoveAwayFromTeammatesEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class MoveToClosestEnemyEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 10;

    // 0x7cc2004
    MoveToClosestEnemyEvaluator() = default;

    // TODO(0x7cc1e14, Quantum.MoveToClosestEnemyEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class MoveTowardsBallEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;

    // 0x7cce980
    MoveTowardsBallEvaluator() = default;

    // TODO(0x7cce924, Quantum.MoveTowardsBallEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class PassingEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 90;

    // 0x7ccefd8
    PassingEvaluator() = default;

    // TODO(0x7ccef7c, Quantum.PassingEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class PriorityTargetTypeCompareFunction : public AIFunctionT<bool> {
  public:
    enum class CompareMode : std::int32_t {
        Equals = 0,
        LessThan = 1,
        GreaterThan = 2,
    };

    AIParamPrioTargetType Value;
    PriorityTargetTypeCompareFunction::CompareMode Mode{};

    // 0x7cd46a0
    PriorityTargetTypeCompareFunction() = default;

    // TODO(0x7cd44ec, Quantum.PriorityTargetTypeCompareFunction.Execute): bool Execute(Frame frame,
    //   EntityRef entity, AIContext aiContext)
    // TODO(0x7cd4600, Quantum.PriorityTargetTypeCompareFunction.GetEnumValue): PrioTargetType
    //   GetEnumValue(Frame frame, EntityRef entity, AIContext aiContext)
};

class ReachEnemyGoalEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;

    // 0x7cce818
    ReachEnemyGoalEvaluator() = default;

    // TODO(0x7cce7bc, Quantum.ReachEnemyGoalEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class ReachShootingZoneEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;
    bool useWeaponRangeAsMaxDistance = true;
    FP weaponRangeMultiplier = FP::FromRaw(49152); // 0.75
    FP positionRefreshRate = FP::FromInt(2);

    // 0x7ccfe54
    ReachShootingZoneEvaluator() = default;

    // TODO(0x7ccfa64, Quantum.ReachShootingZoneEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class ReachStrikeZoneEvaluator : public AIFunctionT<FP> {
  public:
    AIBlackboardValueKey StrikeZonePosition;
    std::int32_t defaultUtility_0_100 = 50;

    // 0x7cce430
    ReachStrikeZoneEvaluator() = default;

    // TODO(0x7cce2c4, Quantum.ReachStrikeZoneEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SeekCrateEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t closeCrateDistance = 7;
    std::int32_t utility_farCrate_0_100 = 30;
    std::int32_t utility_closeCrate_0_100 = 40;

    // 0x7cbf210
    SeekCrateEvaluator() = default;

    // TODO(0x7cbea60, Quantum.SeekCrateEvaluator.IsAValidCrate): bool IsAValidCrate(Frame f,
    //   EntityRef entity, FPVector3 pos, ShrinkingZone* pZone, FP safetyMargin)
    // TODO(0x7cbeb1c, Quantum.SeekCrateEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SeekDroppableEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t utility = 80;

    // 0x7cbf648
    SeekDroppableEvaluator() = default;

    // TODO(0x7cbf26c, Quantum.SeekDroppableEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SeekGoldenBoltEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 40;
    std::int32_t utilityWhenClose_0_100 = 65;
    std::int32_t closeDistanceMeters = 5;
    std::int32_t fov = 75;

    // 0x7cc3e60
    SeekGoldenBoltEvaluator() = default;

    // TODO(0x7cc372c, Quantum.SeekGoldenBoltEvaluator.ComputeUtilityDependingOnDistance): FP
    //   ComputeUtilityDependingOnDistance(Frame f, GoldenBoltManager* goldenBoltManager, AIContext
    //   aiContext)
    // TODO(0x7cc3898, Quantum.SeekGoldenBoltEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class ShootBallEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;

    // 0x7cd05b4
    ShootBallEvaluator() = default;

    // TODO(0x7cd02d4, Quantum.ShootBallEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
};

class ShootEvaluator : public AIFunctionT<FP> {
  public:
    // static std::vector<PotentialTarget> sortedTargets: see the .cpp file

    std::int32_t defaultUtility_0_100 = 40;
    std::int32_t maxAngleInDegree = 65;
    FP blastBallDistanceThreshold = FP::FromInt(7);

    // 0x7cc2aa8
    ShootEvaluator() = default;

    // TODO(0x7cc2054, Quantum.ShootEvaluator.RefreshCurrentCustomTarget): bool
    //   RefreshCurrentCustomTarget(Frame f, AIContext aiContext)
    // TODO(0x7cc28c0, Quantum.ShootEvaluator.Execute): FP Execute(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class StopToShootEvaluator : public AIFunctionT<FP> {
  public:

    // 0x7cbfe80
    StopToShootEvaluator() = default;

    // TODO(0x7cbfe1c, Quantum.StopToShootEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SwitchWeaponEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t noCombatActivityDelayMin_Seconds = 2;
    std::int32_t noCombatActivityDelayMax_Seconds = 5;

    // 0x7cc34ec
    SwitchWeaponEvaluator() = default;

    // TODO(0x7cc3488, Quantum.SwitchWeaponEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class SwitchWeaponUltimateEvaluator : public AIFunctionT<FP> {
  public:

    // 0x7cc36e4
    SwitchWeaponUltimateEvaluator() = default;

    // TODO(0x7cc3540, Quantum.SwitchWeaponUltimateEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class TakeDefensivePositionEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 100;
    FP minForward = FP::FromInt(1);
    FP maxForward = FP::FromInt(4);
    FP maxRight = FP::FromInt(4);
    FP refreshRate = FP::FromInt(2);

    // 0x7ccee50
    TakeDefensivePositionEvaluator() = default;

    // TODO(0x7cce9d0, Quantum.TakeDefensivePositionEvaluator.Execute): FP Execute(Frame frame,
    //   EntityRef agentEntity, AIContext aiContext)
};

class UseGadgetEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 35;
    ENTITY_INFO_FILTER targetTypes = static_cast<ENTITY_INFO_FILTER>(4382);
    FP minInitialCooldown = FP::FromInt(30);
    FP maxInitialCooldown = FP::FromInt(60);

    // 0x7cc4a5c
    UseGadgetEvaluator() = default;

    // TODO(0x7cc45f4, Quantum.UseGadgetEvaluator.Execute): FP Execute(Frame frame, EntityRef
    //   entity, AIContext aiContext)
    // TODO(0x7cc484c, Quantum.UseGadgetEvaluator.HasValidTargetInRange): bool
    //   HasValidTargetInRange(Frame frame, EntityRef botEntity, GadgetSpec gadgetSpec)
};

class UseWrenchEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 40;
    std::int32_t maxAngleInDegree = 65;
    FP maxDistanceToUseWrench = FP::FromInt(10);
    FP minCooldown_Seconds{};
    FP maxCooldown_Seconds = FP::FromInt(5);

    // 0x7cc458c
    UseWrenchEvaluator() = default;

    // TODO(0x7cc3eb4, Quantum.UseWrenchEvaluator.RefreshCurrentCustomTarget): bool
    //   RefreshCurrentCustomTarget(Frame f, AIContext aiContext)
    // TODO(0x7cc42b8, Quantum.UseWrenchEvaluator.Execute): FP Execute(Frame f, EntityRef
    //   agentEntity, AIContext aiContext)
};

class WanderEvaluator : public AIFunctionT<FP> {
  public:
    std::int32_t defaultUtility_0_100 = 1;

    // 0x7cbea10
    WanderEvaluator() = default;

    // TODO(0x7cbe944, Quantum.WanderEvaluator.Execute): FP Execute(Frame f, EntityRef agentEntity,
    //   AIContext aiContext)
};

class PathfindStatusEqualsFunction : public EnumEqualsFunction<PATH_FINDING_STATUS> {
  public:

    // 0x7cd23a0
    PathfindStatusEqualsFunction() = default;

    // TODO(0x7cd2300, Quantum.PathfindStatusEqualsFunction.GetEnumValue): PATH_FINDING_STATUS
    //   GetEnumValue(Frame frame, EntityRef entity, AIContext aiContext)
};

class SweepAttackStateEqualsFunction : public EnumEqualsFunction<SweepAttackState> {
  public:

    // 0x7cc9908
    SweepAttackStateEqualsFunction() = default;

    // TODO(0x7cc9868, Quantum.SweepAttackStateEqualsFunction.GetEnumValue): SweepAttackState
    //   GetEnumValue(Frame frame, EntityRef entity, AIContext aiContext)
};

} // namespace Quantum
