// quantum_code_factories.cpp - builds the gameplay simulation's asset classes
// (quantum.code.dll) and quantum.core's built-in assets from the decoded `Settings` data of
// the game's content files (PC-side glue; new code).
//
// One ReadFields overload per class or stored value type copies its serialised fields - the
// public, non-[NonSerialized] ones, in declaration order, base class first - out of the
// decoded tree; settings_reader.h reads the individual field types. Fields filled by
// Loaded() (resolved references, lookup tables) are not stored and are not read.
//
// Each concrete asset class is registered under the name of the engine-side script class
// that wraps it, which is the C++ class name plus "Asset" (WeaponSpecDefault ->
// "WeaponSpecDefaultAsset").
#include "content/quantum_code_factories.h"

#include "content/quantum_content_provider.h"
#include "content/settings_reader.h"
#include "quantum_code/assets/level_assets.h"

#include <memory>

namespace rr::content {

using namespace Quantum; // the generated readers name the simulation's types unqualified

// The bot SDK's generic parameter types.
template <class T> void ReadFields(const Value& value, Quantum::AIParam<T>& out, ReaderTag) {
    Read(value, "Source", out.Source);
    Read(value, "Key", out.Key);
    Read(value, "DefaultValue", out.DefaultValue);
}

template <class T> void ReadFields(const Value& value, Quantum::EnumAIParam<T>& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIParam<T>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

// Generated value types and asset classes.
void ReadFields(const Value& value, Quantum::AIBlackboardValueKey& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBlackboardEntry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorPhysicsSurfaceSettings& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorStatusTypeSettings& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTDataIndex& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ElementMultiplier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageTypeToElementMapping& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DifficultyIntervalInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ElectricalDamageSettings& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityMultiplierPair& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GadgetSpecModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModesTableEntry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMSetupData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMTransition& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HeightRangePercentPair& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IceDamageSettings& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MeleeComboAttackData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PveScalingLevelEntry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MobLevelMappingData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MovingWallOffset& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RandomWeaponsModifierSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ResponseCurvePack& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TrajectoryCurveData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateSpecModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveSpawnRequest& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponLoadoutData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponSpecModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponStatModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponVisualParams& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamAssetRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamBool& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamEntityRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamFP& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamFPVector2& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamFPVector3& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamGotoDestinationType& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamMobSlamAttackEventType& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamMobsEventType& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIParamPrioTargetType& out, ReaderTag);
void ReadFields(const Value& value, Quantum::NavigateToTargetAction_NavType& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBlackboard& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBlackboardInitializer& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBlackboardInitializer::AIBlackboardInitialValue& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBlackboardInitializer::AIBlackboardInitialValueEntry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIConfig::KeyValuePair& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIConfig::Value& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTNode& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTService& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CompoundHFSMSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Consideration& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPBackValidation& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPGoal& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPHeuristic& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPRoot& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMRoot& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMState& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMTransitionSet& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UTRoot& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTComposite& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTDecorator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTLeaf& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CheckBlackboardInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DebugAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DebugService& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPDefaultAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPDefaultGoal& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GOAPSingleBackValidation& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMLogicalDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IncreaseBlackboardInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IsTrue& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetBlackboardBool& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetBlackboardEntityRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetBlackboardFP& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetBlackboardFPVector3& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetBlackboardInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TrueDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTBlackboardCompare& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTCooldown& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTForceResult& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTLoop& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTRoot& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTSelector& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTSelectorRandom& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BTSequence& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DebugLeaf& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMAndDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMNotDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HFSMOrDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ResponseCurve& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaitLeaf& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AddTargetToBlacklistAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeAttackStartAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeWrenchAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChoosePatrolDestinationAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DashRoboBall& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DestroyAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DispatchMobSlamAttacksEventAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DispatchMobsEventAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DispatchShooterEventAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DispatchSpinnerEventAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DistanceFromPositionDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FindTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameIsStarted& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GoTo& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GoToControlZone& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GoToPlayerDebugPos& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GroundDashAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasLineOfSightToTargetDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasMoveBehaviourBouncedDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasReachedTime& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasTarget& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IdleAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ImpulseAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ImpulseTowardTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IsDeadDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IsGroundedDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IsTargetValidDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::JumpAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::JumpAttackAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MoveTowardTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::NavigateToTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PrepareShootingAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RaiseAgentOfDoomViewEvent& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReachedPointDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ResetMaxSpeedModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ResetTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ResetTimerAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SeekNewTarget& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetAttackInfoActiveAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetCompoundHFSMSchedulePeriodAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetCurrentMaxSpeedModifier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetGravityModifierAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetGroupTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetOwnerInBlackboardAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetPositionInBlackboardAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetShootPrecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetSpawnerPositionInBlackboardAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetSpawnerWanderRadiusInBlackboardAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SetupAoEAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShootBallAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShootTarget& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StartMeleeAttackAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StartShootingAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StopNavigationAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StopPathFinding& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StopShootingAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SweepAttackAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SwitchWeapon& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SwitchWeaponUltimate& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TimerDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ToggleAoEAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ToggleMovementBehaviourAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ToggleMovementPackSlicingAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ToggleTargetFinderAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TurnTowardTargetAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UpdateStrikeZonePositionAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UseGadget& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UseWrench& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Wander& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WithinRangeDecision& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionAND& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionAssetRefList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionBoolList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionByteList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionEntityRefList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionFPList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionFPVector2List& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionFPVector3List& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionIntegerList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionNOT& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIFunctionOR& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AimAtPassingTargetEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AirSaveEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ArithmeticFPFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AttackControlZoneEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AttackPlayerEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeAttackGetMaxDistanceFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeAttackInProgressFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeWrenchEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DashBlastBallEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionAssetRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionBool& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionByte& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionEntityRef& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionFP& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionFPVector2& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionFPVector3& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionInt& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefaultAIFunctionString& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DefendControlZoneEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityExistsFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityIsGroundedFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityRefsAreEqualFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EscapeShrinkingZoneEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FrameTimeHasElapsedFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetAttackInfoActiveFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetAttackInfoTargetFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetAttackInfoWasRunningFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetEntityPositionFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetFrameNumberFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetGroupEntityFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetGroupTargetFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetHighestPriorityTargetEntityFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetHighestPriorityTargetPositionFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetRandomFPFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetRecentPriorityTargetPositionFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetTargetFinderRangeNoWeaponFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GetVec2Element& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GroupTeamEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasGroupTargetFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasNavLineOfSightToTargetFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HasValidRecentPriorityTargetFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::InShootingZoneMovementEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::InputCheatFollowPlayer& out, ReaderTag);
void ReadFields(const Value& value, Quantum::KccIsMovingFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MeleeAttackIsActiveFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MinMaxFPFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MoveAwayFromTeammatesEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MoveToClosestEnemyEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MoveTowardsBallEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PassingEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PriorityTargetTypeCompareFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReachEnemyGoalEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReachShootingZoneEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReachStrikeZoneEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SeekCrateEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SeekDroppableEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SeekGoldenBoltEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShootBallEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShootEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StopToShootEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SwitchWeaponEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SwitchWeaponUltimateEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TakeDefensivePositionEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UseGadgetEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UseWrenchEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WanderEvaluator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PathfindStatusEqualsFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SweepAttackStateEqualsFunction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AmmunitionDamageSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AoeTrailSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AreaOfEffectIgnitionSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AreaOfEffectLevelUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AreaOfEffectSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BigBoomMineSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BlastBallSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BombardierDroneSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BombardierWeaponAutoTargetSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BuzzbladesRicochetSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CallbackAction& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CollisionForceSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CryosliderSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CryosliderWeaponAutoTargetSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageChargeableSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageMultiplierSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec::AoEMultiplier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec::WeaponMultiplier& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableOverride& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::CollectSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::DropSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::ShrinkingZoneSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ElementalDamageTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EnergyCellManagerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityDropperSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EntityGroupSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FireReactorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HealSourceSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::LifetimeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedAutoJumper& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedGrindRail& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedRailColliderData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedSpline& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedSplineSample& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BakedTravolator& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::BlastBallData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::ClosingPointData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::ClosingZoneData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::ControlZoneData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::SpawnPointData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::StaticColliderGrindRailPair& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::StaticDroppableData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MapCustomData::TutorialData& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MoveToSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ParabolaFollowerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PowerUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PrioritisedTargetHandlerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ProjectileSpawnerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ProjectileSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PusherSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RewardSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SeekNewTargetPriorityList& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShapeOverlapperSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SpeederSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StreamEffectSpawnerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StreamEffectSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TargetFinderSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TargetedEffectSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TrailSpawnerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Action_AddModifierMoveSpeed& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Action_KillSelf& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Action_MoveTo_StartMoving& out, ReaderTag);
void ReadFields(const Value& value, Quantum::Action_SpawnEntities& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BoltDropperSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EnergyCellDropperSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EnergyCellPowerUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ExtraDamagePowerUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::OverhealthPowerUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PowerUpDropperSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SpeedBoostPowerUpSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIMoveForwardBehaviourSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AINavigationSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorComponentSpecBase& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorManagerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorStatusModifierSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorStatusSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BoltGathererActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BoltPowerActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ChargeAttackSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ColliderResizeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageableSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DeathAnimationSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableCollectorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DroppableCollectorSpec::MaxCollectPerSecond& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HealthSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HeroSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::JumpAttackSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::KccExtensionConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::KccModifierSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::LedgeGrabbingSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::LevelUpActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MeleeAttackSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MeleeJumpAttackSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PreciseHitReceiverSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PushableSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RewardReceiverSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RollSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShieldedActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SweepAttackSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WallRunningSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorIcedSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorPhysicsSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorSheepedSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ActorStunnedSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AmoeboidLevelUpActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DashActorConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GrinderActorConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GroundDashActorConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HelipackActorSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateAbilityUserSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponLoadoutSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponUserSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GadgetSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationGadgetTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationGadgetTable::GadgetDefinition& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationMeleeWeaponsTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationMeleeWeaponsTable::MeleeWeaponDefinition& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationUltimateTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationUltimateTable::UltimateDefinition& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationWeaponTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationWeaponTable::Entry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateAbilitySpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponBehaviour_ChargingSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponSpec::MultiShotConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AddForceOnUltimateSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DrillDashGadgetSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GadgetWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GatlingWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GatlingWeaponSpec::GatlingLevel& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GloveOfDoomWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HoloshieldGloveWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HoverBootsWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::NegatronWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RynoWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TeslaClawWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateEvolutionSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateEvolutionSpec::EvolutionStat& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateSlamSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateTankSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateWeaponSpec::StatUpgrade& out, ReaderTag);
void ReadFields(const Value& value, Quantum::UltimateWeaponSpec::Upgrade& out, ReaderTag);
void ReadFields(const Value& value, Quantum::VoidRepulserWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponSpecDefault& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WrenchWeaponSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SmokeScreenSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CompoundHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageMultiplierHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DashCooldownReductionHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ElementalBuildUpMultiplierHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HeroEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::JumpAttackHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::JumpMultiplierHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::OnKillCallbacksHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::PlaceholderHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReplaceHealthSpecHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponStatsModifierHeroImprovementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AddDamageIncreaseOnTargetedEffectSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AddEntitiesToSpawnOnProjectileDeathSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AmmoMultiplierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AmoeboidLauncherEnhancementPuddleSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BlasterMoveSpeedEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BlasterSpecialShotEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CompoundEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::CooldownMultiplierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DamageIncreasedOnTargetsEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EnforcerFirstShotBonusEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FirerateMultiplierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GatlingEnhancementResetAmmoOnGadgetUseSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GatlingEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GrenadeLauncherAftershockEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GrenadeLauncherStunGrenadeEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HeadHunterWallHackEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HealthMultiplierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HoloshieldGloveHealZoneEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::HoverBootsEnhancementCollisionSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::IgnoreEntitiesMultiplierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MineLauncherEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MineLauncherProjectileOverrideEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RecoverAmmoOnDashEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ReplaceMultiShotConfigsEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShotgunBerserkerEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShotgunEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TeslaClawMoreBeamEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::VoidRepulserBulldozerEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::VoidRepulserSpeedBoostEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::VoidRepulserVoidWaveEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WarmongerMiniRocketsEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponChargingStatsModifierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponProjectileSwitcherEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponStatsModifierEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WrenchGroundSlamEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WrenchThrowEnhancementSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AIBotSDKConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AISpawnerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameManagerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModeHandlerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModeHandlerLevelConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModesTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GameModifiersSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::GoldenBoltConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MobGroupConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MobInfoSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationHeroesTable& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SimulationHeroesTable::Entry& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SquadSiegeConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SquadSiegeLevelConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveManagerSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveReferences& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveReferences::DroperReference& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WaveReferences::WaveMobReference& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BlastBallGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BoltKingGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EliminationGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::EliminationGameModeSpec::BotAggroSettings& out, ReaderTag);
void ReadFields(const Value& value, Quantum::FreeForAllGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::SquadSiegeGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TeamTotalRumbleGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TrainingGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TutorialGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TutorialGameModeSpec::StepBehaviour& out, ReaderTag);
void ReadFields(const Value& value, Quantum::WeaponShowcaseGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ZoneControlGameModeSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BumperBarConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::BumperConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ItemSpawnerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::LevelButtonConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::LevelButtonGameplayElementConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MovingWallConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ShrinkingZoneSpec& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StaticColliderConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::StaticTriggerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::ArenaCannonConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::AutoJumperStaticTriggerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DestroyElementConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::DestroyMobGroupGameplayElement& out, ReaderTag);
void ReadFields(const Value& value, Quantum::MovingWallGameplayElementConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::RailStaticTriggerConfig& out, ReaderTag);
void ReadFields(const Value& value, Quantum::TravolatorStaticTriggerConfig& out, ReaderTag);

void ReadFields(const Value& value, Quantum::AIBlackboardValueKey& out, ReaderTag) {
    Read(value, "Key", out.Key);
}

void ReadFields(const Value& value, Quantum::AIBlackboardEntry& out, ReaderTag) {
    Read(value, "Type", out.Type);
    Read(value, "Key", out.Key);
}

void ReadFields(const Value& value, Quantum::ActorPhysicsSurfaceSettings& out, ReaderTag) {
    Read(value, "surfaceType", out.surfaceType);
    Read(value, "physicsActorSpec", out.physicsActorSpec);
}

void ReadFields(const Value& value, Quantum::ActorStatusTypeSettings& out, ReaderTag) {
    Read(value, "type", out.type);
    Read(value, "forbiddenNextTypes", out.forbiddenNextTypes);
    Read(value, "componentSpecs", out.componentSpecs);
    Read(value, "prePhysicsPushModifier", out.prePhysicsPushModifier);
    Read(value, "physicsSurfaceSettings", out.physicsSurfaceSettings);
}

void ReadFields(const Value& value, Quantum::BTDataIndex& out, ReaderTag) {
    Read(value, "Index", out.Index);
}

void ReadFields(const Value& value, Quantum::DamageModifier& out, ReaderTag) {
    Read(value, "DamageSpecRef", out.DamageSpecRef);
    Read(value, "DamageModifierAmount", out.DamageModifierAmount);
}

void ReadFields(const Value& value, Quantum::ElementMultiplier& out, ReaderTag) {
    Read(value, "elementType", out.elementType);
    Read(value, "multiplier", out.multiplier);
}

void ReadFields(const Value& value, Quantum::DamageTypeToElementMapping& out, ReaderTag) {
    Read(value, "damageType", out.damageType);
    Read(value, "elementMultipliers", out.elementMultipliers);
}

void ReadFields(const Value& value, Quantum::DifficultyIntervalInt& out, ReaderTag) {
    Read(value, "atMinDiff", out.atMinDiff);
    Read(value, "atMaxDiff", out.atMaxDiff);
}

void ReadFields(const Value& value, Quantum::ElectricalDamageSettings& out, ReaderTag) {
    Read(value, "totalBuildUp", out.totalBuildUp);
    Read(value, "buildUpResetAfter", out.buildUpResetAfter);
    Read(value, "moveSpeedModifierWhenHit", out.moveSpeedModifierWhenHit);
    Read(value, "moveSpeedModifierWhenHitDuration", out.moveSpeedModifierWhenHitDuration);
    Read(value, "stunDurationSeconds", out.stunDurationSeconds);
}

void ReadFields(const Value& value, Quantum::EntityMultiplierPair& out, ReaderTag) {
    Read(value, "EntityInfoType", out.EntityInfoType);
    Read(value, "Multiplier", out.Multiplier);
}

void ReadFields(const Value& value, Quantum::GadgetSpecModifier& out, ReaderTag) {
    Read(value, "prop", out.prop);
    Read(value, "operation", out.operation);
    Read(value, "values", out.values);
}

void ReadFields(const Value& value, Quantum::GameModesTableEntry& out, ReaderTag) {
    Read(value, "id", out.id);
    Read(value, "prototype", out.prototype);
    Read(value, "spec", out.spec);
    Read(value, "status", out.status);
}

void ReadFields(const Value& value, Quantum::HFSMSetupData& out, ReaderTag) {
    Read(value, "hfsmRoot", out.hfsmRoot);
    Read(value, "aiConfig", out.aiConfig);
    Read(value, "blackboardInitializer", out.blackboardInitializer);
}

void ReadFields(const Value& value, Quantum::HFSMTransition& out, ReaderTag) {
    Read(value, "Id", out.Id);
    Read(value, "EventKey", out.EventKey);
    Read(value, "DecisionLink", out.DecisionLink);
    Read(value, "StateLink", out.StateLink);
    Read(value, "TransitionSetLink", out.TransitionSetLink);
}

void ReadFields(const Value& value, Quantum::HeightRangePercentPair& out, ReaderTag) {
    Read(value, "height", out.height);
    Read(value, "rangePercent", out.rangePercent);
}

void ReadFields(const Value& value, Quantum::IceDamageSettings& out, ReaderTag) {
    Read(value, "totalBuildUp", out.totalBuildUp);
    Read(value, "buildUpResetAfter", out.buildUpResetAfter);
    Read(value, "moveSpeedModifierWhenHit", out.moveSpeedModifierWhenHit);
    Read(value, "moveSpeedModifierWhenHitDuration", out.moveSpeedModifierWhenHitDuration);
    Read(value, "icedDurationSeconds", out.icedDurationSeconds);
}

void ReadFields(const Value& value, Quantum::MeleeComboAttackData& out, ReaderTag) {
    Read(value, "AmmunitionAsset", out.AmmunitionAsset);
    Read(value, "MaxTargetDistForAutoImpulse", out.MaxTargetDistForAutoImpulse);
    Read(value, "ImpulseDelay", out.ImpulseDelay);
    Read(value, "ImpulseStrength", out.ImpulseStrength);
    Read(value, "ImpulseDuration", out.ImpulseDuration);
    Read(value, "ForceInputDuration", out.ForceInputDuration);
    Read(value, "TimeToRemainAlignedWithDirection", out.TimeToRemainAlignedWithDirection);
    Read(value, "MaxSpeedModifier", out.MaxSpeedModifier);
    Read(value, "SpeedModifierDuration", out.SpeedModifierDuration);
    Read(value, "CooldownDuration", out.CooldownDuration);
    Read(value, "NextHitTimeframe", out.NextHitTimeframe);
    Read(value, "ImpulseDirectionSnapToTargetStrength", out.ImpulseDirectionSnapToTargetStrength);
}

void ReadFields(const Value& value, Quantum::PveScalingLevelEntry& out, ReaderTag) {
    Read(value, "PveScaling", out.PveScaling);
    Read(value, "Level", out.Level);
}

void ReadFields(const Value& value, Quantum::MobLevelMappingData& out, ReaderTag) {
    Read(value, "MobType", out.MobType);
    Read(value, "PveScalingLevelTable", out.PveScalingLevelTable);
}

void ReadFields(const Value& value, Quantum::MovingWallOffset& out, ReaderTag) {
    Read(value, "Offset", out.Offset);
    Read(value, "LockInPlaceOnArrival", out.LockInPlaceOnArrival);
    Read(value, "StopDuration", out.StopDuration);
}

void ReadFields(const Value& value, Quantum::RandomWeaponsModifierSpec& out, ReaderTag) {
    Read(value, "randomTimeInterval", out.randomTimeInterval);
}

void ReadFields(const Value& value, Quantum::ResponseCurvePack& out, ReaderTag) {
    Read(value, "ResponseCurveRef", out.ResponseCurveRef);
}

void ReadFields(const Value& value, Quantum::TrajectoryCurveData& out, ReaderTag) {
    Read(value, "Angle", out.Angle);
    Read(value, "Curve", out.Curve);
    Read(value, "Height", out.Height);
    Read(value, "RangePercent", out.RangePercent);
    Read(value, "HeightRangePercentPairs", out.HeightRangePercentPairs);
}

void ReadFields(const Value& value, Quantum::UltimateSpecModifier& out, ReaderTag) {
    Read(value, "prop", out.prop);
    Read(value, "operation", out.operation);
    Read(value, "values", out.values);
}

void ReadFields(const Value& value, Quantum::WaveSpawnRequest& out, ReaderTag) {
    Read(value, "MobID", out.MobID);
    Read(value, "SpawnTime", out.SpawnTime);
    Read(value, "Amount", out.Amount);
    Read(value, "SpawnerIndex", out.SpawnerIndex);
    Read(value, "DropperID", out.DropperID);
    Read(value, "DropperAmount", out.DropperAmount);
}

void ReadFields(const Value& value, Quantum::WaveData& out, ReaderTag) {
    Read(value, "WaveMaxDuration", out.WaveMaxDuration);
    Read(value, "SpawnRequestArray", out.SpawnRequestArray);
}

void ReadFields(const Value& value, Quantum::WeaponLoadoutData& out, ReaderTag) {
    Read(value, "first", out.first);
    Read(value, "gadget", out.gadget);
    Read(value, "melee", out.melee);
    Read(value, "ultimate", out.ultimate);
}

void ReadFields(const Value& value, Quantum::WeaponSpecModifier& out, ReaderTag) {
    Read(value, "prop", out.prop);
    Read(value, "operation", out.operation);
    Read(value, "values", out.values);
}

void ReadFields(const Value& value, Quantum::WeaponStatModifier& out, ReaderTag) {
    Read(value, "Property", out.Property);
    Read(value, "OperationType", out.OperationType);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::WeaponVisualParams& out, ReaderTag) {
    Read(value, "weaponID", out.weaponID);
    Read(value, "weaponSpawnPosition", out.weaponSpawnPosition);
    Read(value, "weaponPosition", out.weaponPosition);
    Read(value, "weaponEulerAngles", out.weaponEulerAngles);
    Read(value, "weaponShootEulerAngles", out.weaponShootEulerAngles);
    Read(value, "triggerPressedDuration", out.triggerPressedDuration);
    Read(value, "weaponLastRotationPosition", out.weaponLastRotationPosition);
    Read(value, "weaponLastRotationEulerAngles", out.weaponLastRotationEulerAngles);
}

void ReadFields(const Value& value, Quantum::AIParamAssetRef& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<AssetRef>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamBool& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<bool>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamEntityRef& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<EntityRef>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamFP& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<FP>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamFPVector2& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<FPVector2>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamFPVector3& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<FPVector3>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamInt& out, ReaderTag) {
    ReadFields(value, static_cast<AIParam<std::int32_t>&>(out), ReaderTag{});
    Read(value, "FunctionRef", out.FunctionRef);
}

void ReadFields(const Value& value, Quantum::AIParamGotoDestinationType& out, ReaderTag) {
    ReadFields(value, static_cast<EnumAIParam<GOTO_DESTINATION>&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AIParamMobSlamAttackEventType& out, ReaderTag) {
    ReadFields(value, static_cast<EnumAIParam<MobSlamAttackState>&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AIParamMobsEventType& out, ReaderTag) {
    ReadFields(value, static_cast<EnumAIParam<MobsEventType>&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AIParamPrioTargetType& out, ReaderTag) {
    ReadFields(value, static_cast<EnumAIParam<PrioTargetType>&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::NavigateToTargetAction_NavType& out, ReaderTag) {
    ReadFields(value, static_cast<EnumAIParam<AI_NAVIGATION_TYPE>&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AIAction& out, ReaderTag) {
    Read(value, "Label", out.Label);
}

void ReadFields(const Value& value, Quantum::AIBlackboard& out, ReaderTag) {
    Read(value, "Entries", out.Entries);
    Read(value, "InitializerRef", out.InitializerRef);
}

void ReadFields(const Value& value, Quantum::AIBlackboardInitializer& out, ReaderTag) {
    Read(value, "ReportMissingEntries", out.ReportMissingEntries);
    Read(value, "AIBlackboard", out.AIBlackboard);
    Read(value, "InitialValues", out.InitialValues);
}

void ReadFields(const Value& value, Quantum::AIBlackboardInitializer::AIBlackboardInitialValue& out, ReaderTag) {
    Read(value, "AsBoolean", out.AsBoolean);
    Read(value, "AsByte", out.AsByte);
    Read(value, "AsInteger", out.AsInteger);
    Read(value, "AsFP", out.AsFP);
    Read(value, "AsFPVector2", out.AsFPVector2);
    Read(value, "AsFPVector3", out.AsFPVector3);
    // AsEntityRef: entity references are runtime values the engine does not serialise.
    Read(value, "AsAssetRef", out.AsAssetRef);
}

void ReadFields(const Value& value, Quantum::AIBlackboardInitializer::AIBlackboardInitialValueEntry& out, ReaderTag) {
    Read(value, "Key", out.Key);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::AIConfig& out, ReaderTag) {
    Read(value, "DefaultConfig", out.DefaultConfig);
    Read(value, "KeyValuePairs", out.KeyValuePairs);
}

void ReadFields(const Value& value, Quantum::AIConfig::KeyValuePair& out, ReaderTag) {
    Read(value, "Key", out.Key);
    Read(value, "Type", out.Type);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::AIConfig::Value& out, ReaderTag) {
    Read(value, "Integer", out.Integer);
    Read(value, "Boolean", out.Boolean);
    Read(value, "Byte", out.Byte);
    Read(value, "FP", out.FP);
    Read(value, "FPVector2", out.FPVector2);
    Read(value, "FPVector3", out.FPVector3);
    Read(value, "String", out.String);
    // EntityRef: entity references are runtime values the engine does not serialise.
    Read(value, "AssetRef", out.AssetRef);
}

void ReadFields(const Value&, Quantum::AIFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::BTNode& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "Id", out.Id);
}

void ReadFields(const Value& value, Quantum::BTService& out, ReaderTag) {
    Read(value, "IntervalInSec", out.IntervalInSec);
    Read(value, "RunOnEnter", out.RunOnEnter);
    Read(value, "Id", out.Id);
}

void ReadFields(const Value& value, Quantum::CompoundHFSMSpec& out, ReaderTag) {
    Read(value, "navSetupData", out.navSetupData);
    Read(value, "attackSetupData", out.attackSetupData);
}

void ReadFields(const Value& value, Quantum::Consideration& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "RankRef", out.RankRef);
    Read(value, "CommitmentRef", out.CommitmentRef);
    Read(value, "NextConsiderationsRefs", out.NextConsiderationsRefs);
    Read(value, "OnEnterActionsRefs", out.OnEnterActionsRefs);
    Read(value, "OnUpdateActionsRefs", out.OnUpdateActionsRefs);
    Read(value, "OnExitActionsRefs", out.OnExitActionsRefs);
    Read(value, "ResponseCurvePacks", out.ResponseCurvePacks);
    Read(value, "BaseScore", out.BaseScore);
    Read(value, "MomentumData", out.MomentumData);
    Read(value, "UseNestedMomentum", out.UseNestedMomentum);
    Read(value, "Cooldown", out.Cooldown);
    Read(value, "CooldownCancelsMomentum", out.CooldownCancelsMomentum);
    Read(value, "Depth", out.Depth);
}

void ReadFields(const Value& value, Quantum::GOAPAction& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "Conditions", out.Conditions);
    Read(value, "Effects", out.Effects);
    Read(value, "Interruptible", out.Interruptible);
}

void ReadFields(const Value&, Quantum::GOAPBackValidation&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::GOAPGoal& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "StartState", out.StartState);
    Read(value, "TargetState", out.TargetState);
    Read(value, "InterruptionBehavior", out.InterruptionBehavior);
}

void ReadFields(const Value&, Quantum::GOAPHeuristic&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::GOAPRoot& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "InterruptionCheckInterval", out.InterruptionCheckInterval);
    Read(value, "GoalRefs", out.GoalRefs);
    Read(value, "ActionRefs", out.ActionRefs);
}

void ReadFields(const Value& value, Quantum::HFSMDecision& out, ReaderTag) {
    Read(value, "Label", out.Label);
}

void ReadFields(const Value& value, Quantum::HFSMRoot& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "StatesLinks", out.StatesLinks);
    Read(value, "EventsNames", out.EventsNames);
}

void ReadFields(const Value& value, Quantum::HFSMState& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "Index", out.Index);
    Read(value, "OnUpdateLinks", out.OnUpdateLinks);
    Read(value, "OnEnterLinks", out.OnEnterLinks);
    Read(value, "OnExitLinks", out.OnExitLinks);
    Read(value, "Transitions", out.Transitions);
    Read(value, "ChildrenLinks", out.ChildrenLinks);
    Read(value, "ParentLink", out.ParentLink);
    Read(value, "Level", out.Level);
}

void ReadFields(const Value& value, Quantum::HFSMTransitionSet& out, ReaderTag) {
    Read(value, "Label", out.Label);
    Read(value, "PrerequisiteLink", out.PrerequisiteLink);
    Read(value, "Transitions", out.Transitions);
}

void ReadFields(const Value& value, Quantum::UTRoot& out, ReaderTag) {
    Read(value, "ConsiderationsRefs", out.ConsiderationsRefs);
}

void ReadFields(const Value& value, Quantum::BTComposite& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTNode&>(out), ReaderTag{});
    Read(value, "Children", out.Children);
    Read(value, "Services", out.Services);
    Read(value, "TopmostDecorator", out.TopmostDecorator);
    Read(value, "CurrentChildIndex", out.CurrentChildIndex);
    Read(value, "IsDynamic", out.IsDynamic);
}

void ReadFields(const Value& value, Quantum::BTDecorator& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTNode&>(out), ReaderTag{});
    Read(value, "Child", out.Child);
    Read(value, "AbortType", out.AbortType);
}

void ReadFields(const Value& value, Quantum::BTLeaf& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTNode&>(out), ReaderTag{});
    Read(value, "Services", out.Services);
}

void ReadFields(const Value& value, Quantum::CheckBlackboardInt& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "Comparison", out.Comparison);
    Read(value, "DesiredValue", out.DesiredValue);
}

void ReadFields(const Value& value, Quantum::DebugAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Message", out.Message);
}

void ReadFields(const Value& value, Quantum::DebugService& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTService&>(out), ReaderTag{});
    Read(value, "Message", out.Message);
}

void ReadFields(const Value& value, Quantum::GOAPDefaultAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GOAPAction&>(out), ReaderTag{});
    Read(value, "Validation", out.Validation);
    Read(value, "Cost", out.Cost);
    Read(value, "PlanStateValidationLink", out.PlanStateValidationLink);
    Read(value, "OnActivateLinks", out.OnActivateLinks);
    Read(value, "OnUpdateLinks", out.OnUpdateLinks);
    Read(value, "OnDeactivateLinks", out.OnDeactivateLinks);
    Read(value, "IsDone", out.IsDone);
    Read(value, "IsFailed", out.IsFailed);
}

void ReadFields(const Value& value, Quantum::GOAPDefaultGoal& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GOAPGoal&>(out), ReaderTag{});
    Read(value, "Validation", out.Validation);
    Read(value, "Relevancy", out.Relevancy);
    Read(value, "DisableTime", out.DisableTime);
    Read(value, "OnInitPlanningLinks", out.OnInitPlanningLinks);
    Read(value, "OnActivateLinks", out.OnActivateLinks);
    Read(value, "OnDeactivateLinks", out.OnDeactivateLinks);
    Read(value, "IsFinished", out.IsFinished);
}

void ReadFields(const Value& value, Quantum::GOAPSingleBackValidation& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GOAPBackValidation&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::HFSMLogicalDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "Decisions", out.Decisions);
}

void ReadFields(const Value& value, Quantum::IncreaseBlackboardInt& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "IncrementAmount", out.IncrementAmount);
}

void ReadFields(const Value& value, Quantum::IsTrue& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::SetBlackboardBool& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "value", out.value);
}

void ReadFields(const Value& value, Quantum::SetBlackboardEntityRef& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::SetBlackboardFP& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::SetBlackboardFPVector3& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::SetBlackboardInt& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Key", out.Key);
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::TrueDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::BTBlackboardCompare& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTDecorator&>(out), ReaderTag{});
    Read(value, "BlackboardKeyA", out.BlackboardKeyA);
    Read(value, "BlackboardKeyB", out.BlackboardKeyB);
}

void ReadFields(const Value& value, Quantum::BTCooldown& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTDecorator&>(out), ReaderTag{});
    Read(value, "CooldownTime", out.CooldownTime);
    Read(value, "StartTimeIndex", out.StartTimeIndex);
}

void ReadFields(const Value& value, Quantum::BTForceResult& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTDecorator&>(out), ReaderTag{});
    Read(value, "Result", out.Result);
}

void ReadFields(const Value& value, Quantum::BTLoop& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTDecorator&>(out), ReaderTag{});
    Read(value, "LoopIterations", out.LoopIterations);
    Read(value, "LoopForever", out.LoopForever);
    Read(value, "LoopTimeout", out.LoopTimeout);
    Read(value, "StartTimeIndex", out.StartTimeIndex);
    Read(value, "IterationCountIndex", out.IterationCountIndex);
}

void ReadFields(const Value& value, Quantum::BTRoot& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTDecorator&>(out), ReaderTag{});
    Read(value, "NodesAmount", out.NodesAmount);
}

void ReadFields(const Value& value, Quantum::BTSelector& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTComposite&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::BTSelectorRandom& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTComposite&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::BTSequence& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTComposite&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::DebugLeaf& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTLeaf&>(out), ReaderTag{});
    Read(value, "Message", out.Message);
}

void ReadFields(const Value& value, Quantum::HFSMAndDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMLogicalDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::HFSMNotDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMLogicalDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::HFSMOrDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMLogicalDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ResponseCurve& out, ReaderTag) {
    Read(value, "Input", out.Input);
    Read(value, "Curve", out.Curve);
    Read(value, "Clamp01", out.Clamp01);
    Read(value, "MultiplyFactor", out.MultiplyFactor);
}

void ReadFields(const Value& value, Quantum::WaitLeaf& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::BTLeaf&>(out), ReaderTag{});
    Read(value, "Duration", out.Duration);
    Read(value, "EndTimeIndex", out.EndTimeIndex);
}

void ReadFields(const Value& value, Quantum::AddTargetToBlacklistAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "TargetEntity", out.TargetEntity);
}

void ReadFields(const Value& value, Quantum::ChargeAttackStartAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ChargeWrenchAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "minChargeTime", out.minChargeTime);
    Read(value, "maxChargeTime", out.maxChargeTime);
}

void ReadFields(const Value& value, Quantum::ChoosePatrolDestinationAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "PatrolCenter", out.PatrolCenter);
    Read(value, "PatrolRadius", out.PatrolRadius);
    Read(value, "PatrolDestinationKey", out.PatrolDestinationKey);
    Read(value, "PatrolDestinationFoundKey", out.PatrolDestinationFoundKey);
}

void ReadFields(const Value& value, Quantum::DashRoboBall& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::DestroyAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::DispatchMobSlamAttacksEventAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "EventType", out.EventType);
}

void ReadFields(const Value& value, Quantum::DispatchMobsEventAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "EventType", out.EventType);
}

void ReadFields(const Value& value, Quantum::DispatchShooterEventAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "EventType", out.EventType);
}

void ReadFields(const Value& value, Quantum::DispatchSpinnerEventAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "EventType", out.EventType);
}

void ReadFields(const Value& value, Quantum::DistanceFromPositionDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "StartPosition", out.StartPosition);
    Read(value, "EndPosition", out.EndPosition);
    Read(value, "DistanceToCheck", out.DistanceToCheck);
    Read(value, "Comparison", out.Comparison);
}

void ReadFields(const Value& value, Quantum::FindTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Target", out.Target);
    Read(value, "UseTargetFinder", out.UseTargetFinder);
    Read(value, "TargetFinderRangeOverride", out.TargetFinderRangeOverride);
    Read(value, "UsePushedBy", out.UsePushedBy);
    Read(value, "UseHitBy", out.UseHitBy);
}

void ReadFields(const Value& value, Quantum::GameIsStarted& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::GoTo& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "destinationType", out.destinationType);
}

void ReadFields(const Value& value, Quantum::GoToControlZone& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "RepathMinInterval", out.RepathMinInterval);
    Read(value, "RepathMaxInterval", out.RepathMaxInterval);
}

void ReadFields(const Value& value, Quantum::GoToPlayerDebugPos& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::GroundDashAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::HasLineOfSightToTargetDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TargetKey", out.TargetKey);
}

void ReadFields(const Value& value, Quantum::HasMoveBehaviourBouncedDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::HasReachedTime& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TimerKey", out.TimerKey);
    Read(value, "TargetTime", out.TargetTime);
}

void ReadFields(const Value& value, Quantum::HasTarget& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TargetKey", out.TargetKey);
}

void ReadFields(const Value& value, Quantum::IdleAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ImpulseAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "ImpulseForceParam", out.ImpulseForceParam);
    Read(value, "ImpulseForwardOffsetParam", out.ImpulseForwardOffsetParam);
    Read(value, "DurationParam", out.DurationParam);
    Read(value, "InputFalloffParam", out.InputFalloffParam);
}

void ReadFields(const Value& value, Quantum::ImpulseTowardTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "ImpulseForceParam", out.ImpulseForceParam);
    Read(value, "ImpulseForwardOffsetParam", out.ImpulseForwardOffsetParam);
    Read(value, "DurationParam", out.DurationParam);
    Read(value, "InputFalloffParam", out.InputFalloffParam);
    Read(value, "InputDurationParam", out.InputDurationParam);
    Read(value, "TargetEntityKey", out.TargetEntityKey);
}

void ReadFields(const Value& value, Quantum::IsDeadDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::IsGroundedDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::IsTargetValidDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "DetectionRadius", out.DetectionRadius);
    Read(value, "TargetValueKey", out.TargetValueKey);
}

void ReadFields(const Value& value, Quantum::JumpAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "ImpulseForce", out.ImpulseForce);
}

void ReadFields(const Value& value, Quantum::JumpAttackAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "TargetPosition", out.TargetPosition);
}

void ReadFields(const Value& value, Quantum::MoveTowardTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Target", out.Target);
    Read(value, "CloseDistance", out.CloseDistance);
    Read(value, "ResetActorInputDirectionWhenClose", out.ResetActorInputDirectionWhenClose);
    Read(value, "PreserveForceLookInput", out.PreserveForceLookInput);
}

void ReadFields(const Value& value, Quantum::NavigateToTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "NavType", out.NavType);
    Read(value, "TickFrequency", out.TickFrequency);
    Read(value, "TargetEntity", out.TargetEntity);
    Read(value, "TargetPosition", out.TargetPosition);
    Read(value, "NearbyTargetDistance", out.NearbyTargetDistance);
    Read(value, "TargetKey", out.TargetKey);
    Read(value, "TargetPositionKey", out.TargetPositionKey);
    Read(value, "PathfindStatusKey", out.PathfindStatusKey);
    Read(value, "MaintainInputDirectionWhenRepathing", out.MaintainInputDirectionWhenRepathing);
}

void ReadFields(const Value& value, Quantum::PrepareShootingAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::RaiseAgentOfDoomViewEvent& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "EventType", out.EventType);
}

void ReadFields(const Value& value, Quantum::ReachedPointDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TargetPositionKey", out.TargetPositionKey);
    Read(value, "TargetPosition", out.TargetPosition);
    Read(value, "Tolerance", out.Tolerance);
}

void ReadFields(const Value& value, Quantum::ResetMaxSpeedModifier& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ResetTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Target", out.Target);
}

void ReadFields(const Value& value, Quantum::ResetTimerAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "TimerKey", out.TimerKey);
}

void ReadFields(const Value& value, Quantum::SeekNewTarget& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "DetectionRadius", out.DetectionRadius);
    Read(value, "DetectionFrequency", out.DetectionFrequency);
    Read(value, "TargetKey", out.TargetKey);
    Read(value, "TargetPriorityList", out.TargetPriorityList);
}

void ReadFields(const Value& value, Quantum::SetAttackInfoActiveAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Active", out.Active);
    Read(value, "TargetEntity", out.TargetEntity);
    Read(value, "WasRunningBeforeAttack", out.WasRunningBeforeAttack);
}

void ReadFields(const Value& value, Quantum::SetCompoundHFSMSchedulePeriodAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "mode", out.mode);
}

void ReadFields(const Value& value, Quantum::SetCurrentMaxSpeedModifier& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "MaxSpeedModifier", out.MaxSpeedModifier);
}

void ReadFields(const Value& value, Quantum::SetGravityModifierAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Value", out.Value);
    Read(value, "Duration", out.Duration);
}

void ReadFields(const Value& value, Quantum::SetGroupTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "TargetEntity", out.TargetEntity);
}

void ReadFields(const Value& value, Quantum::SetOwnerInBlackboardAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "OwnerKey", out.OwnerKey);
}

void ReadFields(const Value& value, Quantum::SetPositionInBlackboardAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Condition", out.Condition);
    Read(value, "Position", out.Position);
    Read(value, "TransformEntity", out.TransformEntity);
}

void ReadFields(const Value& value, Quantum::SetShootPrecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::SetSpawnerPositionInBlackboardAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "SpawnerPosition", out.SpawnerPosition);
}

void ReadFields(const Value& value, Quantum::SetSpawnerWanderRadiusInBlackboardAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "SpawnerWanderRadius", out.SpawnerWanderRadius);
}

void ReadFields(const Value& value, Quantum::SetupAoEAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Enabled", out.Enabled);
    Read(value, "Range", out.Range);
}

void ReadFields(const Value& value, Quantum::ShootBallAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ShootTarget& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::StartMeleeAttackAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Delay", out.Delay);
}

void ReadFields(const Value& value, Quantum::StartShootingAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "CustomShootTarget", out.CustomShootTarget);
}

void ReadFields(const Value& value, Quantum::StopNavigationAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::StopPathFinding& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::StopShootingAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::SweepAttackAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::SwitchWeapon& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "minCooldown_Seconds", out.minCooldown_Seconds);
    Read(value, "maxCooldown_Seconds", out.maxCooldown_Seconds);
}

void ReadFields(const Value& value, Quantum::SwitchWeaponUltimate& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::TimerDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TimeToTrueState", out.TimeToTrueState);
}

void ReadFields(const Value& value, Quantum::ToggleAoEAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Enabled", out.Enabled);
}

void ReadFields(const Value& value, Quantum::ToggleMovementBehaviourAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Value", out.Value);
    Read(value, "ConstrainToSpawner", out.ConstrainToSpawner);
}

void ReadFields(const Value& value, Quantum::ToggleMovementPackSlicingAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::ToggleTargetFinderAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::TurnTowardTargetAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "Target", out.Target);
    Read(value, "IsActive", out.IsActive);
    Read(value, "IsInstant", out.IsInstant);
    Read(value, "PreserveForceLookInput", out.PreserveForceLookInput);
}

void ReadFields(const Value& value, Quantum::UpdateStrikeZonePositionAction& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "PreviousBallPosition", out.PreviousBallPosition);
    Read(value, "StrikeZonePosition", out.StrikeZonePosition);
    Read(value, "OwnGoalDistanceThreshold", out.OwnGoalDistanceThreshold);
    Read(value, "EnemyGoalDistanceThreshold", out.EnemyGoalDistanceThreshold);
}

void ReadFields(const Value& value, Quantum::UseGadget& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "minCooldown", out.minCooldown);
    Read(value, "maxCooldown", out.maxCooldown);
}

void ReadFields(const Value& value, Quantum::UseWrench& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "useDelay", out.useDelay);
}

void ReadFields(const Value& value, Quantum::Wander& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AIAction&>(out), ReaderTag{});
    Read(value, "minWanderDist", out.minWanderDist);
    Read(value, "maxWanderDist", out.maxWanderDist);
    Read(value, "minPauseDelayOnDestinationReached", out.minPauseDelayOnDestinationReached);
    Read(value, "maxPauseDelayOnDestinationReached", out.maxPauseDelayOnDestinationReached);
}

void ReadFields(const Value& value, Quantum::WithinRangeDecision& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HFSMDecision&>(out), ReaderTag{});
    Read(value, "TargetKey", out.TargetKey);
    Read(value, "Target", out.Target);
    Read(value, "AttackRangeParam", out.AttackRangeParam);
    Read(value, "UseYTolerance", out.UseYTolerance);
    Read(value, "YTolerance", out.YTolerance);
}

void ReadFields(const Value& value, Quantum::AIFunctionAND& out, ReaderTag) {
    Read(value, "ValueA", out.ValueA);
    Read(value, "ValueB", out.ValueB);
}

void ReadFields(const Value&, Quantum::AIFunctionAssetRefList&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionBoolList&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionByteList&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionEntityRefList&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionFPList&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionFPVector2List&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionFPVector3List&, ReaderTag) {}

void ReadFields(const Value&, Quantum::AIFunctionIntegerList&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::AIFunctionNOT& out, ReaderTag) {
    Read(value, "Value", out.Value);
}

void ReadFields(const Value& value, Quantum::AIFunctionOR& out, ReaderTag) {
    Read(value, "ValueA", out.ValueA);
    Read(value, "ValueB", out.ValueB);
}

void ReadFields(const Value& value, Quantum::AimAtPassingTargetEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::AirSaveEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "dashDelayMin_Milliseconds", out.dashDelayMin_Milliseconds);
    Read(value, "dashDelayMax_Milliseconds", out.dashDelayMax_Milliseconds);
    Read(value, "minAffinityCooldown_Seconds", out.minAffinityCooldown_Seconds);
    Read(value, "maxAffinityCooldown_Seconds", out.maxAffinityCooldown_Seconds);
}

void ReadFields(const Value& value, Quantum::ArithmeticFPFunction& out, ReaderTag) {
    Read(value, "operation", out.operation);
    Read(value, "first", out.first);
    Read(value, "second", out.second);
}

void ReadFields(const Value& value, Quantum::AttackControlZoneEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "contestingUtility_0_100", out.contestingUtility_0_100);
}

void ReadFields(const Value& value, Quantum::AttackPlayerEvaluator& out, ReaderTag) {
    Read(value, "utilityDefaultTarget", out.utilityDefaultTarget);
    Read(value, "utilityPlayerTarget", out.utilityPlayerTarget);
    Read(value, "utilityNPCTarget", out.utilityNPCTarget);
    Read(value, "realPlayerSearchRadius", out.realPlayerSearchRadius);
    Read(value, "durationMinSeconds", out.durationMinSeconds);
    Read(value, "durationMaxSeconds", out.durationMaxSeconds);
    Read(value, "coolDownMinSeconds", out.coolDownMinSeconds);
    Read(value, "coolDownMaxSeconds", out.coolDownMaxSeconds);
}

void ReadFields(const Value&, Quantum::ChargeAttackGetMaxDistanceFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::ChargeAttackInProgressFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::ChargeWrenchEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "startChargingHealthThreshold", out.startChargingHealthThreshold);
    Read(value, "releaseChargeHealthThreshold", out.releaseChargeHealthThreshold);
    Read(value, "minShootDistance", out.minShootDistance);
    Read(value, "maxShootDistance", out.maxShootDistance);
}

void ReadFields(const Value& value, Quantum::DashBlastBallEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "maxDistanceToUse", out.maxDistanceToUse);
    Read(value, "minCooldown_Seconds", out.minCooldown_Seconds);
    Read(value, "maxCooldown_Seconds", out.maxCooldown_Seconds);
}

void ReadFields(const Value&, Quantum::DefaultAIFunctionAssetRef&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionBool&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionByte&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionEntityRef&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionFP&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionFPVector2&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionFPVector3&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionInt&, ReaderTag) {}

void ReadFields(const Value&, Quantum::DefaultAIFunctionString&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::DefendControlZoneEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::EntityExistsFunction& out, ReaderTag) {
    Read(value, "entityToTest", out.entityToTest);
}

void ReadFields(const Value& value, Quantum::EntityIsGroundedFunction& out, ReaderTag) {
    Read(value, "entityToTest", out.entityToTest);
}

void ReadFields(const Value& value, Quantum::EntityRefsAreEqualFunction& out, ReaderTag) {
    Read(value, "First", out.First);
    Read(value, "Second", out.Second);
}

void ReadFields(const Value& value, Quantum::EscapeShrinkingZoneEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "distanceToZoneBorder", out.distanceToZoneBorder);
}

void ReadFields(const Value& value, Quantum::FrameTimeHasElapsedFunction& out, ReaderTag) {
    Read(value, "StartFrame", out.StartFrame);
    Read(value, "Duration", out.Duration);
}

void ReadFields(const Value&, Quantum::GetAttackInfoActiveFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetAttackInfoTargetFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetAttackInfoWasRunningFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::GetEntityPositionFunction& out, ReaderTag) {
    Read(value, "entityToTest", out.entityToTest);
}

void ReadFields(const Value&, Quantum::GetFrameNumberFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetGroupEntityFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetGroupTargetFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetHighestPriorityTargetEntityFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::GetHighestPriorityTargetPositionFunction& out, ReaderTag) {
    Read(value, "UseIntermediateWaypoint", out.UseIntermediateWaypoint);
}

void ReadFields(const Value& value, Quantum::GetRandomFPFunction& out, ReaderTag) {
    Read(value, "Min", out.Min);
    Read(value, "Max", out.Max);
}

void ReadFields(const Value&, Quantum::GetRecentPriorityTargetPositionFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::GetTargetFinderRangeNoWeaponFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::GetVec2Element& out, ReaderTag) {
    Read(value, "Input", out.Input);
    Read(value, "Elem", out.Elem);
}

void ReadFields(const Value& value, Quantum::GroupTeamEvaluator& out, ReaderTag) {
    Read(value, "utilityCloseParam", out.utilityCloseParam);
    Read(value, "utilityFarParam", out.utilityFarParam);
    Read(value, "utilityHelpLeaderParam", out.utilityHelpLeaderParam);
    Read(value, "closeDistanceToTargetParam", out.closeDistanceToTargetParam);
    Read(value, "farDistanceToTargetParam", out.farDistanceToTargetParam);
}

void ReadFields(const Value&, Quantum::HasGroupTargetFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::HasNavLineOfSightToTargetFunction& out, ReaderTag) {
    Read(value, "targetEntityRef", out.targetEntityRef);
}

void ReadFields(const Value&, Quantum::HasValidRecentPriorityTargetFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::InShootingZoneMovementEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "minWeaponRangeMultiplier", out.minWeaponRangeMultiplier);
    Read(value, "maxWeaponRangeMultiplier", out.maxWeaponRangeMultiplier);
    Read(value, "minCooldownTimer", out.minCooldownTimer);
    Read(value, "maxCooldownTimer", out.maxCooldownTimer);
}

void ReadFields(const Value&, Quantum::InputCheatFollowPlayer&, ReaderTag) {}

void ReadFields(const Value&, Quantum::KccIsMovingFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::MeleeAttackIsActiveFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::MinMaxFPFunction& out, ReaderTag) {
    Read(value, "mode", out.mode);
    Read(value, "first", out.first);
    Read(value, "second", out.second);
}

void ReadFields(const Value& value, Quantum::MoveAwayFromTeammatesEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "distanceThreshold", out.distanceThreshold);
}

void ReadFields(const Value& value, Quantum::MoveToClosestEnemyEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::MoveTowardsBallEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::PassingEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::PriorityTargetTypeCompareFunction& out, ReaderTag) {
    Read(value, "Value", out.Value);
    Read(value, "Mode", out.Mode);
}

void ReadFields(const Value& value, Quantum::ReachEnemyGoalEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::ReachShootingZoneEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "useWeaponRangeAsMaxDistance", out.useWeaponRangeAsMaxDistance);
    Read(value, "weaponRangeMultiplier", out.weaponRangeMultiplier);
    Read(value, "positionRefreshRate", out.positionRefreshRate);
}

void ReadFields(const Value& value, Quantum::ReachStrikeZoneEvaluator& out, ReaderTag) {
    Read(value, "StrikeZonePosition", out.StrikeZonePosition);
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::SeekCrateEvaluator& out, ReaderTag) {
    Read(value, "closeCrateDistance", out.closeCrateDistance);
    Read(value, "utility_farCrate_0_100", out.utility_farCrate_0_100);
    Read(value, "utility_closeCrate_0_100", out.utility_closeCrate_0_100);
}

void ReadFields(const Value& value, Quantum::SeekDroppableEvaluator& out, ReaderTag) {
    Read(value, "utility", out.utility);
}

void ReadFields(const Value& value, Quantum::SeekGoldenBoltEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "utilityWhenClose_0_100", out.utilityWhenClose_0_100);
    Read(value, "closeDistanceMeters", out.closeDistanceMeters);
    Read(value, "fov", out.fov);
}

void ReadFields(const Value& value, Quantum::ShootBallEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value& value, Quantum::ShootEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "maxAngleInDegree", out.maxAngleInDegree);
    Read(value, "blastBallDistanceThreshold", out.blastBallDistanceThreshold);
}

void ReadFields(const Value&, Quantum::StopToShootEvaluator&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::SwitchWeaponEvaluator& out, ReaderTag) {
    Read(value, "noCombatActivityDelayMin_Seconds", out.noCombatActivityDelayMin_Seconds);
    Read(value, "noCombatActivityDelayMax_Seconds", out.noCombatActivityDelayMax_Seconds);
}

void ReadFields(const Value&, Quantum::SwitchWeaponUltimateEvaluator&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::TakeDefensivePositionEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "minForward", out.minForward);
    Read(value, "maxForward", out.maxForward);
    Read(value, "maxRight", out.maxRight);
    Read(value, "refreshRate", out.refreshRate);
}

void ReadFields(const Value& value, Quantum::UseGadgetEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "targetTypes", out.targetTypes);
    Read(value, "minInitialCooldown", out.minInitialCooldown);
    Read(value, "maxInitialCooldown", out.maxInitialCooldown);
}

void ReadFields(const Value& value, Quantum::UseWrenchEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
    Read(value, "maxAngleInDegree", out.maxAngleInDegree);
    Read(value, "maxDistanceToUseWrench", out.maxDistanceToUseWrench);
    Read(value, "minCooldown_Seconds", out.minCooldown_Seconds);
    Read(value, "maxCooldown_Seconds", out.maxCooldown_Seconds);
}

void ReadFields(const Value& value, Quantum::WanderEvaluator& out, ReaderTag) {
    Read(value, "defaultUtility_0_100", out.defaultUtility_0_100);
}

void ReadFields(const Value&, Quantum::PathfindStatusEqualsFunction&, ReaderTag) {}

void ReadFields(const Value&, Quantum::SweepAttackStateEqualsFunction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::AmmunitionDamageSpec& out, ReaderTag) {
    Read(value, "UseRangeToDamageMultCurve", out.UseRangeToDamageMultCurve);
    Read(value, "RangeToDamageMultCurve", out.RangeToDamageMultCurve);
}

void ReadFields(const Value& value, Quantum::AoeTrailSpec& out, ReaderTag) {
    Read(value, "AoeTrailSegmentMinimalLength", out.AoeTrailSegmentMinimalLength);
    Read(value, "AoeTrailLifetime", out.AoeTrailLifetime);
    Read(value, "localSpawnOffset", out.localSpawnOffset);
    Read(value, "OnlyGenerateOnGround", out.OnlyGenerateOnGround);
    Read(value, "MaxGroundDistance", out.MaxGroundDistance);
    Read(value, "GroundMask", out.GroundMask);
    Read(value, "GroundYOffset", out.GroundYOffset);
    Read(value, "GeneratePointOnCollision", out.GeneratePointOnCollision);
    Read(value, "OnlyVerticalCollision", out.OnlyVerticalCollision);
    Read(value, "GeneratePointAboveAngleEnabled", out.GeneratePointAboveAngleEnabled);
    Read(value, "GeneratePointAboveAngle", out.GeneratePointAboveAngle);
}

void ReadFields(const Value& value, Quantum::AreaOfEffectIgnitionSpec& out, ReaderTag) {
    Read(value, "IgnitionAoeEntityPrototype", out.IgnitionAoeEntityPrototype);
}

void ReadFields(const Value& value, Quantum::AreaOfEffectLevelUpSpec& out, ReaderTag) {
    Read(value, "EntityPrototypeRef", out.EntityPrototypeRef);
    Read(value, "MaxLevelUpOccurrences", out.MaxLevelUpOccurrences);
}

void ReadFields(const Value& value, Quantum::AreaOfEffectSpec& out, ReaderTag) {
    Read(value, "ShapeConfig", out.ShapeConfig);
    Read(value, "StartsEnabled", out.StartsEnabled);
    Read(value, "StartDelay", out.StartDelay);
    Read(value, "IsContinuous", out.IsContinuous);
    Read(value, "IgnoreAlreadyEffectedEntities", out.IgnoreAlreadyEffectedEntities);
    Read(value, "IgnoreEffectedEntitiesDuration", out.IgnoreEffectedEntitiesDuration);
    Read(value, "AutoRemoveEffectedEntityIfOutOfArea", out.AutoRemoveEffectedEntityIfOutOfArea);
    Read(value, "InitialScale", out.InitialScale);
    Read(value, "IsInfinite", out.IsInfinite);
    Read(value, "Duration", out.Duration);
    Read(value, "GrowsOverDuration", out.GrowsOverDuration);
    Read(value, "MinGrowthRange", out.MinGrowthRange);
    Read(value, "MaxGrowthRange", out.MaxGrowthRange);
    Read(value, "CanAffectEnemies", out.CanAffectEnemies);
    Read(value, "CanAffectTeam", out.CanAffectTeam);
    Read(value, "CanAffectInstigator", out.CanAffectInstigator);
    Read(value, "NeedsLineOfSightCheck", out.NeedsLineOfSightCheck);
    Read(value, "LineOfSightQueryOptions", out.LineOfSightQueryOptions);
    Read(value, "LimitScaleToOne", out.LimitScaleToOne);
    Read(value, "ScaleRequiresRaycast", out.ScaleRequiresRaycast);
    Read(value, "ScaleRaycastLayerMask", out.ScaleRaycastLayerMask);
    Read(value, "ConstrainToForwardAngle", out.ConstrainToForwardAngle);
    Read(value, "ForwardAngleConstraintDegrees", out.ForwardAngleConstraintDegrees);
    Read(value, "ForwardAngleConstraintIs2D", out.ForwardAngleConstraintIs2D);
    Read(value, "ConstrainToHeightOffset", out.ConstrainToHeightOffset);
    Read(value, "MaxHeightDifference", out.MaxHeightDifference);
    Read(value, "ConstrainToInnerRadiusOffset", out.ConstrainToInnerRadiusOffset);
    Read(value, "InnerRadiusOffset", out.InnerRadiusOffset);
    Read(value, "ProjectileForceDirectionIsFlat", out.ProjectileForceDirectionIsFlat);
    Read(value, "DirectionType", out.DirectionType);
    Read(value, "LateralForceCoef", out.LateralForceCoef);
    Read(value, "AlongAxisDirection", out.AlongAxisDirection);
    Read(value, "AwayFromShooterMaxRange", out.AwayFromShooterMaxRange);
    Read(value, "NotifyViewOnHit", out.NotifyViewOnHit);
    Read(value, "NotifyViewOnMiss", out.NotifyViewOnMiss);
    Read(value, "NotifyInstigatorOnHit", out.NotifyInstigatorOnHit);
    Read(value, "SetupEntityPhysicsCollider3D", out.SetupEntityPhysicsCollider3D);
}

void ReadFields(const Value& value, Quantum::BigBoomMineSpec& out, ReaderTag) {
    Read(value, "CollisionLayerMask", out.CollisionLayerMask);
    Read(value, "FallingSpeed", out.FallingSpeed);
    Read(value, "ExplosionDelay", out.ExplosionDelay);
}

void ReadFields(const Value& value, Quantum::BlastBallSpec& out, ReaderTag) {
    Read(value, "maxSpeed", out.maxSpeed);
    Read(value, "meleeWeaponImpulseMultiplier", out.meleeWeaponImpulseMultiplier);
    Read(value, "dragWhenShot", out.dragWhenShot);
    Read(value, "dragRecoveryDelay", out.dragRecoveryDelay);
    Read(value, "dragRecoveryDuration", out.dragRecoveryDuration);
    Read(value, "constantDashDamageSpec", out.constantDashDamageSpec);
    Read(value, "speedModifierOnDashEnded", out.speedModifierOnDashEnded);
    Read(value, "speedModifierOnDashEndedDuration", out.speedModifierOnDashEndedDuration);
    Read(value, "dashInfluenceDistance", out.dashInfluenceDistance);
    Read(value, "meleeCorrection", out.meleeCorrection);
    Read(value, "meleeHitRotationLockTime", out.meleeHitRotationLockTime);
    Read(value, "settledSpeedThreshold", out.settledSpeedThreshold);
    Read(value, "bouncyPhysicsMaterial", out.bouncyPhysicsMaterial);
    Read(value, "settledPhysicsMaterial", out.settledPhysicsMaterial);
    Read(value, "wallBounceForce", out.wallBounceForce);
}

void ReadFields(const Value& value, Quantum::BombardierDroneSpec& out, ReaderTag) {
    Read(value, "MoveSpeed", out.MoveSpeed);
    Read(value, "TakeOffDistance", out.TakeOffDistance);
    Read(value, "TakeOffHeight", out.TakeOffHeight);
    Read(value, "TakeOffCurve", out.TakeOffCurve);
    Read(value, "FlightDistance", out.FlightDistance);
    Read(value, "CollisionLayerMask", out.CollisionLayerMask);
    Read(value, "CanSteer", out.CanSteer);
    Read(value, "SteeringMaxAnglePerSecond", out.SteeringMaxAnglePerSecond);
    Read(value, "BombAsset", out.BombAsset);
    Read(value, "BombFirstDropSide", out.BombFirstDropSide);
    Read(value, "BombDropSideOffset", out.BombDropSideOffset);
    Read(value, "BombDropCount", out.BombDropCount);
    Read(value, "BombDropMaxHeight", out.BombDropMaxHeight);
    Read(value, "BombDropOnFlightStart", out.BombDropOnFlightStart);
    Read(value, "BombDropOnFlightEnd", out.BombDropOnFlightEnd);
    Read(value, "DroppingOnBeingDestroyed", out.DroppingOnBeingDestroyed);
    Read(value, "DroppingGravity", out.DroppingGravity);
    Read(value, "DroppingMaxDuration", out.DroppingMaxDuration);
    Read(value, "DroppingExplosionAsset", out.DroppingExplosionAsset);
}

void ReadFields(const Value& value, Quantum::BombardierWeaponAutoTargetSpec& out, ReaderTag) {
    Read(value, "BombRadiusToAreaFactor", out.BombRadiusToAreaFactor);
    Read(value, "ShowGizmos", out.ShowGizmos);
}

void ReadFields(const Value& value, Quantum::BuzzbladesRicochetSpec& out, ReaderTag) {
    Read(value, "Radius", out.Radius);
    Read(value, "MaxHitCountIncrementPerNewEnemy", out.MaxHitCountIncrementPerNewEnemy);
    Read(value, "ExtendedMaxHitCount", out.ExtendedMaxHitCount);
}

void ReadFields(const Value&, Quantum::CallbackAction&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::CollisionForceSpec& out, ReaderTag) {
    Read(value, "collideIfMovingTowardsTargetOnly", out.collideIfMovingTowardsTargetOnly);
    Read(value, "excludeDashingPlayers", out.excludeDashingPlayers);
    Read(value, "useHitAndDamageDataFromComponentsSpecs", out.useHitAndDamageDataFromComponentsSpecs);
    Read(value, "damageAllies", out.damageAllies);
    Read(value, "fixedDamage", out.fixedDamage);
    Read(value, "maxDamage", out.maxDamage);
    Read(value, "speedToDamageCurve", out.speedToDamageCurve);
    Read(value, "maxSpeed", out.maxSpeed);
    Read(value, "pushAllies", out.pushAllies);
    Read(value, "pushDirectionVerticalBonus", out.pushDirectionVerticalBonus);
    Read(value, "fixedPush", out.fixedPush);
    Read(value, "maxPush", out.maxPush);
    Read(value, "speedToPushCurve", out.speedToPushCurve);
}

void ReadFields(const Value& value, Quantum::CryosliderSpec& out, ReaderTag) {
    Read(value, "killWhenWeaponRangeTravelled", out.killWhenWeaponRangeTravelled);
    Read(value, "killAboveMaxFallHeight", out.killAboveMaxFallHeight);
    Read(value, "maxFallHeight", out.maxFallHeight);
    Read(value, "UseHoming", out.UseHoming);
    Read(value, "HomingAngularSpeed", out.HomingAngularSpeed);
}

void ReadFields(const Value& value, Quantum::CryosliderWeaponAutoTargetSpec& out, ReaderTag) {
    Read(value, "maxYForValidTarget", out.maxYForValidTarget);
    Read(value, "minYForValidTarget", out.minYForValidTarget);
    Read(value, "minZForValidTarget", out.minZForValidTarget);
    Read(value, "useWidthFromAoeTrail", out.useWidthFromAoeTrail);
    Read(value, "autoTargetWidth", out.autoTargetWidth);
    Read(value, "ShowGizmos", out.ShowGizmos);
}

void ReadFields(const Value& value, Quantum::DamageChargeableSpec& out, ReaderTag) {
    Read(value, "MaxChargeValue", out.MaxChargeValue);
    Read(value, "InactivityResetDelay", out.InactivityResetDelay);
    Read(value, "onFullyCharged", out.onFullyCharged);
    Read(value, "ActivationDuration", out.ActivationDuration);
    Read(value, "onTimerElapsed", out.onTimerElapsed);
}

void ReadFields(const Value& value, Quantum::DamageMultiplierSpec& out, ReaderTag) {
    Read(value, "Multiplier", out.Multiplier);
    Read(value, "DamageFilter", out.DamageFilter);
    Read(value, "WeaponID", out.WeaponID);
    Read(value, "UseDamageType", out.UseDamageType);
    Read(value, "DamageType", out.DamageType);
}

void ReadFields(const Value& value, Quantum::DamageSpec& out, ReaderTag) {
    Read(value, "EntityFilter", out.EntityFilter);
    Read(value, "DamageType", out.DamageType);
    Read(value, "ElectricalBuildUpPerHit", out.ElectricalBuildUpPerHit);
    Read(value, "IceBuildUpPerHit", out.IceBuildUpPerHit);
    Read(value, "BaseUnscaledAmount", out.BaseUnscaledAmount);
    Read(value, "Mode", out.Mode);
    Read(value, "Amount", out.Amount);
    Read(value, "BaseAmount", out.BaseAmount);
    Read(value, "MinAmount", out.MinAmount);
    Read(value, "MaxAmount", out.MaxAmount);
    Read(value, "AmountPerLevel", out.AmountPerLevel);
    Read(value, "AmountPerSelfLevel", out.AmountPerSelfLevel);
    Read(value, "DamageShield", out.DamageShield);
    Read(value, "BreakShield", out.BreakShield);
    Read(value, "ShieldDamageMultiplier", out.ShieldDamageMultiplier);
    Read(value, "ApplyMoveSpeedModifier", out.ApplyMoveSpeedModifier);
    Read(value, "DamageHealth", out.DamageHealth);
    Read(value, "HealthDamageMultiplier", out.HealthDamageMultiplier);
    Read(value, "CanKill", out.CanKill);
    Read(value, "UseReticleFeedback", out.UseReticleFeedback);
    Read(value, "SlowDownPlayerActor", out.SlowDownPlayerActor);
}

void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec& out, ReaderTag) {
    Read(value, "mode", out.mode);
    Read(value, "constantMinDamage", out.constantMinDamage);
    Read(value, "constantMaxDamage", out.constantMaxDamage);
    Read(value, "curveMinDamage", out.curveMinDamage);
    Read(value, "curveMaxDamage", out.curveMaxDamage);
    Read(value, "damageToImpulseCurve", out.damageToImpulseCurve);
    Read(value, "curveMinValue", out.curveMinValue);
    Read(value, "curveMaxValue", out.curveMaxValue);
    Read(value, "weaponMultipliers", out.weaponMultipliers);
    Read(value, "aoeMultipliers", out.aoeMultipliers);
    Read(value, "useImpactDirection", out.useImpactDirection);
    Read(value, "impactDirectionInterpolation", out.impactDirectionInterpolation);
    Read(value, "impactVerticalDirectionMult", out.impactVerticalDirectionMult);
    Read(value, "impactHorizontalDirectionMult", out.impactHorizontalDirectionMult);
}

void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec::AoEMultiplier& out, ReaderTag) {
    Read(value, "AoE", out.AoE);
    Read(value, "Multiplier", out.Multiplier);
    Read(value, "UseLateralImpactDirectionOnly", out.UseLateralImpactDirectionOnly);
}

void ReadFields(const Value& value, Quantum::DamageToImpulseConverterSpec::WeaponMultiplier& out, ReaderTag) {
    Read(value, "WeaponID", out.WeaponID);
    Read(value, "Multiplier", out.Multiplier);
    Read(value, "UseLateralImpactDirectionOnly", out.UseLateralImpactDirectionOnly);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec& out, ReaderTag) {
    Read(value, "DefaultDroppableSpec", out.DefaultDroppableSpec);
    Read(value, "DroppableSpecOverrides", out.DroppableSpecOverrides);
    Read(value, "PowerUpOverhealthAssetRef", out.PowerUpOverhealthAssetRef);
    Read(value, "PowerUpExtraDamageAssetRef", out.PowerUpExtraDamageAssetRef);
    Read(value, "PowerUpExtraDamagePVEAssetRef", out.PowerUpExtraDamagePVEAssetRef);
    Read(value, "PowerUpSpeedBoostAssetRef", out.PowerUpSpeedBoostAssetRef);
    Read(value, "PowerUpPermanentOverhealthAssetRef", out.PowerUpPermanentOverhealthAssetRef);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableOverride& out, ReaderTag) {
    Read(value, "Types", out.Types);
    Read(value, "Spec", out.Spec);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec& out, ReaderTag) {
    Read(value, "Drop", out.Drop);
    Read(value, "Collect", out.Collect);
    Read(value, "ShrinkingZone", out.ShrinkingZone);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::CollectSpec& out, ReaderTag) {
    Read(value, "DelayBeforeCollection", out.DelayBeforeCollection);
    Read(value, "Range", out.Range);
    Read(value, "ShortRange", out.ShortRange);
    Read(value, "Duration", out.Duration);
    Read(value, "DelayForDropper", out.DelayForDropper);
    Read(value, "DetectionFrequency", out.DetectionFrequency);
    Read(value, "TimeWindowDuration", out.TimeWindowDuration);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::DropSpec& out, ReaderTag) {
    Read(value, "Duration", out.Duration);
    Read(value, "XZEasing", out.XZEasing);
    Read(value, "YEasing", out.YEasing);
    Read(value, "YAmplitude", out.YAmplitude);
    Read(value, "MinRange", out.MinRange);
    Read(value, "MaxRange", out.MaxRange);
}

void ReadFields(const Value& value, Quantum::DroppableManagerSpec::DroppableSpec::ShrinkingZoneSpec& out, ReaderTag) {
    Read(value, "Margin", out.Margin);
    Read(value, "Delay", out.Delay);
}

void ReadFields(const Value& value, Quantum::ElementalDamageTable& out, ReaderTag) {
    Read(value, "mappings", out.mappings);
    Read(value, "fireDamagePrototype", out.fireDamagePrototype);
    Read(value, "fireDamageEntityFilter", out.fireDamageEntityFilter);
    Read(value, "AreaOfEffectIgnitionSpecRef", out.AreaOfEffectIgnitionSpecRef);
    Read(value, "electricalDamageSettings", out.electricalDamageSettings);
    Read(value, "iceDamageSettings", out.iceDamageSettings);
}

void ReadFields(const Value& value, Quantum::EnergyCellManagerSpec& out, ReaderTag) {
    Read(value, "LowCharge", out.LowCharge);
    Read(value, "MediumCharge", out.MediumCharge);
    Read(value, "HighCharge", out.HighCharge);
}

void ReadFields(const Value& value, Quantum::EntityDropperSpec& out, ReaderTag) {
    Read(value, "Type", out.Type);
    Read(value, "DropOffset", out.DropOffset);
    Read(value, "OverallEntityDropCountPercent", out.OverallEntityDropCountPercent);
    Read(value, "IfAffectedByGameModeMultiplier", out.IfAffectedByGameModeMultiplier);
    Read(value, "DropProbabilityPercent", out.DropProbabilityPercent);
    Read(value, "TryDropOnGround", out.TryDropOnGround);
    Read(value, "FindGroundMaxDistance", out.FindGroundMaxDistance);
    Read(value, "OffsetFromGround", out.OffsetFromGround);
    Read(value, "Lifetime", out.Lifetime);
    Read(value, "PreventOwnerToCollect", out.PreventOwnerToCollect);
    Read(value, "DropsOnDeath", out.DropsOnDeath);
    Read(value, "KillTypes", out.KillTypes);
    Read(value, "DropAllOnDeath", out.DropAllOnDeath);
    Read(value, "MinCounterOnDeath", out.MinCounterOnDeath);
    Read(value, "DropCountOnDeath", out.DropCountOnDeath);
    Read(value, "DropsOnHealthDecreased", out.DropsOnHealthDecreased);
    Read(value, "DropsEveryPercentOnHealthDecreased", out.DropsEveryPercentOnHealthDecreased);
    Read(value, "DropCountOnHealthDecreased", out.DropCountOnHealthDecreased);
}

void ReadFields(const Value& value, Quantum::EntityGroupSpec& out, ReaderTag) {
    Read(value, "GroupMaxMembers", out.GroupMaxMembers);
}

void ReadFields(const Value& value, Quantum::FireReactorSpec& out, ReaderTag) {
    Read(value, "DisabledDuration", out.DisabledDuration);
    Read(value, "EnabledDuration", out.EnabledDuration);
}

void ReadFields(const Value& value, Quantum::HealSourceSpec& out, ReaderTag) {
    Read(value, "Type", out.Type);
    Read(value, "HealPerSecond", out.HealPerSecond);
    Read(value, "PercentGainedPerSecond", out.PercentGainedPerSecond);
}

void ReadFields(const Value& value, Quantum::LifetimeSpec& out, ReaderTag) {
    Read(value, "StartLifetimeOnSpawn", out.StartLifetimeOnSpawn);
    Read(value, "UseOwnerWeaponLevelForLifetime", out.UseOwnerWeaponLevelForLifetime);
    Read(value, "LifetimeDuration", out.LifetimeDuration);
    Read(value, "ReduceDurationIfHitDamageable", out.ReduceDurationIfHitDamageable);
    Read(value, "ReducedLifetimeDuration", out.ReducedLifetimeDuration);
    Read(value, "LeveledLifetimeDurations", out.LeveledLifetimeDurations);
    Read(value, "SpawnEntityOnDeath", out.SpawnEntityOnDeath);
    Read(value, "EntitiesToSpawnOnDeath", out.EntitiesToSpawnOnDeath);
    Read(value, "SpawnOnDeathOffset", out.SpawnOnDeathOffset);
    Read(value, "UseRaycastToSpawnOnDeath", out.UseRaycastToSpawnOnDeath);
    Read(value, "UseRaycastToSpawnOnDeathLayerMask", out.UseRaycastToSpawnOnDeathLayerMask);
    Read(value, "SpawnOnDeathRaycastDistance", out.SpawnOnDeathRaycastDistance);
    Read(value, "SpawnOnDeathOrientOnNormal", out.SpawnOnDeathOrientOnNormal);
}

void ReadFields(const Value& value, Quantum::MapCustomData& out, ReaderTag) {
    Read(value, "SpawnPoints", out.SpawnPoints);
    Read(value, "blastBallData", out.blastBallData);
    Read(value, "ControlZones", out.ControlZones);
    Read(value, "tutorialData", out.tutorialData);
    Read(value, "ClosingZones", out.ClosingZones);
    Read(value, "GoldenBoltSpawnPoints", out.GoldenBoltSpawnPoints);
    Read(value, "StaticDroppables", out.StaticDroppables);
    Read(value, "staticJumpers", out.staticJumpers);
    Read(value, "staticTravolator", out.staticTravolator);
    Read(value, "bakedGrindRails", out.bakedGrindRails);
    Read(value, "staticColliderGrindRailPairs", out.staticColliderGrindRailPairs);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedAutoJumper& out, ReaderTag) {
    Read(value, "staticColliderIndex", out.staticColliderIndex);
    Read(value, "autoJumperStartParams", out.autoJumperStartParams);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedGrindRail& out, ReaderTag) {
    Read(value, "spline", out.spline);
    Read(value, "collidersData", out.collidersData);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedRailColliderData& out, ReaderTag) {
    Read(value, "staticColliderIndex", out.staticColliderIndex);
    Read(value, "distance", out.distance);
    Read(value, "nearbyRailColIndexLeft", out.nearbyRailColIndexLeft);
    Read(value, "nearbyRailColIndexRight", out.nearbyRailColIndexRight);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedSpline& out, ReaderTag) {
    Read(value, "samples", out.samples);
    Read(value, "length", out.length);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedSplineSample& out, ReaderTag) {
    Read(value, "posWorld", out.posWorld);
    Read(value, "distance", out.distance);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BakedTravolator& out, ReaderTag) {
    Read(value, "staticColliderIndex", out.staticColliderIndex);
    Read(value, "direction", out.direction);
}

void ReadFields(const Value& value, Quantum::MapCustomData::BlastBallData& out, ReaderTag) {
    Read(value, "ballPosition", out.ballPosition);
}

void ReadFields(const Value& value, Quantum::MapCustomData::ClosingPointData& out, ReaderTag) {
    Read(value, "targetPosition", out.targetPosition);
    Read(value, "targetRadius", out.targetRadius);
    Read(value, "startTime", out.startTime);
    Read(value, "endTime", out.endTime);
}

void ReadFields(const Value& value, Quantum::MapCustomData::ClosingZoneData& out, ReaderTag) {
    Read(value, "startPosition", out.startPosition);
    Read(value, "startRadius", out.startRadius);
    Read(value, "closingPoints", out.closingPoints);
}

void ReadFields(const Value& value, Quantum::MapCustomData::ControlZoneData& out, ReaderTag) {
    Read(value, "position", out.position);
    Read(value, "radius", out.radius);
    Read(value, "id", out.id);
}

void ReadFields(const Value& value, Quantum::MapCustomData::SpawnPointData& out, ReaderTag) {
    Read(value, "position", out.position);
    Read(value, "rotY", out.rotY);
    Read(value, "options", out.options);
    Read(value, "teamFlag", out.teamFlag);
    Read(value, "introTrackId", out.introTrackId);
    Read(value, "availabilityDuration", out.availabilityDuration);
}

void ReadFields(const Value& value, Quantum::MapCustomData::StaticColliderGrindRailPair& out, ReaderTag) {
    Read(value, "staticColliderIndex", out.staticColliderIndex);
    Read(value, "grindRailIndex", out.grindRailIndex);
}

void ReadFields(const Value& value, Quantum::MapCustomData::StaticDroppableData& out, ReaderTag) {
    Read(value, "type", out.type);
    Read(value, "position", out.position);
}

void ReadFields(const Value& value, Quantum::MapCustomData::TutorialData& out, ReaderTag) {
    Read(value, "tutoShootPosition", out.tutoShootPosition);
    Read(value, "tutoShootRotation", out.tutoShootRotation);
    Read(value, "tutoMeleePosition", out.tutoMeleePosition);
    Read(value, "tutoMeleeRotation", out.tutoMeleeRotation);
    Read(value, "arenaPosition", out.arenaPosition);
    Read(value, "arenaRotation", out.arenaRotation);
    Read(value, "ultimatePosition", out.ultimatePosition);
    Read(value, "ultimateRotation", out.ultimateRotation);
}

void ReadFields(const Value& value, Quantum::MoveToSpec& out, ReaderTag) {
    Read(value, "moveMode", out.moveMode);
    Read(value, "offsetOrientation", out.offsetOrientation);
    Read(value, "position", out.position);
    Read(value, "followEntity", out.followEntity);
    Read(value, "offset", out.offset);
    Read(value, "delay", out.delay);
    Read(value, "duration", out.duration);
    Read(value, "easing", out.easing);
}

void ReadFields(const Value& value, Quantum::ParabolaFollowerSpec& out, ReaderTag) {
    Read(value, "Speed", out.Speed);
    Read(value, "MinDuration", out.MinDuration);
    Read(value, "ConstantSpeed", out.ConstantSpeed);
    Read(value, "SpeedCurve", out.SpeedCurve);
    Read(value, "Height", out.Height);
    Read(value, "CanGoBeyondEnd", out.CanGoBeyondEnd);
    Read(value, "AutoRemoveOnReachedEnd", out.AutoRemoveOnReachedEnd);
    Read(value, "AutoRemoveOnDeath", out.AutoRemoveOnDeath);
    Read(value, "RemoveDelay", out.RemoveDelay);
    Read(value, "ForceClampPosition", out.ForceClampPosition);
    Read(value, "UseMidPointCurve", out.UseMidPointCurve);
    Read(value, "HomingEnabled", out.HomingEnabled);
    Read(value, "HomingPower", out.HomingPower);
    Read(value, "UseTargetFinder", out.UseTargetFinder);
    Read(value, "SnapOnNewTarget", out.SnapOnNewTarget);
    Read(value, "TimeToFindNewTarget", out.TimeToFindNewTarget);
    Read(value, "AirControlEnabled", out.AirControlEnabled);
    Read(value, "AirControlSpeed", out.AirControlSpeed);
    Read(value, "OrientToEndPos", out.OrientToEndPos);
    Read(value, "OnlyOrientYAxis", out.OnlyOrientYAxis);
    Read(value, "OrientationOffset", out.OrientationOffset);
}

void ReadFields(const Value& value, Quantum::PowerUpSpec& out, ReaderTag) {
    Read(value, "HasDuration", out.HasDuration);
    Read(value, "Duration", out.Duration);
}

void ReadFields(const Value& value, Quantum::PrioritisedTargetHandlerSpec& out, ReaderTag) {
    Read(value, "detectionRadius_GroupTarget", out.detectionRadius_GroupTarget);
    Read(value, "detectionRadius_VisibleThreat", out.detectionRadius_VisibleThreat);
    Read(value, "detectionRadius_AttackedByThreat", out.detectionRadius_AttackedByThreat);
    Read(value, "acquireIfAttackedRecentlySeconds", out.acquireIfAttackedRecentlySeconds);
    Read(value, "reacquireVisibleThreatAfterSeconds", out.reacquireVisibleThreatAfterSeconds);
    Read(value, "forgetRecentThreatDuration", out.forgetRecentThreatDuration);
}

void ReadFields(const Value& value, Quantum::ProjectileSpawnerSpec& out, ReaderTag) {
    Read(value, "Projectile", out.Projectile);
    Read(value, "ProjectileSpec", out.ProjectileSpec);
    Read(value, "ProjectileCount", out.ProjectileCount);
    Read(value, "Range", out.Range);
    Read(value, "Height", out.Height);
    Read(value, "Delay", out.Delay);
    Read(value, "Interval", out.Interval);
}

void ReadFields(const Value& value, Quantum::ProjectileSpec& out, ReaderTag) {
    Read(value, "Type", out.Type);
    Read(value, "Speed", out.Speed);
    Read(value, "MaxLifeTime", out.MaxLifeTime);
    Read(value, "OverrideMaxTravelDistance", out.OverrideMaxTravelDistance);
    Read(value, "MaxTravelDistance", out.MaxTravelDistance);
    Read(value, "ReturnsToOwner", out.ReturnsToOwner);
    Read(value, "ForcedTargetedPart", out.ForcedTargetedPart);
    Read(value, "Gravity", out.Gravity);
    Read(value, "OverrideGravityAfterDistance", out.OverrideGravityAfterDistance);
    Read(value, "OverrideGravityDistance", out.OverrideGravityDistance);
    Read(value, "OverrideGravityValue", out.OverrideGravityValue);
    Read(value, "Elevates", out.Elevates);
    Read(value, "ElevationHeight", out.ElevationHeight);
    Read(value, "Accelerates", out.Accelerates);
    Read(value, "AccelerationType", out.AccelerationType);
    Read(value, "LinearAcceleration", out.LinearAcceleration);
    Read(value, "CurveAccelerationMaxSpeed", out.CurveAccelerationMaxSpeed);
    Read(value, "CurveAccelerationDuration", out.CurveAccelerationDuration);
    Read(value, "AccelerationCurve", out.AccelerationCurve);
    Read(value, "LobOvershootMinHeight", out.LobOvershootMinHeight);
    Read(value, "LobOvershootMaxRangePercent", out.LobOvershootMaxRangePercent);
    Read(value, "LobCurves", out.LobCurves);
    Read(value, "LobCurve", out.LobCurve);
    Read(value, "LobFlightTimeDurationSeconds", out.LobFlightTimeDurationSeconds);
    Read(value, "LobScaleFlightHeightWithTargetDistance", out.LobScaleFlightHeightWithTargetDistance);
    Read(value, "LobFlightInverseHeightCoef", out.LobFlightInverseHeightCoef);
    Read(value, "LobAcquireTargetMinDistance", out.LobAcquireTargetMinDistance);
    Read(value, "LobLoseTargetMaxDistance", out.LobLoseTargetMaxDistance);
    Read(value, "SetAutoTargetFromAOE", out.SetAutoTargetFromAOE);
    Read(value, "AimLowerOnEntityTarget", out.AimLowerOnEntityTarget);
    Read(value, "AnticipationForwardCurve", out.AnticipationForwardCurve);
    Read(value, "HorizontalOffsetCurve", out.HorizontalOffsetCurve);
    Read(value, "VerticalOffsetCurve", out.VerticalOffsetCurve);
    Read(value, "CollisionLayerMask", out.CollisionLayerMask);
    Read(value, "ImpactType", out.ImpactType);
    Read(value, "SpawnEntityOnDeath", out.SpawnEntityOnDeath);
    Read(value, "AreaOfEffectAsset", out.AreaOfEffectAsset);
    Read(value, "AssignSpawnedEntityToOwnerTeam", out.AssignSpawnedEntityToOwnerTeam);
    Read(value, "AttachSpawnedEntityToImpactPoint", out.AttachSpawnedEntityToImpactPoint);
    Read(value, "SpawnNormalOffset", out.SpawnNormalOffset);
    Read(value, "OrientSpawnedEntityOnNormal", out.OrientSpawnedEntityOnNormal);
    Read(value, "TryFindBestOrientationNormal", out.TryFindBestOrientationNormal);
    Read(value, "ResetSpawnedEntityUpVector", out.ResetSpawnedEntityUpVector);
    Read(value, "Rebound", out.Rebound);
    Read(value, "GroundNormalAngleTolerance", out.GroundNormalAngleTolerance);
    Read(value, "LeadingFactor", out.LeadingFactor);
    Read(value, "IsHoming", out.IsHoming);
    Read(value, "InitialHomingLerpFactor", out.InitialHomingLerpFactor);
    Read(value, "ProgressiveHomingLerpIncreaseFactor", out.ProgressiveHomingLerpIncreaseFactor);
    Read(value, "HomingMaxAngle", out.HomingMaxAngle);
}

void ReadFields(const Value& value, Quantum::PusherSpec& out, ReaderTag) {
    Read(value, "PushFilters", out.PushFilters);
    Read(value, "Is2D", out.Is2D);
    Read(value, "PushForce", out.PushForce);
    Read(value, "PushUpwardForce", out.PushUpwardForce);
    Read(value, "LeveledPushForce", out.LeveledPushForce);
    Read(value, "UseCurve", out.UseCurve);
    Read(value, "ForceCurve", out.ForceCurve);
    Read(value, "IsInverse", out.IsInverse);
    Read(value, "IsContinuous", out.IsContinuous);
    Read(value, "OnShieldBehaviour", out.OnShieldBehaviour);
    Read(value, "BypassPushStop", out.BypassPushStop);
    Read(value, "ForcePushStop", out.ForcePushStop);
    Read(value, "ForcePushStopStrength", out.ForcePushStopStrength);
    Read(value, "CanBump", out.CanBump);
    Read(value, "BumpType", out.BumpType);
    Read(value, "BumpReductionPercentage", out.BumpReductionPercentage);
    Read(value, "FlatBumpStrength", out.FlatBumpStrength);
}

void ReadFields(const Value& value, Quantum::RewardSpec& out, ReaderTag) {
    Read(value, "type", out.type);
    Read(value, "amountType", out.amountType);
    Read(value, "amount", out.amount);
}

void ReadFields(const Value& value, Quantum::SeekNewTargetPriorityList& out, ReaderTag) {
    Read(value, "DetectionLayers", out.DetectionLayers);
    Read(value, "PrioritisedEntityFilters", out.PrioritisedEntityFilters);
}

void ReadFields(const Value& value, Quantum::ShapeOverlapperSpec& out, ReaderTag) {
    Read(value, "shape", out.shape);
    Read(value, "layerMask", out.layerMask);
    Read(value, "bypassCaster", out.bypassCaster);
    Read(value, "bypassOwner", out.bypassOwner);
    Read(value, "useCastRate", out.useCastRate);
    Read(value, "castRate", out.castRate);
    Read(value, "NeedsLineOfSightCheck", out.NeedsLineOfSightCheck);
    Read(value, "LineOfSightQueryOptions", out.LineOfSightQueryOptions);
}

void ReadFields(const Value& value, Quantum::SpeederSpec& out, ReaderTag) {
    Read(value, "Cooldown", out.Cooldown);
}

void ReadFields(const Value& value, Quantum::StreamEffectSpawnerSpec& out, ReaderTag) {
    Read(value, "StreamEffectAsset", out.StreamEffectAsset);
}

void ReadFields(const Value& value, Quantum::StreamEffectSpec& out, ReaderTag) {
    Read(value, "StartRadius", out.StartRadius);
    Read(value, "EndRadius", out.EndRadius);
    Read(value, "TimeToMaxRadius", out.TimeToMaxRadius);
}

void ReadFields(const Value& value, Quantum::TargetFinderSpec& out, ReaderTag) {
    Read(value, "TargetEntityFilter", out.TargetEntityFilter);
    Read(value, "RangeNoWeapon", out.RangeNoWeapon);
    Read(value, "RequireGrounded", out.RequireGrounded);
    Read(value, "FindOnUpdate", out.FindOnUpdate);
    Read(value, "FindOnWeaponUse", out.FindOnWeaponUse);
    Read(value, "NeedBeInFront", out.NeedBeInFront);
    Read(value, "InFrontAngleLimitDegrees", out.InFrontAngleLimitDegrees);
    Read(value, "BypassIfVeryClose", out.BypassIfVeryClose);
    Read(value, "VeryCloseDistance", out.VeryCloseDistance);
    Read(value, "OverrideWeaponRange", out.OverrideWeaponRange);
    Read(value, "RangeOverride", out.RangeOverride);
    Read(value, "RangeExtensionPercent", out.RangeExtensionPercent);
    Read(value, "HasMinRange", out.HasMinRange);
    Read(value, "MinRange", out.MinRange);
    Read(value, "HasMaxHeightDiff", out.HasMaxHeightDiff);
    Read(value, "MaxHeightDiff", out.MaxHeightDiff);
    Read(value, "NeedLineOfSight", out.NeedLineOfSight);
    Read(value, "LineOfSightStartOffset", out.LineOfSightStartOffset);
    Read(value, "CheckLinearProjectileElevation", out.CheckLinearProjectileElevation);
    Read(value, "LinearProjectileElevatedCurveRaycastCount", out.LinearProjectileElevatedCurveRaycastCount);
    Read(value, "UseCamSightCheck", out.UseCamSightCheck);
    Read(value, "MinCamSightAngleDegreesFar", out.MinCamSightAngleDegreesFar);
    Read(value, "MinCamSightAngleDegreesNear", out.MinCamSightAngleDegreesNear);
    Read(value, "MaxCamSightAngleDegreesFar", out.MaxCamSightAngleDegreesFar);
    Read(value, "MaxCamSightAngleDegreesNear", out.MaxCamSightAngleDegreesNear);
    Read(value, "CamAngleCloseDistance", out.CamAngleCloseDistance);
    Read(value, "CamDotValueForOnReticle", out.CamDotValueForOnReticle);
    Read(value, "ScoreOnReticle", out.ScoreOnReticle);
    Read(value, "ScoreDistance", out.ScoreDistance);
    Read(value, "ScoreInFront", out.ScoreInFront);
    Read(value, "ScorePreviousTarget", out.ScorePreviousTarget);
    Read(value, "EntityTypeScores", out.EntityTypeScores);
    Read(value, "SetAutoTarget", out.SetAutoTarget);
    Read(value, "DisableOnPushed", out.DisableOnPushed);
}

void ReadFields(const Value& value, Quantum::TargetedEffectSpec& out, ReaderTag) {
    Read(value, "InstantFirstTick", out.InstantFirstTick);
    Read(value, "TickRate", out.TickRate);
    Read(value, "MaxTickCount", out.MaxTickCount);
}

void ReadFields(const Value& value, Quantum::TrailSpawnerSpec& out, ReaderTag) {
    Read(value, "TrailAsset", out.TrailAsset);
    Read(value, "AoeTrailWidth", out.AoeTrailWidth);
    Read(value, "AoeTrailSegmentLength", out.AoeTrailSegmentLength);
}

void ReadFields(const Value& value, Quantum::Action_AddModifierMoveSpeed& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::CallbackAction&>(out), ReaderTag{});
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
    Read(value, "MoveSpeedDuration", out.MoveSpeedDuration);
    Read(value, "MoveSpeedModOrigin", out.MoveSpeedModOrigin);
}

void ReadFields(const Value& value, Quantum::Action_KillSelf& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::CallbackAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::Action_MoveTo_StartMoving& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::CallbackAction&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::Action_SpawnEntities& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::CallbackAction&>(out), ReaderTag{});
    Read(value, "EntitiesToSpawn", out.EntitiesToSpawn);
    Read(value, "Offset", out.Offset);
    Read(value, "UseRaycast", out.UseRaycast);
    Read(value, "RaycastLayerMask", out.RaycastLayerMask);
    Read(value, "RaycastDistance", out.RaycastDistance);
    Read(value, "OrientOnNormal", out.OrientOnNormal);
}

void ReadFields(const Value& value, Quantum::BoltDropperSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::EntityDropperSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::EnergyCellDropperSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::EntityDropperSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::EnergyCellPowerUpSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::PowerUpSpec&>(out), ReaderTag{});
    Read(value, "ChargeValue", out.ChargeValue);
}

void ReadFields(const Value& value, Quantum::ExtraDamagePowerUpSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::PowerUpSpec&>(out), ReaderTag{});
    Read(value, "DamageBoostPercent", out.DamageBoostPercent);
}

void ReadFields(const Value& value, Quantum::OverhealthPowerUpSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::PowerUpSpec&>(out), ReaderTag{});
    Read(value, "DecreaseDuration", out.DecreaseDuration);
    Read(value, "DecreaseTickRate", out.DecreaseTickRate);
    Read(value, "Mode", out.Mode);
    Read(value, "FixedHealthBoost", out.FixedHealthBoost);
    Read(value, "MinHealthBoost", out.MinHealthBoost);
    Read(value, "MaxHealthBoost", out.MaxHealthBoost);
    Read(value, "HealthBoostPerLevel", out.HealthBoostPerLevel);
    Read(value, "Percentage", out.Percentage);
}

void ReadFields(const Value& value, Quantum::PowerUpDropperSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::EntityDropperSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::SpeedBoostPowerUpSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::PowerUpSpec&>(out), ReaderTag{});
    Read(value, "SpeedModifier", out.SpeedModifier);
}

void ReadFields(const Value& value, Quantum::AIMoveForwardBehaviourSpec& out, ReaderTag) {
    Read(value, "BounceOnNavMesh", out.BounceOnNavMesh);
    Read(value, "ConstrainToSpawner", out.ConstrainToSpawner);
    Read(value, "DieOnWallHit", out.DieOnWallHit);
    Read(value, "BounceOnDynamicObjects", out.BounceOnDynamicObjects);
}

void ReadFields(const Value& value, Quantum::AINavigationSpec& out, ReaderTag) {
    Read(value, "DisabledRegions", out.DisabledRegions);
    Read(value, "IsObstacle", out.IsObstacle);
    Read(value, "ObstacleRadius", out.ObstacleRadius);
    Read(value, "ObstacleLayer", out.ObstacleLayer);
    Read(value, "UseAvoidance", out.UseAvoidance);
}

void ReadFields(const Value&, Quantum::ActorComponentSpecBase&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::ActorManagerConfig& out, ReaderTag) {
    Read(value, "PendingRespawnDuration", out.PendingRespawnDuration);
    Read(value, "BotsUseSpecificPendingRespawnDuration", out.BotsUseSpecificPendingRespawnDuration);
    Read(value, "BotsPendingRespawnDuration", out.BotsPendingRespawnDuration);
    Read(value, "RespawnDuration", out.RespawnDuration);
    Read(value, "RespawnPointCalculationDelay", out.RespawnPointCalculationDelay);
    Read(value, "ShrinkingZoneRespawnMargin", out.ShrinkingZoneRespawnMargin);
    Read(value, "ShrinkingZoneRespawnSafeDuration", out.ShrinkingZoneRespawnSafeDuration);
    Read(value, "RespawnPointCooldownDuration", out.RespawnPointCooldownDuration);
    Read(value, "RespawnInnerRadius", out.RespawnInnerRadius);
    Read(value, "RespawnOuterRadius", out.RespawnOuterRadius);
}

void ReadFields(const Value& value, Quantum::ActorStatusModifierSpec& out, ReaderTag) {
    Read(value, "wantedType", out.wantedType);
    Read(value, "durationSeconds", out.durationSeconds);
    Read(value, "bypassShield", out.bypassShield);
}

void ReadFields(const Value& value, Quantum::ActorStatusSpec& out, ReaderTag) {
    Read(value, "typeSettings", out.typeSettings);
}

void ReadFields(const Value& value, Quantum::BoltGathererActorSpec& out, ReaderTag) {
    Read(value, "PreventBoltCollectionDefaultDuration", out.PreventBoltCollectionDefaultDuration);
    Read(value, "InitialBoltCount", out.InitialBoltCount);
    Read(value, "MaxBoltsCollectable", out.MaxBoltsCollectable);
}

void ReadFields(const Value& value, Quantum::BoltPowerActorSpec& out, ReaderTag) {
    Read(value, "MaxBolts", out.MaxBolts);
    Read(value, "MaxLevel", out.MaxLevel);
    Read(value, "DamageMultiplier", out.DamageMultiplier);
    Read(value, "MaxHealthMultiplier", out.MaxHealthMultiplier);
    Read(value, "MaxShieldMultiplier", out.MaxShieldMultiplier);
    Read(value, "UltimateTimeMultiplier", out.UltimateTimeMultiplier);
    Read(value, "UltimatePerPushReceivedMultiplier", out.UltimatePerPushReceivedMultiplier);
}

void ReadFields(const Value& value, Quantum::ChargeAttackSpec& out, ReaderTag) {
    Read(value, "anticipationDelay", out.anticipationDelay);
    Read(value, "cooldownDelay", out.cooldownDelay);
    Read(value, "maxTravelDistance", out.maxTravelDistance);
    Read(value, "maxSpeedModifier", out.maxSpeedModifier);
    Read(value, "aoeRange", out.aoeRange);
}

void ReadFields(const Value& value, Quantum::ColliderResizeSpec& out, ReaderTag) {
    Read(value, "TargetExtents", out.TargetExtents);
    Read(value, "TargetCentroid", out.TargetCentroid);
    Read(value, "Duration", out.Duration);
}

void ReadFields(const Value& value, Quantum::DamageableSpec& out, ReaderTag) {
    Read(value, "DamageFilter", out.DamageFilter);
    Read(value, "ElementType", out.ElementType);
    Read(value, "InstantKillByBuffedDamageTypes", out.InstantKillByBuffedDamageTypes);
    Read(value, "InitialInvulnerabilityDuration", out.InitialInvulnerabilityDuration);
    Read(value, "SpawnInvulnerabilityDuration", out.SpawnInvulnerabilityDuration);
    Read(value, "RespawnInvulnerabilityDuration", out.RespawnInvulnerabilityDuration);
    Read(value, "ShieldBreakInvulnerabilityDuration", out.ShieldBreakInvulnerabilityDuration);
    Read(value, "LastHitSaveDuration", out.LastHitSaveDuration);
}

void ReadFields(const Value& value, Quantum::DeathAnimationSpec& out, ReaderTag) {
    Read(value, "ParabolaFollowerAsset", out.ParabolaFollowerAsset);
    Read(value, "MaxDistance", out.MaxDistance);
    Read(value, "MaxHeight", out.MaxHeight);
    Read(value, "CastGroundOffset", out.CastGroundOffset);
    Read(value, "ShapeCastRadius", out.ShapeCastRadius);
    Read(value, "LayerMask", out.LayerMask);
    Read(value, "QueryOptions", out.QueryOptions);
    Read(value, "RayCastDistance", out.RayCastDistance);
    Read(value, "DefaultEndPointDownOffset", out.DefaultEndPointDownOffset);
    Read(value, "EndPointGroundOffset", out.EndPointGroundOffset);
    Read(value, "EndPointWallOffset", out.EndPointWallOffset);
    Read(value, "KillSourceOffset", out.KillSourceOffset);
}

void ReadFields(const Value& value, Quantum::DroppableCollectorSpec& out, ReaderTag) {
    Read(value, "RangeMultiplier", out.RangeMultiplier);
    Read(value, "MaxDroppablesCollectedPerSecond", out.MaxDroppablesCollectedPerSecond);
}

void ReadFields(const Value& value, Quantum::DroppableCollectorSpec::MaxCollectPerSecond& out, ReaderTag) {
    Read(value, "droppableType", out.droppableType);
    Read(value, "amount", out.amount);
}

void ReadFields(const Value& value, Quantum::HealthSpec& out, ReaderTag) {
    Read(value, "Mode", out.Mode);
    Read(value, "LeveledMaxHP", out.LeveledMaxHP);
    Read(value, "MinHP", out.MinHP);
    Read(value, "MaxHP", out.MaxHP);
    Read(value, "BaseHP", out.BaseHP);
    Read(value, "FixedHP", out.FixedHP);
    Read(value, "AmountPerSelfLevel", out.AmountPerSelfLevel);
    Read(value, "SpawnImmunityDuration", out.SpawnImmunityDuration);
    Read(value, "KillWhenNoMoreHP", out.KillWhenNoMoreHP);
    Read(value, "HasDamageThreshold", out.HasDamageThreshold);
    Read(value, "DamageThreshold", out.DamageThreshold);
    Read(value, "HasPushThreshold", out.HasPushThreshold);
    Read(value, "PushPercentageThreshold", out.PushPercentageThreshold);
    Read(value, "CanRegenerate", out.CanRegenerate);
    Read(value, "TimeWithoutDamageBeforeStartRegen", out.TimeWithoutDamageBeforeStartRegen);
    Read(value, "TimeToRegenerate", out.TimeToRegenerate);
    Read(value, "OnlyRegenerateIfShieldIsUp", out.OnlyRegenerateIfShieldIsUp);
    Read(value, "TakeDamageOutsideShrinkingZone", out.TakeDamageOutsideShrinkingZone);
    Read(value, "PercentDamageOutsideShrinkingZone", out.PercentDamageOutsideShrinkingZone);
    Read(value, "DamageCooldownOutsideShrinkingZone", out.DamageCooldownOutsideShrinkingZone);
    Read(value, "HealthRegenPerPickup", out.HealthRegenPerPickup);
}

void ReadFields(const Value& value, Quantum::HeroSpec& out, ReaderTag) {
    Read(value, "LeveledPower", out.LeveledPower);
    Read(value, "LeveledHP", out.LeveledHP);
    Read(value, "HasShield", out.HasShield);
    Read(value, "LeveledShield", out.LeveledShield);
    Read(value, "HealthSpec", out.HealthSpec);
    Read(value, "CharacterController3DConfig", out.CharacterController3DConfig);
    Read(value, "DefaultWeapon", out.DefaultWeapon);
    Read(value, "ColliderShape", out.ColliderShape);
    Read(value, "HeroImprovementSpecs", out.HeroImprovementSpecs);
    Read(value, "TalentsInUnlockOrder", out.TalentsInUnlockOrder);
    Read(value, "AvailableInBucketIndex", out.AvailableInBucketIndex);
}

void ReadFields(const Value& value, Quantum::JumpAttackSpec& out, ReaderTag) {
    Read(value, "AttackDurationSeconds", out.AttackDurationSeconds);
    Read(value, "HorizontalCurve", out.HorizontalCurve);
    Read(value, "HeightCurve", out.HeightCurve);
    Read(value, "IdealAttackRange", out.IdealAttackRange);
    Read(value, "IdealAttackHeight", out.IdealAttackHeight);
    Read(value, "MaxAttackHeight", out.MaxAttackHeight);
    Read(value, "FinalDistanceToTarget", out.FinalDistanceToTarget);
    Read(value, "EntityToSpawn", out.EntityToSpawn);
    Read(value, "SlamTriggerRatio", out.SlamTriggerRatio);
}

void ReadFields(const Value& value, Quantum::KccExtensionConfig& out, ReaderTag) {
    Read(value, "CanMoveFromInputs", out.CanMoveFromInputs);
    Read(value, "Impulse_Braking", out.Impulse_Braking);
    Read(value, "Impulse_Acceleration", out.Impulse_Acceleration);
    Read(value, "Airborne_Braking", out.Airborne_Braking);
    Read(value, "Airborne_Acceleration", out.Airborne_Acceleration);
    Read(value, "OrientationSharpness", out.OrientationSharpness);
    Read(value, "GoThroughWallCheckThreshold", out.GoThroughWallCheckThreshold);
    Read(value, "WallCheckLayerMask", out.WallCheckLayerMask);
    Read(value, "limitVelocityBasedOnInput", out.limitVelocityBasedOnInput);
    Read(value, "limitVelocityMinValue", out.limitVelocityMinValue);
    Read(value, "maxFallSpeed", out.maxFallSpeed);
    Read(value, "CanJump", out.CanJump);
    Read(value, "JumpRequiredGroundedTime", out.JumpRequiredGroundedTime);
    Read(value, "CoyoteTime", out.CoyoteTime);
    Read(value, "DoubleJumpImpulse", out.DoubleJumpImpulse);
    Read(value, "CanBounce", out.CanBounce);
    Read(value, "BounceRestitution", out.BounceRestitution);
    Read(value, "BounceMinSpeed", out.BounceMinSpeed);
    Read(value, "AutoJumpEnabled", out.AutoJumpEnabled);
    Read(value, "CanAutoJumpWhileAirborne", out.CanAutoJumpWhileAirborne);
    Read(value, "AutoJumpVelocityFactor", out.AutoJumpVelocityFactor);
    Read(value, "AutoJumpHeightCheckStart", out.AutoJumpHeightCheckStart);
    Read(value, "AutoJumpHeightCheckDistance", out.AutoJumpHeightCheckDistance);
    Read(value, "AutoJumpMaxAngle", out.AutoJumpMaxAngle);
    Read(value, "AutoJumpDownTime", out.AutoJumpDownTime);
    Read(value, "AutoJumpDownTimeAfterManualJump", out.AutoJumpDownTimeAfterManualJump);
    Read(value, "AutoJumpOnBouncingBalloons", out.AutoJumpOnBouncingBalloons);
    Read(value, "AutoJumpOnBouncingBalloonsDistance", out.AutoJumpOnBouncingBalloonsDistance);
    Read(value, "AutoJumpOnBouncingBalloonsHeight", out.AutoJumpOnBouncingBalloonsHeight);
    Read(value, "AutoJumpOnBouncingBalloonsFrameSlicing", out.AutoJumpOnBouncingBalloonsFrameSlicing);
    Read(value, "AutoJumpOnWallRun", out.AutoJumpOnWallRun);
    Read(value, "AutoJumpOnWallRunDistance", out.AutoJumpOnWallRunDistance);
    Read(value, "AutoJumpOnWallRunHeight", out.AutoJumpOnWallRunHeight);
    Read(value, "AutoJumpOnWallRunFacingThreshold", out.AutoJumpOnWallRunFacingThreshold);
    Read(value, "AutoJumpOnWallRunFrameSlicing", out.AutoJumpOnWallRunFrameSlicing);
    Read(value, "PushReductionSpeed", out.PushReductionSpeed);
    Read(value, "PushReboundFactor", out.PushReboundFactor);
    Read(value, "PushKillThreshold", out.PushKillThreshold);
    Read(value, "PushUpwardLerp", out.PushUpwardLerp);
    Read(value, "PushUpwardAngle_MaxLerp", out.PushUpwardAngle_MaxLerp);
    Read(value, "PushUpwardAngle_MinLerp", out.PushUpwardAngle_MinLerp);
    Read(value, "ReboundAfterPushMaxDelay", out.ReboundAfterPushMaxDelay);
    Read(value, "ReboundPushThreshold", out.ReboundPushThreshold);
    Read(value, "WalkingPushMultiplierByEntity", out.WalkingPushMultiplierByEntity);
    Read(value, "pushGraceDuration", out.pushGraceDuration);
    Read(value, "pushGraceBreakForceFactorThreshold", out.pushGraceBreakForceFactorThreshold);
    Read(value, "pushStopDuration", out.pushStopDuration);
    Read(value, "LedgeSlowEnable", out.LedgeSlowEnable);
    Read(value, "LedgeSlowVelocityFactor", out.LedgeSlowVelocityFactor);
    Read(value, "LedgeSlowHeightThreshold", out.LedgeSlowHeightThreshold);
    Read(value, "LedgeSlowMaxSpeedReductionAmount", out.LedgeSlowMaxSpeedReductionAmount);
    Read(value, "LedgeSlowMaxSpeedReductionLimit", out.LedgeSlowMaxSpeedReductionLimit);
    Read(value, "LedgeSlowMaxAngle", out.LedgeSlowMaxAngle);
    Read(value, "IsSlowedDownOnHit", out.IsSlowedDownOnHit);
    Read(value, "HitReceivedSlowDownMinDamage", out.HitReceivedSlowDownMinDamage);
    Read(value, "HitReceivedSlowDownMaxDamage", out.HitReceivedSlowDownMaxDamage);
    Read(value, "HitReceivedSlowDownMinSlowdownStrength", out.HitReceivedSlowDownMinSlowdownStrength);
    Read(value, "HitReceivedSlowDownMaxSlowdownStrength", out.HitReceivedSlowDownMaxSlowdownStrength);
    Read(value, "HitReceivedSlowDownMinDuration", out.HitReceivedSlowDownMinDuration);
    Read(value, "HitReceivedSlowDownMaxDuration", out.HitReceivedSlowDownMaxDuration);
    Read(value, "HitReceivedSlowDownFalloffDuration", out.HitReceivedSlowDownFalloffDuration);
}

void ReadFields(const Value& value, Quantum::KccModifierSpec& out, ReaderTag) {
    Read(value, "ModifierType", out.ModifierType);
    Read(value, "ModType", out.ModType);
    Read(value, "Value", out.Value);
    Read(value, "Duration", out.Duration);
    Read(value, "OverwriteExisting", out.OverwriteExisting);
    Read(value, "SurfaceType", out.SurfaceType);
}

void ReadFields(const Value& value, Quantum::LedgeGrabbingSpec& out, ReaderTag) {
    Read(value, "AuthorizedActorStatusTypes", out.AuthorizedActorStatusTypes);
    Read(value, "ForwardDistance", out.ForwardDistance);
    Read(value, "DownwardDistance", out.DownwardDistance);
    Read(value, "HeightOffset", out.HeightOffset);
    Read(value, "LayerMask", out.LayerMask);
    Read(value, "ForwardMaxAngle", out.ForwardMaxAngle);
    Read(value, "LedgeMinHeight", out.LedgeMinHeight);
    Read(value, "SchedulePeriod", out.SchedulePeriod);
    Read(value, "VoidMinHeight", out.VoidMinHeight);
    Read(value, "LedgeMinThickness", out.LedgeMinThickness);
    Read(value, "LedgeHitNormalMaxAngle", out.LedgeHitNormalMaxAngle);
    Read(value, "VoidTestOffsetY", out.VoidTestOffsetY);
    Read(value, "snapPositionCurveXZ", out.snapPositionCurveXZ);
    Read(value, "snapPositionCurveY", out.snapPositionCurveY);
    Read(value, "snapRotationCurve", out.snapRotationCurve);
    Read(value, "snapDuration", out.snapDuration);
    Read(value, "snapPosForwardOffset", out.snapPosForwardOffset);
    Read(value, "snapPosHeightOffset", out.snapPosHeightOffset);
    Read(value, "cameraDistanceOffset", out.cameraDistanceOffset);
    Read(value, "cameraVerticalAngle", out.cameraVerticalAngle);
    Read(value, "debug", out.debug);
}

void ReadFields(const Value&, Quantum::LevelUpActorSpec&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::MeleeAttackSpec& out, ReaderTag) {
    Read(value, "attackDuration", out.attackDuration);
    Read(value, "areaOfEffect", out.areaOfEffect);
    Read(value, "aoeSpawnOffset", out.aoeSpawnOffset);
}

void ReadFields(const Value& value, Quantum::MeleeJumpAttackSpec& out, ReaderTag) {
    Read(value, "GroundSlamAoEPrototypeAsset", out.GroundSlamAoEPrototypeAsset);
    Read(value, "CooldownDuration", out.CooldownDuration);
    Read(value, "InputHoldMinDuration", out.InputHoldMinDuration);
    Read(value, "DownImpulseStrengh", out.DownImpulseStrengh);
    Read(value, "MaxFallSpeedModifier", out.MaxFallSpeedModifier);
    Read(value, "UpImpulseStrength", out.UpImpulseStrength);
    Read(value, "UpImpulseDuration", out.UpImpulseDuration);
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
    Read(value, "MoveSpeedModifierFallOf", out.MoveSpeedModifierFallOf);
    Read(value, "OnFinishMoveSpeedModifier", out.OnFinishMoveSpeedModifier);
    Read(value, "OnFinishMoveSpeedModifierDuration", out.OnFinishMoveSpeedModifierDuration);
    Read(value, "OnFinishMoveSpeedModifierFallOf", out.OnFinishMoveSpeedModifierFallOf);
    Read(value, "SwitchingBackToPreviousWeapon", out.SwitchingBackToPreviousWeapon);
    Read(value, "RaycastLength", out.RaycastLength);
    Read(value, "BallonBounceMultiplier", out.BallonBounceMultiplier);
}

void ReadFields(const Value& value, Quantum::PreciseHitReceiverSpec& out, ReaderTag) {
    Read(value, "CriticalHitDistanceToTarget", out.CriticalHitDistanceToTarget);
    Read(value, "HeadshotDistanceToTarget", out.HeadshotDistanceToTarget);
    Read(value, "HeadshotBonusMultiplier", out.HeadshotBonusMultiplier);
}

void ReadFields(const Value& value, Quantum::PushableSpec& out, ReaderTag) {
    Read(value, "PushFilters", out.PushFilters);
    Read(value, "PrePhysicsPushMultiplier", out.PrePhysicsPushMultiplier);
    Read(value, "UsePushStop", out.UsePushStop);
    Read(value, "PushStopDurationSeconds", out.PushStopDurationSeconds);
    Read(value, "DisplayPushDamage", out.DisplayPushDamage);
    Read(value, "LockJumpOnPush", out.LockJumpOnPush);
    Read(value, "LockJumpThreshold", out.LockJumpThreshold);
    Read(value, "LockJumpDuration", out.LockJumpDuration);
    Read(value, "CustomPushOnShieldBreak", out.CustomPushOnShieldBreak);
    Read(value, "PushOnShieldBreakForce", out.PushOnShieldBreakForce);
    Read(value, "PushOnShieldBreakUpwardForce", out.PushOnShieldBreakUpwardForce);
    Read(value, "PushOnDeath", out.PushOnDeath);
    Read(value, "PushOnDeathForce", out.PushOnDeathForce);
    Read(value, "PushOnDeathUpwardForce", out.PushOnDeathUpwardForce);
    Read(value, "PushStopOnDeath", out.PushStopOnDeath);
    Read(value, "PushStopOnDeathDurationSeconds", out.PushStopOnDeathDurationSeconds);
}

void ReadFields(const Value& value, Quantum::RewardReceiverSpec& out, ReaderTag) {
    Read(value, "rewardDelay", out.rewardDelay);
    Read(value, "rivalMultiplier", out.rivalMultiplier);
}

void ReadFields(const Value& value, Quantum::RollSpec& out, ReaderTag) {
    Read(value, "clampVerticalVelocity", out.clampVerticalVelocity);
    Read(value, "verticalMaxVelocity", out.verticalMaxVelocity);
    Read(value, "horizontalFixVelocity", out.horizontalFixVelocity);
    Read(value, "angularFixVelocity", out.angularFixVelocity);
    Read(value, "maxBounce", out.maxBounce);
    Read(value, "disableBounceOnPlayers", out.disableBounceOnPlayers);
    Read(value, "killOnPlayerCollision", out.killOnPlayerCollision);
    Read(value, "killOnCombatParticipantCollision", out.killOnCombatParticipantCollision);
    Read(value, "alsoAdjustForwardOnBounce", out.alsoAdjustForwardOnBounce);
    Read(value, "depenetrationOverlapLayerMask", out.depenetrationOverlapLayerMask);
}

void ReadFields(const Value& value, Quantum::ShieldedActorSpec& out, ReaderTag) {
    Read(value, "Mode", out.Mode);
    Read(value, "shieldPointsMin", out.shieldPointsMin);
    Read(value, "shieldPointsMax", out.shieldPointsMax);
    Read(value, "moveSpeedModifierWhenHit", out.moveSpeedModifierWhenHit);
    Read(value, "moveSpeedModifierTickRate", out.moveSpeedModifierTickRate);
    Read(value, "moveSpeedModifierWhenHitDuration", out.moveSpeedModifierWhenHitDuration);
    Read(value, "moveSpeedModifierWhenBreak", out.moveSpeedModifierWhenBreak);
    Read(value, "moveSpeedModifierWhenBreakDuration", out.moveSpeedModifierWhenBreakDuration);
    Read(value, "timeWithoutDamageBeforeStartRegen", out.timeWithoutDamageBeforeStartRegen);
    Read(value, "timeToRegenerate", out.timeToRegenerate);
}

void ReadFields(const Value& value, Quantum::SweepAttackSpec& out, ReaderTag) {
    Read(value, "PrepareDurationSeconds", out.PrepareDurationSeconds);
    Read(value, "SweepDurationSeconds", out.SweepDurationSeconds);
    Read(value, "SweepAngleDegrees", out.SweepAngleDegrees);
    Read(value, "FinishedDurationSeconds", out.FinishedDurationSeconds);
}

void ReadFields(const Value& value, Quantum::WallRunningSpec& out, ReaderTag) {
    Read(value, "Acceleration", out.Acceleration);
    Read(value, "Deceleration", out.Deceleration);
    Read(value, "Speed", out.Speed);
    Read(value, "PerpendicularThreshold", out.PerpendicularThreshold);
    Read(value, "JumpSpeed", out.JumpSpeed);
    Read(value, "ExitImpulse", out.ExitImpulse);
    Read(value, "HeightCorrectionCentroidOffset", out.HeightCorrectionCentroidOffset);
    Read(value, "HeightCorrectionInterpolationSpeed", out.HeightCorrectionInterpolationSpeed);
    Read(value, "SnapOffset", out.SnapOffset);
}

void ReadFields(const Value& value, Quantum::ActorIcedSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ActorPhysicsSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "useHeroSpecCharacterController3DConfig", out.useHeroSpecCharacterController3DConfig);
    Read(value, "kccConfig", out.kccConfig);
    Read(value, "kccExtensionConfig", out.kccExtensionConfig);
}

void ReadFields(const Value& value, Quantum::ActorSheepedSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ActorStunnedSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AmoeboidLevelUpActorSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::LevelUpActorSpec&>(out), ReaderTag{});
    Read(value, "NextLevelGroupPrototype", out.NextLevelGroupPrototype);
}

void ReadFields(const Value& value, Quantum::DashActorConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "validInputThreshold", out.validInputThreshold);
    Read(value, "frontThreshold", out.frontThreshold);
    Read(value, "force", out.force);
    Read(value, "duration", out.duration);
    Read(value, "inputFalloff", out.inputFalloff);
    Read(value, "cooldown", out.cooldown);
    Read(value, "initialYVelocity", out.initialYVelocity);
    Read(value, "gravityModifier", out.gravityModifier);
    Read(value, "pushForceReductionFactor", out.pushForceReductionFactor);
    Read(value, "moveSpeedModifier", out.moveSpeedModifier);
    Read(value, "moveSpeedModifierDuration", out.moveSpeedModifierDuration);
    Read(value, "moveSpeedModifierFalloffDuration", out.moveSpeedModifierFalloffDuration);
    Read(value, "stunHitPlayers", out.stunHitPlayers);
    Read(value, "stunDuration", out.stunDuration);
    Read(value, "damageSpec", out.damageSpec);
    Read(value, "pusherSpec", out.pusherSpec);
}

void ReadFields(const Value& value, Quantum::GrinderActorConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "grindSpeedMin", out.grindSpeedMin);
    Read(value, "grindSpeedMax", out.grindSpeedMax);
    Read(value, "accelerationTime", out.accelerationTime);
    Read(value, "accelerationDecaySpeed", out.accelerationDecaySpeed);
    Read(value, "exitGrindImpulse", out.exitGrindImpulse);
    Read(value, "timeToSnap", out.timeToSnap);
    Read(value, "snapHeightFactor", out.snapHeightFactor);
    Read(value, "xInputThreshold", out.xInputThreshold);
    Read(value, "perpendicularThreshold", out.perpendicularThreshold);
    Read(value, "switchCooldown", out.switchCooldown);
    Read(value, "SFX_Land", out.SFX_Land);
    Read(value, "snapHeightCurve", out.snapHeightCurve);
}

void ReadFields(const Value& value, Quantum::GroundDashActorConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "gravityModValue", out.gravityModValue);
    Read(value, "maxFallSpeedModValue", out.maxFallSpeedModValue);
    Read(value, "startingDuration", out.startingDuration);
    Read(value, "modifiersOrigin", out.modifiersOrigin);
    Read(value, "aoe", out.aoe);
}

void ReadFields(const Value& value, Quantum::HelipackActorSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "minWorldHeight", out.minWorldHeight);
    Read(value, "minGroundDistanceToStart", out.minGroundDistanceToStart);
    Read(value, "delayToActivate", out.delayToActivate);
    Read(value, "gravityModifier", out.gravityModifier);
    Read(value, "maxFallSpeedModifier", out.maxFallSpeedModifier);
    Read(value, "accelerationModifier", out.accelerationModifier);
    Read(value, "maxSpeedModifier", out.maxSpeedModifier);
    Read(value, "maxDuration", out.maxDuration);
    Read(value, "impulseOnActivation", out.impulseOnActivation);
    Read(value, "modifierTransitionDuration", out.modifierTransitionDuration);
}

void ReadFields(const Value& value, Quantum::UltimateAbilityUserSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::WeaponLoadoutSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
    Read(value, "fallbackLoadout", out.fallbackLoadout);
    Read(value, "delayToSwitch", out.delayToSwitch);
    Read(value, "delayOnEquipped", out.delayOnEquipped);
}

void ReadFields(const Value& value, Quantum::WeaponUserSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ActorComponentSpecBase&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::GadgetSpec& out, ReaderTag) {
    Read(value, "AllowedInputActionDuringUse", out.AllowedInputActionDuringUse);
    Read(value, "cooldown", out.cooldown);
    Read(value, "minCooldown", out.minCooldown);
    Read(value, "useDuration", out.useDuration);
    Read(value, "duration", out.duration);
    Read(value, "failSafeTimeOut", out.failSafeTimeOut);
    Read(value, "ignorePushWhileInUse", out.ignorePushWhileInUse);
    Read(value, "isToggleable", out.isToggleable);
    Read(value, "resetReloadOnActivation", out.resetReloadOnActivation);
    Read(value, "levelUpModifiers", out.levelUpModifiers);
}

void ReadFields(const Value& value, Quantum::SimulationGadgetTable& out, ReaderTag) {
    Read(value, "defaultGadgetSpec", out.defaultGadgetSpec);
    Read(value, "definitions", out.definitions);
}

void ReadFields(const Value& value, Quantum::SimulationGadgetTable::GadgetDefinition& out, ReaderTag) {
    Read(value, "id", out.id);
    Read(value, "weaponId", out.weaponId);
    Read(value, "characterId", out.characterId);
    Read(value, "gadgetSpec", out.gadgetSpec);
}

void ReadFields(const Value& value, Quantum::SimulationMeleeWeaponsTable& out, ReaderTag) {
    Read(value, "defaultMeleeWeapon", out.defaultMeleeWeapon);
    Read(value, "definitions", out.definitions);
}

void ReadFields(const Value& value, Quantum::SimulationMeleeWeaponsTable::MeleeWeaponDefinition& out, ReaderTag) {
    Read(value, "characterId", out.characterId);
    Read(value, "id", out.id);
}

void ReadFields(const Value& value, Quantum::SimulationUltimateTable& out, ReaderTag) {
    Read(value, "defaultUltimateSpec", out.defaultUltimateSpec);
    Read(value, "definitions", out.definitions);
}

void ReadFields(const Value& value, Quantum::SimulationUltimateTable::UltimateDefinition& out, ReaderTag) {
    Read(value, "id", out.id);
    Read(value, "characterId", out.characterId);
    Read(value, "ultimateAbilitySpec", out.ultimateAbilitySpec);
}

void ReadFields(const Value& value, Quantum::SimulationWeaponTable& out, ReaderTag) {
    Read(value, "botsBackpackToWeaponLevelCurve", out.botsBackpackToWeaponLevelCurve);
    Read(value, "botsWeaponLevelRandomRangePercent", out.botsWeaponLevelRandomRangePercent);
    Read(value, "botBanlist", out.botBanlist);
    Read(value, "entries", out.entries);
}

void ReadFields(const Value& value, Quantum::SimulationWeaponTable::Entry& out, ReaderTag) {
    Read(value, "uniqueID", out.uniqueID);
    Read(value, "entityPrototype", out.entityPrototype);
    Read(value, "weaponSpecRef", out.weaponSpecRef);
}

void ReadFields(const Value& value, Quantum::UltimateAbilitySpec& out, ReaderTag) {
    Read(value, "hasDuration", out.hasDuration);
    Read(value, "duration", out.duration);
    Read(value, "finishUltimateOnRelease", out.finishUltimateOnRelease);
    Read(value, "alwaysDisplayProgressLabel", out.alwaysDisplayProgressLabel);
    Read(value, "AllowedInputActionDuringUse", out.AllowedInputActionDuringUse);
    Read(value, "canDashDuringUse", out.canDashDuringUse);
    Read(value, "canJumpDuringUse", out.canJumpDuringUse);
    Read(value, "canUseGadgetDuringUse", out.canUseGadgetDuringUse);
    Read(value, "canRecast", out.canRecast);
    Read(value, "recastDelay", out.recastDelay);
    Read(value, "fullCharge", out.fullCharge);
    Read(value, "resetChargeOnFinishUltimate", out.resetChargeOnFinishUltimate);
    Read(value, "chargeRatePerBolt", out.chargeRatePerBolt);
    Read(value, "chargeRatePerSecond", out.chargeRatePerSecond);
    Read(value, "chargeRatePerHit", out.chargeRatePerHit);
    Read(value, "hitChargeCooldown", out.hitChargeCooldown);
    Read(value, "chargeRatePerPushReceived", out.chargeRatePerPushReceived);
    Read(value, "chargeRatePerDamageDealt", out.chargeRatePerDamageDealt);
    Read(value, "chargeRatePerDamageReceived", out.chargeRatePerDamageReceived);
    Read(value, "chargeRatePerRespawn", out.chargeRatePerRespawn);
    Read(value, "mustBeFullyCharged", out.mustBeFullyCharged);
    Read(value, "minChargeToActivate", out.minChargeToActivate);
    Read(value, "toggleMode", out.toggleMode);
    Read(value, "startInvulnerable", out.startInvulnerable);
    Read(value, "invulnerabilityDuration", out.invulnerabilityDuration);
    Read(value, "invulnerableFilter", out.invulnerableFilter);
    Read(value, "addMoveSpeedModifier", out.addMoveSpeedModifier);
    Read(value, "moveSpeedModifierOnUse", out.moveSpeedModifierOnUse);
    Read(value, "addOverhealth", out.addOverhealth);
    Read(value, "overhealthSpec", out.overhealthSpec);
    Read(value, "levelUpModifiers", out.levelUpModifiers);
    Read(value, "TimeBetweenTwoUsesInSecs", out.TimeBetweenTwoUsesInSecs);
    Read(value, "useUltimateStacks", out.useUltimateStacks);
    Read(value, "maxStacks", out.maxStacks);
    Read(value, "spawnEntityOnActivate", out.spawnEntityOnActivate);
    Read(value, "entityToSpawnOnActivate", out.entityToSpawnOnActivate);
    Read(value, "disableShootByDuration", out.disableShootByDuration);
    Read(value, "disableShootNormedDuration", out.disableShootNormedDuration);
}

void ReadFields(const Value& value, Quantum::WeaponBehaviour_ChargingSpec& out, ReaderTag) {
    Read(value, "UpdateCurrentBurstWithCharge", out.UpdateCurrentBurstWithCharge);
    Read(value, "FullChargeDuration", out.FullChargeDuration);
    Read(value, "NumberOfChargeLevel", out.NumberOfChargeLevel);
    Read(value, "AutoReleaseOnFullCharge", out.AutoReleaseOnFullCharge);
}

void ReadFields(const Value& value, Quantum::WeaponSpec& out, ReaderTag) {
    Read(value, "DamageType", out.DamageType);
    Read(value, "LoadoutSlotType", out.LoadoutSlotType);
    Read(value, "DisableFireButtonDuringUltimate", out.DisableFireButtonDuringUltimate);
    Read(value, "ConsumesUltimateCharges", out.ConsumesUltimateCharges);
    Read(value, "ConsumptionPerShot", out.ConsumptionPerShot);
    Read(value, "WeaponType", out.WeaponType);
    Read(value, "StartUseType", out.StartUseType);
    Read(value, "HasInputBuffering", out.HasInputBuffering);
    Read(value, "InputBufferingDuration", out.InputBufferingDuration);
    Read(value, "HasMinDelayBeforeRelease", out.HasMinDelayBeforeRelease);
    Read(value, "MinFireDelayBeforeRelease", out.MinFireDelayBeforeRelease);
    Read(value, "AutoReleaseAfterDelay", out.AutoReleaseAfterDelay);
    Read(value, "DisplayFireButton", out.DisplayFireButton);
    Read(value, "StopUseType", out.StopUseType);
    Read(value, "Tags", out.Tags);
    Read(value, "weaponFilterType", out.weaponFilterType);
    Read(value, "UseMeleeInput", out.UseMeleeInput);
    Read(value, "CanUseTriangleAimAssist", out.CanUseTriangleAimAssist);
    Read(value, "CanHaveMultipleAmmoInstance", out.CanHaveMultipleAmmoInstance);
    Read(value, "IsFiringWhileAmmoIsAlive", out.IsFiringWhileAmmoIsAlive);
    Read(value, "EffectOffset", out.EffectOffset);
    Read(value, "EffectScale", out.EffectScale);
    Read(value, "EffectHasDuration", out.EffectHasDuration);
    Read(value, "AmmunitionAsset", out.AmmunitionAsset);
    Read(value, "DefaultSpawnOffset", out.DefaultSpawnOffset);
    Read(value, "AutoCycleMultiShotConfigs", out.AutoCycleMultiShotConfigs);
    Read(value, "MultiShotConfigs", out.MultiShotConfigs);
    Read(value, "ConsumeOneAmmoPerMultiShot", out.ConsumeOneAmmoPerMultiShot);
    Read(value, "UseCustomDelayOnEquipped", out.UseCustomDelayOnEquipped);
    Read(value, "DelayOnEquipped", out.DelayOnEquipped);
    Read(value, "FireRate", out.FireRate);
    Read(value, "AmmoPerBurst", out.AmmoPerBurst);
    Read(value, "BurstFireRate", out.BurstFireRate);
    Read(value, "BurstCanBeInterrupted", out.BurstCanBeInterrupted);
    Read(value, "BurstInterruptionWindowDuration", out.BurstInterruptionWindowDuration);
    Read(value, "Range", out.Range);
    Read(value, "OverShootDistance", out.OverShootDistance);
    Read(value, "BoltCost", out.BoltCost);
    Read(value, "SpreadRadius", out.SpreadRadius);
    Read(value, "DisableStickyFireInput", out.DisableStickyFireInput);
    Read(value, "CanDealCriticalHits", out.CanDealCriticalHits);
    Read(value, "UseCooldown", out.UseCooldown);
    Read(value, "CooldownDuration", out.CooldownDuration);
    Read(value, "CooldownFeedbackOnReticle", out.CooldownFeedbackOnReticle);
    Read(value, "UseReload", out.UseReload);
    Read(value, "ReloadDurationSeconds", out.ReloadDurationSeconds);
    Read(value, "InfiniteAmmo", out.InfiniteAmmo);
    Read(value, "UseAutoLoad", out.UseAutoLoad);
    Read(value, "AutoLoadDelaySeconds", out.AutoLoadDelaySeconds);
    Read(value, "OverrideDurationForAutoReload", out.OverrideDurationForAutoReload);
    Read(value, "AutoLoadDurationSeconds", out.AutoLoadDurationSeconds);
    Read(value, "MaxAmmo", out.MaxAmmo);
    Read(value, "AmmoBurnPerShot", out.AmmoBurnPerShot);
    Read(value, "ConsumeAmmoOnDestroy", out.ConsumeAmmoOnDestroy);
    Read(value, "PickupMaxAmmo", out.PickupMaxAmmo);
    Read(value, "SetKccLookTowardsCamera", out.SetKccLookTowardsCamera);
    Read(value, "SetKccLookTowardsCameraSight", out.SetKccLookTowardsCameraSight);
    Read(value, "MoveSpeedModType", out.MoveSpeedModType);
    Read(value, "MoveSpeedModFactorOnUse", out.MoveSpeedModFactorOnUse);
    Read(value, "MoveSpeedModHasCustomDuration", out.MoveSpeedModHasCustomDuration);
    Read(value, "MoveSpeedModDuration", out.MoveSpeedModDuration);
    Read(value, "CanHeadshot", out.CanHeadshot);
    Read(value, "UseOwnerTransformForDefaultLookDir", out.UseOwnerTransformForDefaultLookDir);
    Read(value, "UseRaycastForLookAtPoint", out.UseRaycastForLookAtPoint);
    Read(value, "UseRangeForDefaultLookPoint", out.UseRangeForDefaultLookPoint);
    Read(value, "UseCurrentTargetForDefaultLookPoint", out.UseCurrentTargetForDefaultLookPoint);
    Read(value, "OrientWeaponToTarget", out.OrientWeaponToTarget);
    Read(value, "UseBotCustomTargetOnly", out.UseBotCustomTargetOnly);
    Read(value, "CanAutoShoot", out.CanAutoShoot);
    Read(value, "SnapLookAtPointToGround", out.SnapLookAtPointToGround);
    Read(value, "SnapRaycastDistance", out.SnapRaycastDistance);
    Read(value, "OverrideTargetFinder", out.OverrideTargetFinder);
    Read(value, "TargetFinderSpecOverride", out.TargetFinderSpecOverride);
    Read(value, "levelUpModifiers", out.levelUpModifiers);
    Read(value, "enhancementSpecs", out.enhancementSpecs);
    Read(value, "AvailableInBucketIndex", out.AvailableInBucketIndex);
    Read(value, "BotsMinLongPressTime", out.BotsMinLongPressTime);
    Read(value, "BotsMaxLongPressTime", out.BotsMaxLongPressTime);
}

void ReadFields(const Value& value, Quantum::WeaponSpec::MultiShotConfig& out, ReaderTag) {
    Read(value, "spawnOffsets", out.spawnOffsets);
}

void ReadFields(const Value& value, Quantum::AddForceOnUltimateSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::UltimateAbilitySpec&>(out), ReaderTag{});
    Read(value, "Type", out.Type);
    Read(value, "Strength", out.Strength);
    Read(value, "ImpulseMaxSpeed", out.ImpulseMaxSpeed);
    Read(value, "ImpulseMaxVerticalSpeed", out.ImpulseMaxVerticalSpeed);
    Read(value, "Direction", out.Direction);
    Read(value, "DirectionIsLocal", out.DirectionIsLocal);
    Read(value, "Delay", out.Delay);
    Read(value, "LerpStrengthWithDuration", out.LerpStrengthWithDuration);
    Read(value, "LerpPower", out.LerpPower);
    Read(value, "ImpulseDuration", out.ImpulseDuration);
    Read(value, "FallSpeedDampening", out.FallSpeedDampening);
}

void ReadFields(const Value& value, Quantum::DrillDashGadgetSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GadgetSpec&>(out), ReaderTag{});
    Read(value, "AnticipationDuration", out.AnticipationDuration);
    Read(value, "AnticipationMoveSpeedModifier", out.AnticipationMoveSpeedModifier);
    Read(value, "FreezeOnAnticipation", out.FreezeOnAnticipation);
    Read(value, "DashDuration", out.DashDuration);
    Read(value, "DashPower", out.DashPower);
    Read(value, "DashInputFalloff", out.DashInputFalloff);
    Read(value, "DashMaxDistance", out.DashMaxDistance);
    Read(value, "DetectionShapeConfig", out.DetectionShapeConfig);
    Read(value, "DetectionShapeOverlapOffset", out.DetectionShapeOverlapOffset);
    Read(value, "DetectionLayerMask", out.DetectionLayerMask);
    Read(value, "DetectionQueryOptions", out.DetectionQueryOptions);
    Read(value, "DetectionDamageSpec", out.DetectionDamageSpec);
    Read(value, "DetectionPusherSpec", out.DetectionPusherSpec);
    Read(value, "StopEntityInfoFilter", out.StopEntityInfoFilter);
    Read(value, "AttackDuration", out.AttackDuration);
    Read(value, "AttackAoePrototypeAsset", out.AttackAoePrototypeAsset);
    Read(value, "LastHitDuration", out.LastHitDuration);
    Read(value, "LastHitAoePrototypeAsset", out.LastHitAoePrototypeAsset);
    Read(value, "RecoveryDuration", out.RecoveryDuration);
    Read(value, "RecoveryMoveSpeedModifier", out.RecoveryMoveSpeedModifier);
    Read(value, "GravityModifier", out.GravityModifier);
    Read(value, "TargetFinderAsset", out.TargetFinderAsset);
}

void ReadFields(const Value& value, Quantum::GadgetWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GadgetSpec&>(out), ReaderTag{});
    Read(value, "weaponID", out.weaponID);
}

void ReadFields(const Value& value, Quantum::GatlingWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "Levels", out.Levels);
}

void ReadFields(const Value& value, Quantum::GatlingWeaponSpec::GatlingLevel& out, ReaderTag) {
    Read(value, "SpinUpTime", out.SpinUpTime);
    Read(value, "FireRateMultiplier", out.FireRateMultiplier);
    Read(value, "IsProgressive", out.IsProgressive);
}

void ReadFields(const Value& value, Quantum::GloveOfDoomWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "DoomGroupMaxMembers", out.DoomGroupMaxMembers);
    Read(value, "AutoshootDelayForRespawningLastMember", out.AutoshootDelayForRespawningLastMember);
}

void ReadFields(const Value& value, Quantum::HoloshieldGloveWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "ShieldMaxMembers", out.ShieldMaxMembers);
    Read(value, "AutoShootMaxMembers", out.AutoShootMaxMembers);
    Read(value, "AutoshootDelayForRespawningLastMember", out.AutoshootDelayForRespawningLastMember);
}

void ReadFields(const Value& value, Quantum::HoverBootsWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "StopUseTransitionDuration", out.StopUseTransitionDuration);
    Read(value, "ForceInputDuration", out.ForceInputDuration);
    Read(value, "TargetMaxSpeedModifier", out.TargetMaxSpeedModifier);
    Read(value, "ReachMaxSpeedDuration", out.ReachMaxSpeedDuration);
    Read(value, "MovementReactivity", out.MovementReactivity);
    Read(value, "GravityModifier", out.GravityModifier);
    Read(value, "HitWallDynamicLayersToIgnore", out.HitWallDynamicLayersToIgnore);
    Read(value, "HitWallStunDuration", out.HitWallStunDuration);
    Read(value, "HitWallMaxBounceForce", out.HitWallMaxBounceForce);
    Read(value, "HitWallMaxAngleDegrees", out.HitWallMaxAngleDegrees);
    Read(value, "ForwardRange", out.ForwardRange);
    Read(value, "StraffStrength", out.StraffStrength);
}

void ReadFields(const Value& value, Quantum::NegatronWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "LeveledMaxBeamDurationSeconds", out.LeveledMaxBeamDurationSeconds);
}

void ReadFields(const Value& value, Quantum::RynoWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "MaxLockedTargets", out.MaxLockedTargets);
    Read(value, "TimeToAcquireTarget", out.TimeToAcquireTarget);
    Read(value, "LeveledMaxMissilesPerFire", out.LeveledMaxMissilesPerFire);
    Read(value, "MissileFireRate", out.MissileFireRate);
    Read(value, "AutoFireDelaySeconds", out.AutoFireDelaySeconds);
}

void ReadFields(const Value& value, Quantum::TeslaClawWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "TargetFinderSpecWhenShooting", out.TargetFinderSpecWhenShooting);
    Read(value, "FireCastLayerMask", out.FireCastLayerMask);
}

void ReadFields(const Value& value, Quantum::UltimateEvolutionSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::UltimateAbilitySpec&>(out), ReaderTag{});
    Read(value, "AoESpawnDelay", out.AoESpawnDelay);
    Read(value, "AoEPrototypeAssetRef", out.AoEPrototypeAssetRef);
    Read(value, "NumberOfEvolutionLevel", out.NumberOfEvolutionLevel);
    Read(value, "EvolutionStats", out.EvolutionStats);
}

void ReadFields(const Value& value, Quantum::UltimateEvolutionSpec::EvolutionStat& out, ReaderTag) {
    Read(value, "Scale", out.Scale);
    Read(value, "HealthPointMultiplier", out.HealthPointMultiplier);
}

void ReadFields(const Value& value, Quantum::UltimateSlamSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::UltimateAbilitySpec&>(out), ReaderTag{});
    Read(value, "TargetFinderAsset", out.TargetFinderAsset);
    Read(value, "ParabolaFollowerAsset", out.ParabolaFollowerAsset);
    Read(value, "SlamAoEPrototypeAsset", out.SlamAoEPrototypeAsset);
    Read(value, "SpawnReasonSlamAoE", out.SpawnReasonSlamAoE);
    Read(value, "HorizontalRange", out.HorizontalRange);
    Read(value, "MaxHeightDiff", out.MaxHeightDiff);
    Read(value, "LayerMask", out.LayerMask);
    Read(value, "QueryOptions", out.QueryOptions);
    Read(value, "SlamOffset", out.SlamOffset);
    Read(value, "MaxVerticalRaycastCount", out.MaxVerticalRaycastCount);
    Read(value, "VerticalRaycastLength", out.VerticalRaycastLength);
    Read(value, "Debug", out.Debug);
    Read(value, "FallbackSphereRadius", out.FallbackSphereRadius);
}

void ReadFields(const Value& value, Quantum::UltimateTankSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::UltimateAbilitySpec&>(out), ReaderTag{});
    Read(value, "tankPrototypeAssetRef", out.tankPrototypeAssetRef);
    Read(value, "weaponID", out.weaponID);
    Read(value, "weaponPositionOffset", out.weaponPositionOffset);
    Read(value, "lifetime", out.lifetime);
    Read(value, "healthDecreaseTickRate", out.healthDecreaseTickRate);
}

void ReadFields(const Value& value, Quantum::UltimateWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::UltimateAbilitySpec&>(out), ReaderTag{});
    Read(value, "weaponID", out.weaponID);
    Read(value, "finishUltimateOnWeaponUse", out.finishUltimateOnWeaponUse);
    Read(value, "finishUltimateOnWeaponUseDelay", out.finishUltimateOnWeaponUseDelay);
    Read(value, "UpgradePerLevel", out.UpgradePerLevel);
}

void ReadFields(const Value& value, Quantum::UltimateWeaponSpec::StatUpgrade& out, ReaderTag) {
    Read(value, "modificationType", out.modificationType);
    Read(value, "type", out.type);
    Read(value, "valuel", out.valuel);
}

void ReadFields(const Value& value, Quantum::UltimateWeaponSpec::Upgrade& out, ReaderTag) {
    Read(value, "statUpgrades", out.statUpgrades);
}

void ReadFields(const Value& value, Quantum::VoidRepulserWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::WeaponSpecDefault& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::WrenchWeaponSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponSpec&>(out), ReaderTag{});
    Read(value, "OrientationSharpness", out.OrientationSharpness);
    Read(value, "MaxSpeedModifierHit", out.MaxSpeedModifierHit);
    Read(value, "InputFalloff", out.InputFalloff);
    Read(value, "MinInputSqrMagForRegistration", out.MinInputSqrMagForRegistration);
    Read(value, "MaxTargetAngleCorrection", out.MaxTargetAngleCorrection);
    Read(value, "ImpulseMultiplierFromDistanceToTarget", out.ImpulseMultiplierFromDistanceToTarget);
    Read(value, "ComboAttacks", out.ComboAttacks);
    Read(value, "EnableCliffDetection", out.EnableCliffDetection);
    Read(value, "CliffDetectionDistance", out.CliffDetectionDistance);
    Read(value, "CliffDetectionHeight", out.CliffDetectionHeight);
    Read(value, "CliffDistanceMargin", out.CliffDistanceMargin);
}

void ReadFields(const Value& value, Quantum::SmokeScreenSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::AddForceOnUltimateSpec&>(out), ReaderTag{});
}

void ReadFields(const Value&, Quantum::EnhancementSpec&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::ImprovementSpec& out, ReaderTag) {
    Read(value, "DamageModifiers", out.DamageModifiers);
}

void ReadFields(const Value& value, Quantum::CompoundHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "Improvements", out.Improvements);
}

void ReadFields(const Value& value, Quantum::DamageMultiplierHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "DamageMultiplierSpec", out.DamageMultiplierSpec);
}

void ReadFields(const Value& value, Quantum::DashCooldownReductionHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "DashCooldownReduction", out.DashCooldownReduction);
}

void ReadFields(const Value& value, Quantum::ElementalBuildUpMultiplierHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "AddElectricalBuildUpMultiplier", out.AddElectricalBuildUpMultiplier);
    Read(value, "ElectricalBuildUpMultiplier", out.ElectricalBuildUpMultiplier);
    Read(value, "AddIceBuildUpMultiplier", out.AddIceBuildUpMultiplier);
    Read(value, "IceBuildUpMultiplier", out.IceBuildUpMultiplier);
}

void ReadFields(const Value& value, Quantum::HeroEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::EnhancementSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::JumpAttackHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "MeleeJumpAttackSpec", out.MeleeJumpAttackSpec);
}

void ReadFields(const Value& value, Quantum::JumpMultiplierHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "BaseJumpMultiplier", out.BaseJumpMultiplier);
}

void ReadFields(const Value& value, Quantum::OnKillCallbacksHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "OnPlayerKilled", out.OnPlayerKilled);
}

void ReadFields(const Value& value, Quantum::PlaceholderHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::ReplaceHealthSpecHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "HealthSpec", out.HealthSpec);
}

void ReadFields(const Value& value, Quantum::WeaponEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::EnhancementSpec&>(out), ReaderTag{});
    Read(value, "DamageModifiers", out.DamageModifiers);
    Read(value, "ApplyTo", out.ApplyTo);
    Read(value, "Weapon_ID", out.Weapon_ID);
    Read(value, "Slot", out.Slot);
}

void ReadFields(const Value& value, Quantum::WeaponStatsModifierHeroImprovementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::ImprovementSpec&>(out), ReaderTag{});
    Read(value, "WeaponStatsModifiers", out.WeaponStatsModifiers);
    Read(value, "UseFilter", out.UseFilter);
    Read(value, "WeaponFilter", out.WeaponFilter);
    Read(value, "IncludeUltimates", out.IncludeUltimates);
}

void ReadFields(const Value& value, Quantum::AddDamageIncreaseOnTargetedEffectSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::AddEntitiesToSpawnOnProjectileDeathSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "EntityPrototypesToSpawn", out.EntityPrototypesToSpawn);
}

void ReadFields(const Value& value, Quantum::AmmoMultiplierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "BonusMultiplier", out.BonusMultiplier);
}

void ReadFields(const Value& value, Quantum::AmoeboidLauncherEnhancementPuddleSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AcidPuddleEntityPrototype", out.AcidPuddleEntityPrototype);
}

void ReadFields(const Value& value, Quantum::BlasterMoveSpeedEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
}

void ReadFields(const Value& value, Quantum::BlasterSpecialShotEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "SpecialAmmoAsset", out.SpecialAmmoAsset);
    Read(value, "SpecialShotFrequency", out.SpecialShotFrequency);
}

void ReadFields(const Value& value, Quantum::CompoundEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "Enhancements", out.Enhancements);
}

void ReadFields(const Value& value, Quantum::CooldownMultiplierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "Multiplier", out.Multiplier);
}

void ReadFields(const Value& value, Quantum::DamageIncreasedOnTargetsEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "Duration", out.Duration);
    Read(value, "DamagePercentIncrease", out.DamagePercentIncrease);
}

void ReadFields(const Value& value, Quantum::EnforcerFirstShotBonusEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MoveSpeedBonusValue", out.MoveSpeedBonusValue);
    Read(value, "MoveSpeedBonusDuration", out.MoveSpeedBonusDuration);
}

void ReadFields(const Value& value, Quantum::FirerateMultiplierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "BonusMultiplier", out.BonusMultiplier);
}

void ReadFields(const Value& value, Quantum::GatlingEnhancementResetAmmoOnGadgetUseSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::GatlingEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AdditionalLevel", out.AdditionalLevel);
}

void ReadFields(const Value& value, Quantum::GrenadeLauncherAftershockEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "ProjectileSpawnerSpec", out.ProjectileSpawnerSpec);
}

void ReadFields(const Value& value, Quantum::GrenadeLauncherStunGrenadeEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "PushModifierStatusEffect", out.PushModifierStatusEffect);
    Read(value, "PushModifierStatusEffectDuration", out.PushModifierStatusEffectDuration);
    Read(value, "PushModifierStatusEffectRevertAfterPush", out.PushModifierStatusEffectRevertAfterPush);
}

void ReadFields(const Value& value, Quantum::HeadHunterWallHackEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "ScreenRadiusDetection", out.ScreenRadiusDetection);
}

void ReadFields(const Value& value, Quantum::HealthMultiplierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::HeroEnhancementSpec&>(out), ReaderTag{});
    Read(value, "HealthMultiplier", out.HealthMultiplier);
}

void ReadFields(const Value& value, Quantum::HoloshieldGloveHealZoneEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "HealZoneEntityPrototype", out.HealZoneEntityPrototype);
}

void ReadFields(const Value& value, Quantum::HoverBootsEnhancementCollisionSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "DamageMultiplierSpec", out.DamageMultiplierSpec);
}

void ReadFields(const Value& value, Quantum::IgnoreEntitiesMultiplierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "Multiplier", out.Multiplier);
}

void ReadFields(const Value& value, Quantum::MineLauncherEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MergeDistanceThreshold", out.MergeDistanceThreshold);
    Read(value, "SpawnEntitiesActionOverride", out.SpawnEntitiesActionOverride);
}

void ReadFields(const Value& value, Quantum::MineLauncherProjectileOverrideEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "ProjectileOverride", out.ProjectileOverride);
}

void ReadFields(const Value& value, Quantum::RecoverAmmoOnDashEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AmmoRecoverAmount", out.AmmoRecoverAmount);
}

void ReadFields(const Value& value, Quantum::ReplaceMultiShotConfigsEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MultiShotConfigs", out.MultiShotConfigs);
    Read(value, "consumeOneAmmoPerMultiShot", out.consumeOneAmmoPerMultiShot);
}

void ReadFields(const Value& value, Quantum::ShotgunBerserkerEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "EnhancedAmmoAsset", out.EnhancedAmmoAsset);
}

void ReadFields(const Value& value, Quantum::ShotgunEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "EnhancedAmmoAsset", out.EnhancedAmmoAsset);
}

void ReadFields(const Value& value, Quantum::TeslaClawMoreBeamEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AdditionalBeams", out.AdditionalBeams);
    Read(value, "BeamDamageSpec", out.BeamDamageSpec);
}

void ReadFields(const Value& value, Quantum::VoidRepulserBulldozerEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MoveSpeedBonusDelay", out.MoveSpeedBonusDelay);
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
}

void ReadFields(const Value& value, Quantum::VoidRepulserSpeedBoostEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
    Read(value, "MoveSpeedDuration", out.MoveSpeedDuration);
}

void ReadFields(const Value& value, Quantum::VoidRepulserVoidWaveEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "OnReleaseAmmunitionAsset", out.OnReleaseAmmunitionAsset);
}

void ReadFields(const Value& value, Quantum::WarmongerMiniRocketsEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "EnhancedAmmoAsset", out.EnhancedAmmoAsset);
    Read(value, "AdditionalAmmoCount", out.AdditionalAmmoCount);
}

void ReadFields(const Value& value, Quantum::WeaponChargingStatsModifierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AdditionalChargingStats", out.AdditionalChargingStats);
}

void ReadFields(const Value& value, Quantum::WeaponProjectileSwitcherEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "AlternateAmmunitionAsset", out.AlternateAmmunitionAsset);
    Read(value, "AmmunitionChangeModulo", out.AmmunitionChangeModulo);
    Read(value, "DestroyPreviousAmmoInstanceOnSwitch", out.DestroyPreviousAmmoInstanceOnSwitch);
    Read(value, "SwitchAmmoAfterUseTime", out.SwitchAmmoAfterUseTime);
    Read(value, "SwitchAmmoAfterUseTimeDelay", out.SwitchAmmoAfterUseTimeDelay);
}

void ReadFields(const Value& value, Quantum::WeaponStatsModifierEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "WeaponStatsModifiers", out.WeaponStatsModifiers);
}

void ReadFields(const Value& value, Quantum::WrenchGroundSlamEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "GroundAoePrototype", out.GroundAoePrototype);
}

void ReadFields(const Value& value, Quantum::WrenchThrowEnhancementSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::WeaponEnhancementSpec&>(out), ReaderTag{});
    Read(value, "TargetFinderSpec", out.TargetFinderSpec);
    Read(value, "ThrowableAmmoAsset", out.ThrowableAmmoAsset);
    Read(value, "DefaultAmmoAsset", out.DefaultAmmoAsset);
    Read(value, "HoldInputDelay", out.HoldInputDelay);
}

void ReadFields(const Value& value, Quantum::AIBotSDKConfig& out, ReaderTag) {
    Read(value, "attackPosSearchRadius", out.attackPosSearchRadius);
    Read(value, "attackPosIdealRangeFactor", out.attackPosIdealRangeFactor);
    Read(value, "allowRepositioningDuringAttack", out.allowRepositioningDuringAttack);
    Read(value, "repositioningMinDelay", out.repositioningMinDelay);
    Read(value, "repositioningMaxDelay", out.repositioningMaxDelay);
    Read(value, "safetyMarginForCratesAndCollectables", out.safetyMarginForCratesAndCollectables);
    Read(value, "wanderDangerZoneMargin", out.wanderDangerZoneMargin);
    Read(value, "wanderMinDistZoneRadiusFactor", out.wanderMinDistZoneRadiusFactor);
    Read(value, "wanderDestinationSafetyMargin", out.wanderDestinationSafetyMargin);
    Read(value, "seekTimeoutOnPathNotFound", out.seekTimeoutOnPathNotFound);
    Read(value, "allowRandomJump", out.allowRandomJump);
    Read(value, "randomJumpMinDelay", out.randomJumpMinDelay);
    Read(value, "randomJumpMaxDelay", out.randomJumpMaxDelay);
    Read(value, "shootMissChance", out.shootMissChance);
    Read(value, "shootMissChanceBehindHuman", out.shootMissChanceBehindHuman);
    Read(value, "shootMissAmplitudeHorizontal", out.shootMissAmplitudeHorizontal);
    Read(value, "shootMissAmplitudeVertical", out.shootMissAmplitudeVertical);
    Read(value, "blankShootMinCoef", out.blankShootMinCoef);
    Read(value, "shootCooldownMsMin", out.shootCooldownMsMin);
    Read(value, "shootCooldownMsMax", out.shootCooldownMsMax);
    Read(value, "shootCooldownMsMinForNoMissWeapon", out.shootCooldownMsMinForNoMissWeapon);
    Read(value, "shootCooldownMsMaxForNoMissWeapon", out.shootCooldownMsMaxForNoMissWeapon);
}

void ReadFields(const Value& value, Quantum::AISpawnerConfig& out, ReaderTag) {
    Read(value, "EntityToSpawn", out.EntityToSpawn);
    Read(value, "InitialSpawnDelaySeconds", out.InitialSpawnDelaySeconds);
    Read(value, "SpawnIntervalSeconds", out.SpawnIntervalSeconds);
    Read(value, "UseRandomSpawnTransform", out.UseRandomSpawnTransform);
    Read(value, "SpawnPositionOffsetDistance", out.SpawnPositionOffsetDistance);
    Read(value, "AllowRespawnAfterDeath", out.AllowRespawnAfterDeath);
    Read(value, "AfterDeathSpawnDelaySeconds", out.AfterDeathSpawnDelaySeconds);
}

void ReadFields(const Value& value, Quantum::GameManagerConfig& out, ReaderTag) {
    Read(value, "BotWinStreakRandomCurve", out.BotWinStreakRandomCurve);
    Read(value, "BotWinStreakRandomMinValueIncl", out.BotWinStreakRandomMinValueIncl);
    Read(value, "BotWinStreakRandomMaxValueIncl", out.BotWinStreakRandomMaxValueIncl);
}

void ReadFields(const Value& value, Quantum::GameModeHandlerConfig& out, ReaderTag) {
    Read(value, "referenceDate", out.referenceDate);
    Read(value, "FFAHoursDuration", out.FFAHoursDuration);
    Read(value, "BBHoursDuration", out.BBHoursDuration);
    Read(value, "SSDaysDuration", out.SSDaysDuration);
    Read(value, "SSDaysStart", out.SSDaysStart);
    Read(value, "levelLoadingScreenDuration", out.levelLoadingScreenDuration);
    Read(value, "LevelIntroDuration", out.LevelIntroDuration);
    Read(value, "CountDownDuration", out.CountDownDuration);
    Read(value, "GameOverDuration", out.GameOverDuration);
    Read(value, "GameModeIntroEnabledInEditor", out.GameModeIntroEnabledInEditor);
    Read(value, "ForcedBotsWeapon", out.ForcedBotsWeapon);
}

void ReadFields(const Value& value, Quantum::GameModeHandlerLevelConfig& out, ReaderTag) {
    Read(value, "maxPlayers", out.maxPlayers);
    Read(value, "sceneName", out.sceneName);
    Read(value, "mapRef", out.mapRef);
}

void ReadFields(const Value& value, Quantum::GameModeSpec& out, ReaderTag) {
    Read(value, "id", out.id);
    Read(value, "introFlags", out.introFlags);
    Read(value, "introDuration", out.introDuration);
    Read(value, "gameDuration", out.gameDuration);
    Read(value, "overTimeDuration", out.overTimeDuration);
    Read(value, "outroDuration", out.outroDuration);
    Read(value, "lastManStandingExtraTime", out.lastManStandingExtraTime);
    Read(value, "closeToWinScoreGap", out.closeToWinScoreGap);
    Read(value, "triggerSlowMotionOnlyOnce", out.triggerSlowMotionOnlyOnce);
    Read(value, "closeToWinSlowMotionDuration", out.closeToWinSlowMotionDuration);
    Read(value, "closeToWinSlowMotionModifier", out.closeToWinSlowMotionModifier);
    Read(value, "playerMaxSpeedModifier", out.playerMaxSpeedModifier);
    Read(value, "showPlayerNameOnCanvasMark", out.showPlayerNameOnCanvasMark);
    Read(value, "showBoltAmountOnCanvasMark", out.showBoltAmountOnCanvasMark);
    Read(value, "showKillAmountOnCanvasMark", out.showKillAmountOnCanvasMark);
    Read(value, "showRankOnCanvasMark", out.showRankOnCanvasMark);
    Read(value, "showPermanentOverhealthCount", out.showPermanentOverhealthCount);
    Read(value, "BotPrototype", out.BotPrototype);
    Read(value, "UTMoveAIPrototype", out.UTMoveAIPrototype);
    Read(value, "UTActionAIPrototype", out.UTActionAIPrototype);
    Read(value, "FTUEBotHealthSpec", out.FTUEBotHealthSpec);
    Read(value, "DefaultAggroBehaviour", out.DefaultAggroBehaviour);
    Read(value, "BotAggroBehaviourByIndex", out.BotAggroBehaviourByIndex);
    Read(value, "allowGoldenBolt", out.allowGoldenBolt);
    Read(value, "allowedEntityTypesForMinimap", out.allowedEntityTypesForMinimap);
    Read(value, "goldenBoltConfig", out.goldenBoltConfig);
    Read(value, "boltLossMode", out.boltLossMode);
    Read(value, "boltsLostOnRespawn", out.boltsLostOnRespawn);
    Read(value, "boltsLostOnRespawnPercentage", out.boltsLostOnRespawnPercentage);
    Read(value, "boltsLostOnDeath", out.boltsLostOnDeath);
    Read(value, "boltsLostOnDeathPercentage", out.boltsLostOnDeathPercentage);
    Read(value, "boltsLostOnElimination", out.boltsLostOnElimination);
    Read(value, "boltsLostOnEliminationPercentage", out.boltsLostOnEliminationPercentage);
    Read(value, "rewardOnKillSpec", out.rewardOnKillSpec);
    Read(value, "rewardOnShieldBreakSpec", out.rewardOnShieldBreakSpec);
    Read(value, "isCoop", out.isCoop);
    Read(value, "ShowLocalPlayerTeamatesSilhouettes", out.ShowLocalPlayerTeamatesSilhouettes);
    Read(value, "ShowTeamatesAnnounces", out.ShowTeamatesAnnounces);
    Read(value, "ShowTeamatesHalo", out.ShowTeamatesHalo);
    Read(value, "ShowTeammatesMarks", out.ShowTeammatesMarks);
    Read(value, "playersCanRespawn", out.playersCanRespawn);
    Read(value, "AutoRespawn", out.AutoRespawn);
    Read(value, "RespawnNearTeamLeader", out.RespawnNearTeamLeader);
    Read(value, "RespawnNearTeammate", out.RespawnNearTeammate);
    Read(value, "RespawnDistance", out.RespawnDistance);
    Read(value, "PlayersCanRespawnForASetPeriodOfTime", out.PlayersCanRespawnForASetPeriodOfTime);
    Read(value, "RespawnSetTime", out.RespawnSetTime);
    Read(value, "allowShrinkingZone", out.allowShrinkingZone);
    Read(value, "shrinkingZoneSpeed", out.shrinkingZoneSpeed);
    Read(value, "shrinkingZoneDelay", out.shrinkingZoneDelay);
    Read(value, "shrinkingZoneMinRadius", out.shrinkingZoneMinRadius);
    Read(value, "allowMapDisappearance", out.allowMapDisappearance);
    Read(value, "disappearanceBufferRadius", out.disappearanceBufferRadius);
    Read(value, "mapCenter", out.mapCenter);
    Read(value, "autoFinishGameOnLastPlayerStanding", out.autoFinishGameOnLastPlayerStanding);
    Read(value, "outroTimeScale", out.outroTimeScale);
    Read(value, "outroTimeScalingDuration", out.outroTimeScalingDuration);
    Read(value, "boltDropperSpec", out.boltDropperSpec);
    Read(value, "boltGathererSpec", out.boltGathererSpec);
    Read(value, "droppableCollectorSpecRef", out.droppableCollectorSpecRef);
    Read(value, "dashActorConfig", out.dashActorConfig);
    Read(value, "useBoltPowerAndPerks", out.useBoltPowerAndPerks);
    Read(value, "boltPowerActorSpec", out.boltPowerActorSpec);
    Read(value, "ultimateBoostMinRank", out.ultimateBoostMinRank);
    Read(value, "useLives", out.useLives);
    Read(value, "maxLives", out.maxLives);
    Read(value, "useLivesForBots", out.useLivesForBots);
    Read(value, "maxLivesForBots", out.maxLivesForBots);
    Read(value, "showKillFeed", out.showKillFeed);
    Read(value, "autoShootEnabled", out.autoShootEnabled);
    Read(value, "autoShootDelay", out.autoShootDelay);
}

void ReadFields(const Value& value, Quantum::GameModesTable& out, ReaderTag) {
    Read(value, "prototypes", out.prototypes);
}

void ReadFields(const Value& value, Quantum::GameModifiersSpec& out, ReaderTag) {
    Read(value, "randomWeapon", out.randomWeapon);
}

void ReadFields(const Value& value, Quantum::GoldenBoltConfig& out, ReaderTag) {
    Read(value, "GoldenBoltToSpawn", out.GoldenBoltToSpawn);
    Read(value, "BoltsEarned", out.BoltsEarned);
    Read(value, "SpawnDelaysSeconds", out.SpawnDelaysSeconds);
    Read(value, "DropDurationSeconds", out.DropDurationSeconds);
    Read(value, "DropHeight", out.DropHeight);
    Read(value, "ArrivalHeightOffset", out.ArrivalHeightOffset);
    Read(value, "SafeDurationAfterArrivalInClosingZone", out.SafeDurationAfterArrivalInClosingZone);
}

void ReadFields(const Value& value, Quantum::MobGroupConfig& out, ReaderTag) {
    Read(value, "MemberEntityPrototype", out.MemberEntityPrototype);
}

void ReadFields(const Value& value, Quantum::MobInfoSpec& out, ReaderTag) {
    Read(value, "MobLevelMappingData", out.MobLevelMappingData);
}

void ReadFields(const Value& value, Quantum::SimulationHeroesTable& out, ReaderTag) {
    Read(value, "defaultSpec", out.defaultSpec);
    Read(value, "entries", out.entries);
}

void ReadFields(const Value& value, Quantum::SimulationHeroesTable::Entry& out, ReaderTag) {
    Read(value, "uniqueID", out.uniqueID);
    Read(value, "spec", out.spec);
}

void ReadFields(const Value& value, Quantum::SquadSiegeConfig& out, ReaderTag) {
    Read(value, "Levels", out.Levels);
    Read(value, "AttackerDestructionEntityInfoType", out.AttackerDestructionEntityInfoType);
    Read(value, "AttackerDestructionRange", out.AttackerDestructionRange);
    Read(value, "MobAutoDestructRange", out.MobAutoDestructRange);
    Read(value, "MobAutoDestructStuckDelay", out.MobAutoDestructStuckDelay);
    Read(value, "MobAutoDestructAfterLastSpawnDelay", out.MobAutoDestructAfterLastSpawnDelay);
}

void ReadFields(const Value& value, Quantum::SquadSiegeLevelConfig& out, ReaderTag) {
    Read(value, "SceneName", out.SceneName);
    Read(value, "PveScaling", out.PveScaling);
    Read(value, "PowerRequired", out.PowerRequired);
    Read(value, "Waves", out.Waves);
}

void ReadFields(const Value& value, Quantum::WaveManagerSpec& out, ReaderTag) {
    Read(value, "InitialWaveDelay", out.InitialWaveDelay);
    Read(value, "WaveStartDelay", out.WaveStartDelay);
    Read(value, "AutoStartOnMainLoopStart", out.AutoStartOnMainLoopStart);
    Read(value, "FinishGameOnAllWavesComplete", out.FinishGameOnAllWavesComplete);
    Read(value, "FinishGameDelay", out.FinishGameDelay);
    Read(value, "MobInitialInvulnerabilityDuration", out.MobInitialInvulnerabilityDuration);
}

void ReadFields(const Value& value, Quantum::WaveReferences& out, ReaderTag) {
    Read(value, "MobReferenceArray", out.MobReferenceArray);
    Read(value, "DropperReferenceArray", out.DropperReferenceArray);
}

void ReadFields(const Value& value, Quantum::WaveReferences::DroperReference& out, ReaderTag) {
    Read(value, "ID", out.ID);
    Read(value, "DropperSpecs", out.DropperSpecs);
}

void ReadFields(const Value& value, Quantum::WaveReferences::WaveMobReference& out, ReaderTag) {
    Read(value, "ID", out.ID);
    Read(value, "EntityToSpawn", out.EntityToSpawn);
}

void ReadFields(const Value& value, Quantum::BlastBallGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "footballPrototype", out.footballPrototype);
    Read(value, "scoreToWin", out.scoreToWin);
    Read(value, "scoringSequenceDuration", out.scoringSequenceDuration);
    Read(value, "resetPositionsDelay", out.resetPositionsDelay);
    Read(value, "overtimeTransitionDuration", out.overtimeTransitionDuration);
    Read(value, "overtimeTimeScaleDuration", out.overtimeTimeScaleDuration);
    Read(value, "restartDelay", out.restartDelay);
    Read(value, "maxHitToGoalDelay", out.maxHitToGoalDelay);
    Read(value, "autoShootMinBallToGoalDistance", out.autoShootMinBallToGoalDistance);
    Read(value, "autoShootMaxDot", out.autoShootMaxDot);
    Read(value, "goalSize", out.goalSize);
    Read(value, "mainZoneGoalSizeMultiplier", out.mainZoneGoalSizeMultiplier);
}

void ReadFields(const Value& value, Quantum::BoltKingGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "FirstPhaseDelay", out.FirstPhaseDelay);
    Read(value, "NextPhaseDelay", out.NextPhaseDelay);
    Read(value, "EndGameDelay", out.EndGameDelay);
    Read(value, "ScoreToBecomeKing", out.ScoreToBecomeKing);
    Read(value, "KingChaseDuration", out.KingChaseDuration);
    Read(value, "KingSpeedModifier", out.KingSpeedModifier);
    Read(value, "KingMaxHpMultiplier", out.KingMaxHpMultiplier);
    Read(value, "KingDamageMultiplier", out.KingDamageMultiplier);
    Read(value, "KingSizeMultiplier", out.KingSizeMultiplier);
    Read(value, "KingAnimatorPlaybackSpeed", out.KingAnimatorPlaybackSpeed);
    Read(value, "overHealthSpec", out.overHealthSpec);
    Read(value, "ChaserSpeedModifier", out.ChaserSpeedModifier);
    Read(value, "BotChasersCountPerHumanPlayerCount", out.BotChasersCountPerHumanPlayerCount);
    Read(value, "AllBotChaseHumanKing", out.AllBotChaseHumanKing);
}

void ReadFields(const Value& value, Quantum::EliminationGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "PowerUpEntityDropperSpecAsset", out.PowerUpEntityDropperSpecAsset);
    Read(value, "BotAggroSettingsByFtueStep", out.BotAggroSettingsByFtueStep);
}

void ReadFields(const Value& value, Quantum::EliminationGameModeSpec::BotAggroSettings& out, ReaderTag) {
    Read(value, "FtueStep", out.FtueStep);
    Read(value, "DefaultBotAggroBehaviour", out.DefaultBotAggroBehaviour);
    Read(value, "BotAggroBehaviourByIndex", out.BotAggroBehaviourByIndex);
}

void ReadFields(const Value& value, Quantum::FreeForAllGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "killsToWin", out.killsToWin);
}

void ReadFields(const Value& value, Quantum::SquadSiegeGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::TeamTotalRumbleGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "killsToWin", out.killsToWin);
}

void ReadFields(const Value& value, Quantum::TrainingGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::TutorialGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "stepBehaviours", out.stepBehaviours);
    Read(value, "playerName", out.playerName);
    Read(value, "playerLoadoutSpec", out.playerLoadoutSpec);
    Read(value, "overrideMeleeWeaponSpec", out.overrideMeleeWeaponSpec);
    Read(value, "overrideUltimateWeaponSpec", out.overrideUltimateWeaponSpec);
    Read(value, "overrideGadgetSpec", out.overrideGadgetSpec);
}

void ReadFields(const Value& value, Quantum::TutorialGameModeSpec::StepBehaviour& out, ReaderTag) {
    Read(value, "step", out.step);
    Read(value, "featuresToUnlock", out.featuresToUnlock);
}

void ReadFields(const Value& value, Quantum::WeaponShowcaseGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "weaponUserPrototype", out.weaponUserPrototype);
    Read(value, "isUnlockingAnimationDelay", out.isUnlockingAnimationDelay);
    Read(value, "moveToShootPositionDelay", out.moveToShootPositionDelay);
    Read(value, "moveToShootPositionDuration", out.moveToShootPositionDuration);
    Read(value, "moveToShootAnimationCurve", out.moveToShootAnimationCurve);
    Read(value, "triggerPressedDelay", out.triggerPressedDelay);
    Read(value, "moveBackToSpawnPositionDelay", out.moveBackToSpawnPositionDelay);
    Read(value, "moveBackToSpawnPositionDuration", out.moveBackToSpawnPositionDuration);
    Read(value, "moveBackToSpawnAnimationCurve", out.moveBackToSpawnAnimationCurve);
    Read(value, "allWeaponParams", out.allWeaponParams);
    Read(value, "upgradeXPos", out.upgradeXPos);
    Read(value, "moveWeaponsManually", out.moveWeaponsManually);
    Read(value, "blockFirstRotation", out.blockFirstRotation);
    Read(value, "blockShoot", out.blockShoot);
    Read(value, "blockLastRotation", out.blockLastRotation);
}

void ReadFields(const Value& value, Quantum::ZoneControlGameModeSpec& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::GameModeSpec&>(out), ReaderTag{});
    Read(value, "VictoryPointsToWin", out.VictoryPointsToWin);
    Read(value, "ControlZoneInitialLoadingTime", out.ControlZoneInitialLoadingTime);
    Read(value, "ControlZoneLoadingTime", out.ControlZoneLoadingTime);
    Read(value, "ControlZoneRequiredTime", out.ControlZoneRequiredTime);
    Read(value, "TotalDurationForCompletion", out.TotalDurationForCompletion);
    Read(value, "ScoringIncrementFrequency", out.ScoringIncrementFrequency);
    Read(value, "ComeBackToNeutralDuration", out.ComeBackToNeutralDuration);
    Read(value, "DelayBeforeLosingControl", out.DelayBeforeLosingControl);
}

void ReadFields(const Value& value, Quantum::BumperBarConfig& out, ReaderTag) {
    Read(value, "PushForce", out.PushForce);
    Read(value, "forceUpwardCorrection", out.forceUpwardCorrection);
}

void ReadFields(const Value& value, Quantum::BumperConfig& out, ReaderTag) {
    Read(value, "velocityBlend", out.velocityBlend);
    Read(value, "lockedInputBlend", out.lockedInputBlend);
    Read(value, "lockedInputDuration", out.lockedInputDuration);
    Read(value, "lockedInputFallOff", out.lockedInputFallOff);
    Read(value, "AK_Wwise_Event_GP_JumpPad_Activate", out.AK_Wwise_Event_GP_JumpPad_Activate);
}

void ReadFields(const Value& value, Quantum::ItemSpawnerConfig& out, ReaderTag) {
    Read(value, "EntityPrefabsAmount", out.EntityPrefabsAmount);
    Read(value, "EntityToSpawn", out.EntityToSpawn);
    Read(value, "EntitiesToSpawn", out.EntitiesToSpawn);
    Read(value, "AmountToSpawn", out.AmountToSpawn);
    Read(value, "SpawnOnStart", out.SpawnOnStart);
    Read(value, "DoesRespawn", out.DoesRespawn);
    Read(value, "RespawnDelay", out.RespawnDelay);
    Read(value, "DelayBetweenEachItem", out.DelayBetweenEachItem);
    Read(value, "WakeUpPhysicsBody", out.WakeUpPhysicsBody);
    Read(value, "ItemSpawnHeight", out.ItemSpawnHeight);
    Read(value, "DropDuration", out.DropDuration);
    Read(value, "SpawnPatternOffset", out.SpawnPatternOffset);
    Read(value, "ItemSpawnMinRotation", out.ItemSpawnMinRotation);
    Read(value, "ItemSpawnMaxRotation", out.ItemSpawnMaxRotation);
    Read(value, "RandomOffset", out.RandomOffset);
}

void ReadFields(const Value& value, Quantum::LevelButtonConfig& out, ReaderTag) {
    Read(value, "autoSwitchOffAfterActivation", out.autoSwitchOffAfterActivation);
    Read(value, "timeBeforeAutoSwitchOff", out.timeBeforeAutoSwitchOff);
    Read(value, "cooldownAfterSwitchOff", out.cooldownAfterSwitchOff);
}

void ReadFields(const Value&, Quantum::LevelButtonGameplayElementConfig&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::MovingWallConfig& out, ReaderTag) {
    Read(value, "StartsLockedInPlace", out.StartsLockedInPlace);
    Read(value, "MoveSpeed", out.MoveSpeed);
    Read(value, "WallOffsets", out.WallOffsets);
}

void ReadFields(const Value& value, Quantum::ShrinkingZoneSpec& out, ReaderTag) {
    Read(value, "hitInterval", out.hitInterval);
    Read(value, "DamageSpec", out.DamageSpec);
    Read(value, "DamageMultiplierCurve", out.DamageMultiplierCurve);
    Read(value, "PusherSpec", out.PusherSpec);
}

void ReadFields(const Value& value, Quantum::StaticColliderConfig& out, ReaderTag) {
    Read(value, "CustomId", out.CustomId);
    Read(value, "SurfaceType", out.SurfaceType);
    Read(value, "IgnoreAutoJump", out.IgnoreAutoJump);
    Read(value, "ReboundBonus", out.ReboundBonus);
}

void ReadFields(const Value&, Quantum::StaticTriggerConfig&, ReaderTag) {}

void ReadFields(const Value& value, Quantum::ArenaCannonConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::LevelButtonGameplayElementConfig&>(out), ReaderTag{});
    Read(value, "FireDuration", out.FireDuration);
    Read(value, "InitialDelay", out.InitialDelay);
}

void ReadFields(const Value& value, Quantum::AutoJumperStaticTriggerConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::StaticTriggerConfig&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::DestroyElementConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::LevelButtonGameplayElementConfig&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::DestroyMobGroupGameplayElement& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::LevelButtonGameplayElementConfig&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::MovingWallGameplayElementConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::LevelButtonGameplayElementConfig&>(out), ReaderTag{});
    Read(value, "mode", out.mode);
}

void ReadFields(const Value& value, Quantum::RailStaticTriggerConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::StaticTriggerConfig&>(out), ReaderTag{});
}

void ReadFields(const Value& value, Quantum::TravolatorStaticTriggerConfig& out, ReaderTag) {
    ReadFields(value, static_cast<Quantum::StaticTriggerConfig&>(out), ReaderTag{});
    Read(value, "MoveSpeedModifier", out.MoveSpeedModifier);
    Read(value, "InputBlend", out.InputBlend);
}

namespace {

template <class T> std::shared_ptr<Quantum::AssetObject> Make(const Value& settings) {
    auto asset = std::make_shared<T>();
    ReadFields(settings, *asset, ReaderTag{});
    return asset;
}

} // namespace

void RegisterQuantumCodeAssetFactories() {
    RegisterBuiltInAssetFactories();
    RegisterAssetFactory("AIBlackboardAsset", &Make<Quantum::AIBlackboard>);
    RegisterAssetFactory("AIBlackboardInitializerAsset", &Make<Quantum::AIBlackboardInitializer>);
    RegisterAssetFactory("AIBotSDKConfigAsset", &Make<Quantum::AIBotSDKConfig>);
    RegisterAssetFactory("AIConfigAsset", &Make<Quantum::AIConfig>);
    RegisterAssetFactory("AIFunctionAsset", &Make<Quantum::AIFunction>);
    RegisterAssetFactory("AIFunctionANDAsset", &Make<Quantum::AIFunctionAND>);
    RegisterAssetFactory("AIFunctionNOTAsset", &Make<Quantum::AIFunctionNOT>);
    RegisterAssetFactory("AIFunctionORAsset", &Make<Quantum::AIFunctionOR>);
    RegisterAssetFactory("AIMoveForwardBehaviourSpecAsset", &Make<Quantum::AIMoveForwardBehaviourSpec>);
    RegisterAssetFactory("AINavigationSpecAsset", &Make<Quantum::AINavigationSpec>);
    RegisterAssetFactory("AISpawnerConfigAsset", &Make<Quantum::AISpawnerConfig>);
    RegisterAssetFactory("Action_AddModifierMoveSpeedAsset", &Make<Quantum::Action_AddModifierMoveSpeed>);
    RegisterAssetFactory("Action_KillSelfAsset", &Make<Quantum::Action_KillSelf>);
    RegisterAssetFactory("Action_MoveTo_StartMovingAsset", &Make<Quantum::Action_MoveTo_StartMoving>);
    RegisterAssetFactory("Action_SpawnEntitiesAsset", &Make<Quantum::Action_SpawnEntities>);
    RegisterAssetFactory("ActorIcedSpecAsset", &Make<Quantum::ActorIcedSpec>);
    RegisterAssetFactory("ActorManagerConfigAsset", &Make<Quantum::ActorManagerConfig>);
    RegisterAssetFactory("ActorPhysicsSpecAsset", &Make<Quantum::ActorPhysicsSpec>);
    RegisterAssetFactory("ActorSheepedSpecAsset", &Make<Quantum::ActorSheepedSpec>);
    RegisterAssetFactory("ActorStatusModifierSpecAsset", &Make<Quantum::ActorStatusModifierSpec>);
    RegisterAssetFactory("ActorStatusSpecAsset", &Make<Quantum::ActorStatusSpec>);
    RegisterAssetFactory("ActorStunnedSpecAsset", &Make<Quantum::ActorStunnedSpec>);
    RegisterAssetFactory("AddDamageIncreaseOnTargetedEffectSpecAsset", &Make<Quantum::AddDamageIncreaseOnTargetedEffectSpec>);
    RegisterAssetFactory("AddEntitiesToSpawnOnProjectileDeathSpecAsset", &Make<Quantum::AddEntitiesToSpawnOnProjectileDeathSpec>);
    RegisterAssetFactory("AddTargetToBlacklistActionAsset", &Make<Quantum::AddTargetToBlacklistAction>);
    RegisterAssetFactory("AimAtPassingTargetEvaluatorAsset", &Make<Quantum::AimAtPassingTargetEvaluator>);
    RegisterAssetFactory("AirSaveEvaluatorAsset", &Make<Quantum::AirSaveEvaluator>);
    RegisterAssetFactory("AmmoMultiplierEnhancementSpecAsset", &Make<Quantum::AmmoMultiplierEnhancementSpec>);
    RegisterAssetFactory("AmmunitionDamageSpecAsset", &Make<Quantum::AmmunitionDamageSpec>);
    RegisterAssetFactory("AmoeboidLauncherEnhancementPuddleSpecAsset", &Make<Quantum::AmoeboidLauncherEnhancementPuddleSpec>);
    RegisterAssetFactory("AmoeboidLevelUpActorSpecAsset", &Make<Quantum::AmoeboidLevelUpActorSpec>);
    RegisterAssetFactory("AoeTrailSpecAsset", &Make<Quantum::AoeTrailSpec>);
    RegisterAssetFactory("AreaOfEffectIgnitionSpecAsset", &Make<Quantum::AreaOfEffectIgnitionSpec>);
    RegisterAssetFactory("AreaOfEffectLevelUpSpecAsset", &Make<Quantum::AreaOfEffectLevelUpSpec>);
    RegisterAssetFactory("AreaOfEffectSpecAsset", &Make<Quantum::AreaOfEffectSpec>);
    RegisterAssetFactory("ArenaCannonConfigAsset", &Make<Quantum::ArenaCannonConfig>);
    RegisterAssetFactory("ArithmeticFPFunctionAsset", &Make<Quantum::ArithmeticFPFunction>);
    RegisterAssetFactory("AttackControlZoneEvaluatorAsset", &Make<Quantum::AttackControlZoneEvaluator>);
    RegisterAssetFactory("AttackPlayerEvaluatorAsset", &Make<Quantum::AttackPlayerEvaluator>);
    RegisterAssetFactory("AutoJumperStaticTriggerConfigAsset", &Make<Quantum::AutoJumperStaticTriggerConfig>);
    RegisterAssetFactory("BTBlackboardCompareAsset", &Make<Quantum::BTBlackboardCompare>);
    RegisterAssetFactory("BTCooldownAsset", &Make<Quantum::BTCooldown>);
    RegisterAssetFactory("BTForceResultAsset", &Make<Quantum::BTForceResult>);
    RegisterAssetFactory("BTLoopAsset", &Make<Quantum::BTLoop>);
    RegisterAssetFactory("BTRootAsset", &Make<Quantum::BTRoot>);
    RegisterAssetFactory("BTSelectorAsset", &Make<Quantum::BTSelector>);
    RegisterAssetFactory("BTSelectorRandomAsset", &Make<Quantum::BTSelectorRandom>);
    RegisterAssetFactory("BTSequenceAsset", &Make<Quantum::BTSequence>);
    RegisterAssetFactory("BigBoomMineSpecAsset", &Make<Quantum::BigBoomMineSpec>);
    RegisterAssetFactory("BlastBallGameModeSpecAsset", &Make<Quantum::BlastBallGameModeSpec>);
    RegisterAssetFactory("BlastBallSpecAsset", &Make<Quantum::BlastBallSpec>);
    RegisterAssetFactory("BlasterMoveSpeedEnhancementSpecAsset", &Make<Quantum::BlasterMoveSpeedEnhancementSpec>);
    RegisterAssetFactory("BlasterSpecialShotEnhancementSpecAsset", &Make<Quantum::BlasterSpecialShotEnhancementSpec>);
    RegisterAssetFactory("BoltDropperSpecAsset", &Make<Quantum::BoltDropperSpec>);
    RegisterAssetFactory("BoltGathererActorSpecAsset", &Make<Quantum::BoltGathererActorSpec>);
    RegisterAssetFactory("BoltKingGameModeSpecAsset", &Make<Quantum::BoltKingGameModeSpec>);
    RegisterAssetFactory("BoltPowerActorSpecAsset", &Make<Quantum::BoltPowerActorSpec>);
    RegisterAssetFactory("BombardierDroneSpecAsset", &Make<Quantum::BombardierDroneSpec>);
    RegisterAssetFactory("BombardierWeaponAutoTargetSpecAsset", &Make<Quantum::BombardierWeaponAutoTargetSpec>);
    RegisterAssetFactory("BumperBarConfigAsset", &Make<Quantum::BumperBarConfig>);
    RegisterAssetFactory("BumperConfigAsset", &Make<Quantum::BumperConfig>);
    RegisterAssetFactory("BuzzbladesRicochetSpecAsset", &Make<Quantum::BuzzbladesRicochetSpec>);
    RegisterAssetFactory("ChargeAttackGetMaxDistanceFunctionAsset", &Make<Quantum::ChargeAttackGetMaxDistanceFunction>);
    RegisterAssetFactory("ChargeAttackInProgressFunctionAsset", &Make<Quantum::ChargeAttackInProgressFunction>);
    RegisterAssetFactory("ChargeAttackSpecAsset", &Make<Quantum::ChargeAttackSpec>);
    RegisterAssetFactory("ChargeAttackStartActionAsset", &Make<Quantum::ChargeAttackStartAction>);
    RegisterAssetFactory("ChargeWrenchActionAsset", &Make<Quantum::ChargeWrenchAction>);
    RegisterAssetFactory("ChargeWrenchEvaluatorAsset", &Make<Quantum::ChargeWrenchEvaluator>);
    RegisterAssetFactory("CheckBlackboardIntAsset", &Make<Quantum::CheckBlackboardInt>);
    RegisterAssetFactory("ChoosePatrolDestinationActionAsset", &Make<Quantum::ChoosePatrolDestinationAction>);
    RegisterAssetFactory("ColliderResizeSpecAsset", &Make<Quantum::ColliderResizeSpec>);
    RegisterAssetFactory("CollisionForceSpecAsset", &Make<Quantum::CollisionForceSpec>);
    RegisterAssetFactory("CompoundEnhancementSpecAsset", &Make<Quantum::CompoundEnhancementSpec>);
    RegisterAssetFactory("CompoundHFSMSpecAsset", &Make<Quantum::CompoundHFSMSpec>);
    RegisterAssetFactory("CompoundHeroImprovementSpecAsset", &Make<Quantum::CompoundHeroImprovementSpec>);
    RegisterAssetFactory("ConsiderationAsset", &Make<Quantum::Consideration>);
    RegisterAssetFactory("CooldownMultiplierEnhancementSpecAsset", &Make<Quantum::CooldownMultiplierEnhancementSpec>);
    RegisterAssetFactory("CryosliderSpecAsset", &Make<Quantum::CryosliderSpec>);
    RegisterAssetFactory("CryosliderWeaponAutoTargetSpecAsset", &Make<Quantum::CryosliderWeaponAutoTargetSpec>);
    RegisterAssetFactory("DamageChargeableSpecAsset", &Make<Quantum::DamageChargeableSpec>);
    RegisterAssetFactory("DamageIncreasedOnTargetsEnhancementSpecAsset", &Make<Quantum::DamageIncreasedOnTargetsEnhancementSpec>);
    RegisterAssetFactory("DamageMultiplierHeroImprovementSpecAsset", &Make<Quantum::DamageMultiplierHeroImprovementSpec>);
    RegisterAssetFactory("DamageMultiplierSpecAsset", &Make<Quantum::DamageMultiplierSpec>);
    RegisterAssetFactory("DamageSpecAsset", &Make<Quantum::DamageSpec>);
    RegisterAssetFactory("DamageToImpulseConverterSpecAsset", &Make<Quantum::DamageToImpulseConverterSpec>);
    RegisterAssetFactory("DamageableSpecAsset", &Make<Quantum::DamageableSpec>);
    RegisterAssetFactory("DashActorConfigAsset", &Make<Quantum::DashActorConfig>);
    RegisterAssetFactory("DashBlastBallEvaluatorAsset", &Make<Quantum::DashBlastBallEvaluator>);
    RegisterAssetFactory("DashCooldownReductionHeroImprovementSpecAsset", &Make<Quantum::DashCooldownReductionHeroImprovementSpec>);
    RegisterAssetFactory("DashRoboBallAsset", &Make<Quantum::DashRoboBall>);
    RegisterAssetFactory("DeathAnimationSpecAsset", &Make<Quantum::DeathAnimationSpec>);
    RegisterAssetFactory("DebugActionAsset", &Make<Quantum::DebugAction>);
    RegisterAssetFactory("DebugLeafAsset", &Make<Quantum::DebugLeaf>);
    RegisterAssetFactory("DebugServiceAsset", &Make<Quantum::DebugService>);
    RegisterAssetFactory("DefaultAIFunctionAssetRefAsset", &Make<Quantum::DefaultAIFunctionAssetRef>);
    RegisterAssetFactory("DefaultAIFunctionBoolAsset", &Make<Quantum::DefaultAIFunctionBool>);
    RegisterAssetFactory("DefaultAIFunctionByteAsset", &Make<Quantum::DefaultAIFunctionByte>);
    RegisterAssetFactory("DefaultAIFunctionEntityRefAsset", &Make<Quantum::DefaultAIFunctionEntityRef>);
    RegisterAssetFactory("DefaultAIFunctionFPAsset", &Make<Quantum::DefaultAIFunctionFP>);
    RegisterAssetFactory("DefaultAIFunctionFPVector2Asset", &Make<Quantum::DefaultAIFunctionFPVector2>);
    RegisterAssetFactory("DefaultAIFunctionFPVector3Asset", &Make<Quantum::DefaultAIFunctionFPVector3>);
    RegisterAssetFactory("DefaultAIFunctionIntAsset", &Make<Quantum::DefaultAIFunctionInt>);
    RegisterAssetFactory("DefaultAIFunctionStringAsset", &Make<Quantum::DefaultAIFunctionString>);
    RegisterAssetFactory("DefendControlZoneEvaluatorAsset", &Make<Quantum::DefendControlZoneEvaluator>);
    RegisterAssetFactory("DestroyActionAsset", &Make<Quantum::DestroyAction>);
    RegisterAssetFactory("DestroyElementConfigAsset", &Make<Quantum::DestroyElementConfig>);
    RegisterAssetFactory("DestroyMobGroupGameplayElementAsset", &Make<Quantum::DestroyMobGroupGameplayElement>);
    RegisterAssetFactory("DispatchMobSlamAttacksEventActionAsset", &Make<Quantum::DispatchMobSlamAttacksEventAction>);
    RegisterAssetFactory("DispatchMobsEventActionAsset", &Make<Quantum::DispatchMobsEventAction>);
    RegisterAssetFactory("DispatchShooterEventActionAsset", &Make<Quantum::DispatchShooterEventAction>);
    RegisterAssetFactory("DispatchSpinnerEventActionAsset", &Make<Quantum::DispatchSpinnerEventAction>);
    RegisterAssetFactory("DistanceFromPositionDecisionAsset", &Make<Quantum::DistanceFromPositionDecision>);
    RegisterAssetFactory("DrillDashGadgetSpecAsset", &Make<Quantum::DrillDashGadgetSpec>);
    RegisterAssetFactory("DroppableCollectorSpecAsset", &Make<Quantum::DroppableCollectorSpec>);
    RegisterAssetFactory("DroppableManagerSpecAsset", &Make<Quantum::DroppableManagerSpec>);
    RegisterAssetFactory("ElementalBuildUpMultiplierHeroImprovementSpecAsset", &Make<Quantum::ElementalBuildUpMultiplierHeroImprovementSpec>);
    RegisterAssetFactory("ElementalDamageTableAsset", &Make<Quantum::ElementalDamageTable>);
    RegisterAssetFactory("EliminationGameModeSpecAsset", &Make<Quantum::EliminationGameModeSpec>);
    RegisterAssetFactory("EnergyCellDropperSpecAsset", &Make<Quantum::EnergyCellDropperSpec>);
    RegisterAssetFactory("EnergyCellManagerSpecAsset", &Make<Quantum::EnergyCellManagerSpec>);
    RegisterAssetFactory("EnergyCellPowerUpSpecAsset", &Make<Quantum::EnergyCellPowerUpSpec>);
    RegisterAssetFactory("EnforcerFirstShotBonusEnhancementSpecAsset", &Make<Quantum::EnforcerFirstShotBonusEnhancementSpec>);
    RegisterAssetFactory("EntityExistsFunctionAsset", &Make<Quantum::EntityExistsFunction>);
    RegisterAssetFactory("EntityGroupSpecAsset", &Make<Quantum::EntityGroupSpec>);
    RegisterAssetFactory("EntityIsGroundedFunctionAsset", &Make<Quantum::EntityIsGroundedFunction>);
    RegisterAssetFactory("EntityRefsAreEqualFunctionAsset", &Make<Quantum::EntityRefsAreEqualFunction>);
    RegisterAssetFactory("EscapeShrinkingZoneEvaluatorAsset", &Make<Quantum::EscapeShrinkingZoneEvaluator>);
    RegisterAssetFactory("ExtraDamagePowerUpSpecAsset", &Make<Quantum::ExtraDamagePowerUpSpec>);
    RegisterAssetFactory("FindTargetActionAsset", &Make<Quantum::FindTargetAction>);
    RegisterAssetFactory("FireReactorSpecAsset", &Make<Quantum::FireReactorSpec>);
    RegisterAssetFactory("FirerateMultiplierEnhancementSpecAsset", &Make<Quantum::FirerateMultiplierEnhancementSpec>);
    RegisterAssetFactory("FrameTimeHasElapsedFunctionAsset", &Make<Quantum::FrameTimeHasElapsedFunction>);
    RegisterAssetFactory("FreeForAllGameModeSpecAsset", &Make<Quantum::FreeForAllGameModeSpec>);
    RegisterAssetFactory("GOAPDefaultActionAsset", &Make<Quantum::GOAPDefaultAction>);
    RegisterAssetFactory("GOAPDefaultGoalAsset", &Make<Quantum::GOAPDefaultGoal>);
    RegisterAssetFactory("GOAPRootAsset", &Make<Quantum::GOAPRoot>);
    RegisterAssetFactory("GadgetWeaponSpecAsset", &Make<Quantum::GadgetWeaponSpec>);
    RegisterAssetFactory("GameIsStartedAsset", &Make<Quantum::GameIsStarted>);
    RegisterAssetFactory("GameManagerConfigAsset", &Make<Quantum::GameManagerConfig>);
    RegisterAssetFactory("GameModeHandlerConfigAsset", &Make<Quantum::GameModeHandlerConfig>);
    RegisterAssetFactory("GameModeHandlerLevelConfigAsset", &Make<Quantum::GameModeHandlerLevelConfig>);
    RegisterAssetFactory("GameModesTableAsset", &Make<Quantum::GameModesTable>);
    RegisterAssetFactory("GameModifiersSpecAsset", &Make<Quantum::GameModifiersSpec>);
    RegisterAssetFactory("GatlingEnhancementResetAmmoOnGadgetUseSpecAsset", &Make<Quantum::GatlingEnhancementResetAmmoOnGadgetUseSpec>);
    RegisterAssetFactory("GatlingEnhancementSpecAsset", &Make<Quantum::GatlingEnhancementSpec>);
    RegisterAssetFactory("GatlingWeaponSpecAsset", &Make<Quantum::GatlingWeaponSpec>);
    RegisterAssetFactory("GetAttackInfoActiveFunctionAsset", &Make<Quantum::GetAttackInfoActiveFunction>);
    RegisterAssetFactory("GetAttackInfoTargetFunctionAsset", &Make<Quantum::GetAttackInfoTargetFunction>);
    RegisterAssetFactory("GetAttackInfoWasRunningFunctionAsset", &Make<Quantum::GetAttackInfoWasRunningFunction>);
    RegisterAssetFactory("GetEntityPositionFunctionAsset", &Make<Quantum::GetEntityPositionFunction>);
    RegisterAssetFactory("GetFrameNumberFunctionAsset", &Make<Quantum::GetFrameNumberFunction>);
    RegisterAssetFactory("GetGroupEntityFunctionAsset", &Make<Quantum::GetGroupEntityFunction>);
    RegisterAssetFactory("GetGroupTargetFunctionAsset", &Make<Quantum::GetGroupTargetFunction>);
    RegisterAssetFactory("GetHighestPriorityTargetEntityFunctionAsset", &Make<Quantum::GetHighestPriorityTargetEntityFunction>);
    RegisterAssetFactory("GetHighestPriorityTargetPositionFunctionAsset", &Make<Quantum::GetHighestPriorityTargetPositionFunction>);
    RegisterAssetFactory("GetRandomFPFunctionAsset", &Make<Quantum::GetRandomFPFunction>);
    RegisterAssetFactory("GetRecentPriorityTargetPositionFunctionAsset", &Make<Quantum::GetRecentPriorityTargetPositionFunction>);
    RegisterAssetFactory("GetTargetFinderRangeNoWeaponFunctionAsset", &Make<Quantum::GetTargetFinderRangeNoWeaponFunction>);
    RegisterAssetFactory("GetVec2ElementAsset", &Make<Quantum::GetVec2Element>);
    RegisterAssetFactory("GloveOfDoomWeaponSpecAsset", &Make<Quantum::GloveOfDoomWeaponSpec>);
    RegisterAssetFactory("GoToAsset", &Make<Quantum::GoTo>);
    RegisterAssetFactory("GoToControlZoneAsset", &Make<Quantum::GoToControlZone>);
    RegisterAssetFactory("GoToPlayerDebugPosAsset", &Make<Quantum::GoToPlayerDebugPos>);
    RegisterAssetFactory("GoldenBoltConfigAsset", &Make<Quantum::GoldenBoltConfig>);
    RegisterAssetFactory("GrenadeLauncherAftershockEnhancementSpecAsset", &Make<Quantum::GrenadeLauncherAftershockEnhancementSpec>);
    RegisterAssetFactory("GrenadeLauncherStunGrenadeEnhancementSpecAsset", &Make<Quantum::GrenadeLauncherStunGrenadeEnhancementSpec>);
    RegisterAssetFactory("GrinderActorConfigAsset", &Make<Quantum::GrinderActorConfig>);
    RegisterAssetFactory("GroundDashActionAsset", &Make<Quantum::GroundDashAction>);
    RegisterAssetFactory("GroundDashActorConfigAsset", &Make<Quantum::GroundDashActorConfig>);
    RegisterAssetFactory("GroupTeamEvaluatorAsset", &Make<Quantum::GroupTeamEvaluator>);
    RegisterAssetFactory("HFSMAndDecisionAsset", &Make<Quantum::HFSMAndDecision>);
    RegisterAssetFactory("HFSMNotDecisionAsset", &Make<Quantum::HFSMNotDecision>);
    RegisterAssetFactory("HFSMOrDecisionAsset", &Make<Quantum::HFSMOrDecision>);
    RegisterAssetFactory("HFSMRootAsset", &Make<Quantum::HFSMRoot>);
    RegisterAssetFactory("HFSMStateAsset", &Make<Quantum::HFSMState>);
    RegisterAssetFactory("HFSMTransitionSetAsset", &Make<Quantum::HFSMTransitionSet>);
    RegisterAssetFactory("HasGroupTargetFunctionAsset", &Make<Quantum::HasGroupTargetFunction>);
    RegisterAssetFactory("HasLineOfSightToTargetDecisionAsset", &Make<Quantum::HasLineOfSightToTargetDecision>);
    RegisterAssetFactory("HasMoveBehaviourBouncedDecisionAsset", &Make<Quantum::HasMoveBehaviourBouncedDecision>);
    RegisterAssetFactory("HasNavLineOfSightToTargetFunctionAsset", &Make<Quantum::HasNavLineOfSightToTargetFunction>);
    RegisterAssetFactory("HasReachedTimeAsset", &Make<Quantum::HasReachedTime>);
    RegisterAssetFactory("HasTargetAsset", &Make<Quantum::HasTarget>);
    RegisterAssetFactory("HasValidRecentPriorityTargetFunctionAsset", &Make<Quantum::HasValidRecentPriorityTargetFunction>);
    RegisterAssetFactory("HeadHunterWallHackEnhancementSpecAsset", &Make<Quantum::HeadHunterWallHackEnhancementSpec>);
    RegisterAssetFactory("HealSourceSpecAsset", &Make<Quantum::HealSourceSpec>);
    RegisterAssetFactory("HealthMultiplierEnhancementSpecAsset", &Make<Quantum::HealthMultiplierEnhancementSpec>);
    RegisterAssetFactory("HealthSpecAsset", &Make<Quantum::HealthSpec>);
    RegisterAssetFactory("HelipackActorSpecAsset", &Make<Quantum::HelipackActorSpec>);
    RegisterAssetFactory("HeroSpecAsset", &Make<Quantum::HeroSpec>);
    RegisterAssetFactory("HoloshieldGloveHealZoneEnhancementSpecAsset", &Make<Quantum::HoloshieldGloveHealZoneEnhancementSpec>);
    RegisterAssetFactory("HoloshieldGloveWeaponSpecAsset", &Make<Quantum::HoloshieldGloveWeaponSpec>);
    RegisterAssetFactory("HoverBootsEnhancementCollisionSpecAsset", &Make<Quantum::HoverBootsEnhancementCollisionSpec>);
    RegisterAssetFactory("HoverBootsWeaponSpecAsset", &Make<Quantum::HoverBootsWeaponSpec>);
    RegisterAssetFactory("IdleActionAsset", &Make<Quantum::IdleAction>);
    RegisterAssetFactory("IgnoreEntitiesMultiplierEnhancementSpecAsset", &Make<Quantum::IgnoreEntitiesMultiplierEnhancementSpec>);
    RegisterAssetFactory("ImpulseActionAsset", &Make<Quantum::ImpulseAction>);
    RegisterAssetFactory("ImpulseTowardTargetActionAsset", &Make<Quantum::ImpulseTowardTargetAction>);
    RegisterAssetFactory("InShootingZoneMovementEvaluatorAsset", &Make<Quantum::InShootingZoneMovementEvaluator>);
    RegisterAssetFactory("IncreaseBlackboardIntAsset", &Make<Quantum::IncreaseBlackboardInt>);
    RegisterAssetFactory("InputCheatFollowPlayerAsset", &Make<Quantum::InputCheatFollowPlayer>);
    RegisterAssetFactory("IsDeadDecisionAsset", &Make<Quantum::IsDeadDecision>);
    RegisterAssetFactory("IsGroundedDecisionAsset", &Make<Quantum::IsGroundedDecision>);
    RegisterAssetFactory("IsTargetValidDecisionAsset", &Make<Quantum::IsTargetValidDecision>);
    RegisterAssetFactory("IsTrueAsset", &Make<Quantum::IsTrue>);
    RegisterAssetFactory("ItemSpawnerConfigAsset", &Make<Quantum::ItemSpawnerConfig>);
    RegisterAssetFactory("JumpActionAsset", &Make<Quantum::JumpAction>);
    RegisterAssetFactory("JumpAttackActionAsset", &Make<Quantum::JumpAttackAction>);
    RegisterAssetFactory("JumpAttackHeroImprovementSpecAsset", &Make<Quantum::JumpAttackHeroImprovementSpec>);
    RegisterAssetFactory("JumpAttackSpecAsset", &Make<Quantum::JumpAttackSpec>);
    RegisterAssetFactory("JumpMultiplierHeroImprovementSpecAsset", &Make<Quantum::JumpMultiplierHeroImprovementSpec>);
    RegisterAssetFactory("KccExtensionConfigAsset", &Make<Quantum::KccExtensionConfig>);
    RegisterAssetFactory("KccIsMovingFunctionAsset", &Make<Quantum::KccIsMovingFunction>);
    RegisterAssetFactory("KccModifierSpecAsset", &Make<Quantum::KccModifierSpec>);
    RegisterAssetFactory("LedgeGrabbingSpecAsset", &Make<Quantum::LedgeGrabbingSpec>);
    RegisterAssetFactory("LevelButtonConfigAsset", &Make<Quantum::LevelButtonConfig>);
    RegisterAssetFactory("LifetimeSpecAsset", &Make<Quantum::LifetimeSpec>);
    RegisterAssetFactory("MapCustomDataAsset", &Make<Quantum::MapCustomData>);
    RegisterAssetFactory("MeleeAttackIsActiveFunctionAsset", &Make<Quantum::MeleeAttackIsActiveFunction>);
    RegisterAssetFactory("MeleeAttackSpecAsset", &Make<Quantum::MeleeAttackSpec>);
    RegisterAssetFactory("MeleeJumpAttackSpecAsset", &Make<Quantum::MeleeJumpAttackSpec>);
    RegisterAssetFactory("MinMaxFPFunctionAsset", &Make<Quantum::MinMaxFPFunction>);
    RegisterAssetFactory("MineLauncherEnhancementSpecAsset", &Make<Quantum::MineLauncherEnhancementSpec>);
    RegisterAssetFactory("MineLauncherProjectileOverrideEnhancementSpecAsset", &Make<Quantum::MineLauncherProjectileOverrideEnhancementSpec>);
    RegisterAssetFactory("MobGroupConfigAsset", &Make<Quantum::MobGroupConfig>);
    RegisterAssetFactory("MobInfoSpecAsset", &Make<Quantum::MobInfoSpec>);
    RegisterAssetFactory("MoveAwayFromTeammatesEvaluatorAsset", &Make<Quantum::MoveAwayFromTeammatesEvaluator>);
    RegisterAssetFactory("MoveToClosestEnemyEvaluatorAsset", &Make<Quantum::MoveToClosestEnemyEvaluator>);
    RegisterAssetFactory("MoveToSpecAsset", &Make<Quantum::MoveToSpec>);
    RegisterAssetFactory("MoveTowardTargetActionAsset", &Make<Quantum::MoveTowardTargetAction>);
    RegisterAssetFactory("MoveTowardsBallEvaluatorAsset", &Make<Quantum::MoveTowardsBallEvaluator>);
    RegisterAssetFactory("MovingWallConfigAsset", &Make<Quantum::MovingWallConfig>);
    RegisterAssetFactory("MovingWallGameplayElementConfigAsset", &Make<Quantum::MovingWallGameplayElementConfig>);
    RegisterAssetFactory("NavigateToTargetActionAsset", &Make<Quantum::NavigateToTargetAction>);
    RegisterAssetFactory("NegatronWeaponSpecAsset", &Make<Quantum::NegatronWeaponSpec>);
    RegisterAssetFactory("OnKillCallbacksHeroImprovementSpecAsset", &Make<Quantum::OnKillCallbacksHeroImprovementSpec>);
    RegisterAssetFactory("OverhealthPowerUpSpecAsset", &Make<Quantum::OverhealthPowerUpSpec>);
    RegisterAssetFactory("ParabolaFollowerSpecAsset", &Make<Quantum::ParabolaFollowerSpec>);
    RegisterAssetFactory("PassingEvaluatorAsset", &Make<Quantum::PassingEvaluator>);
    RegisterAssetFactory("PathfindStatusEqualsFunctionAsset", &Make<Quantum::PathfindStatusEqualsFunction>);
    RegisterAssetFactory("PlaceholderHeroImprovementSpecAsset", &Make<Quantum::PlaceholderHeroImprovementSpec>);
    RegisterAssetFactory("PowerUpDropperSpecAsset", &Make<Quantum::PowerUpDropperSpec>);
    RegisterAssetFactory("PreciseHitReceiverSpecAsset", &Make<Quantum::PreciseHitReceiverSpec>);
    RegisterAssetFactory("PrepareShootingActionAsset", &Make<Quantum::PrepareShootingAction>);
    RegisterAssetFactory("PrioritisedTargetHandlerSpecAsset", &Make<Quantum::PrioritisedTargetHandlerSpec>);
    RegisterAssetFactory("PriorityTargetTypeCompareFunctionAsset", &Make<Quantum::PriorityTargetTypeCompareFunction>);
    RegisterAssetFactory("ProjectileSpawnerSpecAsset", &Make<Quantum::ProjectileSpawnerSpec>);
    RegisterAssetFactory("ProjectileSpecAsset", &Make<Quantum::ProjectileSpec>);
    RegisterAssetFactory("PushableSpecAsset", &Make<Quantum::PushableSpec>);
    RegisterAssetFactory("PusherSpecAsset", &Make<Quantum::PusherSpec>);
    RegisterAssetFactory("RailStaticTriggerConfigAsset", &Make<Quantum::RailStaticTriggerConfig>);
    RegisterAssetFactory("RaiseAgentOfDoomViewEventAsset", &Make<Quantum::RaiseAgentOfDoomViewEvent>);
    RegisterAssetFactory("ReachEnemyGoalEvaluatorAsset", &Make<Quantum::ReachEnemyGoalEvaluator>);
    RegisterAssetFactory("ReachShootingZoneEvaluatorAsset", &Make<Quantum::ReachShootingZoneEvaluator>);
    RegisterAssetFactory("ReachStrikeZoneEvaluatorAsset", &Make<Quantum::ReachStrikeZoneEvaluator>);
    RegisterAssetFactory("ReachedPointDecisionAsset", &Make<Quantum::ReachedPointDecision>);
    RegisterAssetFactory("RecoverAmmoOnDashEnhancementSpecAsset", &Make<Quantum::RecoverAmmoOnDashEnhancementSpec>);
    RegisterAssetFactory("ReplaceHealthSpecHeroImprovementSpecAsset", &Make<Quantum::ReplaceHealthSpecHeroImprovementSpec>);
    RegisterAssetFactory("ReplaceMultiShotConfigsEnhancementSpecAsset", &Make<Quantum::ReplaceMultiShotConfigsEnhancementSpec>);
    RegisterAssetFactory("ResetMaxSpeedModifierAsset", &Make<Quantum::ResetMaxSpeedModifier>);
    RegisterAssetFactory("ResetTargetActionAsset", &Make<Quantum::ResetTargetAction>);
    RegisterAssetFactory("ResetTimerActionAsset", &Make<Quantum::ResetTimerAction>);
    RegisterAssetFactory("ResponseCurveAsset", &Make<Quantum::ResponseCurve>);
    RegisterAssetFactory("RewardReceiverSpecAsset", &Make<Quantum::RewardReceiverSpec>);
    RegisterAssetFactory("RewardSpecAsset", &Make<Quantum::RewardSpec>);
    RegisterAssetFactory("RollSpecAsset", &Make<Quantum::RollSpec>);
    RegisterAssetFactory("RynoWeaponSpecAsset", &Make<Quantum::RynoWeaponSpec>);
    RegisterAssetFactory("SeekCrateEvaluatorAsset", &Make<Quantum::SeekCrateEvaluator>);
    RegisterAssetFactory("SeekDroppableEvaluatorAsset", &Make<Quantum::SeekDroppableEvaluator>);
    RegisterAssetFactory("SeekGoldenBoltEvaluatorAsset", &Make<Quantum::SeekGoldenBoltEvaluator>);
    RegisterAssetFactory("SeekNewTargetAsset", &Make<Quantum::SeekNewTarget>);
    RegisterAssetFactory("SeekNewTargetPriorityListAsset", &Make<Quantum::SeekNewTargetPriorityList>);
    RegisterAssetFactory("SetAttackInfoActiveActionAsset", &Make<Quantum::SetAttackInfoActiveAction>);
    RegisterAssetFactory("SetBlackboardBoolAsset", &Make<Quantum::SetBlackboardBool>);
    RegisterAssetFactory("SetBlackboardEntityRefAsset", &Make<Quantum::SetBlackboardEntityRef>);
    RegisterAssetFactory("SetBlackboardFPAsset", &Make<Quantum::SetBlackboardFP>);
    RegisterAssetFactory("SetBlackboardFPVector3Asset", &Make<Quantum::SetBlackboardFPVector3>);
    RegisterAssetFactory("SetBlackboardIntAsset", &Make<Quantum::SetBlackboardInt>);
    RegisterAssetFactory("SetCompoundHFSMSchedulePeriodActionAsset", &Make<Quantum::SetCompoundHFSMSchedulePeriodAction>);
    RegisterAssetFactory("SetCurrentMaxSpeedModifierAsset", &Make<Quantum::SetCurrentMaxSpeedModifier>);
    RegisterAssetFactory("SetGravityModifierActionAsset", &Make<Quantum::SetGravityModifierAction>);
    RegisterAssetFactory("SetGroupTargetActionAsset", &Make<Quantum::SetGroupTargetAction>);
    RegisterAssetFactory("SetOwnerInBlackboardActionAsset", &Make<Quantum::SetOwnerInBlackboardAction>);
    RegisterAssetFactory("SetPositionInBlackboardActionAsset", &Make<Quantum::SetPositionInBlackboardAction>);
    RegisterAssetFactory("SetShootPrecisionAsset", &Make<Quantum::SetShootPrecision>);
    RegisterAssetFactory("SetSpawnerPositionInBlackboardActionAsset", &Make<Quantum::SetSpawnerPositionInBlackboardAction>);
    RegisterAssetFactory("SetSpawnerWanderRadiusInBlackboardActionAsset", &Make<Quantum::SetSpawnerWanderRadiusInBlackboardAction>);
    RegisterAssetFactory("SetupAoEActionAsset", &Make<Quantum::SetupAoEAction>);
    RegisterAssetFactory("ShapeOverlapperSpecAsset", &Make<Quantum::ShapeOverlapperSpec>);
    RegisterAssetFactory("ShieldedActorSpecAsset", &Make<Quantum::ShieldedActorSpec>);
    RegisterAssetFactory("ShootBallActionAsset", &Make<Quantum::ShootBallAction>);
    RegisterAssetFactory("ShootBallEvaluatorAsset", &Make<Quantum::ShootBallEvaluator>);
    RegisterAssetFactory("ShootEvaluatorAsset", &Make<Quantum::ShootEvaluator>);
    RegisterAssetFactory("ShootTargetAsset", &Make<Quantum::ShootTarget>);
    RegisterAssetFactory("ShotgunBerserkerEnhancementSpecAsset", &Make<Quantum::ShotgunBerserkerEnhancementSpec>);
    RegisterAssetFactory("ShotgunEnhancementSpecAsset", &Make<Quantum::ShotgunEnhancementSpec>);
    RegisterAssetFactory("ShrinkingZoneSpecAsset", &Make<Quantum::ShrinkingZoneSpec>);
    RegisterAssetFactory("SimulationGadgetTableAsset", &Make<Quantum::SimulationGadgetTable>);
    RegisterAssetFactory("SimulationHeroesTableAsset", &Make<Quantum::SimulationHeroesTable>);
    RegisterAssetFactory("SimulationMeleeWeaponsTableAsset", &Make<Quantum::SimulationMeleeWeaponsTable>);
    RegisterAssetFactory("SimulationUltimateTableAsset", &Make<Quantum::SimulationUltimateTable>);
    RegisterAssetFactory("SimulationWeaponTableAsset", &Make<Quantum::SimulationWeaponTable>);
    RegisterAssetFactory("SmokeScreenSpecAsset", &Make<Quantum::SmokeScreenSpec>);
    RegisterAssetFactory("SpeedBoostPowerUpSpecAsset", &Make<Quantum::SpeedBoostPowerUpSpec>);
    RegisterAssetFactory("SpeederSpecAsset", &Make<Quantum::SpeederSpec>);
    RegisterAssetFactory("SquadSiegeConfigAsset", &Make<Quantum::SquadSiegeConfig>);
    RegisterAssetFactory("SquadSiegeGameModeSpecAsset", &Make<Quantum::SquadSiegeGameModeSpec>);
    RegisterAssetFactory("SquadSiegeLevelConfigAsset", &Make<Quantum::SquadSiegeLevelConfig>);
    RegisterAssetFactory("StartMeleeAttackActionAsset", &Make<Quantum::StartMeleeAttackAction>);
    RegisterAssetFactory("StartShootingActionAsset", &Make<Quantum::StartShootingAction>);
    RegisterAssetFactory("StaticColliderConfigAsset", &Make<Quantum::StaticColliderConfig>);
    RegisterAssetFactory("StopNavigationActionAsset", &Make<Quantum::StopNavigationAction>);
    RegisterAssetFactory("StopPathFindingAsset", &Make<Quantum::StopPathFinding>);
    RegisterAssetFactory("StopShootingActionAsset", &Make<Quantum::StopShootingAction>);
    RegisterAssetFactory("StopToShootEvaluatorAsset", &Make<Quantum::StopToShootEvaluator>);
    RegisterAssetFactory("StreamEffectSpawnerSpecAsset", &Make<Quantum::StreamEffectSpawnerSpec>);
    RegisterAssetFactory("StreamEffectSpecAsset", &Make<Quantum::StreamEffectSpec>);
    RegisterAssetFactory("SweepAttackActionAsset", &Make<Quantum::SweepAttackAction>);
    RegisterAssetFactory("SweepAttackSpecAsset", &Make<Quantum::SweepAttackSpec>);
    RegisterAssetFactory("SweepAttackStateEqualsFunctionAsset", &Make<Quantum::SweepAttackStateEqualsFunction>);
    RegisterAssetFactory("SwitchWeaponAsset", &Make<Quantum::SwitchWeapon>);
    RegisterAssetFactory("SwitchWeaponEvaluatorAsset", &Make<Quantum::SwitchWeaponEvaluator>);
    RegisterAssetFactory("SwitchWeaponUltimateAsset", &Make<Quantum::SwitchWeaponUltimate>);
    RegisterAssetFactory("SwitchWeaponUltimateEvaluatorAsset", &Make<Quantum::SwitchWeaponUltimateEvaluator>);
    RegisterAssetFactory("TakeDefensivePositionEvaluatorAsset", &Make<Quantum::TakeDefensivePositionEvaluator>);
    RegisterAssetFactory("TargetFinderSpecAsset", &Make<Quantum::TargetFinderSpec>);
    RegisterAssetFactory("TargetedEffectSpecAsset", &Make<Quantum::TargetedEffectSpec>);
    RegisterAssetFactory("TeamTotalRumbleGameModeSpecAsset", &Make<Quantum::TeamTotalRumbleGameModeSpec>);
    RegisterAssetFactory("TeslaClawMoreBeamEnhancementSpecAsset", &Make<Quantum::TeslaClawMoreBeamEnhancementSpec>);
    RegisterAssetFactory("TeslaClawWeaponSpecAsset", &Make<Quantum::TeslaClawWeaponSpec>);
    RegisterAssetFactory("TimerDecisionAsset", &Make<Quantum::TimerDecision>);
    RegisterAssetFactory("ToggleAoEActionAsset", &Make<Quantum::ToggleAoEAction>);
    RegisterAssetFactory("ToggleMovementBehaviourActionAsset", &Make<Quantum::ToggleMovementBehaviourAction>);
    RegisterAssetFactory("ToggleMovementPackSlicingActionAsset", &Make<Quantum::ToggleMovementPackSlicingAction>);
    RegisterAssetFactory("ToggleTargetFinderActionAsset", &Make<Quantum::ToggleTargetFinderAction>);
    RegisterAssetFactory("TrailSpawnerSpecAsset", &Make<Quantum::TrailSpawnerSpec>);
    RegisterAssetFactory("TrainingGameModeSpecAsset", &Make<Quantum::TrainingGameModeSpec>);
    RegisterAssetFactory("TravolatorStaticTriggerConfigAsset", &Make<Quantum::TravolatorStaticTriggerConfig>);
    RegisterAssetFactory("TrueDecisionAsset", &Make<Quantum::TrueDecision>);
    RegisterAssetFactory("TurnTowardTargetActionAsset", &Make<Quantum::TurnTowardTargetAction>);
    RegisterAssetFactory("TutorialGameModeSpecAsset", &Make<Quantum::TutorialGameModeSpec>);
    RegisterAssetFactory("UTRootAsset", &Make<Quantum::UTRoot>);
    RegisterAssetFactory("UltimateAbilityUserSpecAsset", &Make<Quantum::UltimateAbilityUserSpec>);
    RegisterAssetFactory("UltimateEvolutionSpecAsset", &Make<Quantum::UltimateEvolutionSpec>);
    RegisterAssetFactory("UltimateSlamSpecAsset", &Make<Quantum::UltimateSlamSpec>);
    RegisterAssetFactory("UltimateTankSpecAsset", &Make<Quantum::UltimateTankSpec>);
    RegisterAssetFactory("UltimateWeaponSpecAsset", &Make<Quantum::UltimateWeaponSpec>);
    RegisterAssetFactory("UpdateStrikeZonePositionActionAsset", &Make<Quantum::UpdateStrikeZonePositionAction>);
    RegisterAssetFactory("UseGadgetAsset", &Make<Quantum::UseGadget>);
    RegisterAssetFactory("UseGadgetEvaluatorAsset", &Make<Quantum::UseGadgetEvaluator>);
    RegisterAssetFactory("UseWrenchAsset", &Make<Quantum::UseWrench>);
    RegisterAssetFactory("UseWrenchEvaluatorAsset", &Make<Quantum::UseWrenchEvaluator>);
    RegisterAssetFactory("VoidRepulserBulldozerEnhancementSpecAsset", &Make<Quantum::VoidRepulserBulldozerEnhancementSpec>);
    RegisterAssetFactory("VoidRepulserSpeedBoostEnhancementSpecAsset", &Make<Quantum::VoidRepulserSpeedBoostEnhancementSpec>);
    RegisterAssetFactory("VoidRepulserVoidWaveEnhancementSpecAsset", &Make<Quantum::VoidRepulserVoidWaveEnhancementSpec>);
    RegisterAssetFactory("VoidRepulserWeaponSpecAsset", &Make<Quantum::VoidRepulserWeaponSpec>);
    RegisterAssetFactory("WaitLeafAsset", &Make<Quantum::WaitLeaf>);
    RegisterAssetFactory("WallRunningSpecAsset", &Make<Quantum::WallRunningSpec>);
    RegisterAssetFactory("WanderAsset", &Make<Quantum::Wander>);
    RegisterAssetFactory("WanderEvaluatorAsset", &Make<Quantum::WanderEvaluator>);
    RegisterAssetFactory("WarmongerMiniRocketsEnhancementSpecAsset", &Make<Quantum::WarmongerMiniRocketsEnhancementSpec>);
    RegisterAssetFactory("WaveManagerSpecAsset", &Make<Quantum::WaveManagerSpec>);
    RegisterAssetFactory("WaveReferencesAsset", &Make<Quantum::WaveReferences>);
    RegisterAssetFactory("WeaponBehaviour_ChargingSpecAsset", &Make<Quantum::WeaponBehaviour_ChargingSpec>);
    RegisterAssetFactory("WeaponChargingStatsModifierEnhancementSpecAsset", &Make<Quantum::WeaponChargingStatsModifierEnhancementSpec>);
    RegisterAssetFactory("WeaponLoadoutSpecAsset", &Make<Quantum::WeaponLoadoutSpec>);
    RegisterAssetFactory("WeaponProjectileSwitcherEnhancementSpecAsset", &Make<Quantum::WeaponProjectileSwitcherEnhancementSpec>);
    RegisterAssetFactory("WeaponShowcaseGameModeSpecAsset", &Make<Quantum::WeaponShowcaseGameModeSpec>);
    RegisterAssetFactory("WeaponSpecDefaultAsset", &Make<Quantum::WeaponSpecDefault>);
    RegisterAssetFactory("WeaponStatsModifierEnhancementSpecAsset", &Make<Quantum::WeaponStatsModifierEnhancementSpec>);
    RegisterAssetFactory("WeaponStatsModifierHeroImprovementSpecAsset", &Make<Quantum::WeaponStatsModifierHeroImprovementSpec>);
    RegisterAssetFactory("WeaponUserSpecAsset", &Make<Quantum::WeaponUserSpec>);
    RegisterAssetFactory("WithinRangeDecisionAsset", &Make<Quantum::WithinRangeDecision>);
    RegisterAssetFactory("WrenchGroundSlamEnhancementSpecAsset", &Make<Quantum::WrenchGroundSlamEnhancementSpec>);
    RegisterAssetFactory("WrenchThrowEnhancementSpecAsset", &Make<Quantum::WrenchThrowEnhancementSpec>);
    RegisterAssetFactory("WrenchWeaponSpecAsset", &Make<Quantum::WrenchWeaponSpec>);
    RegisterAssetFactory("ZoneControlGameModeSpecAsset", &Make<Quantum::ZoneControlGameModeSpec>);
}

} // namespace rr::content
