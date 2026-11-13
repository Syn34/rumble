// frame_signals.cpp - Quantum.Frame.FrameSignals.
#include "quantum_code/frame_signals.h"

#include "quantum_code/frame.h"

namespace Quantum {

void FrameSignals::AIMoveForward_OnBounce(EntityRef bouncer) {
    for (const auto& system : _f->_ISignalAIMoveForward_OnBounceSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AIMoveForward_OnBounce(_f, bouncer);
        }
    }
}

void FrameSignals::AISpawner_OnSpawned(EntityRef spawner, EntityRef spawned) {
    for (const auto& system : _f->_ISignalAISpawner_OnSpawnedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AISpawner_OnSpawned(_f, spawner, spawned);
        }
    }
}

void FrameSignals::AISpawner_OnSpawnDone(EntityRef spawner) {
    for (const auto& system : _f->_ISignalAISpawner_OnSpawnDoneSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AISpawner_OnSpawnDone(_f, spawner);
        }
    }
}

void FrameSignals::PlayerActor_OnInputLocked(EntityRef entity, QBoolean locked) {
    for (const auto& system : _f->_ISignalPlayerActor_OnInputLockedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->PlayerActor_OnInputLocked(_f, entity, locked);
        }
    }
}

void FrameSignals::ActorStatus_OnChanged(EntityRef actorEntity) {
    for (const auto& system : _f->_ISignalActorStatus_OnChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->ActorStatus_OnChanged(_f, actorEntity);
        }
    }
}

void FrameSignals::AreaOfEffect_OnCalculateDynamicShape(EntityRef areaOfEffectEntity, Shape3D* outShape) {
    for (const auto& system : _f->_ISignalAreaOfEffect_OnCalculateDynamicShapeSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AreaOfEffect_OnCalculateDynamicShape(_f, areaOfEffectEntity, outShape);
        }
    }
}

void FrameSignals::AreaOfEffect_OnHit(EntityRef areaOfEffectEntity) {
    for (const auto& system : _f->_ISignalAreaOfEffect_OnHitSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AreaOfEffect_OnHit(_f, areaOfEffectEntity);
        }
    }
}

void FrameSignals::AreaOfEffect_OnMiss(EntityRef areaOfEffectEntity) {
    for (const auto& system : _f->_ISignalAreaOfEffect_OnMissSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AreaOfEffect_OnMiss(_f, areaOfEffectEntity);
        }
    }
}

void FrameSignals::AutoJumperActor_OnStartJump(EntityRef entity) {
    for (const auto& system : _f->_ISignalAutoJumperActor_OnStartJumpSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AutoJumperActor_OnStartJump(_f, entity);
        }
    }
}

void FrameSignals::AutoJumperActor_OnEndJump(EntityRef entity) {
    for (const auto& system : _f->_ISignalAutoJumperActor_OnEndJumpSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->AutoJumperActor_OnEndJump(_f, entity);
        }
    }
}

void FrameSignals::BlastBallGame_StateChanged(BlastBallGameState newState) {
    for (const auto& system : _f->_ISignalBlastBallGame_StateChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->BlastBallGame_StateChanged(_f, newState);
        }
    }
}

void FrameSignals::BlastBallGame_LeadChanged() {
    for (const auto& system : _f->_ISignalBlastBallGame_LeadChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->BlastBallGame_LeadChanged(_f);
        }
    }
}

void FrameSignals::Bolt_OnCollected(EntityRef collector, std::int32_t amount) {
    for (const auto& system : _f->_ISignalBolt_OnCollectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Bolt_OnCollected(_f, collector, amount);
        }
    }
}

void FrameSignals::Bolt_OnLost(EntityRef collector, std::int32_t amount) {
    for (const auto& system : _f->_ISignalBolt_OnLostSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Bolt_OnLost(_f, collector, amount);
        }
    }
}

void FrameSignals::Damageable_OnBeforeHit(EntityRef targetEntity, HitData* hitData) {
    for (const auto& system : _f->_ISignalDamageable_OnBeforeHitSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Damageable_OnBeforeHit(_f, targetEntity, hitData);
        }
    }
}

void FrameSignals::Damageable_HitReceived(EntityRef entity, const HitData& hitData, const HitResult& hitResult) {
    for (const auto& system : _f->_ISignalDamageable_HitReceivedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Damageable_HitReceived(_f, entity, hitData, hitResult);
        }
    }
}

void FrameSignals::DamageToImpulseConverter_ComputeImpulse(EntityRef entity, const HitData& hitData, DamageToImpulseData* impulseData) {
    for (const auto& system : _f->_ISignalDamageToImpulseConverter_ComputeImpulseSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DamageToImpulseConverter_ComputeImpulse(_f, entity, hitData, impulseData);
        }
    }
}

void FrameSignals::DamageToImpulseConverter_OnBeforeImpulse(EntityRef entity, const DamageToImpulseData& impulseData) {
    for (const auto& system : _f->_ISignalDamageToImpulseConverter_OnBeforeImpulseSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DamageToImpulseConverter_OnBeforeImpulse(_f, entity, impulseData);
        }
    }
}

void FrameSignals::DamageToImpulseConverter_OnAfterImpulse(EntityRef entity) {
    for (const auto& system : _f->_ISignalDamageToImpulseConverter_OnAfterImpulseSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DamageToImpulseConverter_OnAfterImpulse(_f, entity);
        }
    }
}

void FrameSignals::DashActor_OnBeforeDash(EntityRef entity, FPVector3* dashDir) {
    for (const auto& system : _f->_ISignalDashActor_OnBeforeDashSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DashActor_OnBeforeDash(_f, entity, dashDir);
        }
    }
}

void FrameSignals::DashActor_OnDashed(EntityRef entity) {
    for (const auto& system : _f->_ISignalDashActor_OnDashedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DashActor_OnDashed(_f, entity);
        }
    }
}

void FrameSignals::DashActor_OnDashIsReady(EntityRef entity) {
    for (const auto& system : _f->_ISignalDashActor_OnDashIsReadySystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DashActor_OnDashIsReady(_f, entity);
        }
    }
}

void FrameSignals::DashActor_OnAfterDash(EntityRef entity) {
    for (const auto& system : _f->_ISignalDashActor_OnAfterDashSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DashActor_OnAfterDash(_f, entity);
        }
    }
}

void FrameSignals::DrillDash_OnStateChanged(EntityRef entity, DrillDash* pDrillDash, DrillDashState newState) {
    for (const auto& system : _f->_ISignalDrillDash_OnStateChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->DrillDash_OnStateChanged(_f, entity, pDrillDash, newState);
        }
    }
}

void FrameSignals::Droppable_OnDestroyed(const Droppable& droppable) {
    for (const auto& system : _f->_ISignalDroppable_OnDestroyedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Droppable_OnDestroyed(_f, droppable);
        }
    }
}

void FrameSignals::GadgetUser_OnGadgetActivated(EntityRef entity) {
    for (const auto& system : _f->_ISignalGadgetUser_OnGadgetActivatedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GadgetUser_OnGadgetActivated(_f, entity);
        }
    }
}

void FrameSignals::GadgetUser_OnGadgetDeactivated(EntityRef entity) {
    for (const auto& system : _f->_ISignalGadgetUser_OnGadgetDeactivatedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GadgetUser_OnGadgetDeactivated(_f, entity);
        }
    }
}

void FrameSignals::GameMode_OnMainLoopStart(EntityRef gameMode) {
    for (const auto& system : _f->_ISignalGameMode_OnMainLoopStartSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GameMode_OnMainLoopStart(_f, gameMode);
        }
    }
}

void FrameSignals::GameMode_OnStateChanged(GAME_MODE_HANDLER_STATE prevState, GAME_MODE_HANDLER_STATE newState) {
    for (const auto& system : _f->_ISignalGameMode_OnStateChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GameMode_OnStateChanged(_f, prevState, newState);
        }
    }
}

void FrameSignals::GameMode_OnPlayerEliminated(EntityRef entity) {
    for (const auto& system : _f->_ISignalGameMode_OnPlayerEliminatedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GameMode_OnPlayerEliminated(_f, entity);
        }
    }
}

void FrameSignals::GameMode_OnPlayerFinished(EntityRef entity) {
    for (const auto& system : _f->_ISignalGameMode_OnPlayerFinishedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GameMode_OnPlayerFinished(_f, entity);
        }
    }
}

void FrameSignals::Bolt_OnGoldenBoltCollected(EntityRef collector) {
    for (const auto& system : _f->_ISignalBolt_OnGoldenBoltCollectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Bolt_OnGoldenBoltCollected(_f, collector);
        }
    }
}

void FrameSignals::GrinderActor_OnExitGrind(EntityRef entity, FP grindingDuration) {
    for (const auto& system : _f->_ISignalGrinderActor_OnExitGrindSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GrinderActor_OnExitGrind(_f, entity, grindingDuration);
        }
    }
}

void FrameSignals::GroundDashActor_OnDashStarting(EntityRef userEntity) {
    for (const auto& system : _f->_ISignalGroundDashActor_OnDashStartingSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GroundDashActor_OnDashStarting(_f, userEntity);
        }
    }
}

void FrameSignals::GroundDashActor_OnDashEnded(EntityRef userEntity, GroundDashEndReason endReason) {
    for (const auto& system : _f->_ISignalGroundDashActor_OnDashEndedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->GroundDashActor_OnDashEnded(_f, userEntity, endReason);
        }
    }
}

void FrameSignals::Health_OnHealthDecreased(EntityRef entity, FP previousHP, FP currentHP, FP maxHP) {
    for (const auto& system : _f->_ISignalHealth_OnHealthDecreasedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Health_OnHealthDecreased(_f, entity, previousHP, currentHP, maxHP);
        }
    }
}

void FrameSignals::KccExtension_OnFilterCollision3D(EntityRef character, const Physics3D::Hit3D& hit, bool* ignoreHit) {
    for (const auto& system : _f->_ISignalKccExtension_OnFilterCollision3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnFilterCollision3D(_f, character, hit, ignoreHit);
        }
    }
}

void FrameSignals::KccExtension_OnStaticWallCollision3D(EntityRef character, const Physics3D::Hit3D& hit) {
    for (const auto& system : _f->_ISignalKccExtension_OnStaticWallCollision3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnStaticWallCollision3D(_f, character, hit);
        }
    }
}

void FrameSignals::KccExtension_OnCharacterCollision3D(EntityRef character, const Physics3D::Hit3D& hit) {
    for (const auto& system : _f->_ISignalKccExtension_OnCharacterCollision3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnCharacterCollision3D(_f, character, hit);
        }
    }
}

void FrameSignals::KccExtension_OnMovementPackComputed(EntityRef character, CharacterController3DMovementExt* movementPack) {
    for (const auto& system : _f->_ISignalKccExtension_OnMovementPackComputedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnMovementPackComputed(_f, character, movementPack);
        }
    }
}

void FrameSignals::KccExtension_OnValidLedgeDetected(EntityRef character, const Physics3D::Hit3D& hit, const FPVector3& AboveLedgePoint) {
    for (const auto& system : _f->_ISignalKccExtension_OnValidLedgeDetectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnValidLedgeDetected(_f, character, hit, AboveLedgePoint);
        }
    }
}

void FrameSignals::KccExtension_OnConfigChanged(EntityRef entity, KccExtension* kccExt) {
    for (const auto& system : _f->_ISignalKccExtension_OnConfigChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->KccExtension_OnConfigChanged(_f, entity, kccExt);
        }
    }
}

void FrameSignals::Killable_OnKill(EntityRef toKill, const KilledByData& killedBy) {
    for (const auto& system : _f->_ISignalKillable_OnKillSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Killable_OnKill(_f, toKill, killedBy);
        }
    }
}

void FrameSignals::Killable_OnRevive(EntityRef toRevive, const FPVector3& position, const FPQuaternion& rotation) {
    for (const auto& system : _f->_ISignalKillable_OnReviveSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Killable_OnRevive(_f, toRevive, position, rotation);
        }
    }
}

void FrameSignals::LedgeGrabbingActor_OnLedgeGrabStart(EntityRef entity) {
    for (const auto& system : _f->_ISignalLedgeGrabbingActor_OnLedgeGrabStartSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->LedgeGrabbingActor_OnLedgeGrabStart(_f, entity);
        }
    }
}

void FrameSignals::Lifetime_OnSpawnEntityOnDeath(const OnSpawnEntityOnDeathParams& spawnParams) {
    for (const auto& system : _f->_ISignalLifetime_OnSpawnEntityOnDeathSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Lifetime_OnSpawnEntityOnDeath(_f, spawnParams);
        }
    }
}

void FrameSignals::MobGroup_OnAllMembersDied(EntityRef groupEntity) {
    for (const auto& system : _f->_ISignalMobGroup_OnAllMembersDiedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->MobGroup_OnAllMembersDied(_f, groupEntity);
        }
    }
}

void FrameSignals::MoveTo_OnMoveStarted(EntityRef entity) {
    for (const auto& system : _f->_ISignalMoveTo_OnMoveStartedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->MoveTo_OnMoveStarted(_f, entity);
        }
    }
}

void FrameSignals::MoveTo_OnMoveFinished(EntityRef entity) {
    for (const auto& system : _f->_ISignalMoveTo_OnMoveFinishedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->MoveTo_OnMoveFinished(_f, entity);
        }
    }
}

void FrameSignals::ParabolaFollower_OnRemove(EntityRef entity, REMOVE_REASON reason) {
    for (const auto& system : _f->_ISignalParabolaFollower_OnRemoveSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->ParabolaFollower_OnRemove(_f, entity, reason);
        }
    }
}

void FrameSignals::ParabolaFollower_OnReachedEnd(EntityRef entity) {
    for (const auto& system : _f->_ISignalParabolaFollower_OnReachedEndSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->ParabolaFollower_OnReachedEnd(_f, entity);
        }
    }
}

void FrameSignals::PlayerActor_OnInitialized(EntityRef actorEntity, PlayerActor* actor) {
    for (const auto& system : _f->_ISignalPlayerActor_OnInitializedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->PlayerActor_OnInitialized(_f, actorEntity, actor);
        }
    }
}

void FrameSignals::PlayerActor_OnAfterInitialized(EntityRef actorEntity, PlayerActor* actor) {
    for (const auto& system : _f->_ISignalPlayerActor_OnAfterInitializedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->PlayerActor_OnAfterInitialized(_f, actorEntity, actor);
        }
    }
}

void FrameSignals::PowerUp_OnCollected(EntityRef collector, PowerUpType type, CollectionSource source) {
    for (const auto& system : _f->_ISignalPowerUp_OnCollectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->PowerUp_OnCollected(_f, collector, type, source);
        }
    }
}

void FrameSignals::Projectile_OnSpawnEntityOnDeath(EntityRef spawnedEntity, EntityRef projectileEntity, ProjectileHitInfo* hitInfo) {
    for (const auto& system : _f->_ISignalProjectile_OnSpawnEntityOnDeathSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Projectile_OnSpawnEntityOnDeath(_f, spawnedEntity, projectileEntity, hitInfo);
        }
    }
}

void FrameSignals::Pushable_OnAddPush(Pushable* pushable, PushData* pushParams) {
    for (const auto& system : _f->_ISignalPushable_OnAddPushSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Pushable_OnAddPush(_f, pushable, pushParams);
        }
    }
}

void FrameSignals::Pushable_OnBeforePush(EntityRef entity, PushData* pushParams) {
    for (const auto& system : _f->_ISignalPushable_OnBeforePushSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Pushable_OnBeforePush(_f, entity, pushParams);
        }
    }
}

void FrameSignals::Pushable_OnPushed(const PushData& pushParams, const OnPushedData& onPushedData) {
    for (const auto& system : _f->_ISignalPushable_OnPushedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Pushable_OnPushed(_f, pushParams, onPushedData);
        }
    }
}

void FrameSignals::Pushable_OnPushStopEnd(EntityRef entity, const PushData& pushParams) {
    for (const auto& system : _f->_ISignalPushable_OnPushStopEndSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Pushable_OnPushStopEnd(_f, entity, pushParams);
        }
    }
}

void FrameSignals::Ranking_OnLeaderChanged() {
    for (const auto& system : _f->_ISignalRanking_OnLeaderChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Ranking_OnLeaderChanged(_f);
        }
    }
}

void FrameSignals::Reward_OnCollected(EntityRef collectorEntity, DroppableType type, std::uint16_t amount) {
    for (const auto& system : _f->_ISignalReward_OnCollectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Reward_OnCollected(_f, collectorEntity, type, amount);
        }
    }
}

void FrameSignals::Shield_OnBreaks(EntityRef entity, EntityRef sourceEntity, const FPVector3& direction) {
    for (const auto& system : _f->_ISignalShield_OnBreaksSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Shield_OnBreaks(_f, entity, sourceEntity, direction);
        }
    }
}

void FrameSignals::Shield_OnStartRegen(EntityRef entity) {
    for (const auto& system : _f->_ISignalShield_OnStartRegenSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Shield_OnStartRegen(_f, entity);
        }
    }
}

void FrameSignals::Shield_OnFull(EntityRef entity) {
    for (const auto& system : _f->_ISignalShield_OnFullSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Shield_OnFull(_f, entity);
        }
    }
}

void FrameSignals::TargetedEffect_OnTick(EntityRef targetedEffectEntity, EntityRef targetEntity) {
    for (const auto& system : _f->_ISignalTargetedEffect_OnTickSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->TargetedEffect_OnTick(_f, targetedEffectEntity, targetEntity);
        }
    }
}

void FrameSignals::TargetedEffect_OnTargetSet(EntityRef targetedEffectEntity, EntityRef targetEntity) {
    for (const auto& system : _f->_ISignalTargetedEffect_OnTargetSetSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->TargetedEffect_OnTargetSet(_f, targetedEffectEntity, targetEntity);
        }
    }
}

void FrameSignals::UltimateAbilityUser_OnStateChanged(EntityRef entity, UltimateAbilityState state) {
    for (const auto& system : _f->_ISignalUltimateAbilityUser_OnStateChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->UltimateAbilityUser_OnStateChanged(_f, entity, state);
        }
    }
}

void FrameSignals::UltimateAbilityUser_OnRecast(EntityRef entity) {
    for (const auto& system : _f->_ISignalUltimateAbilityUser_OnRecastSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->UltimateAbilityUser_OnRecast(_f, entity);
        }
    }
}

void FrameSignals::OnFeatureUnlocked(EntityRef entity, UnlockedFeature unlockedFeature) {
    for (const auto& system : _f->_ISignalOnFeatureUnlockedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnFeatureUnlocked(_f, entity, unlockedFeature);
        }
    }
}

void FrameSignals::OnFeatureLocked(EntityRef entity, UnlockedFeature lockedFeature) {
    for (const auto& system : _f->_ISignalOnFeatureLockedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnFeatureLocked(_f, entity, lockedFeature);
        }
    }
}

void FrameSignals::OnVoidRepulserShieldStateChanged(EntityRef entity, VoidRepulserState state) {
    for (const auto& system : _f->_ISignalOnVoidRepulserShieldStateChangedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnVoidRepulserShieldStateChanged(_f, entity, state);
        }
    }
}

void FrameSignals::WallRunningActor_OnWallRunningStart(EntityRef entity) {
    for (const auto& system : _f->_ISignalWallRunningActor_OnWallRunningStartSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->WallRunningActor_OnWallRunningStart(_f, entity);
        }
    }
}

void FrameSignals::WaveSpawner_OnSpawned(EntityRef spawned, WaveSpawner* spawner) {
    for (const auto& system : _f->_ISignalWaveSpawner_OnSpawnedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->WaveSpawner_OnSpawned(_f, spawned, spawner);
        }
    }
}

void FrameSignals::Weapon_OnCreated(EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnCreatedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnCreated(_f, weaponEntity);
        }
    }
}

void FrameSignals::Weapon_OnReload(EntityRef weaponEntity, const Weapon& weapon) {
    for (const auto& system : _f->_ISignalWeapon_OnReloadSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnReload(_f, weaponEntity, weapon);
        }
    }
}

void FrameSignals::Weapon_OnShoot(EntityRef weaponEntity, EntityRef ammunitionEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnShootSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnShoot(_f, weaponEntity, ammunitionEntity);
        }
    }
}

void FrameSignals::WeaponLoadoutActor_OnManualSwap(EntityRef loadoutActor, WeaponLoadoutSlot slot) {
    for (const auto& system : _f->_ISignalWeaponLoadoutActor_OnManualSwapSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->WeaponLoadoutActor_OnManualSwap(_f, loadoutActor, slot);
        }
    }
}

void FrameSignals::Weapon_OnBeforeEquip(EntityRef userEntity, EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnBeforeEquipSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnBeforeEquip(_f, userEntity, weaponEntity);
        }
    }
}

void FrameSignals::Weapon_OnEquip(EntityRef userEntity, EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnEquipSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnEquip(_f, userEntity, weaponEntity);
        }
    }
}

void FrameSignals::Weapon_OnUnEquip(EntityRef userEntity, EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnUnEquipSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnUnEquip(_f, userEntity, weaponEntity);
        }
    }
}

void FrameSignals::Weapon_OnBeforeUse(EntityRef userEntity, EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnBeforeUseSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnBeforeUse(_f, userEntity, weaponEntity);
        }
    }
}

void FrameSignals::Weapon_OnUse(EntityRef userEntity, EntityRef weaponEntity) {
    for (const auto& system : _f->_ISignalWeapon_OnUseSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->Weapon_OnUse(_f, userEntity, weaponEntity);
        }
    }
}

void FrameSignals::OnPlayerDataSet(PlayerRef player) {
    for (const auto& system : _f->_ISignalOnPlayerDataSet) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnPlayerDataSet(_f, player);
        }
    }
}

void FrameSignals::OnEntityPrototypeMaterialized(EntityRef entity, const EntityPrototypeRef& prototypeRef) {
    for (const auto& system : _f->_ISignalOnEntityPrototypeMaterializedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnEntityPrototypeMaterialized(_f, entity, prototypeRef);
        }
    }
}

void FrameSignals::OnPlayerConnected(PlayerRef player) {
    for (const auto& system : _f->_ISignalOnPlayerConnectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnPlayerConnected(_f, player);
        }
    }
}

void FrameSignals::OnPlayerDisconnected(PlayerRef player) {
    for (const auto& system : _f->_ISignalOnPlayerDisconnectedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnPlayerDisconnected(_f, player);
        }
    }
}

void FrameSignals::OnNavMeshWaypointReached(EntityRef entity, const FPVector3& waypoint, WaypointFlag waypointFlags, bool& resetAgent) {
    for (const auto& system : _f->_ISignalOnNavMeshWaypointReachedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnNavMeshWaypointReached(_f, entity, waypoint, waypointFlags, resetAgent);
        }
    }
}

void FrameSignals::OnNavMeshSearchFailed(EntityRef entity, bool& resetAgent) {
    for (const auto& system : _f->_ISignalOnNavMeshSearchFailedSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnNavMeshSearchFailed(_f, entity, resetAgent);
        }
    }
}

void FrameSignals::OnNavMeshMoveAgent(EntityRef entity, const FPVector2& desiredDirection) {
    for (const auto& system : _f->_ISignalOnNavMeshMoveAgentSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnNavMeshMoveAgent(_f, entity, desiredDirection);
        }
    }
}

void FrameSignals::OnCollision3D(const CollisionInfo3D& info) {
    for (const auto& system : _f->_ISignalOnCollision3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnCollision3D(_f, info);
        }
    }
}

void FrameSignals::OnCollisionEnter3D(const CollisionInfo3D& info) {
    for (const auto& system : _f->_ISignalOnCollisionEnter3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnCollisionEnter3D(_f, info);
        }
    }
}

void FrameSignals::OnCollisionExit3D(const ExitInfo3D& info) {
    for (const auto& system : _f->_ISignalOnCollisionExit3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnCollisionExit3D(_f, info);
        }
    }
}

void FrameSignals::OnTrigger3D(const TriggerInfo3D& info) {
    for (const auto& system : _f->_ISignalOnTrigger3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnTrigger3D(_f, info);
        }
    }
}

void FrameSignals::OnTriggerEnter3D(const TriggerInfo3D& info) {
    for (const auto& system : _f->_ISignalOnTriggerEnter3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnTriggerEnter3D(_f, info);
        }
    }
}

void FrameSignals::OnTriggerExit3D(const ExitInfo3D& info) {
    for (const auto& system : _f->_ISignalOnTriggerExit3DSystems) {
        if (_f->SystemIsEnabledInHierarchy(system.System)) {
            system.Signal->OnTriggerExit3D(_f, info);
        }
    }
}

} // namespace Quantum
