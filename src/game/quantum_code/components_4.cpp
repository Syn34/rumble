// components_4.cpp - GetHashCode / Serialize / collection lifetime of the types in
// components.h (split over several files only to keep build times down).
#include "quantum_code/components.h"

#include "quantum_code/frame.h"

namespace Quantum {

std::int32_t WaveManager::GetHashCode() const {
    return HashBuilder(3701u)
        .Add(Spec)
        .Add(SquadSiegeConfig)
        .Add(allWavesCompleted)
        .Add(currentWaveIndex)
        .Add(currentWaveStartFrame)
        .Add(nextWaveStartFrame)
        .Add(referencesAsset)
        .Add(waveSpawnRequestIndex)
        .Add(waveSpawnersPtr)
        .Value();
}

void WaveManager::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WaveManager*>(ptr);
    SerializeField(&p->waveSpawnRequestIndex, serializer);
    SerializeField(&p->currentWaveIndex, serializer);
    SerializeField(&p->currentWaveStartFrame, serializer);
    SerializeField(&p->nextWaveStartFrame, serializer);
    SerializeField(&p->allWavesCompleted, serializer);
    SerializeDictionary<std::uint16_t, EntityRef>(&p->waveSpawnersPtr, serializer);
    SerializeField(&p->SquadSiegeConfig, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->referencesAsset, serializer);
}

void WaveManager::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (waveSpawnersPtr == Ptr::Null()) {
        waveSpawnersPtr = AllocateDictionaryPtr<std::uint16_t, EntityRef>(f, 8);
    }
}

void WaveManager::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (waveSpawnersPtr != Ptr::Null()) {
        f->FreeDictionary(get_waveSpawners());
    }
    waveSpawnersPtr = Ptr::Null();
}

void WaveManager::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<WaveManager*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void WaveManager::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<WaveManager*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t SpawnConfig::GetHashCode() const {
    return HashBuilder(1861u)
        .Add(fallbackYOffset)
        .Add(findPositionAttempts)
        .Add(freePositionRadius)
        .Add(spawnPositionOffsetDistance)
        .Add(useRandomSpawnTransform)
        .Value();
}

void SpawnConfig::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<SpawnConfig*>(ptr);
    SerializeField(&p->findPositionAttempts, serializer);
    SerializeField(&p->useRandomSpawnTransform, serializer);
    SerializeField(&p->fallbackYOffset, serializer);
    SerializeField(&p->freePositionRadius, serializer);
    SerializeField(&p->spawnPositionOffsetDistance, serializer);
}

std::int32_t WaveSpawner::GetHashCode() const {
    return HashBuilder(3709u)
        .Add(currentEntityIndex)
        .Add(currentGroup)
        .Add(droppers)
        .Add(index)
        .Add(lastSpawnFrame)
        .Add(spawnConfig)
        .Add(spawnInterval)
        .Add(wanderRadius)
        .Add(waypointRadius)
        .Add(waypointsPtr)
        .Value();
}

void WaveSpawner::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WaveSpawner*>(ptr);
    SerializeField(&p->currentEntityIndex, serializer);
    SerializeField(&p->lastSpawnFrame, serializer);
    SerializeList<FPVector3>(&p->waypointsPtr, serializer);
    SerializeField(&p->index, serializer);
    SerializeField(&p->droppers, serializer);
    SerializeField(&p->currentGroup, serializer);
    SerializeField(&p->spawnInterval, serializer);
    SerializeField(&p->wanderRadius, serializer);
    SerializeField(&p->waypointRadius, serializer);
    SerializeField(&p->spawnConfig, serializer);
}

void WaveSpawner::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    waypointsPtr = Ptr::Null();
}

void WaveSpawner::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<WaveSpawner*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t WeaponStats::GetHashCode() const {
    return HashBuilder(1993u)
        .Add(AmmoPerBurst)
        .Add(ConsumeOneAmmoPerMultiShot)
        .Add(CooldownDurationSeconds)
        .Add(CurrentMultiShotConfig)
        .Add(FireRate)
        .Add(MaxAmmo)
        .Add(ModifiersWereApplied)
        .Add(MoveSpeedModifierOnUse)
        .Add(MultiShotConfigsPtr)
        .Add(OverShootDistance)
        .Add(Range)
        .Add(ReloadDurationSeconds)
        .Add(SetKccLookTowardsCamera)
        .Add(StartUseType)
        .Add(UseProceduralCameraOffsetsOnUse)
        .Value();
}

void WeaponStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponStats*>(ptr);
    SerializeField(&p->AmmoPerBurst, serializer);
    SerializeField(&p->CurrentMultiShotConfig, serializer);
    SerializeField(&p->MaxAmmo, serializer);
    SerializeField(&p->ConsumeOneAmmoPerMultiShot, serializer);
    SerializeField(&p->ModifiersWereApplied, serializer);
    SerializeField(&p->SetKccLookTowardsCamera, serializer);
    SerializeField(&p->UseProceduralCameraOffsetsOnUse, serializer);
    SerializeList<QMultiShotConfig>(&p->MultiShotConfigsPtr, serializer);
    SerializeField(&p->StartUseType, serializer);
    SerializeField(&p->CooldownDurationSeconds, serializer);
    SerializeField(&p->FireRate, serializer);
    SerializeField(&p->MoveSpeedModifierOnUse, serializer);
    SerializeField(&p->OverShootDistance, serializer);
    SerializeField(&p->Range, serializer);
    SerializeField(&p->ReloadDurationSeconds, serializer);
}

void WeaponStats::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (MultiShotConfigsPtr == Ptr::Null()) {
        MultiShotConfigsPtr = AllocateListPtr<QMultiShotConfig>(f, 8);
    }
}

void WeaponStats::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    MultiShotConfigsPtr = Ptr::Null();
}

std::int32_t Weapon::GetHashCode() const {
    return HashBuilder(3719u)
        .Add(AmmoInstance)
        .Add(AmmunitionAsset)
        .Add(BonusAmmoMultiplier)
        .Add(BonusFirerate)
        .Add(BufferedInputFrame)
        .Add(CooldownMultiplier)
        .Add(CooldownStartFrame)
        .Add(CurrentBurstFireRate)
        .Add(CurrentBurstShot)
        .Add(CurrentCooldownDuration)
        .Add(CurrentFireRate)
        .Add(EquippedFrame)
        .Add(FiredFrame)
        .Add(IgnoreEntitiesMultiplier)
        .Add(IsFiring)
        .Add(LobProjectileSpec)
        .Add(NextAllowedFireFrame)
        .Add(RawInputPressedFrame)
        .Add(ReleaseTriggerHandledExternally)
        .Add(ReloadStartFrame)
        .Add(RemainingAmmo)
        .Add(RemainingAmmoWhenStartAutoLoad)
        .Add(ShouldCancelNextFire)
        .Add(Stats)
        .Add(UsageModifersAppliedFrame)
        .Add(WeaponData)
        .Add(WeaponUser)
        .Add(_ChangedBeingUsed)
        .Add(_IsBeingUsed)
        .Add(_TriggerIsPressed)
        .Add(_TriggerPressedFrame)
        .Add(_TriggerReleasedFrame)
        .Add(_TriggerWasPressed)
        .Value();
}

void Weapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Weapon*>(ptr);
    SerializeField(&p->BufferedInputFrame, serializer);
    SerializeField(&p->CooldownStartFrame, serializer);
    SerializeField(&p->CurrentBurstShot, serializer);
    SerializeField(&p->EquippedFrame, serializer);
    SerializeField(&p->FiredFrame, serializer);
    SerializeField(&p->NextAllowedFireFrame, serializer);
    SerializeField(&p->RawInputPressedFrame, serializer);
    SerializeField(&p->ReloadStartFrame, serializer);
    SerializeField(&p->RemainingAmmo, serializer);
    SerializeField(&p->RemainingAmmoWhenStartAutoLoad, serializer);
    SerializeField(&p->UsageModifersAppliedFrame, serializer);
    SerializeField(&p->_TriggerPressedFrame, serializer);
    SerializeField(&p->_TriggerReleasedFrame, serializer);
    SerializeField(&p->IsFiring, serializer);
    SerializeField(&p->ReleaseTriggerHandledExternally, serializer);
    SerializeField(&p->ShouldCancelNextFire, serializer);
    SerializeField(&p->_ChangedBeingUsed, serializer);
    SerializeField(&p->_IsBeingUsed, serializer);
    SerializeField(&p->_TriggerIsPressed, serializer);
    SerializeField(&p->_TriggerWasPressed, serializer);
    SerializeField(&p->AmmunitionAsset, serializer);
    SerializeField(&p->LobProjectileSpec, serializer);
    SerializeField(&p->WeaponData, serializer);
    SerializeField(&p->AmmoInstance, serializer);
    SerializeField(&p->WeaponUser, serializer);
    SerializeField(&p->BonusAmmoMultiplier, serializer);
    SerializeField(&p->BonusFirerate, serializer);
    SerializeField(&p->CooldownMultiplier, serializer);
    SerializeField(&p->CurrentBurstFireRate, serializer);
    SerializeField(&p->CurrentCooldownDuration, serializer);
    SerializeField(&p->CurrentFireRate, serializer);
    SerializeField(&p->IgnoreEntitiesMultiplier, serializer);
    SerializeField(&p->Stats, serializer);
}

void Weapon::OnAdded(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Weapon*>(ptr)->AllocatePointers(static_cast<Frame*>(frame), entityRef);
}

void Weapon::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<Weapon*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t ChargingStats::GetHashCode() const {
    return HashBuilder(1327u)
        .Add(AutoReleaseOnFullCharge)
        .Add(FullChargeDuration)
        .Add(NumberOfChargeLevel)
        .Value();
}

void ChargingStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ChargingStats*>(ptr);
    SerializeField(&p->NumberOfChargeLevel, serializer);
    SerializeField(&p->AutoReleaseOnFullCharge, serializer);
    SerializeField(&p->FullChargeDuration, serializer);
}

std::int32_t WeaponBehaviour_Charging::GetHashCode() const {
    return HashBuilder(3727u)
        .Add(ChargingStartFrame)
        .Add(NeedReset)
        .Add(Spec)
        .Add(Stats)
        .Value();
}

void WeaponBehaviour_Charging::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponBehaviour_Charging*>(ptr);
    SerializeField(&p->ChargingStartFrame, serializer);
    SerializeField(&p->NeedReset, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Stats, serializer);
}

std::int32_t WeaponLoadoutItemData::GetHashCode() const {
    return HashBuilder(1987u)
        .Add(enhancementLevel)
        .Add(uniqueWeaponID)
        .Add(weaponLevel)
        .Value();
}

void WeaponLoadoutItemData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponLoadoutItemData*>(ptr);
    SerializeField(&p->enhancementLevel, serializer);
    SerializeField(&p->weaponLevel, serializer);
    SerializeField(&p->uniqueWeaponID, serializer);
}

std::int32_t WeaponLoadoutItem::GetHashCode() const {
    return HashBuilder(1979u)
        .Add(data)
        .Add(equippedFrame)
        .Add(spawnedWeapon)
        .Add(totalEquippeDurationInFrames)
        .Add(weaponProtoRef)
        .Value();
}

void WeaponLoadoutItem::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponLoadoutItem*>(ptr);
    SerializeField(&p->equippedFrame, serializer);
    SerializeField(&p->totalEquippeDurationInFrames, serializer);
    SerializeField(&p->weaponProtoRef, serializer);
    SerializeField(&p->spawnedWeapon, serializer);
    SerializeField(&p->data, serializer);
}

std::int32_t WeaponLoadout::GetHashCode() const {
    return HashBuilder(1951u)
        .Add(_slots_)
        .Value();
}

void WeaponLoadout::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponLoadout*>(ptr);
    SerializeField(&p->_slots_, serializer);
}

std::int32_t WeaponLoadoutActor::GetHashCode() const {
    return HashBuilder(3733u)
        .Add(activeSlot)
        .Add(disabled)
        .Add(infiniteFiringEnabled)
        .Add(loadout)
        .Add(previousSlotBeforeGadget)
        .Add(previousSlotBeforeMelee)
        .Add(previousSlotBeforeUltimate)
        .Add(spec)
        .Add(switchWeaponStartFrame)
        .Value();
}

void WeaponLoadoutActor::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponLoadoutActor*>(ptr);
    SerializeField(&p->switchWeaponStartFrame, serializer);
    SerializeField(&p->disabled, serializer);
    SerializeField(&p->infiniteFiringEnabled, serializer);
    SerializeField(&p->activeSlot, serializer);
    SerializeField(&p->previousSlotBeforeGadget, serializer);
    SerializeField(&p->previousSlotBeforeMelee, serializer);
    SerializeField(&p->previousSlotBeforeUltimate, serializer);
    SerializeField(&p->spec, serializer);
    SerializeField(&p->loadout, serializer);
}

std::int32_t WeaponProjectileSwitcherEnhancement::GetHashCode() const {
    return HashBuilder(3739u)
        .Add(OriginalAmmoAsset)
        .Add(ShotCounter)
        .Add(Spec)
        .Value();
}

void WeaponProjectileSwitcherEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponProjectileSwitcherEnhancement*>(ptr);
    SerializeField(&p->ShotCounter, serializer);
    SerializeField(&p->OriginalAmmoAsset, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t WeaponShowcaseGameMode::GetHashCode() const {
    return HashBuilder(3761u)
        .Add(currentDelay)
        .Add(currentTimer)
        .Add(lastRotationAngle)
        .Add(lastRotationPositon)
        .Add(playAnimation)
        .Add(shootAngle)
        .Add(shootPosition)
        .Add(spawnAngle)
        .Add(spawnPosition)
        .Add(startFrame)
        .Add(startTrigger)
        .Add(stopAnimation)
        .Add(stopTrigger)
        .Add(triggerIsPressed)
        .Add(userEntity)
        .Add(weaponEntity)
        .Value();
}

void WeaponShowcaseGameMode::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponShowcaseGameMode*>(ptr);
    SerializeField(&p->startFrame, serializer);
    SerializeField(&p->playAnimation, serializer);
    SerializeField(&p->startTrigger, serializer);
    SerializeField(&p->stopAnimation, serializer);
    SerializeField(&p->stopTrigger, serializer);
    SerializeField(&p->triggerIsPressed, serializer);
    SerializeField(&p->userEntity, serializer);
    SerializeField(&p->weaponEntity, serializer);
    SerializeField(&p->currentDelay, serializer);
    SerializeField(&p->currentTimer, serializer);
    SerializeField(&p->lastRotationAngle, serializer);
    SerializeField(&p->lastRotationPositon, serializer);
    SerializeField(&p->shootAngle, serializer);
    SerializeField(&p->shootPosition, serializer);
    SerializeField(&p->spawnAngle, serializer);
    SerializeField(&p->spawnPosition, serializer);
}

std::int32_t WeaponUser::GetHashCode() const {
    return HashBuilder(3767u)
        .Add(BonusAmmoMultiplier)
        .Add(DamageMultiplier)
        .Add(DamageMultiplier_Boss)
        .Add(DamageMultiplier_Electric)
        .Add(DamageMultiplier_Melee)
        .Add(DamageMultiplier_Mobs)
        .Add(DamageMultiplier_MoreBolts)
        .Add(DefaultDamageMultiplier)
        .Add(Disabled)
        .Add(EquippedWeapon)
        .Add(ForceMissEntity)
        .Add(ForceMissProbability)
        .Add(LastUsedFrame)
        .Add(LookPoint)
        .Add(LookPointEntity)
        .Add(LookPointIsRangedRaycastHit)
        .Add(ProjectedCameraPosition)
        .Add(ProjectedCameraSight)
        .Add(Prototype)
        .Add(ToLookPointApproximate)
        .Add(WeaponToLookPointRaycastAngle)
        .Add(WeaponToLookPointRaycastLength)
        .Add(WeapontoLookPointType)
        .Value();
}

void WeaponUser::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponUser*>(ptr);
    SerializeField(&p->LastUsedFrame, serializer);
    SerializeField(&p->WeapontoLookPointType, serializer);
    SerializeField(&p->Disabled, serializer);
    SerializeField(&p->LookPointIsRangedRaycastHit, serializer);
    SerializeField(&p->EquippedWeapon, serializer);
    SerializeField(&p->ForceMissEntity, serializer);
    SerializeField(&p->LookPointEntity, serializer);
    SerializeField(&p->BonusAmmoMultiplier, serializer);
    SerializeField(&p->DamageMultiplier, serializer);
    SerializeField(&p->DamageMultiplier_Boss, serializer);
    SerializeField(&p->DamageMultiplier_Electric, serializer);
    SerializeField(&p->DamageMultiplier_Melee, serializer);
    SerializeField(&p->DamageMultiplier_Mobs, serializer);
    SerializeField(&p->DamageMultiplier_MoreBolts, serializer);
    SerializeField(&p->DefaultDamageMultiplier, serializer);
    SerializeField(&p->ForceMissProbability, serializer);
    SerializeField(&p->WeaponToLookPointRaycastAngle, serializer);
    SerializeField(&p->WeaponToLookPointRaycastLength, serializer);
    SerializeField(&p->Prototype, serializer);
    SerializeField(&p->LookPoint, serializer);
    SerializeField(&p->ProjectedCameraPosition, serializer);
    SerializeField(&p->ProjectedCameraSight, serializer);
    SerializeField(&p->ToLookPointApproximate, serializer);
}

std::int32_t WrenchGroundSlamEnhancement::GetHashCode() const {
    return HashBuilder(3769u)
        .Add(Spec)
        .Value();
}

void WrenchGroundSlamEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WrenchGroundSlamEnhancement*>(ptr);
    SerializeField(&p->Spec, serializer);
}

std::int32_t WrenchThrowEnhancement::GetHashCode() const {
    return HashBuilder(3779u)
        .Add(FireButtonPressedFrame)
        .Add(IsArmed)
        .Add(Spec)
        .Value();
}

void WrenchThrowEnhancement::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WrenchThrowEnhancement*>(ptr);
    SerializeField(&p->FireButtonPressedFrame, serializer);
    SerializeField(&p->IsArmed, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t WrenchThrowProjectile::GetHashCode() const {
    return HashBuilder(3793u).Value();
}

void WrenchThrowProjectile::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t WrenchThrowWeapon::GetHashCode() const {
    return HashBuilder(3797u).Value();
}

void WrenchThrowWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    // Nothing to write: the component carries no data.
    (void)ptr;
    (void)serializer;
}

std::int32_t DelayedImpulseInfo::GetHashCode() const {
    return HashBuilder(1429u)
        .Add(comboCounter)
        .Add(forceInputDuration)
        .Add(impulse)
        .Add(impulseDelay)
        .Add(impulseDuration)
        .Add(impulseTime)
        .Add(inputFalloff)
        .Add(pending)
        .Add(target)
        .Value();
}

void DelayedImpulseInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DelayedImpulseInfo*>(ptr);
    SerializeField(&p->comboCounter, serializer);
    SerializeField(&p->impulseTime, serializer);
    SerializeField(&p->pending, serializer);
    SerializeField(&p->target, serializer);
    SerializeField(&p->forceInputDuration, serializer);
    SerializeField(&p->impulseDelay, serializer);
    SerializeField(&p->impulseDuration, serializer);
    SerializeField(&p->inputFalloff, serializer);
    SerializeField(&p->impulse, serializer);
}

std::int32_t WrenchWeapon::GetHashCode() const {
    return HashBuilder(3803u)
        .Add(ComboCounter)
        .Add(DelayedImpulse)
        .Add(Disabled)
        .Add(TimeToRemainAlignedWithDirection)
        .Add(ignoreMoveSpeedCounter)
        .Value();
}

void WrenchWeapon::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WrenchWeapon*>(ptr);
    SerializeField(&p->ComboCounter, serializer);
    SerializeField(&p->Disabled, serializer);
    SerializeField(&p->TimeToRemainAlignedWithDirection, serializer);
    SerializeField(&p->ignoreMoveSpeedCounter, serializer);
    SerializeField(&p->DelayedImpulse, serializer);
}

std::int32_t ZoneControlTeamScoring::GetHashCode() const {
    return HashBuilder(2011u)
        .Add(LastScoringFrame)
        .Add(TeamId)
        .Add(VictoryPoints)
        .Add(ZoneCompletion)
        .Value();
}

void ZoneControlTeamScoring::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ZoneControlTeamScoring*>(ptr);
    SerializeField(&p->VictoryPoints, serializer);
    SerializeField(&p->LastScoringFrame, serializer);
    SerializeField(&p->TeamId, serializer);
    SerializeField(&p->ZoneCompletion, serializer);
}

std::int32_t ZoneControlScoring::GetHashCode() const {
    return HashBuilder(2003u)
        .Add(TeamA)
        .Add(TeamB)
        .Value();
}

void ZoneControlScoring::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ZoneControlScoring*>(ptr);
    SerializeField(&p->TeamA, serializer);
    SerializeField(&p->TeamB, serializer);
}

std::int32_t ZoneControlGameMode::GetHashCode() const {
    return HashBuilder(3821u)
        .Add(ActiveZoneIndex)
        .Add(ControlZoneStateHoldersPtr)
        .Add(Scoring)
        .Add(Spec)
        .Add(ZonesActivationOrderPtr)
        .Value();
}

void ZoneControlGameMode::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ZoneControlGameMode*>(ptr);
    SerializeField(&p->ActiveZoneIndex, serializer);
    SerializeList<ControlZoneStateHolder>(&p->ControlZoneStateHoldersPtr, serializer);
    SerializeList<std::int32_t>(&p->ZonesActivationOrderPtr, serializer);
    SerializeField(&p->Spec, serializer);
    SerializeField(&p->Scoring, serializer);
}

void ZoneControlGameMode::ClearPointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    (void)f;
    ControlZoneStateHoldersPtr = Ptr::Null();
    ZonesActivationOrderPtr = Ptr::Null();
}

void ZoneControlGameMode::OnRemoved(Core::FrameBase* frame, EntityRef entityRef, void* ptr) {
    static_cast<ZoneControlGameMode*>(ptr)->ClearPointers(static_cast<Frame*>(frame), entityRef);
}

std::int32_t HealthHitResult::GetHashCode() const {
    return HashBuilder(1559u)
        .Add(damages)
        .Add(overHealthDamages)
        .Value();
}

std::int32_t ShieldHitResult::GetHashCode() const {
    return HashBuilder(1831u)
        .Add(damages)
        .Value();
}

std::int32_t HitResult::GetHashCode() const {
    return HashBuilder(1583u)
        .Add(health)
        .Add(shield)
        .Value();
}

std::int32_t DamageToImpulseData::GetHashCode() const {
    return HashBuilder(1427u)
        .Add(delay)
        .Add(impulse)
        .Add(priority)
        .Add(resetVelocity)
        .Value();
}

std::int32_t OnSpawnEntityOnDeathParams::GetHashCode() const {
    return HashBuilder(1663u)
        .Add(dying)
        .Add(raycastCoef)
        .Add(spawning)
        .Value();
}

std::int32_t OnPushedData::GetHashCode() const {
    return HashBuilder(1657u)
        .Add(isPushStop)
        .Add(pushStopDuration)
        .Value();
}

std::int32_t GameModeSettings::GetHashCode() const {
    return HashBuilder(1523u)
        .Add(gameMode)
        .Add(showcaseWeaponId)
        .Value();
}

std::int32_t BotWeightedExtraPower::GetHashCode() const {
    return HashBuilder(1319u)
        .Add(extraPowerStats)
        .Add(weight)
        .Value();
}

void BotWeightedExtraPower::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotWeightedExtraPower*>(ptr);
    SerializeField(&p->extraPowerStats, serializer);
    SerializeField(&p->weight, serializer);
}

std::int32_t CustomBotInfo::GetHashCode() const {
    return HashBuilder(1399u)
        .Add(nickname)
        .Add(skinId)
        .Add(teamId)
        .Value();
}

void CustomBotInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CustomBotInfo*>(ptr);
    SerializeField(&p->skinId, serializer);
    SerializeField(&p->teamId, serializer);
    SerializeField(&p->nickname, serializer);
}

std::int32_t BotsSettings::GetHashCode() const {
    return HashBuilder(1321u)
        .Add(BackpackBracketPlayerPlacement)
        .Add(BackpackLevelBracketPercentage)
        .Add(BotLoadoutPresets)
        .Add(BotsDifficulty_0_100)
        .Add(BotsPowerStats)
        .Add(CustomBotCount)
        .Add(_CustomBots_)
        .Add(DamageMultiplier)
        .Add(_ExtraPowerLevels_)
        .Add(FixedBotWeapons)
        .Add(NumBotLoadoutPresets)
        .Add(PossibleBotCharacterIDs)
        .Add(PossibleBotWeaponIDs)
        .Add(RoomMasterBucketIndex)
        .Add(RoomMasterBucketIndexForChoosingHeroesAndWeapons)
        .Value();
}

std::int32_t UnlockedFeatures::GetHashCode() const {
    return HashBuilder(1931u)
        .Add(Gameplay_CameraMovement)
        .Add(Gameplay_CharacterMovement)
        .Add(Gameplay_Dash)
        .Add(Gameplay_DoubleJump)
        .Add(Gameplay_Fire)
        .Add(Gameplay_FirstWeapon)
        .Add(Gameplay_GadgetWeapon)
        .Add(Gameplay_Helipack)
        .Add(Gameplay_Jump)
        .Add(Gameplay_Melee)
        .Add(Gameplay_SecondWeapon_Deprecated)
        .Add(Gameplay_ThirdWeapon_Deprecated)
        .Add(Gameplay_UltimateWeapon)
        .Add(HUD_BoltCounter)
        .Add(HUD_BoltLevel)
        .Add(HUD_Health)
        .Add(HUD_Ranking)
        .Add(HUD_Shield)
        .Add(HUD_Timer)
        .Value();
}

std::int32_t BlackboardEntry::GetHashCode() const {
    return HashBuilder(1279u)
        .Add(ReactiveDecoratorsPtr)
        .Add(Value)
        .Value();
}

void BlackboardEntry::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BlackboardEntry*>(ptr);
    SerializeList<AssetRefBTDecorator>(&p->ReactiveDecoratorsPtr, serializer);
    SerializeField(&p->Value, serializer);
}

std::int32_t NavBlacklistEntry::GetHashCode() const {
    return HashBuilder(1637u)
        .Add(entity)
        .Add(frame)
        .Add(position)
        .Value();
}

void NavBlacklistEntry::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<NavBlacklistEntry*>(ptr);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->position, serializer);
}

std::int32_t AISpawned::GetHashCode() const {
    return HashBuilder(1231u)
        .Add(destroyedAtFrame)
        .Add(entity)
        .Value();
}

void AISpawned::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<AISpawned*>(ptr);
    SerializeField(&p->destroyedAtFrame, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t ActiveCheckpoint::GetHashCode() const {
    return HashBuilder(1237u)
        .Add(Actor)
        .Add(Checkpoint)
        .Value();
}

void ActiveCheckpoint::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActiveCheckpoint*>(ptr);
    SerializeField(&p->Actor, serializer);
    SerializeField(&p->Checkpoint, serializer);
}

std::int32_t RespawnTimers::GetHashCode() const {
    return HashBuilder(1777u)
        .Add(IsRespawnPointCalculated)
        .Add(Position)
        .Add(Rotation)
        .Add(ToRespawn)
        .Add(UnspawnedFrame)
        .Value();
}

void RespawnTimers::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RespawnTimers*>(ptr);
    SerializeField(&p->UnspawnedFrame, serializer);
    SerializeField(&p->IsRespawnPointCalculated, serializer);
    SerializeField(&p->ToRespawn, serializer);
    SerializeField(&p->Position, serializer);
    SerializeField(&p->Rotation, serializer);
}

std::int32_t SpawnPointDynamicData::GetHashCode() const {
    return HashBuilder(1867u)
        .Add(isOutsideShrinkZone)
        .Add(lastRespawnFrame)
        .Add(numActorsInInnerRange)
        .Add(numActorsInOuterRange)
        .Value();
}

void SpawnPointDynamicData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<SpawnPointDynamicData*>(ptr);
    SerializeField(&p->lastRespawnFrame, serializer);
    SerializeField(&p->numActorsInInnerRange, serializer);
    SerializeField(&p->numActorsInOuterRange, serializer);
    SerializeField(&p->isOutsideShrinkZone, serializer);
}

std::int32_t DoomGroupMember::GetHashCode() const {
    return HashBuilder(1433u)
        .Add(entity)
        .Add(joinedGroupFrame)
        .Value();
}

void DoomGroupMember::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DoomGroupMember*>(ptr);
    SerializeField(&p->joinedGroupFrame, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t TrailPoint::GetHashCode() const {
    return HashBuilder(1889u)
        .Add(frameAdded)
        .Add(position)
        .Add(trailCutFromPoint)
        .Value();
}

void TrailPoint::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TrailPoint*>(ptr);
    SerializeField(&p->trailCutFromPoint, serializer);
    SerializeField(&p->frameAdded, serializer);
    SerializeField(&p->position, serializer);
}

std::int32_t EffectHitInfo::GetHashCode() const {
    return HashBuilder(1453u)
        .Add(Entity)
        .Add(Point)
        .Value();
}

void EffectHitInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EffectHitInfo*>(ptr);
    SerializeField(&p->Entity, serializer);
    SerializeField(&p->Point, serializer);
}

std::int32_t EffectedEntity::GetHashCode() const {
    return HashBuilder(1459u)
        .Add(Entity)
        .Add(Frame)
        .Value();
}

void EffectedEntity::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EffectedEntity*>(ptr);
    SerializeField(&p->Frame, serializer);
    SerializeField(&p->Entity, serializer);
}

std::int32_t PowerUpStats::GetHashCode() const {
    return HashBuilder(1699u)
        .Add(Amount)
        .Add(Source)
        .Add(Type)
        .Value();
}

void PowerUpStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PowerUpStats*>(ptr);
    SerializeField(&p->Source, serializer);
    SerializeField(&p->Amount, serializer);
    SerializeField(&p->Type, serializer);
}

std::int32_t WeaponUsageStats::GetHashCode() const {
    return HashBuilder(1997u)
        .Add(numActorKilled)
        .Add(numCratesBroken)
        .Add(numCriticals_Mobs)
        .Add(numCriticals_Players)
        .Add(numHeadshots_Players)
        .Add(numHits_Env)
        .Add(numHits_Mobs)
        .Add(numHits_Players)
        .Add(numNPCKilled)
        .Add(numShots)
        .Add(totalDamageDealt)
        .Value();
}

void WeaponUsageStats::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<WeaponUsageStats*>(ptr);
    SerializeField(&p->numActorKilled, serializer);
    SerializeField(&p->numCratesBroken, serializer);
    SerializeField(&p->numCriticals_Mobs, serializer);
    SerializeField(&p->numCriticals_Players, serializer);
    SerializeField(&p->numHeadshots_Players, serializer);
    SerializeField(&p->numHits_Env, serializer);
    SerializeField(&p->numHits_Mobs, serializer);
    SerializeField(&p->numHits_Players, serializer);
    SerializeField(&p->numNPCKilled, serializer);
    SerializeField(&p->numShots, serializer);
    SerializeField(&p->totalDamageDealt, serializer);
}

std::int32_t CollidingEntity::GetHashCode() const {
    return HashBuilder(1367u)
        .Add(collisionFrame)
        .Add(collisionVelocity)
        .Add(entity)
        .Add(hitNormal)
        .Add(impactPosition)
        .Value();
}

void CollidingEntity::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CollidingEntity*>(ptr);
    SerializeField(&p->collisionFrame, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->collisionVelocity, serializer);
    SerializeField(&p->hitNormal, serializer);
    SerializeField(&p->impactPosition, serializer);
}

std::int32_t ResetValues::GetHashCode() const {
    return HashBuilder(1759u)
        .Add(position)
        .Add(rotation)
        .Value();
}

void ResetValues::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ResetValues*>(ptr);
    SerializeField(&p->position, serializer);
    SerializeField(&p->rotation, serializer);
}

std::int32_t GoalScoreFrame::GetHashCode() const {
    return HashBuilder(1549u)
        .Add(frame)
        .Add(team)
        .Value();
}

void GoalScoreFrame::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GoalScoreFrame*>(ptr);
    SerializeField(&p->frame, serializer);
    SerializeField(&p->team, serializer);
}

std::int32_t BotUTData::GetHashCode() const {
    return HashBuilder(1307u)
        .Add(rank)
        .Add(score)
        .Value();
}

void BotUTData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotUTData*>(ptr);
    SerializeField(&p->rank, serializer);
    SerializeField(&p->score, serializer);
}

std::int32_t RicochetTargetInfo::GetHashCode() const {
    return HashBuilder(1787u)
        .Add(entity)
        .Add(priority)
        .Value();
}

void RicochetTargetInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RicochetTargetInfo*>(ptr);
    SerializeField(&p->priority, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t DamageIncreaseData::GetHashCode() const {
    return HashBuilder(1409u)
        .Add(DamagePercentIncrease)
        .Add(Duration)
        .Add(StartFrame)
        .Value();
}

void DamageIncreaseData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageIncreaseData*>(ptr);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->DamagePercentIncrease, serializer);
    SerializeField(&p->Duration, serializer);
}

std::int32_t DamageMultiplierData::GetHashCode() const {
    return HashBuilder(1423u)
        .Add(endFrame)
        .Add(origin)
        .Add(value)
        .Value();
}

void DamageMultiplierData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DamageMultiplierData*>(ptr);
    SerializeField(&p->origin, serializer);
    SerializeField(&p->endFrame, serializer);
    SerializeField(&p->value, serializer);
}

std::int32_t CollectData::GetHashCode() const {
    return HashBuilder(1361u)
        .Add(collectCount)
        .Add(collectStartFrame)
        .Value();
}

void CollectData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<CollectData*>(ptr);
    SerializeField(&p->collectCount, serializer);
    SerializeField(&p->collectStartFrame, serializer);
}

std::int32_t DropAnimData::GetHashCode() const {
    return HashBuilder(1447u)
        .Add(dropPos)
        .Add(targetPos)
        .Value();
}

void DropAnimData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<DropAnimData*>(ptr);
    SerializeField(&p->dropPos, serializer);
    SerializeField(&p->targetPos, serializer);
}

std::int32_t Droppable::GetHashCode() const {
    return HashBuilder(1451u)
        .Add(collectorIndex)
        .Add(collectorVersion)
        .Add(destroyFrame)
        .Add(dropFrame)
        .Add(dropperIndex)
        .Add(dropperVersion)
        .Add(id)
        .Add(position)
        .Add(shortRange)
        .Add(state)
        .Add(teamIndex)
        .Add(type)
        .Value();
}

void Droppable::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Droppable*>(ptr);
    SerializeField(&p->collectorVersion, serializer);
    SerializeField(&p->dropperVersion, serializer);
    SerializeField(&p->teamIndex, serializer);
    SerializeField(&p->state, serializer);
    SerializeField(&p->type, serializer);
    SerializeField(&p->collectorIndex, serializer);
    SerializeField(&p->destroyFrame, serializer);
    SerializeField(&p->dropFrame, serializer);
    SerializeField(&p->dropperIndex, serializer);
    SerializeField(&p->id, serializer);
    SerializeField(&p->shortRange, serializer);
    SerializeField(&p->position, serializer);
}

std::int32_t EntityGroupMember::GetHashCode() const {
    return HashBuilder(1481u)
        .Add(Entity)
        .Add(JoinedGroupFrame)
        .Value();
}

void EntityGroupMember::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<EntityGroupMember*>(ptr);
    SerializeField(&p->JoinedGroupFrame, serializer);
    SerializeField(&p->Entity, serializer);
}

std::int32_t GatlingFireRateLevel::GetHashCode() const {
    return HashBuilder(1543u)
        .Add(fireRate)
        .Add(isProgressive)
        .Add(spinUpTime)
        .Value();
}

void GatlingFireRateLevel::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<GatlingFireRateLevel*>(ptr);
    SerializeField(&p->isProgressive, serializer);
    SerializeField(&p->fireRate, serializer);
    SerializeField(&p->spinUpTime, serializer);
}

std::int32_t PendingGoldBoltSpawn::GetHashCode() const {
    return HashBuilder(1693u)
        .Add(SpawnFrame)
        .Add(SpawnPointIndex)
        .Value();
}

void PendingGoldBoltSpawn::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<PendingGoldBoltSpawn*>(ptr);
    SerializeField(&p->SpawnFrame, serializer);
    SerializeField(&p->SpawnPointIndex, serializer);
}

std::int32_t Overhealth::GetHashCode() const {
    return HashBuilder(1667u)
        .Add(DecreaseDuration)
        .Add(DecreaseTickRate)
        .Add(Duration)
        .Add(StartFrame)
        .Add(Value)
        .Value();
}

void Overhealth::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Overhealth*>(ptr);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->DecreaseDuration, serializer);
    SerializeField(&p->DecreaseTickRate, serializer);
    SerializeField(&p->Duration, serializer);
    SerializeField(&p->Value, serializer);
}

std::int32_t ModInfo::GetHashCode() const {
    return HashBuilder(1627u)
        .Add(date)
        .Add(duration)
        .Add(falloffDuration)
        .Add(origin)
        .Add(value)
        .Value();
}

void ModInfo::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ModInfo*>(ptr);
    SerializeField(&p->origin, serializer);
    SerializeField(&p->date, serializer);
    SerializeField(&p->duration, serializer);
    SerializeField(&p->falloffDuration, serializer);
    SerializeField(&p->value, serializer);
}

std::int32_t KillZoneArea::GetHashCode() const {
    return HashBuilder(1607u)
        .Add(corner)
        .Add(size)
        .Add(surfaceType)
        .Value();
}

void KillZoneArea::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<KillZoneArea*>(ptr);
    SerializeField(&p->surfaceType, serializer);
    SerializeField(&p->size, serializer);
    SerializeField(&p->corner, serializer);
}

std::int32_t ActivePowerUp::GetHashCode() const {
    return HashBuilder(1249u)
        .Add(PowerUpType)
        .Add(Spec)
        .Add(StartFrame)
        .Value();
}

void ActivePowerUp::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ActivePowerUp*>(ptr);
    SerializeField(&p->StartFrame, serializer);
    SerializeField(&p->PowerUpType, serializer);
    SerializeField(&p->Spec, serializer);
}

std::int32_t RankingItem::GetHashCode() const {
    return HashBuilder(1753u)
        .Add(eliminationFrame)
        .Add(entity)
        .Add(rank)
        .Add(score)
        .Add(scoreFrame)
        .Add(teamId)
        .Value();
}

void RankingItem::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<RankingItem*>(ptr);
    SerializeField(&p->rank, serializer);
    SerializeField(&p->score, serializer);
    SerializeField(&p->eliminationFrame, serializer);
    SerializeField(&p->scoreFrame, serializer);
    SerializeField(&p->teamId, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t Reward::GetHashCode() const {
    return HashBuilder(1783u)
        .Add(amount)
        .Add(collectFrame)
        .Add(isRival)
        .Add(reason)
        .Add(sourceEntityPosition)
        .Add(type)
        .Value();
}

void Reward::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Reward*>(ptr);
    SerializeField(&p->type, serializer);
    SerializeField(&p->amount, serializer);
    SerializeField(&p->collectFrame, serializer);
    SerializeField(&p->isRival, serializer);
    SerializeField(&p->reason, serializer);
    SerializeField(&p->sourceEntityPosition, serializer);
}

std::int32_t LockedTarget::GetHashCode() const {
    return HashBuilder(1621u)
        .Add(entity)
        .Add(isLocked)
        .Add(startLockFrame)
        .Value();
}

void LockedTarget::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<LockedTarget*>(ptr);
    SerializeField(&p->startLockFrame, serializer);
    SerializeField(&p->isLocked, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t ShieldGroupMember::GetHashCode() const {
    return HashBuilder(1811u)
        .Add(entity)
        .Add(joinedGroupFrame)
        .Value();
}

void ShieldGroupMember::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ShieldGroupMember*>(ptr);
    SerializeField(&p->joinedGroupFrame, serializer);
    SerializeField(&p->entity, serializer);
}

std::int32_t Stat::GetHashCode() const {
    return HashBuilder(1871u)
        .Add(id)
        .Add(value)
        .Value();
}

void Stat::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<Stat*>(ptr);
    SerializeField(&p->id, serializer);
    SerializeField(&p->value, serializer);
}

std::int32_t StatSummary::GetHashCode() const {
    return HashBuilder(1873u)
        .Add(avg)
        .Add(lastValue)
        .Add(max)
        .Add(min)
        .Value();
}

void StatSummary::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<StatSummary*>(ptr);
    SerializeField(&p->avg, serializer);
    SerializeField(&p->lastValue, serializer);
    SerializeField(&p->max, serializer);
    SerializeField(&p->min, serializer);
}

std::int32_t StreamPoint::GetHashCode() const {
    return HashBuilder(1877u)
        .Add(creationFrame)
        .Add(entity)
        .Add(position)
        .Add(previousPointEntity)
        .Value();
}

void StreamPoint::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<StreamPoint*>(ptr);
    SerializeField(&p->creationFrame, serializer);
    SerializeField(&p->entity, serializer);
    SerializeField(&p->previousPointEntity, serializer);
    SerializeField(&p->position, serializer);
}

std::int32_t TargetingData::GetHashCode() const {
    return HashBuilder(1879u)
        .Add(target)
        .Add(targetPoint)
        .Value();
}

void TargetingData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<TargetingData*>(ptr);
    SerializeField(&p->target, serializer);
    SerializeField(&p->targetPoint, serializer);
}

std::int32_t ControlZoneStateHolder::GetHashCode() const {
    return HashBuilder(1381u)
        .Add(ControlPercent)
        .Add(LastControllerTeamId)
        .Add(LastFullControlFrame)
        .Add(MapCustomDataIndex)
        .Add(NbOfControlSwitches)
        .Add(ScoringFrame)
        .Add(State)
        .Add(StateStartFrame)
        .Value();
}

void ControlZoneStateHolder::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<ControlZoneStateHolder*>(ptr);
    SerializeField(&p->State, serializer);
    SerializeField(&p->LastControllerTeamId, serializer);
    SerializeField(&p->LastFullControlFrame, serializer);
    SerializeField(&p->MapCustomDataIndex, serializer);
    SerializeField(&p->NbOfControlSwitches, serializer);
    SerializeField(&p->ScoringFrame, serializer);
    SerializeField(&p->StateStartFrame, serializer);
    SerializeField(&p->ControlPercent, serializer);
}

std::int32_t BotSDKData::GetHashCode() const {
    return HashBuilder(1301u)
        .Add(ElapsedPartialTicks)
        .Add(ElapsedTicks)
        .Add(OriginalDeltaTime)
        .Value();
}

void BotSDKData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<BotSDKData*>(ptr);
    SerializeField(&p->ElapsedTicks, serializer);
    SerializeField(&p->ElapsedPartialTicks, serializer);
    SerializeField(&p->OriginalDeltaTime, serializer);
}

std::int32_t UTMomentumData::GetHashCode() const {
    return HashBuilder(1901u)
        .Add(DecayAmount)
        .Add(Value)
        .Value();
}

void UTMomentumData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UTMomentumData*>(ptr);
    SerializeField(&p->DecayAmount, serializer);
    SerializeField(&p->Value, serializer);
}

std::int32_t UTMomentumPack::GetHashCode() const {
    return HashBuilder(1907u)
        .Add(ConsiderationRef)
        .Add(MomentumData)
        .Value();
}

void UTMomentumPack::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<UTMomentumPack*>(ptr);
    SerializeField(&p->ConsiderationRef, serializer);
    SerializeField(&p->MomentumData, serializer);
}

std::int32_t QMultiShotConfig::GetHashCode() const {
    return HashBuilder(1741u)
        .Add(DataPtr)
        .Value();
}

void QMultiShotConfig::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<QMultiShotConfig*>(ptr);
    SerializeList<QMultiShotData>(&p->DataPtr, serializer);
}

std::int32_t QMultiShotData::GetHashCode() const {
    return HashBuilder(1747u)
        .Add(ShotAmmunition)
        .Add(SpawnOffset)
        .Add(UseRandomOffset)
        .Add(UseSpecialAmmunition)
        .Value();
}

void QMultiShotData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* p = static_cast<QMultiShotData*>(ptr);
    SerializeField(&p->UseRandomOffset, serializer);
    SerializeField(&p->UseSpecialAmmunition, serializer);
    SerializeField(&p->ShotAmmunition, serializer);
    SerializeField(&p->SpawnOffset, serializer);
}

} // namespace Quantum
