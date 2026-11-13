// components_3.cpp - GetHashCode / Serialize / collection lifetime of the types in
// components.h (split over several files only to keep build times down).
#include "quantum_code/components.h"

#include "quantum_code/frame.h"

namespace Quantum {

std::int32_t MovementPackCache::GetHashCode() const {
    return HashBuilder(3089u)
        .Add(slopeTangent)
        .Add(tangent)
        .Add(updateFrequency)
        .Value();
}

void MovementPackCache::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MovementPackCache*>(ptr);
    SerializeField(&p->updateFrequency, serializer);
    SerializeField(&p->slopeTangent, serializer);
    SerializeField(&p->tangent, serializer);
}

std::int32_t MovingWall::GetHashCode() const {
    return HashBuilder(3109u)
        .Add(Config)
        .Add(initialPosition)
        .Add(navmeshRegionID)
        .Add(offsetIndex)
        .Add(stopped)
        .Add(stoppedFrame)
        .Value();
}

void MovingWall::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MovingWall*>(ptr);
    SerializeField(&p->navmeshRegionID, serializer);
    SerializeField(&p->offsetIndex, serializer);
    SerializeField(&p->stoppedFrame, serializer);
    SerializeField(&p->stopped, serializer);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->initialPosition, serializer);
}

std::int32_t OnKillCallbacks::GetHashCode() const {
    return HashBuilder(3119u)
        .Add(onPlayerKilledPtr)
        .Value();
}

void OnKillCallbacks::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<OnKillCallbacks*>(ptr);
    SerializeList<AssetRefCallbackAction>(&p->onPlayerKilledPtr, serializer);
}

void OnKillCallbacks::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onPlayerKilledPtr == Ptr::Null()) {
        onPlayerKilledPtr = AllocateListPtr<AssetRefCallbackAction>(f, 8);
    }
}

void OnKillCallbacks::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onPlayerKilledPtr != Ptr::Null()) {
        f->FreeList(get_onPlayerKilled());
    }
    onPlayerKilledPtr = Ptr::Null();
}

void OnKillCallbacks::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<OnKillCallbacks*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void OnKillCallbacks::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<OnKillCallbacks*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t OnTriggerEnter::GetHashCode() const {
    return HashBuilder(3121u)
        .Add(ActivationFrame)
        .Add(Prototype)
        .Add(onTriggerEnterPtr)
        .Value();
}

void OnTriggerEnter::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<OnTriggerEnter*>(ptr);
    SerializeField(&p->ActivationFrame, serializer);
    SerializeList<AssetRefCallbackAction>(&p->onTriggerEnterPtr, serializer);
    SerializeField(&p->Prototype, serializer);
}

void OnTriggerEnter::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onTriggerEnterPtr == Ptr::Null()) {
        onTriggerEnterPtr = AllocateListPtr<AssetRefCallbackAction>(f, 8);
    }
}

void OnTriggerEnter::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onTriggerEnterPtr != Ptr::Null()) {
        f->FreeList(get_onTriggerEnter());
    }
    onTriggerEnterPtr = Ptr::Null();
}

void OnTriggerEnter::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<OnTriggerEnter*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void OnTriggerEnter::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<OnTriggerEnter*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Ownership::GetHashCode() const {
    return HashBuilder(3137u)
        .Add(IsOwnerUltimate)
        .Add(Owner)
        .Add(Source)
        .Add(Weapon)
        .Value();
}

void Ownership::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Ownership*>(ptr);
    SerializeField(&p->IsOwnerUltimate, serializer);
    SerializeField(&p->Owner, serializer);
    SerializeField(&p->Source, serializer);
    SerializeField(&p->Weapon, serializer);
}

std::int32_t ParabolaFollowerParams::GetHashCode() const {
    return HashBuilder(1669u)
        .Add(End)
        .Add(MaxHeight)
        .Add(Start)
        .Add(WillStopAtEndPos)
        .Value();
}

void ParabolaFollowerParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ParabolaFollowerParams*>(ptr);
    SerializeField(&p->WillStopAtEndPos, serializer);
    SerializeField(&p->MaxHeight, serializer);
    SerializeField(&p->End, serializer);
    SerializeField(&p->Start, serializer);
}

std::int32_t ParabolaFollower::GetHashCode() const {
    return HashBuilder(3163u)
        .Add(HomingTarget)
        .Add(LastPosition)
        .Add(LastUsedT)
        .Add(Params)
        .Add(RemoveRequestReason)
        .Add(RemoveRequestTick)
        .Add(Spec)
        .Add(StartFrame)
        .Add(TotalDuration)
        .Value();
}

void ParabolaFollower::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ParabolaFollower*>(ptr);
    SerializeField(&p->RemoveRequestTick, serializer);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->RemoveRequestReason, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->HomingTarget, serializer);
    SerializeField(&p->LastUsedT, serializer);
    SerializeField(&p->TotalDuration, serializer);
    SerializeField(&p->LastPosition, serializer);
    SerializeField(&p->Params, serializer);
}

std::int32_t PendingPhysicsImpulse::GetHashCode() const {
    return HashBuilder(3167u)
        .Add(hitpoint)
        .Add(impulse)
        .Add(impulseDelay)
        .Add(impulseTime)
        .Add(processed)
        .Value();
}

void PendingPhysicsImpulse::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PendingPhysicsImpulse*>(ptr);
    SerializeField(&p->impulseTime, serializer);
    SerializeField(&p->processed, serializer);
    SerializeField(&p->impulseDelay, serializer);
    SerializeField(&p->hitpoint, serializer);
    SerializeField(&p->impulse, serializer);
}

std::int32_t PushData::GetHashCode() const {
    return HashBuilder(1723u)
        .Add(bumpPercentage)
        .Add(bumpType)
        .Add(bypassPrePushMultiplier)
        .Add(bypassPushStop)
        .Add(canBump)
        .Add(flatBumpStrength)
        .Add(forcePushStop)
        .Add(forcePushStopDuration)
        .Add(forcePushStopStrength)
        .Add(hitpoint)
        .Add(impulse)
        .Add(isContinuous)
        .Add(onShieldBehaviour)
        .Add(pushed)
        .Add(pusher)
        .Add(pusherOwner)
        .Value();
}

void PushData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PushData*>(ptr);
    SerializeField(&p->bumpType, serializer);
    SerializeField(&p->onShieldBehaviour, serializer);
    SerializeField(&p->bypassPrePushMultiplier, serializer);
    SerializeField(&p->bypassPushStop, serializer);
    SerializeField(&p->canBump, serializer);
    SerializeField(&p->forcePushStop, serializer);
    SerializeField(&p->isContinuous, serializer);
    SerializeField(&p->pushed, serializer);
    SerializeField(&p->pusher, serializer);
    SerializeField(&p->pusherOwner, serializer);
    SerializeField(&p->bumpPercentage, serializer);
    SerializeField(&p->flatBumpStrength, serializer);
    SerializeField(&p->forcePushStopDuration, serializer);
    SerializeField(&p->forcePushStopStrength, serializer);
    SerializeField(&p->hitpoint, serializer);
    SerializeField(&p->impulse, serializer);
}

std::int32_t PendingPushStop::GetHashCode() const {
    return HashBuilder(3169u)
        .Add(isContinuous)
        .Add(pendingData)
        .Add(pushStopDuration)
        .Value();
}

void PendingPushStop::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PendingPushStop*>(ptr);
    SerializeField(&p->isContinuous, serializer);
    SerializeField(&p->pushStopDuration, serializer);
    SerializeField(&p->pendingData, serializer);
}

std::int32_t PlayerActor::GetHashCode() const {
    return HashBuilder(3181u)
        .Add(actorID)
        .Add(characterIndex)
        .Add(combatStartFrame)
        .Add(durationBeforeCombatReset)
        .Add(eliminationFrame)
        .Add(gameModeFinished)
        .Add(hasDisconnected)
        .Add(heroLevel)
        .Add(heroTalentLevel)
        .Add(inCombatWith)
        .Add(introTrackId)
        .Add(isBot)
        .Add(isInactive)
        .Add(killCount)
        .Add(killStreak)
        .Add(lastKillFrame)
        .Add(maxNoActionTimeAfterPush)
        .Add(noActionCounter)
        .Add(noActionMaxPushForce)
        .Add(noActionPushForceThreshold)
        .Add(noActionTimeAfterPush)
        .Add(playerRef)
        .Add(rivalEntity)
        .Add(rngSeed)
        .Add(secondarySkinIndex)
        .Add(skinIndex)
        .Add(timeWithoutCombatActivity)
        .Add(winStreak)
        .Value();
}

void PlayerActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PlayerActor*>(ptr);
    SerializeField(&p->killCount, serializer);
    SerializeField(&p->introTrackId, serializer);
    SerializeField(&p->winStreak, serializer);
    SerializeField(&p->actorID, serializer);
    SerializeField(&p->characterIndex, serializer);
    SerializeField(&p->combatStartFrame, serializer);
    SerializeField(&p->eliminationFrame, serializer);
    SerializeField(&p->heroLevel, serializer);
    SerializeField(&p->heroTalentLevel, serializer);
    SerializeField(&p->killStreak, serializer);
    SerializeField(&p->lastKillFrame, serializer);
    SerializeField(&p->rngSeed, serializer);
    SerializeField(&p->secondarySkinIndex, serializer);
    SerializeField(&p->skinIndex, serializer);
    SerializeField(&p->playerRef, serializer);
    SerializeField(&p->gameModeFinished, serializer);
    SerializeField(&p->hasDisconnected, serializer);
    SerializeField(&p->isBot, serializer);
    SerializeField(&p->isInactive, serializer);
    SerializeField(&p->inCombatWith, serializer);
    SerializeField(&p->rivalEntity, serializer);
    SerializeField(&p->durationBeforeCombatReset, serializer);
    SerializeField(&p->maxNoActionTimeAfterPush, serializer);
    SerializeField(&p->noActionCounter, serializer);
    SerializeField(&p->noActionMaxPushForce, serializer);
    SerializeField(&p->noActionPushForceThreshold, serializer);
    SerializeField(&p->noActionTimeAfterPush, serializer);
    SerializeField(&p->timeWithoutCombatActivity, serializer);
}

std::int32_t PlayerActorInitialized::GetHashCode() const {
    return HashBuilder(3187u).Value();
}

void PlayerActorInitialized::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t PowerUp::GetHashCode() const {
    return HashBuilder(3191u)
        .Add(IsPickedUp)
        .Add(Spec)
        .Value();
}

void PowerUp::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PowerUp*>(ptr);
    SerializeField(&p->IsPickedUp, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t PowerUpUser::GetHashCode() const {
    return HashBuilder(3203u)
        .Add(ActivePowerUpsPtr)
        .Value();
}

void PowerUpUser::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PowerUpUser*>(ptr);
    SerializeList<ActivePowerUp>(&p->ActivePowerUpsPtr, serializer);
}

void PowerUpUser::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (ActivePowerUpsPtr == Ptr::Null()) {
        ActivePowerUpsPtr = AllocateListPtr<ActivePowerUp>(f, 8);
    }
}

void PowerUpUser::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (ActivePowerUpsPtr != Ptr::Null()) {
        f->FreeList(get_ActivePowerUps());
    }
    ActivePowerUpsPtr = Ptr::Null();
}

void PowerUpUser::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<PowerUpUser*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void PowerUpUser::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<PowerUpUser*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t PreciseHitReceiver::GetHashCode() const {
    return HashBuilder(3209u)
        .Add(Spec)
        .Value();
}

void PreciseHitReceiver::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PreciseHitReceiver*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t PrioritisedTarget::GetHashCode() const {
    return HashBuilder(1709u)
        .Add(acquiredFrame)
        .Add(currentWaypointIndex)
        .Add(entity)
        .Add(entityPosition)
        .Add(lostFrame)
        .Add(waypointRadius)
        .Add(waypointsPtr)
        .Value();
}

void PrioritisedTarget::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PrioritisedTarget*>(ptr);
    SerializeField(&p->acquiredFrame, serializer);
    SerializeField(&p->currentWaypointIndex, serializer);
    SerializeField(&p->lostFrame, serializer);
    SerializeList<FPVector3>(&p->waypointsPtr, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->waypointRadius, serializer);
    SerializeField(&p->entityPosition, serializer);
}

void PrioritisedTarget::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    waypointsPtr = Ptr::Null();
}

std::int32_t PrioritisedTargetHandler::GetHashCode() const {
    return HashBuilder(3217u)
        .Add(spec)
        .Add(_targets_)
        .Value();
}

void PrioritisedTargetHandler::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PrioritisedTargetHandler*>(ptr);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->_targets_, serializer);
}

void PrioritisedTargetHandler::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<PrioritisedTargetHandler*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Projectile::GetHashCode() const {
    return HashBuilder(3221u)
        .Add(CurrVelocity)
        .Add(CurrentHomingLerpFactor)
        .Add(CurveRemainingDistanceToTarget)
        .Add(DeathType)
        .Add(HasValidCollisionImpact)
        .Add(Impact)
        .Add(ImpactFrame)
        .Add(LastEntitySpawnFrame)
        .Add(NextPosition)
        .Add(OwnerActor)
        .Add(OwnerWeapon)
        .Add(OwnerWeaponHierarchyOffset)
        .Add(PreviousPosition)
        .Add(RaycastQueryIndex)
        .Add(ReachedTarget)
        .Add(ReachedTargetFrame)
        .Add(ReachedTargetPosition)
        .Add(SpawnOffset)
        .Add(SpawnedFrame)
        .Add(SpawnedPosition)
        .Add(Spec)
        .Add(TargetEntity)
        .Add(TargetPosIsRangedRaycastHit)
        .Add(TargetPosition)
        .Add(TargetedPart)
        .Add(ToLookPointRaycastLength)
        .Add(WeaponRange)
        .Add(entitiesToSpawnOnImpactPtr)
        .Add(forceDisableHoming)
        .Value();
}

void Projectile::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Projectile*>(ptr);
    SerializeField(&p->ImpactFrame, serializer);
    SerializeField(&p->LastEntitySpawnFrame, serializer);
    SerializeField(&p->RaycastQueryIndex, serializer);
    SerializeField(&p->ReachedTargetFrame, serializer);
    SerializeField(&p->SpawnedFrame, serializer);
    SerializeField(&p->DeathType, serializer);
    SerializeField(&p->TargetedPart, serializer);
    SerializeField(&p->HasValidCollisionImpact, serializer);
    SerializeField(&p->ReachedTarget, serializer);
    SerializeField(&p->TargetPosIsRangedRaycastHit, serializer);
    SerializeField(&p->forceDisableHoming, serializer);
    SerializeList<AssetRefEntityPrototype>(&p->entitiesToSpawnOnImpactPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->OwnerActor, serializer);
    SerializeField(&p->OwnerWeapon, serializer);
    SerializeField(&p->TargetEntity, serializer);
    SerializeField(&p->CurrentHomingLerpFactor, serializer);
    SerializeField(&p->CurveRemainingDistanceToTarget, serializer);
    SerializeField(&p->ToLookPointRaycastLength, serializer);
    SerializeField(&p->WeaponRange, serializer);
    SerializeField(&p->CurrVelocity, serializer);
    SerializeField(&p->NextPosition, serializer);
    SerializeField(&p->OwnerWeaponHierarchyOffset, serializer);
    SerializeField(&p->PreviousPosition, serializer);
    SerializeField(&p->ReachedTargetPosition, serializer);
    SerializeField(&p->SpawnOffset, serializer);
    SerializeField(&p->SpawnedPosition, serializer);
    SerializeField(&p->TargetPosition, serializer);
    SerializeField(&p->Impact, serializer);
}

void Projectile::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (entitiesToSpawnOnImpactPtr == Ptr::Null()) {
        entitiesToSpawnOnImpactPtr = AllocateListPtr<AssetRefEntityPrototype>(f, 8);
    }
}

void Projectile::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (entitiesToSpawnOnImpactPtr != Ptr::Null()) {
        f->FreeList(get_entitiesToSpawnOnImpact());
    }
    entitiesToSpawnOnImpactPtr = Ptr::Null();
}

void Projectile::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Projectile*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Projectile::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Projectile*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ProjectileDebugInfo::GetHashCode() const {
    return HashBuilder(3229u)
        .Add(raycastEndPosition)
        .Add(raycastStartPosition)
        .Value();
}

void ProjectileDebugInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ProjectileDebugInfo*>(ptr);
    SerializeField(&p->raycastEndPosition, serializer);
    SerializeField(&p->raycastStartPosition, serializer);
}

std::int32_t ProjectileHitInfo::GetHashCode() const {
    return HashBuilder(3251u)
        .Add(HasValidCollisionImpact)
        .Add(hit)
        .Add(projectileDeathType)
        .Add(projectileOwner)
        .Add(projectileWeapon)
        .Add(spawnPosition)
        .Add(velocity)
        .Value();
}

void ProjectileHitInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ProjectileHitInfo*>(ptr);
    SerializeField(&p->projectileDeathType, serializer);
    SerializeField(&p->HasValidCollisionImpact, serializer);
    SerializeField(&p->projectileOwner, serializer);
    SerializeField(&p->projectileWeapon, serializer);
    SerializeField(&p->spawnPosition, serializer);
    SerializeField(&p->velocity, serializer);
    SerializeField(&p->hit, serializer);
}

std::int32_t ProjectileSpawner::GetHashCode() const {
    return HashBuilder(3253u)
        .Add(CurrSpawnIndex)
        .Add(LastSpawnFrame)
        .Add(Normal)
        .Add(RandomRad)
        .Add(Spec)
        .Add(StartFrame)
        .Value();
}

void ProjectileSpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ProjectileSpawner*>(ptr);
    SerializeField(&p->CurrSpawnIndex, serializer);
    SerializeField(&p->LastSpawnFrame, serializer);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->RandomRad, serializer);
    SerializeField(&p->Normal, serializer);
}

std::int32_t PushedByData::GetHashCode() const {
    return HashBuilder(1733u)
        .Add(frame)
        .Add(owner)
        .Add(source)
        .Add(weapon)
        .Value();
}

void PushedByData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PushedByData*>(ptr);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->owner, serializer);
    SerializeField(&p->source, serializer);
    SerializeField(&p->weapon, serializer);
}

std::int32_t Pushable::GetHashCode() const {
    return HashBuilder(3257u)
        .Add(PendingPushRequestsPtr)
        .Add(Spec)
        .Add(consecutivePushCount)
        .Add(isInsideContinuousPush)
        .Add(prePhysicsPushForceMultiplier)
        .Add(pushModifierStatusEffect)
        .Add(pushModifierStatusEffectLastFrame)
        .Add(pushModifierStatusEffectRevertAfterPush)
        .Add(pushModifierStatusEffectStartFrame)
        .Add(pushStopStartFrame)
        .Add(pushedBy)
        .Value();
}

void Pushable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Pushable*>(ptr);
    SerializeField(&p->consecutivePushCount, serializer);
    SerializeField(&p->pushModifierStatusEffectLastFrame, serializer);
    SerializeField(&p->pushModifierStatusEffectStartFrame, serializer);
    SerializeField(&p->pushStopStartFrame, serializer);
    SerializeField(&p->isInsideContinuousPush, serializer);
    SerializeField(&p->pushModifierStatusEffectRevertAfterPush, serializer);
    SerializeList<PushData>(&p->PendingPushRequestsPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->prePhysicsPushForceMultiplier, serializer);
    SerializeField(&p->pushModifierStatusEffect, serializer);
    SerializeField(&p->pushedBy, serializer);
}

void Pushable::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (PendingPushRequestsPtr == Ptr::Null()) {
        PendingPushRequestsPtr = AllocateListPtr<PushData>(f, 8);
    }
}

void Pushable::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (PendingPushRequestsPtr != Ptr::Null()) {
        f->FreeList(get_PendingPushRequests());
    }
    PendingPushRequestsPtr = Ptr::Null();
}

void Pushable::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Pushable*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Pushable::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Pushable*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t PushableTrigger::GetHashCode() const {
    return HashBuilder(3259u).Value();
}

void PushableTrigger::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t Pusher::GetHashCode() const {
    return HashBuilder(3271u)
        .Add(CurrentPusherRange)
        .Add(Spec)
        .Value();
}

void Pusher::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Pusher*>(ptr);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->CurrentPusherRange, serializer);
}

std::int32_t Rail::GetHashCode() const {
    return HashBuilder(3299u)
        .Add(bakedRailIndex)
        .Add(entity)
        .Add(grindersPtr)
        .Value();
}

void Rail::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Rail*>(ptr);
    SerializeField(&p->bakedRailIndex, serializer);
    SerializeList<EntityRef>(&p->grindersPtr, serializer);
    SerializeField(&p->entity, serializer);
}

void Rail::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (grindersPtr == Ptr::Null()) {
        grindersPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void Rail::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (grindersPtr != Ptr::Null()) {
        f->FreeList(get_grinders());
    }
    grindersPtr = Ptr::Null();
}

void Rail::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Rail*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Rail::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Rail*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t RailCollider::GetHashCode() const {
    return HashBuilder(3301u)
        .Add(distanceOnRail)
        .Add(entity)
        .Add(nearbyRailColLeft)
        .Add(nearbyRailColRight)
        .Add(railEntity)
        .Value();
}

void RailCollider::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RailCollider*>(ptr);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->nearbyRailColLeft, serializer);
    SerializeField(&p->nearbyRailColRight, serializer);
    SerializeField(&p->railEntity, serializer);
    SerializeField(&p->distanceOnRail, serializer);
}

std::int32_t RandomWeaponsActor::GetHashCode() const {
    return HashBuilder(3307u)
        .Add(nextRandomWeaponFrame)
        .Value();
}

void RandomWeaponsActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RandomWeaponsActor*>(ptr);
    SerializeField(&p->nextRandomWeaponFrame, serializer);
}

std::int32_t RankingManager::GetHashCode() const {
    return HashBuilder(3313u)
        .Add(closeToWinSlowMotionDone)
        .Add(lastUpdateFrame)
        .Add(needUpdate)
        .Add(rankingPtr)
        .Value();
}

void RankingManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RankingManager*>(ptr);
    SerializeField(&p->lastUpdateFrame, serializer);
    SerializeField(&p->closeToWinSlowMotionDone, serializer);
    SerializeField(&p->needUpdate, serializer);
    SerializeList<RankingItem>(&p->rankingPtr, serializer);
}

void RankingManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (rankingPtr == Ptr::Null()) {
        rankingPtr = AllocateListPtr<RankingItem>(f, 8);
    }
}

void RankingManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (rankingPtr != Ptr::Null()) {
        f->FreeList(get_ranking());
    }
    rankingPtr = Ptr::Null();
}

void RankingManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RankingManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void RankingManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RankingManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t RecoverAmmoOnDashEnhancement::GetHashCode() const {
    return HashBuilder(3319u)
        .Add(Spec)
        .Value();
}

void RecoverAmmoOnDashEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RecoverAmmoOnDashEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t ReplaceMultiShotConfigsEnhancement::GetHashCode() const {
    return HashBuilder(3323u)
        .Add(Spec)
        .Value();
}

void ReplaceMultiShotConfigsEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ReplaceMultiShotConfigsEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t RewardReceiver::GetHashCode() const {
    return HashBuilder(3329u)
        .Add(Spec)
        .Add(pendingRewardsPtr)
        .Value();
}

void RewardReceiver::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RewardReceiver*>(ptr);
    SerializeList<Reward>(&p->pendingRewardsPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void RewardReceiver::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pendingRewardsPtr == Ptr::Null()) {
        pendingRewardsPtr = AllocateListPtr<Reward>(f, 8);
    }
}

void RewardReceiver::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pendingRewardsPtr != Ptr::Null()) {
        f->FreeList(get_pendingRewards());
    }
    pendingRewardsPtr = Ptr::Null();
}

void RewardReceiver::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RewardReceiver*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void RewardReceiver::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RewardReceiver*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Roll::GetHashCode() const {
    return HashBuilder(3331u)
        .Add(Spec)
        .Add(bounceCount)
        .Add(direction)
        .Add(hitEntitiesPtr)
        .Value();
}

void Roll::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Roll*>(ptr);
    SerializeList<CollidingEntity>(&p->hitEntitiesPtr, serializer);
    SerializeField(&p->bounceCount, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->direction, serializer);
}

void Roll::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitEntitiesPtr == Ptr::Null()) {
        hitEntitiesPtr = AllocateListPtr<CollidingEntity>(f, 8);
    }
}

void Roll::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_hitEntities());
    }
    hitEntitiesPtr = Ptr::Null();
}

void Roll::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Roll*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Roll::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Roll*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t RunnableWall::GetHashCode() const {
    return HashBuilder(3343u).Value();
}

void RunnableWall::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t RunnableWallTriggerZone::GetHashCode() const {
    return HashBuilder(3347u).Value();
}

void RunnableWallTriggerZone::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t RynoStats::GetHashCode() const {
    return HashBuilder(1801u)
        .Add(initialised)
        .Add(maxMissilesPerFire)
        .Value();
}

void RynoStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RynoStats*>(ptr);
    SerializeField(&p->initialised, serializer);
    SerializeField(&p->maxMissilesPerFire, serializer);
}

std::int32_t RynoWeapon::GetHashCode() const {
    return HashBuilder(3359u)
        .Add(LastFiredTargetIndex)
        .Add(LastFiredTime)
        .Add(LockedTargetsPtr)
        .Add(RemainingMissiles)
        .Add(StartLockingFrame)
        .Add(StartLockingWeaponUser)
        .Add(Stats)
        .Value();
}

void RynoWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RynoWeapon*>(ptr);
    SerializeField(&p->LastFiredTargetIndex, serializer);
    SerializeField(&p->LastFiredTime, serializer);
    SerializeField(&p->StartLockingFrame, serializer);
    SerializeList<LockedTarget>(&p->LockedTargetsPtr, serializer);
    SerializeField(&p->RemainingMissiles, serializer);
    SerializeField(&p->StartLockingWeaponUser, serializer);
    SerializeField(&p->Stats, serializer);
}

void RynoWeapon::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (LockedTargetsPtr == Ptr::Null()) {
        LockedTargetsPtr = AllocateListPtr<LockedTarget>(f, 8);
    }
}

void RynoWeapon::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (LockedTargetsPtr != Ptr::Null()) {
        f->FreeList(get_LockedTargets());
    }
    LockedTargetsPtr = Ptr::Null();
}

void RynoWeapon::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RynoWeapon*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void RynoWeapon::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<RynoWeapon*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ShapeOverlapper::GetHashCode() const {
    return HashBuilder(3361u)
        .Add(Spec)
        .Add(hitsPtr)
        .Add(lastCastFrame)
        .Value();
}

void ShapeOverlapper::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShapeOverlapper*>(ptr);
    SerializeField(&p->lastCastFrame, serializer);
    SerializeList<CollidingEntity>(&p->hitsPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void ShapeOverlapper::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitsPtr == Ptr::Null()) {
        hitsPtr = AllocateListPtr<CollidingEntity>(f, 8);
    }
}

void ShapeOverlapper::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitsPtr != Ptr::Null()) {
        f->FreeList(get_hits());
    }
    hitsPtr = Ptr::Null();
}

void ShapeOverlapper::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ShapeOverlapper*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void ShapeOverlapper::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ShapeOverlapper*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ShieldedStats::GetHashCode() const {
    return HashBuilder(1847u)
        .Add(initialShieldPoints)
        .Add(shieldMaxPoints)
        .Add(statsInitialised)
        .Value();
}

void ShieldedStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShieldedStats*>(ptr);
    SerializeField(&p->statsInitialised, serializer);
    SerializeField(&p->initialShieldPoints, serializer);
    SerializeField(&p->shieldMaxPoints, serializer);
}

std::int32_t ShieldedActor::GetHashCode() const {
    return HashBuilder(3371u)
        .Add(DamageMultiplier_Shieldless)
        .Add(brokenThisFrame)
        .Add(canRegen)
        .Add(lastFrameShieldHitSpeedModWasApplied)
        .Add(shieldBypassedThisFrame)
        .Add(shieldIsBroken)
        .Add(shieldPoints)
        .Add(spec)
        .Add(startRegenFrame)
        .Add(state)
        .Add(stats)
        .Value();
}

void ShieldedActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShieldedActor*>(ptr);
    SerializeField(&p->lastFrameShieldHitSpeedModWasApplied, serializer);
    SerializeField(&p->startRegenFrame, serializer);
    SerializeField(&p->brokenThisFrame, serializer);
    SerializeField(&p->canRegen, serializer);
    SerializeField(&p->shieldBypassedThisFrame, serializer);
    SerializeField(&p->shieldIsBroken, serializer);
    SerializeField(&p->state, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->DamageMultiplier_Shieldless, serializer);
    SerializeField(&p->shieldPoints, serializer);
    SerializeField(&p->stats, serializer);
}

std::int32_t ShieldGroupStats::GetHashCode() const {
    return HashBuilder(1823u)
        .Add(AutoShootMaxMembers)
        .Add(Initialised)
        .Add(LastMaxedOutFrame)
        .Add(ShieldGroupMaxMembers)
        .Value();
}

void ShieldGroupStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShieldGroupStats*>(ptr);
    SerializeField(&p->AutoShootMaxMembers, serializer);
    SerializeField(&p->LastMaxedOutFrame, serializer);
    SerializeField(&p->ShieldGroupMaxMembers, serializer);
    SerializeField(&p->Initialised, serializer);
}

std::int32_t ShieldsGroup::GetHashCode() const {
    return HashBuilder(3373u)
        .Add(membersPtr)
        .Add(stats)
        .Value();
}

void ShieldsGroup::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShieldsGroup*>(ptr);
    SerializeList<ShieldGroupMember>(&p->membersPtr, serializer);
    SerializeField(&p->stats, serializer);
}

void ShieldsGroup::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (membersPtr == Ptr::Null()) {
        membersPtr = AllocateListPtr<ShieldGroupMember>(f, 8);
    }
}

void ShieldsGroup::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (membersPtr != Ptr::Null()) {
        f->FreeList(get_members());
    }
    membersPtr = Ptr::Null();
}

void ShieldsGroup::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ShieldsGroup*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void ShieldsGroup::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ShieldsGroup*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ShotgunBerserkerEnhancement::GetHashCode() const {
    return HashBuilder(3389u)
        .Add(OriginalAmmoAsset)
        .Add(Spec)
        .Value();
}

void ShotgunBerserkerEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShotgunBerserkerEnhancement*>(ptr);
    SerializeField(&p->OriginalAmmoAsset, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t ShotgunBerserkerEnhancementEffect::GetHashCode() const {
    return HashBuilder(3391u).Value();
}

void ShotgunBerserkerEnhancementEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ShotgunEnhancement::GetHashCode() const {
    return HashBuilder(3407u)
        .Add(OriginalAmmoAsset)
        .Add(Spec)
        .Value();
}

void ShotgunEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShotgunEnhancement*>(ptr);
    SerializeField(&p->OriginalAmmoAsset, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t ShotgunEnhancementEffect::GetHashCode() const {
    return HashBuilder(3413u).Value();
}

void ShotgunEnhancementEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ShrinkingZone::GetHashCode() const {
    return HashBuilder(3433u)
        .Add(Spec)
        .Add(closingPointIndex)
        .Add(closingZoneIndex)
        .Add(currentCenterPositonForBots)
        .Add(endPosition)
        .Add(endRadius)
        .Add(endTime)
        .Add(initialized)
        .Add(isShrinking)
        .Add(position)
        .Add(radius)
        .Add(startPosition)
        .Add(startRadius)
        .Add(startTime)
        .Add(updating)
        .Value();
}

void ShrinkingZone::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShrinkingZone*>(ptr);
    SerializeField(&p->closingPointIndex, serializer);
    SerializeField(&p->closingZoneIndex, serializer);
    SerializeField(&p->initialized, serializer);
    SerializeField(&p->isShrinking, serializer);
    SerializeField(&p->updating, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->endRadius, serializer);
    SerializeField(&p->endTime, serializer);
    SerializeField(&p->radius, serializer);
    SerializeField(&p->startRadius, serializer);
    SerializeField(&p->startTime, serializer);
    SerializeField(&p->currentCenterPositonForBots, serializer);
    SerializeField(&p->endPosition, serializer);
    SerializeField(&p->position, serializer);
    SerializeField(&p->startPosition, serializer);
}

std::int32_t ShrinkingZoneActor::GetHashCode() const {
    return HashBuilder(3449u)
        .Add(IgnoreShrinkingZone)
        .Add(IsInsideZone)
        .Add(enteredZoneFrame)
        .Add(lastHitFrame)
        .Value();
}

void ShrinkingZoneActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShrinkingZoneActor*>(ptr);
    SerializeField(&p->enteredZoneFrame, serializer);
    SerializeField(&p->lastHitFrame, serializer);
    SerializeField(&p->IgnoreShrinkingZone, serializer);
    SerializeField(&p->IsInsideZone, serializer);
}

std::int32_t SpawnProjectilesOnDestroy::GetHashCode() const {
    return HashBuilder(3457u)
        .Add(SpecsPtr)
        .Value();
}

void SpawnProjectilesOnDestroy::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<SpawnProjectilesOnDestroy*>(ptr);
    SerializeList<AssetRefProjectileSpawnerSpec>(&p->SpecsPtr, serializer);
}

void SpawnProjectilesOnDestroy::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    SpecsPtr = Ptr::Null();
}

void SpawnProjectilesOnDestroy::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<SpawnProjectilesOnDestroy*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Spawned::GetHashCode() const {
    return HashBuilder(3461u)
        .Add(spawnerEntity)
        .Value();
}

void Spawned::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Spawned*>(ptr);
    SerializeField(&p->spawnerEntity, serializer);
}

std::int32_t SpawnedGoldenBoltData::GetHashCode() const {
    return HashBuilder(3463u)
        .Add(ArrivalPoint)
        .Add(Entity)
        .Add(SpawnFrame)
        .Add(SpawnPointIndex)
        .Add(State)
        .Value();
}

void SpawnedGoldenBoltData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<SpawnedGoldenBoltData*>(ptr);
    SerializeField(&p->State, serializer);
    SerializeField(&p->SpawnFrame, serializer);
    SerializeField(&p->SpawnPointIndex, serializer);
    SerializeField(&p->Entity, serializer);
    SerializeField(&p->ArrivalPoint, serializer);
}

std::int32_t Speeder::GetHashCode() const {
    return HashBuilder(3467u)
        .Add(Duration)
        .Add(Spec)
        .Add(SpeedModifier)
        .Add(Team)
        .Add(UsedFrame)
        .Value();
}

void Speeder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Speeder*>(ptr);
    SerializeField(&p->UsedFrame, serializer);
    SerializeField(&p->Team, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Duration, serializer);
    SerializeField(&p->SpeedModifier, serializer);
}

std::int32_t StatsManager::GetHashCode() const {
    return HashBuilder(3469u)
        .Add(allFrameStatsPtr)
        .Add(curFramestatsPtr)
        .Add(summaryPtr)
        .Value();
}

void StatsManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<StatsManager*>(ptr);
    SerializeList<Stat>(&p->allFrameStatsPtr, serializer);
    SerializeList<Stat>(&p->curFramestatsPtr, serializer);
    SerializeDictionary<STAT_ID, StatSummary>(&p->summaryPtr, serializer);
}

void StatsManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (allFrameStatsPtr == Ptr::Null()) {
        allFrameStatsPtr = AllocateListPtr<Stat>(f, 8);
    }
    if (curFramestatsPtr == Ptr::Null()) {
        curFramestatsPtr = AllocateListPtr<Stat>(f, 8);
    }
    if (summaryPtr == Ptr::Null()) {
        summaryPtr = AllocateEnumDictionaryPtr<STAT_ID, StatSummary>(f, 8);
    }
}

void StatsManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (allFrameStatsPtr != Ptr::Null()) {
        f->FreeList(get_allFrameStats());
    }
    allFrameStatsPtr = Ptr::Null();
    if (curFramestatsPtr != Ptr::Null()) {
        f->FreeList(get_curFramestats());
    }
    curFramestatsPtr = Ptr::Null();
    if (summaryPtr != Ptr::Null()) {
        f->FreeDictionary(get_summary());
    }
    summaryPtr = Ptr::Null();
}

void StatsManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<StatsManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void StatsManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<StatsManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t StreamEffect::GetHashCode() const {
    return HashBuilder(3491u)
        .Add(Spec)
        .Add(compoundShape)
        .Add(overlapTransform)
        .Add(pointsPtr)
        .Value();
}

void StreamEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<StreamEffect*>(ptr);
    SerializeList<StreamPoint>(&p->pointsPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->overlapTransform, serializer);
    SerializeField(&p->compoundShape, serializer);
}

void StreamEffect::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pointsPtr == Ptr::Null()) {
        pointsPtr = AllocateListPtr<StreamPoint>(f, 8);
    }
}

void StreamEffect::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pointsPtr != Ptr::Null()) {
        f->FreeList(get_points());
    }
    pointsPtr = Ptr::Null();
}

void StreamEffect::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<StreamEffect*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void StreamEffect::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<StreamEffect*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t StreamEffectSpawner::GetHashCode() const {
    return HashBuilder(3499u)
        .Add(Spec)
        .Add(streamEffect)
        .Value();
}

void StreamEffectSpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<StreamEffectSpawner*>(ptr);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->streamEffect, serializer);
}

std::int32_t SweepAttack::GetHashCode() const {
    return HashBuilder(3511u)
        .Add(Spec)
        .Add(changeStateFrame)
        .Add(currentState)
        .Add(initialForward)
        .Value();
}

void SweepAttack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<SweepAttack*>(ptr);
    SerializeField(&p->changeStateFrame, serializer);
    SerializeField(&p->currentState, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->initialForward, serializer);
}

std::int32_t PotentialTarget::GetHashCode() const {
    return HashBuilder(1697u)
        .Add(Entity)
        .Add(LineOfSightQueryIndicesPtr)
        .Add(Position)
        .Add(Velocity)
        .Add(WeaponScore)
        .Add(bypassCameraAiming)
        .Add(inBack)
        .Add(inHeightDiff)
        .Add(inSight)
        .Add(inWeaponRange)
        .Add(inWeaponRangePreCheck)
        .Add(matchesWeaponTargetFilter)
        .Add(overShootDistance)
        .Value();
}

void PotentialTarget::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PotentialTarget*>(ptr);
    SerializeField(&p->bypassCameraAiming, serializer);
    SerializeField(&p->inBack, serializer);
    SerializeField(&p->inHeightDiff, serializer);
    SerializeField(&p->inSight, serializer);
    SerializeField(&p->inWeaponRange, serializer);
    SerializeField(&p->inWeaponRangePreCheck, serializer);
    SerializeField(&p->matchesWeaponTargetFilter, serializer);
    SerializeList<std::int32_t>(&p->LineOfSightQueryIndicesPtr, serializer);
    SerializeField(&p->Entity, serializer);
    SerializeField(&p->WeaponScore, serializer);
    SerializeField(&p->overShootDistance, serializer);
    SerializeField(&p->Position, serializer);
    SerializeField(&p->Velocity, serializer);
}

void PotentialTarget::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    LineOfSightQueryIndicesPtr = Ptr::Null();
}

std::int32_t TargetFinder::GetHashCode() const {
    return HashBuilder(3517u)
        .Add(AutoTargetInfo)
        .Add(BotCustomShootTarget)
        .Add(EquippedWeaponDamageType)
        .Add(EquippedWeaponFinderSpecOverride)
        .Add(EquippedWeaponOverShootDistance)
        .Add(EquippedWeaponRange)
        .Add(HasShootableTarget)
        .Add(LineOfSightCheckRequested)
        .Add(PotentialTargetsPtr)
        .Add(Spec)
        .Add(WrenchAutoTargetInfo)
        .Add(disabled)
        .Add(forceSetAutoTarget)
        .Add(keepAllPotentialTargets)
        .Add(overridenMaxRange)
        .Add(useContinuousLineOfSightChecks)
        .Value();
}

void TargetFinder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TargetFinder*>(ptr);
    SerializeField(&p->EquippedWeaponDamageType, serializer);
    SerializeField(&p->HasShootableTarget, serializer);
    SerializeField(&p->LineOfSightCheckRequested, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->forceSetAutoTarget, serializer);
    SerializeField(&p->keepAllPotentialTargets, serializer);
    SerializeField(&p->useContinuousLineOfSightChecks, serializer);
    SerializeList<PotentialTarget>(&p->PotentialTargetsPtr, serializer);
    SerializeField(&p->EquippedWeaponFinderSpecOverride, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->BotCustomShootTarget, serializer);
    SerializeField(&p->EquippedWeaponOverShootDistance, serializer);
    SerializeField(&p->EquippedWeaponRange, serializer);
    SerializeField(&p->overridenMaxRange, serializer);
    SerializeField(&p->AutoTargetInfo, serializer);
    SerializeField(&p->WrenchAutoTargetInfo, serializer);
}

void TargetFinder::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<TargetFinder*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void TargetFinder::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<TargetFinder*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t TargetHolder::GetHashCode() const {
    return HashBuilder(3527u)
        .Add(targetsPtr)
        .Value();
}

void TargetHolder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TargetHolder*>(ptr);
    SerializeList<TargetingData>(&p->targetsPtr, serializer);
}

void TargetHolder::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (targetsPtr != Ptr::Null()) {
        f->FreeList(get_targets());
    }
    targetsPtr = Ptr::Null();
}

void TargetHolder::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<TargetHolder*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Targetable::GetHashCode() const {
    return HashBuilder(3529u)
        .Add(BoundsSizeMultiplier)
        .Add(IgnoreAutoShoot)
        .Add(IgnoreAutoTargetInsideBounds)
        .Add(IgnoreCameraAiming)
        .Value();
}

void Targetable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Targetable*>(ptr);
    SerializeField(&p->IgnoreAutoShoot, serializer);
    SerializeField(&p->IgnoreAutoTargetInsideBounds, serializer);
    SerializeField(&p->IgnoreCameraAiming, serializer);
    SerializeField(&p->BoundsSizeMultiplier, serializer);
}

std::int32_t TargetedEffect::GetHashCode() const {
    return HashBuilder(3533u)
        .Add(maxTickCount)
        .Add(spec)
        .Add(startFrame)
        .Add(startPosition)
        .Add(target)
        .Add(tickCount)
        .Add(tickedEntitiesPtr)
        .Value();
}

void TargetedEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TargetedEffect*>(ptr);
    SerializeField(&p->maxTickCount, serializer);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->tickCount, serializer);
    SerializeList<EntityRef>(&p->tickedEntitiesPtr, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->target, serializer);
    SerializeField(&p->startPosition, serializer);
}

void TargetedEffect::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (tickedEntitiesPtr == Ptr::Null()) {
        tickedEntitiesPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void TargetedEffect::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (tickedEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_tickedEntities());
    }
    tickedEntitiesPtr = Ptr::Null();
}

void TargetedEffect::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<TargetedEffect*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void TargetedEffect::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<TargetedEffect*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Team::GetHashCode() const {
    return HashBuilder(3539u)
        .Add(TeamID)
        .Add(eliminationFrame)
        .Add(isLeader)
        .Value();
}

void Team::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Team*>(ptr);
    SerializeField(&p->TeamID, serializer);
    SerializeField(&p->eliminationFrame, serializer);
    SerializeField(&p->isLeader, serializer);
}

std::int32_t TeslaClawMoreBeam::GetHashCode() const {
    return HashBuilder(3541u)
        .Add(Spec)
        .Value();
}

void TeslaClawMoreBeam::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TeslaClawMoreBeam*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t TimeScale::GetHashCode() const {
    return HashBuilder(3547u)
        .Add(autoReverse)
        .Add(endFrame)
        .Add(endTimeScale)
        .Add(startFrame)
        .Add(startTimeScale)
        .Value();
}

void TimeScale::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TimeScale*>(ptr);
    SerializeField(&p->endFrame, serializer);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->autoReverse, serializer);
    SerializeField(&p->endTimeScale, serializer);
    SerializeField(&p->startTimeScale, serializer);
}

std::int32_t TrailSpawner::GetHashCode() const {
    return HashBuilder(3557u)
        .Add(Spec)
        .Add(aoeTrail)
        .Value();
}

void TrailSpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TrailSpawner*>(ptr);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->aoeTrail, serializer);
}

std::int32_t TravolatorActor::GetHashCode() const {
    return HashBuilder(3559u)
        .Add(Direction)
        .Add(Spec)
        .Value();
}

void TravolatorActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TravolatorActor*>(ptr);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Direction, serializer);
}

std::int32_t TurnTo::GetHashCode() const {
    return HashBuilder(3571u)
        .Add(lookAtDirection)
        .Add(turnSpeed)
        .Value();
}

void TurnTo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TurnTo*>(ptr);
    SerializeField(&p->turnSpeed, serializer);
    SerializeField(&p->lookAtDirection, serializer);
}

std::int32_t TutorialGameMode::GetHashCode() const {
    return HashBuilder(3581u)
        .Add(Spec)
        .Add(currentStep)
        .Add(isPVETuto)
        .Value();
}

void TutorialGameMode::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TutorialGameMode*>(ptr);
    SerializeField(&p->isPVETuto, serializer);
    SerializeField(&p->currentStep, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t UtilityReasoner::GetHashCode() const {
    return HashBuilder(1933u)
        .Add(ConsiderationsPtr)
        .Add(CooldownsDictPtr)
        .Add(HighRankConsiderationsPtr)
        .Add(MomentumListPtr)
        .Add(PreviousExecutionPtr)
        .Add(TimeToTick)
        .Add(UTRoot)
        .Value();
}

void UtilityReasoner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UtilityReasoner*>(ptr);
    SerializeList<AssetRefConsideration>(&p->ConsiderationsPtr, serializer);
    SerializeDictionary<AssetRefConsideration, FP>(&p->CooldownsDictPtr, serializer);
    SerializeList<AssetRefConsideration>(&p->HighRankConsiderationsPtr, serializer);
    SerializeList<UTMomentumPack>(&p->MomentumListPtr, serializer);
    SerializeList<AssetRefConsideration>(&p->PreviousExecutionPtr, serializer);
    SerializeField(&p->UTRoot, serializer);
    SerializeField(&p->TimeToTick, serializer);
}

void UtilityReasoner::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    ConsiderationsPtr = Ptr::Null();
    CooldownsDictPtr = Ptr::Null();
    HighRankConsiderationsPtr = Ptr::Null();
    MomentumListPtr = Ptr::Null();
    PreviousExecutionPtr = Ptr::Null();
}

std::int32_t UTAgent::GetHashCode() const {
    return HashBuilder(3583u)
        .Add(Config)
        .Add(UtilityReasoner)
        .Value();
}

void UTAgent::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UTAgent*>(ptr);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->UtilityReasoner, serializer);
}

void UTAgent::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<UTAgent*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t UltimateStats::GetHashCode() const {
    return HashBuilder(1913u)
        .Add(FullCharge)
        .Value();
}

void UltimateStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UltimateStats*>(ptr);
    SerializeField(&p->FullCharge, serializer);
}

std::int32_t UltimateAbilityUser::GetHashCode() const {
    return HashBuilder(3593u)
        .Add(Stats)
        .Add(abilitySpec)
        .Add(activationFrame)
        .Add(bonusChargePerSecond)
        .Add(chargePerPushReceivedMultiplier)
        .Add(chargeWithTimeDisabled)
        .Add(chargeWithTimeMultiplier)
        .Add(chargedValue)
        .Add(deactivationFrame)
        .Add(disabled)
        .Add(lastHitChargeFrame)
        .Add(userSpec)
        .Value();
}

void UltimateAbilityUser::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UltimateAbilityUser*>(ptr);
    SerializeField(&p->activationFrame, serializer);
    SerializeField(&p->deactivationFrame, serializer);
    SerializeField(&p->lastHitChargeFrame, serializer);
    SerializeField(&p->chargeWithTimeDisabled, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->Stats, serializer);
    SerializeField(&p->abilitySpec, serializer);
    SerializeField(&p->userSpec, serializer);
    SerializeField(&p->bonusChargePerSecond, serializer);
    SerializeField(&p->chargePerPushReceivedMultiplier, serializer);
    SerializeField(&p->chargeWithTimeMultiplier, serializer);
    SerializeField(&p->chargedValue, serializer);
}

std::int32_t UltimateEvolution::GetHashCode() const {
    return HashBuilder(3607u)
        .Add(AoESpawned)
        .Add(CurrentEvolutionLevel)
        .Value();
}

void UltimateEvolution::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UltimateEvolution*>(ptr);
    SerializeField(&p->CurrentEvolutionLevel, serializer);
    SerializeField(&p->AoESpawned, serializer);
}

std::int32_t UltimateSlam::GetHashCode() const {
    return HashBuilder(3613u)
        .Add(endFrame)
        .Add(startPosition)
        .Value();
}

void UltimateSlam::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UltimateSlam*>(ptr);
    SerializeField(&p->endFrame, serializer);
    SerializeField(&p->startPosition, serializer);
}

std::int32_t UltimateTank::GetHashCode() const {
    return HashBuilder(3617u)
        .Add(healthDecreasePerSecond)
        .Add(playerInitialMaxHP)
        .Add(playerMaxHP)
        .Add(spawnFrame)
        .Value();
}

void UltimateTank::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UltimateTank*>(ptr);
    SerializeField(&p->spawnFrame, serializer);
    SerializeField(&p->healthDecreasePerSecond, serializer);
    SerializeField(&p->playerInitialMaxHP, serializer);
    SerializeField(&p->playerMaxHP, serializer);
}

std::int32_t UnlockedFeaturesManager::GetHashCode() const {
    return HashBuilder(3623u)
        .Add(_unlockedFeatures_)
        .Value();
}

void UnlockedFeaturesManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UnlockedFeaturesManager*>(ptr);
    SerializeField(&p->_unlockedFeatures_, serializer);
}

std::int32_t VoidRepulserBulldozerEnhancement::GetHashCode() const {
    return HashBuilder(3631u)
        .Add(IsMoveSpeedModifierApplied)
        .Add(Spec)
        .Add(UsageStartFrame)
        .Value();
}

void VoidRepulserBulldozerEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<VoidRepulserBulldozerEnhancement*>(ptr);
    SerializeField(&p->UsageStartFrame, serializer);
    SerializeField(&p->IsMoveSpeedModifierApplied, serializer);
    SerializeField(&p->Spec, serializer);
}

} // namespace Quantum
