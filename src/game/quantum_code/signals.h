// signals.h - the simulation's signal interfaces (quantum.code.dll).
//
// A signal is a synchronous call from one part of the simulation to every system that
// implements the matching ISignalXxx interface (and is enabled), in system order. The
// frame collects the implementers once (Frame::InitGen); FrameSignals raises them.
#pragma once

#include "quantum_code/components.h"

namespace Quantum {

class Frame;
class PowerUpSpec;
// Framework types whose reimplementation is pending (physics callbacks, prototypes,
// kinematic character movement); signals only pass them through.
struct CollisionInfo3D;
struct ExitInfo3D;
struct TriggerInfo3D;
struct EntityPrototypeRef;
struct CharacterController3DMovementExt;
enum class WaypointFlag : std::uint8_t;

// Common base so that systems can be collected per interface with dynamic_cast.
class ISignal {
  public:
    virtual ~ISignal() = default;
};

class ISignalAIMoveForward_OnBounce : public virtual ISignal {
  public:
    virtual void AIMoveForward_OnBounce(Frame* f, EntityRef bouncer) = 0;
};

class ISignalAISpawner_OnSpawned : public virtual ISignal {
  public:
    virtual void AISpawner_OnSpawned(Frame* f, EntityRef spawner, EntityRef spawned) = 0;
};

class ISignalAISpawner_OnSpawnDone : public virtual ISignal {
  public:
    virtual void AISpawner_OnSpawnDone(Frame* f, EntityRef spawner) = 0;
};

class ISignalPlayerActor_OnInputLocked : public virtual ISignal {
  public:
    virtual void PlayerActor_OnInputLocked(Frame* f, EntityRef entity, QBoolean locked) = 0;
};

class ISignalActorStatus_OnChanged : public virtual ISignal {
  public:
    virtual void ActorStatus_OnChanged(Frame* f, EntityRef actorEntity) = 0;
};

class ISignalAreaOfEffect_OnCalculateDynamicShape : public virtual ISignal {
  public:
    virtual void AreaOfEffect_OnCalculateDynamicShape(Frame* f, EntityRef areaOfEffectEntity, Shape3D* outShape) = 0;
};

class ISignalAreaOfEffect_OnHit : public virtual ISignal {
  public:
    virtual void AreaOfEffect_OnHit(Frame* f, EntityRef areaOfEffectEntity) = 0;
};

class ISignalAreaOfEffect_OnMiss : public virtual ISignal {
  public:
    virtual void AreaOfEffect_OnMiss(Frame* f, EntityRef areaOfEffectEntity) = 0;
};

class ISignalAutoJumperActor_OnStartJump : public virtual ISignal {
  public:
    virtual void AutoJumperActor_OnStartJump(Frame* f, EntityRef entity) = 0;
};

class ISignalAutoJumperActor_OnEndJump : public virtual ISignal {
  public:
    virtual void AutoJumperActor_OnEndJump(Frame* f, EntityRef entity) = 0;
};

class ISignalBlastBallGame_StateChanged : public virtual ISignal {
  public:
    virtual void BlastBallGame_StateChanged(Frame* f, BlastBallGameState newState) = 0;
};

class ISignalBlastBallGame_LeadChanged : public virtual ISignal {
  public:
    virtual void BlastBallGame_LeadChanged(Frame* f) = 0;
};

class ISignalBolt_OnCollected : public virtual ISignal {
  public:
    virtual void Bolt_OnCollected(Frame* f, EntityRef collector, std::int32_t amount) = 0;
};

class ISignalBolt_OnLost : public virtual ISignal {
  public:
    virtual void Bolt_OnLost(Frame* f, EntityRef collector, std::int32_t amount) = 0;
};

class ISignalDamageable_OnBeforeHit : public virtual ISignal {
  public:
    virtual void Damageable_OnBeforeHit(Frame* f, EntityRef targetEntity, HitData* hitData) = 0;
};

class ISignalDamageable_HitReceived : public virtual ISignal {
  public:
    virtual void Damageable_HitReceived(Frame* f, EntityRef entity, const HitData& hitData, const HitResult& hitResult) = 0;
};

class ISignalDamageToImpulseConverter_ComputeImpulse : public virtual ISignal {
  public:
    virtual void DamageToImpulseConverter_ComputeImpulse(Frame* f, EntityRef entity, const HitData& hitData, DamageToImpulseData* impulseData) = 0;
};

class ISignalDamageToImpulseConverter_OnBeforeImpulse : public virtual ISignal {
  public:
    virtual void DamageToImpulseConverter_OnBeforeImpulse(Frame* f, EntityRef entity, const DamageToImpulseData& impulseData) = 0;
};

class ISignalDamageToImpulseConverter_OnAfterImpulse : public virtual ISignal {
  public:
    virtual void DamageToImpulseConverter_OnAfterImpulse(Frame* f, EntityRef entity) = 0;
};

class ISignalDashActor_OnBeforeDash : public virtual ISignal {
  public:
    virtual void DashActor_OnBeforeDash(Frame* f, EntityRef entity, FPVector3* dashDir) = 0;
};

class ISignalDashActor_OnDashed : public virtual ISignal {
  public:
    virtual void DashActor_OnDashed(Frame* f, EntityRef entity) = 0;
};

class ISignalDashActor_OnDashIsReady : public virtual ISignal {
  public:
    virtual void DashActor_OnDashIsReady(Frame* f, EntityRef entity) = 0;
};

class ISignalDashActor_OnAfterDash : public virtual ISignal {
  public:
    virtual void DashActor_OnAfterDash(Frame* f, EntityRef entity) = 0;
};

class ISignalDrillDash_OnStateChanged : public virtual ISignal {
  public:
    virtual void DrillDash_OnStateChanged(Frame* f, EntityRef entity, DrillDash* pDrillDash, DrillDashState newState) = 0;
};

class ISignalDroppable_OnDestroyed : public virtual ISignal {
  public:
    virtual void Droppable_OnDestroyed(Frame* f, const Droppable& droppable) = 0;
};

class ISignalGadgetUser_OnGadgetActivated : public virtual ISignal {
  public:
    virtual void GadgetUser_OnGadgetActivated(Frame* f, EntityRef entity) = 0;
};

class ISignalGadgetUser_OnGadgetDeactivated : public virtual ISignal {
  public:
    virtual void GadgetUser_OnGadgetDeactivated(Frame* f, EntityRef entity) = 0;
};

class ISignalGameMode_OnMainLoopStart : public virtual ISignal {
  public:
    virtual void GameMode_OnMainLoopStart(Frame* f, EntityRef gameMode) = 0;
};

class ISignalGameMode_OnStateChanged : public virtual ISignal {
  public:
    virtual void GameMode_OnStateChanged(Frame* f, GAME_MODE_HANDLER_STATE prevState, GAME_MODE_HANDLER_STATE newState) = 0;
};

class ISignalGameMode_OnPlayerEliminated : public virtual ISignal {
  public:
    virtual void GameMode_OnPlayerEliminated(Frame* f, EntityRef entity) = 0;
};

class ISignalGameMode_OnPlayerFinished : public virtual ISignal {
  public:
    virtual void GameMode_OnPlayerFinished(Frame* f, EntityRef entity) = 0;
};

class ISignalBolt_OnGoldenBoltCollected : public virtual ISignal {
  public:
    virtual void Bolt_OnGoldenBoltCollected(Frame* f, EntityRef collector) = 0;
};

class ISignalGrinderActor_OnExitGrind : public virtual ISignal {
  public:
    virtual void GrinderActor_OnExitGrind(Frame* f, EntityRef entity, FP grindingDuration) = 0;
};

class ISignalGroundDashActor_OnDashStarting : public virtual ISignal {
  public:
    virtual void GroundDashActor_OnDashStarting(Frame* f, EntityRef userEntity) = 0;
};

class ISignalGroundDashActor_OnDashEnded : public virtual ISignal {
  public:
    virtual void GroundDashActor_OnDashEnded(Frame* f, EntityRef userEntity, GroundDashEndReason endReason) = 0;
};

class ISignalHealth_OnHealthDecreased : public virtual ISignal {
  public:
    virtual void Health_OnHealthDecreased(Frame* f, EntityRef entity, FP previousHP, FP currentHP, FP maxHP) = 0;
};

class ISignalKccExtension_OnFilterCollision3D : public virtual ISignal {
  public:
    virtual void KccExtension_OnFilterCollision3D(Frame* f, EntityRef character, const Physics3D::Hit3D& hit, bool* ignoreHit) = 0;
};

class ISignalKccExtension_OnStaticWallCollision3D : public virtual ISignal {
  public:
    virtual void KccExtension_OnStaticWallCollision3D(Frame* f, EntityRef character, const Physics3D::Hit3D& hit) = 0;
};

class ISignalKccExtension_OnCharacterCollision3D : public virtual ISignal {
  public:
    virtual void KccExtension_OnCharacterCollision3D(Frame* f, EntityRef character, const Physics3D::Hit3D& hit) = 0;
};

class ISignalKccExtension_OnMovementPackComputed : public virtual ISignal {
  public:
    virtual void KccExtension_OnMovementPackComputed(Frame* f, EntityRef character, CharacterController3DMovementExt* movementPack) = 0;
};

class ISignalKccExtension_OnValidLedgeDetected : public virtual ISignal {
  public:
    virtual void KccExtension_OnValidLedgeDetected(Frame* f, EntityRef character, const Physics3D::Hit3D& hit, const FPVector3& AboveLedgePoint) = 0;
};

class ISignalKccExtension_OnConfigChanged : public virtual ISignal {
  public:
    virtual void KccExtension_OnConfigChanged(Frame* f, EntityRef entity, KccExtension* kccExt) = 0;
};

class ISignalKillable_OnKill : public virtual ISignal {
  public:
    virtual void Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) = 0;
};

class ISignalKillable_OnRevive : public virtual ISignal {
  public:
    virtual void Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position, const FPQuaternion& rotation) = 0;
};

class ISignalLedgeGrabbingActor_OnLedgeGrabStart : public virtual ISignal {
  public:
    virtual void LedgeGrabbingActor_OnLedgeGrabStart(Frame* f, EntityRef entity) = 0;
};

class ISignalLifetime_OnSpawnEntityOnDeath : public virtual ISignal {
  public:
    virtual void Lifetime_OnSpawnEntityOnDeath(Frame* f, const OnSpawnEntityOnDeathParams& spawnParams) = 0;
};

class ISignalMobGroup_OnAllMembersDied : public virtual ISignal {
  public:
    virtual void MobGroup_OnAllMembersDied(Frame* f, EntityRef groupEntity) = 0;
};

class ISignalMoveTo_OnMoveStarted : public virtual ISignal {
  public:
    virtual void MoveTo_OnMoveStarted(Frame* f, EntityRef entity) = 0;
};

class ISignalMoveTo_OnMoveFinished : public virtual ISignal {
  public:
    virtual void MoveTo_OnMoveFinished(Frame* f, EntityRef entity) = 0;
};

class ISignalParabolaFollower_OnRemove : public virtual ISignal {
  public:
    virtual void ParabolaFollower_OnRemove(Frame* f, EntityRef entity, REMOVE_REASON reason) = 0;
};

class ISignalParabolaFollower_OnReachedEnd : public virtual ISignal {
  public:
    virtual void ParabolaFollower_OnReachedEnd(Frame* f, EntityRef entity) = 0;
};

class ISignalPlayerActor_OnInitialized : public virtual ISignal {
  public:
    virtual void PlayerActor_OnInitialized(Frame* f, EntityRef actorEntity, PlayerActor* actor) = 0;
};

class ISignalPlayerActor_OnAfterInitialized : public virtual ISignal {
  public:
    virtual void PlayerActor_OnAfterInitialized(Frame* f, EntityRef actorEntity, PlayerActor* actor) = 0;
};

class ISignalPowerUp_OnCollected : public virtual ISignal {
  public:
    virtual void PowerUp_OnCollected(Frame* f, EntityRef collector, PowerUpType type, CollectionSource source) = 0;
};

class ISignalProjectile_OnSpawnEntityOnDeath : public virtual ISignal {
  public:
    virtual void Projectile_OnSpawnEntityOnDeath(Frame* f, EntityRef spawnedEntity, EntityRef projectileEntity, ProjectileHitInfo* hitInfo) = 0;
};

class ISignalPushable_OnAddPush : public virtual ISignal {
  public:
    virtual void Pushable_OnAddPush(Frame* f, Pushable* pushable, PushData* pushParams) = 0;
};

class ISignalPushable_OnBeforePush : public virtual ISignal {
  public:
    virtual void Pushable_OnBeforePush(Frame* f, EntityRef entity, PushData* pushParams) = 0;
};

class ISignalPushable_OnPushed : public virtual ISignal {
  public:
    virtual void Pushable_OnPushed(Frame* f, const PushData& pushParams, const OnPushedData& onPushedData) = 0;
};

class ISignalPushable_OnPushStopEnd : public virtual ISignal {
  public:
    virtual void Pushable_OnPushStopEnd(Frame* f, EntityRef entity, const PushData& pushParams) = 0;
};

class ISignalRanking_OnLeaderChanged : public virtual ISignal {
  public:
    virtual void Ranking_OnLeaderChanged(Frame* f) = 0;
};

class ISignalReward_OnCollected : public virtual ISignal {
  public:
    virtual void Reward_OnCollected(Frame* f, EntityRef collectorEntity, DroppableType type, std::uint16_t amount) = 0;
};

class ISignalShield_OnBreaks : public virtual ISignal {
  public:
    virtual void Shield_OnBreaks(Frame* f, EntityRef entity, EntityRef sourceEntity, const FPVector3& direction) = 0;
};

class ISignalShield_OnStartRegen : public virtual ISignal {
  public:
    virtual void Shield_OnStartRegen(Frame* f, EntityRef entity) = 0;
};

class ISignalShield_OnFull : public virtual ISignal {
  public:
    virtual void Shield_OnFull(Frame* f, EntityRef entity) = 0;
};

class ISignalTargetedEffect_OnTick : public virtual ISignal {
  public:
    virtual void TargetedEffect_OnTick(Frame* f, EntityRef targetedEffectEntity, EntityRef targetEntity) = 0;
};

class ISignalTargetedEffect_OnTargetSet : public virtual ISignal {
  public:
    virtual void TargetedEffect_OnTargetSet(Frame* f, EntityRef targetedEffectEntity, EntityRef targetEntity) = 0;
};

class ISignalUltimateAbilityUser_OnStateChanged : public virtual ISignal {
  public:
    virtual void UltimateAbilityUser_OnStateChanged(Frame* f, EntityRef entity, UltimateAbilityState state) = 0;
};

class ISignalUltimateAbilityUser_OnRecast : public virtual ISignal {
  public:
    virtual void UltimateAbilityUser_OnRecast(Frame* f, EntityRef entity) = 0;
};

class ISignalOnFeatureUnlocked : public virtual ISignal {
  public:
    virtual void OnFeatureUnlocked(Frame* f, EntityRef entity, UnlockedFeature unlockedFeature) = 0;
};

class ISignalOnFeatureLocked : public virtual ISignal {
  public:
    virtual void OnFeatureLocked(Frame* f, EntityRef entity, UnlockedFeature lockedFeature) = 0;
};

class ISignalOnVoidRepulserShieldStateChanged : public virtual ISignal {
  public:
    virtual void OnVoidRepulserShieldStateChanged(Frame* f, EntityRef entity, VoidRepulserState state) = 0;
};

class ISignalWallRunningActor_OnWallRunningStart : public virtual ISignal {
  public:
    virtual void WallRunningActor_OnWallRunningStart(Frame* f, EntityRef entity) = 0;
};

class ISignalWaveSpawner_OnSpawned : public virtual ISignal {
  public:
    virtual void WaveSpawner_OnSpawned(Frame* f, EntityRef spawned, WaveSpawner* spawner) = 0;
};

class ISignalWeapon_OnCreated : public virtual ISignal {
  public:
    virtual void Weapon_OnCreated(Frame* f, EntityRef weaponEntity) = 0;
};

class ISignalWeapon_OnReload : public virtual ISignal {
  public:
    virtual void Weapon_OnReload(Frame* f, EntityRef weaponEntity, const Weapon& weapon) = 0;
};

class ISignalWeapon_OnShoot : public virtual ISignal {
  public:
    virtual void Weapon_OnShoot(Frame* f, EntityRef weaponEntity, EntityRef ammunitionEntity) = 0;
};

class ISignalWeaponLoadoutActor_OnManualSwap : public virtual ISignal {
  public:
    virtual void WeaponLoadoutActor_OnManualSwap(Frame* f, EntityRef loadoutActor, WeaponLoadoutSlot slot) = 0;
};

class ISignalWeapon_OnBeforeEquip : public virtual ISignal {
  public:
    virtual void Weapon_OnBeforeEquip(Frame* f, EntityRef userEntity, EntityRef weaponEntity) = 0;
};

class ISignalWeapon_OnEquip : public virtual ISignal {
  public:
    virtual void Weapon_OnEquip(Frame* f, EntityRef userEntity, EntityRef weaponEntity) = 0;
};

class ISignalWeapon_OnUnEquip : public virtual ISignal {
  public:
    virtual void Weapon_OnUnEquip(Frame* f, EntityRef userEntity, EntityRef weaponEntity) = 0;
};

class ISignalWeapon_OnBeforeUse : public virtual ISignal {
  public:
    virtual void Weapon_OnBeforeUse(Frame* f, EntityRef userEntity, EntityRef weaponEntity) = 0;
};

class ISignalWeapon_OnUse : public virtual ISignal {
  public:
    virtual void Weapon_OnUse(Frame* f, EntityRef userEntity, EntityRef weaponEntity) = 0;
};

class ISignalOnPlayerDataSet : public virtual ISignal {
  public:
    virtual void OnPlayerDataSet(Frame* f, PlayerRef player) = 0;
};

class ISignalOnEntityPrototypeMaterialized : public virtual ISignal {
  public:
    virtual void OnEntityPrototypeMaterialized(Frame* f, EntityRef entity, const EntityPrototypeRef& prototypeRef) = 0;
};

class ISignalOnPlayerConnected : public virtual ISignal {
  public:
    virtual void OnPlayerConnected(Frame* f, PlayerRef player) = 0;
};

class ISignalOnPlayerDisconnected : public virtual ISignal {
  public:
    virtual void OnPlayerDisconnected(Frame* f, PlayerRef player) = 0;
};

class ISignalOnNavMeshWaypointReached : public virtual ISignal {
  public:
    virtual void OnNavMeshWaypointReached(Frame* f, EntityRef entity, const FPVector3& waypoint, WaypointFlag waypointFlags, bool& resetAgent) = 0;
};

class ISignalOnNavMeshSearchFailed : public virtual ISignal {
  public:
    virtual void OnNavMeshSearchFailed(Frame* f, EntityRef entity, bool& resetAgent) = 0;
};

class ISignalOnNavMeshMoveAgent : public virtual ISignal {
  public:
    virtual void OnNavMeshMoveAgent(Frame* f, EntityRef entity, const FPVector2& desiredDirection) = 0;
};

class ISignalOnCollision3D : public virtual ISignal {
  public:
    virtual void OnCollision3D(Frame* f, const CollisionInfo3D& info) = 0;
};

class ISignalOnCollisionEnter3D : public virtual ISignal {
  public:
    virtual void OnCollisionEnter3D(Frame* f, const CollisionInfo3D& info) = 0;
};

class ISignalOnCollisionExit3D : public virtual ISignal {
  public:
    virtual void OnCollisionExit3D(Frame* f, const ExitInfo3D& info) = 0;
};

class ISignalOnTrigger3D : public virtual ISignal {
  public:
    virtual void OnTrigger3D(Frame* f, const TriggerInfo3D& info) = 0;
};

class ISignalOnTriggerEnter3D : public virtual ISignal {
  public:
    virtual void OnTriggerEnter3D(Frame* f, const TriggerInfo3D& info) = 0;
};

class ISignalOnTriggerExit3D : public virtual ISignal {
  public:
    virtual void OnTriggerExit3D(Frame* f, const ExitInfo3D& info) = 0;
};

} // namespace Quantum
