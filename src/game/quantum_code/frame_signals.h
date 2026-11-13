// frame_signals.h - Quantum.Frame.FrameSignals (quantum.code.dll): raising signals.
//
// Each method calls the signal on every system that implements it, in system order,
// skipping systems that are not enabled in the hierarchy at call time.
#pragma once

#include "quantum_code/signals.h"

namespace Quantum {

struct FrameSignals {
    Frame* _f = nullptr;

    // 0x7c7c460
    explicit FrameSignals(Frame* f) : _f(f) {}

    // 0x7c73dd0
    void AIMoveForward_OnBounce(EntityRef bouncer);
    // 0x7c73f54
    void AISpawner_OnSpawned(EntityRef spawner, EntityRef spawned);
    // 0x7c740e0
    void AISpawner_OnSpawnDone(EntityRef spawner);
    // 0x7c74264
    void PlayerActor_OnInputLocked(EntityRef entity, QBoolean locked);
    // 0x7c743f0
    void ActorStatus_OnChanged(EntityRef actorEntity);
    // 0x7c74574
    void AreaOfEffect_OnCalculateDynamicShape(EntityRef areaOfEffectEntity, Shape3D* outShape);
    // 0x7c74700
    void AreaOfEffect_OnHit(EntityRef areaOfEffectEntity);
    // 0x7c74884
    void AreaOfEffect_OnMiss(EntityRef areaOfEffectEntity);
    // 0x7c74a08
    void AutoJumperActor_OnStartJump(EntityRef entity);
    // 0x7c74b8c
    void AutoJumperActor_OnEndJump(EntityRef entity);
    // 0x7c74d10
    void BlastBallGame_StateChanged(BlastBallGameState newState);
    // 0x7c74e94
    void BlastBallGame_LeadChanged();
    // 0x7c75008
    void Bolt_OnCollected(EntityRef collector, std::int32_t amount);
    // 0x7c75194
    void Bolt_OnLost(EntityRef collector, std::int32_t amount);
    // 0x7c75320
    void Damageable_OnBeforeHit(EntityRef targetEntity, HitData* hitData);
    // 0x7c754ac
    void Damageable_HitReceived(EntityRef entity, const HitData& hitData, const HitResult& hitResult);
    // 0x7c756f8
    void DamageToImpulseConverter_ComputeImpulse(EntityRef entity, const HitData& hitData, DamageToImpulseData* impulseData);
    // 0x7c75924
    void DamageToImpulseConverter_OnBeforeImpulse(EntityRef entity, const DamageToImpulseData& impulseData);
    // 0x7c75b24
    void DamageToImpulseConverter_OnAfterImpulse(EntityRef entity);
    // 0x7c75ca8
    void DashActor_OnBeforeDash(EntityRef entity, FPVector3* dashDir);
    // 0x7c75e34
    void DashActor_OnDashed(EntityRef entity);
    // 0x7c75fb8
    void DashActor_OnDashIsReady(EntityRef entity);
    // 0x7c7613c
    void DashActor_OnAfterDash(EntityRef entity);
    // 0x7c762c0
    void DrillDash_OnStateChanged(EntityRef entity, DrillDash* pDrillDash, DrillDashState newState);
    // 0x7c7645c
    void Droppable_OnDestroyed(const Droppable& droppable);
    // 0x7c7664c
    void GadgetUser_OnGadgetActivated(EntityRef entity);
    // 0x7c767d0
    void GadgetUser_OnGadgetDeactivated(EntityRef entity);
    // 0x7c76954
    void GameMode_OnMainLoopStart(EntityRef gameMode);
    // 0x7c76ad8
    void GameMode_OnStateChanged(GAME_MODE_HANDLER_STATE prevState, GAME_MODE_HANDLER_STATE newState);
    // 0x7c76c64
    void GameMode_OnPlayerEliminated(EntityRef entity);
    // 0x7c76de8
    void GameMode_OnPlayerFinished(EntityRef entity);
    // 0x7c76f6c
    void Bolt_OnGoldenBoltCollected(EntityRef collector);
    // 0x7c770f0
    void GrinderActor_OnExitGrind(EntityRef entity, FP grindingDuration);
    // 0x7c7727c
    void GroundDashActor_OnDashStarting(EntityRef userEntity);
    // 0x7c77400
    void GroundDashActor_OnDashEnded(EntityRef userEntity, GroundDashEndReason endReason);
    // 0x7c7758c
    void Health_OnHealthDecreased(EntityRef entity, FP previousHP, FP currentHP, FP maxHP);
    // 0x7c77730
    void KccExtension_OnFilterCollision3D(EntityRef character, const Physics3D::Hit3D& hit, bool* ignoreHit);
    // 0x7c7795c
    void KccExtension_OnStaticWallCollision3D(EntityRef character, const Physics3D::Hit3D& hit);
    // 0x7c77b80
    void KccExtension_OnCharacterCollision3D(EntityRef character, const Physics3D::Hit3D& hit);
    // 0x7c77da4
    void KccExtension_OnMovementPackComputed(EntityRef character, CharacterController3DMovementExt* movementPack);
    // 0x7c77f30
    void KccExtension_OnValidLedgeDetected(EntityRef character, const Physics3D::Hit3D& hit, const FPVector3& AboveLedgePoint);
    // 0x7c7817c
    void KccExtension_OnConfigChanged(EntityRef entity, KccExtension* kccExt);
    // 0x7c78308
    void Killable_OnKill(EntityRef toKill, const KilledByData& killedBy);
    // 0x7c78508
    void Killable_OnRevive(EntityRef toRevive, const FPVector3& position, const FPQuaternion& rotation);
    // 0x7c78720
    void LedgeGrabbingActor_OnLedgeGrabStart(EntityRef entity);
    // 0x7c788a4
    void Lifetime_OnSpawnEntityOnDeath(const OnSpawnEntityOnDeathParams& spawnParams);
    // 0x7c78a94
    void MobGroup_OnAllMembersDied(EntityRef groupEntity);
    // 0x7c78c18
    void MoveTo_OnMoveStarted(EntityRef entity);
    // 0x7c78d9c
    void MoveTo_OnMoveFinished(EntityRef entity);
    // 0x7c78f20
    void ParabolaFollower_OnRemove(EntityRef entity, REMOVE_REASON reason);
    // 0x7c790ac
    void ParabolaFollower_OnReachedEnd(EntityRef entity);
    // 0x7c79230
    void PlayerActor_OnInitialized(EntityRef actorEntity, PlayerActor* actor);
    // 0x7c793bc
    void PlayerActor_OnAfterInitialized(EntityRef actorEntity, PlayerActor* actor);
    // 0x7c79548
    void PowerUp_OnCollected(EntityRef collector, PowerUpType type, CollectionSource source);
    // 0x7c796e4
    void Projectile_OnSpawnEntityOnDeath(EntityRef spawnedEntity, EntityRef projectileEntity, ProjectileHitInfo* hitInfo);
    // 0x7c79880
    void Pushable_OnAddPush(Pushable* pushable, PushData* pushParams);
    // 0x7c79a0c
    void Pushable_OnBeforePush(EntityRef entity, PushData* pushParams);
    // 0x7c79b98
    void Pushable_OnPushed(const PushData& pushParams, const OnPushedData& onPushedData);
    // 0x7c79dc4
    void Pushable_OnPushStopEnd(EntityRef entity, const PushData& pushParams);
    // 0x7c79fe8
    void Ranking_OnLeaderChanged();
    // 0x7c7a15c
    void Reward_OnCollected(EntityRef collectorEntity, DroppableType type, std::uint16_t amount);
    // 0x7c7a2f8
    void Shield_OnBreaks(EntityRef entity, EntityRef sourceEntity, const FPVector3& direction);
    // 0x7c7a500
    void Shield_OnStartRegen(EntityRef entity);
    // 0x7c7a684
    void Shield_OnFull(EntityRef entity);
    // 0x7c7a808
    void TargetedEffect_OnTick(EntityRef targetedEffectEntity, EntityRef targetEntity);
    // 0x7c7a994
    void TargetedEffect_OnTargetSet(EntityRef targetedEffectEntity, EntityRef targetEntity);
    // 0x7c7ab20
    void UltimateAbilityUser_OnStateChanged(EntityRef entity, UltimateAbilityState state);
    // 0x7c7acac
    void UltimateAbilityUser_OnRecast(EntityRef entity);
    // 0x7c7ae30
    void OnFeatureUnlocked(EntityRef entity, UnlockedFeature unlockedFeature);
    // 0x7c7afbc
    void OnFeatureLocked(EntityRef entity, UnlockedFeature lockedFeature);
    // 0x7c7b148
    void OnVoidRepulserShieldStateChanged(EntityRef entity, VoidRepulserState state);
    // 0x7c7b2d4
    void WallRunningActor_OnWallRunningStart(EntityRef entity);
    // 0x7c7b458
    void WaveSpawner_OnSpawned(EntityRef spawned, WaveSpawner* spawner);
    // 0x7c7b5e4
    void Weapon_OnCreated(EntityRef weaponEntity);
    // 0x7c7b768
    void Weapon_OnReload(EntityRef weaponEntity, const Weapon& weapon);
    // 0x7c7b98c
    void Weapon_OnShoot(EntityRef weaponEntity, EntityRef ammunitionEntity);
    // 0x7c7bb18
    void WeaponLoadoutActor_OnManualSwap(EntityRef loadoutActor, WeaponLoadoutSlot slot);
    // 0x7c7bca4
    void Weapon_OnBeforeEquip(EntityRef userEntity, EntityRef weaponEntity);
    // 0x7c7be30
    void Weapon_OnEquip(EntityRef userEntity, EntityRef weaponEntity);
    // 0x7c7bfbc
    void Weapon_OnUnEquip(EntityRef userEntity, EntityRef weaponEntity);
    // 0x7c7c148
    void Weapon_OnBeforeUse(EntityRef userEntity, EntityRef weaponEntity);
    // 0x7c7c2d4
    void Weapon_OnUse(EntityRef userEntity, EntityRef weaponEntity);
    // 0x7c7c468
    void OnPlayerDataSet(PlayerRef player);
    // 0x7c7c5ec
    void OnEntityPrototypeMaterialized(EntityRef entity, const EntityPrototypeRef& prototypeRef);
    // 0x7c7c788
    void OnPlayerConnected(PlayerRef player);
    // 0x7c7c90c
    void OnPlayerDisconnected(PlayerRef player);
    // 0x7c7ca90
    void OnNavMeshWaypointReached(EntityRef entity, const FPVector3& waypoint, WaypointFlag waypointFlags, bool& resetAgent);
    // 0x7c7ccbc
    void OnNavMeshSearchFailed(EntityRef entity, bool& resetAgent);
    // 0x7c7ce48
    void OnNavMeshMoveAgent(EntityRef entity, const FPVector2& desiredDirection);
    // 0x7c7cfe4
    void OnCollision3D(const CollisionInfo3D& info);
    // 0x7c7d194
    void OnCollisionEnter3D(const CollisionInfo3D& info);
    // 0x7c7d344
    void OnCollisionExit3D(const ExitInfo3D& info);
    // 0x7c7d4d4
    void OnTrigger3D(const TriggerInfo3D& info);
    // 0x7c7d674
    void OnTriggerEnter3D(const TriggerInfo3D& info);
    // 0x7c7d814
    void OnTriggerExit3D(const ExitInfo3D& info);
};

} // namespace Quantum
