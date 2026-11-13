// prototype_readers.cpp - reads the game's component prototypes from decoded content data
// (PC-side glue; new code).
//
// Prototype fields are serialised under their original names; the value shapes are those
// of settings_reader.h, plus MapEntityId { SceneIndexPlusOne } and the enum prototypes
// { Value }. Unknown prototype type names yield null.
#include "content/prototype_readers.h"

#include "content/settings_reader.h"
#include "quantum_code/prototypes/flat_entity_prototype_container.h"

#include <functional>
#include <unordered_map>

namespace rr::content {

using namespace Quantum::Prototypes;

void ReadFields(const Value& v, UTMomentumData_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UTMomentumPack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DictionaryEntry_AssetRefConsideration_FP_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UtilityReasoner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, SpawnConfig_Prototype& out, ReaderTag);
void ReadFields(const Value& v, KillZoneSurfaceType_Prototype& out, ReaderTag);
void ReadFields(const Value& v, KillZoneArea_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HFSMData_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BTDataValue_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlackboardValue_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlackboardEntry_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DictionaryEntry_Int32_BlackboardEntry_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EntityInfoType_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GAME_MODIFICATOR_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MobType_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AIBlackboardComponent_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AIMoveForwardBehaviour_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AINavigationInitialiser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AISpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ActorInputInitialiser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ActorManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ActorStatus_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ActorStatusModifier_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ActorStunned_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AddDamageIncreaseOnTargetedEffect_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AddEntitiesToSpawnOnProjectileDeath_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AgentOfDoom_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AgentOfDoomGrouped_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AmoeboidLauncherEnhancementPuddle_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AoeTrail_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AreaOfEffect_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AreaOfEffectLevelUp_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AreaOfEffectRaycastInfo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ArenaCannon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, AttackInfo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BTAgent_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BattleStatsActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BigBoomMine_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BigBoomWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlackboardInitializer_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlastBall_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlastBallGame_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlastBallGoal_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlastBallPlayer_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlasterMoveSpeedEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BlasterSpecialShotEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BoltCounterElement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BoltGathererActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BoltPowerActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BombardierDrone_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BombardierWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Bot_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Bumper_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BumperBar_Prototype& out, ReaderTag);
void ReadFields(const Value& v, BuzzbladesRicochet_Prototype& out, ReaderTag);
void ReadFields(const Value& v, CharacterColliderScaler_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ChargeAttack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Checkpoint_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ColliderResize_Prototype& out, ReaderTag);
void ReadFields(const Value& v, CollisionForce_Prototype& out, ReaderTag);
void ReadFields(const Value& v, CombatParticipant_Prototype& out, ReaderTag);
void ReadFields(const Value& v, CompoundHFSM_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Cryoslider_Prototype& out, ReaderTag);
void ReadFields(const Value& v, CryosliderWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageChargeable_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageIncreasedOnTargetsEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageMultiplier_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageMultiplierHeroImprovement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageSource_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DamageToImpulseConverter_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Damageable_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DashActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DrillDash_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DroppableCollector_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DroppableEntity_Prototype& out, ReaderTag);
void ReadFields(const Value& v, DroppableManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ElementalBuildUpMultiplier_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EliminationGameMode_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EnergyCellManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EnforcerFirstShotBonusEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EntityDropper_Prototype& out, ReaderTag);
void ReadFields(const Value& v, EntityInfo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, FireReactor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GOAPAgent_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GOAPData_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GadgetUser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GameManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GameModeActivator_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GameModeHandler_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GameModifiersManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GatlingWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GloveOfDoomWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GoldenBolt_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GoldenBoltManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GrinderActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, GroundDashActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HFSMAgent_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HeadHunterWallHackEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HealSource_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Health_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HelipackActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Hierarchy_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoloshieldGloveHealZoneEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoloshieldGloveShield_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoloshieldGloveShields_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoloshieldGloveShieldsGrouped_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoloshieldGloveWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, HoverBootsWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ItemSpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, JumpAttack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, KccExtension_Prototype& out, ReaderTag);
void ReadFields(const Value& v, KccModifier_Prototype& out, ReaderTag);
void ReadFields(const Value& v, KillZone_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Killable_Prototype& out, ReaderTag);
void ReadFields(const Value& v, LedgeGrabbingActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, LevelButton_Prototype& out, ReaderTag);
void ReadFields(const Value& v, LevelButtonGameplayElement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, LevelHolder_Prototype& out, ReaderTag);
void ReadFields(const Value& v, LevelUpActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Lifetime_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MeleeAttack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MeleeJumpAttack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MineLauncherEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MineMerger_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MobGroup_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MobGroupMember_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MobInfo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MoveTo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MovementPackCache_Prototype& out, ReaderTag);
void ReadFields(const Value& v, MovingWall_Prototype& out, ReaderTag);
void ReadFields(const Value& v, OnKillCallbacks_Prototype& out, ReaderTag);
void ReadFields(const Value& v, OnTriggerEnter_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PlayerActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PowerUp_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PowerUpUser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PreciseHitReceiver_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PrioritisedTargetHandler_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Projectile_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ProjectileSpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Pushable_Prototype& out, ReaderTag);
void ReadFields(const Value& v, PushableTrigger_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Pusher_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Rail_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RailCollider_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RankingManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RecoverAmmoOnDashEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ReplaceMultiShotConfigsEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RewardReceiver_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Roll_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RunnableWall_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RunnableWallTriggerZone_Prototype& out, ReaderTag);
void ReadFields(const Value& v, RynoWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShapeOverlapper_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShieldedActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShotgunBerserkerEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShotgunEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShrinkingZone_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ShrinkingZoneActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, SpawnProjectilesOnDestroy_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Speeder_Prototype& out, ReaderTag);
void ReadFields(const Value& v, StatsManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, StreamEffect_Prototype& out, ReaderTag);
void ReadFields(const Value& v, StreamEffectSpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, SweepAttack_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TargetFinder_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TargetHolder_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Targetable_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TargetedEffect_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Team_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TeslaClawMoreBeam_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TimeScale_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TrailSpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TravolatorActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TurnTo_Prototype& out, ReaderTag);
void ReadFields(const Value& v, TutorialGameMode_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UTAgent_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UltimateAbilityUser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UltimateEvolution_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UltimateTank_Prototype& out, ReaderTag);
void ReadFields(const Value& v, UnlockedFeaturesManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, VoidRepulserShield_Prototype& out, ReaderTag);
void ReadFields(const Value& v, VoidRepulserSpeedBoostEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, VoidRepulserVoidWaveEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, VoidRepulserWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WakeUpSurroundingsOnPushOrDestroy_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WallRunningActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WarmongerMiniRocketsEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WaveElement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WaveManager_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WaveSpawner_Prototype& out, ReaderTag);
void ReadFields(const Value& v, Weapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WeaponBehaviour_Charging_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WeaponLoadoutActor_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WeaponProjectileSwitcherEnhancement_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WeaponShowcaseGameMode_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WeaponUser_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WrenchThrowProjectile_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WrenchThrowWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, WrenchWeapon_Prototype& out, ReaderTag);
void ReadFields(const Value& v, ZoneControlGameMode_Prototype& out, ReaderTag);

void ReadFields(const Value& v, UTMomentumData_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
    Read(v, "DecayAmount", out.DecayAmount);
}
void ReadFields(const Value& v, UTMomentumPack_Prototype& out, ReaderTag) {
    Read(v, "ConsiderationRef", out.ConsiderationRef);
    Read(v, "MomentumData", out.MomentumData);
}
void ReadFields(const Value& v, DictionaryEntry_AssetRefConsideration_FP_Prototype& out,
                ReaderTag) {
    Read(v, "Key", out.Key);
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, UtilityReasoner_Prototype& out, ReaderTag) {
    Read(v, "UTRoot", out.UTRoot);
    Read(v, "Considerations", out.Considerations);
    Read(v, "HighRankConsiderations", out.HighRankConsiderations);
    Read(v, "MomentumList", out.MomentumList);
    Read(v, "TimeToTick", out.TimeToTick);
    Read(v, "CooldownsDict", out.CooldownsDict);
    Read(v, "PreviousExecution", out.PreviousExecution);
}
void ReadFields(const Value& v, SpawnConfig_Prototype& out, ReaderTag) {
    Read(v, "useRandomSpawnTransform", out.useRandomSpawnTransform);
    Read(v, "spawnPositionOffsetDistance", out.spawnPositionOffsetDistance);
    Read(v, "findPositionAttempts", out.findPositionAttempts);
    Read(v, "freePositionRadius", out.freePositionRadius);
    Read(v, "fallbackYOffset", out.fallbackYOffset);
}
void ReadFields(const Value& v, KillZoneSurfaceType_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, KillZoneArea_Prototype& out, ReaderTag) {
    Read(v, "corner", out.corner);
    Read(v, "size", out.size);
    Read(v, "surfaceType", out.surfaceType);
}
void ReadFields(const Value& v, HFSMData_Prototype& out, ReaderTag) {
    Read(v, "Root", out.Root);
    Read(v, "CurrentState", out.CurrentState);
    Read(v, "Time", out.Time);
    Read(v, "Times", out.Times);
    Read(v, "UsesStateHistory", out.UsesStateHistory);
    Read(v, "StatesHistory", out.StatesHistory);
}
void ReadFields(const Value& v, BTDataValue_Prototype& out, ReaderTag) {
    Read(v, "_field_used_", out._field_used_);
    Read(v, "FPValue", out.FPValue);
    Read(v, "IntValue", out.IntValue);
}
void ReadFields(const Value& v, BlackboardValue_Prototype& out, ReaderTag) {
    Read(v, "_field_used_", out._field_used_);
    Read(v, "BooleanValue", out.BooleanValue);
    Read(v, "ByteValue", out.ByteValue);
    Read(v, "IntegerValue", out.IntegerValue);
    Read(v, "FPValue", out.FPValue);
    Read(v, "FPVector2Value", out.FPVector2Value);
    Read(v, "FPVector3Value", out.FPVector3Value);
    Read(v, "EntityRefValue", out.EntityRefValue);
    Read(v, "AssetRefValue", out.AssetRefValue);
}
void ReadFields(const Value& v, BlackboardEntry_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
    Read(v, "ReactiveDecorators", out.ReactiveDecorators);
}
void ReadFields(const Value& v, DictionaryEntry_Int32_BlackboardEntry_Prototype& out, ReaderTag) {
    Read(v, "Key", out.Key);
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, EntityInfoType_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, GAME_MODIFICATOR_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, MobType_Prototype& out, ReaderTag) {
    Read(v, "Value", out.Value);
}
void ReadFields(const Value& v, AIBlackboardComponent_Prototype& out, ReaderTag) {
    Read(v, "Board", out.Board);
    Read(v, "Entries", out.Entries);
}
void ReadFields(const Value& v, AIMoveForwardBehaviour_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, AINavigationInitialiser_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, AISpawner_Prototype& out, ReaderTag) {
    Read(v, "EntityToSpawn", out.EntityToSpawn);
    Read(v, "StartEnabled", out.StartEnabled);
    Read(v, "NumToSpawn", out.NumToSpawn);
    Read(v, "WanderRadius", out.WanderRadius);
    Read(v, "InitialSpawnDelaySeconds", out.InitialSpawnDelaySeconds);
    Read(v, "SpawnIntervalSeconds", out.SpawnIntervalSeconds);
    Read(v, "SpawnConfig", out.SpawnConfig);
    Read(v, "AllowRespawnAfterDeath", out.AllowRespawnAfterDeath);
    Read(v, "AfterDeathSpawnDelaySeconds", out.AfterDeathSpawnDelaySeconds);
    Read(v, "configWasDeprecated", out.configWasDeprecated);
}
void ReadFields(const Value&, ActorInputInitialiser_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ActorManager_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, ActorStatus_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, ActorStatusModifier_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value&, ActorStunned_Prototype&, ReaderTag) {}
void ReadFields(const Value&, AddDamageIncreaseOnTargetedEffect_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, AddEntitiesToSpawnOnProjectileDeath_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, AgentOfDoom_Prototype&, ReaderTag) {}
void ReadFields(const Value&, AgentOfDoomGrouped_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, AmoeboidLauncherEnhancementPuddle_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, AoeTrail_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, AreaOfEffect_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, AreaOfEffectLevelUp_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, AreaOfEffectRaycastInfo_Prototype&, ReaderTag) {}
void ReadFields(const Value&, ArenaCannon_Prototype&, ReaderTag) {}
void ReadFields(const Value&, AttackInfo_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, BTAgent_Prototype& out, ReaderTag) {
    Read(v, "Tree", out.Tree);
    Read(v, "Current", out.Current);
    Read(v, "Config", out.Config);
    Read(v, "NodesStatus", out.NodesStatus);
    Read(v, "BTDataValues", out.BTDataValues);
    Read(v, "ActiveServices", out.ActiveServices);
    Read(v, "ServicesEndTimes", out.ServicesEndTimes);
    Read(v, "DynamicComposites", out.DynamicComposites);
    Read(v, "AbortNodeId", out.AbortNodeId);
}
void ReadFields(const Value&, BattleStatsActor_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, BigBoomMine_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, BigBoomWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, BlackboardInitializer_Prototype& out, ReaderTag) {
    Read(v, "BBInitializer", out.BBInitializer);
}
void ReadFields(const Value& v, BlastBall_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, BlastBallGame_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, BlastBallGoal_Prototype& out, ReaderTag) {
    Read(v, "OwnerTeam", out.OwnerTeam);
    Read(v, "ballEntity", out.ballEntity);
    Read(v, "ballInside", out.ballInside);
}
void ReadFields(const Value&, BlastBallPlayer_Prototype&, ReaderTag) {}
void ReadFields(const Value&, BlasterMoveSpeedEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, BlasterSpecialShotEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, BoltCounterElement_Prototype& out, ReaderTag) {
    Read(v, "requiredBolts", out.requiredBolts);
    Read(v, "gameplayElementConfig", out.gameplayElementConfig);
}
void ReadFields(const Value& v, BoltGathererActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, BoltPowerActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, BombardierDrone_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, BombardierWeapon_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, Bot_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, Bumper_Prototype& out, ReaderTag) {
    Read(v, "config", out.config);
    Read(v, "impulseStrength", out.impulseStrength);
    Read(v, "impulseDirection", out.impulseDirection);
}
void ReadFields(const Value& v, BumperBar_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, BuzzbladesRicochet_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, CharacterColliderScaler_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ChargeAttack_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, Checkpoint_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ColliderResize_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, CollisionForce_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, CombatParticipant_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, CompoundHFSM_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, Cryoslider_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, CryosliderWeapon_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, DamageChargeable_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, DamageIncreasedOnTargetsEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, DamageMultiplier_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, DamageMultiplierHeroImprovement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, DamageSource_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
    Read(v, "AmmunitionSpec", out.AmmunitionSpec);
    Read(v, "hitTrackingDone", out.hitTrackingDone);
}
void ReadFields(const Value& v, DamageToImpulseConverter_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Damageable_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, DashActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, DrillDash_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, DroppableCollector_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, DroppableEntity_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, DroppableManager_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, ElementalBuildUpMultiplier_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, EliminationGameMode_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, EnergyCellManager_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, EnforcerFirstShotBonusEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, EntityDropper_Prototype& out, ReaderTag) {
    Read(v, "Specs", out.Specs);
}
void ReadFields(const Value& v, EntityInfo_Prototype& out, ReaderTag) {
    Read(v, "type", out.type);
    Read(v, "id", out.id);
    Read(v, "isActiveObjective", out.isActiveObjective);
}
void ReadFields(const Value& v, FireReactor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, GOAPAgent_Prototype& out, ReaderTag) {
    Read(v, "Root", out.Root);
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, GOAPData_Prototype& out, ReaderTag) {
    Read(v, "HeuristicCost", out.HeuristicCost);
}
void ReadFields(const Value&, GadgetUser_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, GameManager_Prototype& out, ReaderTag) {
    Read(v, "nbPlayersSpawned", out.nbPlayersSpawned);
    Read(v, "config", out.config);
}
void ReadFields(const Value& v, GameModeActivator_Prototype& out, ReaderTag) {
    Read(v, "modificator", out.modificator);
    Read(v, "enabled", out.enabled);
}
void ReadFields(const Value& v, GameModeHandler_Prototype& out, ReaderTag) {
    Read(v, "config", out.config);
    Read(v, "levelConfig", out.levelConfig);
    Read(v, "gameModesTable", out.gameModesTable);
}
void ReadFields(const Value& v, GameModifiersManager_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, GatlingWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value&, GloveOfDoomWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, GoldenBolt_Prototype& out, ReaderTag) {
    Read(v, "isCollected", out.isCollected);
}
void ReadFields(const Value&, GoldenBoltManager_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, GrinderActor_Prototype& out, ReaderTag) {
    Read(v, "config", out.config);
}
void ReadFields(const Value& v, GroundDashActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, HFSMAgent_Prototype& out, ReaderTag) {
    Read(v, "Data", out.Data);
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, HeadHunterWallHackEnhancement_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, HealSource_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Health_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, HelipackActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Hierarchy_Prototype& out, ReaderTag) {
    Read(v, "Parent", out.Parent);
    Read(v, "Offset", out.Offset);
}
void ReadFields(const Value&, HoloshieldGloveHealZoneEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, HoloshieldGloveShield_Prototype&, ReaderTag) {}
void ReadFields(const Value&, HoloshieldGloveShields_Prototype&, ReaderTag) {}
void ReadFields(const Value&, HoloshieldGloveShieldsGrouped_Prototype&, ReaderTag) {}
void ReadFields(const Value&, HoloshieldGloveWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value&, HoverBootsWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ItemSpawner_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
    Read(v, "spawnedItemId", out.spawnedItemId);
}
void ReadFields(const Value& v, JumpAttack_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, KccExtension_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, KccModifier_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, KillZone_Prototype& out, ReaderTag) {
    Read(v, "areas", out.areas);
    Read(v, "verticalThreshold", out.verticalThreshold);
}
void ReadFields(const Value& v, Killable_Prototype& out, ReaderTag) {
    Read(v, "deathAnimationSpec", out.deathAnimationSpec);
    Read(v, "OnKillBehaviour", out.OnKillBehaviour);
    Read(v, "DelayBeforeDestruction", out.DelayBeforeDestruction);
    Read(v, "OnKillDisablePhysics", out.OnKillDisablePhysics);
    Read(v, "OnKillDisableCollider", out.OnKillDisableCollider);
    Read(v, "EntityToSpawnOnDeath", out.EntityToSpawnOnDeath);
}
void ReadFields(const Value& v, LedgeGrabbingActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, LevelButton_Prototype& out, ReaderTag) {
    Read(v, "gameplayElementEntity", out.gameplayElementEntity);
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, LevelButtonGameplayElement_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}
void ReadFields(const Value&, LevelHolder_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, LevelUpActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Lifetime_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, MeleeAttack_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, MeleeJumpAttack_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, MineLauncherEnhancement_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, MineMerger_Prototype& out, ReaderTag) {
    Read(v, "mergeCount", out.mergeCount);
    Read(v, "wasMergeChecked", out.wasMergeChecked);
}
void ReadFields(const Value& v, MobGroup_Prototype& out, ReaderTag) {
    Read(v, "SwarmerConfig", out.SwarmerConfig);
    Read(v, "NumToSpawn", out.NumToSpawn);
}
void ReadFields(const Value&, MobGroupMember_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, MobInfo_Prototype& out, ReaderTag) {
    Read(v, "Type", out.Type);
    Read(v, "Spec", out.Spec);
    Read(v, "UseOwnerLevel", out.UseOwnerLevel);
}
void ReadFields(const Value& v, MoveTo_Prototype& out, ReaderTag) {
    Read(v, "moveToSpec", out.moveToSpec);
    Read(v, "onMoveFinished", out.onMoveFinished);
}
void ReadFields(const Value&, MovementPackCache_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, MovingWall_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
    Read(v, "navmeshRegionID", out.navmeshRegionID);
}
void ReadFields(const Value& v, OnKillCallbacks_Prototype& out, ReaderTag) {
    Read(v, "onPlayerKilled", out.onPlayerKilled);
}
void ReadFields(const Value& v, OnTriggerEnter_Prototype& out, ReaderTag) {
    Read(v, "onTriggerEnter", out.onTriggerEnter);
    Read(v, "EntityFilter", out.EntityFilter);
    Read(v, "CanBeTriggeredByEnemies", out.CanBeTriggeredByEnemies);
    Read(v, "CanBeTriggeredByTeammates", out.CanBeTriggeredByTeammates);
    Read(v, "CanBeTriggeredByInstigator", out.CanBeTriggeredByInstigator);
    Read(v, "SendTriggeringEntityAsParameter", out.SendTriggeringEntityAsParameter);
    Read(v, "ActivationDelay", out.ActivationDelay);
}
void ReadFields(const Value& v, PlayerActor_Prototype& out, ReaderTag) {
    Read(v, "noActionPushForceThreshold", out.noActionPushForceThreshold);
    Read(v, "noActionMaxPushForce", out.noActionMaxPushForce);
    Read(v, "noActionTimeAfterPush", out.noActionTimeAfterPush);
    Read(v, "maxNoActionTimeAfterPush", out.maxNoActionTimeAfterPush);
    Read(v, "durationBeforeCombatReset", out.durationBeforeCombatReset);
}
void ReadFields(const Value& v, PowerUp_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
    Read(v, "IsPickedUp", out.IsPickedUp);
}
void ReadFields(const Value&, PowerUpUser_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, PreciseHitReceiver_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, PrioritisedTargetHandler_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, Projectile_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, ProjectileSpawner_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Pushable_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, PushableTrigger_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, Pusher_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Rail_Prototype& out, ReaderTag) {
    Read(v, "bakedRailIndex", out.bakedRailIndex);
}
void ReadFields(const Value& v, RailCollider_Prototype& out, ReaderTag) {
    Read(v, "railEntity", out.railEntity);
    Read(v, "distanceOnRail", out.distanceOnRail);
    Read(v, "nearbyRailColLeft", out.nearbyRailColLeft);
    Read(v, "nearbyRailColRight", out.nearbyRailColRight);
}
void ReadFields(const Value&, RankingManager_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, RecoverAmmoOnDashEnhancement_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, ReplaceMultiShotConfigsEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, RewardReceiver_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, Roll_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, RunnableWall_Prototype&, ReaderTag) {}
void ReadFields(const Value&, RunnableWallTriggerZone_Prototype&, ReaderTag) {}
void ReadFields(const Value&, RynoWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ShapeOverlapper_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, ShieldedActor_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value&, ShotgunBerserkerEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, ShotgunEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ShrinkingZone_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, ShrinkingZoneActor_Prototype& out, ReaderTag) {
    Read(v, "IgnoreShrinkingZone", out.IgnoreShrinkingZone);
}
void ReadFields(const Value& v, SpawnProjectilesOnDestroy_Prototype& out, ReaderTag) {
    Read(v, "Specs", out.Specs);
}
void ReadFields(const Value& v, Speeder_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
    Read(v, "SpeedModifier", out.SpeedModifier);
    Read(v, "Duration", out.Duration);
    Read(v, "Team", out.Team);
    Read(v, "UsedFrame", out.UsedFrame);
}
void ReadFields(const Value&, StatsManager_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, StreamEffect_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, StreamEffectSpawner_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, SweepAttack_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, TargetFinder_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, TargetHolder_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, Targetable_Prototype& out, ReaderTag) {
    Read(v, "IgnoreAutoShoot", out.IgnoreAutoShoot);
    Read(v, "IgnoreAutoTargetInsideBounds", out.IgnoreAutoTargetInsideBounds);
    Read(v, "IgnoreCameraAiming", out.IgnoreCameraAiming);
    Read(v, "BoundsSizeMultiplier", out.BoundsSizeMultiplier);
}
void ReadFields(const Value& v, TargetedEffect_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value& v, Team_Prototype& out, ReaderTag) {
    Read(v, "TeamID", out.TeamID);
}
void ReadFields(const Value& v, TeslaClawMoreBeam_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, TimeScale_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, TrailSpawner_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, TravolatorActor_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, TurnTo_Prototype& out, ReaderTag) {
    Read(v, "turnSpeed", out.turnSpeed);
}
void ReadFields(const Value& v, TutorialGameMode_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, UTAgent_Prototype& out, ReaderTag) {
    Read(v, "UtilityReasoner", out.UtilityReasoner);
    Read(v, "Config", out.Config);
}
void ReadFields(const Value& v, UltimateAbilityUser_Prototype& out, ReaderTag) {
    Read(v, "userSpec", out.userSpec);
}
void ReadFields(const Value&, UltimateEvolution_Prototype&, ReaderTag) {}
void ReadFields(const Value&, UltimateTank_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, UnlockedFeaturesManager_Prototype& out, ReaderTag) {
    Read(v, "unlockedFeatures", out.unlockedFeatures);
}
void ReadFields(const Value&, VoidRepulserShield_Prototype&, ReaderTag) {}
void ReadFields(const Value&, VoidRepulserSpeedBoostEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, VoidRepulserVoidWaveEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, VoidRepulserWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, WakeUpSurroundingsOnPushOrDestroy_Prototype& out, ReaderTag) {
    Read(v, "radius", out.radius);
}
void ReadFields(const Value& v, WallRunningActor_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, WarmongerMiniRocketsEnhancement_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value&, WaveElement_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, WaveManager_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
    Read(v, "SquadSiegeConfig", out.SquadSiegeConfig);
    Read(v, "referencesAsset", out.referencesAsset);
}
void ReadFields(const Value& v, WaveSpawner_Prototype& out, ReaderTag) {
    Read(v, "index", out.index);
    Read(v, "spawnConfig", out.spawnConfig);
    Read(v, "wanderRadius", out.wanderRadius);
    Read(v, "spawnInterval", out.spawnInterval);
    Read(v, "waypoints", out.waypoints);
    Read(v, "waypointRadius", out.waypointRadius);
}
void ReadFields(const Value& v, Weapon_Prototype& out, ReaderTag) {
    Read(v, "WeaponData", out.WeaponData);
}
void ReadFields(const Value& v, WeaponBehaviour_Charging_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}
void ReadFields(const Value& v, WeaponLoadoutActor_Prototype& out, ReaderTag) {
    Read(v, "spec", out.spec);
}
void ReadFields(const Value&, WeaponProjectileSwitcherEnhancement_Prototype&, ReaderTag) {}
void ReadFields(const Value&, WeaponShowcaseGameMode_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, WeaponUser_Prototype& out, ReaderTag) {
    Read(v, "DefaultEquippedWeapon", out.DefaultEquippedWeapon);
    Read(v, "weaponPositionOffset", out.weaponPositionOffset);
    Read(v, "weaponDefaultRotation", out.weaponDefaultRotation);
}
void ReadFields(const Value&, WrenchThrowProjectile_Prototype&, ReaderTag) {}
void ReadFields(const Value&, WrenchThrowWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value&, WrenchWeapon_Prototype&, ReaderTag) {}
void ReadFields(const Value& v, ZoneControlGameMode_Prototype& out, ReaderTag) {
    Read(v, "Spec", out.Spec);
}

namespace {

template <class T> std::shared_ptr<Quantum::ComponentPrototype> Make(const Value& value) {
    auto prototype = std::make_shared<T>();
    ReadValue(value, *prototype);
    return prototype;
}

using Maker = std::shared_ptr<Quantum::ComponentPrototype> (*)(const Value&);

const std::unordered_map<std::string, Maker>& GameMakers() {
    static const std::unordered_map<std::string, Maker> makers = {
        {"AIBlackboardComponent_Prototype", &Make<AIBlackboardComponent_Prototype>},
        {"AIMoveForwardBehaviour_Prototype", &Make<AIMoveForwardBehaviour_Prototype>},
        {"AINavigationInitialiser_Prototype", &Make<AINavigationInitialiser_Prototype>},
        {"AISpawner_Prototype", &Make<AISpawner_Prototype>},
        {"ActorInputInitialiser_Prototype", &Make<ActorInputInitialiser_Prototype>},
        {"ActorManager_Prototype", &Make<ActorManager_Prototype>},
        {"ActorStatus_Prototype", &Make<ActorStatus_Prototype>},
        {"ActorStatusModifier_Prototype", &Make<ActorStatusModifier_Prototype>},
        {"ActorStunned_Prototype", &Make<ActorStunned_Prototype>},
        {"AddDamageIncreaseOnTargetedEffect_Prototype",
         &Make<AddDamageIncreaseOnTargetedEffect_Prototype>},
        {"AddEntitiesToSpawnOnProjectileDeath_Prototype",
         &Make<AddEntitiesToSpawnOnProjectileDeath_Prototype>},
        {"AgentOfDoom_Prototype", &Make<AgentOfDoom_Prototype>},
        {"AgentOfDoomGrouped_Prototype", &Make<AgentOfDoomGrouped_Prototype>},
        {"AmoeboidLauncherEnhancementPuddle_Prototype",
         &Make<AmoeboidLauncherEnhancementPuddle_Prototype>},
        {"AoeTrail_Prototype", &Make<AoeTrail_Prototype>},
        {"AreaOfEffect_Prototype", &Make<AreaOfEffect_Prototype>},
        {"AreaOfEffectLevelUp_Prototype", &Make<AreaOfEffectLevelUp_Prototype>},
        {"AreaOfEffectRaycastInfo_Prototype", &Make<AreaOfEffectRaycastInfo_Prototype>},
        {"ArenaCannon_Prototype", &Make<ArenaCannon_Prototype>},
        {"AttackInfo_Prototype", &Make<AttackInfo_Prototype>},
        {"BTAgent_Prototype", &Make<BTAgent_Prototype>},
        {"BattleStatsActor_Prototype", &Make<BattleStatsActor_Prototype>},
        {"BigBoomMine_Prototype", &Make<BigBoomMine_Prototype>},
        {"BigBoomWeapon_Prototype", &Make<BigBoomWeapon_Prototype>},
        {"BlackboardInitializer_Prototype", &Make<BlackboardInitializer_Prototype>},
        {"BlastBall_Prototype", &Make<BlastBall_Prototype>},
        {"BlastBallGame_Prototype", &Make<BlastBallGame_Prototype>},
        {"BlastBallGoal_Prototype", &Make<BlastBallGoal_Prototype>},
        {"BlastBallPlayer_Prototype", &Make<BlastBallPlayer_Prototype>},
        {"BlasterMoveSpeedEnhancement_Prototype", &Make<BlasterMoveSpeedEnhancement_Prototype>},
        {"BlasterSpecialShotEnhancement_Prototype", &Make<BlasterSpecialShotEnhancement_Prototype>},
        {"BoltCounterElement_Prototype", &Make<BoltCounterElement_Prototype>},
        {"BoltGathererActor_Prototype", &Make<BoltGathererActor_Prototype>},
        {"BoltPowerActor_Prototype", &Make<BoltPowerActor_Prototype>},
        {"BombardierDrone_Prototype", &Make<BombardierDrone_Prototype>},
        {"BombardierWeapon_Prototype", &Make<BombardierWeapon_Prototype>},
        {"Bot_Prototype", &Make<Bot_Prototype>},
        {"Bumper_Prototype", &Make<Bumper_Prototype>},
        {"BumperBar_Prototype", &Make<BumperBar_Prototype>},
        {"BuzzbladesRicochet_Prototype", &Make<BuzzbladesRicochet_Prototype>},
        {"CharacterColliderScaler_Prototype", &Make<CharacterColliderScaler_Prototype>},
        {"ChargeAttack_Prototype", &Make<ChargeAttack_Prototype>},
        {"Checkpoint_Prototype", &Make<Checkpoint_Prototype>},
        {"ColliderResize_Prototype", &Make<ColliderResize_Prototype>},
        {"CollisionForce_Prototype", &Make<CollisionForce_Prototype>},
        {"CombatParticipant_Prototype", &Make<CombatParticipant_Prototype>},
        {"CompoundHFSM_Prototype", &Make<CompoundHFSM_Prototype>},
        {"Cryoslider_Prototype", &Make<Cryoslider_Prototype>},
        {"CryosliderWeapon_Prototype", &Make<CryosliderWeapon_Prototype>},
        {"DamageChargeable_Prototype", &Make<DamageChargeable_Prototype>},
        {"DamageIncreasedOnTargetsEnhancement_Prototype",
         &Make<DamageIncreasedOnTargetsEnhancement_Prototype>},
        {"DamageMultiplier_Prototype", &Make<DamageMultiplier_Prototype>},
        {"DamageMultiplierHeroImprovement_Prototype",
         &Make<DamageMultiplierHeroImprovement_Prototype>},
        {"DamageSource_Prototype", &Make<DamageSource_Prototype>},
        {"DamageToImpulseConverter_Prototype", &Make<DamageToImpulseConverter_Prototype>},
        {"Damageable_Prototype", &Make<Damageable_Prototype>},
        {"DashActor_Prototype", &Make<DashActor_Prototype>},
        {"DrillDash_Prototype", &Make<DrillDash_Prototype>},
        {"DroppableCollector_Prototype", &Make<DroppableCollector_Prototype>},
        {"DroppableEntity_Prototype", &Make<DroppableEntity_Prototype>},
        {"DroppableManager_Prototype", &Make<DroppableManager_Prototype>},
        {"ElementalBuildUpMultiplier_Prototype", &Make<ElementalBuildUpMultiplier_Prototype>},
        {"EliminationGameMode_Prototype", &Make<EliminationGameMode_Prototype>},
        {"EnergyCellManager_Prototype", &Make<EnergyCellManager_Prototype>},
        {"EnforcerFirstShotBonusEnhancement_Prototype",
         &Make<EnforcerFirstShotBonusEnhancement_Prototype>},
        {"EntityDropper_Prototype", &Make<EntityDropper_Prototype>},
        {"EntityInfo_Prototype", &Make<EntityInfo_Prototype>},
        {"FireReactor_Prototype", &Make<FireReactor_Prototype>},
        {"GOAPAgent_Prototype", &Make<GOAPAgent_Prototype>},
        {"GOAPData_Prototype", &Make<GOAPData_Prototype>},
        {"GadgetUser_Prototype", &Make<GadgetUser_Prototype>},
        {"GameManager_Prototype", &Make<GameManager_Prototype>},
        {"GameModeActivator_Prototype", &Make<GameModeActivator_Prototype>},
        {"GameModeHandler_Prototype", &Make<GameModeHandler_Prototype>},
        {"GameModifiersManager_Prototype", &Make<GameModifiersManager_Prototype>},
        {"GatlingWeapon_Prototype", &Make<GatlingWeapon_Prototype>},
        {"GloveOfDoomWeapon_Prototype", &Make<GloveOfDoomWeapon_Prototype>},
        {"GoldenBolt_Prototype", &Make<GoldenBolt_Prototype>},
        {"GoldenBoltManager_Prototype", &Make<GoldenBoltManager_Prototype>},
        {"GrinderActor_Prototype", &Make<GrinderActor_Prototype>},
        {"GroundDashActor_Prototype", &Make<GroundDashActor_Prototype>},
        {"HFSMAgent_Prototype", &Make<HFSMAgent_Prototype>},
        {"HeadHunterWallHackEnhancement_Prototype", &Make<HeadHunterWallHackEnhancement_Prototype>},
        {"HealSource_Prototype", &Make<HealSource_Prototype>},
        {"Health_Prototype", &Make<Health_Prototype>},
        {"HelipackActor_Prototype", &Make<HelipackActor_Prototype>},
        {"Hierarchy_Prototype", &Make<Hierarchy_Prototype>},
        {"HoloshieldGloveHealZoneEnhancement_Prototype",
         &Make<HoloshieldGloveHealZoneEnhancement_Prototype>},
        {"HoloshieldGloveShield_Prototype", &Make<HoloshieldGloveShield_Prototype>},
        {"HoloshieldGloveShields_Prototype", &Make<HoloshieldGloveShields_Prototype>},
        {"HoloshieldGloveShieldsGrouped_Prototype", &Make<HoloshieldGloveShieldsGrouped_Prototype>},
        {"HoloshieldGloveWeapon_Prototype", &Make<HoloshieldGloveWeapon_Prototype>},
        {"HoverBootsWeapon_Prototype", &Make<HoverBootsWeapon_Prototype>},
        {"ItemSpawner_Prototype", &Make<ItemSpawner_Prototype>},
        {"JumpAttack_Prototype", &Make<JumpAttack_Prototype>},
        {"KccExtension_Prototype", &Make<KccExtension_Prototype>},
        {"KccModifier_Prototype", &Make<KccModifier_Prototype>},
        {"KillZone_Prototype", &Make<KillZone_Prototype>},
        {"Killable_Prototype", &Make<Killable_Prototype>},
        {"LedgeGrabbingActor_Prototype", &Make<LedgeGrabbingActor_Prototype>},
        {"LevelButton_Prototype", &Make<LevelButton_Prototype>},
        {"LevelButtonGameplayElement_Prototype", &Make<LevelButtonGameplayElement_Prototype>},
        {"LevelHolder_Prototype", &Make<LevelHolder_Prototype>},
        {"LevelUpActor_Prototype", &Make<LevelUpActor_Prototype>},
        {"Lifetime_Prototype", &Make<Lifetime_Prototype>},
        {"MeleeAttack_Prototype", &Make<MeleeAttack_Prototype>},
        {"MeleeJumpAttack_Prototype", &Make<MeleeJumpAttack_Prototype>},
        {"MineLauncherEnhancement_Prototype", &Make<MineLauncherEnhancement_Prototype>},
        {"MineMerger_Prototype", &Make<MineMerger_Prototype>},
        {"MobGroup_Prototype", &Make<MobGroup_Prototype>},
        {"MobGroupMember_Prototype", &Make<MobGroupMember_Prototype>},
        {"MobInfo_Prototype", &Make<MobInfo_Prototype>},
        {"MoveTo_Prototype", &Make<MoveTo_Prototype>},
        {"MovementPackCache_Prototype", &Make<MovementPackCache_Prototype>},
        {"MovingWall_Prototype", &Make<MovingWall_Prototype>},
        {"OnKillCallbacks_Prototype", &Make<OnKillCallbacks_Prototype>},
        {"OnTriggerEnter_Prototype", &Make<OnTriggerEnter_Prototype>},
        {"PlayerActor_Prototype", &Make<PlayerActor_Prototype>},
        {"PowerUp_Prototype", &Make<PowerUp_Prototype>},
        {"PowerUpUser_Prototype", &Make<PowerUpUser_Prototype>},
        {"PreciseHitReceiver_Prototype", &Make<PreciseHitReceiver_Prototype>},
        {"PrioritisedTargetHandler_Prototype", &Make<PrioritisedTargetHandler_Prototype>},
        {"Projectile_Prototype", &Make<Projectile_Prototype>},
        {"ProjectileSpawner_Prototype", &Make<ProjectileSpawner_Prototype>},
        {"Pushable_Prototype", &Make<Pushable_Prototype>},
        {"PushableTrigger_Prototype", &Make<PushableTrigger_Prototype>},
        {"Pusher_Prototype", &Make<Pusher_Prototype>},
        {"Rail_Prototype", &Make<Rail_Prototype>},
        {"RailCollider_Prototype", &Make<RailCollider_Prototype>},
        {"RankingManager_Prototype", &Make<RankingManager_Prototype>},
        {"RecoverAmmoOnDashEnhancement_Prototype", &Make<RecoverAmmoOnDashEnhancement_Prototype>},
        {"ReplaceMultiShotConfigsEnhancement_Prototype",
         &Make<ReplaceMultiShotConfigsEnhancement_Prototype>},
        {"RewardReceiver_Prototype", &Make<RewardReceiver_Prototype>},
        {"Roll_Prototype", &Make<Roll_Prototype>},
        {"RunnableWall_Prototype", &Make<RunnableWall_Prototype>},
        {"RunnableWallTriggerZone_Prototype", &Make<RunnableWallTriggerZone_Prototype>},
        {"RynoWeapon_Prototype", &Make<RynoWeapon_Prototype>},
        {"ShapeOverlapper_Prototype", &Make<ShapeOverlapper_Prototype>},
        {"ShieldedActor_Prototype", &Make<ShieldedActor_Prototype>},
        {"ShotgunBerserkerEnhancement_Prototype", &Make<ShotgunBerserkerEnhancement_Prototype>},
        {"ShotgunEnhancement_Prototype", &Make<ShotgunEnhancement_Prototype>},
        {"ShrinkingZone_Prototype", &Make<ShrinkingZone_Prototype>},
        {"ShrinkingZoneActor_Prototype", &Make<ShrinkingZoneActor_Prototype>},
        {"SpawnProjectilesOnDestroy_Prototype", &Make<SpawnProjectilesOnDestroy_Prototype>},
        {"Speeder_Prototype", &Make<Speeder_Prototype>},
        {"StatsManager_Prototype", &Make<StatsManager_Prototype>},
        {"StreamEffect_Prototype", &Make<StreamEffect_Prototype>},
        {"StreamEffectSpawner_Prototype", &Make<StreamEffectSpawner_Prototype>},
        {"SweepAttack_Prototype", &Make<SweepAttack_Prototype>},
        {"TargetFinder_Prototype", &Make<TargetFinder_Prototype>},
        {"TargetHolder_Prototype", &Make<TargetHolder_Prototype>},
        {"Targetable_Prototype", &Make<Targetable_Prototype>},
        {"TargetedEffect_Prototype", &Make<TargetedEffect_Prototype>},
        {"Team_Prototype", &Make<Team_Prototype>},
        {"TeslaClawMoreBeam_Prototype", &Make<TeslaClawMoreBeam_Prototype>},
        {"TimeScale_Prototype", &Make<TimeScale_Prototype>},
        {"TrailSpawner_Prototype", &Make<TrailSpawner_Prototype>},
        {"TravolatorActor_Prototype", &Make<TravolatorActor_Prototype>},
        {"TurnTo_Prototype", &Make<TurnTo_Prototype>},
        {"TutorialGameMode_Prototype", &Make<TutorialGameMode_Prototype>},
        {"UTAgent_Prototype", &Make<UTAgent_Prototype>},
        {"UltimateAbilityUser_Prototype", &Make<UltimateAbilityUser_Prototype>},
        {"UltimateEvolution_Prototype", &Make<UltimateEvolution_Prototype>},
        {"UltimateTank_Prototype", &Make<UltimateTank_Prototype>},
        {"UnlockedFeaturesManager_Prototype", &Make<UnlockedFeaturesManager_Prototype>},
        {"VoidRepulserShield_Prototype", &Make<VoidRepulserShield_Prototype>},
        {"VoidRepulserSpeedBoostEnhancement_Prototype",
         &Make<VoidRepulserSpeedBoostEnhancement_Prototype>},
        {"VoidRepulserVoidWaveEnhancement_Prototype",
         &Make<VoidRepulserVoidWaveEnhancement_Prototype>},
        {"VoidRepulserWeapon_Prototype", &Make<VoidRepulserWeapon_Prototype>},
        {"WakeUpSurroundingsOnPushOrDestroy_Prototype",
         &Make<WakeUpSurroundingsOnPushOrDestroy_Prototype>},
        {"WallRunningActor_Prototype", &Make<WallRunningActor_Prototype>},
        {"WarmongerMiniRocketsEnhancement_Prototype",
         &Make<WarmongerMiniRocketsEnhancement_Prototype>},
        {"WaveElement_Prototype", &Make<WaveElement_Prototype>},
        {"WaveManager_Prototype", &Make<WaveManager_Prototype>},
        {"WaveSpawner_Prototype", &Make<WaveSpawner_Prototype>},
        {"Weapon_Prototype", &Make<Weapon_Prototype>},
        {"WeaponBehaviour_Charging_Prototype", &Make<WeaponBehaviour_Charging_Prototype>},
        {"WeaponLoadoutActor_Prototype", &Make<WeaponLoadoutActor_Prototype>},
        {"WeaponProjectileSwitcherEnhancement_Prototype",
         &Make<WeaponProjectileSwitcherEnhancement_Prototype>},
        {"WeaponShowcaseGameMode_Prototype", &Make<WeaponShowcaseGameMode_Prototype>},
        {"WeaponUser_Prototype", &Make<WeaponUser_Prototype>},
        {"WrenchThrowProjectile_Prototype", &Make<WrenchThrowProjectile_Prototype>},
        {"WrenchThrowWeapon_Prototype", &Make<WrenchThrowWeapon_Prototype>},
        {"WrenchWeapon_Prototype", &Make<WrenchWeapon_Prototype>},
        {"ZoneControlGameMode_Prototype", &Make<ZoneControlGameMode_Prototype>},
    };
    return makers;
}

template <class T>
void ReadList(const Value& parent, std::string_view name, std::vector<std::shared_ptr<T>>& out) {
    const Value* list = Child(parent, name);
    if (!list || !list->AsArray())
        return;
    for (const auto& item : *list->AsArray()) {
        auto prototype = std::make_shared<T>();
        ReadValue(*item, *prototype);
        out.push_back(std::move(prototype));
    }
}

} // namespace

std::shared_ptr<Quantum::ComponentPrototype> ReadGameComponentPrototype(const std::string& typeName,
                                                                        const Value& value) {
    const auto& makers = GameMakers();
    auto it = makers.find(typeName);
    return it != makers.end() ? it->second(value) : nullptr;
}

void ReadFlatEntityPrototypeContainer(const Value& value, FlatEntityPrototypeContainer& out) {
    ReadList(value, "AIBlackboardComponent", out.AIBlackboardComponent);
    ReadList(value, "AIMoveForwardBehaviour", out.AIMoveForwardBehaviour);
    ReadList(value, "AINavigationInitialiser", out.AINavigationInitialiser);
    ReadList(value, "AISpawner", out.AISpawner);
    ReadList(value, "ActorInputInitialiser", out.ActorInputInitialiser);
    ReadList(value, "ActorManager", out.ActorManager);
    ReadList(value, "ActorStatus", out.ActorStatus);
    ReadList(value, "ActorStatusModifier", out.ActorStatusModifier);
    ReadList(value, "ActorStunned", out.ActorStunned);
    ReadList(value, "AddDamageIncreaseOnTargetedEffect", out.AddDamageIncreaseOnTargetedEffect);
    ReadList(value, "AddEntitiesToSpawnOnProjectileDeath", out.AddEntitiesToSpawnOnProjectileDeath);
    ReadList(value, "AgentOfDoom", out.AgentOfDoom);
    ReadList(value, "AgentOfDoomGrouped", out.AgentOfDoomGrouped);
    ReadList(value, "AmoeboidLauncherEnhancementPuddle", out.AmoeboidLauncherEnhancementPuddle);
    ReadList(value, "AoeTrail", out.AoeTrail);
    ReadList(value, "AreaOfEffect", out.AreaOfEffect);
    ReadList(value, "AreaOfEffectLevelUp", out.AreaOfEffectLevelUp);
    ReadList(value, "AreaOfEffectRaycastInfo", out.AreaOfEffectRaycastInfo);
    ReadList(value, "ArenaCannon", out.ArenaCannon);
    ReadList(value, "AttackInfo", out.AttackInfo);
    ReadList(value, "BTAgent", out.BTAgent);
    ReadList(value, "BattleStatsActor", out.BattleStatsActor);
    ReadList(value, "BigBoomMine", out.BigBoomMine);
    ReadList(value, "BigBoomWeapon", out.BigBoomWeapon);
    ReadList(value, "BlackboardInitializer", out.BlackboardInitializer);
    ReadList(value, "BlastBall", out.BlastBall);
    ReadList(value, "BlastBallGame", out.BlastBallGame);
    ReadList(value, "BlastBallGoal", out.BlastBallGoal);
    ReadList(value, "BlastBallPlayer", out.BlastBallPlayer);
    ReadList(value, "BlasterMoveSpeedEnhancement", out.BlasterMoveSpeedEnhancement);
    ReadList(value, "BlasterSpecialShotEnhancement", out.BlasterSpecialShotEnhancement);
    ReadList(value, "BoltCounterElement", out.BoltCounterElement);
    ReadList(value, "BoltGathererActor", out.BoltGathererActor);
    ReadList(value, "BoltPowerActor", out.BoltPowerActor);
    ReadList(value, "BombardierDrone", out.BombardierDrone);
    ReadList(value, "BombardierWeapon", out.BombardierWeapon);
    ReadList(value, "Bot", out.Bot);
    ReadList(value, "Bumper", out.Bumper);
    ReadList(value, "BumperBar", out.BumperBar);
    ReadList(value, "BuzzbladesRicochet", out.BuzzbladesRicochet);
    ReadList(value, "CharacterColliderScaler", out.CharacterColliderScaler);
    ReadList(value, "ChargeAttack", out.ChargeAttack);
    ReadList(value, "Checkpoint", out.Checkpoint);
    ReadList(value, "ColliderResize", out.ColliderResize);
    ReadList(value, "CollisionForce", out.CollisionForce);
    ReadList(value, "CombatParticipant", out.CombatParticipant);
    ReadList(value, "CompoundHFSM", out.CompoundHFSM);
    ReadList(value, "Cryoslider", out.Cryoslider);
    ReadList(value, "CryosliderWeapon", out.CryosliderWeapon);
    ReadList(value, "DamageChargeable", out.DamageChargeable);
    ReadList(value, "DamageIncreasedOnTargetsEnhancement", out.DamageIncreasedOnTargetsEnhancement);
    ReadList(value, "DamageMultiplier", out.DamageMultiplier);
    ReadList(value, "DamageMultiplierHeroImprovement", out.DamageMultiplierHeroImprovement);
    ReadList(value, "DamageSource", out.DamageSource);
    ReadList(value, "DamageToImpulseConverter", out.DamageToImpulseConverter);
    ReadList(value, "Damageable", out.Damageable);
    ReadList(value, "DashActor", out.DashActor);
    ReadList(value, "DrillDash", out.DrillDash);
    ReadList(value, "DroppableCollector", out.DroppableCollector);
    ReadList(value, "DroppableEntity", out.DroppableEntity);
    ReadList(value, "DroppableManager", out.DroppableManager);
    ReadList(value, "ElementalBuildUpMultiplier", out.ElementalBuildUpMultiplier);
    ReadList(value, "EliminationGameMode", out.EliminationGameMode);
    ReadList(value, "EnergyCellManager", out.EnergyCellManager);
    ReadList(value, "EnforcerFirstShotBonusEnhancement", out.EnforcerFirstShotBonusEnhancement);
    ReadList(value, "EntityDropper", out.EntityDropper);
    ReadList(value, "EntityInfo", out.EntityInfo);
    ReadList(value, "FireReactor", out.FireReactor);
    ReadList(value, "GOAPAgent", out.GOAPAgent);
    ReadList(value, "GOAPData", out.GOAPData);
    ReadList(value, "GadgetUser", out.GadgetUser);
    ReadList(value, "GameManager", out.GameManager);
    ReadList(value, "GameModeActivator", out.GameModeActivator);
    ReadList(value, "GameModeHandler", out.GameModeHandler);
    ReadList(value, "GameModifiersManager", out.GameModifiersManager);
    ReadList(value, "GatlingWeapon", out.GatlingWeapon);
    ReadList(value, "GloveOfDoomWeapon", out.GloveOfDoomWeapon);
    ReadList(value, "GoldenBolt", out.GoldenBolt);
    ReadList(value, "GoldenBoltManager", out.GoldenBoltManager);
    ReadList(value, "GrinderActor", out.GrinderActor);
    ReadList(value, "GroundDashActor", out.GroundDashActor);
    ReadList(value, "HFSMAgent", out.HFSMAgent);
    ReadList(value, "HeadHunterWallHackEnhancement", out.HeadHunterWallHackEnhancement);
    ReadList(value, "HealSource", out.HealSource);
    ReadList(value, "Health", out.Health);
    ReadList(value, "HelipackActor", out.HelipackActor);
    ReadList(value, "Hierarchy", out.Hierarchy);
    ReadList(value, "HoloshieldGloveHealZoneEnhancement", out.HoloshieldGloveHealZoneEnhancement);
    ReadList(value, "HoloshieldGloveShield", out.HoloshieldGloveShield);
    ReadList(value, "HoloshieldGloveShields", out.HoloshieldGloveShields);
    ReadList(value, "HoloshieldGloveShieldsGrouped", out.HoloshieldGloveShieldsGrouped);
    ReadList(value, "HoloshieldGloveWeapon", out.HoloshieldGloveWeapon);
    ReadList(value, "HoverBootsWeapon", out.HoverBootsWeapon);
    ReadList(value, "ItemSpawner", out.ItemSpawner);
    ReadList(value, "JumpAttack", out.JumpAttack);
    ReadList(value, "KccExtension", out.KccExtension);
    ReadList(value, "KccModifier", out.KccModifier);
    ReadList(value, "KillZone", out.KillZone);
    ReadList(value, "Killable", out.Killable);
    ReadList(value, "LedgeGrabbingActor", out.LedgeGrabbingActor);
    ReadList(value, "LevelButton", out.LevelButton);
    ReadList(value, "LevelButtonGameplayElement", out.LevelButtonGameplayElement);
    ReadList(value, "LevelHolder", out.LevelHolder);
    ReadList(value, "LevelUpActor", out.LevelUpActor);
    ReadList(value, "Lifetime", out.Lifetime);
    ReadList(value, "MeleeAttack", out.MeleeAttack);
    ReadList(value, "MeleeJumpAttack", out.MeleeJumpAttack);
    ReadList(value, "MineLauncherEnhancement", out.MineLauncherEnhancement);
    ReadList(value, "MineMerger", out.MineMerger);
    ReadList(value, "MobGroup", out.MobGroup);
    ReadList(value, "MobGroupMember", out.MobGroupMember);
    ReadList(value, "MobInfo", out.MobInfo);
    ReadList(value, "MoveTo", out.MoveTo);
    ReadList(value, "MovementPackCache", out.MovementPackCache);
    ReadList(value, "MovingWall", out.MovingWall);
    ReadList(value, "OnKillCallbacks", out.OnKillCallbacks);
    ReadList(value, "OnTriggerEnter", out.OnTriggerEnter);
    ReadList(value, "PlayerActor", out.PlayerActor);
    ReadList(value, "PowerUp", out.PowerUp);
    ReadList(value, "PowerUpUser", out.PowerUpUser);
    ReadList(value, "PreciseHitReceiver", out.PreciseHitReceiver);
    ReadList(value, "PrioritisedTargetHandler", out.PrioritisedTargetHandler);
    ReadList(value, "Projectile", out.Projectile);
    ReadList(value, "ProjectileSpawner", out.ProjectileSpawner);
    ReadList(value, "Pushable", out.Pushable);
    ReadList(value, "PushableTrigger", out.PushableTrigger);
    ReadList(value, "Pusher", out.Pusher);
    ReadList(value, "Rail", out.Rail);
    ReadList(value, "RailCollider", out.RailCollider);
    ReadList(value, "RankingManager", out.RankingManager);
    ReadList(value, "RecoverAmmoOnDashEnhancement", out.RecoverAmmoOnDashEnhancement);
    ReadList(value, "ReplaceMultiShotConfigsEnhancement", out.ReplaceMultiShotConfigsEnhancement);
    ReadList(value, "RewardReceiver", out.RewardReceiver);
    ReadList(value, "Roll", out.Roll);
    ReadList(value, "RunnableWall", out.RunnableWall);
    ReadList(value, "RunnableWallTriggerZone", out.RunnableWallTriggerZone);
    ReadList(value, "RynoWeapon", out.RynoWeapon);
    ReadList(value, "ShapeOverlapper", out.ShapeOverlapper);
    ReadList(value, "ShieldedActor", out.ShieldedActor);
    ReadList(value, "ShotgunBerserkerEnhancement", out.ShotgunBerserkerEnhancement);
    ReadList(value, "ShotgunEnhancement", out.ShotgunEnhancement);
    ReadList(value, "ShrinkingZone", out.ShrinkingZone);
    ReadList(value, "ShrinkingZoneActor", out.ShrinkingZoneActor);
    ReadList(value, "SpawnProjectilesOnDestroy", out.SpawnProjectilesOnDestroy);
    ReadList(value, "Speeder", out.Speeder);
    ReadList(value, "StatsManager", out.StatsManager);
    ReadList(value, "StreamEffect", out.StreamEffect);
    ReadList(value, "StreamEffectSpawner", out.StreamEffectSpawner);
    ReadList(value, "SweepAttack", out.SweepAttack);
    ReadList(value, "TargetFinder", out.TargetFinder);
    ReadList(value, "TargetHolder", out.TargetHolder);
    ReadList(value, "Targetable", out.Targetable);
    ReadList(value, "TargetedEffect", out.TargetedEffect);
    ReadList(value, "Team", out.Team);
    ReadList(value, "TeslaClawMoreBeam", out.TeslaClawMoreBeam);
    ReadList(value, "TimeScale", out.TimeScale);
    ReadList(value, "TrailSpawner", out.TrailSpawner);
    ReadList(value, "TravolatorActor", out.TravolatorActor);
    ReadList(value, "TurnTo", out.TurnTo);
    ReadList(value, "TutorialGameMode", out.TutorialGameMode);
    ReadList(value, "UTAgent", out.UTAgent);
    ReadList(value, "UltimateAbilityUser", out.UltimateAbilityUser);
    ReadList(value, "UltimateEvolution", out.UltimateEvolution);
    ReadList(value, "UltimateTank", out.UltimateTank);
    ReadList(value, "UnlockedFeaturesManager", out.UnlockedFeaturesManager);
    ReadList(value, "VoidRepulserShield", out.VoidRepulserShield);
    ReadList(value, "VoidRepulserSpeedBoostEnhancement", out.VoidRepulserSpeedBoostEnhancement);
    ReadList(value, "VoidRepulserVoidWaveEnhancement", out.VoidRepulserVoidWaveEnhancement);
    ReadList(value, "VoidRepulserWeapon", out.VoidRepulserWeapon);
    ReadList(value, "WakeUpSurroundingsOnPushOrDestroy", out.WakeUpSurroundingsOnPushOrDestroy);
    ReadList(value, "WallRunningActor", out.WallRunningActor);
    ReadList(value, "WarmongerMiniRocketsEnhancement", out.WarmongerMiniRocketsEnhancement);
    ReadList(value, "WaveElement", out.WaveElement);
    ReadList(value, "WaveManager", out.WaveManager);
    ReadList(value, "WaveSpawner", out.WaveSpawner);
    ReadList(value, "Weapon", out.Weapon);
    ReadList(value, "WeaponBehaviour_Charging", out.WeaponBehaviour_Charging);
    ReadList(value, "WeaponLoadoutActor", out.WeaponLoadoutActor);
    ReadList(value, "WeaponProjectileSwitcherEnhancement", out.WeaponProjectileSwitcherEnhancement);
    ReadList(value, "WeaponShowcaseGameMode", out.WeaponShowcaseGameMode);
    ReadList(value, "WeaponUser", out.WeaponUser);
    ReadList(value, "WrenchThrowProjectile", out.WrenchThrowProjectile);
    ReadList(value, "WrenchThrowWeapon", out.WrenchThrowWeapon);
    ReadList(value, "WrenchWeapon", out.WrenchWeapon);
    ReadList(value, "ZoneControlGameMode", out.ZoneControlGameMode);
    ReadList(value, "CharacterController2D", out.CharacterController2D);
    ReadList(value, "CharacterController3D", out.CharacterController3D);
    ReadList(value, "NavMeshAvoidanceAgent", out.NavMeshAvoidanceAgent);
    ReadList(value, "NavMeshAvoidanceObstacle", out.NavMeshAvoidanceObstacle);
    ReadList(value, "NavMeshPathfinder", out.NavMeshPathfinder);
    ReadList(value, "NavMeshSteeringAgent", out.NavMeshSteeringAgent);
    ReadList(value, "PhysicsBody2D", out.PhysicsBody2D);
    ReadList(value, "PhysicsBody3D", out.PhysicsBody3D);
    ReadList(value, "PhysicsCollider2D", out.PhysicsCollider2D);
    ReadList(value, "PhysicsCollider3D", out.PhysicsCollider3D);
    ReadList(value, "PhysicsCallbacks2D", out.PhysicsCallbacks2D);
    ReadList(value, "PhysicsCallbacks3D", out.PhysicsCallbacks3D);
    ReadList(value, "Transform2D", out.Transform2D);
    ReadList(value, "Transform2DVertical", out.Transform2DVertical);
    ReadList(value, "Transform3D", out.Transform3D);
    ReadList(value, "View", out.View);
    ReadList(value, "PhysicsJoints2D", out.PhysicsJoints2D);
    ReadList(value, "PhysicsJoints3D", out.PhysicsJoints3D);
}

} // namespace rr::content
