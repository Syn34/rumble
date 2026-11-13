// components_1.cpp - GetHashCode / Serialize / collection lifetime of the types in
// components.h (split over several files only to keep build times down).
#include "quantum_code/components.h"

#include "quantum_code/frame.h"

namespace Quantum {

std::int32_t AIBlackboardComponent::GetHashCode() const {
    return HashBuilder(2039u)
        .Add(Board)
        .Add(EntriesPtr)
        .Value();
}

void AIBlackboardComponent::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AIBlackboardComponent*>(ptr);
    SerializeDictionary<std::int32_t, BlackboardEntry>(&p->EntriesPtr, serializer);
    SerializeField(&p->Board, serializer);
}

void AIBlackboardComponent::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    EntriesPtr = Ptr::Null();
}

void AIBlackboardComponent::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AIBlackboardComponent*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t AIMoveForwardBehaviour::GetHashCode() const {
    return HashBuilder(2053u)
        .Add(BouncedFrame)
        .Add(ConstrainToSpawnerEnabled)
        .Add(Enabled)
        .Add(Spec)
        .Add(WantedDirection)
        .Add(WasEnabledBeforeSystemDisabled)
        .Value();
}

void AIMoveForwardBehaviour::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AIMoveForwardBehaviour*>(ptr);
    SerializeField(&p->BouncedFrame, serializer);
    SerializeField(&p->ConstrainToSpawnerEnabled, serializer);
    SerializeField(&p->Enabled, serializer);
    SerializeField(&p->WasEnabledBeforeSystemDisabled, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->WantedDirection, serializer);
}

std::int32_t AINavigation::GetHashCode() const {
    return HashBuilder(2063u)
        .Add(bouncingBalloonEntity)
        .Add(bumperInputDirection)
        .Add(hasBouncedOnBalloon)
        .Add(isOnJumpLink)
        .Add(isOnRailLink)
        .Add(isTakingBumper)
        .Add(jumpLinkDirection)
        .Add(lastPos)
        .Add(lookAtDir)
        .Add(maintainDirectionUntilPathFound)
        .Add(navTarget)
        .Add(needRepathOnLanding)
        .Add(nextRepathFrame)
        .Add(pathTTL)
        .Add(pathfindingStatus)
        .Add(spec)
        .Value();
}

void AINavigation::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AINavigation*>(ptr);
    SerializeField(&p->pathfindingStatus, serializer);
    SerializeField(&p->nextRepathFrame, serializer);
    SerializeField(&p->hasBouncedOnBalloon, serializer);
    SerializeField(&p->isOnJumpLink, serializer);
    SerializeField(&p->isOnRailLink, serializer);
    SerializeField(&p->isTakingBumper, serializer);
    SerializeField(&p->maintainDirectionUntilPathFound, serializer);
    SerializeField(&p->needRepathOnLanding, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->bouncingBalloonEntity, serializer);
    SerializeField(&p->pathTTL, serializer);
    SerializeField(&p->bumperInputDirection, serializer);
    SerializeField(&p->jumpLinkDirection, serializer);
    SerializeField(&p->lastPos, serializer);
    SerializeField(&p->lookAtDir, serializer);
    SerializeField(&p->navTarget, serializer);
}

std::int32_t AINavigationBlacklist::GetHashCode() const {
    return HashBuilder(2069u)
        .Add(entriesPtr)
        .Value();
}

void AINavigationBlacklist::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AINavigationBlacklist*>(ptr);
    SerializeList<NavBlacklistEntry>(&p->entriesPtr, serializer);
}

void AINavigationBlacklist::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (entriesPtr == Ptr::Null()) {
        entriesPtr = AllocateListPtr<NavBlacklistEntry>(f, 8);
    }
}

void AINavigationBlacklist::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (entriesPtr != Ptr::Null()) {
        f->FreeList(get_entries());
    }
    entriesPtr = Ptr::Null();
}

void AINavigationBlacklist::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AINavigationBlacklist*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void AINavigationBlacklist::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AINavigationBlacklist*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t AINavigationInitialiser::GetHashCode() const {
    return HashBuilder(2081u)
        .Add(spec)
        .Value();
}

void AINavigationInitialiser::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AINavigationInitialiser*>(ptr);
    SerializeField(&p->spec, serializer);
}

std::int32_t AISpawnDone::GetHashCode() const {
    return HashBuilder(2083u).Value();
}

void AISpawnDone::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t AISpawner::GetHashCode() const {
    return HashBuilder(2087u)
        .Add(Prototype)
        .Add(SpawnedEntitiesPtr)
        .Add(damageMultiplier)
        .Add(enabled)
        .Add(groupEntity)
        .Add(hpMultiplier)
        .Add(initialSpawnFrame)
        .Value();
}

void AISpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AISpawner*>(ptr);
    SerializeField(&p->initialSpawnFrame, serializer);
    SerializeField(&p->enabled, serializer);
    SerializeList<AISpawned>(&p->SpawnedEntitiesPtr, serializer);
    SerializeField(&p->groupEntity, serializer);
    SerializeField(&p->damageMultiplier, serializer);
    SerializeField(&p->hpMultiplier, serializer);
    SerializeField(&p->Prototype, serializer);
}

void AISpawner::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpawnedEntitiesPtr == Ptr::Null()) {
        SpawnedEntitiesPtr = AllocateListPtr<AISpawned>(f, 8);
    }
}

void AISpawner::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpawnedEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_SpawnedEntities());
    }
    SpawnedEntitiesPtr = Ptr::Null();
}

void AISpawner::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AISpawner*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void AISpawner::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AISpawner*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ActorIced::GetHashCode() const {
    return HashBuilder(2089u).Value();
}

void ActorIced::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ActorInput::GetHashCode() const {
    return HashBuilder(2099u)
        .Add(CameraPosition)
        .Add(CameraSight)
        .Add(DashButton)
        .Add(DashStateWanted)
        .Add(Direction)
        .Add(FireButton)
        .Add(FireStateWanted)
        .Add(GadgetButton)
        .Add(GadgetStateWanted)
        .Add(JumpButton)
        .Add(JumpStateWanted)
        .Add(LookInputVector)
        .Add(MeleeButton)
        .Add(MeleeStateWanted)
        .Add(ReloadButton)
        .Add(ReloadStateWanted)
        .Add(UltimateButton)
        .Add(UltimateStateWanted)
        .Add(areHumanInputs)
        .Add(inputsLocked)
        .Value();
}

void ActorInput::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActorInput*>(ptr);
    SerializeField(&p->inputsLocked, serializer);
    SerializeField(&p->DashStateWanted, serializer);
    SerializeField(&p->FireStateWanted, serializer);
    SerializeField(&p->GadgetStateWanted, serializer);
    SerializeField(&p->JumpStateWanted, serializer);
    SerializeField(&p->MeleeStateWanted, serializer);
    SerializeField(&p->ReloadStateWanted, serializer);
    SerializeField(&p->UltimateStateWanted, serializer);
    SerializeField(&p->areHumanInputs, serializer);
    SerializeField(&p->DashButton, serializer);
    SerializeField(&p->FireButton, serializer);
    SerializeField(&p->GadgetButton, serializer);
    SerializeField(&p->JumpButton, serializer);
    SerializeField(&p->MeleeButton, serializer);
    SerializeField(&p->ReloadButton, serializer);
    SerializeField(&p->UltimateButton, serializer);
    SerializeField(&p->Direction, serializer);
    SerializeField(&p->CameraPosition, serializer);
    SerializeField(&p->CameraSight, serializer);
    SerializeField(&p->LookInputVector, serializer);
}

std::int32_t ActorInputInitialiser::GetHashCode() const {
    return HashBuilder(2111u).Value();
}

void ActorInputInitialiser::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ActorManager::GetHashCode() const {
    return HashBuilder(2113u)
        .Add(Config)
        .Add(activeCheckpointsPtr)
        .Add(availableBotNickNamesPtr)
        .Add(initialSpawnPointsPtr)
        .Add(nbInitialSpawnPoints)
        .Add(podiumPointsPtr)
        .Add(respawnTimersPtr)
        .Add(respawnablePointsPtr)
        .Add(spawnPointDynamicDataPtr)
        .Value();
}

void ActorManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActorManager*>(ptr);
    SerializeField(&p->nbInitialSpawnPoints, serializer);
    SerializeList<ActiveCheckpoint>(&p->activeCheckpointsPtr, serializer);
    SerializeList<QString50>(&p->availableBotNickNamesPtr, serializer);
    SerializeList<std::int16_t>(&p->initialSpawnPointsPtr, serializer);
    SerializeList<std::int16_t>(&p->podiumPointsPtr, serializer);
    SerializeList<RespawnTimers>(&p->respawnTimersPtr, serializer);
    SerializeList<std::int16_t>(&p->respawnablePointsPtr, serializer);
    SerializeList<SpawnPointDynamicData>(&p->spawnPointDynamicDataPtr, serializer);
    SerializeField(&p->Config, serializer);
}

void ActorManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (activeCheckpointsPtr == Ptr::Null()) {
        activeCheckpointsPtr = AllocateListPtr<ActiveCheckpoint>(f, 8);
    }
    if (availableBotNickNamesPtr == Ptr::Null()) {
        availableBotNickNamesPtr = AllocateListPtr<QString50>(f, 8);
    }
    if (initialSpawnPointsPtr == Ptr::Null()) {
        initialSpawnPointsPtr = AllocateListPtr<std::int16_t>(f, 8);
    }
    if (podiumPointsPtr == Ptr::Null()) {
        podiumPointsPtr = AllocateListPtr<std::int16_t>(f, 8);
    }
    if (respawnTimersPtr == Ptr::Null()) {
        respawnTimersPtr = AllocateListPtr<RespawnTimers>(f, 8);
    }
    if (respawnablePointsPtr == Ptr::Null()) {
        respawnablePointsPtr = AllocateListPtr<std::int16_t>(f, 8);
    }
    if (spawnPointDynamicDataPtr == Ptr::Null()) {
        spawnPointDynamicDataPtr = AllocateListPtr<SpawnPointDynamicData>(f, 8);
    }
}

void ActorManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (activeCheckpointsPtr != Ptr::Null()) {
        f->FreeList(get_activeCheckpoints());
    }
    activeCheckpointsPtr = Ptr::Null();
    if (availableBotNickNamesPtr != Ptr::Null()) {
        f->FreeList(get_availableBotNickNames());
    }
    availableBotNickNamesPtr = Ptr::Null();
    if (initialSpawnPointsPtr != Ptr::Null()) {
        f->FreeList(get_initialSpawnPoints());
    }
    initialSpawnPointsPtr = Ptr::Null();
    if (podiumPointsPtr != Ptr::Null()) {
        f->FreeList(get_podiumPoints());
    }
    podiumPointsPtr = Ptr::Null();
    if (respawnTimersPtr != Ptr::Null()) {
        f->FreeList(get_respawnTimers());
    }
    respawnTimersPtr = Ptr::Null();
    if (respawnablePointsPtr != Ptr::Null()) {
        f->FreeList(get_respawnablePoints());
    }
    respawnablePointsPtr = Ptr::Null();
    if (spawnPointDynamicDataPtr != Ptr::Null()) {
        f->FreeList(get_spawnPointDynamicData());
    }
    spawnPointDynamicDataPtr = Ptr::Null();
}

void ActorManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ActorManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void ActorManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ActorManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ActorSheeped::GetHashCode() const {
    return HashBuilder(2129u).Value();
}

void ActorSheeped::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ActorStatus::GetHashCode() const {
    return HashBuilder(2131u)
        .Add(currentSurfaceType)
        .Add(currentType)
        .Add(previousType)
        .Add(spec)
        .Add(typeChangedFrame)
        .Add(wantedDuration)
        .Value();
}

void ActorStatus::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActorStatus*>(ptr);
    SerializeField(&p->currentType, serializer);
    SerializeField(&p->previousType, serializer);
    SerializeField(&p->typeChangedFrame, serializer);
    SerializeField(&p->currentSurfaceType, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->wantedDuration, serializer);
}

std::int32_t ActorStatusChangeRequested::GetHashCode() const {
    return HashBuilder(2137u)
        .Add(wantedDuration)
        .Add(wantedType)
        .Value();
}

void ActorStatusChangeRequested::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActorStatusChangeRequested*>(ptr);
    SerializeField(&p->wantedType, serializer);
    SerializeField(&p->wantedDuration, serializer);
}

std::int32_t ActorStatusModifier::GetHashCode() const {
    return HashBuilder(2141u)
        .Add(spec)
        .Value();
}

void ActorStatusModifier::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActorStatusModifier*>(ptr);
    SerializeField(&p->spec, serializer);
}

std::int32_t ActorStunned::GetHashCode() const {
    return HashBuilder(2143u).Value();
}

void ActorStunned::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t AddDamageIncreaseOnTargetedEffect::GetHashCode() const {
    return HashBuilder(2153u).Value();
}

void AddDamageIncreaseOnTargetedEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t AddEntitiesToSpawnOnProjectileDeath::GetHashCode() const {
    return HashBuilder(2161u)
        .Add(Spec)
        .Value();
}

void AddEntitiesToSpawnOnProjectileDeath::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AddEntitiesToSpawnOnProjectileDeath*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t AddForceOnUltimate::GetHashCode() const {
    return HashBuilder(2179u)
        .Add(delayIsOver)
        .Value();
}

void AddForceOnUltimate::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AddForceOnUltimate*>(ptr);
    SerializeField(&p->delayIsOver, serializer);
}

std::int32_t AgentOfDoom::GetHashCode() const {
    return HashBuilder(2203u).Value();
}

void AgentOfDoom::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t DoomGroupStats::GetHashCode() const {
    return HashBuilder(1439u)
        .Add(DoomGroupMaxMembers)
        .Add(Initialised)
        .Add(LastMaxedOutFrame)
        .Value();
}

void DoomGroupStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DoomGroupStats*>(ptr);
    SerializeField(&p->DoomGroupMaxMembers, serializer);
    SerializeField(&p->LastMaxedOutFrame, serializer);
    SerializeField(&p->Initialised, serializer);
}

std::int32_t AgentOfDoomGroup::GetHashCode() const {
    return HashBuilder(2207u)
        .Add(Stats)
        .Add(membersPtr)
        .Value();
}

void AgentOfDoomGroup::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AgentOfDoomGroup*>(ptr);
    SerializeList<DoomGroupMember>(&p->membersPtr, serializer);
    SerializeField(&p->Stats, serializer);
}

void AgentOfDoomGroup::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (membersPtr == Ptr::Null()) {
        membersPtr = AllocateListPtr<DoomGroupMember>(f, 8);
    }
}

void AgentOfDoomGroup::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (membersPtr != Ptr::Null()) {
        f->FreeList(get_members());
    }
    membersPtr = Ptr::Null();
}

void AgentOfDoomGroup::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AgentOfDoomGroup*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void AgentOfDoomGroup::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AgentOfDoomGroup*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t AgentOfDoomGrouped::GetHashCode() const {
    return HashBuilder(2213u).Value();
}

void AgentOfDoomGrouped::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t AmoeboidLauncherEnhancementPuddle::GetHashCode() const {
    return HashBuilder(2221u)
        .Add(Spec)
        .Value();
}

void AmoeboidLauncherEnhancementPuddle::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AmoeboidLauncherEnhancementPuddle*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t AoeTrail::GetHashCode() const {
    return HashBuilder(2237u)
        .Add(Spec)
        .Add(compoundShape)
        .Add(currentEndPosition)
        .Add(distanceTravelled)
        .Add(distanceWhenLastTrailSegmentWasDropped)
        .Add(followedEntity)
        .Add(pointsPtr)
        .Add(trailSegmentLength)
        .Add(trailWidth)
        .Value();
}

void AoeTrail::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AoeTrail*>(ptr);
    SerializeList<TrailPoint>(&p->pointsPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->followedEntity, serializer);
    SerializeField(&p->distanceTravelled, serializer);
    SerializeField(&p->distanceWhenLastTrailSegmentWasDropped, serializer);
    SerializeField(&p->trailSegmentLength, serializer);
    SerializeField(&p->trailWidth, serializer);
    SerializeField(&p->currentEndPosition, serializer);
    SerializeField(&p->compoundShape, serializer);
}

void AoeTrail::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pointsPtr == Ptr::Null()) {
        pointsPtr = AllocateListPtr<TrailPoint>(f, 8);
    }
}

void AoeTrail::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (pointsPtr != Ptr::Null()) {
        f->FreeList(get_points());
    }
    pointsPtr = Ptr::Null();
}

void AoeTrail::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AoeTrail*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void AoeTrail::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AoeTrail*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t AreaOfEffect::GetHashCode() const {
    return HashBuilder(2239u)
        .Add(CachedHitInfoPtr)
        .Add(Duration)
        .Add(EffectedEntitiesPtr)
        .Add(EnabledFrame)
        .Add(HasMissed)
        .Add(IsEnabled)
        .Add(IsRunning)
        .Add(KillInstantly)
        .Add(Range)
        .Add(RemainingDuration)
        .Add(Scale)
        .Add(ScaleCoefficient)
        .Add(Spec)
        .Add(StartFrame)
        .Value();
}

void AreaOfEffect::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AreaOfEffect*>(ptr);
    SerializeField(&p->EnabledFrame, serializer);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->HasMissed, serializer);
    SerializeField(&p->IsEnabled, serializer);
    SerializeField(&p->IsRunning, serializer);
    SerializeField(&p->KillInstantly, serializer);
    SerializeList<EffectHitInfo>(&p->CachedHitInfoPtr, serializer);
    SerializeList<EffectedEntity>(&p->EffectedEntitiesPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Duration, serializer);
    SerializeField(&p->Range, serializer);
    SerializeField(&p->RemainingDuration, serializer);
    SerializeField(&p->Scale, serializer);
    SerializeField(&p->ScaleCoefficient, serializer);
}

void AreaOfEffect::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (CachedHitInfoPtr == Ptr::Null()) {
        CachedHitInfoPtr = AllocateListPtr<EffectHitInfo>(f, 8);
    }
    if (EffectedEntitiesPtr == Ptr::Null()) {
        EffectedEntitiesPtr = AllocateListPtr<EffectedEntity>(f, 8);
    }
}

void AreaOfEffect::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (CachedHitInfoPtr != Ptr::Null()) {
        f->FreeList(get_CachedHitInfo());
    }
    CachedHitInfoPtr = Ptr::Null();
    if (EffectedEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_EffectedEntities());
    }
    EffectedEntitiesPtr = Ptr::Null();
}

void AreaOfEffect::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AreaOfEffect*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void AreaOfEffect::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<AreaOfEffect*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t AreaOfEffectIgnitionShapeHolder::GetHashCode() const {
    return HashBuilder(2243u)
        .Add(Scale)
        .Add(Shape)
        .Value();
}

void AreaOfEffectIgnitionShapeHolder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AreaOfEffectIgnitionShapeHolder*>(ptr);
    SerializeField(&p->Scale, serializer);
    SerializeField(&p->Shape, serializer);
}

std::int32_t AreaOfEffectLevelUp::GetHashCode() const {
    return HashBuilder(2251u)
        .Add(Spec)
        .Value();
}

void AreaOfEffectLevelUp::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AreaOfEffectLevelUp*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t AreaOfEffectRaycastInfo::GetHashCode() const {
    return HashBuilder(2267u)
        .Add(endPos)
        .Add(hitPoint)
        .Value();
}

void AreaOfEffectRaycastInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AreaOfEffectRaycastInfo*>(ptr);
    SerializeField(&p->endPos, serializer);
    SerializeField(&p->hitPoint, serializer);
}

std::int32_t ArenaCannon::GetHashCode() const {
    return HashBuilder(2269u)
        .Add(firing)
        .Add(startedToFireFrameNumber)
        .Value();
}

void ArenaCannon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ArenaCannon*>(ptr);
    SerializeField(&p->startedToFireFrameNumber, serializer);
    SerializeField(&p->firing, serializer);
}

std::int32_t AttackInfo::GetHashCode() const {
    return HashBuilder(2273u)
        .Add(isActive)
        .Add(targetEntity)
        .Add(wasRunningBeforeAttack)
        .Value();
}

void AttackInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AttackInfo*>(ptr);
    SerializeField(&p->isActive, serializer);
    SerializeField(&p->wasRunningBeforeAttack, serializer);
    SerializeField(&p->targetEntity, serializer);
}

std::int32_t AutoDestroyOnMapChanged::GetHashCode() const {
    return HashBuilder(2281u).Value();
}

void AutoDestroyOnMapChanged::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t AutoJumperStartParams::GetHashCode() const {
    return HashBuilder(1259u)
        .Add(airDrag)
        .Add(gravity)
        .Add(launchVelocity)
        .Add(travelTimeTotal)
        .Add(travelTimeUntilAirControlRestored)
        .Value();
}

void AutoJumperStartParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AutoJumperStartParams*>(ptr);
    SerializeField(&p->airDrag, serializer);
    SerializeField(&p->gravity, serializer);
    SerializeField(&p->travelTimeTotal, serializer);
    SerializeField(&p->travelTimeUntilAirControlRestored, serializer);
    SerializeField(&p->launchVelocity, serializer);
}

std::int32_t AutoJumperActor::GetHashCode() const {
    return HashBuilder(2287u)
        .Add(startFrame)
        .Add(startParams)
        .Add(velocity)
        .Value();
}

void AutoJumperActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AutoJumperActor*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->velocity, serializer);
    SerializeField(&p->startParams, serializer);
}

std::int32_t BTAgent::GetHashCode() const {
    return HashBuilder(2293u)
        .Add(AbortNodeId)
        .Add(ActiveServicesPtr)
        .Add(BTDataValuesPtr)
        .Add(Config)
        .Add(Current)
        .Add(DynamicCompositesPtr)
        .Add(NodesStatusPtr)
        .Add(ServicesEndTimesPtr)
        .Add(Tree)
        .Value();
}

void BTAgent::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BTAgent*>(ptr);
    SerializeField(&p->AbortNodeId, serializer);
    SerializeList<AssetRefBTService>(&p->ActiveServicesPtr, serializer);
    SerializeList<BTDataValue>(&p->BTDataValuesPtr, serializer);
    SerializeList<AssetRefBTComposite>(&p->DynamicCompositesPtr, serializer);
    SerializeList<std::uint8_t>(&p->NodesStatusPtr, serializer);
    SerializeList<FP>(&p->ServicesEndTimesPtr, serializer);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->Current, serializer);
    SerializeField(&p->Tree, serializer);
}

void BTAgent::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    DynamicCompositesPtr = Ptr::Null();
    NodesStatusPtr = Ptr::Null();
    ActiveServicesPtr = Ptr::Null();
    BTDataValuesPtr = Ptr::Null();
    ServicesEndTimesPtr = Ptr::Null();
}

void BTAgent::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BTAgent*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t BattleStatsActor::GetHashCode() const {
    return HashBuilder(2297u)
        .Add(damageTakenFromBots)
        .Add(damageTakenFromHumanPlayers)
        .Add(mobTypeKilledCountsPtr)
        .Add(nbDeathEnviro)
        .Add(nbDeathFromBotPlayers)
        .Add(nbDeathFromHumanPlayers)
        .Add(nbDeathPve)
        .Add(numAmmoDepletedReloads)
        .Add(numDash)
        .Add(numDoubleJumps)
        .Add(numGadgetUse)
        .Add(numGlide)
        .Add(numGoalsScored)
        .Add(numJumpersUsed)
        .Add(numJumps)
        .Add(numManualReloads)
        .Add(numRailSlides)
        .Add(numUltimateUse)
        .Add(numWallRuns)
        .Add(numWeaponSwaps)
        .Add(numWeaponSwaps_0)
        .Add(powerupStatsPtr)
        .Add(pveDamageDealt)
        .Add(pveDamageTaken)
        .Add(pveMeleeDamageDealt)
        .Add(pvpDamageDealt)
        .Add(pvpMeleeDamageDealt)
        .Add(totalBoltsCollected)
        .Add(totalGoldenBoltsCollected)
        .Add(totalGrindSeconds)
        .Add(weaponUsageStatsPtr)
        .Value();
}

void BattleStatsActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BattleStatsActor*>(ptr);
    SerializeField(&p->nbDeathEnviro, serializer);
    SerializeField(&p->nbDeathFromBotPlayers, serializer);
    SerializeField(&p->nbDeathFromHumanPlayers, serializer);
    SerializeField(&p->nbDeathPve, serializer);
    SerializeField(&p->numAmmoDepletedReloads, serializer);
    SerializeField(&p->numDash, serializer);
    SerializeField(&p->numDoubleJumps, serializer);
    SerializeField(&p->numGadgetUse, serializer);
    SerializeField(&p->numGlide, serializer);
    SerializeField(&p->numGoalsScored, serializer);
    SerializeField(&p->numJumpersUsed, serializer);
    SerializeField(&p->numJumps, serializer);
    SerializeField(&p->numManualReloads, serializer);
    SerializeField(&p->numRailSlides, serializer);
    SerializeField(&p->numUltimateUse, serializer);
    SerializeField(&p->numWallRuns, serializer);
    SerializeField(&p->numWeaponSwaps, serializer);
    SerializeField(&p->numWeaponSwaps_0, serializer);
    SerializeField(&p->totalBoltsCollected, serializer);
    SerializeField(&p->totalGoldenBoltsCollected, serializer);
    SerializeDictionary<EntityInfoType, std::int32_t>(&p->mobTypeKilledCountsPtr, serializer);
    SerializeList<PowerUpStats>(&p->powerupStatsPtr, serializer);
    SerializeDictionary<GENERATED_WEAPON_ID, WeaponUsageStats>(&p->weaponUsageStatsPtr, serializer);
    SerializeField(&p->damageTakenFromBots, serializer);
    SerializeField(&p->damageTakenFromHumanPlayers, serializer);
    SerializeField(&p->pveDamageDealt, serializer);
    SerializeField(&p->pveDamageTaken, serializer);
    SerializeField(&p->pveMeleeDamageDealt, serializer);
    SerializeField(&p->pvpDamageDealt, serializer);
    SerializeField(&p->pvpMeleeDamageDealt, serializer);
    SerializeField(&p->totalGrindSeconds, serializer);
}

void BattleStatsActor::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (weaponUsageStatsPtr == Ptr::Null()) {
        weaponUsageStatsPtr = AllocateEnumDictionaryPtr<GENERATED_WEAPON_ID, WeaponUsageStats>(f, 8);
    }
}

void BattleStatsActor::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (mobTypeKilledCountsPtr != Ptr::Null()) {
        f->FreeDictionary(get_mobTypeKilledCounts());
    }
    mobTypeKilledCountsPtr = Ptr::Null();
    if (powerupStatsPtr != Ptr::Null()) {
        f->FreeList(get_powerupStats());
    }
    powerupStatsPtr = Ptr::Null();
    if (weaponUsageStatsPtr != Ptr::Null()) {
        f->FreeDictionary(get_weaponUsageStats());
    }
    weaponUsageStatsPtr = Ptr::Null();
}

void BattleStatsActor::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BattleStatsActor*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void BattleStatsActor::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BattleStatsActor*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t BigBoomMine::GetHashCode() const {
    return HashBuilder(2309u)
        .Add(FallingSpeed)
        .Add(IsStuck)
        .Add(Spec)
        .Add(StuckTo)
        .Add(TriggeredFrame)
        .Value();
}

void BigBoomMine::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BigBoomMine*>(ptr);
    SerializeField(&p->TriggeredFrame, serializer);
    SerializeField(&p->IsStuck, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->StuckTo, serializer);
    SerializeField(&p->FallingSpeed, serializer);
}

std::int32_t BigBoomWeapon::GetHashCode() const {
    return HashBuilder(2311u).Value();
}

void BigBoomWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t BlackboardInitializer::GetHashCode() const {
    return HashBuilder(2333u)
        .Add(BBInitializer)
        .Value();
}

void BlackboardInitializer::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlackboardInitializer*>(ptr);
    SerializeField(&p->BBInitializer, serializer);
}

std::int32_t LastBallHitData::GetHashCode() const {
    return HashBuilder(1613u)
        .Add(frame)
        .Add(player)
        .Add(weapon)
        .Value();
}

void LastBallHitData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LastBallHitData*>(ptr);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->player, serializer);
    SerializeField(&p->weapon, serializer);
}

std::int32_t BlastBall::GetHashCode() const {
    return HashBuilder(2339u)
        .Add(Spec)
        .Add(dashingPlayersPtr)
        .Add(initialDrag)
        .Add(lastHitTeamA)
        .Add(lastHitTeamB)
        .Add(lockRotation)
        .Add(lockRotationFrame)
        .Add(shotFrame)
        .Add(surfaceNormal)
        .Value();
}

void BlastBall::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlastBall*>(ptr);
    SerializeField(&p->lockRotationFrame, serializer);
    SerializeField(&p->shotFrame, serializer);
    SerializeList<CollidingEntity>(&p->dashingPlayersPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->initialDrag, serializer);
    SerializeField(&p->surfaceNormal, serializer);
    SerializeField(&p->lastHitTeamA, serializer);
    SerializeField(&p->lastHitTeamB, serializer);
    SerializeField(&p->lockRotation, serializer);
}

void BlastBall::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (dashingPlayersPtr == Ptr::Null()) {
        dashingPlayersPtr = AllocateListPtr<CollidingEntity>(f, 8);
    }
}

void BlastBall::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (dashingPlayersPtr != Ptr::Null()) {
        f->FreeList(get_dashingPlayers());
    }
    dashingPlayersPtr = Ptr::Null();
}

void BlastBall::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BlastBall*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void BlastBall::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BlastBall*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t BlastBallTeamData::GetHashCode() const {
    return HashBuilder(1283u)
        .Add(goalPosition)
        .Add(score)
        .Add(state)
        .Value();
}

void BlastBallTeamData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlastBallTeamData*>(ptr);
    SerializeField(&p->score, serializer);
    SerializeField(&p->state, serializer);
    SerializeField(&p->goalPosition, serializer);
}

std::int32_t BlastBallGame::GetHashCode() const {
    return HashBuilder(2341u)
        .Add(ballEntity)
        .Add(ballPosition)
        .Add(isOverTime)
        .Add(lastScoringPlayer)
        .Add(lastScoringTeam)
        .Add(previousState)
        .Add(resetValuesPtr)
        .Add(scoringFramesPtr)
        .Add(state)
        .Add(stateStartFrame)
        .Add(teamA)
        .Add(teamB)
        .Value();
}

void BlastBallGame::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlastBallGame*>(ptr);
    SerializeField(&p->previousState, serializer);
    SerializeField(&p->state, serializer);
    SerializeField(&p->stateStartFrame, serializer);
    SerializeField(&p->isOverTime, serializer);
    SerializeDictionary<EntityRef, ResetValues>(&p->resetValuesPtr, serializer);
    SerializeList<GoalScoreFrame>(&p->scoringFramesPtr, serializer);
    SerializeField(&p->lastScoringTeam, serializer);
    SerializeField(&p->ballEntity, serializer);
    SerializeField(&p->lastScoringPlayer, serializer);
    SerializeField(&p->ballPosition, serializer);
    SerializeField(&p->teamA, serializer);
    SerializeField(&p->teamB, serializer);
}

void BlastBallGame::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (resetValuesPtr == Ptr::Null()) {
        resetValuesPtr = AllocateDictionaryPtr<EntityRef, ResetValues>(f, 8);
    }
    if (scoringFramesPtr == Ptr::Null()) {
        scoringFramesPtr = AllocateListPtr<GoalScoreFrame>(f, 8);
    }
}

void BlastBallGame::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (resetValuesPtr != Ptr::Null()) {
        f->FreeDictionary(get_resetValues());
    }
    resetValuesPtr = Ptr::Null();
    if (scoringFramesPtr != Ptr::Null()) {
        f->FreeList(get_scoringFrames());
    }
    scoringFramesPtr = Ptr::Null();
}

void BlastBallGame::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BlastBallGame*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void BlastBallGame::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BlastBallGame*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t BlastBallGoal::GetHashCode() const {
    return HashBuilder(2347u)
        .Add(OwnerTeam)
        .Add(ballEntity)
        .Add(ballInside)
        .Value();
}

void BlastBallGoal::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlastBallGoal*>(ptr);
    SerializeField(&p->ballInside, serializer);
    SerializeField(&p->OwnerTeam, serializer);
    SerializeField(&p->ballEntity, serializer);
}

std::int32_t BlastBallPlayer::GetHashCode() const {
    return HashBuilder(2351u)
        .Add(goals)
        .Add(meleeGoals)
        .Add(rangedGoals)
        .Value();
}

void BlastBallPlayer::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlastBallPlayer*>(ptr);
    SerializeField(&p->goals, serializer);
    SerializeField(&p->meleeGoals, serializer);
    SerializeField(&p->rangedGoals, serializer);
}

std::int32_t BlasterMoveSpeedEnhancement::GetHashCode() const {
    return HashBuilder(2357u)
        .Add(MoveSpeedModifierIsApplied)
        .Add(Spec)
        .Value();
}

void BlasterMoveSpeedEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlasterMoveSpeedEnhancement*>(ptr);
    SerializeField(&p->MoveSpeedModifierIsApplied, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t BlasterSpecialShotEnhancement::GetHashCode() const {
    return HashBuilder(2371u)
        .Add(OriginalAmmoAsset)
        .Add(ShotCounter)
        .Add(Spec)
        .Value();
}

void BlasterSpecialShotEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlasterSpecialShotEnhancement*>(ptr);
    SerializeField(&p->ShotCounter, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->OriginalAmmoAsset, serializer);
}

std::int32_t BoltCounterElement::GetHashCode() const {
    return HashBuilder(2377u)
        .Add(currentBolts)
        .Add(gameplayElementConfig)
        .Add(reachedRequired)
        .Add(requiredBolts)
        .Value();
}

void BoltCounterElement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BoltCounterElement*>(ptr);
    SerializeField(&p->currentBolts, serializer);
    SerializeField(&p->requiredBolts, serializer);
    SerializeField(&p->reachedRequired, serializer);
    SerializeField(&p->gameplayElementConfig, serializer);
}

std::int32_t BoltGathererActor::GetHashCode() const {
    return HashBuilder(2381u)
        .Add(BoltsCollected)
        .Add(PreventBoltCollectionEndFrame)
        .Add(Spec)
        .Add(hasTeamCounter)
        .Value();
}

void BoltGathererActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BoltGathererActor*>(ptr);
    SerializeField(&p->BoltsCollected, serializer);
    SerializeField(&p->hasTeamCounter, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->PreventBoltCollectionEndFrame, serializer);
}

std::int32_t BoltPowerActor::GetHashCode() const {
    return HashBuilder(2383u)
        .Add(Spec)
        .Add(highestLevelReached)
        .Add(level)
        .Add(maxLevel)
        .Value();
}

void BoltPowerActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BoltPowerActor*>(ptr);
    SerializeField(&p->highestLevelReached, serializer);
    SerializeField(&p->level, serializer);
    SerializeField(&p->maxLevel, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t BombardierDrone::GetHashCode() const {
    return HashBuilder(2389u)
        .Add(Spec)
        .Add(bombDropCount)
        .Add(cachedHeight)
        .Add(fallingSpeed)
        .Add(lastBombDropSide)
        .Add(startFrame)
        .Add(state)
        .Value();
}

void BombardierDrone::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BombardierDrone*>(ptr);
    SerializeField(&p->state, serializer);
    SerializeField(&p->lastBombDropSide, serializer);
    SerializeField(&p->bombDropCount, serializer);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->cachedHeight, serializer);
    SerializeField(&p->fallingSpeed, serializer);
}

std::int32_t BombardierWeapon::GetHashCode() const {
    return HashBuilder(2393u)
        .Add(Spec)
        .Value();
}

void BombardierWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BombardierWeapon*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t BotAirSaveParams::GetHashCode() const {
    return HashBuilder(1289u)
        .Add(affinity)
        .Add(cooldownTimer)
        .Add(dashTimer)
        .Add(directionToMaintain)
        .Add(maintainDirectionTimer)
        .Value();
}

void BotAirSaveParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotAirSaveParams*>(ptr);
    SerializeField(&p->affinity, serializer);
    SerializeField(&p->cooldownTimer, serializer);
    SerializeField(&p->dashTimer, serializer);
    SerializeField(&p->maintainDirectionTimer, serializer);
    SerializeField(&p->directionToMaintain, serializer);
}

std::int32_t BotBlastBallParams::GetHashCode() const {
    return HashBuilder(1291u)
        .Add(blastBallDashCooldownTimer)
        .Add(defensivePositionComputeFrame)
        .Add(inShootingZoneLastRepositionFrame)
        .Add(inShootingZonePositionChangeCooldownTimer)
        .Add(passingTarget)
        .Add(passingTargetPos)
        .Add(shootDistance)
        .Add(targetBBPosition)
        .Add(targetDefensivePosition)
        .Add(targetEnemyGoal)
        .Add(targetStrikePos)
        .Value();
}

void BotBlastBallParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotBlastBallParams*>(ptr);
    SerializeField(&p->defensivePositionComputeFrame, serializer);
    SerializeField(&p->inShootingZoneLastRepositionFrame, serializer);
    SerializeField(&p->passingTarget, serializer);
    SerializeField(&p->blastBallDashCooldownTimer, serializer);
    SerializeField(&p->inShootingZonePositionChangeCooldownTimer, serializer);
    SerializeField(&p->shootDistance, serializer);
    SerializeField(&p->passingTargetPos, serializer);
    SerializeField(&p->targetBBPosition, serializer);
    SerializeField(&p->targetDefensivePosition, serializer);
    SerializeField(&p->targetEnemyGoal, serializer);
    SerializeField(&p->targetStrikePos, serializer);
}

std::int32_t BotShootingParams::GetHashCode() const {
    return HashBuilder(1303u)
        .Add(missChance_0_100)
        .Add(missChance_0_100_BehindHuman)
        .Add(successiveShotsCooldownMax)
        .Add(successiveShotsCooldownMin)
        .Add(targetOffset)
        .Value();
}

void BotShootingParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotShootingParams*>(ptr);
    SerializeField(&p->missChance_0_100, serializer);
    SerializeField(&p->missChance_0_100_BehindHuman, serializer);
    SerializeField(&p->successiveShotsCooldownMax, serializer);
    SerializeField(&p->successiveShotsCooldownMin, serializer);
    SerializeField(&p->targetOffset, serializer);
}

std::int32_t FireRequest::GetHashCode() const {
    return HashBuilder(1487u)
        .Add(type)
        .Value();
}

void FireRequest::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<FireRequest*>(ptr);
    SerializeField(&p->type, serializer);
}

std::int32_t ZoneControlBotParams::GetHashCode() const {
    return HashBuilder(1999u)
        .Add(repathTimer)
        .Add(targetZonePosition)
        .Value();
}

void ZoneControlBotParams::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ZoneControlBotParams*>(ptr);
    SerializeField(&p->repathTimer, serializer);
    SerializeField(&p->targetZonePosition, serializer);
}

std::int32_t Bot::GetHashCode() const {
    return HashBuilder(2399u)
        .Add(UTActionEntity)
        .Add(UTDebugActionPtr)
        .Add(UTDebugMovePtr)
        .Add(UTMoveEntity)
        .Add(airSave)
        .Add(attackingTarget)
        .Add(attackingTargetCoolDownTimer)
        .Add(attackingTargetPos)
        .Add(blankPushesListPtr)
        .Add(blastBall)
        .Add(currentAggroBehaviour)
        .Add(destination)
        .Add(destinationTransitionPoint)
        .Add(difficulty_0_1)
        .Add(disabled)
        .Add(disabledEndFrame)
        .Add(droppableAwarenessRangeSq)
        .Add(fireRequestCooldown)
        .Add(fireRequestCounter)
        .Add(forceAttackPosComputationSuccess)
        .Add(goldenBoltUnreachablePos)
        .Add(groupTeamNavPos)
        .Add(hasReachedTransitionPoint)
        .Add(inRangeDroppablesPtr)
        .Add(lastGroundedPosition)
        .Add(lastRepositionFrame)
        .Add(nextRepositionFrame)
        .Add(nickname)
        .Add(pendingFireRequest)
        .Add(presetIndex)
        .Add(randomJumpTimer)
        .Add(seekForbiddenEndFrame)
        .Add(shootingParams)
        .Add(targetCrate)
        .Add(targetDroppableId)
        .Add(targetDroppableIndex)
        .Add(targetDroppableNavPos)
        .Add(targetGoldenBolt)
        .Add(targetGoldenBoltNavPos)
        .Add(targetKingPos)
        .Add(targetObjectivePos)
        .Add(targetWrenchCombo)
        .Add(useGadgetCooldownEndFrame)
        .Add(useTransitionPoint)
        .Add(useWrenchCooldownTimer)
        .Add(useWrenchRequestedFrame)
        .Add(weaponSwitchCooldownTimer)
        .Add(weaponSwitchExcludedTags)
        .Add(weaponSwitchNoCombatActivityDelay)
        .Add(zoneControl)
        .Value();
}

void Bot::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Bot*>(ptr);
    SerializeField(&p->presetIndex, serializer);
    SerializeField(&p->targetDroppableId, serializer);
    SerializeField(&p->targetDroppableIndex, serializer);
    SerializeField(&p->targetWrenchCombo, serializer);
    SerializeField(&p->currentAggroBehaviour, serializer);
    SerializeField(&p->pendingFireRequest, serializer);
    SerializeField(&p->disabledEndFrame, serializer);
    SerializeField(&p->lastRepositionFrame, serializer);
    SerializeField(&p->nextRepositionFrame, serializer);
    SerializeField(&p->seekForbiddenEndFrame, serializer);
    SerializeField(&p->useGadgetCooldownEndFrame, serializer);
    SerializeField(&p->useWrenchRequestedFrame, serializer);
    SerializeField(&p->weaponSwitchExcludedTags, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->forceAttackPosComputationSuccess, serializer);
    SerializeField(&p->hasReachedTransitionPoint, serializer);
    SerializeField(&p->useTransitionPoint, serializer);
    SerializeDictionary<QString50, BotUTData>(&p->UTDebugActionPtr, serializer);
    SerializeDictionary<QString50, BotUTData>(&p->UTDebugMovePtr, serializer);
    SerializeList<EntityRef>(&p->blankPushesListPtr, serializer);
    SerializeDictionary<std::uint16_t, std::uint16_t>(&p->inRangeDroppablesPtr, serializer);
    SerializeField(&p->UTActionEntity, serializer);
    SerializeField(&p->UTMoveEntity, serializer);
    SerializeField(&p->attackingTarget, serializer);
    SerializeField(&p->targetCrate, serializer);
    SerializeField(&p->targetGoldenBolt, serializer);
    SerializeField(&p->attackingTargetCoolDownTimer, serializer);
    SerializeField(&p->difficulty_0_1, serializer);
    SerializeField(&p->droppableAwarenessRangeSq, serializer);
    SerializeField(&p->fireRequestCooldown, serializer);
    SerializeField(&p->fireRequestCounter, serializer);
    SerializeField(&p->randomJumpTimer, serializer);
    SerializeField(&p->useWrenchCooldownTimer, serializer);
    SerializeField(&p->weaponSwitchCooldownTimer, serializer);
    SerializeField(&p->weaponSwitchNoCombatActivityDelay, serializer);
    SerializeField(&p->attackingTargetPos, serializer);
    SerializeField(&p->destination, serializer);
    SerializeField(&p->destinationTransitionPoint, serializer);
    SerializeField(&p->goldenBoltUnreachablePos, serializer);
    SerializeField(&p->groupTeamNavPos, serializer);
    SerializeField(&p->lastGroundedPosition, serializer);
    SerializeField(&p->targetDroppableNavPos, serializer);
    SerializeField(&p->targetGoldenBoltNavPos, serializer);
    SerializeField(&p->targetKingPos, serializer);
    SerializeField(&p->targetObjectivePos, serializer);
    SerializeField(&p->zoneControl, serializer);
    SerializeField(&p->nickname, serializer);
    SerializeField(&p->airSave, serializer);
    SerializeField(&p->shootingParams, serializer);
    SerializeField(&p->blastBall, serializer);
}

void Bot::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (UTDebugActionPtr == Ptr::Null()) {
        UTDebugActionPtr = AllocateDictionaryPtr<QString50, BotUTData>(f, 8);
    }
    if (UTDebugMovePtr == Ptr::Null()) {
        UTDebugMovePtr = AllocateDictionaryPtr<QString50, BotUTData>(f, 8);
    }
    if (blankPushesListPtr == Ptr::Null()) {
        blankPushesListPtr = AllocateListPtr<EntityRef>(f, 8);
    }
    if (inRangeDroppablesPtr == Ptr::Null()) {
        inRangeDroppablesPtr = AllocateDictionaryPtr<std::uint16_t, std::uint16_t>(f, 8);
    }
}

void Bot::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (UTDebugActionPtr != Ptr::Null()) {
        f->FreeDictionary(get_UTDebugAction());
    }
    UTDebugActionPtr = Ptr::Null();
    if (UTDebugMovePtr != Ptr::Null()) {
        f->FreeDictionary(get_UTDebugMove());
    }
    UTDebugMovePtr = Ptr::Null();
    if (blankPushesListPtr != Ptr::Null()) {
        f->FreeList(get_blankPushesList());
    }
    blankPushesListPtr = Ptr::Null();
    if (inRangeDroppablesPtr != Ptr::Null()) {
        f->FreeDictionary(get_inRangeDroppables());
    }
    inRangeDroppablesPtr = Ptr::Null();
}

void Bot::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Bot*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Bot::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Bot*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Bumper::GetHashCode() const {
    return HashBuilder(2411u)
        .Add(config)
        .Add(entity)
        .Add(impulseDirection)
        .Add(impulseStrength)
        .Value();
}

void Bumper::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Bumper*>(ptr);
    SerializeField(&p->config, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->impulseStrength, serializer);
    SerializeField(&p->impulseDirection, serializer);
}

std::int32_t BumperBar::GetHashCode() const {
    return HashBuilder(2417u)
        .Add(Config)
        .Add(entity)
        .Value();
}

void BumperBar::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BumperBar*>(ptr);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t BuzzbladesRicochet::GetHashCode() const {
    return HashBuilder(2423u)
        .Add(Spec)
        .Add(changeTargetFrame)
        .Add(targetsPtr)
        .Value();
}

void BuzzbladesRicochet::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BuzzbladesRicochet*>(ptr);
    SerializeField(&p->changeTargetFrame, serializer);
    SerializeList<RicochetTargetInfo>(&p->targetsPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void BuzzbladesRicochet::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (targetsPtr == Ptr::Null()) {
        targetsPtr = AllocateListPtr<RicochetTargetInfo>(f, 8);
    }
}

void BuzzbladesRicochet::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (targetsPtr != Ptr::Null()) {
        f->FreeList(get_targets());
    }
    targetsPtr = Ptr::Null();
}

void BuzzbladesRicochet::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BuzzbladesRicochet*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void BuzzbladesRicochet::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<BuzzbladesRicochet*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t CharacterColliderScaler::GetHashCode() const {
    return HashBuilder(2437u)
        .Add(CurrentScaleFactor)
        .Add(OriginExtents)
        .Add(OriginRadius)
        .Add(OriginSphereHeight)
        .Value();
}

void CharacterColliderScaler::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CharacterColliderScaler*>(ptr);
    SerializeField(&p->CurrentScaleFactor, serializer);
    SerializeField(&p->OriginRadius, serializer);
    SerializeField(&p->OriginSphereHeight, serializer);
    SerializeField(&p->OriginExtents, serializer);
}

std::int32_t ChargeAttack::GetHashCode() const {
    return HashBuilder(2441u)
        .Add(Spec)
        .Add(changeStateFrame)
        .Add(movementStarted)
        .Add(startPosition)
        .Add(state)
        .Value();
}

void ChargeAttack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ChargeAttack*>(ptr);
    SerializeField(&p->state, serializer);
    SerializeField(&p->changeStateFrame, serializer);
    SerializeField(&p->movementStarted, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->startPosition, serializer);
}

std::int32_t CheatManager::GetHashCode() const {
    return HashBuilder(2447u)
        .Add(finishRaceFast)
        .Value();
}

void CheatManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CheatManager*>(ptr);
    SerializeField(&p->finishRaceFast, serializer);
}

std::int32_t Cheats::GetHashCode() const {
    return HashBuilder(2459u)
        .Add(godModeEnabled)
        .Add(infiniteGadget)
        .Add(infiniteUltimate)
        .Add(noClipEnabled)
        .Value();
}

void Cheats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Cheats*>(ptr);
    SerializeField(&p->godModeEnabled, serializer);
    SerializeField(&p->infiniteGadget, serializer);
    SerializeField(&p->infiniteUltimate, serializer);
    SerializeField(&p->noClipEnabled, serializer);
}

std::int32_t Checkpoint::GetHashCode() const {
    return HashBuilder(2467u).Value();
}

void Checkpoint::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t ColliderResize::GetHashCode() const {
    return HashBuilder(2473u)
        .Add(CurrentCentroid)
        .Add(CurrentExtents)
        .Add(OriginCentroid)
        .Add(OriginExtents)
        .Add(SpawnFrame)
        .Add(Spec)
        .Add(Status)
        .Value();
}

void ColliderResize::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ColliderResize*>(ptr);
    SerializeField(&p->Status, serializer);
    SerializeField(&p->SpawnFrame, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->CurrentCentroid, serializer);
    SerializeField(&p->CurrentExtents, serializer);
    SerializeField(&p->OriginCentroid, serializer);
    SerializeField(&p->OriginExtents, serializer);
}

std::int32_t CollisionForce::GetHashCode() const {
    return HashBuilder(2477u)
        .Add(Spec)
        .Add(collidedDamageablesPtr)
        .Add(damageAmountToDealThisFrame)
        .Add(lastGroundedFrame)
        .Add(pushAmountToDealThisFrame)
        .Add(pushSourceIsCollisionForceComponent)
        .Value();
}

void CollisionForce::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CollisionForce*>(ptr);
    SerializeField(&p->lastGroundedFrame, serializer);
    SerializeField(&p->pushSourceIsCollisionForceComponent, serializer);
    SerializeList<CollidingEntity>(&p->collidedDamageablesPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->damageAmountToDealThisFrame, serializer);
    SerializeField(&p->pushAmountToDealThisFrame, serializer);
}

void CollisionForce::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (collidedDamageablesPtr == Ptr::Null()) {
        collidedDamageablesPtr = AllocateListPtr<CollidingEntity>(f, 8);
    }
}

void CollisionForce::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (collidedDamageablesPtr != Ptr::Null()) {
        f->FreeList(get_collidedDamageables());
    }
    collidedDamageablesPtr = Ptr::Null();
}

void CollisionForce::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<CollisionForce*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void CollisionForce::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<CollisionForce*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t CombatParticipant::GetHashCode() const {
    return HashBuilder(2503u).Value();
}

void CombatParticipant::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t HFSMData::GetHashCode() const {
    return HashBuilder(1553u)
        .Add(CurrentState)
        .Add(Root)
        .Add(StatesHistory)
        .Add(Time)
        .Add(_Times_)
        .Add(UsesStateHistory)
        .Value();
}

void HFSMData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HFSMData*>(ptr);
    SerializeField(&p->StatesHistory, serializer);
    SerializeField(&p->UsesStateHistory, serializer);
    SerializeField(&p->Root, serializer);
    SerializeField(&p->CurrentState, serializer);
    SerializeField(&p->Time, serializer);
    SerializeField(&p->_Times_, serializer);
}

std::int32_t HFSMAgent::GetHashCode() const {
    return HashBuilder(2803u)
        .Add(Config)
        .Add(Data)
        .Value();
}

void HFSMAgent::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HFSMAgent*>(ptr);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->Data, serializer);
}

std::int32_t RuntimeHFSM::GetHashCode() const {
    return HashBuilder(1789u)
        .Add(agent)
        .Add(blackboard)
        .Value();
}

void RuntimeHFSM::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RuntimeHFSM*>(ptr);
    SerializeField(&p->blackboard, serializer);
    SerializeField(&p->agent, serializer);
}

std::int32_t CompoundHFSM::GetHashCode() const {
    return HashBuilder(2521u)
        .Add(attackHFSM)
        .Add(navHFSM)
        .Add(schedulePeriod)
        .Add(spec)
        .Value();
}

void CompoundHFSM::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CompoundHFSM*>(ptr);
    SerializeField(&p->schedulePeriod, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->attackHFSM, serializer);
    SerializeField(&p->navHFSM, serializer);
}

void CompoundHFSM::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<CompoundHFSM*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t Cryoslider::GetHashCode() const {
    return HashBuilder(2531u)
        .Add(Spec)
        .Add(distanceTravelled)
        .Add(lastFloorHeightTouched)
        .Value();
}

void Cryoslider::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Cryoslider*>(ptr);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->distanceTravelled, serializer);
    SerializeField(&p->lastFloorHeightTouched, serializer);
}

std::int32_t CryosliderWeapon::GetHashCode() const {
    return HashBuilder(2539u)
        .Add(Spec)
        .Value();
}

void CryosliderWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CryosliderWeapon*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t DamageChargeable::GetHashCode() const {
    return HashBuilder(2543u)
        .Add(Spec)
        .Add(activatedFrame)
        .Add(chargeAmount)
        .Add(lastHitFrame)
        .Value();
}

void DamageChargeable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageChargeable*>(ptr);
    SerializeField(&p->activatedFrame, serializer);
    SerializeField(&p->lastHitFrame, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->chargeAmount, serializer);
}

std::int32_t DamageIncreasedOnTargetsEnhancement::GetHashCode() const {
    return HashBuilder(2549u)
        .Add(DamageIncreaseByTargetPtr)
        .Add(Spec)
        .Value();
}

void DamageIncreasedOnTargetsEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageIncreasedOnTargetsEnhancement*>(ptr);
    SerializeDictionary<EntityRef, DamageIncreaseData>(&p->DamageIncreaseByTargetPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void DamageIncreasedOnTargetsEnhancement::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (DamageIncreaseByTargetPtr == Ptr::Null()) {
        DamageIncreaseByTargetPtr = AllocateDictionaryPtr<EntityRef, DamageIncreaseData>(f, 8);
    }
}

void DamageIncreasedOnTargetsEnhancement::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (DamageIncreaseByTargetPtr != Ptr::Null()) {
        f->FreeDictionary(get_DamageIncreaseByTarget());
    }
    DamageIncreaseByTargetPtr = Ptr::Null();
}

void DamageIncreasedOnTargetsEnhancement::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DamageIncreasedOnTargetsEnhancement*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void DamageIncreasedOnTargetsEnhancement::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DamageIncreasedOnTargetsEnhancement*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t DamageMultiplier::GetHashCode() const {
    return HashBuilder(2551u)
        .Add(Spec)
        .Value();
}

void DamageMultiplier::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageMultiplier*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t DamageMultiplierHeroImprovement::GetHashCode() const {
    return HashBuilder(2557u).Value();
}

void DamageMultiplierHeroImprovement::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t DamageSource::GetHashCode() const {
    return HashBuilder(2579u)
        .Add(AmmunitionSpec)
        .Add(Spec)
        .Add(hitTrackingDone)
        .Add(multipliersPtr)
        .Value();
}

void DamageSource::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageSource*>(ptr);
    SerializeField(&p->hitTrackingDone, serializer);
    SerializeList<DamageMultiplierData>(&p->multipliersPtr, serializer);
    SerializeField(&p->AmmunitionSpec, serializer);
    SerializeField(&p->Spec, serializer);
}

void DamageSource::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (multipliersPtr == Ptr::Null()) {
        multipliersPtr = AllocateListPtr<DamageMultiplierData>(f, 8);
    }
}

void DamageSource::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (multipliersPtr != Ptr::Null()) {
        f->FreeList(get_multipliers());
    }
    multipliersPtr = Ptr::Null();
}

void DamageSource::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DamageSource*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void DamageSource::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DamageSource*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

} // namespace Quantum
