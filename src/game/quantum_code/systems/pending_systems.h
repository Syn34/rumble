// pending_systems.h - systems of quantum.code.dll that are not reimplemented yet.
//
// Each class has the original's name, its place in the system list (SystemSetup) and its
// start-enabled state, so system indices, enable bits and scheduling order match the
// original; its methods are TODO stubs that do nothing. Generated from the original's
// type list; a system moves out of this file when it is reimplemented.
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

// Quantum.RankingSystem (original base: Quantum.SystemMainThread)
class RankingSystem : public SystemMainThread {
  public:
    // 0x7bca83c
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bca844, Quantum.RankingSystem.OnInit): OnInit(frame); not reimplemented yet.
    // TODO(0x7bca8d0, Quantum.RankingSystem.Update): Update(frame); not reimplemented yet.
    // TODO(0x7bca9a4, Quantum.RankingSystem.UpdateRanking): UpdateRanking(frame, rankingManager); not reimplemented yet.
    // TODO(0x7bcadc4, Quantum.RankingSystem.SetNeedUpdate): SetNeedUpdate(frame); not reimplemented yet.
    // TODO(0x7bcae58, Quantum.RankingSystem.GameMode_OnStateChanged): GameMode_OnStateChanged(frame, prevState, newState); not reimplemented yet.
    // TODO(0x7bcaf54, Quantum.RankingSystem.CompareRankingItems): CompareRankingItems(a, b); not reimplemented yet.
    // TODO(0x7bcafe4, Quantum.RankingSystem..ctor): .ctor(); not reimplemented yet.
};

// CollisionSystem (original base: Quantum.SystemSignalsOnly)
class CollisionSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7b7c290, CollisionSystem.OnAdded): OnAdded(frame, entity, pBumper); not reimplemented yet.
    // TODO(0x7b7c29c, CollisionSystem.OnAdded): OnAdded(frame, entity, pBumperBar); not reimplemented yet.
    // TODO(0x7b7c2a8, CollisionSystem.OnTriggerEnter3D): OnTriggerEnter3D(f, info); not reimplemented yet.
    // TODO(0x7b7c5d8, CollisionSystem.TryGetStaticTriggerConfig): TryGetStaticTriggerConfig(f, staticData, triggerConfig); not reimplemented yet.
    // TODO(0x7b7c87c, CollisionSystem.OnTrigger3D): OnTrigger3D(f, info); not reimplemented yet.
    // TODO(0x7b7c69c, CollisionSystem.OnPlayerActorEnter): OnPlayerActorEnter(f, triggerEntity, actorEntity, pActor); not reimplemented yet.
    // TODO(0x7b7c910, CollisionSystem.OnTriggerExit3D): OnTriggerExit3D(f, info); not reimplemented yet.
    // TODO(0x7b7cb04, CollisionSystem..ctor): .ctor(); not reimplemented yet.
};

// KillZoneSystem (original base: Quantum.SystemMainThread)
class KillZoneSystem : public SystemMainThread {
  public:
    // 0x7b7be28
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7b7be30, KillZoneSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7b7be50, KillZoneSystem.Update): Update(f); not reimplemented yet.
    // TODO(0x7b7c100, KillZoneSystem.BelowThreshold): BelowThreshold(frame, killzone, position, foundArea); not reimplemented yet.
    // TODO(0x7b7c288, KillZoneSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AIBotSDKSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AIBotSDKSystem : public SystemMainThread {
  public:
    // 0x7cbdb48
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7cbdb50, Quantum.AIBotSDKSystem.OnEnabled): OnEnabled(f); not reimplemented yet.
    // TODO(0x7cbdd9c, Quantum.AIBotSDKSystem.InitializeUT): InitializeUT(f, entityUT, parentEntity, suffix); not reimplemented yet.
    // TODO(0x7cbdc60, Quantum.AIBotSDKSystem.InitBotSDK): InitBotSDK(f, entity, pBot); not reimplemented yet.
    // TODO(0x7cbe008, Quantum.AIBotSDKSystem.OnAdded): OnAdded(f, entity, pBot); not reimplemented yet.
    // TODO(0x7cbe00c, Quantum.AIBotSDKSystem.OnRemoved): OnRemoved(f, entity, pBot); not reimplemented yet.
    // TODO(0x7cbde9c, Quantum.AIBotSDKSystem.Reset): Reset(f, entity); not reimplemented yet.
    // TODO(0x7cbe0a8, Quantum.AIBotSDKSystem.Killable_OnKill): Killable_OnKill(f, entity, killedBy); not reimplemented yet.
    // TODO(0x7cbe140, Quantum.AIBotSDKSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(f, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7cbe434, Quantum.AIBotSDKSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cbe788, Quantum.AIBotSDKSystem.UpdateUT): UpdateUT(f, agentEntity, parentEntity, aiContext); not reimplemented yet.
    // TODO(0x7cbe838, Quantum.AIBotSDKSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AIMoveForwardSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AIMoveForwardSystem : public SystemMainThread {
  public:
    // TODO(0x7cc89d8, Quantum.AIMoveForwardSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cc8b84, Quantum.AIMoveForwardSystem.OnEnabled): OnEnabled(f); not reimplemented yet.
    // TODO(0x7cc8ca0, Quantum.AIMoveForwardSystem.OnDisabled): OnDisabled(f); not reimplemented yet.
    // TODO(0x7cc8e00, Quantum.AIMoveForwardSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cc9424, Quantum.AIMoveForwardSystem.KccExtension_OnStaticWallCollision3D): KccExtension_OnStaticWallCollision3D(frame, character, hit); not reimplemented yet.
    // TODO(0x7cc9538, Quantum.AIMoveForwardSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(frame, character, hit); not reimplemented yet.
    // TODO(0x7cc74d8, Quantum.AIMoveForwardSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AINavBlacklistSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AINavBlacklistSystem : public SystemMainThread {
  public:
    // TODO(0x7cd58f8, Quantum.AINavBlacklistSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd5b90, Quantum.AINavBlacklistSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AINavigationInitialiserSystem (original base: Quantum.SystemSignalsOnly)
class AINavigationInitialiserSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cd5bd8, Quantum.AINavigationInitialiserSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cd5f48, Quantum.AINavigationInitialiserSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AINavigationSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AINavigationSystem : public SystemMainThread {
  public:
    // 0x7cd5f50
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7cd5f58, Quantum.AINavigationSystem.OnEnabled): OnEnabled(f); not reimplemented yet.
    // TODO(0x7cd6394, Quantum.AINavigationSystem.OnAdded): OnAdded(f, entity, aiNav); not reimplemented yet.
    // TODO(0x7cd6398, Quantum.AINavigationSystem.Killable_OnKill): Killable_OnKill(f, entity, killedBy); not reimplemented yet.
    // TODO(0x7cd64b8, Quantum.AINavigationSystem.Killable_OnRevive): Killable_OnRevive(f, entity, position, rotation); not reimplemented yet.
    // TODO(0x7cd6058, Quantum.AINavigationSystem.InitPathFinder): InitPathFinder(f, entity, aiNav); not reimplemented yet.
    // TODO(0x7cd6560, Quantum.AINavigationSystem.ToggleRegion): ToggleRegion(f, pathFinder, region, enabled); not reimplemented yet.
    // TODO(0x7cd63a4, Quantum.AINavigationSystem.ResetPathfinding): ResetPathfinding(f, entity); not reimplemented yet.
    // TODO(0x7cd6688, Quantum.AINavigationSystem.OnNavMeshSearchFailed): OnNavMeshSearchFailed(f, entity, resetAgent); not reimplemented yet.
    // TODO(0x7cd668c, Quantum.AINavigationSystem.MarkPathFindingFailed): MarkPathFindingFailed(f, entity); not reimplemented yet.
    // TODO(0x7cccafc, Quantum.AINavigationSystem.FindNearestNavMeshPos): FindNearestNavMeshPos(f, position, maxDistanceXZFromPosition, maxTries, navMeshPos); not reimplemented yet.
    // TODO(0x7ccc830, Quantum.AINavigationSystem.FindGroundUnderPosition): FindGroundUnderPosition(f, position, maxYDistance, groundPosition, skipSecondaryShapeCast); not reimplemented yet.
    // TODO(0x7cd68c8, Quantum.AINavigationSystem.SetDestination): SetDestination(f, entity, aiNav, destination); not reimplemented yet.
    // TODO(0x7cd69dc, Quantum.AINavigationSystem.StopPathFinding): StopPathFinding(f, entity); not reimplemented yet.
    // TODO(0x7cd69e0, Quantum.AINavigationSystem.MaintainDirectionForAirSave): MaintainDirectionForAirSave(f, entity, pBot, actorInput); not reimplemented yet.
    // TODO(0x7cd6a20, Quantum.AINavigationSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd7de0, Quantum.AINavigationSystem.HandleRandomJump): HandleRandomJump(f, filter, bot); not reimplemented yet.
    // TODO(0x7cd84ec, Quantum.AINavigationSystem.ScheduleNextRandomJump): ScheduleNextRandomJump(f, bot, config); not reimplemented yet.
    // TODO(0x7cd80d8, Quantum.AINavigationSystem.SetInputDirection): SetInputDirection(f, filter, direction, normalize); not reimplemented yet.
    // TODO(0x7cd7c34, Quantum.AINavigationSystem.CheckDestination): CheckDestination(f, agentEntity, aiNavigation, pathFinder); not reimplemented yet.
    // TODO(0x7cd7b30, Quantum.AINavigationSystem.SetLookDirection): SetLookDirection(transform, actorInput, lookAtDir); not reimplemented yet.
    // TODO(0x7cd810c, Quantum.AINavigationSystem.TryJumpToPosition): TryJumpToPosition(f, filter, targetPosition, starLink, ignoreHeightDiff); not reimplemented yet.
    // TODO(0x7cd8530, Quantum.AINavigationSystem.IsOnRailOrIntendingToTakeARail): IsOnRailOrIntendingToTakeARail(f, entity); not reimplemented yet.
    // TODO(0x7cd83f8, Quantum.AINavigationSystem.IsGrindingOnRail): IsGrindingOnRail(f, entity); not reimplemented yet.
    // TODO(0x7cd865c, Quantum.AINavigationSystem.IsTakingBumper): IsTakingBumper(f, entity); not reimplemented yet.
    // TODO(0x7cd884c, Quantum.AINavigationSystem.IsJumping): IsJumping(frame, entity); not reimplemented yet.
    // TODO(0x7cd88f8, Quantum.AINavigationSystem.IsFreeNavigationPosition): IsFreeNavigationPosition(frame, position, radius); not reimplemented yet.
    // TODO(0x7cd8ae8, Quantum.AINavigationSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AIRespawnDeadSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AIRespawnDeadSystem : public SystemMainThread {
  public:
    // TODO(0x7cd8b30, Quantum.AIRespawnDeadSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd8dd0, Quantum.AIRespawnDeadSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AISpawnerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AISpawnerSystem : public SystemMainThread {
  public:
    // TODO(0x7cd97cc, Quantum.AISpawnerSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7cd9854, Quantum.AISpawnerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cd9b5c, Quantum.AISpawnerSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cda1b4, Quantum.AISpawnerSystem.MobGroup_OnAllMembersDied): MobGroup_OnAllMembersDied(f, groupEntity); not reimplemented yet.
    // TODO(0x7cda26c, Quantum.AISpawnerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorCombatActivitySystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorCombatActivitySystem : public SystemMainThread {
  public:
    // TODO(0x7cb1a88, Quantum.ActorCombatActivitySystem.Killable_OnRevive): Killable_OnRevive(f, entity, position, rotation); not reimplemented yet.
    // TODO(0x7cb1b20, Quantum.ActorCombatActivitySystem.Pushable_OnPushed): Pushable_OnPushed(f, pushData, onPushedData); not reimplemented yet.
    // TODO(0x7cb1c58, Quantum.ActorCombatActivitySystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb1d08, Quantum.ActorCombatActivitySystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7cb1dd8, Quantum.ActorCombatActivitySystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorIcedSystem (original base: Quantum.SystemSignalsOnly)
class ActorIcedSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb9c10, Quantum.ActorIcedSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9c54, Quantum.ActorIcedSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9c98, Quantum.ActorIcedSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorInitialisedSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorInitialisedSystem : public SystemMainThread {
  public:
    // TODO(0x7cb1e20, Quantum.ActorInitialisedSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7cb1ea8, Quantum.ActorInitialisedSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb1f20, Quantum.ActorInitialisedSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorInputConverterSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorInputConverterSystem : public SystemMainThread {
  public:
    // TODO(0x7cb201c, Quantum.ActorInputConverterSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb23c8, Quantum.ActorInputConverterSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorInputInitialiserSystem (original base: Quantum.SystemSignalsOnly)
class ActorInputInitialiserSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb1f68, Quantum.ActorInputInitialiserSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb2018, Quantum.ActorInputInitialiserSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorInputUpdateSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorInputUpdateSystem : public SystemMainThread {
  public:
    // TODO(0x7cb2410, Quantum.ActorInputUpdateSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb263c, Quantum.ActorInputUpdateSystem.HandlePlayerCommands): HandlePlayerCommands(frame, entity); not reimplemented yet.
    // TODO(0x7cb27c4, Quantum.ActorInputUpdateSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorKillsHistorySystem (original base: Quantum.SystemSignalsOnly)
class ActorKillsHistorySystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bb0f9c, Quantum.ActorKillsHistorySystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bb1120, Quantum.ActorKillsHistorySystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorManagerRespawnTesterSystem (original base: Quantum.SystemSignalsOnly)
class ActorManagerRespawnTesterSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb2b54, Quantum.ActorManagerRespawnTesterSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7cb2d1c, Quantum.ActorManagerRespawnTesterSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorManagerSystem (original base: Quantum.SystemMainThread)
class ActorManagerSystem : public SystemMainThread {
  public:
    // TODO(0x7cb2834, Quantum.ActorManagerSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7cb2924, Quantum.ActorManagerSystem.Update): Update(frame); not reimplemented yet.
    // TODO(0x7cb299c, Quantum.ActorManagerSystem.Quantum.ISignalOnMapChanged.OnMapChanged): Quantum.ISignalOnMapChanged.OnMapChanged(f, previousMap); not reimplemented yet.
    // TODO(0x7cb2a18, Quantum.ActorManagerSystem.OnAdded): OnAdded(f, entity, pActor); not reimplemented yet.
    // TODO(0x7cb2a70, Quantum.ActorManagerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb2a90, Quantum.ActorManagerSystem.GameMode_OnPlayerFinished): GameMode_OnPlayerFinished(f, entity); not reimplemented yet.
    // TODO(0x7cb2b2c, Quantum.ActorManagerSystem.PlayerActor_OnAfterInitialized): PlayerActor_OnAfterInitialized(f, actorEntity, actor); not reimplemented yet.
    // TODO(0x7cb2b50, Quantum.ActorManagerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorSheepedSystem (original base: Quantum.SystemSignalsOnly)
class ActorSheepedSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb9d8c, Quantum.ActorSheepedSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9dd0, Quantum.ActorSheepedSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9e14, Quantum.ActorSheepedSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorSlowDownOnHitSystem (original base: Quantum.SystemSignalsOnly)
class ActorSlowDownOnHitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bb7f8c, Quantum.ActorSlowDownOnHitSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bb813c, Quantum.ActorSlowDownOnHitSystem.DashActor_OnDashed): DashActor_OnDashed(f, entity); not reimplemented yet.
    // TODO(0x7bb8204, Quantum.ActorSlowDownOnHitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ActorStatusChangeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorStatusChangeSystem : public SystemMainThread {
  public:
    // TODO(0x7cb8b80, Quantum.ActorStatusChangeSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb8bb4, Quantum.ActorStatusChangeSystem.Killable_OnRevive): Killable_OnRevive(f, toRevive, pos, rot); not reimplemented yet.
    // TODO(0x7cb8bc8, Quantum.ActorStatusChangeSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb9274, Quantum.ActorStatusChangeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorStatusSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ActorStatusSystem : public SystemMainThread {
  public:
    // TODO(0x7cb965c, Quantum.ActorStatusSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7cb96e4, Quantum.ActorStatusSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb9adc, Quantum.ActorStatusSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ActorStunnedSystem (original base: Quantum.SystemSignalsOnly)
class ActorStunnedSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb9f08, Quantum.ActorStunnedSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9f4c, Quantum.ActorStunnedSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7cb9f90, Quantum.ActorStunnedSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AddDamageIncreaseOnTargetedEffectSystem (original base: Quantum.SystemSignalsOnly)
class AddDamageIncreaseOnTargetedEffectSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bed540, Quantum.AddDamageIncreaseOnTargetedEffectSystem.TargetedEffect_OnTargetSet): TargetedEffect_OnTargetSet(f, targetedEffectEntity, targetEntity); not reimplemented yet.
    // TODO(0x7bed7b8, Quantum.AddDamageIncreaseOnTargetedEffectSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AddEntitiesToSpawnOnProjectileDeathSystem (original base: Quantum.SystemSignalsOnly)
class AddEntitiesToSpawnOnProjectileDeathSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bec8c8, Quantum.AddEntitiesToSpawnOnProjectileDeathSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(f, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7becafc, Quantum.AddEntitiesToSpawnOnProjectileDeathSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AddForceOnUltimateSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AddForceOnUltimateSystem : public SystemMainThread {
  public:
    // TODO(0x7c0ace0, Quantum.AddForceOnUltimateSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7c0b044, Quantum.AddForceOnUltimateSystem.OnImpulseStart): OnImpulseStart(f, entity, kcc); not reimplemented yet.
    // TODO(0x7c0b05c, Quantum.AddForceOnUltimateSystem.GetDirection): GetDirection(f, entity, addForceOnUltimateSpec); not reimplemented yet.
    // TODO(0x7c0b190, Quantum.AddForceOnUltimateSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7c0b378, Quantum.AddForceOnUltimateSystem.ExitGrind): ExitGrind(f, entity, pKccExt, pKcc); not reimplemented yet.
    // TODO(0x7c0b514, Quantum.AddForceOnUltimateSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AgentOfDoomGroupSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AgentOfDoomGroupSystem : public SystemMainThread {
  public:
    // TODO(0x7cd25b0, Quantum.AgentOfDoomGroupSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7cd2638, Quantum.AgentOfDoomGroupSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd29c8, Quantum.AgentOfDoomGroupSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AimBotSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AimBotSystem : public SystemMainThread {
  public:
    // TODO(0x7cc5e78, Quantum.AimBotSystem.HasPendingFireRequest): HasPendingFireRequest(f, bot); not reimplemented yet.
    // TODO(0x7cc2a78, Quantum.AimBotSystem.HasPendingShootRequest): HasPendingShootRequest(f, bot); not reimplemented yet.
    // TODO(0x7cc2a94, Quantum.AimBotSystem.ResetFireRequest): ResetFireRequest(f, bot); not reimplemented yet.
    // TODO(0x7cc5e94, Quantum.AimBotSystem.RequestFire): RequestFire(f, pBot, type, pressDuration, cooldownDuration); not reimplemented yet.
    // TODO(0x7cc5544, Quantum.AimBotSystem.UseWrenchNow): UseWrenchNow(f, bot); not reimplemented yet.
    // TODO(0x7cc7520, Quantum.AimBotSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cc7490, Quantum.AimBotSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AmoeboidLauncherEnhancementPuddleSystem (original base: Quantum.SystemSignalsOnly)
class AmoeboidLauncherEnhancementPuddleSystem : public SystemSignalsOnly {
  public:
    // 0x7beef20
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7beef28, Quantum.AmoeboidLauncherEnhancementPuddleSystem.AISpawner_OnSpawned): AISpawner_OnSpawned(f, spawner, spawned); not reimplemented yet.
    // TODO(0x7bef158, Quantum.AmoeboidLauncherEnhancementPuddleSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AoeTrailSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AoeTrailSystem : public SystemMainThread {
  public:
    // TODO(0x7bf3e90, Quantum.AoeTrailSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf4ab8, Quantum.AoeTrailSystem.OnAdded): OnAdded(f, entity, aoeTrail); not reimplemented yet.
    // TODO(0x7bf4b2c, Quantum.AoeTrailSystem.OnRemoved): OnRemoved(f, entity, aoeTrail); not reimplemented yet.
    // TODO(0x7bf4b48, Quantum.AoeTrailSystem.AreaOfEffect_OnCalculateDynamicShape): AreaOfEffect_OnCalculateDynamicShape(f, areaOfEffectEntity, outShape); not reimplemented yet.
    // TODO(0x7bf5050, Quantum.AoeTrailSystem.HandleCollision): HandleCollision(f, aoeTrail, info); not reimplemented yet.
    // TODO(0x7bf48e8, Quantum.AoeTrailSystem.GenerateFramePoint): GenerateFramePoint(f, _transform, _localOffset); not reimplemented yet.
    // TODO(0x7bf49c4, Quantum.AoeTrailSystem.AddPoint): AddPoint(f, _point, _trail); not reimplemented yet.
    // TODO(0x7bf5230, Quantum.AoeTrailSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ApplyingDamageMultiplierSystem (original base: Quantum.SystemSignalsOnly)
class ApplyingDamageMultiplierSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7becb04, Quantum.ApplyingDamageMultiplierSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7becc4c, Quantum.ApplyingDamageMultiplierSystem.CheckApplyDamageMultiplier): CheckApplyDamageMultiplier(f, hitData, damageMultiplierSpec, checkDamageFilterType); not reimplemented yet.
    // TODO(0x7bece2c, Quantum.ApplyingDamageMultiplierSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AreaOfEffectActorStatusSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectActorStatusSystem : public SystemMainThread {
  public:
    // TODO(0x7bb2408, Quantum.AreaOfEffectActorStatusSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb284c, Quantum.AreaOfEffectActorStatusSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectHealingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectHealingSystem : public SystemMainThread {
  public:
    // TODO(0x7bb2894, Quantum.AreaOfEffectHealingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb2d1c, Quantum.AreaOfEffectHealingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectHitsFilteringSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectHitsFilteringSystem : public SystemMainThread {
  public:
    // TODO(0x7bb2d64, Quantum.AreaOfEffectHitsFilteringSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb33bc, Quantum.AreaOfEffectHitsFilteringSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectIgnitionSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectIgnitionSystem : public SystemMainThread {
  public:
    // TODO(0x7bb385c, Quantum.AreaOfEffectIgnitionSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bb3908, Quantum.AreaOfEffectIgnitionSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb3f48, Quantum.AreaOfEffectIgnitionSystem.AreaOfEffect_OnCalculateDynamicShape): AreaOfEffect_OnCalculateDynamicShape(f, areaOfEffectEntity, outShape); not reimplemented yet.
    // TODO(0x7bb40b4, Quantum.AreaOfEffectIgnitionSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectInitSystem (original base: Quantum.SystemSignalsOnly)
class AreaOfEffectInitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bb40fc, Quantum.AreaOfEffectInitSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bb4360, Quantum.AreaOfEffectInitSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(frame, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7bb4414, Quantum.AreaOfEffectInitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AreaOfEffectKccModifierSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectKccModifierSystem : public SystemMainThread {
  public:
    // TODO(0x7bb5eb8, Quantum.AreaOfEffectKccModifierSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb62a0, Quantum.AreaOfEffectKccModifierSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectLevelUpSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectLevelUpSystem : public SystemMainThread {
  public:
    // TODO(0x7bb441c, Quantum.AreaOfEffectLevelUpSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb49b0, Quantum.AreaOfEffectLevelUpSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectLifetimeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectLifetimeSystem : public SystemMainThread {
  public:
    // TODO(0x7bb49f8, Quantum.AreaOfEffectLifetimeSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb5188, Quantum.AreaOfEffectLifetimeSystem.Lifetime_OnSpawnEntityOnDeath): Lifetime_OnSpawnEntityOnDeath(frame, spawnParams); not reimplemented yet.
    // TODO(0x7bb529c, Quantum.AreaOfEffectLifetimeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectOverlapSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectOverlapSystem : public SystemMainThread {
  public:
    // TODO(0x7bb122c, Quantum.AreaOfEffectOverlapSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb2328, Quantum.AreaOfEffectOverlapSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectPusherSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectPusherSystem : public SystemMainThread {
  public:
    // TODO(0x7bb52e4, Quantum.AreaOfEffectPusherSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7bb536c, Quantum.AreaOfEffectPusherSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb5880, Quantum.AreaOfEffectPusherSystem.CalculateImpulseDirection): CalculateImpulseDirection(frame, aoeEntity, transform, spec, hit); not reimplemented yet.
    // TODO(0x7bb5e70, Quantum.AreaOfEffectPusherSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AreaOfEffectStartSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AreaOfEffectStartSystem : public SystemMainThread {
  public:
    // TODO(0x7bb62e8, Quantum.AreaOfEffectStartSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb6420, Quantum.AreaOfEffectStartSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ArenaCannonSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ArenaCannonSystem : public SystemMainThread {
  public:
    // TODO(0x7bd7acc, Quantum.ArenaCannonSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd7cc8, Quantum.ArenaCannonSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.AutoDestroyOnMapChangedSystem (original base: Quantum.SystemSignalsOnly)
class AutoDestroyOnMapChangedSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be1b40, Quantum.AutoDestroyOnMapChangedSystem.Quantum.ISignalOnMapChanged.OnMapChanged): Quantum.ISignalOnMapChanged.OnMapChanged(f, previousMap); not reimplemented yet.
    // TODO(0x7be1c50, Quantum.AutoDestroyOnMapChangedSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.AutoJumperActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class AutoJumperActorSystem : public SystemMainThread {
  public:
    // 0x7bd5908
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bd5910, Quantum.AutoJumperActorSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bd5c98, Quantum.AutoJumperActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BattleStatsActorSystem (original base: Quantum.SystemSignalsOnly)
class BattleStatsActorSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cb2d20, Quantum.BattleStatsActorSystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, pActor); not reimplemented yet.
    // TODO(0x7cb3068, Quantum.BattleStatsActorSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7cb3440, Quantum.BattleStatsActorSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb3860, Quantum.BattleStatsActorSystem.Bolt_OnGoldenBoltCollected): Bolt_OnGoldenBoltCollected(f, collector); not reimplemented yet.
    // TODO(0x7cb3900, Quantum.BattleStatsActorSystem.Bolt_OnCollected): Bolt_OnCollected(f, collector, amount); not reimplemented yet.
    // TODO(0x7cb39a4, Quantum.BattleStatsActorSystem.GrinderActor_OnExitGrind): GrinderActor_OnExitGrind(f, entity, grindingDuration); not reimplemented yet.
    // TODO(0x7cb3a48, Quantum.BattleStatsActorSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(frame, entity, state); not reimplemented yet.
    // TODO(0x7cb3af0, Quantum.BattleStatsActorSystem.GadgetUser_OnGadgetActivated): GadgetUser_OnGadgetActivated(f, entity); not reimplemented yet.
    // TODO(0x7cb3b90, Quantum.BattleStatsActorSystem.PowerUp_OnCollected): PowerUp_OnCollected(f, collector, type, source); not reimplemented yet.
    // TODO(0x7cb2ec0, Quantum.BattleStatsActorSystem.TryGetWeaponUsageStats): TryGetWeaponUsageStats(f, pBattleStatsActor, entity, weapon, stats); not reimplemented yet.
    // TODO(0x7cb3d50, Quantum.BattleStatsActorSystem.Weapon_OnReload): Weapon_OnReload(f, weaponEntity, weapon); not reimplemented yet.
    // TODO(0x7cb3e0c, Quantum.BattleStatsActorSystem.WeaponLoadoutActor_OnManualSwap): WeaponLoadoutActor_OnManualSwap(f, loadoutActor, slot); not reimplemented yet.
    // TODO(0x7cb30f8, Quantum.BattleStatsActorSystem.ComputeStatsForDamageTaken): ComputeStatsForDamageTaken(f, entity, hitData); not reimplemented yet.
    // TODO(0x7cb3224, Quantum.BattleStatsActorSystem.ComputeStatsForDamageDealt): ComputeStatsForDamageDealt(f, entity, hitData); not reimplemented yet.
    // TODO(0x7cb34d0, Quantum.BattleStatsActorSystem.ComputeStatsForVictim): ComputeStatsForVictim(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb35f8, Quantum.BattleStatsActorSystem.ComputeStatsForKiller): ComputeStatsForKiller(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cb3ec0, Quantum.BattleStatsActorSystem.IsEntityOwnedByPlayer): IsEntityOwnedByPlayer(f, entity); not reimplemented yet.
    // TODO(0x7cb3f54, Quantum.BattleStatsActorSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.BigBoomMineSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BigBoomMineSystem : public SystemMainThread {
  public:
    // TODO(0x7bef340, Quantum.BigBoomMineSystem.OnAdded): OnAdded(f, entity, mine); not reimplemented yet.
    // TODO(0x7bef368, Quantum.BigBoomMineSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bef45c, Quantum.BigBoomMineSystem.HandleMineSticking): HandleMineSticking(f, entity, transform, mine, hierarchy, bigBoomMineSpec); not reimplemented yet.
    // TODO(0x7bef574, Quantum.BigBoomMineSystem.CheckMineFallingAndSticking): CheckMineFallingAndSticking(f, transform, mine, hierarchy, bigBoomMineSpec); not reimplemented yet.
    // TODO(0x7bef84c, Quantum.BigBoomMineSystem.CheckMineGettingUnstuck): CheckMineGettingUnstuck(f, transform, mine, hierarchy); not reimplemented yet.
    // TODO(0x7bef4d0, Quantum.BigBoomMineSystem.HandleMineExplosion): HandleMineExplosion(f, entity, mine, ownership, bigBoomMineSpec); not reimplemented yet.
    // TODO(0x7bef904, Quantum.BigBoomMineSystem.CheckMineExplosionTriggered): CheckMineExplosionTriggered(f, mine, ownership); not reimplemented yet.
    // TODO(0x7befa10, Quantum.BigBoomMineSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BlackboardInitializerSystem (original base: Quantum.SystemSignalsOnly)
class BlackboardInitializerSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cc9d38, Quantum.BlackboardInitializerSystem.OnAdded): OnAdded(f, entity, initializer); not reimplemented yet.
    // TODO(0x7cc9ea0, Quantum.BlackboardInitializerSystem.OnRemoved): OnRemoved(f, entity, blackboardComponent); not reimplemented yet.
    // TODO(0x7cc9eac, Quantum.BlackboardInitializerSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.BlastBallGameSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BlastBallGameSystem : public SystemMainThread {
  public:
    // 0x7cdeeb8
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7cdeec0, Quantum.BlastBallGameSystem.OnEnabled): OnEnabled(frame); not reimplemented yet.
    // TODO(0x7cdef30, Quantum.BlastBallGameSystem.OnDisabled): OnDisabled(frame); not reimplemented yet.
    // TODO(0x7cdefa0, Quantum.BlastBallGameSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7cdf4fc, Quantum.BlastBallGameSystem.UpdateTeamStates): UpdateTeamStates(frame, gameData); not reimplemented yet.
    // TODO(0x7cdf580, Quantum.BlastBallGameSystem.ScoreGoal): ScoreGoal(frame, goalEntity, ballEntity); not reimplemented yet.
    // TODO(0x7cdf768, Quantum.BlastBallGameSystem.OnTriggerEnter3D): OnTriggerEnter3D(frame, info); not reimplemented yet.
    // TODO(0x7cdf90c, Quantum.BlastBallGameSystem.OnTriggerExit3D): OnTriggerExit3D(frame, info); not reimplemented yet.
    // TODO(0x7cdfa6c, Quantum.BlastBallGameSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(frame, gameMode); not reimplemented yet.
    // TODO(0x7cdff14, Quantum.BlastBallGameSystem.TryCreateBall): TryCreateBall(frame, ballEntity, ballTransform); not reimplemented yet.
    // TODO(0x7ce006c, Quantum.BlastBallGameSystem.BlastBallGame_StateChanged): BlastBallGame_StateChanged(frame, newState); not reimplemented yet.
    // TODO(0x7ce0178, Quantum.BlastBallGameSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BlastBallSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BlastBallSystem : public SystemMainThread {
  public:
    // 0x7b9f004
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7b9f00c, Quantum.BlastBallSystem.OnAdded): OnAdded(f, entity, ball); not reimplemented yet.
    // TODO(0x7b9f2ac, Quantum.BlastBallSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7b9feac, Quantum.BlastBallSystem.UpdateDrag): UpdateDrag(f, filter, ballSpec); not reimplemented yet.
    // TODO(0x7b9fdb0, Quantum.BlastBallSystem.HandleLockRotation): HandleLockRotation(f, filter, ballSpec); not reimplemented yet.
    // TODO(0x7b9f778, Quantum.BlastBallSystem.HandleDashingPlayers): HandleDashingPlayers(f, filter, ballSpec); not reimplemented yet.
    // TODO(0x7b9f4c0, Quantum.BlastBallSystem.RespawnBall): RespawnBall(f, filter, killable); not reimplemented yet.
    // TODO(0x7b9ff60, Quantum.BlastBallSystem.DamageToImpulseConverter_OnAfterImpulse): DamageToImpulseConverter_OnAfterImpulse(f, entity); not reimplemented yet.
    // TODO(0x7b9f690, Quantum.BlastBallSystem.ClampMaxVelocity): ClampMaxVelocity(physicsBody, ballSpec); not reimplemented yet.
    // TODO(0x7b9fc88, Quantum.BlastBallSystem.SetPhysicsMaterialBasedOnVelocity): SetPhysicsMaterialBasedOnVelocity(f, filter, ballSpec); not reimplemented yet.
    // TODO(0x7ba0094, Quantum.BlastBallSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(f, character, hit); not reimplemented yet.
    // TODO(0x7ba0128, Quantum.BlastBallSystem.HandleCollisionWithEntity): HandleCollisionWithEntity(f, ballEntity, otherEntity, impactPosition); not reimplemented yet.
    // TODO(0x7b9fd10, Quantum.BlastBallSystem.SaveBallPosition): SaveBallPosition(f, position); not reimplemented yet.
    // TODO(0x7ba03dc, Quantum.BlastBallSystem.DamageToImpulseConverter_ComputeImpulse): DamageToImpulseConverter_ComputeImpulse(frame, entity, hitData, impulseData); not reimplemented yet.
    // TODO(0x7ba0d00, Quantum.BlastBallSystem.DashActor_OnBeforeDash): DashActor_OnBeforeDash(f, entity, dashDir); not reimplemented yet.
    // TODO(0x7ba0f68, Quantum.BlastBallSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7ba126c, Quantum.BlastBallSystem.OnCollisionEnter3D): OnCollisionEnter3D(f, info); not reimplemented yet.
    // TODO(0x7ba0980, Quantum.BlastBallSystem.RedirectBlastBallTowardsGoal): RedirectBlastBallTowardsGoal(ballSpec, originalImpulse, ballPos, damagerPos, ownGoalPos, opposingGoalPos, desiredImpulseDirection); not reimplemented yet.
    // TODO(0x7ba1480, Quantum.BlastBallSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BlasterMoveSpeedEnhancementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BlasterMoveSpeedEnhancementSystem : public SystemMainThread {
  public:
    // TODO(0x7befc24, Quantum.BlasterMoveSpeedEnhancementSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7befcd0, Quantum.BlasterMoveSpeedEnhancementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7beffdc, Quantum.BlasterMoveSpeedEnhancementSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bf016c, Quantum.BlasterMoveSpeedEnhancementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BlasterSpecialShotEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class BlasterSpecialShotEnhancementSystem : public SystemSignalsOnly {
  public:
    // 0x7bf0308
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bf0310, Quantum.BlasterSpecialShotEnhancementSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bf052c, Quantum.BlasterSpecialShotEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.BoltCounterElementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BoltCounterElementSystem : public SystemMainThread {
  public:
    // TODO(0x7bd5fb4, Quantum.BoltCounterElementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd6284, Quantum.BoltCounterElementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BoltPowerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BoltPowerSystem : public SystemMainThread {
  public:
    // TODO(0x7bb68d8, Quantum.BoltPowerSystem.OnAdded): OnAdded(frame, entity, boltPowerActor); not reimplemented yet.
    // TODO(0x7bb6980, Quantum.BoltPowerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb6a4c, Quantum.BoltPowerSystem.UpdatePowerLevel): UpdatePowerLevel(frame, filter, boltPowerActorSpec); not reimplemented yet.
    // TODO(0x7bb6ae4, Quantum.BoltPowerSystem.UpdatePowerStats): UpdatePowerStats(frame, filter, boltPowerActorSpec); not reimplemented yet.
    // TODO(0x7bb6ef8, Quantum.BoltPowerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BoltRankingSystem (original base: Quantum.RankingSystem)
class BoltRankingSystem : public RankingSystem {
  public:
    // TODO(0x7ba1d10, Quantum.BoltRankingSystem.FillRankingList): FillRankingList(frame, list); not reimplemented yet.
    // TODO(0x7ba1eec, Quantum.BoltRankingSystem.Bolt_OnCollected): Bolt_OnCollected(frame, collector, amount); not reimplemented yet.
    // TODO(0x7ba20c8, Quantum.BoltRankingSystem.Bolt_OnLost): Bolt_OnLost(frame, collector, amount); not reimplemented yet.
    // TODO(0x7ba20d0, Quantum.BoltRankingSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba1f8c, Quantum.BoltRankingSystem.CloseToWinCheck): CloseToWinCheck(f, rankingManager); not reimplemented yet.
    // TODO(0x7ba21e0, Quantum.BoltRankingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BoltSignalSystem (original base: Quantum.SystemSignalsOnly)
class BoltSignalSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bb72ec, Quantum.BoltSignalSystem.Reward_OnCollected): Reward_OnCollected(frame, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bb7310, Quantum.BoltSignalSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bb7490, Quantum.BoltSignalSystem.Killable_OnKill): Killable_OnKill(frame, killedEntity, killedBy); not reimplemented yet.
    // TODO(0x7bb770c, Quantum.BoltSignalSystem.GameMode_OnPlayerEliminated): GameMode_OnPlayerEliminated(frame, playerEntity); not reimplemented yet.
    // TODO(0x7bb788c, Quantum.BoltSignalSystem.Killable_OnRevive): Killable_OnRevive(frame, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7bb755c, Quantum.BoltSignalSystem.LoseBoltsFixedAmount): LoseBoltsFixedAmount(frame, playerEntity, amount); not reimplemented yet.
    // TODO(0x7bb761c, Quantum.BoltSignalSystem.LoseBoltsPercentage): LoseBoltsPercentage(frame, entity, percentage); not reimplemented yet.
    // TODO(0x7bb7918, Quantum.BoltSignalSystem.LoseBolts): LoseBolts(frame, entity, boltGatherer, amount); not reimplemented yet.
    // TODO(0x7bb79e8, Quantum.BoltSignalSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.BombardierDroneSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BombardierDroneSystem : public SystemMainThread {
  public:
    // TODO(0x7bf0534, Quantum.BombardierDroneSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bf05e0, Quantum.BombardierDroneSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf0888, Quantum.BombardierDroneSystem.Init): Init(f, bombardierDrone, transform); not reimplemented yet.
    // TODO(0x7bf0974, Quantum.BombardierDroneSystem.UpdateSteering): UpdateSteering(f, entity, bombardierDrone, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf129c, Quantum.BombardierDroneSystem.TryGetBestTargetPosition): TryGetBestTargetPosition(f, transform, targetFinder, remainingRange, targetPosition); not reimplemented yet.
    // TODO(0x7bf153c, Quantum.BombardierDroneSystem.GetPotentialTargetScore): GetPotentialTargetScore(transform, potentialTarget, remainingRange); not reimplemented yet.
    // TODO(0x7bf0cac, Quantum.BombardierDroneSystem.UpdateTakingOff): UpdateTakingOff(f, bombardierDrone, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf0de8, Quantum.BombardierDroneSystem.UpdateFlying): UpdateFlying(f, entity, bombardierDrone, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf16a4, Quantum.BombardierDroneSystem.GetBombDropCount): GetBombDropCount(bombardierDroneSpec, flightLifetime); not reimplemented yet.
    // TODO(0x7bf1764, Quantum.BombardierDroneSystem.DropBomb): DropBomb(f, entity, bombardierDrone, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf1c44, Quantum.BombardierDroneSystem.GetBombDropSide): GetBombDropSide(f, bombardierDrone, bombardierDroneSpec); not reimplemented yet.
    // TODO(0x7bf0ed0, Quantum.BombardierDroneSystem.UpdateBeingDestroyed): UpdateBeingDestroyed(f, entity, bombardierDrone, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf1068, Quantum.BombardierDroneSystem.CheckCollision): CheckCollision(f, entity, previousPosition, position, bombardierDroneSpec, transform); not reimplemented yet.
    // TODO(0x7bf1c78, Quantum.BombardierDroneSystem.SpawnExplosionAoE): SpawnExplosionAoE(f, prototype, source, impact, assignToSourceTeam); not reimplemented yet.
    // TODO(0x7bf1050, Quantum.BombardierDroneSystem.DestroySelf): DestroySelf(f, entity); not reimplemented yet.
    // TODO(0x7bf1edc, Quantum.BombardierDroneSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BombardierWeaponAutoTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BombardierWeaponAutoTargetSystem : public SystemMainThread {
  public:
    // TODO(0x7bf1f24, Quantum.BombardierWeaponAutoTargetSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7bf1fac, Quantum.BombardierWeaponAutoTargetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf2780, Quantum.BombardierWeaponAutoTargetSystem.InverseTransformPoint): InverseTransformPoint(point, rotation, position); not reimplemented yet.
    // TODO(0x7bf26f0, Quantum.BombardierWeaponAutoTargetSystem.TransformPoint): TransformPoint(point, rotation, position); not reimplemented yet.
    // TODO(0x7bf2830, Quantum.BombardierWeaponAutoTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.BotSDKSystem (original base: Quantum.SystemSignalsOnly)
class BotSDKSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7b801b0, Quantum.BotSDKSystem.GetBlackboardComponent): GetBlackboardComponent(frame, entity, aiContext); not reimplemented yet.
    // TODO(0x7b80238, Quantum.BotSDKSystem.GetAIConfig): GetAIConfig(frame, entity, aiContext); not reimplemented yet.
};

// Quantum.BuzzbladesRicochetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class BuzzbladesRicochetSystem : public SystemMainThread {
  public:
    // TODO(0x7bf2878, Quantum.BuzzbladesRicochetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf2910, Quantum.BuzzbladesRicochetSystem.FindTargets): FindTargets(f, filter); not reimplemented yet.
    // TODO(0x7bf3678, Quantum.BuzzbladesRicochetSystem.GetTargetablePriority): GetTargetablePriority(entityType); not reimplemented yet.
    // TODO(0x7bf312c, Quantum.BuzzbladesRicochetSystem.ChangeTarget): ChangeTarget(f, targetedEffectEntity, targetedEffect, buzzbladesRicochet); not reimplemented yet.
    // TODO(0x7bf3704, Quantum.BuzzbladesRicochetSystem.GetRandomTopPriorityTarget): GetRandomTopPriorityTarget(f, potentialTargets); not reimplemented yet.
    // TODO(0x7bf3464, Quantum.BuzzbladesRicochetSystem.UpdatePosition): UpdatePosition(f, transform, targetedEffect); not reimplemented yet.
    // TODO(0x7bf3a3c, Quantum.BuzzbladesRicochetSystem.TargetedEffect_OnTick): TargetedEffect_OnTick(f, targetedEffectEntity, targetEntity); not reimplemented yet.
    // TODO(0x7bf3cb0, Quantum.BuzzbladesRicochetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CharacterColliderScalerSystem (original base: Quantum.SystemSignalsOnly)
class CharacterColliderScalerSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bda568, Quantum.CharacterColliderScalerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bda580, Quantum.CharacterColliderScalerSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ChargeAttackSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ChargeAttackSystem : public SystemMainThread {
  public:
    // TODO(0x7cc7900, Quantum.ChargeAttackSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cc7c38, Quantum.ChargeAttackSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ColliderResizeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ColliderResizeSystem : public SystemMainThread {
  public:
    // TODO(0x7bcb070, Quantum.ColliderResizeSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bcb364, Quantum.ColliderResizeSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bcb37c, Quantum.ColliderResizeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CollisionForceSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class CollisionForceSystem : public SystemMainThread {
  public:
    // TODO(0x7bdba38, Quantum.CollisionForceSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7bdbac0, Quantum.CollisionForceSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bdc810, Quantum.CollisionForceSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(f, character, hit); not reimplemented yet.
    // TODO(0x7bdcbe8, Quantum.CollisionForceSystem.OnCollision3D): OnCollision3D(f, info); not reimplemented yet.
    // TODO(0x7bdc8a4, Quantum.CollisionForceSystem.HandleCollisionWithEntity): HandleCollisionWithEntity(f, entity, otherEntity, impactPosition); not reimplemented yet.
    // TODO(0x7bdcd7c, Quantum.CollisionForceSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7bdcebc, Quantum.CollisionForceSystem.Pushable_OnAddPush): Pushable_OnAddPush(f, pushable, pushData); not reimplemented yet.
    // TODO(0x7bdd06c, Quantum.CollisionForceSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CompoundHFSMSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class CompoundHFSMSystem : public SystemMainThread {
  public:
    // TODO(0x7cc9ebc, Quantum.CompoundHFSMSystem.OnAdded): OnAdded(frame, entity, compoundHFSM); not reimplemented yet.
    // TODO(0x7cca2ac, Quantum.CompoundHFSMSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cca108, Quantum.CompoundHFSMSystem.InitRuntimeHFSM): InitRuntimeHFSM(frame, entity, setupData, runtimeHFSM); not reimplemented yet.
    // TODO(0x7cca39c, Quantum.CompoundHFSMSystem.UpdateRuntimeHFSM): UpdateRuntimeHFSM(f, filter, deltaTime, runtimeHFSM); not reimplemented yet.
    // TODO(0x7cc7448, Quantum.CompoundHFSMSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CoopBoltRankingSystem (original base: Quantum.RankingSystem)
class CoopBoltRankingSystem : public RankingSystem {
  public:
    // TODO(0x7ba21e8, Quantum.CoopBoltRankingSystem.FillRankingList): FillRankingList(frame, list); not reimplemented yet.
    // TODO(0x7ba23d0, Quantum.CoopBoltRankingSystem.Bolt_OnCollected): Bolt_OnCollected(frame, collector, amount); not reimplemented yet.
    // TODO(0x7ba23d8, Quantum.CoopBoltRankingSystem.Bolt_OnLost): Bolt_OnLost(frame, collector, amount); not reimplemented yet.
    // TODO(0x7ba23e0, Quantum.CoopBoltRankingSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba23e8, Quantum.CoopBoltRankingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CryosliderSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class CryosliderSystem : public SystemMainThread {
  public:
    // TODO(0x7bf5278, Quantum.CryosliderSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf5714, Quantum.CryosliderSystem.TryGetBestTargetPosition): TryGetBestTargetPosition(f, filter, remainingDistance, targetPosition); not reimplemented yet.
    // TODO(0x7bf5a4c, Quantum.CryosliderSystem.GetPotentialTargetScore): GetPotentialTargetScore(transform, potentialTarget, remainingDistance); not reimplemented yet.
    // TODO(0x7bf5bb4, Quantum.CryosliderSystem.OnCollision3D): OnCollision3D(f, info); not reimplemented yet.
    // TODO(0x7bf5d88, Quantum.CryosliderSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.CryosliderWeaponAutoTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class CryosliderWeaponAutoTargetSystem : public SystemMainThread {
  public:
    // TODO(0x7bf5dd0, Quantum.CryosliderWeaponAutoTargetSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7bf5e58, Quantum.CryosliderWeaponAutoTargetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf6580, Quantum.CryosliderWeaponAutoTargetSystem.InverseTransformPoint): InverseTransformPoint(point, rotation, position); not reimplemented yet.
    // TODO(0x7bf64f0, Quantum.CryosliderWeaponAutoTargetSystem.TransformPoint): TransformPoint(point, rotation, position); not reimplemented yet.
    // TODO(0x7bf6630, Quantum.CryosliderWeaponAutoTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DamageAndPushOverTimeSystem (original base: Quantum.SystemSignalsOnly)
class DamageAndPushOverTimeSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bd07b4, Quantum.DamageAndPushOverTimeSystem.TargetedEffect_OnTick): TargetedEffect_OnTick(f, targetedEffectEntity, targetEntity); not reimplemented yet.
    // TODO(0x7bd0970, Quantum.DamageAndPushOverTimeSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.DamageChargeableSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DamageChargeableSystem : public SystemMainThread {
  public:
    // TODO(0x7bd62cc, Quantum.DamageChargeableSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd64e0, Quantum.DamageChargeableSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bd66ac, Quantum.DamageChargeableSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DamageIncreasedOnTargetsEnhancementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DamageIncreasedOnTargetsEnhancementSystem : public SystemMainThread {
  public:
    // TODO(0x7becfb8, Quantum.DamageIncreasedOnTargetsEnhancementSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7bed128, Quantum.DamageIncreasedOnTargetsEnhancementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bed494, Quantum.DamageIncreasedOnTargetsEnhancementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DamageToImpulseConverterSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DamageToImpulseConverterSystem : public SystemMainThread {
  public:
    // 0x7bbb644
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bbb64c, Quantum.DamageToImpulseConverterSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bbb6f8, Quantum.DamageToImpulseConverterSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bbb848, Quantum.DamageToImpulseConverterSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bbc32c, Quantum.DamageToImpulseConverterSystem.TryGetWeapon): TryGetWeapon(f, hitData, weapon); not reimplemented yet.
    // TODO(0x7bbbe84, Quantum.DamageToImpulseConverterSystem.TryGetWeaponData): TryGetWeaponData(f, hitData, converterSpec, weaponMultiplier); not reimplemented yet.
    // TODO(0x7bbc128, Quantum.DamageToImpulseConverterSystem.TryGetAoESourceData): TryGetAoESourceData(f, hitData, converterSpec, aoeMultiplier); not reimplemented yet.
    // TODO(0x7bbc454, Quantum.DamageToImpulseConverterSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DamageableSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DamageableSystem : public SystemMainThread {
  public:
    // TODO(0x7bb820c, Quantum.DamageableSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bb895c, Quantum.DamageableSystem.ResetElectricalDamageGuage): ResetElectricalDamageGuage(frame, damageable, elementalDamageTable); not reimplemented yet.
    // TODO(0x7bb89d0, Quantum.DamageableSystem.ResetIceBuildUp): ResetIceBuildUp(frame, damageable, elementalDamageTable); not reimplemented yet.
    // TODO(0x7bb831c, Quantum.DamageableSystem.IsDead): IsDead(frame, entity); not reimplemented yet.
    // TODO(0x7bb8398, Quantum.DamageableSystem.ResolveHits): ResolveHits(frame, hittenEntity, damageable, damageableSpec, elementalDamageTable); not reimplemented yet.
    // TODO(0x7bb8a5c, Quantum.DamageableSystem.GetElementalDamageMultiplier): GetElementalDamageMultiplier(frame, elementalDamageTable, hit, damageableSpec); not reimplemented yet.
    // TODO(0x7bb8ad8, Quantum.DamageableSystem.ResolveShieldDamages): ResolveShieldDamages(frame, entity, hitData); not reimplemented yet.
    // TODO(0x7bb8c04, Quantum.DamageableSystem.ResolveHealthDamages): ResolveHealthDamages(frame, entity, hitData); not reimplemented yet.
    // TODO(0x7bb910c, Quantum.DamageableSystem.Killable_OnRevive): Killable_OnRevive(frame, entity, position, rotation); not reimplemented yet.
    // TODO(0x7bb9238, Quantum.DamageableSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(frame, gameMode); not reimplemented yet.
    // TODO(0x7bb9418, Quantum.DamageableSystem.Shield_OnBreaks): Shield_OnBreaks(frame, entity, sourceEntity, direction); not reimplemented yet.
    // TODO(0x7bb9544, Quantum.DamageableSystem.OnAdded): OnAdded(frame, entity, damageable); not reimplemented yet.
    // TODO(0x7bb9614, Quantum.DamageableSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7bb9b10, Quantum.DamageableSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DashSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DashSystem : public SystemMainThread {
  public:
    // TODO(0x7cb42cc, Quantum.DashSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb44b8, Quantum.DashSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb4844, Quantum.DashSystem.Dash): Dash(f, entity, dir, dashActor, kcc, kccExt, config); not reimplemented yet.
    // TODO(0x7cb4b20, Quantum.DashSystem.UpdateDashCooldown): UpdateDashCooldown(f, entity, pDash); not reimplemented yet.
    // TODO(0x7cb4bb4, Quantum.DashSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(frame, character, hit); not reimplemented yet.
    // TODO(0x7cb4f94, Quantum.DashSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, e, hitData); not reimplemented yet.
    // TODO(0x7cb5070, Quantum.DashSystem.AutoJumperActor_OnStartJump): AutoJumperActor_OnStartJump(f, entity); not reimplemented yet.
    // TODO(0x7cb5118, Quantum.DashSystem.AutoJumperActor_OnEndJump): AutoJumperActor_OnEndJump(f, entity); not reimplemented yet.
    // TODO(0x7cb51c0, Quantum.DashSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DisableEnergyCollectionOnAbilityUseSystem (original base: Quantum.SystemSignalsOnly)
class DisableEnergyCollectionOnAbilityUseSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be3a48, Quantum.DisableEnergyCollectionOnAbilityUseSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7be3b74, Quantum.DisableEnergyCollectionOnAbilityUseSystem.DisableEnergyCollection): DisableEnergyCollection(disabledDroppables); not reimplemented yet.
    // TODO(0x7be3c3c, Quantum.DisableEnergyCollectionOnAbilityUseSystem.EnableEnergyCollection): EnableEnergyCollection(disabledDroppables); not reimplemented yet.
    // TODO(0x7be3d04, Quantum.DisableEnergyCollectionOnAbilityUseSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.DrillDashSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DrillDashSystem : public SystemMainThread {
  public:
    // TODO(0x7cdb7f0, Quantum.DrillDashSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7cdb89c, Quantum.DrillDashSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cdbfa4, Quantum.DrillDashSystem.DrillDash_OnStateChanged): DrillDash_OnStateChanged(f, entity, pDrillDash, newState); not reimplemented yet.
    // TODO(0x7cdca1c, Quantum.DrillDashSystem.AutoJumperActor_OnStartJump): AutoJumperActor_OnStartJump(f, entity); not reimplemented yet.
    // TODO(0x7cdcba0, Quantum.DrillDashSystem.WallRunningActor_OnWallRunningStart): WallRunningActor_OnWallRunningStart(f, entity); not reimplemented yet.
    // TODO(0x7cdcba4, Quantum.DrillDashSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7cdcba8, Quantum.DrillDashSystem.ActorStatus_OnChanged): ActorStatus_OnChanged(f, actorEntity); not reimplemented yet.
    // TODO(0x7cdc144, Quantum.DrillDashSystem.OnAnticipationStateEnter): OnAnticipationStateEnter(f, entity, spec); not reimplemented yet.
    // TODO(0x7cdc2e8, Quantum.DrillDashSystem.OnDashStateEnter): OnDashStateEnter(f, entity, pDrillDash, spec); not reimplemented yet.
    // TODO(0x7cdcf78, Quantum.DrillDashSystem.TryGetToTargetDirection): TryGetToTargetDirection(f, entity, direction); not reimplemented yet.
    // TODO(0x7cdd124, Quantum.DrillDashSystem.SpawnAttackAoe): SpawnAttackAoe(f, entity, pDrillDash, spec); not reimplemented yet.
    // TODO(0x7cdc5a8, Quantum.DrillDashSystem.OnAttackStateEnter): OnAttackStateEnter(f, entity, pDrillDash, spec); not reimplemented yet.
    // TODO(0x7cdc5f8, Quantum.DrillDashSystem.OnLastHitStateEnter): OnLastHitStateEnter(f, entity, pDrillDash, spec); not reimplemented yet.
    // TODO(0x7cdc84c, Quantum.DrillDashSystem.OnRecoveryStateEnter): OnRecoveryStateEnter(f, entity, spec); not reimplemented yet.
    // TODO(0x7cdbb68, Quantum.DrillDashSystem.UpdateAnticipationState): UpdateAnticipationState(f, entity, pDrillDash, drillDashGadgetSpec); not reimplemented yet.
    // TODO(0x7cdd2c0, Quantum.DrillDashSystem.ForceCharacterOrientation): ForceCharacterOrientation(f, entity); not reimplemented yet.
    // TODO(0x7cdbc10, Quantum.DrillDashSystem.UpdateDashState): UpdateDashState(f, entity, pDrillDash, drillDashGadgetSpec); not reimplemented yet.
    // TODO(0x7cdd424, Quantum.DrillDashSystem.ProcessDetectionOverlap): ProcessDetectionOverlap(f, entity, pDrillDash, drillDashGadgetSpec); not reimplemented yet.
    // TODO(0x7cdd904, Quantum.DrillDashSystem.CalculateOffsetPosition): CalculateOffsetPosition(origin, direction, offset); not reimplemented yet.
    // TODO(0x7cdbcd4, Quantum.DrillDashSystem.UpdateAttackState): UpdateAttackState(f, entity, pDrillDash, drillDashGadgetSpec); not reimplemented yet.
    // TODO(0x7cdbd74, Quantum.DrillDashSystem.UpdateLastHitState): UpdateLastHitState(f, entity, pDrillDash, drillDashGadgetSpec); not reimplemented yet.
    // TODO(0x7cdbe34, Quantum.DrillDashSystem.TestFix): TestFix(f, entity); not reimplemented yet.
    // TODO(0x7cdbe14, Quantum.DrillDashSystem.UpdateRecoveryState): UpdateRecoveryState(f, entity, pDrillDash, drillDashGadgetSpec, pUser); not reimplemented yet.
    // TODO(0x7cdcc54, Quantum.DrillDashSystem.FreezeEntityKcc): FreezeEntityKcc(f, entity); not reimplemented yet.
    // TODO(0x7cdce00, Quantum.DrillDashSystem.ExitGrind): ExitGrind(f, entity); not reimplemented yet.
    // TODO(0x7cdca20, Quantum.DrillDashSystem.InterruptIfNeeded): InterruptIfNeeded(f, entity); not reimplemented yet.
    // TODO(0x7cdda34, Quantum.DrillDashSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.DroppableManagerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class DroppableManagerSystem : public SystemMainThread {
  public:
    // TODO(0x7bbe674, Quantum.DroppableManagerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bbe694, Quantum.DroppableManagerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bbe6b4, Quantum.DroppableManagerSystem.OnInit): OnInit(frame); not reimplemented yet.
    // TODO(0x7bbe8b0, Quantum.DroppableManagerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bbea6c, Quantum.DroppableManagerSystem.UpdateDroppablesList): UpdateDroppablesList(frame, spec, shrinkingZone, dropAnims, collectors, droppables); not reimplemented yet.
    // TODO(0x7bbeba0, Quantum.DroppableManagerSystem.UpdateDroppable): UpdateDroppable(frame, droppable, index, anim, spec, shrinkingZone, collectors); not reimplemented yet.
    // TODO(0x7bbf024, Quantum.DroppableManagerSystem.CheckLifetime): CheckLifetime(frame, droppable); not reimplemented yet.
    // TODO(0x7bbef44, Quantum.DroppableManagerSystem.CheckShrinkingZone): CheckShrinkingZone(frame, droppable, spec, shrinkingZone); not reimplemented yet.
    // TODO(0x7bbed40, Quantum.DroppableManagerSystem.UpdateDropAnimation): UpdateDropAnimation(frame, droppable, anim, spec); not reimplemented yet.
    // TODO(0x7bbf058, Quantum.DroppableManagerSystem.CheckIfCollected): CheckIfCollected(frame, droppable, index, spec, collectors); not reimplemented yet.
    // TODO(0x7bbf458, Quantum.DroppableManagerSystem.TryGetCollectorEntity): TryGetCollectorEntity(frame, droppable, index, spec, collectors); not reimplemented yet.
    // TODO(0x7bbf38c, Quantum.DroppableManagerSystem.HandleDestruction): HandleDestruction(frame, droppable); not reimplemented yet.
    // TODO(0x7bbfb80, Quantum.DroppableManagerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ElectricDamageSystem (original base: Quantum.SystemSignalsOnly)
class ElectricDamageSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bbc49c, Quantum.ElectricDamageSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bbc548, Quantum.ElectricDamageSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bbc900, Quantum.ElectricDamageSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.EliminationGameModeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class EliminationGameModeSystem : public SystemMainThread {
  public:
    // 0x7ba2a50
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7ba2a58, Quantum.EliminationGameModeSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7ba2a5c, Quantum.EliminationGameModeSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba2bec, Quantum.EliminationGameModeSystem.CloseToWinCheck): CloseToWinCheck(f, pGameMode, gameModeSpec, deadPlayerListCount); not reimplemented yet.
    // TODO(0x7ba2d98, Quantum.EliminationGameModeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.EnergyCellSignalSystem (original base: Quantum.SystemSignalsOnly)
class EnergyCellSignalSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bbd61c, Quantum.EnergyCellSignalSystem.Reward_OnCollected): Reward_OnCollected(frame, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bbd644, Quantum.EnergyCellSignalSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.EnforcerEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class EnforcerEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bf6ab0, Quantum.EnforcerEnhancementSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bf6b5c, Quantum.EnforcerEnhancementSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bf6d4c, Quantum.EnforcerEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.EntityDropperSpawnSystem (original base: Quantum.SystemSignalsOnly)
class EntityDropperSpawnSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bbfbc8, Quantum.EntityDropperSpawnSystem.OnAdded): OnAdded(frame, entity, entityDropper); not reimplemented yet.
    // TODO(0x7bbffb4, Quantum.EntityDropperSpawnSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bc0420, Quantum.EntityDropperSpawnSystem.TryDropEntities): TryDropEntities(frame, dropperEntity, dropperSpec, transform, dropCount, position); not reimplemented yet.
    // TODO(0x7bc0790, Quantum.EntityDropperSpawnSystem.SpawnEntities): SpawnEntities(frame, origin, targetOffset, dropCount, entityDropper, spec); not reimplemented yet.
    // TODO(0x7bc08c4, Quantum.EntityDropperSpawnSystem.Health_OnHealthDecreased): Health_OnHealthDecreased(frame, entity, previousHP, currentHP, maxHP); not reimplemented yet.
    // TODO(0x7bc0668, Quantum.EntityDropperSpawnSystem.GetMultipliedDropCount): GetMultipliedDropCount(frame, dropCount, spec); not reimplemented yet.
    // TODO(0x7bc0cd0, Quantum.EntityDropperSpawnSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.EntityGroupSystem (original base: Quantum.SystemSignalsOnly)
class EntityGroupSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bc0cd8, Quantum.EntityGroupSystem.OnAdded): OnAdded(f, entity, entityGroup); not reimplemented yet.
    // TODO(0x7bc0dfc, Quantum.EntityGroupSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.EntityInfoSystem (original base: Quantum.SystemSignalsOnly)
class EntityInfoSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cdaf68, Quantum.EntityInfoSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cdb008, Quantum.EntityInfoSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.FireDamageSystem (original base: Quantum.SystemSignalsOnly)
class FireDamageSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bbc9e8, Quantum.FireDamageSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bbd154, Quantum.FireDamageSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.FireReactorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class FireReactorSystem : public SystemMainThread {
  public:
    // TODO(0x7bd66f4, Quantum.FireReactorSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd68ec, Quantum.FireReactorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.FreeForAllRankingSystem (original base: Quantum.RankingSystem)
class FreeForAllRankingSystem : public RankingSystem {
  public:
    // TODO(0x7ba3148, Quantum.FreeForAllRankingSystem.FillRankingList): FillRankingList(frame, list); not reimplemented yet.
    // TODO(0x7ba3308, Quantum.FreeForAllRankingSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba33e4, Quantum.FreeForAllRankingSystem.CloseToWinCheck): CloseToWinCheck(f, rankingManager); not reimplemented yet.
    // TODO(0x7ba3514, Quantum.FreeForAllRankingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GadgetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class GadgetSystem : public SystemMainThread {
  public:
    // TODO(0x7c01038, Quantum.GadgetSystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, playerActor); not reimplemented yet.
    // TODO(0x7c012f0, Quantum.GadgetSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c013b0, Quantum.GadgetSystem.Weapon_OnReload): Weapon_OnReload(f, weaponEntity, weapon); not reimplemented yet.
    // TODO(0x7c0154c, Quantum.GadgetSystem.Pushable_OnBeforePush): Pushable_OnBeforePush(frame, targetEntity, pushParams); not reimplemented yet.
    // TODO(0x7c016a8, Quantum.GadgetSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7c017f4, Quantum.GadgetSystem.ActorStatus_OnChanged): ActorStatus_OnChanged(f, actorEntity); not reimplemented yet.
    // TODO(0x7c01970, Quantum.GadgetSystem.BlastBallGame_StateChanged): BlastBallGame_StateChanged(frame, newState); not reimplemented yet.
    // TODO(0x7c01b50, Quantum.GadgetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GameManagerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class GameManagerSystem : public SystemMainThread {
  public:
    // TODO(0x7bc0e04, Quantum.GameManagerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc0e50, Quantum.GameManagerSystem.OnPlayerDataSet): OnPlayerDataSet(f, player); not reimplemented yet.
    // TODO(0x7bc0fe0, Quantum.GameManagerSystem.OnPlayerDisconnected): OnPlayerDisconnected(frame, player); not reimplemented yet.
    // TODO(0x7bc113c, Quantum.GameManagerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bc11dc, Quantum.GameManagerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GameModeActivatorSystem (original base: Quantum.SystemSignalsOnly)
class GameModeActivatorSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7ba351c, Quantum.GameModeActivatorSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7ba359c, Quantum.GameModeActivatorSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.GameModeBoltCountSystem (original base: Quantum.SystemSignalsOnly)
class GameModeBoltCountSystem : public SystemSignalsOnly {
  public:
    // 0x7ba35a4
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7ba35ac, Quantum.GameModeBoltCountSystem.Bolt_OnCollected): Bolt_OnCollected(f, collector, amount); not reimplemented yet.
    // TODO(0x7ba377c, Quantum.GameModeBoltCountSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.GameModeHandlerSystem (original base: Quantum.SystemMainThread)
class GameModeHandlerSystem : public SystemMainThread {
  public:
    // TODO(0x7ba4258, Quantum.GameModeHandlerSystem.OnAdded): OnAdded(f, entity, pHandler); not reimplemented yet.
    // TODO(0x7ba4430, Quantum.GameModeHandlerSystem.Update): Update(f); not reimplemented yet.
    // TODO(0x7ba4ed0, Quantum.GameModeHandlerSystem.SetGameModeStepMainLoop): SetGameModeStepMainLoop(f, gameModeEntity, pHandler); not reimplemented yet.
    // TODO(0x7ba4c70, Quantum.GameModeHandlerSystem.PollPlayerCommands): PollPlayerCommands(f, pHandler); not reimplemented yet.
    // TODO(0x7ba53ec, Quantum.GameModeHandlerSystem.Killable_OnKill): Killable_OnKill(frame, killedEntity, killedBy); not reimplemented yet.
    // TODO(0x7ba4f3c, Quantum.GameModeHandlerSystem.UpdateDynamicSpawnPointData): UpdateDynamicSpawnPointData(f, pHandler, gameModeSpec); not reimplemented yet.
    // TODO(0x7ba5564, Quantum.GameModeHandlerSystem.ComputePlayersSkinListAndStoreInGameMode): ComputePlayersSkinListAndStoreInGameMode(f, pGameMode, config, configLevel, gameModeSpec); not reimplemented yet.
    // TODO(0x7ba4db8, Quantum.GameModeHandlerSystem.UpdateStep_WaitConnections): UpdateStep_WaitConnections(f, pGameMode, config, configLevel, gameModeSpec); not reimplemented yet.
    // TODO(0x7ba4e68, Quantum.GameModeHandlerSystem.UpdateStep_WaitForPrefabsLoaded): UpdateStep_WaitForPrefabsLoaded(f, pGameMode, config, configLevel, gameModeSpec); not reimplemented yet.
    // TODO(0x7ba5d44, Quantum.GameModeHandlerSystem.IsReadyToStart): IsReadyToStart(f, pGameMode, nExpectedHumanPlayers); not reimplemented yet.
    // TODO(0x7ba4a6c, Quantum.GameModeHandlerSystem.PollPlayerReadyCommands): PollPlayerReadyCommands(f, pHandler); not reimplemented yet.
    // TODO(0x7ba597c, Quantum.GameModeHandlerSystem.GetOrderedHumanPlayersList): GetOrderedHumanPlayersList(f, pHandler, maxPossiblePlayers, nHumanSpawned); not reimplemented yet.
    // TODO(0x7ba5e3c, Quantum.GameModeHandlerSystem.SpawnBotsAndPlayerPrototypes): SpawnBotsAndPlayerPrototypes(f, pHandler, config, gameModeSpec); not reimplemented yet.
    // TODO(0x7ba6bec, Quantum.GameModeHandlerSystem.GetRandomizedPowerStatsForBot): GetRandomizedPowerStatsForBot(frame, basePowerStats); not reimplemented yet.
    // TODO(0x7ba693c, Quantum.GameModeHandlerSystem.GetPlayerIndex): GetPlayerIndex(teamRepartition, expectedTeamId, teamId); not reimplemented yet.
    // TODO(0x7ba6d58, Quantum.GameModeHandlerSystem.Quantum.ISignalOnMapChanged.OnMapChanged): Quantum.ISignalOnMapChanged.OnMapChanged(f, previousMap); not reimplemented yet.
    // TODO(0x7ba3a34, Quantum.GameModeHandlerSystem.IsStateSkipped): IsStateSkipped(f, stateId, pHandler, config); not reimplemented yet.
    // TODO(0x7ba6d5c, Quantum.GameModeHandlerSystem.GameMode_OnStateChanged): GameMode_OnStateChanged(frame, prevState, newState); not reimplemented yet.
    // TODO(0x7ba6eb4, Quantum.GameModeHandlerSystem.Ranking_OnLeaderChanged): Ranking_OnLeaderChanged(f); not reimplemented yet.
    // TODO(0x7ba6f60, Quantum.GameModeHandlerSystem.BlastBallGame_LeadChanged): BlastBallGame_LeadChanged(f); not reimplemented yet.
    // TODO(0x7ba6ebc, Quantum.GameModeHandlerSystem.UpdateLeadChangedStats): UpdateLeadChangedStats(f); not reimplemented yet.
    // TODO(0x7ba5d40, Quantum.GameModeHandlerSystem.BalancingLog): BalancingLog(text); not reimplemented yet.
    // TODO(0x7ba7030, Quantum.GameModeHandlerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GameModeKillCountSystem (original base: Quantum.SystemSignalsOnly)
class GameModeKillCountSystem : public SystemSignalsOnly {
  public:
    // 0x7ba7158
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7ba7160, Quantum.GameModeKillCountSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba73f0, Quantum.GameModeKillCountSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.GameModifiersSystem (original base: Quantum.SystemMainThreadGroup)
class GameModifiersSystem : public SystemMainThread {
  public:
    // TODO(0x7bc6eac, Quantum.GameModifiersSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7bc6fe0, Quantum.GameModifiersSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc7000, Quantum.GameModifiersSystem.OnInit): OnInit(frame); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GameTimerSystem (original base: Quantum.SystemMainThread)
class GameTimerSystem : public SystemMainThread {
  public:
    // TODO(0x7ba7d00, Quantum.GameTimerSystem.Update): Update(frame); not reimplemented yet.
    // TODO(0x7ba7df4, Quantum.GameTimerSystem.CheckGameTime): CheckGameTime(frame); not reimplemented yet.
    // TODO(0x7ba79b4, Quantum.GameTimerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GloveOfDoomTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class GloveOfDoomTargetSystem : public SystemMainThread {
  public:
    // 0x7bf7b28
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bf7b30, Quantum.GloveOfDoomTargetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf7e7c, Quantum.GloveOfDoomTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GloveOfDoomWeaponSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class GloveOfDoomWeaponSystem : public SystemMainThread {
  public:
    // 0x7bf7908
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bf7910, Quantum.GloveOfDoomWeaponSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf7a50, Quantum.GloveOfDoomWeaponSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GoldenBoltSystem (original base: Quantum.SystemMainThread)
class GoldenBoltSystem : public SystemMainThread {
  public:
    // TODO(0x7bc1224, Quantum.GoldenBoltSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bc12cc, Quantum.GoldenBoltSystem.OnMapChanged): OnMapChanged(f, previousMap); not reimplemented yet.
    // TODO(0x7bc141c, Quantum.GoldenBoltSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc143c, Quantum.GoldenBoltSystem.Update): Update(f); not reimplemented yet.
    // TODO(0x7bc15a8, Quantum.GoldenBoltSystem.CheckPendingGoldBoltSpawns): CheckPendingGoldBoltSpawns(f, goldenBoltManager, spawnedGoldenBolts, goldenBoltConfig); not reimplemented yet.
    // TODO(0x7bc16fc, Quantum.GoldenBoltSystem.CheckGoldenBoltSpawn): CheckGoldenBoltSpawn(f, pGameModeHandler, spawnedGoldenBolts, goldenBoltConfig); not reimplemented yet.
    // TODO(0x7bc1b3c, Quantum.GoldenBoltSystem.SpawnGoldBolt): SpawnGoldBolt(f, spawnedGoldenBolts, goldenBoltConfig, spawnPointIndex); not reimplemented yet.
    // TODO(0x7bc1d08, Quantum.GoldenBoltSystem.GetGoldenBoltSpawnPointIndex): GetGoldenBoltSpawnPointIndex(f, mapCustomData, spawnedGoldenBolts, safeTime); not reimplemented yet.
    // TODO(0x7bc1854, Quantum.GoldenBoltSystem.HandleGoldenBoltsMovement): HandleGoldenBoltsMovement(f, spawnedGoldenBolts, goldenBoltConfig); not reimplemented yet.
    // TODO(0x7bc2310, Quantum.GoldenBoltSystem.GoldenBoltIsValid): GoldenBoltIsValid(f, goldenBoltEntity); not reimplemented yet.
    // TODO(0x7bc25dc, Quantum.GoldenBoltSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.GrenadeLauncherEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class GrenadeLauncherEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bf80d4, Quantum.GrenadeLauncherEnhancementSystem.Weapon_OnShoot): Weapon_OnShoot(f, weaponEntity, ammunitionEntity); not reimplemented yet.
    // TODO(0x7bf84a8, Quantum.GrenadeLauncherEnhancementSystem.Shield_OnBreaks): Shield_OnBreaks(f, entity, sourceEntity, direction); not reimplemented yet.
    // TODO(0x7bf8664, Quantum.GrenadeLauncherEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.GroundDashSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class GroundDashSystem : public SystemMainThread {
  public:
    // TODO(0x7cb55e8, Quantum.GroundDashSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb57cc, Quantum.GroundDashSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7cb5bac, Quantum.GroundDashSystem.PerformGroundDash): PerformGroundDash(frame, userEntity, config, groundDashActor, kccExt); not reimplemented yet.
    // TODO(0x7cb5b10, Quantum.GroundDashSystem.StartGroundDash): StartGroundDash(frame, userEntity, config, groundDashActor, kccExt); not reimplemented yet.
    // TODO(0x7cb5958, Quantum.GroundDashSystem.StopGroundDash): StopGroundDash(frame, userEntity, config, groundDashActor, kccExt, endReason); not reimplemented yet.
    // TODO(0x7cb59ec, Quantum.GroundDashSystem.InstantiateAoE): InstantiateAoE(frame, entity, transform, config); not reimplemented yet.
    // TODO(0x7cb5ca0, Quantum.GroundDashSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(frame, character, hit); not reimplemented yet.
    // TODO(0x7cb5ff8, Quantum.GroundDashSystem.Killable_OnKill): Killable_OnKill(f, entity, killedBy); not reimplemented yet.
    // TODO(0x7cb6144, Quantum.GroundDashSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HFSMSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HFSMSystem : public SystemMainThread {
  public:
    // TODO(0x7cd23e8, Quantum.HFSMSystem.OnAdded): OnAdded(f, entity, agent); not reimplemented yet.
    // TODO(0x7cd24b0, Quantum.HFSMSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd2568, Quantum.HFSMSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HealthSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HealthSystem : public SystemMainThread {
  public:
    // TODO(0x7bc30ec, Quantum.HealthSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bc33ac, Quantum.HealthSystem.UpdateActiveOverhealths): UpdateActiveOverhealths(f, health); not reimplemented yet.
    // TODO(0x7bc322c, Quantum.HealthSystem.Regenerate): Regenerate(frame, healthEntity, health, spec); not reimplemented yet.
    // TODO(0x7bc363c, Quantum.HealthSystem.Shield_OnFull): Shield_OnFull(frame, entity); not reimplemented yet.
    // TODO(0x7bc373c, Quantum.HealthSystem.Killable_OnRevive): Killable_OnRevive(frame, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7bc37e0, Quantum.HealthSystem.Reward_OnCollected): Reward_OnCollected(frame, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bc392c, Quantum.HealthSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc39b8, Quantum.HealthSystem.BlastBallGame_StateChanged): BlastBallGame_StateChanged(f, newState); not reimplemented yet.
    // TODO(0x7bc3b68, Quantum.HealthSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HelipackActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HelipackActorSystem : public SystemMainThread {
  public:
    // TODO(0x7cb63d4, Quantum.HelipackActorSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb6554, Quantum.HelipackActorSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb6964, Quantum.HelipackActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HierarchySystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HierarchySystem : public SystemMainThread {
  public:
    // TODO(0x7be35b4, Quantum.HierarchySystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7be3648, Quantum.HierarchySystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be3658, Quantum.HierarchySystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HoloshieldGloveHealZoneEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class HoloshieldGloveHealZoneEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bf8764, Quantum.HoloshieldGloveHealZoneEnhancementSystem.Weapon_OnShoot): Weapon_OnShoot(f, weaponEntity, ammunitionEntity); not reimplemented yet.
    // TODO(0x7bf8904, Quantum.HoloshieldGloveHealZoneEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.HoloshieldGloveShieldSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HoloshieldGloveShieldSystem : public SystemMainThread {
  public:
    // TODO(0x7bf890c, Quantum.HoloshieldGloveShieldSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bf8c44, Quantum.HoloshieldGloveShieldSystem.Init): Init(f, holoshieldGloveShield, transform); not reimplemented yet.
    // TODO(0x7bf8d28, Quantum.HoloshieldGloveShieldSystem.SetShieldMembers): SetShieldMembers(f, shieldsGroup, shieldEntity, holoshieldGloveShield); not reimplemented yet.
    // TODO(0x7bf9074, Quantum.HoloshieldGloveShieldSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HoloshieldGloveTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HoloshieldGloveTargetSystem : public SystemMainThread {
  public:
    // TODO(0x7bf92f8, Quantum.HoloshieldGloveTargetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf9644, Quantum.HoloshieldGloveTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.HoloshieldGloveWeaponSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class HoloshieldGloveWeaponSystem : public SystemMainThread {
  public:
    // TODO(0x7bf9164, Quantum.HoloshieldGloveWeaponSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf92b0, Quantum.HoloshieldGloveWeaponSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.IceDamageSystem (original base: Quantum.SystemSignalsOnly)
class IceDamageSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bbd1a8, Quantum.IceDamageSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bbd254, Quantum.IceDamageSystem.Damageable_HitReceived): Damageable_HitReceived(f, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bbd60c, Quantum.IceDamageSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.InitializeRollSystem (original base: Quantum.SystemSignalsOnly)
class InitializeRollSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bdf2a0, Quantum.InitializeRollSystem.OnAdded): OnAdded(f, entity, roll); not reimplemented yet.
    // TODO(0x7bdf478, Quantum.InitializeRollSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ItemSpawnerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ItemSpawnerSystem : public SystemMainThread {
  public:
    // TODO(0x7bd75e8, Quantum.ItemSpawnerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bd769c, Quantum.ItemSpawnerSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7bd76e0, Quantum.ItemSpawnerSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd79f4, Quantum.ItemSpawnerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.JumpAttackSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class JumpAttackSystem : public SystemMainThread {
  public:
    // TODO(0x7cc7e80, Quantum.JumpAttackSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cc822c, Quantum.JumpAttackSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.KccOverlapInjectionSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class KccOverlapInjectionSystem : public SystemMainThread {
  public:
    // TODO(0x7bdd174, Quantum.KccOverlapInjectionSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bdd210, Quantum.KccOverlapInjectionSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.KccSurfaceResetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class KccSurfaceResetSystem : public SystemMainThread {
  public:
    // TODO(0x7bdd114, Quantum.KccSurfaceResetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bdd12c, Quantum.KccSurfaceResetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.KillableSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class KillableSystem : public SystemMainThread {
  public:
    // TODO(0x7bc4738, Quantum.KillableSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bc4880, Quantum.KillableSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc48c8, Quantum.KillableSystem.Reward_OnCollected): Reward_OnCollected(frame, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bc48ec, Quantum.KillableSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bc53ec, Quantum.KillableSystem.ParabolaFollower_OnReachedEnd): ParabolaFollower_OnReachedEnd(f, entity); not reimplemented yet.
    // TODO(0x7bc5494, Quantum.KillableSystem.Killable_OnRevive): Killable_OnRevive(f, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7bc552c, Quantum.KillableSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.LedgeGrabbingActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class LedgeGrabbingActorSystem : public SystemMainThread {
  public:
    // TODO(0x7cb69ac, Quantum.LedgeGrabbingActorSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb6ca8, Quantum.LedgeGrabbingActorSystem.KccExtension_OnValidLedgeDetected): KccExtension_OnValidLedgeDetected(f, character, hit, aboveLedgePoint); not reimplemented yet.
    // TODO(0x7cb6f74, Quantum.LedgeGrabbingActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.LevelButtonSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class LevelButtonSystem : public SystemMainThread {
  public:
    // TODO(0x7bd7edc, Quantum.LevelButtonSystem.OnAdded): OnAdded(f, entity, pLevelButton); not reimplemented yet.
    // TODO(0x7bd7ef0, Quantum.LevelButtonSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd8094, Quantum.LevelButtonSystem.Damageable_HitReceived): Damageable_HitReceived(frame, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bd8138, Quantum.LevelButtonSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.LifetimeInitSystem (original base: Quantum.SystemSignalsOnly)
class LifetimeInitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bc5f88, Quantum.LifetimeInitSystem.OnAdded): OnAdded(f, entity, lifetime); not reimplemented yet.
    // TODO(0x7bc612c, Quantum.LifetimeInitSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(f, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7bc63f8, Quantum.LifetimeInitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.LifetimeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class LifetimeSystem : public SystemMainThread {
  public:
    // TODO(0x7bc6400, Quantum.LifetimeSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7bc6488, Quantum.LifetimeSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bc6584, Quantum.LifetimeSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bc6b9c, Quantum.LifetimeSystem.OnAdded): OnAdded(frame, entity, lifetime); not reimplemented yet.
    // TODO(0x7bc6dd0, Quantum.LifetimeSystem.OnRemoved): OnRemoved(f, entity, lifetime); not reimplemented yet.
    // TODO(0x7bc6e54, Quantum.LifetimeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MeleeAttackSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MeleeAttackSystem : public SystemMainThread {
  public:
    // 0x7cc83f8
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7cc8400, Quantum.MeleeAttackSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7cc86b4, Quantum.MeleeAttackSystem.AreaOfEffect_OnHit): AreaOfEffect_OnHit(f, areaOfEffectEntity); not reimplemented yet.
    // TODO(0x7cc87f4, Quantum.MeleeAttackSystem.AreaOfEffect_OnMiss): AreaOfEffect_OnMiss(f, areaOfEffectEntity); not reimplemented yet.
    // TODO(0x7cc86bc, Quantum.MeleeAttackSystem.HandleAreaOfEffectHitOrMiss): HandleAreaOfEffectHitOrMiss(f, areaOfEffectEntity, hit); not reimplemented yet.
    // TODO(0x7cc87fc, Quantum.MeleeAttackSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MeleeJumpAttackSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MeleeJumpAttackSystem : public SystemMainThread {
  public:
    // TODO(0x7bd452c, Quantum.MeleeJumpAttackSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bd460c, Quantum.MeleeJumpAttackSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd4c84, Quantum.MeleeJumpAttackSystem.SetState): SetState(frame, meleeJumpAttack, newState); not reimplemented yet.
    // TODO(0x7bd481c, Quantum.MeleeJumpAttackSystem.UpdateState_Waiting): UpdateState_Waiting(frame, filter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd4910, Quantum.MeleeJumpAttackSystem.UpdateState_Charging): UpdateState_Charging(frame, filter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd4a2c, Quantum.MeleeJumpAttackSystem.UpdateState_Performing): UpdateState_Performing(frame, filter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd4c00, Quantum.MeleeJumpAttackSystem.UpdateState_Cooldown): UpdateState_Cooldown(frame, filter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd4e0c, Quantum.MeleeJumpAttackSystem.StartCharging): StartCharging(frame, filter); not reimplemented yet.
    // TODO(0x7bd4e60, Quantum.MeleeJumpAttackSystem.CancelCharge): CancelCharge(f, filter); not reimplemented yet.
    // TODO(0x7bd4ee4, Quantum.MeleeJumpAttackSystem.StartJumpAttack): StartJumpAttack(frame, filter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd53dc, Quantum.MeleeJumpAttackSystem.StopJumpAttack): StopJumpAttack(frame, filter, weaponUserFilter); not reimplemented yet.
    // TODO(0x7bd4fb4, Quantum.MeleeJumpAttackSystem.CompleteJumpAttack): CompleteJumpAttack(frame, filter, weaponUserFilter, meleeJumpAttackSpec); not reimplemented yet.
    // TODO(0x7bd5178, Quantum.MeleeJumpAttackSystem.PerformJumpAttack): PerformJumpAttack(frame, weaponUserEntity, filter, weaponUserFilter, spec); not reimplemented yet.
    // TODO(0x7bd4ca8, Quantum.MeleeJumpAttackSystem.CanPerform): CanPerform(frame, entity, weapon, meleeJumpAttack, spec); not reimplemented yet.
    // TODO(0x7bd54b0, Quantum.MeleeJumpAttackSystem.ResetJumpAttack): ResetJumpAttack(frame, userEntity); not reimplemented yet.
    // TODO(0x7bd55fc, Quantum.MeleeJumpAttackSystem.WallRunningActor_OnWallRunningStart): WallRunningActor_OnWallRunningStart(frame, entity); not reimplemented yet.
    // TODO(0x7bd5600, Quantum.MeleeJumpAttackSystem.GrinderActor_OnExitGrind): GrinderActor_OnExitGrind(frame, entity, grindingDuration); not reimplemented yet.
    // TODO(0x7bd5604, Quantum.MeleeJumpAttackSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bd5608, Quantum.MeleeJumpAttackSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MineLauncherEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class MineLauncherEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bfaaac, Quantum.MineLauncherEnhancementSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(f, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7bfaba4, Quantum.MineLauncherEnhancementSystem.Weapon_OnBeforeUse): Weapon_OnBeforeUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfaccc, Quantum.MineLauncherEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.MineMergingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MineMergingSystem : public SystemMainThread {
  public:
    // TODO(0x7bfacd4, Quantum.MineMergingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bfb130, Quantum.MineMergingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MobGroupMemberSystem (original base: Quantum.SystemSignalsOnly)
class MobGroupMemberSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7cd3470, Quantum.MobGroupMemberSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(f, entity, hit); not reimplemented yet.
    // TODO(0x7cd3614, Quantum.MobGroupMemberSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.MobGroupSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MobGroupSystem : public SystemMainThread {
  public:
    // TODO(0x7cd361c, Quantum.MobGroupSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cd3818, Quantum.MobGroupSystem.UpdateMemberClosestToTarget): UpdateMemberClosestToTarget(f, filter); not reimplemented yet.
    // TODO(0x7cd3ba0, Quantum.MobGroupSystem.Lifetime_OnSpawnEntityOnDeath): Lifetime_OnSpawnEntityOnDeath(f, spawnParams); not reimplemented yet.
    // TODO(0x7cd3d2c, Quantum.MobGroupSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MoveToSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MoveToSystem : public SystemMainThread {
  public:
    // 0x7bd0a08
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bd0a10, Quantum.MoveToSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bd0a1c, Quantum.MoveToSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MovementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MovementSystem : public SystemMainThread {
  public:
    // TODO(0x7bdd258, Quantum.MovementSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bdd310, Quantum.MovementSystem.KccExtension_OnConfigChanged): KccExtension_OnConfigChanged(f, entity, component); not reimplemented yet.
    // TODO(0x7bdd3b4, Quantum.MovementSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bdd530, Quantum.MovementSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bdd548, Quantum.MovementSystem.Killable_OnRevive): Killable_OnRevive(frame, entity, position, rotation); not reimplemented yet.
    // TODO(0x7bdd554, Quantum.MovementSystem.ResetKccMotion): ResetKccMotion(frame, entity); not reimplemented yet.
    // TODO(0x7bdd6d8, Quantum.MovementSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bdd760, Quantum.MovementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bde1dc, Quantum.MovementSystem.UpdateRotation): UpdateRotation(f, filter, input, configExt); not reimplemented yet.
    // TODO(0x7bde664, Quantum.MovementSystem.OnCharacterCollision3D): OnCharacterCollision3D(fbase, character, hit); not reimplemented yet.
    // TODO(0x7bdebc4, Quantum.MovementSystem.OnCharacterTrigger3D): OnCharacterTrigger3D(f, character, hit); not reimplemented yet.
    // TODO(0x7bdebc8, Quantum.MovementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.MovingWallSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class MovingWallSystem : public SystemMainThread {
  public:
    // TODO(0x7bd8338, Quantum.MovingWallSystem.OnAdded): OnAdded(frame, entity, wall); not reimplemented yet.
    // TODO(0x7bd85bc, Quantum.MovingWallSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd8880, Quantum.MovingWallSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.OnKillCallbacksSystem (original base: Quantum.SystemSignalsOnly)
class OnKillCallbacksSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bc7718, Quantum.OnKillCallbacksSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bc7960, Quantum.OnKillCallbacksSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.OnTriggerEnterSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class OnTriggerEnterSystem : public SystemMainThread {
  public:
    // TODO(0x7bb79f0, Quantum.OnTriggerEnterSystem.OnAdded): OnAdded(f, entity, onTriggerEnter); not reimplemented yet.
    // TODO(0x7bb7a60, Quantum.OnTriggerEnterSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb7a64, Quantum.OnTriggerEnterSystem.CheckTriggerWithAreaOfEffect): CheckTriggerWithAreaOfEffect(f, filter); not reimplemented yet.
    // TODO(0x7bb7f00, Quantum.OnTriggerEnterSystem.OnTriggerEnter3D): OnTriggerEnter3D(f, info); not reimplemented yet.
    // TODO(0x7bb7c8c, Quantum.OnTriggerEnterSystem.CheckTrigger): CheckTrigger(f, triggeredEntity, triggeringEntity); not reimplemented yet.
    // TODO(0x7bb7f44, Quantum.OnTriggerEnterSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ParabolaFollowerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ParabolaFollowerSystem : public SystemMainThread {
  public:
    // 0x7bd0f78
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bd0f80, Quantum.ParabolaFollowerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bd1298, Quantum.ParabolaFollowerSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd1c7c, Quantum.ParabolaFollowerSystem.UpdateHoming): UpdateHoming(f, entity, pFollower, followerSpec); not reimplemented yet.
    // TODO(0x7bd1edc, Quantum.ParabolaFollowerSystem.UpdateAirControl): UpdateAirControl(f, entity, pFollower, followerSpec); not reimplemented yet.
    // TODO(0x7bd21a0, Quantum.ParabolaFollowerSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bd22c4, Quantum.ParabolaFollowerSystem.AutoJumperActor_OnStartJump): AutoJumperActor_OnStartJump(f, entity); not reimplemented yet.
    // TODO(0x7bd233c, Quantum.ParabolaFollowerSystem.Pushable_OnPushed): Pushable_OnPushed(f, pushParams, onPushedData); not reimplemented yet.
    // TODO(0x7bd1b5c, Quantum.ParabolaFollowerSystem.RequestStopParabolaFollower): RequestStopParabolaFollower(f, entity, reason); not reimplemented yet.
    // TODO(0x7bd1a5c, Quantum.ParabolaFollowerSystem.StopParabolaFollower): StopParabolaFollower(f, entity, reason); not reimplemented yet.
    // TODO(0x7bd23b4, Quantum.ParabolaFollowerSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7bd2044, Quantum.ParabolaFollowerSystem.<UpdateHoming>g__TryToAssignNewTarget|5_0): <UpdateHoming>g__TryToAssignNewTarget|5_0(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.PendingPhysicsImpulseSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class PendingPhysicsImpulseSystem : public SystemMainThread {
  public:
    // TODO(0x7bdec10, Quantum.PendingPhysicsImpulseSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bdedd8, Quantum.PendingPhysicsImpulseSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.PhysicsSleepSystem (original base: Quantum.SystemSignalsOnly)
class PhysicsSleepSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bdf27c, Quantum.PhysicsSleepSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bdf280, Quantum.PhysicsSleepSystem.OnAdded): OnAdded(f, entity, pRigidBody); not reimplemented yet.
    // TODO(0x7bdf298, Quantum.PhysicsSleepSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.PowerUpSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class PowerUpSystem : public SystemMainThread {
  public:
    // TODO(0x7bc7f60, Quantum.PowerUpSystem.OnTriggerEnter3D): OnTriggerEnter3D(f, info); not reimplemented yet.
    // TODO(0x7bc825c, Quantum.PowerUpSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bc8538, Quantum.PowerUpSystem.ClearPowerUps): ClearPowerUps(frame, userEntity); not reimplemented yet.
    // TODO(0x7bc81f4, Quantum.PowerUpSystem.CollectPowerUp): CollectPowerUp(f, userEntity, powerUpSpec, source); not reimplemented yet.
    // TODO(0x7bc880c, Quantum.PowerUpSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bc881c, Quantum.PowerUpSystem.CheckFinishedActivePowerUps): CheckFinishedActivePowerUps(f, userEntity, powerUpUser); not reimplemented yet.
    // TODO(0x7bc89ec, Quantum.PowerUpSystem.Reward_OnCollected): Reward_OnCollected(f, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bc8b44, Quantum.PowerUpSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.PrioritisedTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class PrioritisedTargetSystem : public SystemMainThread {
  public:
    // TODO(0x7cd46e8, Quantum.PrioritisedTargetSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7cd4770, Quantum.PrioritisedTargetSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7cd4af4, Quantum.PrioritisedTargetSystem.UpdateMobGroupTargets): UpdateMobGroupTargets(frame, filter, spec); not reimplemented yet.
    // TODO(0x7cd50dc, Quantum.PrioritisedTargetSystem.UpdateAttackedByThreats): UpdateAttackedByThreats(frame, filter, spec); not reimplemented yet.
    // TODO(0x7cd5368, Quantum.PrioritisedTargetSystem.UpdateVisibleThreats): UpdateVisibleThreats(frame, filter, spec); not reimplemented yet.
    // TODO(0x7cd55e8, Quantum.PrioritisedTargetSystem.UpdateRecentThreats): UpdateRecentThreats(frame, filter, spec); not reimplemented yet.
    // TODO(0x7cd5788, Quantum.PrioritisedTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ProjectileCollisionHandlerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ProjectileCollisionHandlerSystem : public SystemMainThread {
  public:
    // TODO(0x7c05fb0, Quantum.ProjectileCollisionHandlerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c07264, Quantum.ProjectileCollisionHandlerSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7c063bc, Quantum.ProjectileCollisionHandlerSystem.TrySetProjectileImpact): TrySetProjectileImpact(frame, projectileEntity, projectile, projectileTransform); not reimplemented yet.
    // TODO(0x7c073d4, Quantum.ProjectileCollisionHandlerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ProjectileCollisionInjectionSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ProjectileCollisionInjectionSystem : public SystemMainThread {
  public:
    // TODO(0x7c05b68, Quantum.ProjectileCollisionInjectionSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c05f68, Quantum.ProjectileCollisionInjectionSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ProjectileInitSystem (original base: Quantum.SystemSignalsOnly)
class ProjectileInitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7c0741c, Quantum.ProjectileInitSystem.OnAdded): OnAdded(f, entity, projectile); not reimplemented yet.
    // TODO(0x7c075ac, Quantum.ProjectileInitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ProjectileMovementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ProjectileMovementSystem : public SystemMainThread {
  public:
    // TODO(0x7c096ec, Quantum.ProjectileMovementSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c0a06c, Quantum.ProjectileMovementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ProjectileSpawnerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ProjectileSpawnerSystem : public SystemMainThread {
  public:
    // TODO(0x7c0a34c, Quantum.ProjectileSpawnerSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7c0a3f8, Quantum.ProjectileSpawnerSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7c0a9bc, Quantum.ProjectileSpawnerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7c0aa04, Quantum.ProjectileSpawnerSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7c0ac98, Quantum.ProjectileSpawnerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.PushStopSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class PushStopSystem : public SystemMainThread {
  public:
    // TODO(0x7bc9f54, Quantum.PushStopSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7bc9fac, Quantum.PushStopSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bca198, Quantum.PushStopSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.PushableSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class PushableSystem : public SystemMainThread {
  public:
    // TODO(0x7bc8f74, Quantum.PushableSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7bc8fcc, Quantum.PushableSystem.OnAdded): OnAdded(frame, entity, pushable); not reimplemented yet.
    // TODO(0x7bc9160, Quantum.PushableSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bc9178, Quantum.PushableSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bc970c, Quantum.PushableSystem.InitialiseStatsIfNeeded): InitialiseStatsIfNeeded(frame, pushableEntity, pushable, pushableSpec); not reimplemented yet.
    // TODO(0x7bc9734, Quantum.PushableSystem.Shield_OnBreaks): Shield_OnBreaks(frame, entity, sourceEntity, direction); not reimplemented yet.
    // TODO(0x7bc9a98, Quantum.PushableSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bc9dac, Quantum.PushableSystem.Killable_OnRevive): Killable_OnRevive(frame, entity, position, rotation); not reimplemented yet.
    // TODO(0x7bc9e4c, Quantum.PushableSystem.KccExtension_OnCharacterCollision3D): KccExtension_OnCharacterCollision3D(f, character, hit); not reimplemented yet.
    // TODO(0x7bc9f0c, Quantum.PushableSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RailsSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class RailsSystem : public SystemMainThread {
  public:
    // TODO(0x7bd8d14, Quantum.RailsSystem.OnInit): OnInit(frame); not reimplemented yet.
    // TODO(0x7bd8d18, Quantum.RailsSystem.OnAdded): OnAdded(frame, entity, component); not reimplemented yet.
    // TODO(0x7bd8d4c, Quantum.RailsSystem.OnAdded): OnAdded(frame, entity, component); not reimplemented yet.
    // TODO(0x7bd8d60, Quantum.RailsSystem.OnAdded): OnAdded(frame, entity, component); not reimplemented yet.
    // TODO(0x7bd8d6c, Quantum.RailsSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd8da0, Quantum.RailsSystem.Quantum.ISignalOnMapChanged.OnMapChanged): Quantum.ISignalOnMapChanged.OnMapChanged(frame, previousMap); not reimplemented yet.
    // TODO(0x7bd8da4, Quantum.RailsSystem.Pushable_OnPushed): Pushable_OnPushed(f, pushParams, onPushedData); not reimplemented yet.
    // TODO(0x7bd8ed0, Quantum.RailsSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RandomWeaponsModifierSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class RandomWeaponsModifierSystem : public SystemMainThread {
  public:
    // 0x7bc7074
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bc707c, Quantum.RandomWeaponsModifierSystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, actor); not reimplemented yet.
    // TODO(0x7bc73a8, Quantum.RandomWeaponsModifierSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(frame, gameMode); not reimplemented yet.
    // TODO(0x7bc7588, Quantum.RandomWeaponsModifierSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bc7184, Quantum.RandomWeaponsModifierSystem.ChangeWeapons): ChangeWeapons(frame, actorEntity, playerActor, loadoutActor); not reimplemented yet.
    // TODO(0x7bc6f98, Quantum.RandomWeaponsModifierSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RecoverAmmoOnDashEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class RecoverAmmoOnDashEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bed7c0, Quantum.RecoverAmmoOnDashEnhancementSystem.DashActor_OnDashed): DashActor_OnDashed(f, entity); not reimplemented yet.
    // TODO(0x7beda00, Quantum.RecoverAmmoOnDashEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ReviveTotemSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ReviveTotemSystem : public SystemMainThread {
  public:
    // 0x7bc5948
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bc5950, Quantum.ReviveTotemSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bc59d8, Quantum.ReviveTotemSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bc5c04, Quantum.ReviveTotemSystem.Killable_OnRevive): Killable_OnRevive(frame, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7bc5c10, Quantum.ReviveTotemSystem.GameMode_OnPlayerFinished): GameMode_OnPlayerFinished(frame, entity); not reimplemented yet.
    // TODO(0x7bc5a88, Quantum.ReviveTotemSystem.DropReviveTotem): DropReviveTotem(frame, killedPlayer); not reimplemented yet.
    // TODO(0x7bc5c1c, Quantum.ReviveTotemSystem.FindSafeDropPosition): FindSafeDropPosition(frame, dropper); not reimplemented yet.
    // TODO(0x7bc5dfc, Quantum.ReviveTotemSystem.Reward_OnCollected): Reward_OnCollected(frame, collectorEntity, type, amount); not reimplemented yet.
    // TODO(0x7bc5f00, Quantum.ReviveTotemSystem.Droppable_OnDestroyed): Droppable_OnDestroyed(frame, droppable); not reimplemented yet.
    // TODO(0x7bc5f40, Quantum.ReviveTotemSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RewardSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class RewardSystem : public SystemMainThread {
  public:
    // TODO(0x7bcb720, Quantum.RewardSystem.Killable_OnKill): Killable_OnKill(frame, killedEntity, killedBy); not reimplemented yet.
    // TODO(0x7bcb8c4, Quantum.RewardSystem.Shield_OnBreaks): Shield_OnBreaks(frame, entity, sourceEntity, direction); not reimplemented yet.
    // TODO(0x7bcba78, Quantum.RewardSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bcbbc4, Quantum.RewardSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RollBouncingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class RollBouncingSystem : public SystemMainThread {
  public:
    // 0x7bdf510
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bdf518, Quantum.RollBouncingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bdf5b4, Quantum.RollBouncingSystem.HandleHits): HandleHits(f, filter); not reimplemented yet.
    // TODO(0x7bdfee4, Quantum.RollBouncingSystem.Bounce): Bounce(position, normal, f, filter); not reimplemented yet.
    // TODO(0x7be01dc, Quantum.RollBouncingSystem.Weapon_OnShoot): Weapon_OnShoot(f, weaponEntity, ammunitionEntity); not reimplemented yet.
    // TODO(0x7be0340, Quantum.RollBouncingSystem.Projectile_OnSpawnEntityOnDeath): Projectile_OnSpawnEntityOnDeath(f, spawnedEntity, projectileEntity, hitInfo); not reimplemented yet.
    // TODO(0x7be04a4, Quantum.RollBouncingSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7bdfce4, Quantum.RollBouncingSystem.<HandleHits>g__EvaluateHit|4_0): <HandleHits>g__EvaluateHit|4_0(f, hitEntity); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RollPhysicSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class RollPhysicSystem : public SystemMainThread {
  public:
    // TODO(0x7be04ec, Quantum.RollPhysicSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be08f8, Quantum.RollPhysicSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.RollSignalSystem (original base: Quantum.SystemSignalsOnly)
class RollSignalSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be0940, Quantum.RollSignalSystem.OnCollisionEnter3D): OnCollisionEnter3D(f, info); not reimplemented yet.
    // TODO(0x7be0c6c, Quantum.RollSignalSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.ShapeOverlappingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ShapeOverlappingSystem : public SystemMainThread {
  public:
    // 0x7be0d04
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7be0d0c, Quantum.ShapeOverlappingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be1734, Quantum.ShapeOverlappingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ShieldedActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ShieldedActorSystem : public SystemMainThread {
  public:
    // 0x7cb763c
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7cb7644, Quantum.ShieldedActorSystem.Killable_OnRevive): Killable_OnRevive(frame, entity, position, rotation); not reimplemented yet.
    // TODO(0x7cb774c, Quantum.ShieldedActorSystem.InitActorIfNeeded): InitActorIfNeeded(frame, entity, shieldedActor, spec); not reimplemented yet.
    // TODO(0x7cb7a00, Quantum.ShieldedActorSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7cb7b08, Quantum.ShieldedActorSystem.WaitForRegen): WaitForRegen(frame, entity, shieldedActor); not reimplemented yet.
    // TODO(0x7cb7b74, Quantum.ShieldedActorSystem.Regenerate): Regenerate(frame, entity, shieldedActor, spec); not reimplemented yet.
    // TODO(0x7cb7c20, Quantum.ShieldedActorSystem.OnDisabled): OnDisabled(f); not reimplemented yet.
    // TODO(0x7cb7d9c, Quantum.ShieldedActorSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7cb7dc4, Quantum.ShieldedActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ShotgunEnhancementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ShotgunEnhancementSystem : public SystemMainThread {
  public:
    // TODO(0x7bfc07c, Quantum.ShotgunEnhancementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bfc2dc, Quantum.ShotgunEnhancementSystem.DashActor_OnDashed): DashActor_OnDashed(f, playerEntity); not reimplemented yet.
    // TODO(0x7bfc518, Quantum.ShotgunEnhancementSystem.DashActor_OnDashIsReady): DashActor_OnDashIsReady(f, playerEntity); not reimplemented yet.
    // TODO(0x7bfc730, Quantum.ShotgunEnhancementSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfc7b8, Quantum.ShotgunEnhancementSystem.Killable_OnRevive): Killable_OnRevive(f, playerEntity, position, rotation); not reimplemented yet.
    // TODO(0x7bfc5dc, Quantum.ShotgunEnhancementSystem.TryResetToDefaultWeaponData): TryResetToDefaultWeaponData(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfc87c, Quantum.ShotgunEnhancementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ShrinkingZoneActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ShrinkingZoneActorSystem : public SystemMainThread {
  public:
    // 0x7bcc05c
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bcc064, Quantum.ShrinkingZoneActorSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bcc260, Quantum.ShrinkingZoneActorSystem.DealDamageToEntityOutsideZone): DealDamageToEntityOutsideZone(frame, filter, zone, zoneSpec); not reimplemented yet.
    // TODO(0x7bcc498, Quantum.ShrinkingZoneActorSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(frame, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7bcc634, Quantum.ShrinkingZoneActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ShrinkingZoneSystem (original base: Quantum.SystemMainThread)
class ShrinkingZoneSystem : public SystemMainThread {
  public:
    // TODO(0x7bcbc0c, Quantum.ShrinkingZoneSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bcbcc8, Quantum.ShrinkingZoneSystem.Update): Update(f); not reimplemented yet.
    // TODO(0x7bcc054, Quantum.ShrinkingZoneSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.SpeederSystem (original base: Quantum.SystemSignalsOnly)
class SpeederSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bd9a94, Quantum.SpeederSystem.OnTriggerEnter3D): OnTriggerEnter3D(frame, info); not reimplemented yet.
    // TODO(0x7bd9d88, Quantum.SpeederSystem.OnTriggerExit3D): OnTriggerExit3D(frame, info); not reimplemented yet.
    // TODO(0x7bd9c04, Quantum.SpeederSystem.IsValidTrigger): IsValidTrigger(frame, triggerEntity, otherEntity, speeder, kcc); not reimplemented yet.
    // TODO(0x7bd9de8, Quantum.SpeederSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.SquadSiegeGameModeSystem (original base: Quantum.SystemMainThread)
class SquadSiegeGameModeSystem : public SystemMainThread {
  public:
    // 0x7ba8600
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7ba8608, Quantum.SquadSiegeGameModeSystem.OnInit): OnInit(frame); not reimplemented yet.
    // TODO(0x7ba894c, Quantum.SquadSiegeGameModeSystem.Update): Update(frame); not reimplemented yet.
    // TODO(0x7ba9150, Quantum.SquadSiegeGameModeSystem.CheckTimeSinceLastSpawn): CheckTimeSinceLastSpawn(frame); not reimplemented yet.
    // TODO(0x7ba934c, Quantum.SquadSiegeGameModeSystem.DestroyAllMobs): DestroyAllMobs(frame); not reimplemented yet.
    // TODO(0x7ba89c0, Quantum.SquadSiegeGameModeSystem.DestroyAllStuckMobs): DestroyAllStuckMobs(frame); not reimplemented yet.
    // TODO(0x7ba9608, Quantum.SquadSiegeGameModeSystem.WaveSpawner_OnSpawned): WaveSpawner_OnSpawned(f, spawned, spawner); not reimplemented yet.
    // TODO(0x7ba9860, Quantum.SquadSiegeGameModeSystem.AISpawner_OnSpawned): AISpawner_OnSpawned(f, spawner, spawned); not reimplemented yet.
    // TODO(0x7ba9960, Quantum.SquadSiegeGameModeSystem.TryGetWaypointsToTarget): TryGetWaypointsToTarget(frame, entity, waypoints, waypointRadius, currentWaypointIndex); not reimplemented yet.
    // TODO(0x7ba96d8, Quantum.SquadSiegeGameModeSystem.SetTargetEntityOnSpawnedMob): SetTargetEntityOnSpawnedMob(f, spawned, waypoints, waypointRadius, currentWaypointIndex); not reimplemented yet.
    // TODO(0x7ba9d30, Quantum.SquadSiegeGameModeSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(frame, gameMode); not reimplemented yet.
    // TODO(0x7ba9f10, Quantum.SquadSiegeGameModeSystem.Killable_OnKill): Killable_OnKill(frame, toKill, killedBy); not reimplemented yet.
    // TODO(0x7ba9f60, Quantum.SquadSiegeGameModeSystem.Damageable_HitReceived): Damageable_HitReceived(frame, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7baa0a0, Quantum.SquadSiegeGameModeSystem.Target_HitReceived): Target_HitReceived(frame, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7baa254, Quantum.SquadSiegeGameModeSystem.Weapon_OnShoot): Weapon_OnShoot(frame, weaponEntity, ammunitionEntity); not reimplemented yet.
    // TODO(0x7baa300, Quantum.SquadSiegeGameModeSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7baa308, Quantum.SquadSiegeGameModeSystem..cctor): .cctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.StreamEffectSpawnerSystem (original base: Quantum.SystemSignalsOnly)
class StreamEffectSpawnerSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bcc67c, Quantum.StreamEffectSpawnerSystem.OnAdded): OnAdded(f, entity, streamEffectSpawner); not reimplemented yet.
    // TODO(0x7bcc804, Quantum.StreamEffectSpawnerSystem.Weapon_OnCreated): Weapon_OnCreated(f, weaponEntity); not reimplemented yet.
    // TODO(0x7bcca90, Quantum.StreamEffectSpawnerSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bccd38, Quantum.StreamEffectSpawnerSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.StreamEffectSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class StreamEffectSystem : public SystemMainThread {
  public:
    // TODO(0x7bccd40, Quantum.StreamEffectSystem.OnAdded): OnAdded(f, entity, streamEffect); not reimplemented yet.
    // TODO(0x7bccdb4, Quantum.StreamEffectSystem.OnRemoved): OnRemoved(f, entity, streamEffect); not reimplemented yet.
    // TODO(0x7bccdd0, Quantum.StreamEffectSystem.AreaOfEffect_OnCalculateDynamicShape): AreaOfEffect_OnCalculateDynamicShape(f, areaOfEffectEntity, outShape); not reimplemented yet.
    // TODO(0x7bcd394, Quantum.StreamEffectSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bcd39c, Quantum.StreamEffectSystem.UpdateStreamPointsPosition): UpdateStreamPointsPosition(f, streamEffect); not reimplemented yet.
    // TODO(0x7bcd71c, Quantum.StreamEffectSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.SweepAttackSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class SweepAttackSystem : public SystemMainThread {
  public:
    // TODO(0x7cc9950, Quantum.SweepAttackSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cc9cf0, Quantum.SweepAttackSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetableSystem_FindTargetsInRange (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetableSystem_FindTargetsInRange : public SystemMainThread {
  public:
    // TODO(0x7bcec98, Quantum.TargetableSystem_FindTargetsInRange.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bced54, Quantum.TargetableSystem_FindTargetsInRange.OnDisabled): OnDisabled(frame); not reimplemented yet.
    // TODO(0x7bceed8, Quantum.TargetableSystem_FindTargetsInRange.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bcf010, Quantum.TargetableSystem_FindTargetsInRange.Weapon_OnEquip): Weapon_OnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bcf1c0, Quantum.TargetableSystem_FindTargetsInRange.Weapon_OnBeforeUse): Weapon_OnBeforeUse(frame, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bcf2c8, Quantum.TargetableSystem_FindTargetsInRange..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetableSystem_HandleRayCastToPotentialTargets (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetableSystem_HandleRayCastToPotentialTargets : public SystemMainThread {
  public:
    // TODO(0x7bcfc74, Quantum.TargetableSystem_HandleRayCastToPotentialTargets.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bd0074, Quantum.TargetableSystem_HandleRayCastToPotentialTargets..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetableSystem_InjectRayCastToTargetsInRange (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetableSystem_InjectRayCastToTargetsInRange : public SystemMainThread {
  public:
    // TODO(0x7bcf504, Quantum.TargetableSystem_InjectRayCastToTargetsInRange.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bcfc2c, Quantum.TargetableSystem_InjectRayCastToTargetsInRange..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetableSystem_ScoreTargets (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetableSystem_ScoreTargets : public SystemMainThread {
  public:
    // TODO(0x7bd00bc, Quantum.TargetableSystem_ScoreTargets.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bd0190, Quantum.TargetableSystem_ScoreTargets.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bd0268, Quantum.TargetableSystem_ScoreTargets..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetableSystem_UpdateTargetPositions (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetableSystem_UpdateTargetPositions : public SystemMainThread {
  public:
    // TODO(0x7bcf310, Quantum.TargetableSystem_UpdateTargetPositions.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bcf4bc, Quantum.TargetableSystem_UpdateTargetPositions..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TargetedEffectSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class TargetedEffectSystem : public SystemMainThread {
  public:
    // TODO(0x7bd02b0, Quantum.TargetedEffectSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd0754, Quantum.TargetedEffectSystem.DestroySelf): DestroySelf(f, entity); not reimplemented yet.
    // TODO(0x7bd076c, Quantum.TargetedEffectSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TeamTotalRumbleRankingSystem (original base: Quantum.RankingSystem)
class TeamTotalRumbleRankingSystem : public RankingSystem {
  public:
    // TODO(0x7baa958, Quantum.TeamTotalRumbleRankingSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7baab64, Quantum.TeamTotalRumbleRankingSystem.FillRankingList): FillRankingList(frame, list); not reimplemented yet.
    // TODO(0x7baaa34, Quantum.TeamTotalRumbleRankingSystem.CloseToWinCheck): CloseToWinCheck(f, rankingManager); not reimplemented yet.
    // TODO(0x7ba7a54, Quantum.TeamTotalRumbleRankingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TimeScaleSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class TimeScaleSystem : public SystemMainThread {
  public:
    // TODO(0x7bab1f0, Quantum.TimeScaleSystem.OnInit): OnInit(frame); not reimplemented yet.
    // TODO(0x7bab284, Quantum.TimeScaleSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bab3cc, Quantum.TimeScaleSystem.GameMode_OnStateChanged): GameMode_OnStateChanged(frame, prevState, newState); not reimplemented yet.
    // TODO(0x7ba796c, Quantum.TimeScaleSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TrailSpawnerSystem (original base: Quantum.SystemSignalsOnly)
class TrailSpawnerSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bf6678, Quantum.TrailSpawnerSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bf6850, Quantum.TrailSpawnerSystem.OnCollisionEnter3D): OnCollisionEnter3D(f, info); not reimplemented yet.
    // TODO(0x7bf6a30, Quantum.TrailSpawnerSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.TrainingGameModeSystem (original base: Quantum.SystemMainThread)
class TrainingGameModeSystem : public SystemMainThread {
  public:
    // 0x7bab648
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bab650, Quantum.TrainingGameModeSystem.OnDisabled): OnDisabled(f); not reimplemented yet.
    // TODO(0x7bab684, Quantum.TrainingGameModeSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bab694, Quantum.TrainingGameModeSystem.Update): Update(f); not reimplemented yet.
    // TODO(0x7bab698, Quantum.TrainingGameModeSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(f, gameMode); not reimplemented yet.
    // TODO(0x7ba7a04, Quantum.TrainingGameModeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TravolatorActorSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class TravolatorActorSystem : public SystemMainThread {
  public:
    // TODO(0x7cb7e0c, Quantum.TravolatorActorSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7cb7fc0, Quantum.TravolatorActorSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TurnToSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class TurnToSystem : public SystemMainThread {
  public:
    // 0x7bd260c
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bd2614, Quantum.TurnToSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd28ec, Quantum.TurnToSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.TutorialGameModeSystem (original base: Quantum.GameModeSystemBase<Filter>)
class TutorialGameModeSystem : public SystemMainThread {
  public:
    // Generic GameModeSystemBase<T>.get_StartEnabled (instance 0x5a1cb90)
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bac0c0, Quantum.TutorialGameModeSystem.OnDisabled): OnDisabled(f); not reimplemented yet.
    // TODO(0x7bac0f4, Quantum.TutorialGameModeSystem.AISpawner_OnSpawned): AISpawner_OnSpawned(f, spawner, spawned); not reimplemented yet.
    // TODO(0x7bac22c, Quantum.TutorialGameModeSystem.Killable_OnKill): Killable_OnKill(frame, entity, killedBy); not reimplemented yet.
    // TODO(0x7bac5a8, Quantum.TutorialGameModeSystem.Bolt_OnCollected): Bolt_OnCollected(frame, collector, amount); not reimplemented yet.
    // TODO(0x7bac5ac, Quantum.TutorialGameModeSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bac674, Quantum.TutorialGameModeSystem.HandleStepCommand): HandleStepCommand(frame, gameMode); not reimplemented yet.
    // TODO(0x7bac754, Quantum.TutorialGameModeSystem.HandleUnlockFeatureCommand): HandleUnlockFeatureCommand(frame, gameMode); not reimplemented yet.
    // TODO(0x7bac828, Quantum.TutorialGameModeSystem.HandleUseUltimateCommand): HandleUseUltimateCommand(frame, gameMode); not reimplemented yet.
    // TODO(0x7bac8f4, Quantum.TutorialGameModeSystem.HandleToggleBotCommand): HandleToggleBotCommand(frame, gameMode); not reimplemented yet.
    // TODO(0x7bacb30, Quantum.TutorialGameModeSystem.SetTutorialStep): SetTutorialStep(frame, playerEntity, gameMode, step, isPVETuto); not reimplemented yet.
    // TODO(0x7badd14, Quantum.TutorialGameModeSystem.TeleportPlayer): TeleportPlayer(frame, playerFilter, position, rotation, ignoreDistance); not reimplemented yet.
    // TODO(0x7bac9e0, Quantum.TutorialGameModeSystem.GetPlayerFilter): GetPlayerFilter(frame); not reimplemented yet.
    // TODO(0x7badfcc, Quantum.TutorialGameModeSystem.Weapon_OnBeforeEquip): Weapon_OnBeforeEquip(frame, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bae188, Quantum.TutorialGameModeSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(frame, entity, state); not reimplemented yet.
    // TODO(0x7bae22c, Quantum.TutorialGameModeSystem.PlayerActor_OnAfterInitialized): PlayerActor_OnAfterInitialized(frame, actorEntity, actor); not reimplemented yet.
    // TODO(0x7ba7924, Quantum.TutorialGameModeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.UltimateAbilitySystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class UltimateAbilitySystem : public SystemMainThread {
  public:
    // TODO(0x7c0c930, Quantum.UltimateAbilitySystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, playerActor); not reimplemented yet.
    // TODO(0x7c0cce4, Quantum.UltimateAbilitySystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c0cda8, Quantum.UltimateAbilitySystem.ApplyRankingModifiers): ApplyRankingModifiers(frame, entity, ultimateAbilityUser); not reimplemented yet.
    // TODO(0x7c0ce18, Quantum.UltimateAbilitySystem.ChargeUltimateValueWithTime): ChargeUltimateValueWithTime(frame, filter, spec); not reimplemented yet.
    // TODO(0x7c0cf5c, Quantum.UltimateAbilitySystem.CheckFinishUltimateIfDurationHasElapsed): CheckFinishUltimateIfDurationHasElapsed(frame, filter, spec); not reimplemented yet.
    // TODO(0x7c0d000, Quantum.UltimateAbilitySystem.OnAdded): OnAdded(frame, entity, ultimateAbilityUser); not reimplemented yet.
    // TODO(0x7c0d00c, Quantum.UltimateAbilitySystem.Bolt_OnCollected): Bolt_OnCollected(frame, collector, amount); not reimplemented yet.
    // TODO(0x7c0d304, Quantum.UltimateAbilitySystem.Pushable_OnAddPush): Pushable_OnAddPush(frame, pushable, pushData); not reimplemented yet.
    // TODO(0x7c0d498, Quantum.UltimateAbilitySystem.Pushable_OnPushed): Pushable_OnPushed(frame, pushParams, onPushedData); not reimplemented yet.
    // TODO(0x7c0d5cc, Quantum.UltimateAbilitySystem.Damageable_HitReceived): Damageable_HitReceived(frame, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7c0d808, Quantum.UltimateAbilitySystem.Killable_OnRevive): Killable_OnRevive(frame, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7c0d1d8, Quantum.UltimateAbilitySystem.TryGiveChargeFromSpec): TryGiveChargeFromSpec(frame, entity, specDelegate); not reimplemented yet.
    // TODO(0x7c0d900, Quantum.UltimateAbilitySystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(frame, entity, state); not reimplemented yet.
    // TODO(0x7c0da8c, Quantum.UltimateAbilitySystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7c0dbf4, Quantum.UltimateAbilitySystem.UltimateAbilityUser_OnRecast): UltimateAbilityUser_OnRecast(f, entity); not reimplemented yet.
    // TODO(0x7c0dc10, Quantum.UltimateAbilitySystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7c0dc1c, Quantum.UltimateAbilitySystem.BlastBallGame_StateChanged): BlastBallGame_StateChanged(f, newState); not reimplemented yet.
    // TODO(0x7c0dd68, Quantum.UltimateAbilitySystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.UltimateEvolutionSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class UltimateEvolutionSystem : public SystemMainThread {
  public:
    // TODO(0x7be3d7c, Quantum.UltimateEvolutionSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7be3e28, Quantum.UltimateEvolutionSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be4160, Quantum.UltimateEvolutionSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7be4170, Quantum.UltimateEvolutionSystem.OnAbilityEnds): OnAbilityEnds(f, entity); not reimplemented yet.
    // TODO(0x7be4214, Quantum.UltimateEvolutionSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.UltimateSlamSystem (original base: Quantum.SystemSignalsOnly)
class UltimateSlamSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be4318, Quantum.UltimateSlamSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7be43c4, Quantum.UltimateSlamSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7be4554, Quantum.UltimateSlamSystem.OnUltimateSlamActivated): OnUltimateSlamActivated(f, entity, pSlam); not reimplemented yet.
    // TODO(0x7be4d20, Quantum.UltimateSlamSystem.OnUltimateSlamEnded): OnUltimateSlamEnded(f, entity, pSlam); not reimplemented yet.
    // TODO(0x7be4f90, Quantum.UltimateSlamSystem.TryToOverrideTargetFinderSpec): TryToOverrideTargetFinderSpec(f, entity, slamSpec); not reimplemented yet.
    // TODO(0x7be53e8, Quantum.UltimateSlamSystem.TryToResetTargetFinderSpec): TryToResetTargetFinderSpec(f, entity); not reimplemented yet.
    // TODO(0x7be55b4, Quantum.UltimateSlamSystem.ParabolaFollower_OnRemove): ParabolaFollower_OnRemove(f, entity, reason); not reimplemented yet.
    // TODO(0x7be4e18, Quantum.UltimateSlamSystem.ExitGrindFromSlam): ExitGrindFromSlam(f, entity); not reimplemented yet.
    // TODO(0x7be5898, Quantum.UltimateSlamSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7be50a4, Quantum.UltimateSlamSystem.<OnUltimateSlamActivated>g__GetDestination|4_0): <OnUltimateSlamActivated>g__GetDestination|4_0(f, raycastCount, fallbackPos, ); not reimplemented yet.
    // TODO(0x7be58a0, Quantum.UltimateSlamSystem.<OnUltimateSlamActivated>g__EvaluateVerticalRayHit|4_1): <OnUltimateSlamActivated>g__EvaluateVerticalRayHit|4_1(raycastCount, hitMagnitude, ); not reimplemented yet.
};

// Quantum.UltimateTankSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class UltimateTankSystem : public SystemMainThread {
  public:
    // TODO(0x7be5ce0, Quantum.UltimateTankSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, playerEntity, state); not reimplemented yet.
    // TODO(0x7be5ec0, Quantum.UltimateTankSystem.OnUltimateActivated): OnUltimateActivated(frame, playerEntity, playerCollider, playerHealth, ultimateTankSpec); not reimplemented yet.
    // TODO(0x7be63b4, Quantum.UltimateTankSystem.OnUltimateCharging): OnUltimateCharging(frame, playerEntity, playerCollider, playerHealth); not reimplemented yet.
    // TODO(0x7be6490, Quantum.UltimateTankSystem.DestroyTankEntityAndResetHealth): DestroyTankEntityAndResetHealth(frame, playerEntity, playerHealth); not reimplemented yet.
    // TODO(0x7be66ac, Quantum.UltimateTankSystem.LedgeGrabbingActor_OnLedgeGrabStart): LedgeGrabbingActor_OnLedgeGrabStart(frame, playerEntity); not reimplemented yet.
    // TODO(0x7be6584, Quantum.UltimateTankSystem.TryGetUltimateTank): TryGetUltimateTank(frame, playerEntity, tankEntity); not reimplemented yet.
    // TODO(0x7be66ec, Quantum.UltimateTankSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be6a14, Quantum.UltimateTankSystem.DecreaseHealthWithTime): DecreaseHealthWithTime(f, ultimateTank, tankHealth, ultimateTankSpec); not reimplemented yet.
    // TODO(0x7be6ae4, Quantum.UltimateTankSystem.Damageable_OnBeforeHit): Damageable_OnBeforeHit(f, targetEntity, hitData); not reimplemented yet.
    // TODO(0x7be5c08, Quantum.UltimateTankSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.UltimateWeaponSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class UltimateWeaponSystem : public SystemMainThread {
  public:
    // TODO(0x7be6c70, Quantum.UltimateWeaponSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7be6d18, Quantum.UltimateWeaponSystem.CheckFinishUltimateOnWeaponUse): CheckFinishUltimateOnWeaponUse(frame, filter, spec); not reimplemented yet.
    // TODO(0x7be6e70, Quantum.UltimateWeaponSystem.UltimateAbilityUser_OnStateChanged): UltimateAbilityUser_OnStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7be5bc0, Quantum.UltimateWeaponSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.UnlockedFeaturesManagerSystem (original base: Quantum.SystemSignalsOnly)
class UnlockedFeaturesManagerSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be7240, Quantum.UnlockedFeaturesManagerSystem.OnAdded): OnAdded(frame, entity, unlockedFeaturesManager); not reimplemented yet.
    // TODO(0x7be7328, Quantum.UnlockedFeaturesManagerSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.VoidRepulserBulldozerEnhancementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class VoidRepulserBulldozerEnhancementSystem : public SystemMainThread {
  public:
    // TODO(0x7bfd554, Quantum.VoidRepulserBulldozerEnhancementSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bfd634, Quantum.VoidRepulserBulldozerEnhancementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bfd91c, Quantum.VoidRepulserBulldozerEnhancementSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfd8dc, Quantum.VoidRepulserBulldozerEnhancementSystem.RemoveMoveSpeedModifier): RemoveMoveSpeedModifier(f, bulldozer, kcc, moveSpeedModOrigin); not reimplemented yet.
    // TODO(0x7bfda90, Quantum.VoidRepulserBulldozerEnhancementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.VoidRepulserShieldSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class VoidRepulserShieldSystem : public SystemMainThread {
  public:
    // TODO(0x7bfdae0, Quantum.VoidRepulserShieldSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bfdeb0, Quantum.VoidRepulserShieldSystem.Damageable_HitReceived): Damageable_HitReceived(frame, entity, hitData, hitResult); not reimplemented yet.
    // TODO(0x7bfdf2c, Quantum.VoidRepulserShieldSystem.OnRemoved): OnRemoved(frame, entity, component); not reimplemented yet.
    // TODO(0x7bfdd44, Quantum.VoidRepulserShieldSystem.RestoreData): RestoreData(frame, entity, voidRepulserShield); not reimplemented yet.
    // TODO(0x7bfe0b4, Quantum.VoidRepulserShieldSystem.Weapon_OnReload): Weapon_OnReload(f, weaponEntity, weapon); not reimplemented yet.
    // TODO(0x7bfe1c4, Quantum.VoidRepulserShieldSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.VoidRepulserSpeedBoostEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class VoidRepulserSpeedBoostEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bfe284, Quantum.VoidRepulserSpeedBoostEnhancementSystem.OnVoidRepulserShieldStateChanged): OnVoidRepulserShieldStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7bfe4bc, Quantum.VoidRepulserSpeedBoostEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.VoidRepulserVoidWaveEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class VoidRepulserVoidWaveEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bfe540, Quantum.VoidRepulserVoidWaveEnhancementSystem.OnVoidRepulserShieldStateChanged): OnVoidRepulserShieldStateChanged(f, entity, state); not reimplemented yet.
    // TODO(0x7bfe8a0, Quantum.VoidRepulserVoidWaveEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WakeUpSurroundingsOnPushOrDestroySystem (original base: Quantum.SystemSignalsOnly)
class WakeUpSurroundingsOnPushOrDestroySystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be177c, Quantum.WakeUpSurroundingsOnPushOrDestroySystem.WakeUpSurroundings): WakeUpSurroundings(f, entity); not reimplemented yet.
    // TODO(0x7be1afc, Quantum.WakeUpSurroundingsOnPushOrDestroySystem.Killable_OnKill): Killable_OnKill(f, entity, killedBy); not reimplemented yet.
    // TODO(0x7be1b00, Quantum.WakeUpSurroundingsOnPushOrDestroySystem.Pushable_OnAddPush): Pushable_OnAddPush(f, pushable, pushParams); not reimplemented yet.
    // TODO(0x7be1b14, Quantum.WakeUpSurroundingsOnPushOrDestroySystem.OnCollisionEnter3D): OnCollisionEnter3D(frame, info); not reimplemented yet.
    // TODO(0x7be1b38, Quantum.WakeUpSurroundingsOnPushOrDestroySystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WallRunningSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WallRunningSystem : public SystemMainThread {
  public:
    // TODO(0x7bd9324, Quantum.WallRunningSystem.OnAdded): OnAdded(f, entity, wallRunningActor); not reimplemented yet.
    // TODO(0x7bd934c, Quantum.WallRunningSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bd93f8, Quantum.WallRunningSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bd9464, Quantum.WallRunningSystem.Pushable_OnPushed): Pushable_OnPushed(f, pushParams, onPushedData); not reimplemented yet.
    // TODO(0x7bd95e4, Quantum.WallRunningSystem.DashActor_OnDashed): DashActor_OnDashed(f, entity); not reimplemented yet.
    // TODO(0x7bd96b4, Quantum.WallRunningSystem.Killable_OnRevive): Killable_OnRevive(f, toRevive, position, rotation); not reimplemented yet.
    // TODO(0x7bd983c, Quantum.WallRunningSystem.BlastBallGame_StateChanged): BlastBallGame_StateChanged(f, newState); not reimplemented yet.
    // TODO(0x7bd9a4c, Quantum.WallRunningSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WarmongerMiniRocketsEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class WarmongerMiniRocketsEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bfe920, Quantum.WarmongerMiniRocketsEnhancementSystem.Weapon_OnEquip): Weapon_OnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfec58, Quantum.WarmongerMiniRocketsEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WaveManagerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WaveManagerSystem : public SystemMainThread {
  public:
    // 0x7bd2aa4
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bd2aac, Quantum.WaveManagerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bd34e4, Quantum.WaveManagerSystem.GameMode_OnMainLoopStart): GameMode_OnMainLoopStart(f, gameMode); not reimplemented yet.
    // TODO(0x7bd39d4, Quantum.WaveManagerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponBehaviour_ChargingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponBehaviour_ChargingSystem : public SystemMainThread {
  public:
    // TODO(0x7be75d8, Quantum.WeaponBehaviour_ChargingSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7be77d0, Quantum.WeaponBehaviour_ChargingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be7b20, Quantum.WeaponBehaviour_ChargingSystem.Weapon_OnBeforeUse): Weapon_OnBeforeUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7be7cd0, Quantum.WeaponBehaviour_ChargingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponBigBoomSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponBigBoomSystem : public SystemMainThread {
  public:
    // TODO(0x7befa58, Quantum.WeaponBigBoomSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7befb60, Quantum.WeaponBigBoomSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponEquipSystem (original base: Quantum.SystemSignalsOnly)
class WeaponEquipSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be902c, Quantum.WeaponEquipSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7be9124, Quantum.WeaponEquipSystem.Weapon_OnEquip): Weapon_OnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7be9458, Quantum.WeaponEquipSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7be9618, Quantum.WeaponEquipSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WeaponFiringSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponFiringSystem : public SystemMainThread {
  public:
    // TODO(0x7be9620, Quantum.WeaponFiringSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7be9af8, Quantum.WeaponFiringSystem.UpdateReloading): UpdateReloading(frame, weapon, weaponData); not reimplemented yet.
    // TODO(0x7be9cd4, Quantum.WeaponFiringSystem.UpdateCooldown): UpdateCooldown(frame, weapon); not reimplemented yet.
    // TODO(0x7be9e10, Quantum.WeaponFiringSystem.CheckCooldown): CheckCooldown(frame, weaponEntity, weapon, weaponData); not reimplemented yet.
    // TODO(0x7be9e68, Quantum.WeaponFiringSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7be9f04, Quantum.WeaponFiringSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bea068, Quantum.WeaponFiringSystem..ctor): .ctor(); not reimplemented yet.
    // TODO(0x7be9d58, Quantum.WeaponFiringSystem.<Update>g__IsLedgeGrabbing|1_0): <Update>g__IsLedgeGrabbing|1_0(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponGatlingSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponGatlingSystem : public SystemMainThread {
  public:
    // TODO(0x7bf6f00, Quantum.WeaponGatlingSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf7168, Quantum.WeaponGatlingSystem.OnRemoved): OnRemoved(f, entity, gatlingWeapon); not reimplemented yet.
    // TODO(0x7bf71ec, Quantum.WeaponGatlingSystem.Weapon_OnCreated): Weapon_OnCreated(f, weaponEntity); not reimplemented yet.
    // TODO(0x7bf7558, Quantum.WeaponGatlingSystem.GadgetUser_OnGadgetActivated): GadgetUser_OnGadgetActivated(frame, entity); not reimplemented yet.
    // TODO(0x7bf7820, Quantum.WeaponGatlingSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponHoverBootsSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponHoverBootsSystem : public SystemMainThread {
  public:
    // TODO(0x7bf993c, Quantum.WeaponHoverBootsSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bf99e8, Quantum.WeaponHoverBootsSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bf9eb4, Quantum.WeaponHoverBootsSystem.StopHoverBoots): StopHoverBoots(f, weapon, hoverBoots, kccExtension); not reimplemented yet.
    // TODO(0x7bf9f34, Quantum.WeaponHoverBootsSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfa07c, Quantum.WeaponHoverBootsSystem.KccExtension_OnMovementPackComputed): KccExtension_OnMovementPackComputed(f, character, movementPack); not reimplemented yet.
    // TODO(0x7bfa4e8, Quantum.WeaponHoverBootsSystem.LedgeGrabbingActor_OnLedgeGrabStart): LedgeGrabbingActor_OnLedgeGrabStart(f, entity); not reimplemented yet.
    // TODO(0x7bfa678, Quantum.WeaponHoverBootsSystem.WallRunningActor_OnWallRunningStart): WallRunningActor_OnWallRunningStart(f, entity); not reimplemented yet.
    // TODO(0x7bfa808, Quantum.WeaponHoverBootsSystem.PlayerActor_OnInputLocked): PlayerActor_OnInputLocked(frame, entity, locked); not reimplemented yet.
    // TODO(0x7bfa974, Quantum.WeaponHoverBootsSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponInitSystem (original base: Quantum.SystemSignalsOnly)
class WeaponInitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bea0b0, Quantum.WeaponInitSystem.OnAdded): OnAdded(frame, entity, weapon); not reimplemented yet.
    // TODO(0x7bea5a0, Quantum.WeaponInitSystem.OnRemoved): OnRemoved(f, entity, weapon); not reimplemented yet.
    // TODO(0x7bea710, Quantum.WeaponInitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WeaponInputsSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponInputsSystem : public SystemMainThread {
  public:
    // TODO(0x7bea718, Quantum.WeaponInputsSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bea9a0, Quantum.WeaponInputsSystem.UpdateWeaponLookatPoint): UpdateWeaponLookatPoint(frame, filter, weaponFilter, weaponData); not reimplemented yet.
    // TODO(0x7beb4a8, Quantum.WeaponInputsSystem.UpdateWeaponLookatTransform): UpdateWeaponLookatTransform(frame, filter, weaponFilter); not reimplemented yet.
    // TODO(0x7beb7a0, Quantum.WeaponInputsSystem.FindBestLobAngleForDistance): FindBestLobAngleForDistance(projectileSpec, toTargetRangeRatio, height); not reimplemented yet.
    // TODO(0x7beba14, Quantum.WeaponInputsSystem.UpdateTriggerFromButtons): UpdateTriggerFromButtons(frame, weapon, weaponData, input); not reimplemented yet.
    // TODO(0x7bebbf8, Quantum.WeaponInputsSystem.IsInputBuffered): IsInputBuffered(frame, weapon, weaponData); not reimplemented yet.
    // TODO(0x7beb548, Quantum.WeaponInputsSystem.UpdateTriggerIsPressed): UpdateTriggerIsPressed(frame, filter, weaponFilter, weaponData); not reimplemented yet.
    // TODO(0x7bebc90, Quantum.WeaponInputsSystem.PlayerActor_OnInputLocked): PlayerActor_OnInputLocked(frame, entity, locked); not reimplemented yet.
    // TODO(0x7bebe40, Quantum.WeaponInputsSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponLoadoutSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponLoadoutSystem : public SystemMainThread {
  public:
    // TODO(0x7c03b4c, Quantum.WeaponLoadoutSystem.PlayerActor_OnInitialized): PlayerActor_OnInitialized(frame, actorEntity, actor); not reimplemented yet.
    // TODO(0x7c03ffc, Quantum.WeaponLoadoutSystem.ActorStatus_OnChanged): ActorStatus_OnChanged(frame, actorEntity); not reimplemented yet.
    // TODO(0x7c04258, Quantum.WeaponLoadoutSystem.get_Any): get_Any(); not reimplemented yet.
    // TODO(0x7c042e0, Quantum.WeaponLoadoutSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7c049c8, Quantum.WeaponLoadoutSystem.HandlePlayerCommands): HandlePlayerCommands(frame, entity, loadoutActor); not reimplemented yet.
    // TODO(0x7c04b14, Quantum.WeaponLoadoutSystem.OnAdded): OnAdded(frame, entity, gameModeHandler); not reimplemented yet.
    // TODO(0x7c045bc, Quantum.WeaponLoadoutSystem.HandleReloadWeaponRequested): HandleReloadWeaponRequested(f, filter); not reimplemented yet.
    // TODO(0x7c04630, Quantum.WeaponLoadoutSystem.HandleSwitchWeaponRequested): HandleSwitchWeaponRequested(frame, filter, nextSlot); not reimplemented yet.
    // TODO(0x7c04810, Quantum.WeaponLoadoutSystem.HandleUltimateAbilityButtonPressed): HandleUltimateAbilityButtonPressed(frame, filter); not reimplemented yet.
    // TODO(0x7c0495c, Quantum.WeaponLoadoutSystem.HandleUltimateAbilityButtonReleased): HandleUltimateAbilityButtonReleased(frame, filter); not reimplemented yet.
    // TODO(0x7c049c0, Quantum.WeaponLoadoutSystem.HandleGadgetWeaponButtonPressed): HandleGadgetWeaponButtonPressed(frame, filter); not reimplemented yet.
    // TODO(0x7c054ac, Quantum.WeaponLoadoutSystem.Killable_OnRevive): Killable_OnRevive(frame, entity, position, rotation); not reimplemented yet.
    // TODO(0x7c055a8, Quantum.WeaponLoadoutSystem.GameMode_OnStateChanged): GameMode_OnStateChanged(f, prevState, newState); not reimplemented yet.
    // TODO(0x7c0590c, Quantum.WeaponLoadoutSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponLobAutoTargetSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponLobAutoTargetSystem : public SystemMainThread {
  public:
    // TODO(0x7be7f24, Quantum.WeaponLobAutoTargetSystem.get_Without): get_Without(); not reimplemented yet.
    // TODO(0x7be7fac, Quantum.WeaponLobAutoTargetSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7be8578, Quantum.WeaponLobAutoTargetSystem.GetCurveHit): GetCurveHit(f, filter, weaponTransform, weapon, projectileSpec); not reimplemented yet.
    // TODO(0x7be8658, Quantum.WeaponLobAutoTargetSystem.GetLobCurveHit): GetLobCurveHit(f, filter, weaponTransform, weapon, projectileSpec); not reimplemented yet.
    // TODO(0x7be8c7c, Quantum.WeaponLobAutoTargetSystem.GetLobFixedFlightTimeCurveHit): GetLobFixedFlightTimeCurveHit(f, filter, weaponTransform, weapon, projectileSpec); not reimplemented yet.
    // TODO(0x7be8fe4, Quantum.WeaponLobAutoTargetSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponLobEquipSystem (original base: Quantum.SystemSignalsOnly)
class WeaponLobEquipSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7be7d18, Quantum.WeaponLobEquipSystem.Weapon_OnEquip): Weapon_OnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7be7f1c, Quantum.WeaponLobEquipSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WeaponMovementModifierSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponMovementModifierSystem : public SystemMainThread {
  public:
    // TODO(0x7bebe88, Quantum.WeaponMovementModifierSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bec038, Quantum.WeaponMovementModifierSystem.UpdateUsageModifiers): UpdateUsageModifiers(frame, filter, weaponFilter, weaponData); not reimplemented yet.
    // TODO(0x7bec36c, Quantum.WeaponMovementModifierSystem.Weapon_OnUnEquip): Weapon_OnUnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bec40c, Quantum.WeaponMovementModifierSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponProjectileSwitcherEnhancementSystem (original base: Quantum.SystemSignalsOnly)
class WeaponProjectileSwitcherEnhancementSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7bee450, Quantum.WeaponProjectileSwitcherEnhancementSystem.Weapon_OnEquip): Weapon_OnEquip(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bee720, Quantum.WeaponProjectileSwitcherEnhancementSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bee4bc, Quantum.WeaponProjectileSwitcherEnhancementSystem.TryGetData): TryGetData(f, weaponEntity, enhancement, enhancementSpec, weapon); not reimplemented yet.
    // TODO(0x7bee614, Quantum.WeaponProjectileSwitcherEnhancementSystem.SetOriginalAmmoAsset): SetOriginalAmmoAsset(enhancement, weapon); not reimplemented yet.
    // TODO(0x7bee668, Quantum.WeaponProjectileSwitcherEnhancementSystem.SetWeaponAmmunitionAsset): SetWeaponAmmunitionAsset(frame, enhancement, enhancementSpec, weapon, forceRefresh); not reimplemented yet.
    // TODO(0x7bee780, Quantum.WeaponProjectileSwitcherEnhancementSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WeaponProjectileSwitcherEnhancementUpdateSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponProjectileSwitcherEnhancementUpdateSystem : public SystemMainThread {
  public:
    // 0x7bee1dc
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bee1e4, Quantum.WeaponProjectileSwitcherEnhancementUpdateSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bee408, Quantum.WeaponProjectileSwitcherEnhancementUpdateSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponRynoSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponRynoSystem : public SystemMainThread {
  public:
    // TODO(0x7bfb3ec, Quantum.WeaponRynoSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bfbf1c, Quantum.WeaponRynoSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponShowcaseGameModeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponShowcaseGameModeSystem : public SystemMainThread {
  public:
    // 0x7bae550
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bae558, Quantum.WeaponShowcaseGameModeSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bae560, Quantum.WeaponShowcaseGameModeSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7baf068, Quantum.WeaponShowcaseGameModeSystem.MoveToShootAngle): MoveToShootAngle(f, gameModeSpec, gameMode); not reimplemented yet.
    // TODO(0x7baf2dc, Quantum.WeaponShowcaseGameModeSystem.MoveBackToSpawn): MoveBackToSpawn(f, gameModeSpec, gameMode); not reimplemented yet.
    // TODO(0x7baee54, Quantum.WeaponShowcaseGameModeSystem.Cheat_MoveWeaponManually): Cheat_MoveWeaponManually(f, gameModeSpec, gameMode); not reimplemented yet.
    // TODO(0x7ba7a0c, Quantum.WeaponShowcaseGameModeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponTeslaClawSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponTeslaClawSystem : public SystemMainThread {
  public:
    // TODO(0x7bfd360, Quantum.WeaponTeslaClawSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bfd484, Quantum.WeaponTeslaClawSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponTriggerSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponTriggerSystem : public SystemMainThread {
  public:
    // TODO(0x7bec454, Quantum.WeaponTriggerSystem.Update): Update(frame, filter); not reimplemented yet.
    // TODO(0x7bec71c, Quantum.WeaponTriggerSystem.ActorStatus_OnChanged): ActorStatus_OnChanged(f, actorEntity); not reimplemented yet.
    // TODO(0x7bec80c, Quantum.WeaponTriggerSystem.Killable_OnKill): Killable_OnKill(f, toKill, killedBy); not reimplemented yet.
    // TODO(0x7bec720, Quantum.WeaponTriggerSystem.TryToResetWeaponDefaultState): TryToResetWeaponDefaultState(f, actorEntity); not reimplemented yet.
    // TODO(0x7bec880, Quantum.WeaponTriggerSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WeaponUserInitSystem (original base: Quantum.SystemSignalsOnly)
class WeaponUserInitSystem : public SystemSignalsOnly {
  public:
    // TODO(0x7c0e534, Quantum.WeaponUserInitSystem.OnAdded): OnAdded(frame, entity, weaponUser); not reimplemented yet.
    // TODO(0x7c0e9cc, Quantum.WeaponUserInitSystem.OnRemoved): OnRemoved(frame, entity, weaponUser); not reimplemented yet.
    // TODO(0x7c0ea58, Quantum.WeaponUserInitSystem..ctor): .ctor(); not reimplemented yet.
};

// Quantum.WeaponWrenchSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WeaponWrenchSystem : public SystemMainThread {
  public:
    // TODO(0x7bfec60, Quantum.WeaponWrenchSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7bfed0c, Quantum.WeaponWrenchSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bff6e8, Quantum.WeaponWrenchSystem.Weapon_OnBeforeUse): Weapon_OnBeforeUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7bfff6c, Quantum.WeaponWrenchSystem.Pushable_OnPushed): Pushable_OnPushed(f, pushData, onPushedData); not reimplemented yet.
    // TODO(0x7c00068, Quantum.WeaponWrenchSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.WrenchEnhancementSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class WrenchEnhancementSystem : public SystemMainThread {
  public:
    // TODO(0x7c001ac, Quantum.WrenchEnhancementSystem.OnInit): OnInit(f); not reimplemented yet.
    // TODO(0x7c00258, Quantum.WrenchEnhancementSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7c00650, Quantum.WrenchEnhancementSystem.Weapon_OnBeforeUse): Weapon_OnBeforeUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7c00660, Quantum.WrenchEnhancementSystem.SelectGroundSlamAmmo): SelectGroundSlamAmmo(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7c008f4, Quantum.WrenchEnhancementSystem.Weapon_OnUse): Weapon_OnUse(f, userEntity, weaponEntity); not reimplemented yet.
    // TODO(0x7c009e8, Quantum.WrenchEnhancementSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7c00c30, Quantum.WrenchEnhancementSystem.GroundDashActor_OnDashEnded): GroundDashActor_OnDashEnded(frame, userEntity, endReason); not reimplemented yet.
    // TODO(0x7c00e14, Quantum.WrenchEnhancementSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};

// Quantum.ZoneControlGameModeSystem (original base: Quantum.SystemMainThreadFilter<Filter>)
class ZoneControlGameModeSystem : public SystemMainThread {
  public:
    // 0x7bafce8
    bool get_StartEnabled() const override { return false; }
    // TODO(0x7bafcf0, Quantum.ZoneControlGameModeSystem.Update): Update(f, filter); not reimplemented yet.
    // TODO(0x7bb07dc, Quantum.ZoneControlGameModeSystem.OnAdded): OnAdded(f, entity, component); not reimplemented yet.
    // TODO(0x7bb0ac0, Quantum.ZoneControlGameModeSystem.OnRemoved): OnRemoved(f, entity, component); not reimplemented yet.
    // TODO(0x7bb0038, Quantum.ZoneControlGameModeSystem.GameStarted): GameStarted(f); not reimplemented yet.
    // TODO(0x7bb00d0, Quantum.ZoneControlGameModeSystem.TrySetupNextControlZone): TrySetupNextControlZone(f, controlZoneStateHolderList, pGameMode); not reimplemented yet.
    // TODO(0x7bb02fc, Quantum.ZoneControlGameModeSystem.ControlZoneIsLoading): ControlZoneIsLoading(f, gameModeSpec, controlZoneState, gameMode); not reimplemented yet.
    // TODO(0x7bb03a8, Quantum.ZoneControlGameModeSystem.ComputeActiveControlZoneState): ComputeActiveControlZoneState(f, gameModeSpec, mapCustomData, controlZoneState); not reimplemented yet.
    // TODO(0x7bb0b34, Quantum.ZoneControlGameModeSystem.CalculateZoneOccupancy): CalculateZoneOccupancy(f, controlZoneData, entitiesInsideTeamA, entitiesInsideTeamB); not reimplemented yet.
    // TODO(0x7bb0614, Quantum.ZoneControlGameModeSystem.ProcessControlZoneScoring): ProcessControlZoneScoring(f, pGameMode, gameModeSpec, controlZoneState); not reimplemented yet.
    // TODO(0x7bb0e44, Quantum.ZoneControlGameModeSystem.AwardOneVictoryPoint): AwardOneVictoryPoint(f, pGameMode, gameModeSpec, controlZoneState); not reimplemented yet.
    // TODO(0x7ba79bc, Quantum.ZoneControlGameModeSystem..ctor): .ctor(); not reimplemented yet.
    void Update(Frame* f) override { (void)f; }
};
} // namespace Quantum
