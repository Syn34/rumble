// component_prototype_visitor.h - double dispatch over every component prototype of the
// game (Quantum.ComponentPrototypeVisitor, quantum.code.dll).
//
// Extends the framework's visitor with one Visit per gameplay component prototype. Each
// defaults to VisitFallback (which throws), so a visitor overrides what it handles. The
// framework's own Visit overloads stay visible through the using-declaration.
#pragma once

#include "quantum_core/component_prototypes.h"

namespace Quantum {

namespace Prototypes {
class AIBlackboardComponent_Prototype;
class AIMoveForwardBehaviour_Prototype;
class AINavigationInitialiser_Prototype;
class AISpawner_Prototype;
class ActorInputInitialiser_Prototype;
class ActorManager_Prototype;
class ActorStatus_Prototype;
class ActorStatusModifier_Prototype;
class ActorStunned_Prototype;
class AddDamageIncreaseOnTargetedEffect_Prototype;
class AddEntitiesToSpawnOnProjectileDeath_Prototype;
class AgentOfDoom_Prototype;
class AgentOfDoomGrouped_Prototype;
class AmoeboidLauncherEnhancementPuddle_Prototype;
class AoeTrail_Prototype;
class AreaOfEffect_Prototype;
class AreaOfEffectLevelUp_Prototype;
class AreaOfEffectRaycastInfo_Prototype;
class ArenaCannon_Prototype;
class AttackInfo_Prototype;
class BTAgent_Prototype;
class BattleStatsActor_Prototype;
class BigBoomMine_Prototype;
class BigBoomWeapon_Prototype;
class BlackboardInitializer_Prototype;
class BlastBall_Prototype;
class BlastBallGame_Prototype;
class BlastBallGoal_Prototype;
class BlastBallPlayer_Prototype;
class BlasterMoveSpeedEnhancement_Prototype;
class BlasterSpecialShotEnhancement_Prototype;
class BoltCounterElement_Prototype;
class BoltGathererActor_Prototype;
class BoltPowerActor_Prototype;
class BombardierDrone_Prototype;
class BombardierWeapon_Prototype;
class Bot_Prototype;
class Bumper_Prototype;
class BumperBar_Prototype;
class BuzzbladesRicochet_Prototype;
class CharacterColliderScaler_Prototype;
class ChargeAttack_Prototype;
class Checkpoint_Prototype;
class ColliderResize_Prototype;
class CollisionForce_Prototype;
class CombatParticipant_Prototype;
class CompoundHFSM_Prototype;
class Cryoslider_Prototype;
class CryosliderWeapon_Prototype;
class DamageChargeable_Prototype;
class DamageIncreasedOnTargetsEnhancement_Prototype;
class DamageMultiplier_Prototype;
class DamageMultiplierHeroImprovement_Prototype;
class DamageSource_Prototype;
class DamageToImpulseConverter_Prototype;
class Damageable_Prototype;
class DashActor_Prototype;
class DrillDash_Prototype;
class DroppableCollector_Prototype;
class DroppableEntity_Prototype;
class DroppableManager_Prototype;
class ElementalBuildUpMultiplier_Prototype;
class EliminationGameMode_Prototype;
class EnergyCellManager_Prototype;
class EnforcerFirstShotBonusEnhancement_Prototype;
class EntityDropper_Prototype;
class EntityInfo_Prototype;
class FireReactor_Prototype;
class GOAPAgent_Prototype;
class GOAPData_Prototype;
class GadgetUser_Prototype;
class GameManager_Prototype;
class GameModeActivator_Prototype;
class GameModeHandler_Prototype;
class GameModifiersManager_Prototype;
class GatlingWeapon_Prototype;
class GloveOfDoomWeapon_Prototype;
class GoldenBolt_Prototype;
class GoldenBoltManager_Prototype;
class GrinderActor_Prototype;
class GroundDashActor_Prototype;
class HFSMAgent_Prototype;
class HeadHunterWallHackEnhancement_Prototype;
class HealSource_Prototype;
class Health_Prototype;
class HelipackActor_Prototype;
class Hierarchy_Prototype;
class HoloshieldGloveHealZoneEnhancement_Prototype;
class HoloshieldGloveShield_Prototype;
class HoloshieldGloveShields_Prototype;
class HoloshieldGloveShieldsGrouped_Prototype;
class HoloshieldGloveWeapon_Prototype;
class HoverBootsWeapon_Prototype;
class ItemSpawner_Prototype;
class JumpAttack_Prototype;
class KccExtension_Prototype;
class KccModifier_Prototype;
class KillZone_Prototype;
class Killable_Prototype;
class LedgeGrabbingActor_Prototype;
class LevelButton_Prototype;
class LevelButtonGameplayElement_Prototype;
class LevelHolder_Prototype;
class LevelUpActor_Prototype;
class Lifetime_Prototype;
class MeleeAttack_Prototype;
class MeleeJumpAttack_Prototype;
class MineLauncherEnhancement_Prototype;
class MineMerger_Prototype;
class MobGroup_Prototype;
class MobGroupMember_Prototype;
class MobInfo_Prototype;
class MoveTo_Prototype;
class MovementPackCache_Prototype;
class MovingWall_Prototype;
class OnKillCallbacks_Prototype;
class OnTriggerEnter_Prototype;
class PlayerActor_Prototype;
class PowerUp_Prototype;
class PowerUpUser_Prototype;
class PreciseHitReceiver_Prototype;
class PrioritisedTargetHandler_Prototype;
class Projectile_Prototype;
class ProjectileSpawner_Prototype;
class Pushable_Prototype;
class PushableTrigger_Prototype;
class Pusher_Prototype;
class Rail_Prototype;
class RailCollider_Prototype;
class RankingManager_Prototype;
class RecoverAmmoOnDashEnhancement_Prototype;
class ReplaceMultiShotConfigsEnhancement_Prototype;
class RewardReceiver_Prototype;
class Roll_Prototype;
class RunnableWall_Prototype;
class RunnableWallTriggerZone_Prototype;
class RynoWeapon_Prototype;
class ShapeOverlapper_Prototype;
class ShieldedActor_Prototype;
class ShotgunBerserkerEnhancement_Prototype;
class ShotgunEnhancement_Prototype;
class ShrinkingZone_Prototype;
class ShrinkingZoneActor_Prototype;
class SpawnProjectilesOnDestroy_Prototype;
class Speeder_Prototype;
class StatsManager_Prototype;
class StreamEffect_Prototype;
class StreamEffectSpawner_Prototype;
class SweepAttack_Prototype;
class TargetFinder_Prototype;
class TargetHolder_Prototype;
class Targetable_Prototype;
class TargetedEffect_Prototype;
class Team_Prototype;
class TeslaClawMoreBeam_Prototype;
class TimeScale_Prototype;
class TrailSpawner_Prototype;
class TravolatorActor_Prototype;
class TurnTo_Prototype;
class TutorialGameMode_Prototype;
class UTAgent_Prototype;
class UltimateAbilityUser_Prototype;
class UltimateEvolution_Prototype;
class UltimateTank_Prototype;
class UnlockedFeaturesManager_Prototype;
class VoidRepulserShield_Prototype;
class VoidRepulserSpeedBoostEnhancement_Prototype;
class VoidRepulserVoidWaveEnhancement_Prototype;
class VoidRepulserWeapon_Prototype;
class WakeUpSurroundingsOnPushOrDestroy_Prototype;
class WallRunningActor_Prototype;
class WarmongerMiniRocketsEnhancement_Prototype;
class WaveElement_Prototype;
class WaveManager_Prototype;
class WaveSpawner_Prototype;
class Weapon_Prototype;
class WeaponBehaviour_Charging_Prototype;
class WeaponLoadoutActor_Prototype;
class WeaponProjectileSwitcherEnhancement_Prototype;
class WeaponShowcaseGameMode_Prototype;
class WeaponUser_Prototype;
class WrenchThrowProjectile_Prototype;
class WrenchThrowWeapon_Prototype;
class WrenchWeapon_Prototype;
class ZoneControlGameMode_Prototype;
} // namespace Prototypes

class ComponentPrototypeVisitor : public Prototypes::ComponentPrototypeVisitorBase {
  public:
    using Prototypes::ComponentPrototypeVisitorBase::Visit;

    // 0x7c96b3c
    ComponentPrototypeVisitor() = default;

    // 0x7c95fac
    virtual void Visit(Prototypes::AIBlackboardComponent_Prototype& prototype);
    // 0x7c95fbc
    virtual void Visit(Prototypes::AIMoveForwardBehaviour_Prototype& prototype);
    // 0x7c95fcc
    virtual void Visit(Prototypes::AINavigationInitialiser_Prototype& prototype);
    // 0x7c95fdc
    virtual void Visit(Prototypes::AISpawner_Prototype& prototype);
    // 0x7c95fec
    virtual void Visit(Prototypes::ActorInputInitialiser_Prototype& prototype);
    // 0x7c95ffc
    virtual void Visit(Prototypes::ActorManager_Prototype& prototype);
    // 0x7c9600c
    virtual void Visit(Prototypes::ActorStatus_Prototype& prototype);
    // 0x7c9601c
    virtual void Visit(Prototypes::ActorStatusModifier_Prototype& prototype);
    // 0x7c9602c
    virtual void Visit(Prototypes::ActorStunned_Prototype& prototype);
    // 0x7c9603c
    virtual void Visit(Prototypes::AddDamageIncreaseOnTargetedEffect_Prototype& prototype);
    // 0x7c9604c
    virtual void Visit(Prototypes::AddEntitiesToSpawnOnProjectileDeath_Prototype& prototype);
    // 0x7c9605c
    virtual void Visit(Prototypes::AgentOfDoom_Prototype& prototype);
    // 0x7c9606c
    virtual void Visit(Prototypes::AgentOfDoomGrouped_Prototype& prototype);
    // 0x7c9607c
    virtual void Visit(Prototypes::AmoeboidLauncherEnhancementPuddle_Prototype& prototype);
    // 0x7c9608c
    virtual void Visit(Prototypes::AoeTrail_Prototype& prototype);
    // 0x7c9609c
    virtual void Visit(Prototypes::AreaOfEffect_Prototype& prototype);
    // 0x7c960ac
    virtual void Visit(Prototypes::AreaOfEffectLevelUp_Prototype& prototype);
    // 0x7c960bc
    virtual void Visit(Prototypes::AreaOfEffectRaycastInfo_Prototype& prototype);
    // 0x7c960cc
    virtual void Visit(Prototypes::ArenaCannon_Prototype& prototype);
    // 0x7c960dc
    virtual void Visit(Prototypes::AttackInfo_Prototype& prototype);
    // 0x7c960ec
    virtual void Visit(Prototypes::BTAgent_Prototype& prototype);
    // 0x7c960fc
    virtual void Visit(Prototypes::BattleStatsActor_Prototype& prototype);
    // 0x7c9610c
    virtual void Visit(Prototypes::BigBoomMine_Prototype& prototype);
    // 0x7c9611c
    virtual void Visit(Prototypes::BigBoomWeapon_Prototype& prototype);
    // 0x7c9612c
    virtual void Visit(Prototypes::BlackboardInitializer_Prototype& prototype);
    // 0x7c9613c
    virtual void Visit(Prototypes::BlastBall_Prototype& prototype);
    // 0x7c9614c
    virtual void Visit(Prototypes::BlastBallGame_Prototype& prototype);
    // 0x7c9615c
    virtual void Visit(Prototypes::BlastBallGoal_Prototype& prototype);
    // 0x7c9616c
    virtual void Visit(Prototypes::BlastBallPlayer_Prototype& prototype);
    // 0x7c9617c
    virtual void Visit(Prototypes::BlasterMoveSpeedEnhancement_Prototype& prototype);
    // 0x7c9618c
    virtual void Visit(Prototypes::BlasterSpecialShotEnhancement_Prototype& prototype);
    // 0x7c9619c
    virtual void Visit(Prototypes::BoltCounterElement_Prototype& prototype);
    // 0x7c961ac
    virtual void Visit(Prototypes::BoltGathererActor_Prototype& prototype);
    // 0x7c961bc
    virtual void Visit(Prototypes::BoltPowerActor_Prototype& prototype);
    // 0x7c961cc
    virtual void Visit(Prototypes::BombardierDrone_Prototype& prototype);
    // 0x7c961dc
    virtual void Visit(Prototypes::BombardierWeapon_Prototype& prototype);
    // 0x7c961ec
    virtual void Visit(Prototypes::Bot_Prototype& prototype);
    // 0x7c961fc
    virtual void Visit(Prototypes::Bumper_Prototype& prototype);
    // 0x7c9620c
    virtual void Visit(Prototypes::BumperBar_Prototype& prototype);
    // 0x7c9621c
    virtual void Visit(Prototypes::BuzzbladesRicochet_Prototype& prototype);
    // 0x7c9622c
    virtual void Visit(Prototypes::CharacterColliderScaler_Prototype& prototype);
    // 0x7c9623c
    virtual void Visit(Prototypes::ChargeAttack_Prototype& prototype);
    // 0x7c9624c
    virtual void Visit(Prototypes::Checkpoint_Prototype& prototype);
    // 0x7c9625c
    virtual void Visit(Prototypes::ColliderResize_Prototype& prototype);
    // 0x7c9626c
    virtual void Visit(Prototypes::CollisionForce_Prototype& prototype);
    // 0x7c9627c
    virtual void Visit(Prototypes::CombatParticipant_Prototype& prototype);
    // 0x7c9628c
    virtual void Visit(Prototypes::CompoundHFSM_Prototype& prototype);
    // 0x7c9629c
    virtual void Visit(Prototypes::Cryoslider_Prototype& prototype);
    // 0x7c962ac
    virtual void Visit(Prototypes::CryosliderWeapon_Prototype& prototype);
    // 0x7c962bc
    virtual void Visit(Prototypes::DamageChargeable_Prototype& prototype);
    // 0x7c962cc
    virtual void Visit(Prototypes::DamageIncreasedOnTargetsEnhancement_Prototype& prototype);
    // 0x7c962dc
    virtual void Visit(Prototypes::DamageMultiplier_Prototype& prototype);
    // 0x7c962ec
    virtual void Visit(Prototypes::DamageMultiplierHeroImprovement_Prototype& prototype);
    // 0x7c962fc
    virtual void Visit(Prototypes::DamageSource_Prototype& prototype);
    // 0x7c9630c
    virtual void Visit(Prototypes::DamageToImpulseConverter_Prototype& prototype);
    // 0x7c9631c
    virtual void Visit(Prototypes::Damageable_Prototype& prototype);
    // 0x7c9632c
    virtual void Visit(Prototypes::DashActor_Prototype& prototype);
    // 0x7c9633c
    virtual void Visit(Prototypes::DrillDash_Prototype& prototype);
    // 0x7c9634c
    virtual void Visit(Prototypes::DroppableCollector_Prototype& prototype);
    // 0x7c9635c
    virtual void Visit(Prototypes::DroppableEntity_Prototype& prototype);
    // 0x7c9636c
    virtual void Visit(Prototypes::DroppableManager_Prototype& prototype);
    // 0x7c9637c
    virtual void Visit(Prototypes::ElementalBuildUpMultiplier_Prototype& prototype);
    // 0x7c9638c
    virtual void Visit(Prototypes::EliminationGameMode_Prototype& prototype);
    // 0x7c9639c
    virtual void Visit(Prototypes::EnergyCellManager_Prototype& prototype);
    // 0x7c963ac
    virtual void Visit(Prototypes::EnforcerFirstShotBonusEnhancement_Prototype& prototype);
    // 0x7c963bc
    virtual void Visit(Prototypes::EntityDropper_Prototype& prototype);
    // 0x7c963cc
    virtual void Visit(Prototypes::EntityInfo_Prototype& prototype);
    // 0x7c963dc
    virtual void Visit(Prototypes::FireReactor_Prototype& prototype);
    // 0x7c963ec
    virtual void Visit(Prototypes::GOAPAgent_Prototype& prototype);
    // 0x7c963fc
    virtual void Visit(Prototypes::GOAPData_Prototype& prototype);
    // 0x7c9640c
    virtual void Visit(Prototypes::GadgetUser_Prototype& prototype);
    // 0x7c9641c
    virtual void Visit(Prototypes::GameManager_Prototype& prototype);
    // 0x7c9642c
    virtual void Visit(Prototypes::GameModeActivator_Prototype& prototype);
    // 0x7c9643c
    virtual void Visit(Prototypes::GameModeHandler_Prototype& prototype);
    // 0x7c9644c
    virtual void Visit(Prototypes::GameModifiersManager_Prototype& prototype);
    // 0x7c9645c
    virtual void Visit(Prototypes::GatlingWeapon_Prototype& prototype);
    // 0x7c9646c
    virtual void Visit(Prototypes::GloveOfDoomWeapon_Prototype& prototype);
    // 0x7c9647c
    virtual void Visit(Prototypes::GoldenBolt_Prototype& prototype);
    // 0x7c9648c
    virtual void Visit(Prototypes::GoldenBoltManager_Prototype& prototype);
    // 0x7c9649c
    virtual void Visit(Prototypes::GrinderActor_Prototype& prototype);
    // 0x7c964ac
    virtual void Visit(Prototypes::GroundDashActor_Prototype& prototype);
    // 0x7c964bc
    virtual void Visit(Prototypes::HFSMAgent_Prototype& prototype);
    // 0x7c964cc
    virtual void Visit(Prototypes::HeadHunterWallHackEnhancement_Prototype& prototype);
    // 0x7c964dc
    virtual void Visit(Prototypes::HealSource_Prototype& prototype);
    // 0x7c964ec
    virtual void Visit(Prototypes::Health_Prototype& prototype);
    // 0x7c964fc
    virtual void Visit(Prototypes::HelipackActor_Prototype& prototype);
    // 0x7c9650c
    virtual void Visit(Prototypes::Hierarchy_Prototype& prototype);
    // 0x7c9651c
    virtual void Visit(Prototypes::HoloshieldGloveHealZoneEnhancement_Prototype& prototype);
    // 0x7c9652c
    virtual void Visit(Prototypes::HoloshieldGloveShield_Prototype& prototype);
    // 0x7c9653c
    virtual void Visit(Prototypes::HoloshieldGloveShields_Prototype& prototype);
    // 0x7c9654c
    virtual void Visit(Prototypes::HoloshieldGloveShieldsGrouped_Prototype& prototype);
    // 0x7c9655c
    virtual void Visit(Prototypes::HoloshieldGloveWeapon_Prototype& prototype);
    // 0x7c9656c
    virtual void Visit(Prototypes::HoverBootsWeapon_Prototype& prototype);
    // 0x7c9657c
    virtual void Visit(Prototypes::ItemSpawner_Prototype& prototype);
    // 0x7c9658c
    virtual void Visit(Prototypes::JumpAttack_Prototype& prototype);
    // 0x7c9659c
    virtual void Visit(Prototypes::KccExtension_Prototype& prototype);
    // 0x7c965ac
    virtual void Visit(Prototypes::KccModifier_Prototype& prototype);
    // 0x7c965bc
    virtual void Visit(Prototypes::KillZone_Prototype& prototype);
    // 0x7c965cc
    virtual void Visit(Prototypes::Killable_Prototype& prototype);
    // 0x7c965dc
    virtual void Visit(Prototypes::LedgeGrabbingActor_Prototype& prototype);
    // 0x7c965ec
    virtual void Visit(Prototypes::LevelButton_Prototype& prototype);
    // 0x7c965fc
    virtual void Visit(Prototypes::LevelButtonGameplayElement_Prototype& prototype);
    // 0x7c9660c
    virtual void Visit(Prototypes::LevelHolder_Prototype& prototype);
    // 0x7c9661c
    virtual void Visit(Prototypes::LevelUpActor_Prototype& prototype);
    // 0x7c9662c
    virtual void Visit(Prototypes::Lifetime_Prototype& prototype);
    // 0x7c9663c
    virtual void Visit(Prototypes::MeleeAttack_Prototype& prototype);
    // 0x7c9664c
    virtual void Visit(Prototypes::MeleeJumpAttack_Prototype& prototype);
    // 0x7c9665c
    virtual void Visit(Prototypes::MineLauncherEnhancement_Prototype& prototype);
    // 0x7c9666c
    virtual void Visit(Prototypes::MineMerger_Prototype& prototype);
    // 0x7c9667c
    virtual void Visit(Prototypes::MobGroup_Prototype& prototype);
    // 0x7c9668c
    virtual void Visit(Prototypes::MobGroupMember_Prototype& prototype);
    // 0x7c9669c
    virtual void Visit(Prototypes::MobInfo_Prototype& prototype);
    // 0x7c966ac
    virtual void Visit(Prototypes::MoveTo_Prototype& prototype);
    // 0x7c966bc
    virtual void Visit(Prototypes::MovementPackCache_Prototype& prototype);
    // 0x7c966cc
    virtual void Visit(Prototypes::MovingWall_Prototype& prototype);
    // 0x7c966dc
    virtual void Visit(Prototypes::OnKillCallbacks_Prototype& prototype);
    // 0x7c966ec
    virtual void Visit(Prototypes::OnTriggerEnter_Prototype& prototype);
    // 0x7c966fc
    virtual void Visit(Prototypes::PlayerActor_Prototype& prototype);
    // 0x7c9670c
    virtual void Visit(Prototypes::PowerUp_Prototype& prototype);
    // 0x7c9671c
    virtual void Visit(Prototypes::PowerUpUser_Prototype& prototype);
    // 0x7c9672c
    virtual void Visit(Prototypes::PreciseHitReceiver_Prototype& prototype);
    // 0x7c9673c
    virtual void Visit(Prototypes::PrioritisedTargetHandler_Prototype& prototype);
    // 0x7c9674c
    virtual void Visit(Prototypes::Projectile_Prototype& prototype);
    // 0x7c9675c
    virtual void Visit(Prototypes::ProjectileSpawner_Prototype& prototype);
    // 0x7c9676c
    virtual void Visit(Prototypes::Pushable_Prototype& prototype);
    // 0x7c9677c
    virtual void Visit(Prototypes::PushableTrigger_Prototype& prototype);
    // 0x7c9678c
    virtual void Visit(Prototypes::Pusher_Prototype& prototype);
    // 0x7c9679c
    virtual void Visit(Prototypes::Rail_Prototype& prototype);
    // 0x7c967ac
    virtual void Visit(Prototypes::RailCollider_Prototype& prototype);
    // 0x7c967bc
    virtual void Visit(Prototypes::RankingManager_Prototype& prototype);
    // 0x7c967cc
    virtual void Visit(Prototypes::RecoverAmmoOnDashEnhancement_Prototype& prototype);
    // 0x7c967dc
    virtual void Visit(Prototypes::ReplaceMultiShotConfigsEnhancement_Prototype& prototype);
    // 0x7c967ec
    virtual void Visit(Prototypes::RewardReceiver_Prototype& prototype);
    // 0x7c967fc
    virtual void Visit(Prototypes::Roll_Prototype& prototype);
    // 0x7c9680c
    virtual void Visit(Prototypes::RunnableWall_Prototype& prototype);
    // 0x7c9681c
    virtual void Visit(Prototypes::RunnableWallTriggerZone_Prototype& prototype);
    // 0x7c9682c
    virtual void Visit(Prototypes::RynoWeapon_Prototype& prototype);
    // 0x7c9683c
    virtual void Visit(Prototypes::ShapeOverlapper_Prototype& prototype);
    // 0x7c9684c
    virtual void Visit(Prototypes::ShieldedActor_Prototype& prototype);
    // 0x7c9685c
    virtual void Visit(Prototypes::ShotgunBerserkerEnhancement_Prototype& prototype);
    // 0x7c9686c
    virtual void Visit(Prototypes::ShotgunEnhancement_Prototype& prototype);
    // 0x7c9687c
    virtual void Visit(Prototypes::ShrinkingZone_Prototype& prototype);
    // 0x7c9688c
    virtual void Visit(Prototypes::ShrinkingZoneActor_Prototype& prototype);
    // 0x7c9689c
    virtual void Visit(Prototypes::SpawnProjectilesOnDestroy_Prototype& prototype);
    // 0x7c968ac
    virtual void Visit(Prototypes::Speeder_Prototype& prototype);
    // 0x7c968bc
    virtual void Visit(Prototypes::StatsManager_Prototype& prototype);
    // 0x7c968cc
    virtual void Visit(Prototypes::StreamEffect_Prototype& prototype);
    // 0x7c968dc
    virtual void Visit(Prototypes::StreamEffectSpawner_Prototype& prototype);
    // 0x7c968ec
    virtual void Visit(Prototypes::SweepAttack_Prototype& prototype);
    // 0x7c968fc
    virtual void Visit(Prototypes::TargetFinder_Prototype& prototype);
    // 0x7c9690c
    virtual void Visit(Prototypes::TargetHolder_Prototype& prototype);
    // 0x7c9691c
    virtual void Visit(Prototypes::Targetable_Prototype& prototype);
    // 0x7c9692c
    virtual void Visit(Prototypes::TargetedEffect_Prototype& prototype);
    // 0x7c9693c
    virtual void Visit(Prototypes::Team_Prototype& prototype);
    // 0x7c9694c
    virtual void Visit(Prototypes::TeslaClawMoreBeam_Prototype& prototype);
    // 0x7c9695c
    virtual void Visit(Prototypes::TimeScale_Prototype& prototype);
    // 0x7c9696c
    virtual void Visit(Prototypes::TrailSpawner_Prototype& prototype);
    // 0x7c9697c
    virtual void Visit(Prototypes::TravolatorActor_Prototype& prototype);
    // 0x7c9698c
    virtual void Visit(Prototypes::TurnTo_Prototype& prototype);
    // 0x7c9699c
    virtual void Visit(Prototypes::TutorialGameMode_Prototype& prototype);
    // 0x7c969ac
    virtual void Visit(Prototypes::UTAgent_Prototype& prototype);
    // 0x7c969bc
    virtual void Visit(Prototypes::UltimateAbilityUser_Prototype& prototype);
    // 0x7c969cc
    virtual void Visit(Prototypes::UltimateEvolution_Prototype& prototype);
    // 0x7c969dc
    virtual void Visit(Prototypes::UltimateTank_Prototype& prototype);
    // 0x7c969ec
    virtual void Visit(Prototypes::UnlockedFeaturesManager_Prototype& prototype);
    // 0x7c969fc
    virtual void Visit(Prototypes::VoidRepulserShield_Prototype& prototype);
    // 0x7c96a0c
    virtual void Visit(Prototypes::VoidRepulserSpeedBoostEnhancement_Prototype& prototype);
    // 0x7c96a1c
    virtual void Visit(Prototypes::VoidRepulserVoidWaveEnhancement_Prototype& prototype);
    // 0x7c96a2c
    virtual void Visit(Prototypes::VoidRepulserWeapon_Prototype& prototype);
    // 0x7c96a3c
    virtual void Visit(Prototypes::WakeUpSurroundingsOnPushOrDestroy_Prototype& prototype);
    // 0x7c96a4c
    virtual void Visit(Prototypes::WallRunningActor_Prototype& prototype);
    // 0x7c96a5c
    virtual void Visit(Prototypes::WarmongerMiniRocketsEnhancement_Prototype& prototype);
    // 0x7c96a6c
    virtual void Visit(Prototypes::WaveElement_Prototype& prototype);
    // 0x7c96a7c
    virtual void Visit(Prototypes::WaveManager_Prototype& prototype);
    // 0x7c96a8c
    virtual void Visit(Prototypes::WaveSpawner_Prototype& prototype);
    // 0x7c96a9c
    virtual void Visit(Prototypes::Weapon_Prototype& prototype);
    // 0x7c96aac
    virtual void Visit(Prototypes::WeaponBehaviour_Charging_Prototype& prototype);
    // 0x7c96abc
    virtual void Visit(Prototypes::WeaponLoadoutActor_Prototype& prototype);
    // 0x7c96acc
    virtual void Visit(Prototypes::WeaponProjectileSwitcherEnhancement_Prototype& prototype);
    // 0x7c96adc
    virtual void Visit(Prototypes::WeaponShowcaseGameMode_Prototype& prototype);
    // 0x7c96aec
    virtual void Visit(Prototypes::WeaponUser_Prototype& prototype);
    // 0x7c96afc
    virtual void Visit(Prototypes::WrenchThrowProjectile_Prototype& prototype);
    // 0x7c96b0c
    virtual void Visit(Prototypes::WrenchThrowWeapon_Prototype& prototype);
    // 0x7c96b1c
    virtual void Visit(Prototypes::WrenchWeapon_Prototype& prototype);
    // 0x7c96b2c
    virtual void Visit(Prototypes::ZoneControlGameMode_Prototype& prototype);
};

} // namespace Quantum
