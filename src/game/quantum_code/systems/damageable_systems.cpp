// damageable_systems.cpp - DamageableSystem, ShieldedActorLib, ShieldedActorSystem, the
// Damageable / Projectile component helpers and ElementalDamageTable's lookups.
#include "quantum_code/systems/damageable_systems.h"

#include "photon_deterministic/fp_math.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/battle_stats_systems.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/health_systems.h"
#include "quantum_code/systems/libs.h"

#include <stdexcept>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

namespace {

// The frame `duration` from now (the invulnerability and regeneration timers all round
// this way).
std::int32_t FrameAfter(Frame* frame, FP duration) {
    return frame->Number + (duration / frame->get_DeltaTime()).AsInt();
}

// A "1 + bonus" damage factor as the damage hooks use it: never negative.
FP BonusFactor(FP bonus) {
    return FPMath::Clamp(bonus + FP::One(), FP{}, FP::MaxValue());
}

// The tail of DamageableSystem.ResolveHits: a resolved hit by a player's normal loadout
// weapon counts once in that weapon's usage statistics (by target kind, criticals and
// headshots).
void CountWeaponHit(Frame* frame, EntityRef hittenEntity, const HitData& hit) {
    const EntityRef owner = hit.owner;
    const EntityRef source = hit.source;
    const EntityRef weapon = hit.weapon;
    if (weapon == EntityRef::None() || hittenEntity == owner || !frame->Has<PlayerActor>(owner)) {
        return;
    }
    BattleStatsActor* battleStats = nullptr;
    DamageSource* damageSource = nullptr;
    Code::EntityInfo targetInfo{};
    Weapon* weaponComponent = nullptr;
    WeaponSpec* weaponSpec = nullptr;
    if (!frame->TryGetPointer<BattleStatsActor>(owner, battleStats) ||
        !frame->TryGetPointer<DamageSource>(source, damageSource) ||
        !frame->TryGet<Code::EntityInfo>(hittenEntity, targetInfo) ||
        !frame->TryGetPointer<Weapon>(weapon, weaponComponent) ||
        !frame->TryFindAssetInternal(weaponComponent->WeaponData.Id, true, weaponSpec) ||
        weaponSpec->LoadoutSlotType != WeaponLoadoutSlotType::NORMAL) {
        return;
    }
    WeaponUsageStats* stats = nullptr;
    if (!BattleStatsActorSystem::TryGetWeaponUsageStats(frame, battleStats, owner, weapon, stats) ||
        damageSource->hitTrackingDone) {
        return;
    }
    if (EntityInfoLib::FilterContains(ENTITY_INFO_FILTER::NPC, targetInfo.type)) {
        ++stats->numHits_Mobs;
        if (hit.isCriticalHit) {
            ++stats->numCriticals_Mobs;
        }
    } else if (targetInfo.type == EntityInfoType::Player) {
        ++stats->numHits_Players;
        Projectile* projectile = nullptr;
        if (frame->TryGetPointer<Projectile>(source, projectile) && projectile->IsImpactHeadshot(frame)) {
            ++stats->numHeadshots_Players;
        } else if (hit.isCriticalHit) {
            ++stats->numCriticals_Players;
        }
    } else {
        ++stats->numHits_Env;
    }
    damageSource->hitTrackingDone = true;
}

} // namespace

// --- ElementalDamageTable (asset) ----------------------------------------------------------------

bool ElementalDamageTable::TryGetTable(Frame* frame, ElementalDamageTable*& table) {
    return frame->TryFindAssetInternal(frame->get_RuntimeConfig()->elementalDamageTable.Id, true, table);
}

FP ElementalDamageTable::GetDamageMultiplier(Frame* frame, const ElementalDamageTable* table, DamageType damageType,
                                             ElementType elementType) {
    (void)frame;
    if (table == nullptr || damageType == DamageType::None || elementType == ElementType::None) {
        return FP::One();
    }
    for (const DamageTypeToElementMapping& mapping : table->mappings) {
        if (mapping.damageType != damageType) {
            continue;
        }
        for (const ElementMultiplier& element : mapping.elementMultipliers) {
            if (element.elementType == elementType) {
                return element.multiplier;
            }
        }
        return FP::One();
    }
    return FP::One();
}

// --- Damageable / Projectile (component helpers) ------------------------------------------------

void Damageable::SetInvulnerable(Frame* frame, FP duration, INVULNERABILITY_FILTER invulnerabilityType) {
    invulnerabilityEndFrame = FrameAfter(frame, duration);
    invulnerabilityFilter = invulnerabilityType;
}

bool Damageable::IsInvulnerable(Frame* frame, EntityRef damageableEntity) const {
    (void)damageableEntity;
    return invulnerabilityEndFrame > frame->Number && invulnerabilityFilter == INVULNERABILITY_FILTER::ALL;
}

bool Damageable::IsInvulnerableToDamage(Frame* frame, EntityRef damageableEntity) const {
    (void)damageableEntity;
    return invulnerabilityEndFrame > frame->Number &&
           DamageLib::Contains(invulnerabilityFilter, INVULNERABILITY_FILTER::DAMAGE);
}

bool Damageable::IsInvulnerableToPush(Frame* frame, EntityRef damageableEntity) const {
    (void)damageableEntity;
    return invulnerabilityEndFrame > frame->Number &&
           DamageLib::Contains(invulnerabilityFilter, INVULNERABILITY_FILTER::PUSH);
}

FP Damageable::GetInvulnerabilityRemainingDuration(Frame* frame, EntityRef damageableEntity) const {
    (void)damageableEntity;
    const std::int32_t framesLeft = invulnerabilityEndFrame - frame->Number;
    return framesLeft > 0 ? frame->get_DeltaTime() * framesLeft : FP{};
}

void Damageable::Hit(Frame* frame, EntityRef damageableEntity, const HitData& hitData) {
    if (hitData.damages.RawValue() < 1 || IsInvulnerableToDamage(frame, damageableEntity)) {
        return;
    }
    frame->ResolveList(get_hits()).Add(hitData);
}

bool Projectile::IsImpactHeadshot(Frame* frame) const {
    if (!HasValidCollisionImpact || !frame->Exists(OwnerWeapon)) {
        return false;
    }
    Weapon* weapon = nullptr;
    WeaponSpec* weaponSpec = nullptr;
    if (!frame->TryGetPointer<Weapon>(OwnerWeapon, weapon) ||
        !frame->TryFindAssetInternal(weapon->WeaponData.Id, true, weaponSpec) || !weaponSpec->CanHeadshot) {
        return false;
    }
    return Impact.ShapeUserTag == 1;
}

// --- DamageableSystem ------------------------------------------------------------------------------

void DamageableSystem::Update(Frame* frame, Filter& filter) {
    if (IsDead(frame, filter.entity)) {
        return;
    }
    DamageableSpec* spec = nullptr;
    ElementalDamageTable* table = nullptr;
    if (!frame->TryFindAssetInternal(filter.damageable->Spec.Id, true, spec) ||
        !ElementalDamageTable::TryGetTable(frame, table)) {
        return;
    }
    ResolveHits(frame, filter.entity, filter.damageable, spec, table);
    ResetElectricalDamageGuage(frame, filter.damageable, table);
    ResetIceBuildUp(frame, filter.damageable, table);
}

void DamageableSystem::ResetElectricalDamageGuage(Frame* frame, Damageable* damageable,
                                                  const ElementalDamageTable* elementalDamageTable) {
    if (damageable->electricalDamageLastFrame == 0) {
        return;
    }
    const FP sinceLastHit = frame->get_DeltaTime() * (frame->Number - damageable->electricalDamageLastFrame);
    if (sinceLastHit >= elementalDamageTable->electricalDamageSettings.buildUpResetAfter) {
        damageable->electricalDamageLastFrame = 0;
        damageable->electricalCurrentBuildUp = FP{};
    }
}

void DamageableSystem::ResetIceBuildUp(Frame* frame, Damageable* damageable, const ElementalDamageTable* elementalDamageTable) {
    if (damageable->iceDamageLastFrame == 0) {
        return;
    }
    const FP sinceLastHit = frame->get_DeltaTime() * (frame->Number - damageable->iceDamageLastFrame);
    if (sinceLastHit >= elementalDamageTable->iceDamageSettings.buildUpResetAfter) {
        damageable->iceDamageLastFrame = 0;
        damageable->iceCurrentBuildUp = FP{};
    }
}

bool DamageableSystem::IsDead(Frame* frame, EntityRef entity) {
    return frame->Has<Killable>(entity) && !KillableLib::IsAlive(frame, entity);
}

void DamageableSystem::ResolveHits(Frame* frame, EntityRef hittenEntity, Damageable* damageable,
                                   DamageableSpec* damageableSpec, const ElementalDamageTable* elementalDamageTable) {
    Collections::QList<HitData> hits = frame->ResolveList(damageable->get_hits());
    if (hits.get_Count() == 0) {
        return;
    }
    const bool invulnerable = damageable->invulnerabilityEndFrame > frame->Number &&
                              (static_cast<std::int32_t>(damageable->invulnerabilityFilter) & 1) != 0;
    if (!invulnerable) {
        for (int i = 0; i < hits.get_Count(); ++i) {
            if (IsDead(frame, hittenEntity)) {
                break;
            }
            HitData* hit = hits.GetPointer(i);
            hit->damages = hit->damages * GetElementalDamageMultiplier(frame, elementalDamageTable, hit, damageableSpec);
            HitResult result{};
            result.shield = ResolveShieldDamages(frame, hittenEntity, hit);
            result.health = ResolveHealthDamages(frame, hittenEntity, hit);
            damageable->hitBy = *hit;
            frame->Signals.Damageable_HitReceived(hittenEntity, *hit, result);
            frame->Events.Damageable_HitReceived(hittenEntity, *hit, result);
            // Receivers may have queued more hits; the list storage can have moved.
            CountWeaponHit(frame, hittenEntity, *hits.GetPointer(i));
        }
    }
    hits.Clear();
}

FP DamageableSystem::GetElementalDamageMultiplier(Frame* frame, const ElementalDamageTable* elementalDamageTable,
                                                  HitData* hit, DamageableSpec* damageableSpec) {
    FP multiplier =
        ElementalDamageTable::GetDamageMultiplier(frame, elementalDamageTable, hit->type, damageableSpec->ElementType);
    if (damageableSpec->InstantKillByBuffedDamageTypes && multiplier > FP::One()) {
        // Large enough that the scaled hit outweighs any health it can reach.
        const FP largestHitMultiplier =
            FPMath::Max(FP::One(), FPMath::Max(hit->shield.damageMultiplier, hit->health.damageMultiplier));
        multiplier = FP::UseableMax() / largestHitMultiplier;
    }
    return multiplier;
}

ShieldHitResult DamageableSystem::ResolveShieldDamages(Frame* frame, EntityRef entity, HitData* hitData) {
    ShieldedActor* shieldedActor = nullptr;
    if (hitData->shield.ignoreShield || !frame->TryGetPointer<ShieldedActor>(entity, shieldedActor)) {
        return ShieldHitResult::get_NotDamaged();
    }
    const ShieldHitResult result = ShieldedActorLib::ApplyHit(frame, entity, *hitData);
    if (result.damages.RawValue() < 1) {
        return ShieldHitResult::get_NotDamaged();
    }
    hitData->CancelHealthDamage();
    return result;
}

HealthHitResult DamageableSystem::ResolveHealthDamages(Frame* frame, EntityRef entity, HitData* hitData) {
    Health* health = nullptr;
    if (hitData->health.damageMultiplier.RawValue() < 1 || !frame->TryGetPointer<Health>(entity, health)) {
        return HealthHitResult::get_NotDamaged();
    }
    return HealthLib::ApplyHit(frame, entity, *hitData);
}

void DamageableSystem::Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                                         const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    Damageable* damageable = nullptr;
    DamageableSpec* spec = nullptr;
    if (frame->TryGetPointer<Damageable>(entity, damageable) &&
        frame->TryFindAssetInternal(damageable->Spec.Id, true, spec)) {
        damageable->SetInvulnerable(frame, spec->RespawnInvulnerabilityDuration, INVULNERABILITY_FILTER::ALL);
    }
}

void DamageableSystem::GameMode_OnMainLoopStart(Frame* frame, EntityRef gameMode) {
    (void)gameMode;
    for (const auto& pair : frame->Unsafe.GetComponentBlockIterator<Damageable>().Range()) {
        DamageableSpec* spec = nullptr;
        if (frame->TryFindAssetInternal(pair.Component->Spec.Id, true, spec)) {
            pair.Component->SetInvulnerable(frame, spec->InitialInvulnerabilityDuration, INVULNERABILITY_FILTER::ALL);
        }
    }
}

void DamageableSystem::Shield_OnBreaks(Frame* frame, EntityRef entity, EntityRef sourceEntity, const FPVector3& direction) {
    (void)sourceEntity;
    (void)direction;
    Damageable* damageable = nullptr;
    DamageableSpec* spec = nullptr;
    if (frame->TryGetPointer<Damageable>(entity, damageable) &&
        frame->TryFindAssetInternal(damageable->Spec.Id, true, spec)) {
        damageable->SetInvulnerable(frame, spec->ShieldBreakInvulnerabilityDuration, INVULNERABILITY_FILTER::ALL);
    }
}

void DamageableSystem::OnAdded(Frame* frame, EntityRef entity, Damageable* damageable) {
    (void)entity;
    DamageableSpec* spec = nullptr;
    if (frame->TryFindAssetInternal(damageable->Spec.Id, true, spec)) {
        damageable->SetInvulnerable(frame, spec->SpawnInvulnerabilityDuration, INVULNERABILITY_FILTER::ALL);
    }
}

void DamageableSystem::Damageable_OnBeforeHit(Frame* f, EntityRef targetEntity, HitData* hitData) {
    // A projectile headshot drops the pending critical-hit bonus and rebuilds the hit from the
    // source's damage spec (its headshot multiplier then comes from CalculateDamages).
    Projectile* projectile = nullptr;
    if (f->TryGetPointer<Projectile>(hitData->source, projectile) && projectile->IsImpactHeadshot(f)) {
        DamageSource damageSource{};
        DamageSpec* damageSpec = nullptr;
        if (f->TryGet<DamageSource>(hitData->source, damageSource) &&
            f->TryFindAssetInternal(damageSource.Spec.Id, true, damageSpec)) {
            DamageLib::ClearDamageMultiplier(f, hitData->source, DAMAGE_MULTIPLIER_ORIGIN::CriticalHit);
            *hitData = damageSpec->CreateHitData(f, hitData->position, hitData->direction, hitData->source,
                                                 targetEntity);
        }
    }
    if (EntityInfoLib::EntityHasTypeMatching(f, targetEntity, ENTITY_INFO_FILTER::Boss)) {
        WeaponUser* attacker = nullptr;
        if (f->TryGetPointer<WeaponUser>(hitData->owner, attacker)) {
            hitData->damages = BonusFactor(attacker->DamageMultiplier_Boss) * hitData->damages;
        }
        return;
    }
    ShieldedActor* attackerShield = nullptr;
    if (f->TryGetPointer<ShieldedActor>(hitData->owner, attackerShield) && attackerShield->shieldIsBroken) {
        hitData->damages = BonusFactor(attackerShield->DamageMultiplier_Shieldless) * hitData->damages;
    }
    const std::int32_t targetBolts = BoltLib::GetCollectedBolts(f, targetEntity);
    if (targetBolts >= 1 && targetBolts > BoltLib::GetCollectedBolts(f, hitData->owner)) {
        WeaponUser* attacker = nullptr;
        if (f->TryGetPointer<WeaponUser>(hitData->owner, attacker)) {
            hitData->damages = BonusFactor(attacker->DamageMultiplier_MoreBolts) * hitData->damages;
        }
    }
}

// --- ShieldedActorLib ------------------------------------------------------------------------------

ShieldHitResult ShieldedActorLib::ApplyHit(Frame* frame, EntityRef entity, const HitData& hitData) {
    if (hitData.shield.ignoreShield) {
        return ShieldHitResult::get_NotDamaged();
    }
    ShieldedActor* shieldedActor = nullptr;
    ShieldedActorSpec* spec = nullptr;
    if (!frame->TryGetPointer<ShieldedActor>(entity, shieldedActor) ||
        !frame->TryFindAssetInternal(shieldedActor->spec.Id, true, spec)) {
        return ShieldHitResult::get_NotDamaged();
    }
    FP amount;
    if (hitData.shield.breakShield ||
        (shieldedActor->shieldIsBroken && shieldedActor->state == ShieldState::REGENERATING)) {
        amount = shieldedActor->shieldPoints;
    } else {
        amount = hitData.shield.damageMultiplier * hitData.damages;
    }
    shieldedActor->shieldPoints = shieldedActor->shieldPoints - amount;
    SetState(frame, entity, ShieldState::WAIT_FOR_REGEN);
    StartOrRestartRegenTimers(frame, entity, shieldedActor, spec);
    if (shieldedActor->shieldIsBroken) {
        return ShieldHitResult::get_NotDamaged();
    }
    if (shieldedActor->shieldPoints > FP{}) {
        frame->Events.ShieldHit(entity, hitData, amount, shieldedActor->shieldPoints);
        if (hitData.shield.applyMoveModifier) {
            ApplyMoveSpeedModifier(frame, entity, shieldedActor, spec);
        }
    } else {
        shieldedActor->shieldPoints = FP{};
        shieldedActor->shieldIsBroken = true;
        shieldedActor->brokenThisFrame = true;
        frame->Signals.Shield_OnBreaks(entity, hitData.source, hitData.direction);
        frame->Events.ShieldBreaks(entity);
    }
    return ShieldHitResult::Damaged(amount);
}

void ShieldedActorLib::SetState(Frame* frame, EntityRef entity, ShieldState state) {
    ShieldedActor* shieldedActor = nullptr;
    ShieldedActorSpec* spec = nullptr;
    if (!frame->TryGetPointer<ShieldedActor>(entity, shieldedActor) ||
        !frame->TryFindAssetInternal(shieldedActor->spec.Id, true, spec) || shieldedActor->state == state) {
        return;
    }
    shieldedActor->state = state;
    switch (state) {
    case ShieldState::WAIT_FOR_REGEN:
        StartOrRestartRegenTimers(frame, entity, shieldedActor, spec);
        break;
    case ShieldState::REGENERATING:
        frame->Signals.Shield_OnStartRegen(entity);
        break;
    case ShieldState::FULL:
        shieldedActor->shieldPoints = shieldedActor->stats.shieldMaxPoints;
        shieldedActor->shieldIsBroken = false;
        frame->Events.ShieldFull(entity);
        frame->Signals.Shield_OnFull(entity);
        break;
    }
}

void ShieldedActorLib::StartOrRestartRegenTimers(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                                 ShieldedActorSpec* spec) {
    (void)entity;
    shieldedActor->startRegenFrame = FrameAfter(frame, spec->timeWithoutDamageBeforeStartRegen);
}

void ShieldedActorLib::ApplyMoveSpeedModifier(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                              ShieldedActorSpec* spec) {
    KccExtension* kcc = nullptr;
    if (!frame->TryGetPointer<KccExtension>(entity, kcc)) {
        return;
    }
    if (shieldedActor->shieldPoints > FP{}) {
        // A hit slows at most once per tick interval.
        const FP sinceLastSlow =
            frame->get_DeltaTime() * (frame->Number - shieldedActor->lastFrameShieldHitSpeedModWasApplied);
        if (sinceLastSlow >= spec->moveSpeedModifierTickRate) {
            kcc->AddModifier_MoveSpeed(frame, entity, MOD_ORIGIN::SHIELD, spec->moveSpeedModifierWhenHit,
                                       spec->moveSpeedModifierWhenHitDuration, true, true);
            shieldedActor->lastFrameShieldHitSpeedModWasApplied = frame->Number;
        }
    } else {
        kcc->AddModifier_MoveSpeed(frame, entity, MOD_ORIGIN::SHIELD, spec->moveSpeedModifierWhenBreak,
                                   spec->moveSpeedModifierWhenBreakDuration, true, true);
    }
}

// --- ShieldedActorSystem ---------------------------------------------------------------------------

void ShieldedActorSystem::Killable_OnRevive(Frame* frame, EntityRef entity, const FPVector3& position,
                                            const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    ShieldedActor* shieldedActor = nullptr;
    ShieldedActorSpec* spec = nullptr;
    if (frame->TryGetPointer<ShieldedActor>(entity, shieldedActor) &&
        frame->TryFindAssetInternal(shieldedActor->spec.Id, true, spec)) {
        shieldedActor->state = ShieldState::FULL;
        shieldedActor->shieldPoints = shieldedActor->stats.shieldMaxPoints;
        shieldedActor->shieldIsBroken = false;
    }
}

void ShieldedActorSystem::InitActorIfNeeded(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                            ShieldedActorSpec* spec) {
    if (shieldedActor->stats.statsInitialised) {
        return;
    }
    FP shieldPoints;
    if (spec->Mode == ShieldSpecMode::HERO) {
        PlayerActor actor{};
        if (!frame->TryGet<PlayerActor>(entity, actor)) {
            throw std::runtime_error("ShieldedActorSystem: hero shield on an entity that is not a player");
        }
        HeroSpec* heroSpec = nullptr;
        BalancingLib::TryGetHeroSpec(frame, static_cast<GENERATED_CHARACTER_ID>(actor.characterIndex), heroSpec);
        if (heroSpec == nullptr) {
            throw std::runtime_error("ShieldedActorSystem: the player's hero has no spec");
        }
        if (!heroSpec->HasShield) {
            frame->Remove<ShieldedActor>(entity);
            return;
        }
        const std::int32_t level = std::max(0, std::min(actor.heroLevel, 10));
        shieldPoints = heroSpec->LeveledShield.at(static_cast<std::size_t>(level));
    } else {
        FP ratio;
        if (!BalancingLib::TryGetHumanWeaponsLevelRatio(frame, ratio)) {
            return;
        }
        shieldPoints = FPMath::Lerp(spec->shieldPointsMin, spec->shieldPointsMax, ratio);
    }
    shieldedActor->shieldPoints = shieldPoints;
    shieldedActor->stats.initialShieldPoints = shieldPoints;
    shieldedActor->stats.shieldMaxPoints = shieldPoints;
    shieldedActor->shieldIsBroken = false;
    shieldedActor->state = ShieldState::FULL;
    shieldedActor->stats.statsInitialised = true;
}

void ShieldedActorSystem::Update(Frame* frame, Filter& filter) {
    if (KillableLib::IsKillableDead(frame, filter.entity)) {
        return;
    }
    ShieldedActorSpec* spec = frame->FindAsset<ShieldedActorSpec>(filter.shieldedActor->spec.Id);
    InitActorIfNeeded(frame, filter.entity, filter.shieldedActor, spec);
    filter.shieldedActor->brokenThisFrame = false;
    filter.shieldedActor->shieldBypassedThisFrame = false;
    WaitForRegen(frame, filter.entity, filter.shieldedActor);
    Regenerate(frame, filter.entity, filter.shieldedActor, spec);
}

void ShieldedActorSystem::WaitForRegen(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor) {
    if (shieldedActor->state == ShieldState::WAIT_FOR_REGEN && shieldedActor->canRegen &&
        frame->Number >= shieldedActor->startRegenFrame) {
        ShieldedActorLib::SetState(frame, entity, ShieldState::REGENERATING);
    }
}

void ShieldedActorSystem::Regenerate(Frame* frame, EntityRef entity, ShieldedActor* shieldedActor,
                                     ShieldedActorSpec* spec) {
    if (shieldedActor->state != ShieldState::REGENERATING) {
        return;
    }
    const FP maxPoints = shieldedActor->stats.shieldMaxPoints;
    const FP perSecond = maxPoints / spec->timeToRegenerate;
    shieldedActor->shieldPoints = shieldedActor->shieldPoints + frame->get_DeltaTime() * perSecond;
    if (shieldedActor->shieldPoints >= maxPoints) {
        shieldedActor->shieldPoints = maxPoints;
        ShieldedActorLib::SetState(frame, entity, ShieldState::FULL);
    }
}

void ShieldedActorSystem::OnDisabled(Frame* f) {
    auto shields = f->Filter<ShieldedActor>();
    EntityRef entity;
    ShieldedActor* shieldedActor = nullptr;
    while (shields.NextUnsafe(entity, shieldedActor)) {
        f->Remove<ShieldedActor>(entity);
    }
}

void ShieldedActorSystem::OnAdded(Frame* f, EntityRef entity, ShieldedActor* component) {
    (void)f;
    (void)entity;
    component->canRegen = true;
}

} // namespace Quantum
