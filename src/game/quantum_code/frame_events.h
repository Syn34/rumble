// frame_events.h - Quantum.Frame.FrameEvents (quantum.code.dll): raising events.
//
// Each method takes an event object of its kind from the frame context's pool, fills in
// the arguments, stamps it with the frame number and appends it to the context's event
// list (Frame::AddEvent), then returns it.
#pragma once

#include "quantum_code/event_types.h"

namespace Quantum {

class Frame;

struct FrameEvents {
    static constexpr int EVENT_TYPE_COUNT = 128;
    // Id reserved for the shared base of the entity events (EventBaseEntityEvent).
    static constexpr int EventBaseEntityEvent_ID = 36;

    Frame* _f = nullptr;

    // 0x7c83624
    explicit FrameEvents(Frame* f) : _f(f) {}

    // 0x7c7d9a4 - The id of the event's base event class, -1 for none.
    static int GetParentEventID(int eventID);
    // TODO(0x7c7da0c, FrameEvents.GetEventType): returns the System.Type of an
    // event id; C++ has no equivalent object (EventName gives the name).
    static const char* EventName(int eventID);

    // 0x7c7ecbc
    EventAISpawner_OnSpawned* AISpawner_OnSpawned(EntityRef spawnerEntity, EntityRef spawnedEntity);
    // 0x7c7ed54
    EventAISpawner_OnDespawned* AISpawner_OnDespawned(EntityRef spawnerEntity, EntityRef despawnedEntity);
    // 0x7c7edec
    EventActorIcedChange* ActorIcedChange(EntityRef entity, QBoolean iced);
    // 0x7c7ee88
    EventRespawnTimerCreated* RespawnTimerCreated(EntityRef entity, FP respawnPointCalculationDelay);
    // 0x7c7ef20
    EventRespawnPointSet* RespawnPointSet(EntityRef entity, const FPVector3& spawnPosition, const FPQuaternion& spawnRotation);
    // 0x7c7efd4
    EventRespawnTimerDestroyed* RespawnTimerDestroyed(EntityRef entity);
    // 0x7c7f060
    EventActorSheepedChange* ActorSheepedChange(EntityRef entity, QBoolean sheeped);
    // 0x7c7f0fc
    EventActorStunnedChange* ActorStunnedChange(EntityRef entity, QBoolean stunned);
    // 0x7c7f198
    EventAgentOfDoomEvent* AgentOfDoomEvent(EntityRef entity, AgentOfDoomEventType eventType);
    // 0x7c7f234
    EventAreaOfEffect_OnHit* AreaOfEffect_OnHit(EntityRef entity);
    // 0x7c7f2c0
    EventAreaOfEffect_OnMiss* AreaOfEffect_OnMiss(EntityRef entity);
    // 0x7c7f34c
    EventAreaOfEffect_OnHitInstigator* AreaOfEffect_OnHitInstigator(EntityRef instigator);
    // 0x7c7f3d8
    EventAreaOfEffect_OnEnterArea* AreaOfEffect_OnEnterArea(EntityRef inAreaEntity, EntityRef aoeEntity);
    // 0x7c7f470
    EventAreaOfEffect_OnExitArea* AreaOfEffect_OnExitArea(EntityRef outOfAreaEntity, EntityRef aoeEntity);
    // 0x7c7f508
    EventAutoJumperTriggered* AutoJumperTriggered(EntityRef jumpingEntity, std::int32_t staticColliderIndex);
    // 0x7c7f5a4
    EventBigBoomMine_Triggered* BigBoomMine_Triggered(EntityRef Owner);
    // 0x7c7f630
    EventBlastBallGame_StateChanged* BlastBallGame_StateChanged(BlastBallGameState newState);
    // 0x7c7f6d4
    EventBlastBallGame_StartOverTime* BlastBallGame_StartOverTime();
    // 0x7c7f770
    EventBlastBallGame_TeamScored* BlastBallGame_TeamScored(TeamFlag scoringTeam, std::uint16_t score, EntityRef scoringPlayer);
    // 0x7c7f82c
    EventBlastBallGame_BallHit* BlastBallGame_BallHit(EntityRef player, const FPVector3& position, const FPVector3& direction);
    // 0x7c7f8e8
    EventOnBoltCollected* OnBoltCollected(EntityRef entity, std::int32_t amount);
    // 0x7c7f984
    EventOnBoltLost* OnBoltLost(EntityRef entity, std::int32_t amount);
    // 0x7c7fa20
    EventBoltCounter_OnUpdateBolts* BoltCounter_OnUpdateBolts(EntityRef counterEntity, std::int32_t currentBolts);
    // 0x7c7fabc
    EventBumperTriggered* BumperTriggered(EntityRef bumper, EntityRef player);
    // 0x7c7fb54
    EventCharacterColliderScaler_OnScaleChanged* CharacterColliderScaler_OnScaleChanged(EntityRef entity);
    // 0x7c7fbe0
    EventColliderResize_OnResizeStarted* ColliderResize_OnResizeStarted(EntityRef entity);
    // 0x7c7fc6c
    EventCollisionForce_OnCollision* CollisionForce_OnCollision(EntityRef entity, EntityRef otherEntity, const FPVector3& hitPoint, const FPVector3& hitNormal);
    // 0x7c7fd34
    EventDamageable_HitReceived* Damageable_HitReceived(EntityRef entity, const HitData& hitData, const HitResult& hitResult);
    // 0x7c7fdf0
    EventOnDamageChargeableReset* OnDamageChargeableReset(EntityRef chargeableEntity);
    // 0x7c7fe7c
    EventDashStarted* DashStarted(EntityRef entity);
    // 0x7c7ff08
    EventDashComponentDisabled* DashComponentDisabled(EntityRef entity);
    // 0x7c7ff94
    EventDrillDash_OnStateChanged* DrillDash_OnStateChanged(EntityRef entity, EntityRef weaponEntity, DrillDashState newState);
    // 0x7c80034
    EventOnDroppableDropped* OnDroppableDropped(EntityRef dropperEntity, DroppableType type, const FPVector3& position, std::uint16_t amount);
    // 0x7c8010c
    EventEliminationGameMode_OnCloseToWin* EliminationGameMode_OnCloseToWin(EntityRef entity);
    // 0x7c80198
    EventEnergyCell_OnCollected* EnergyCell_OnCollected(EntityRef collector, std::int32_t value);
    // 0x7c80234
    EventFireReactor_OnStateChanged* FireReactor_OnStateChanged(EntityRef entity, QBoolean enabled);
    // 0x7c802d0
    EventGadgetUser_OnGadgetActivated* GadgetUser_OnGadgetActivated(EntityRef entity);
    // 0x7c8035c
    EventGadgetUser_OnGadgetDeactivated* GadgetUser_OnGadgetDeactivated(EntityRef entity);
    // 0x7c803e8
    EventIntroEnded* IntroEnded(EntityRef entity);
    // 0x7c80474
    EventBoltMultiplierOn* BoltMultiplierOn();
    // 0x7c804f8
    EventGameModeHandlerStateChanged* GameModeHandlerStateChanged(EntityRef entity, GAME_MODE_HANDLER_STATE prevState, GAME_MODE_HANDLER_STATE newState);
    // 0x7c805b0
    EventGameMode_OnPlayerEliminated* GameMode_OnPlayerEliminated(EntityRef entity);
    // 0x7c8063c
    EventGameMode_OnPlayerFinished* GameMode_OnPlayerFinished(EntityRef entity);
    // 0x7c806c8
    EventPreloadAddressablePrefabs* PreloadAddressablePrefabs();
    // 0x7c80764
    EventGoldenBoltSpawned* GoldenBoltSpawned(QBoolean isUnique);
    // 0x7c80808
    EventGoldenBoltCollected* GoldenBoltCollected(EntityRef boltEntity, EntityRef collector);
    // 0x7c808b8
    EventStartGrind* StartGrind(EntityRef entity, QBoolean jumpAnim);
    // 0x7c80954
    EventSnappedToRail* SnappedToRail(EntityRef entity);
    // 0x7c809e0
    EventExitGrind* ExitGrind(EntityRef entity, QBoolean jumpAnim);
    // 0x7c80a7c
    EventSwitchRail* SwitchRail(EntityRef entity, QBoolean isLeft);
    // 0x7c80b18
    EventGroundDashStarting* GroundDashStarting(EntityRef userEntity);
    // 0x7c80ba4
    EventGroundDashStarted* GroundDashStarted(EntityRef userEntity);
    // 0x7c80c30
    EventGroundDashEnded* GroundDashEnded(EntityRef userEntity, GroundDashEndReason endReason);
    // 0x7c80ccc
    EventHealSource_OnHeal* HealSource_OnHeal(EntityRef entity);
    // 0x7c80d58
    EventOnHealthDecreased* OnHealthDecreased(EntityRef entity, FP amount, const HitData& hitData);
    // 0x7c80e04
    EventHelipackStart* HelipackStart(EntityRef entity, FP verticalVelocity);
    // 0x7c80e9c
    EventHelipackStop* HelipackStop(EntityRef entity);
    // 0x7c80f28
    EventItemSpawnerStatus* ItemSpawnerStatus(EntityRef entity, ItemSpawnerStatusType status, QBoolean enabled);
    // 0x7c80fcc
    EventItemSpawnerEntitySpawn* ItemSpawnerEntitySpawn(EntityRef entity);
    // 0x7c81058
    EventKccExtension_OnJumpStateChanged* KccExtension_OnJumpStateChanged(EntityRef entity, JumpState jumpState);
    // 0x7c810f4
    EventKccExtensions_OnMoveSpeedModifierAdded* KccExtensions_OnMoveSpeedModifierAdded(EntityRef entity, FP value, FP duration);
    // 0x7c81194
    EventKillEvent* KillEvent(EntityRef entity, KillType killType, KillZoneSurfaceType killZoneSurfaceType);
    // 0x7c8124c
    EventKillableSpawnEvent* KillableSpawnEvent(EntityRef entity);
    // 0x7c812d8
    EventReviveEvent* ReviveEvent(EntityRef entity, const FPVector3& orientation);
    // 0x7c81380
    EventOnLedgeGrabStart* OnLedgeGrabStart(EntityRef character, FP cameraDistanceOffset, FP cameraVerticalAngle);
    // 0x7c81420
    EventOnLedgeGrabEnd* OnLedgeGrabEnd(EntityRef character);
    // 0x7c814ac
    EventOnLevelButtonPressed* OnLevelButtonPressed(EntityRef entity);
    // 0x7c81538
    EventOnLevelButtonReleased* OnLevelButtonReleased(EntityRef entity);
    // 0x7c815c4
    EventOnArenaCannonActivated* OnArenaCannonActivated(EntityRef entity);
    // 0x7c81650
    EventMeleeAttack_Start* MeleeAttack_Start(EntityRef attackOwner);
    // 0x7c816dc
    EventMeleeAttack_HitOrMissed* MeleeAttack_HitOrMissed(EntityRef attackOwner, QBoolean hit);
    // 0x7c81778
    EventMeleeJumpAttack_OnPerformed* MeleeJumpAttack_OnPerformed(EntityRef entity);
    // 0x7c81804
    EventMobSlamAttackEvent* MobSlamAttackEvent(EntityRef entity, MobSlamAttackState attackState);
    // 0x7c818a0
    EventMobsEvent* MobsEvent(EntityRef entity, MobsEventType eventType);
    // 0x7c8193c
    EventMoveTo_OnMoveStarted* MoveTo_OnMoveStarted(EntityRef entity);
    // 0x7c819c8
    EventMoveTo_OnMoveFinished* MoveTo_OnMoveFinished(EntityRef entity);
    // 0x7c81a54
    EventOnMovingWallStart* OnMovingWallStart(EntityRef entity);
    // 0x7c81ae0
    EventPowerUp_OnStarted* PowerUp_OnStarted(EntityRef PowerUpUserEntity, PowerUpSpec* PowerUpSpec);
    // 0x7c81b84
    EventPowerUp_OnFinished* PowerUp_OnFinished(EntityRef PowerUpUserEntity, PowerUpSpec* PowerUpSpec, std::int32_t stackIndex);
    // 0x7c81c40
    EventProjectile_OnHit* Projectile_OnHit(EntityRef entity, const Physics3D::Hit3D& hit);
    // 0x7c81ce8
    EventOnPushed* OnPushed(EntityRef entity, const PushData& pushData, const FPVector3& force, FP pushModifierStatusEffect);
    // 0x7c81db0
    EventOnPushContinued* OnPushContinued(EntityRef entity, EntityRef pusher, const FPVector3& force);
    // 0x7c81e5c
    EventOnStopPushBegin* OnStopPushBegin(EntityRef entity, EntityRef pusher, const FPVector3& force);
    // 0x7c81f08
    EventOnPushResistanceChanged* OnPushResistanceChanged(EntityRef entity, FP newResistance);
    // 0x7c81fa0
    EventPushStoppedEnemy* PushStoppedEnemy(EntityRef WeaponUser, const FPVector3& UserVelocity, FP PushStopDuration);
    // 0x7c8204c
    EventReward_OnPending* Reward_OnPending(EntityRef entity, const Reward& reward);
    // 0x7c820f8
    EventReward_OnCollected* Reward_OnCollected(EntityRef entity, DroppableType type, std::uint16_t amount);
    // 0x7c8219c
    EventBounceEvent* BounceEvent(EntityRef entity, const FPVector3& position, const FPVector3& normal, std::uint32_t bounceCount);
    // 0x7c82268
    EventShieldHit* ShieldHit(EntityRef entity, const HitData& hitData, FP amount, FP remainingShieldPoints);
    // 0x7c82324
    EventShieldFull* ShieldFull(EntityRef entity);
    // 0x7c823b0
    EventShieldBreaks* ShieldBreaks(EntityRef entity);
    // 0x7c8243c
    EventShootingEnemyEvent* ShootingEnemyEvent(EntityRef entity, ShootingEnemyEventType eventType);
    // 0x7c824d8
    EventShrinkingZone_MoveStart* ShrinkingZone_MoveStart(std::uint16_t zoneIndex);
    // 0x7c82564
    EventSpinningEnemyEvent* SpinningEnemyEvent(EntityRef entity, SpinningEnemyEventType eventType);
    // 0x7c82600
    EventTutorialGameMode_StepChanged* TutorialGameMode_StepChanged(TutorialGameModeStep newStep);
    // 0x7c8268c
    EventUltimateAbilityUser_OnStateChanged* UltimateAbilityUser_OnStateChanged(EntityRef entity, UltimateAbilityState state);
    // 0x7c82728
    EventUltimateAbilityUser_OnRecast* UltimateAbilityUser_OnRecast(EntityRef entity);
    // 0x7c827b4
    EventUltimateEvolution_OnEvolve* UltimateEvolution_OnEvolve(EntityRef entity, std::int32_t level);
    // 0x7c82850
    EventUltimateTank_OnLedgeGrabStart* UltimateTank_OnLedgeGrabStart(EntityRef tankEntity);
    // 0x7c828dc
    EventOnFeatureUnlocked* OnFeatureUnlocked(EntityRef entity, UnlockedFeature unlockedFeature);
    // 0x7c82978
    EventOnFeatureLocked* OnFeatureLocked(EntityRef entity, UnlockedFeature lockedFeature);
    // 0x7c82a14
    EventVoidRepulserShieldStateChange* VoidRepulserShieldStateChange(EntityRef entity, VoidRepulserState state);
    // 0x7c82ab0
    EventOnWallRunningStart* OnWallRunningStart(EntityRef entity, QBoolean left);
    // 0x7c82b4c
    EventOnWallRunningStop* OnWallRunningStop(EntityRef entity);
    // 0x7c82bd8
    EventWaveManager_OnWaveStarted* WaveManager_OnWaveStarted(std::int32_t waveIndex, std::int32_t wavesCount);
    // 0x7c82c88
    EventWaveManager_OnWaveStep* WaveManager_OnWaveStep(std::int32_t waveIndex, std::int32_t wavesCount, std::int32_t stepIndex);
    // 0x7c82d40
    EventWaveManager_OnWaveComplete* WaveManager_OnWaveComplete(std::int32_t waveIndex, std::int32_t wavesCount, WaveCompletionReason reason);
    // 0x7c82df8
    EventWaveSpawner_OnSpawned* WaveSpawner_OnSpawned(EntityRef spawner, QBoolean isBoss);
    // 0x7c82e94
    EventWeaponBehaviour_Charging_OnStartCharging* WeaponBehaviour_Charging_OnStartCharging(EntityRef weaponEntity, EntityRef weaponUser);
    // 0x7c82f2c
    EventWeaponUser_Activation* WeaponUser_Activation(EntityRef WeaponUser, QBoolean Disabled);
    // 0x7c82fc8
    EventWeaponUser_Equipped* WeaponUser_Equipped(EntityRef WeaponUser, EntityRef Weapon);
    // 0x7c83060
    EventWeaponUser_PrepareUse* WeaponUser_PrepareUse(EntityRef WeaponUser, EntityRef Weapon);
    // 0x7c830f8
    EventWeaponUser_Use* WeaponUser_Use(EntityRef WeaponUser, EntityRef Weapon);
    // 0x7c83190
    EventWeaponUser_StopUse* WeaponUser_StopUse(EntityRef WeaponUser);
    // 0x7c8321c
    EventWeaponUser_StartReload* WeaponUser_StartReload(EntityRef WeaponUser);
    // 0x7c832a8
    EventWeaponUser_EndReload* WeaponUser_EndReload(EntityRef WeaponUser);
    // 0x7c83334
    EventOnArmWrenchThrow* OnArmWrenchThrow(EntityRef WeaponEntity);
    // 0x7c833c0
    EventOnStartWrenchThrow* OnStartWrenchThrow(EntityRef WeaponEntity);
    // 0x7c8344c
    EventOnEndWrenchThrow* OnEndWrenchThrow(EntityRef WeaponEntity);
    // 0x7c834d8
    EventWrenchWeapon_UseWrench* WrenchWeapon_UseWrench(EntityRef weapon, std::uint16_t comboCounter);
    // 0x7c83574
    EventZoneControl_OnZoneStateChanged* ZoneControl_OnZoneStateChanged(CONTROL_ZONE_STATE prevState, CONTROL_ZONE_STATE newState, const ControlZoneStateHolder& stateHolder);
};

} // namespace Quantum
