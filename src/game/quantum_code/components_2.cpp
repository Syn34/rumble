// components_2.cpp - GetHashCode / Serialize / collection lifetime of the types in
// components.h (split over several files only to keep build times down).
#include "quantum_code/components.h"

#include "quantum_code/frame.h"

namespace Quantum {

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

std::int32_t DamageToImpulseConverter::GetHashCode() const {
    return HashBuilder(2591u)
        .Add(Spec)
        .Add(impulse)
        .Add(impulseFrame)
        .Add(priority)
        .Add(resetVelocity)
        .Value();
}

void DamageToImpulseConverter::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageToImpulseConverter*>(ptr);
    SerializeField(&p->impulseFrame, serializer);
    SerializeField(&p->priority, serializer);
    SerializeField(&p->resetVelocity, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->impulse, serializer);
}

std::int32_t HitHealthData::GetHashCode() const {
    return HashBuilder(1579u)
        .Add(damageMultiplier)
        .Value();
}

void HitHealthData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HitHealthData*>(ptr);
    SerializeField(&p->damageMultiplier, serializer);
}

std::int32_t HitShieldData::GetHashCode() const {
    return HashBuilder(1597u)
        .Add(applyMoveModifier)
        .Add(breakShield)
        .Add(damageMultiplier)
        .Add(ignoreShield)
        .Value();
}

void HitShieldData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HitShieldData*>(ptr);
    SerializeField(&p->applyMoveModifier, serializer);
    SerializeField(&p->breakShield, serializer);
    SerializeField(&p->ignoreShield, serializer);
    SerializeField(&p->damageMultiplier, serializer);
}

std::int32_t HitData::GetHashCode() const {
    return HashBuilder(1571u)
        .Add(baseDamage)
        .Add(damages)
        .Add(direction)
        .Add(frame)
        .Add(health)
        .Add(isContinuous)
        .Add(isCriticalHit)
        .Add(owner)
        .Add(position)
        .Add(shield)
        .Add(slowDownOnHit)
        .Add(source)
        .Add(type)
        .Add(useReticleFeedback)
        .Add(weapon)
        .Value();
}

void HitData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HitData*>(ptr);
    SerializeField(&p->type, serializer);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->isContinuous, serializer);
    SerializeField(&p->isCriticalHit, serializer);
    SerializeField(&p->slowDownOnHit, serializer);
    SerializeField(&p->useReticleFeedback, serializer);
    SerializeField(&p->owner, serializer);
    SerializeField(&p->source, serializer);
    SerializeField(&p->weapon, serializer);
    SerializeField(&p->baseDamage, serializer);
    SerializeField(&p->damages, serializer);
    SerializeField(&p->health, serializer);
    SerializeField(&p->direction, serializer);
    SerializeField(&p->position, serializer);
    SerializeField(&p->shield, serializer);
}

std::int32_t Damageable::GetHashCode() const {
    return HashBuilder(2593u)
        .Add(Spec)
        .Add(activeDamageEffectsPtr)
        .Add(electricalCurrentBuildUp)
        .Add(electricalDamageLastFrame)
        .Add(hitBy)
        .Add(hitsPtr)
        .Add(iceCurrentBuildUp)
        .Add(iceDamageLastFrame)
        .Add(invulnerabilityEndFrame)
        .Add(invulnerabilityFilter)
        .Value();
}

void Damageable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Damageable*>(ptr);
    SerializeField(&p->electricalDamageLastFrame, serializer);
    SerializeField(&p->iceDamageLastFrame, serializer);
    SerializeField(&p->invulnerabilityEndFrame, serializer);
    SerializeField(&p->invulnerabilityFilter, serializer);
    SerializeList<EntityRef>(&p->activeDamageEffectsPtr, serializer);
    SerializeList<HitData>(&p->hitsPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->electricalCurrentBuildUp, serializer);
    SerializeField(&p->iceCurrentBuildUp, serializer);
    SerializeField(&p->hitBy, serializer);
}

void Damageable::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (activeDamageEffectsPtr == Ptr::Null()) {
        activeDamageEffectsPtr = AllocateListPtr<EntityRef>(f, 8);
    }
    if (hitsPtr == Ptr::Null()) {
        hitsPtr = AllocateListPtr<HitData>(f, 8);
    }
}

void Damageable::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (activeDamageEffectsPtr != Ptr::Null()) {
        f->FreeList(get_activeDamageEffects());
    }
    activeDamageEffectsPtr = Ptr::Null();
    if (hitsPtr != Ptr::Null()) {
        f->FreeList(get_hits());
    }
    hitsPtr = Ptr::Null();
}

void Damageable::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Damageable*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Damageable::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Damageable*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t DashActor::GetHashCode() const {
    return HashBuilder(2609u)
        .Add(Spec)
        .Add(bonusCooldownReduction)
        .Add(bonusDashDamage)
        .Add(cooldownTimer)
        .Add(dashImpulse)
        .Add(dashTimer)
        .Add(disabledReasons)
        .Add(hitEntitiesPtr)
        .Add(ignoreMoveSpeedCounter)
        .Add(isDashing)
        .Add(lastHitFrame)
        .Add(lastValidMoveInput)
        .Add(noCooldownCheat)
        .Add(speedBonusDurationOnDashEnded)
        .Add(speedBonusOnDashEnded)
        .Value();
}

void DashActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DashActor*>(ptr);
    SerializeField(&p->disabledReasons, serializer);
    SerializeField(&p->lastHitFrame, serializer);
    SerializeField(&p->isDashing, serializer);
    SerializeField(&p->noCooldownCheat, serializer);
    SerializeList<EntityRef>(&p->hitEntitiesPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->bonusCooldownReduction, serializer);
    SerializeField(&p->bonusDashDamage, serializer);
    SerializeField(&p->cooldownTimer, serializer);
    SerializeField(&p->dashTimer, serializer);
    SerializeField(&p->ignoreMoveSpeedCounter, serializer);
    SerializeField(&p->speedBonusDurationOnDashEnded, serializer);
    SerializeField(&p->speedBonusOnDashEnded, serializer);
    SerializeField(&p->dashImpulse, serializer);
    SerializeField(&p->lastValidMoveInput, serializer);
}

void DashActor::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitEntitiesPtr == Ptr::Null()) {
        hitEntitiesPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void DashActor::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (hitEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_hitEntities());
    }
    hitEntitiesPtr = Ptr::Null();
}

void DashActor::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DashActor*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void DashActor::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DashActor*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t DrillDash::GetHashCode() const {
    return HashBuilder(2617u)
        .Add(AttackAoeEntity)
        .Add(CurrentState)
        .Add(DashDirection)
        .Add(StateStartFrame)
        .Value();
}

void DrillDash::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DrillDash*>(ptr);
    SerializeField(&p->CurrentState, serializer);
    SerializeField(&p->StateStartFrame, serializer);
    SerializeField(&p->AttackAoeEntity, serializer);
    SerializeField(&p->DashDirection, serializer);
}

std::int32_t DroppableCollector::GetHashCode() const {
    return HashBuilder(2621u)
        .Add(Spec)
        .Add(disabledDroppablesPtr)
        .Add(lastCollectDataByDropTypePtr)
        .Add(nbCollectedByTypePtr)
        .Value();
}

void DroppableCollector::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DroppableCollector*>(ptr);
    SerializeList<DroppableType>(&p->disabledDroppablesPtr, serializer);
    SerializeDictionary<DroppableType, CollectData>(&p->lastCollectDataByDropTypePtr, serializer);
    SerializeDictionary<DroppableType, std::uint16_t>(&p->nbCollectedByTypePtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void DroppableCollector::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (disabledDroppablesPtr == Ptr::Null()) {
        disabledDroppablesPtr = AllocateListPtr<DroppableType>(f, 8);
    }
    if (lastCollectDataByDropTypePtr == Ptr::Null()) {
        lastCollectDataByDropTypePtr = AllocateEnumDictionaryPtr<DroppableType, CollectData>(f, 8);
    }
    if (nbCollectedByTypePtr == Ptr::Null()) {
        nbCollectedByTypePtr = AllocateEnumDictionaryPtr<DroppableType, std::uint16_t>(f, 8);
    }
}

void DroppableCollector::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (disabledDroppablesPtr != Ptr::Null()) {
        f->FreeList(get_disabledDroppables());
    }
    disabledDroppablesPtr = Ptr::Null();
    if (lastCollectDataByDropTypePtr != Ptr::Null()) {
        f->FreeDictionary(get_lastCollectDataByDropType());
    }
    lastCollectDataByDropTypePtr = Ptr::Null();
    if (nbCollectedByTypePtr != Ptr::Null()) {
        f->FreeDictionary(get_nbCollectedByType());
    }
    nbCollectedByTypePtr = Ptr::Null();
}

void DroppableCollector::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DroppableCollector*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void DroppableCollector::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DroppableCollector*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t DroppableEntity::GetHashCode() const {
    return HashBuilder(2633u)
        .Add(destroyFrame)
        .Add(dropperSpec)
        .Add(removeOwnershipFrame)
        .Add(spawnFrame)
        .Add(spawnPos)
        .Add(targetPos)
        .Value();
}

void DroppableEntity::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DroppableEntity*>(ptr);
    SerializeField(&p->destroyFrame, serializer);
    SerializeField(&p->removeOwnershipFrame, serializer);
    SerializeField(&p->spawnFrame, serializer);
    SerializeField(&p->dropperSpec, serializer);
    SerializeField(&p->spawnPos, serializer);
    SerializeField(&p->targetPos, serializer);
}

std::int32_t DroppableManager::GetHashCode() const {
    return HashBuilder(2647u)
        .Add(Spec)
        .Add(dropAnimsPtr)
        .Add(droppablesHiPtr)
        .Add(droppablesLoPtr)
        .Add(nextDroppableId)
        .Value();
}

void DroppableManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DroppableManager*>(ptr);
    SerializeField(&p->nextDroppableId, serializer);
    SerializeDictionary<std::uint16_t, DropAnimData>(&p->dropAnimsPtr, serializer);
    SerializeList<Droppable>(&p->droppablesHiPtr, serializer);
    SerializeList<Droppable>(&p->droppablesLoPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void DroppableManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (dropAnimsPtr == Ptr::Null()) {
        dropAnimsPtr = AllocateDictionaryPtr<std::uint16_t, DropAnimData>(f, 64);
    }
    if (droppablesHiPtr == Ptr::Null()) {
        droppablesHiPtr = AllocateListPtr<Droppable>(f, 170);
    }
    if (droppablesLoPtr == Ptr::Null()) {
        droppablesLoPtr = AllocateListPtr<Droppable>(f, 170);
    }
}

void DroppableManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (dropAnimsPtr != Ptr::Null()) {
        f->FreeDictionary(get_dropAnims());
    }
    dropAnimsPtr = Ptr::Null();
    if (droppablesHiPtr != Ptr::Null()) {
        f->FreeList(get_droppablesHi());
    }
    droppablesHiPtr = Ptr::Null();
    if (droppablesLoPtr != Ptr::Null()) {
        f->FreeList(get_droppablesLo());
    }
    droppablesLoPtr = Ptr::Null();
}

void DroppableManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DroppableManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void DroppableManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<DroppableManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ElementalBuildUpMultiplier::GetHashCode() const {
    return HashBuilder(2657u)
        .Add(electricalBuildUpMultiplier)
        .Add(electricalBuildUpMultipliersPtr)
        .Add(iceBuildUpMultiplier)
        .Add(iceBuildUpMultipliersPtr)
        .Value();
}

void ElementalBuildUpMultiplier::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ElementalBuildUpMultiplier*>(ptr);
    SerializeList<FP>(&p->electricalBuildUpMultipliersPtr, serializer);
    SerializeList<FP>(&p->iceBuildUpMultipliersPtr, serializer);
    SerializeField(&p->electricalBuildUpMultiplier, serializer);
    SerializeField(&p->iceBuildUpMultiplier, serializer);
}

void ElementalBuildUpMultiplier::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (electricalBuildUpMultipliersPtr == Ptr::Null()) {
        electricalBuildUpMultipliersPtr = AllocateListPtr<FP>(f, 8);
    }
    if (iceBuildUpMultipliersPtr == Ptr::Null()) {
        iceBuildUpMultipliersPtr = AllocateListPtr<FP>(f, 8);
    }
}

void ElementalBuildUpMultiplier::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (electricalBuildUpMultipliersPtr != Ptr::Null()) {
        f->FreeList(get_electricalBuildUpMultipliers());
    }
    electricalBuildUpMultipliersPtr = Ptr::Null();
    if (iceBuildUpMultipliersPtr != Ptr::Null()) {
        f->FreeList(get_iceBuildUpMultipliers());
    }
    iceBuildUpMultipliersPtr = Ptr::Null();
}

void ElementalBuildUpMultiplier::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ElementalBuildUpMultiplier*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void ElementalBuildUpMultiplier::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ElementalBuildUpMultiplier*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t EliminationGameMode::GetHashCode() const {
    return HashBuilder(2659u)
        .Add(CloseToWinSlowMotionDone)
        .Add(DeadPlayersPtr)
        .Add(Spec)
        .Value();
}

void EliminationGameMode::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EliminationGameMode*>(ptr);
    SerializeField(&p->CloseToWinSlowMotionDone, serializer);
    SerializeList<EntityRef>(&p->DeadPlayersPtr, serializer);
    SerializeField(&p->Spec, serializer);
}

void EliminationGameMode::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (DeadPlayersPtr == Ptr::Null()) {
        DeadPlayersPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void EliminationGameMode::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (DeadPlayersPtr != Ptr::Null()) {
        f->FreeList(get_DeadPlayers());
    }
    DeadPlayersPtr = Ptr::Null();
}

void EliminationGameMode::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EliminationGameMode*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void EliminationGameMode::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EliminationGameMode*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t EnergyCellManager::GetHashCode() const {
    return HashBuilder(2663u)
        .Add(Spec)
        .Value();
}

void EnergyCellManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EnergyCellManager*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t EnforcerFirstShotBonusEnhancement::GetHashCode() const {
    return HashBuilder(2671u)
        .Add(Spec)
        .Value();
}

void EnforcerFirstShotBonusEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EnforcerFirstShotBonusEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t EntityDropper::GetHashCode() const {
    return HashBuilder(2677u)
        .Add(SpecsPtr)
        .Add(lastPushDropTime)
        .Value();
}

void EntityDropper::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EntityDropper*>(ptr);
    SerializeList<AssetRefEntityDropperSpec>(&p->SpecsPtr, serializer);
    SerializeField(&p->lastPushDropTime, serializer);
}

void EntityDropper::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpecsPtr == Ptr::Null()) {
        SpecsPtr = AllocateListPtr<AssetRefEntityDropperSpec>(f, 8);
    }
}

void EntityDropper::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpecsPtr != Ptr::Null()) {
        f->FreeList(get_Specs());
    }
    SpecsPtr = Ptr::Null();
}

void EntityDropper::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EntityDropper*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void EntityDropper::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EntityDropper*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t EntityGroupStats::GetHashCode() const {
    return HashBuilder(1483u)
        .Add(EntityGroupMaxMembers)
        .Add(IsInitialised)
        .Value();
}

void EntityGroupStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EntityGroupStats*>(ptr);
    SerializeField(&p->EntityGroupMaxMembers, serializer);
    SerializeField(&p->IsInitialised, serializer);
}

std::int32_t EntityGroup::GetHashCode() const {
    return HashBuilder(2683u)
        .Add(GroupMembersPtr)
        .Add(LastFrameGroupFull)
        .Add(Spec)
        .Add(Stats)
        .Value();
}

void EntityGroup::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EntityGroup*>(ptr);
    SerializeField(&p->LastFrameGroupFull, serializer);
    SerializeList<EntityGroupMember>(&p->GroupMembersPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Stats, serializer);
}

void EntityGroup::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (GroupMembersPtr == Ptr::Null()) {
        GroupMembersPtr = AllocateListPtr<EntityGroupMember>(f, 8);
    }
}

void EntityGroup::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (GroupMembersPtr != Ptr::Null()) {
        f->FreeList(get_GroupMembers());
    }
    GroupMembersPtr = Ptr::Null();
}

void EntityGroup::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EntityGroup*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void EntityGroup::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<EntityGroup*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t EntityGroupMemberGrouped::GetHashCode() const {
    return HashBuilder(2687u).Value();
}

void EntityGroupMemberGrouped::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t Code::EntityInfo::GetHashCode() const {
    return HashBuilder(2689u)
        .Add(id)
        .Add(isActiveObjective)
        .Add(type)
        .Value();
}

void Code::EntityInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Code::EntityInfo*>(ptr);
    SerializeField(&p->type, serializer);
    SerializeField(&p->id, serializer);
    SerializeField(&p->isActiveObjective, serializer);
}

std::int32_t FireReactor::GetHashCode() const {
    return HashBuilder(2693u)
        .Add(IsEnabled)
        .Add(Spec)
        .Add(TimeBeforeNextCycle)
        .Value();
}

void FireReactor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<FireReactor*>(ptr);
    SerializeField(&p->IsEnabled, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->TimeBeforeNextCycle, serializer);
}

std::int32_t GOAPState::GetHashCode() const {
    return HashBuilder(1493u)
        .Add(Negative)
        .Add(Positive)
        .Value();
}

void GOAPState::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GOAPState*>(ptr);
    SerializeField(&p->Negative, serializer);
    SerializeField(&p->Positive, serializer);
}

std::int32_t GOAPAgent::GetHashCode() const {
    return HashBuilder(2699u)
        .Add(Config)
        .Add(CurrentActionIndex)
        .Add(CurrentActionTime)
        .Add(CurrentGoal)
        .Add(CurrentGoalTime)
        .Add(CurrentPlanSize)
        .Add(CurrentState)
        .Add(GoalDisableTimesPtr)
        .Add(GoalState)
        .Add(InterruptionCheckCooldown)
        .Add(LastProcessedActionIndex)
        .Add(_Plan_)
        .Add(Root)
        .Value();
}

void GOAPAgent::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GOAPAgent*>(ptr);
    SerializeField(&p->CurrentPlanSize, serializer);
    SerializeField(&p->CurrentActionIndex, serializer);
    SerializeField(&p->LastProcessedActionIndex, serializer);
    SerializeList<FP>(&p->GoalDisableTimesPtr, serializer);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->_Plan_, serializer);
    SerializeField(&p->CurrentGoal, serializer);
    SerializeField(&p->Root, serializer);
    SerializeField(&p->CurrentActionTime, serializer);
    SerializeField(&p->CurrentGoalTime, serializer);
    SerializeField(&p->InterruptionCheckCooldown, serializer);
    SerializeField(&p->CurrentState, serializer);
    SerializeField(&p->GoalState, serializer);
}

void GOAPAgent::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    GoalDisableTimesPtr = Ptr::Null();
}

void GOAPAgent::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GOAPAgent*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t GOAPData::GetHashCode() const {
    return HashBuilder(2707u)
        .Add(HeuristicCost)
        .Value();
}

void GOAPData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GOAPData*>(ptr);
    SerializeField(&p->HeuristicCost, serializer);
}

std::int32_t GadgetStats::GetHashCode() const {
    return HashBuilder(1499u)
        .Add(Cooldown)
        .Value();
}

void GadgetStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GadgetStats*>(ptr);
    SerializeField(&p->Cooldown, serializer);
}

std::int32_t GadgetUser::GetHashCode() const {
    return HashBuilder(2711u)
        .Add(Stats)
        .Add(activationFrame)
        .Add(cooldownEndFrame)
        .Add(cooldownStartFrame)
        .Add(gadgetSpec)
        .Value();
}

void GadgetUser::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GadgetUser*>(ptr);
    SerializeField(&p->activationFrame, serializer);
    SerializeField(&p->cooldownEndFrame, serializer);
    SerializeField(&p->cooldownStartFrame, serializer);
    SerializeField(&p->gadgetSpec, serializer);
    SerializeField(&p->Stats, serializer);
}

std::int32_t GameManager::GetHashCode() const {
    return HashBuilder(2713u)
        .Add(config)
        .Add(nbPlayersSpawned)
        .Value();
}

void GameManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GameManager*>(ptr);
    SerializeField(&p->nbPlayersSpawned, serializer);
    SerializeField(&p->config, serializer);
}

std::int32_t GameModeActivator::GetHashCode() const {
    return HashBuilder(2719u)
        .Add(enabled)
        .Add(modificator)
        .Value();
}

void GameModeActivator::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GameModeActivator*>(ptr);
    SerializeField(&p->modificator, serializer);
    SerializeField(&p->enabled, serializer);
}

std::int32_t GameModeHandlerStats::GetHashCode() const {
    return HashBuilder(1511u)
        .Add(leadChangesForMinute)
        .Add(mainloopEndFrame)
        .Add(mainloopStartFrame)
        .Add(totalLeadChanges)
        .Value();
}

void GameModeHandlerStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GameModeHandlerStats*>(ptr);
    SerializeBuffer(p->leadChangesForMinute, serializer);
    SerializeField(&p->mainloopEndFrame, serializer);
    SerializeField(&p->mainloopStartFrame, serializer);
    SerializeField(&p->totalLeadChanges, serializer);
}

std::int32_t GameModeHandler::GetHashCode() const {
    return HashBuilder(2729u)
        .Add(averageHumanHeroLevel)
        .Add(averageHumanHeroLevelIsComputed)
        .Add(averageHumanWeaponLevel)
        .Add(boltMultiplierOn)
        .Add(config)
        .Add(debugSkipRequested)
        .Add(entity)
        .Add(finishFrame)
        .Add(gameModeEntity)
        .Add(gameModeSpecRef)
        .Add(gameModesTable)
        .Add(introFlags)
        .Add(isPaused)
        .Add(levelConfig)
        .Add(modeEndBotsTimeoutCounter)
        .Add(nPlayersReadyAfterLoadingAddressables)
        .Add(nPlayersReadyForCurrentGameMode)
        .Add(numBotsSpawned)
        .Add(numHumanPlayersSpawned)
        .Add(pausedTime)
        .Add(playersCharacterIDsPtr)
        .Add(playersSkinsIDsPtr)
        .Add(randomBotCharacterSelectionOffset)
        .Add(randomBotWeaponSelectionOffset)
        .Add(state)
        .Add(stateStartFrame)
        .Add(stats)
        .Add(unlimitedTimeCheatActive)
        .Add(waitExpectedHumanPlayersTimer)
        .Add(waitUnexpectedPlayersTimer)
        .Value();
}

void GameModeHandler::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GameModeHandler*>(ptr);
    SerializeField(&p->state, serializer);
    SerializeField(&p->averageHumanHeroLevel, serializer);
    SerializeField(&p->finishFrame, serializer);
    SerializeField(&p->introFlags, serializer);
    SerializeField(&p->randomBotCharacterSelectionOffset, serializer);
    SerializeField(&p->randomBotWeaponSelectionOffset, serializer);
    SerializeField(&p->stateStartFrame, serializer);
    SerializeField(&p->averageHumanHeroLevelIsComputed, serializer);
    SerializeField(&p->boltMultiplierOn, serializer);
    SerializeField(&p->debugSkipRequested, serializer);
    SerializeField(&p->isPaused, serializer);
    SerializeField(&p->unlimitedTimeCheatActive, serializer);
    SerializeList<std::int32_t>(&p->playersCharacterIDsPtr, serializer);
    SerializeList<std::int32_t>(&p->playersSkinsIDsPtr, serializer);
    SerializeField(&p->nPlayersReadyAfterLoadingAddressables, serializer);
    SerializeField(&p->nPlayersReadyForCurrentGameMode, serializer);
    SerializeField(&p->numBotsSpawned, serializer);
    SerializeField(&p->numHumanPlayersSpawned, serializer);
    SerializeField(&p->config, serializer);
    SerializeField(&p->levelConfig, serializer);
    SerializeField(&p->gameModeSpecRef, serializer);
    SerializeField(&p->gameModesTable, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->gameModeEntity, serializer);
    SerializeField(&p->averageHumanWeaponLevel, serializer);
    SerializeField(&p->modeEndBotsTimeoutCounter, serializer);
    SerializeField(&p->pausedTime, serializer);
    SerializeField(&p->waitExpectedHumanPlayersTimer, serializer);
    SerializeField(&p->waitUnexpectedPlayersTimer, serializer);
    SerializeField(&p->stats, serializer);
}

void GameModeHandler::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (playersCharacterIDsPtr == Ptr::Null()) {
        playersCharacterIDsPtr = AllocateListPtr<std::int32_t>(f, 8);
    }
    if (playersSkinsIDsPtr == Ptr::Null()) {
        playersSkinsIDsPtr = AllocateListPtr<std::int32_t>(f, 8);
    }
}

void GameModeHandler::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (playersCharacterIDsPtr != Ptr::Null()) {
        f->FreeList(get_playersCharacterIDs());
    }
    playersCharacterIDsPtr = Ptr::Null();
    if (playersSkinsIDsPtr != Ptr::Null()) {
        f->FreeList(get_playersSkinsIDs());
    }
    playersSkinsIDsPtr = Ptr::Null();
}

void GameModeHandler::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GameModeHandler*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void GameModeHandler::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GameModeHandler*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t GameModifiersManager::GetHashCode() const {
    return HashBuilder(2731u)
        .Add(Spec)
        .Value();
}

void GameModifiersManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GameModifiersManager*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t GatlingEnhancement::GetHashCode() const {
    return HashBuilder(2741u)
        .Add(ShouldResetAmmoOnGadgetUse)
        .Add(Spec)
        .Value();
}

void GatlingEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GatlingEnhancement*>(ptr);
    SerializeField(&p->ShouldResetAmmoOnGadgetUse, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t GatlingWeapon::GetHashCode() const {
    return HashBuilder(2749u)
        .Add(fireRateLevelsPtr)
        .Add(spinUpValue)
        .Value();
}

void GatlingWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GatlingWeapon*>(ptr);
    SerializeList<GatlingFireRateLevel>(&p->fireRateLevelsPtr, serializer);
    SerializeField(&p->spinUpValue, serializer);
}

void GatlingWeapon::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    fireRateLevelsPtr = Ptr::Null();
}

void GatlingWeapon::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GatlingWeapon*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t GloveOfDoomWeapon::GetHashCode() const {
    return HashBuilder(2753u).Value();
}

void GloveOfDoomWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t GoldenBolt::GetHashCode() const {
    return HashBuilder(2767u)
        .Add(isCollected)
        .Value();
}

void GoldenBolt::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GoldenBolt*>(ptr);
    SerializeField(&p->isCollected, serializer);
}

std::int32_t GoldenBoltManager::GetHashCode() const {
    return HashBuilder(2777u)
        .Add(PendingGoldBoltSpawnsPtr)
        .Add(SpawnedGoldenBoltsPtr)
        .Value();
}

void GoldenBoltManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GoldenBoltManager*>(ptr);
    SerializeList<PendingGoldBoltSpawn>(&p->PendingGoldBoltSpawnsPtr, serializer);
    SerializeList<SpawnedGoldenBoltData>(&p->SpawnedGoldenBoltsPtr, serializer);
}

void GoldenBoltManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (PendingGoldBoltSpawnsPtr == Ptr::Null()) {
        PendingGoldBoltSpawnsPtr = AllocateListPtr<PendingGoldBoltSpawn>(f, 8);
    }
    if (SpawnedGoldenBoltsPtr == Ptr::Null()) {
        SpawnedGoldenBoltsPtr = AllocateListPtr<SpawnedGoldenBoltData>(f, 8);
    }
}

void GoldenBoltManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (PendingGoldBoltSpawnsPtr != Ptr::Null()) {
        f->FreeList(get_PendingGoldBoltSpawns());
    }
    PendingGoldBoltSpawnsPtr = Ptr::Null();
    if (SpawnedGoldenBoltsPtr != Ptr::Null()) {
        f->FreeList(get_SpawnedGoldenBolts());
    }
    SpawnedGoldenBoltsPtr = Ptr::Null();
}

void GoldenBoltManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GoldenBoltManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void GoldenBoltManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GoldenBoltManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t GrenadeLauncherAftershockEnhancement::GetHashCode() const {
    return HashBuilder(2789u)
        .Add(SpecsPtr)
        .Value();
}

void GrenadeLauncherAftershockEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GrenadeLauncherAftershockEnhancement*>(ptr);
    SerializeList<AssetRefGrenadeLauncherAftershockEnhancementSpec>(&p->SpecsPtr, serializer);
}

void GrenadeLauncherAftershockEnhancement::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    SpecsPtr = Ptr::Null();
}

void GrenadeLauncherAftershockEnhancement::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<GrenadeLauncherAftershockEnhancement*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t GrenadeLauncherStunGrenadeEnhancement::GetHashCode() const {
    return HashBuilder(2791u)
        .Add(Spec)
        .Value();
}

void GrenadeLauncherStunGrenadeEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GrenadeLauncherStunGrenadeEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t GrinderActor::GetHashCode() const {
    return HashBuilder(2797u)
        .Add(accelCounter)
        .Add(config)
        .Add(currentDistanceOnRail)
        .Add(currentRailEntity)
        .Add(currentRailStaticColliderIndex)
        .Add(disableTimer)
        .Add(disabled)
        .Add(entity)
        .Add(grindSign)
        .Add(grindVelocity)
        .Add(isGrinding)
        .Add(isOnRail)
        .Add(isSnapping)
        .Add(snapCounter)
        .Add(snapEndPos)
        .Add(snapStartPos)
        .Add(startGrindingFrame)
        .Value();
}

void GrinderActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GrinderActor*>(ptr);
    SerializeField(&p->currentRailStaticColliderIndex, serializer);
    SerializeField(&p->startGrindingFrame, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->isGrinding, serializer);
    SerializeField(&p->isOnRail, serializer);
    SerializeField(&p->isSnapping, serializer);
    SerializeField(&p->config, serializer);
    SerializeField(&p->currentRailEntity, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->accelCounter, serializer);
    SerializeField(&p->currentDistanceOnRail, serializer);
    SerializeField(&p->disableTimer, serializer);
    SerializeField(&p->grindSign, serializer);
    SerializeField(&p->snapCounter, serializer);
    SerializeField(&p->grindVelocity, serializer);
    SerializeField(&p->snapEndPos, serializer);
    SerializeField(&p->snapStartPos, serializer);
}

std::int32_t GroundDashActor::GetHashCode() const {
    return HashBuilder(2801u)
        .Add(Spec)
        .Add(disabled)
        .Add(isDashing)
        .Add(isStarting)
        .Add(shouldDash)
        .Add(startingLastFrame)
        .Value();
}

void GroundDashActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GroundDashActor*>(ptr);
    SerializeField(&p->startingLastFrame, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->isDashing, serializer);
    SerializeField(&p->isStarting, serializer);
    SerializeField(&p->shouldDash, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t HeadHunterWallHackEnhancement::GetHashCode() const {
    return HashBuilder(2819u)
        .Add(Spec)
        .Value();
}

void HeadHunterWallHackEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HeadHunterWallHackEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t HealSource::GetHashCode() const {
    return HashBuilder(2833u)
        .Add(Spec)
        .Value();
}

void HealSource::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HealSource*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t HealthStats::GetHashCode() const {
    return HashBuilder(1567u)
        .Add(InitialMaxHP)
        .Add(Initialised)
        .Add(MaxHP)
        .Value();
}

void HealthStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HealthStats*>(ptr);
    SerializeField(&p->Initialised, serializer);
    SerializeField(&p->InitialMaxHP, serializer);
    SerializeField(&p->MaxHP, serializer);
}

std::int32_t Health::GetHashCode() const {
    return HashBuilder(2837u)
        .Add(ActiveOverhealthsPtr)
        .Add(CurrentHP)
        .Add(CurrentOverHP)
        .Add(_MaxHpMultArray_)
        .Add(MaxOverHP)
        .Add(Spec)
        .Add(StartRegenHP)
        .Add(Stats)
        .Add(regenFrameReference)
        .Value();
}

void Health::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Health*>(ptr);
    SerializeField(&p->regenFrameReference, serializer);
    SerializeList<Overhealth>(&p->ActiveOverhealthsPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->CurrentHP, serializer);
    SerializeField(&p->CurrentOverHP, serializer);
    SerializeField(&p->_MaxHpMultArray_, serializer);
    SerializeField(&p->MaxOverHP, serializer);
    SerializeField(&p->StartRegenHP, serializer);
    SerializeField(&p->Stats, serializer);
}

void Health::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (ActiveOverhealthsPtr == Ptr::Null()) {
        ActiveOverhealthsPtr = AllocateListPtr<Overhealth>(f, 8);
    }
}

void Health::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (ActiveOverhealthsPtr != Ptr::Null()) {
        f->FreeList(get_ActiveOverhealths());
    }
    ActiveOverhealthsPtr = Ptr::Null();
}

void Health::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Health*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Health::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Health*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t HelipackActor::GetHashCode() const {
    return HashBuilder(2843u)
        .Add(Spec)
        .Add(bonusDuration)
        .Add(disabled)
        .Add(impulseIsDone)
        .Add(isActive)
        .Add(isExpired)
        .Add(startFrame)
        .Add(usageTime)
        .Value();
}

void HelipackActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HelipackActor*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->impulseIsDone, serializer);
    SerializeField(&p->isActive, serializer);
    SerializeField(&p->isExpired, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->bonusDuration, serializer);
    SerializeField(&p->usageTime, serializer);
}

std::int32_t Hierarchy::GetHashCode() const {
    return HashBuilder(2851u)
        .Add(LocalRotation)
        .Add(Offset)
        .Add(Parent)
        .Value();
}

void Hierarchy::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Hierarchy*>(ptr);
    SerializeField(&p->Parent, serializer);
    SerializeField(&p->Offset, serializer);
    SerializeField(&p->LocalRotation, serializer);
}

std::int32_t HoloshieldGloveHealZoneEnhancement::GetHashCode() const {
    return HashBuilder(2857u)
        .Add(Spec)
        .Value();
}

void HoloshieldGloveHealZoneEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HoloshieldGloveHealZoneEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t HoloshieldGloveShield::GetHashCode() const {
    return HashBuilder(2861u)
        .Add(addedToGroup)
        .Add(startFrame)
        .Add(state)
        .Value();
}

void HoloshieldGloveShield::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HoloshieldGloveShield*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->addedToGroup, serializer);
    SerializeField(&p->state, serializer);
}

std::int32_t HoloshieldGloveShields::GetHashCode() const {
    return HashBuilder(2879u).Value();
}

void HoloshieldGloveShields::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t HoloshieldGloveShieldsGrouped::GetHashCode() const {
    return HashBuilder(2887u).Value();
}

void HoloshieldGloveShieldsGrouped::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t HoloshieldGloveWeapon::GetHashCode() const {
    return HashBuilder(2897u).Value();
}

void HoloshieldGloveWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t HoverBootsWeapon::GetHashCode() const {
    return HashBuilder(2903u)
        .Add(StartUseFrame)
        .Value();
}

void HoverBootsWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<HoverBootsWeapon*>(ptr);
    SerializeField(&p->StartUseFrame, serializer);
}

std::int32_t ItemSpawner::GetHashCode() const {
    return HashBuilder(2909u)
        .Add(Config)
        .Add(SpawnedEntitiesPtr)
        .Add(enabled)
        .Add(frameAllDestroyed)
        .Add(spawnedItemCounter)
        .Add(spawnedItemId)
        .Add(spawning)
        .Value();
}

void ItemSpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ItemSpawner*>(ptr);
    SerializeField(&p->frameAllDestroyed, serializer);
    SerializeField(&p->spawnedItemCounter, serializer);
    SerializeField(&p->spawnedItemId, serializer);
    SerializeField(&p->enabled, serializer);
    SerializeField(&p->spawning, serializer);
    SerializeList<EntityRef>(&p->SpawnedEntitiesPtr, serializer);
    SerializeField(&p->Config, serializer);
}

void ItemSpawner::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpawnedEntitiesPtr == Ptr::Null()) {
        SpawnedEntitiesPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void ItemSpawner::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (SpawnedEntitiesPtr != Ptr::Null()) {
        f->FreeList(get_SpawnedEntities());
    }
    SpawnedEntitiesPtr = Ptr::Null();
}

void ItemSpawner::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ItemSpawner*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void ItemSpawner::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ItemSpawner*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t JumpAttack::GetHashCode() const {
    return HashBuilder(2917u)
        .Add(Spec)
        .Add(attackIsActive)
        .Add(endPosition)
        .Add(performedSlam)
        .Add(startFrame)
        .Add(startPosition)
        .Value();
}

void JumpAttack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<JumpAttack*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->attackIsActive, serializer);
    SerializeField(&p->performedSlam, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->endPosition, serializer);
    SerializeField(&p->startPosition, serializer);
}

std::int32_t KccExtension::GetHashCode() const {
    return HashBuilder(2927u)
        .Add(Config)
        .Add(Impulsed)
        .Add(accelerationModifiersPtr)
        .Add(airTime)
        .Add(autoJumpDownTimeAfterManualJumpCounter)
        .Add(autoJumpDownTimeCounter)
        .Add(baseJumpMultiplier)
        .Add(brakingModifiersPtr)
        .Add(cancelUpwardPushCorrection)
        .Add(currentSurfaceType)
        .Add(disableFrame)
        .Add(enabled)
        .Add(forceInput)
        .Add(forceInputBlend)
        .Add(forceInputCounter)
        .Add(forceInputFallOff)
        .Add(forceInputMaxBlend)
        .Add(forceInputSnapOnGround)
        .Add(forceLookInput)
        .Add(forceOrientation)
        .Add(gravityModifiersPtr)
        .Add(groundNormal)
        .Add(grounded)
        .Add(groundedTime)
        .Add(groundedTimeSinceJump)
        .Add(hasImpulseToConsume)
        .Add(hasPushToConsume)
        .Add(ignoreMoveSpeedModifier)
        .Add(impulse)
        .Add(impulseMaxSpeed)
        .Add(impulseMaxVerticalSpeed)
        .Add(impulseTarget)
        .Add(impulseTargetSnapLerpStrength)
        .Add(impulseTimer)
        .Add(jumpRequested)
        .Add(jumpState)
        .Add(lastGroundedPos)
        .Add(lockJump)
        .Add(lockJumpTimer)
        .Add(lockRotation)
        .Add(lockRotationTimer)
        .Add(lookTowardsCamera)
        .Add(lookTowardsCameraSight)
        .Add(manualJumped)
        .Add(maxFallSpeedModifiersPtr)
        .Add(maxSpeedModifier)
        .Add(maxSpeedModifiersPtr)
        .Add(moveSpeedModifier)
        .Add(orientationSharpness)
        .Add(preserveForceLookInput)
        .Add(push)
        .Add(pushVelocity)
        .Add(resetPush)
        .Add(speedModifiersPtr)
        .Add(surfaceSetPriority)
        .Add(velocity)
        .Add(velocityBlend)
        .Add(wallCheckExclusionsPtr)
        .Value();
}

void KccExtension::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KccExtension*>(ptr);
    SerializeField(&p->disableFrame, serializer);
    SerializeField(&p->jumpState, serializer);
    SerializeField(&p->surfaceSetPriority, serializer);
    SerializeField(&p->currentSurfaceType, serializer);
    SerializeField(&p->Impulsed, serializer);
    SerializeField(&p->cancelUpwardPushCorrection, serializer);
    SerializeField(&p->enabled, serializer);
    SerializeField(&p->forceInputSnapOnGround, serializer);
    SerializeField(&p->forceOrientation, serializer);
    SerializeField(&p->grounded, serializer);
    SerializeField(&p->hasImpulseToConsume, serializer);
    SerializeField(&p->hasPushToConsume, serializer);
    SerializeField(&p->ignoreMoveSpeedModifier, serializer);
    SerializeField(&p->jumpRequested, serializer);
    SerializeField(&p->lockJump, serializer);
    SerializeField(&p->lockRotation, serializer);
    SerializeField(&p->lookTowardsCamera, serializer);
    SerializeField(&p->lookTowardsCameraSight, serializer);
    SerializeField(&p->manualJumped, serializer);
    SerializeField(&p->preserveForceLookInput, serializer);
    SerializeField(&p->resetPush, serializer);
    SerializeList<ModInfo>(&p->accelerationModifiersPtr, serializer);
    SerializeList<ModInfo>(&p->brakingModifiersPtr, serializer);
    SerializeList<ModInfo>(&p->gravityModifiersPtr, serializer);
    SerializeList<ModInfo>(&p->maxFallSpeedModifiersPtr, serializer);
    SerializeList<ModInfo>(&p->maxSpeedModifiersPtr, serializer);
    SerializeList<ModInfo>(&p->speedModifiersPtr, serializer);
    SerializeList<EntityRef>(&p->wallCheckExclusionsPtr, serializer);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->impulseTarget, serializer);
    SerializeField(&p->airTime, serializer);
    SerializeField(&p->autoJumpDownTimeAfterManualJumpCounter, serializer);
    SerializeField(&p->autoJumpDownTimeCounter, serializer);
    SerializeField(&p->baseJumpMultiplier, serializer);
    SerializeField(&p->forceInputBlend, serializer);
    SerializeField(&p->forceInputCounter, serializer);
    SerializeField(&p->forceInputFallOff, serializer);
    SerializeField(&p->forceInputMaxBlend, serializer);
    SerializeField(&p->groundedTime, serializer);
    SerializeField(&p->groundedTimeSinceJump, serializer);
    SerializeField(&p->impulseMaxSpeed, serializer);
    SerializeField(&p->impulseMaxVerticalSpeed, serializer);
    SerializeField(&p->impulseTargetSnapLerpStrength, serializer);
    SerializeField(&p->impulseTimer, serializer);
    SerializeField(&p->lockJumpTimer, serializer);
    SerializeField(&p->lockRotationTimer, serializer);
    SerializeField(&p->maxSpeedModifier, serializer);
    SerializeField(&p->moveSpeedModifier, serializer);
    SerializeField(&p->orientationSharpness, serializer);
    SerializeField(&p->velocityBlend, serializer);
    SerializeField(&p->forceInput, serializer);
    SerializeField(&p->forceLookInput, serializer);
    SerializeField(&p->groundNormal, serializer);
    SerializeField(&p->impulse, serializer);
    SerializeField(&p->lastGroundedPos, serializer);
    SerializeField(&p->push, serializer);
    SerializeField(&p->pushVelocity, serializer);
    SerializeField(&p->velocity, serializer);
}

void KccExtension::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (accelerationModifiersPtr == Ptr::Null()) {
        accelerationModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (brakingModifiersPtr == Ptr::Null()) {
        brakingModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (gravityModifiersPtr == Ptr::Null()) {
        gravityModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (maxFallSpeedModifiersPtr == Ptr::Null()) {
        maxFallSpeedModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (maxSpeedModifiersPtr == Ptr::Null()) {
        maxSpeedModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (speedModifiersPtr == Ptr::Null()) {
        speedModifiersPtr = AllocateListPtr<ModInfo>(f, 8);
    }
    if (wallCheckExclusionsPtr == Ptr::Null()) {
        wallCheckExclusionsPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void KccExtension::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (accelerationModifiersPtr != Ptr::Null()) {
        f->FreeList(get_accelerationModifiers());
    }
    accelerationModifiersPtr = Ptr::Null();
    if (brakingModifiersPtr != Ptr::Null()) {
        f->FreeList(get_brakingModifiers());
    }
    brakingModifiersPtr = Ptr::Null();
    if (gravityModifiersPtr != Ptr::Null()) {
        f->FreeList(get_gravityModifiers());
    }
    gravityModifiersPtr = Ptr::Null();
    if (maxFallSpeedModifiersPtr != Ptr::Null()) {
        f->FreeList(get_maxFallSpeedModifiers());
    }
    maxFallSpeedModifiersPtr = Ptr::Null();
    if (maxSpeedModifiersPtr != Ptr::Null()) {
        f->FreeList(get_maxSpeedModifiers());
    }
    maxSpeedModifiersPtr = Ptr::Null();
    if (speedModifiersPtr != Ptr::Null()) {
        f->FreeList(get_speedModifiers());
    }
    speedModifiersPtr = Ptr::Null();
    if (wallCheckExclusionsPtr != Ptr::Null()) {
        f->FreeList(get_wallCheckExclusions());
    }
    wallCheckExclusionsPtr = Ptr::Null();
}

void KccExtension::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<KccExtension*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void KccExtension::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<KccExtension*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t KccModifier::GetHashCode() const {
    return HashBuilder(2939u)
        .Add(Spec)
        .Value();
}

void KccModifier::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KccModifier*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t KillNextFrame::GetHashCode() const {
    return HashBuilder(2953u)
        .Add(KillDelay)
        .Add(MarkedForKillFrame)
        .Add(killType)
        .Add(killer)
        .Value();
}

void KillNextFrame::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KillNextFrame*>(ptr);
    SerializeField(&p->MarkedForKillFrame, serializer);
    SerializeField(&p->killType, serializer);
    SerializeField(&p->killer, serializer);
    SerializeField(&p->KillDelay, serializer);
}

std::int32_t KillZone::GetHashCode() const {
    return HashBuilder(2957u)
        .Add(areasPtr)
        .Add(verticalThreshold)
        .Value();
}

void KillZone::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KillZone*>(ptr);
    SerializeList<KillZoneArea>(&p->areasPtr, serializer);
    SerializeField(&p->verticalThreshold, serializer);
}

void KillZone::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    areasPtr = Ptr::Null();
}

void KillZone::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<KillZone*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t KilledByData::GetHashCode() const {
    return HashBuilder(1609u)
        .Add(frame)
        .Add(isOwnerUltimate)
        .Add(killType)
        .Add(owner)
        .Add(source)
        .Add(weapon)
        .Value();
}

void KilledByData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KilledByData*>(ptr);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->killType, serializer);
    SerializeField(&p->isOwnerUltimate, serializer);
    SerializeField(&p->owner, serializer);
    SerializeField(&p->source, serializer);
    SerializeField(&p->weapon, serializer);
}

std::int32_t Killable::GetHashCode() const {
    return HashBuilder(2963u)
        .Add(IsAlive)
        .Add(Prototype)
        .Add(deathAnimationSpec)
        .Add(deathAnimationState)
        .Add(deathCount)
        .Add(killedBy)
        .Add(lastKillzoneSurfaceType)
        .Add(maxDeath)
        .Add(reviveTotemId)
        .Value();
}

void Killable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Killable*>(ptr);
    SerializeField(&p->reviveTotemId, serializer);
    SerializeField(&p->deathAnimationState, serializer);
    SerializeField(&p->deathCount, serializer);
    SerializeField(&p->maxDeath, serializer);
    SerializeField(&p->lastKillzoneSurfaceType, serializer);
    SerializeField(&p->IsAlive, serializer);
    SerializeField(&p->deathAnimationSpec, serializer);
    SerializeField(&p->Prototype, serializer);
    SerializeField(&p->killedBy, serializer);
}

std::int32_t LedgeGrabbingActor::GetHashCode() const {
    return HashBuilder(2969u)
        .Add(Destination)
        .Add(GrabStartPosition)
        .Add(GrabStartRotation)
        .Add(IsLedgeGrabbing)
        .Add(LedgeGrabbingStartFrame)
        .Add(SnapPosition)
        .Add(SnapRotation)
        .Add(Spec)
        .Value();
}

void LedgeGrabbingActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LedgeGrabbingActor*>(ptr);
    SerializeField(&p->LedgeGrabbingStartFrame, serializer);
    SerializeField(&p->IsLedgeGrabbing, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Destination, serializer);
    SerializeField(&p->GrabStartPosition, serializer);
    SerializeField(&p->SnapPosition, serializer);
    SerializeField(&p->GrabStartRotation, serializer);
    SerializeField(&p->SnapRotation, serializer);
}

std::int32_t LevelButton::GetHashCode() const {
    return HashBuilder(2971u)
        .Add(Config)
        .Add(entity)
        .Add(gameplayElementEntity)
        .Add(hasBeenHit)
        .Add(isOnCooldown)
        .Add(lastActivationFrame)
        .Value();
}

void LevelButton::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LevelButton*>(ptr);
    SerializeField(&p->lastActivationFrame, serializer);
    SerializeField(&p->hasBeenHit, serializer);
    SerializeField(&p->isOnCooldown, serializer);
    SerializeField(&p->Config, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->gameplayElementEntity, serializer);
}

std::int32_t LevelButtonGameplayElement::GetHashCode() const {
    return HashBuilder(2999u)
        .Add(Config)
        .Value();
}

void LevelButtonGameplayElement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LevelButtonGameplayElement*>(ptr);
    SerializeField(&p->Config, serializer);
}

std::int32_t LevelHolder::GetHashCode() const {
    return HashBuilder(3001u)
        .Add(Level)
        .Value();
}

void LevelHolder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LevelHolder*>(ptr);
    SerializeField(&p->Level, serializer);
}

std::int32_t LevelUpActor::GetHashCode() const {
    return HashBuilder(3011u)
        .Add(Spec)
        .Value();
}

void LevelUpActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LevelUpActor*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t LifetimeStats::GetHashCode() const {
    return HashBuilder(1619u)
        .Add(initialised)
        .Add(lifetimeDuration)
        .Add(spawnEntitiesOnDeath)
        .Value();
}

void LifetimeStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LifetimeStats*>(ptr);
    SerializeField(&p->initialised, serializer);
    SerializeField(&p->spawnEntitiesOnDeath, serializer);
    SerializeField(&p->lifetimeDuration, serializer);
}

std::int32_t Lifetime::GetHashCode() const {
    return HashBuilder(3019u)
        .Add(EntitiesToSpawnOnDeathPtr)
        .Add(Spec)
        .Add(StartFrame)
        .Add(Stats)
        .Value();
}

void Lifetime::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Lifetime*>(ptr);
    SerializeField(&p->StartFrame, serializer);
    SerializeList<AssetRefEntityPrototype>(&p->EntitiesToSpawnOnDeathPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Stats, serializer);
}

void Lifetime::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    EntitiesToSpawnOnDeathPtr = Ptr::Null();
}

void Lifetime::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Lifetime*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t MeleeAttack::GetHashCode() const {
    return HashBuilder(3023u)
        .Add(Spec)
        .Add(attackHit)
        .Add(attackIsActive)
        .Add(attackMissed)
        .Add(meleeDamageMultiplier)
        .Add(spawnedAreaOfEffect)
        .Add(startFrame)
        .Value();
}

void MeleeAttack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MeleeAttack*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->attackHit, serializer);
    SerializeField(&p->attackIsActive, serializer);
    SerializeField(&p->attackMissed, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->spawnedAreaOfEffect, serializer);
    SerializeField(&p->meleeDamageMultiplier, serializer);
}

std::int32_t MeleeJumpAttack::GetHashCode() const {
    return HashBuilder(3037u)
        .Add(spec)
        .Add(state)
        .Add(stateStartFrame)
        .Value();
}

void MeleeJumpAttack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MeleeJumpAttack*>(ptr);
    SerializeField(&p->stateStartFrame, serializer);
    SerializeField(&p->state, serializer);
    SerializeField(&p->spec, serializer);
}

std::int32_t MineLauncherEnhancement::GetHashCode() const {
    return HashBuilder(3041u)
        .Add(Spec)
        .Value();
}

void MineLauncherEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MineLauncherEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t MineMerger::GetHashCode() const {
    return HashBuilder(3049u)
        .Add(mergeCount)
        .Add(wasMergeChecked)
        .Value();
}

void MineMerger::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MineMerger*>(ptr);
    SerializeField(&p->mergeCount, serializer);
    SerializeField(&p->wasMergeChecked, serializer);
}

std::int32_t MobGroup::GetHashCode() const {
    return HashBuilder(3061u)
        .Add(ClosestMemberToTarget)
        .Add(MembersPtr)
        .Add(NumToSpawn)
        .Add(SwarmerConfig)
        .Add(TargetEntity)
        .Value();
}

void MobGroup::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MobGroup*>(ptr);
    SerializeField(&p->NumToSpawn, serializer);
    SerializeList<EntityRef>(&p->MembersPtr, serializer);
    SerializeField(&p->SwarmerConfig, serializer);
    SerializeField(&p->ClosestMemberToTarget, serializer);
    SerializeField(&p->TargetEntity, serializer);
}

void MobGroup::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (MembersPtr == Ptr::Null()) {
        MembersPtr = AllocateListPtr<EntityRef>(f, 8);
    }
}

void MobGroup::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (MembersPtr != Ptr::Null()) {
        f->FreeList(get_Members());
    }
    MembersPtr = Ptr::Null();
}

void MobGroup::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<MobGroup*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void MobGroup::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<MobGroup*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t MobGroupMember::GetHashCode() const {
    return HashBuilder(3067u)
        .Add(groupEntity)
        .Value();
}

void MobGroupMember::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MobGroupMember*>(ptr);
    SerializeField(&p->groupEntity, serializer);
}

std::int32_t MobInfo::GetHashCode() const {
    return HashBuilder(3079u)
        .Add(Spec)
        .Add(Type)
        .Add(UseOwnerLevel)
        .Value();
}

void MobInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MobInfo*>(ptr);
    SerializeField(&p->Type, serializer);
    SerializeField(&p->UseOwnerLevel, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t MoveTo::GetHashCode() const {
    return HashBuilder(3083u)
        .Add(delay)
        .Add(duration)
        .Add(easing)
        .Add(followTarget)
        .Add(isInitialized)
        .Add(moveMode)
        .Add(moveStartFrame)
        .Add(moveStartPosition)
        .Add(moveState)
        .Add(moveToSpec)
        .Add(offset)
        .Add(offsetOrientation)
        .Add(onMoveFinishedPtr)
        .Add(targetEntity)
        .Add(targetPosition)
        .Add(waitStartFrame)
        .Value();
}

void MoveTo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<MoveTo*>(ptr);
    SerializeField(&p->easing, serializer);
    SerializeField(&p->moveStartFrame, serializer);
    SerializeField(&p->waitStartFrame, serializer);
    SerializeField(&p->moveMode, serializer);
    SerializeField(&p->offsetOrientation, serializer);
    SerializeField(&p->moveState, serializer);
    SerializeField(&p->followTarget, serializer);
    SerializeField(&p->isInitialized, serializer);
    SerializeList<AssetRefCallbackAction>(&p->onMoveFinishedPtr, serializer);
    SerializeField(&p->moveToSpec, serializer);
    SerializeField(&p->targetEntity, serializer);
    SerializeField(&p->delay, serializer);
    SerializeField(&p->duration, serializer);
    SerializeField(&p->moveStartPosition, serializer);
    SerializeField(&p->offset, serializer);
    SerializeField(&p->targetPosition, serializer);
}

void MoveTo::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onMoveFinishedPtr == Ptr::Null()) {
        onMoveFinishedPtr = AllocateListPtr<AssetRefCallbackAction>(f, 8);
    }
}

void MoveTo::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (onMoveFinishedPtr != Ptr::Null()) {
        f->FreeList(get_onMoveFinished());
    }
    onMoveFinishedPtr = Ptr::Null();
}

void MoveTo::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<MoveTo*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void MoveTo::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<MoveTo*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

} // namespace Quantum
