// damage_specs.cpp - how much damage a hit does: DamageSpec (hit creation, raw damage per
// mode, multipliers and talent/enhancement modifiers) and the small libraries it uses.
#include "photon_deterministic/fp_math.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"
#include "quantum_core/transform.h"

#include <algorithm>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

Ownership Ownership::GetOwnershipOrDefault(Frame* frame, EntityRef entity) {
    Ownership ownership{};
    if (frame->TryGet<Ownership>(entity, ownership)) {
        return ownership;
    }
    ownership = Ownership{};
    ownership.Owner = entity;
    ownership.Source = entity;
    return ownership;
}

bool BalancingLib::TryGetHumanHeroesLevelRatio(Frame* f, FP& ratio) {
    ratio = FP{};
    GameModeHandler* handler = nullptr;
    if (!f->TryGetPointerSingleton<GameModeHandler>(handler) || !handler->get_AverageHumanHeroLevelRatioAvailable()) {
        return false;
    }
    ratio = handler->get_AverageHumanHeroLevelRatio();
    return true;
}

bool WeaponLib::TryGetWeaponLevel(Frame* frame, EntityRef weapon, std::int32_t& level) {
    Ownership* ownership = nullptr;
    const bool owned = Ownership::TryGetOwnership(frame, weapon, ownership);
    level = owned ? GetWeaponLevel(frame, ownership->Owner, weapon) : 0;
    return owned;
}

bool Unsafe::TryGetWeaponUser(Frame* frame, EntityRef entity, WeaponUser*& weaponUser) {
    weaponUser = nullptr;
    return frame->TryGetPointer<WeaponUser>(entity, weaponUser) && !weaponUser->Disabled;
}

void ProjectileLib::ComputeProjectileDamageMultiplier(Frame* frame, EntityRef projectileEntity, EntityRef targetEntity,
                                                      FP& damageMultiplier) {
    Projectile* projectile = nullptr;
    if (!frame->TryGetPointer<Projectile>(projectileEntity, projectile) || !projectile->IsImpactHeadshot(frame)) {
        return;
    }
    PreciseHitReceiver receiver{};
    PreciseHitReceiverSpec* spec = nullptr;
    if (frame->TryGet<PreciseHitReceiver>(targetEntity, receiver) &&
        frame->TryFindAssetInternal<PreciseHitReceiverSpec>(receiver.Spec.Id, true, spec)) {
        damageMultiplier = spec->HeadshotBonusMultiplier + damageMultiplier;
    }
}

HitData DamageSpec::CreateHitData(Frame* frame, FPVector3 position, FPVector3 direction, EntityRef sourceEntity,
                                  EntityRef targetEntity) const {
    const Ownership ownership = Ownership::GetOwnershipOrDefault(frame, sourceEntity);
    FP criticalMultiplier;
    HitData hit{};
    hit.type = DamageType;
    hit.frame = frame->Number;
    hit.isContinuous = false;
    // A pending critical-hit multiplier on the source marks the hit as critical.
    hit.isCriticalHit = DamageLib::TryGetDamageMultiplier(frame, sourceEntity, DAMAGE_MULTIPLIER_ORIGIN::CriticalHit,
                                                          criticalMultiplier);
    hit.slowDownOnHit = SlowDownPlayerActor;
    hit.useReticleFeedback = UseReticleFeedback;
    hit.owner = ownership.Owner;
    hit.source = sourceEntity;
    hit.weapon = ownership.Weapon;
    hit.baseDamage = BaseUnscaledAmount;
    hit.damages = FP{};
    hit.health.damageMultiplier = DamageHealth ? HealthDamageMultiplier : FP{};
    hit.direction = direction;
    hit.position = position;
    hit.shield.applyMoveModifier = ApplyMoveSpeedModifier;
    hit.shield.breakShield = BreakShield;
    hit.shield.ignoreShield = !DamageShield;
    hit.shield.damageMultiplier = DamageShield ? ShieldDamageMultiplier : FP{};

    const FP damage = CalculateDamages(frame, hit, targetEntity);
    std::vector<DamageModifier> modifiers;
    GetDamageModifier(frame, ownership, modifiers);
    FP modifierTotal = FP::One();
    for (const DamageModifier& modifier : modifiers) {
        modifierTotal = modifier.DamageModifierAmount + modifierTotal;
    }
    hit.damages = modifierTotal * damage;
    return hit;
}

FP DamageSpec::CalculateDamages(Frame* frame, const HitData& hitData, EntityRef targetEntity) const {
    const FP rawDamage = CalculateRawDamages(frame, hitData);
    FP multiplier = DamageLib::GetTotalDamageMultiplier(frame, hitData.source);

    WeaponUser* weaponUser = nullptr;
    if (Unsafe::TryGetWeaponUser(frame, hitData.owner, weaponUser)) {
        multiplier = weaponUser->DamageMultiplier;
        Weapon* weapon = nullptr;
        WeaponSpec* weaponSpec = nullptr;
        if (frame->TryGetPointer<Weapon>(hitData.weapon, weapon) &&
            frame->TryFindAssetInternal<WeaponSpec>(weapon->WeaponData.Id, true, weaponSpec) &&
            weaponSpec->LoadoutSlotType == WeaponLoadoutSlotType::MELEE) {
            multiplier = weaponUser->DamageMultiplier_Melee * multiplier;
        }
        if (DamageType == ::Quantum::DamageType::Electric) {
            multiplier = weaponUser->DamageMultiplier_Electric * multiplier;
        }
        multiplier = DamageLib::GetTotalDamageMultiplier(frame, hitData.source) * multiplier;
    }

    // Extra-damage power-ups add their percentage.
    PowerUpUser* powerUps = nullptr;
    if (frame->TryGetPointer<PowerUpUser>(hitData.owner, powerUps)) {
        Collections::QList<ActivePowerUp> active = frame->ResolveList(powerUps->get_ActivePowerUps());
        for (std::int32_t i = 0; i < active.get_Count(); ++i) {
            const ActivePowerUp& powerUp = active.get_Item(i);
            if (powerUp.PowerUpType != PowerUpType::ExtraDamage) {
                continue;
            }
            PowerUpSpec* spec = nullptr;
            if (!frame->TryFindAssetInternal<PowerUpSpec>(powerUp.Spec.Id, true, spec)) {
                continue;
            }
            if (const auto* extraDamage = dynamic_cast<const ExtraDamagePowerUpSpec*>(spec)) {
                multiplier = extraDamage->DamageBoostPercent / 100 + multiplier;
            }
        }
    }

    ProjectileLib::ComputeProjectileDamageMultiplier(frame, hitData.source, targetEntity, multiplier);

    // Ammunition that weakens with range (relative to the weapon's range).
    Transform3D ownerTransform{};
    DamageSource damageSource{};
    AmmunitionDamageSpec* ammunition = nullptr;
    Weapon weapon{};
    WeaponSpec* weaponSpec = nullptr;
    if (frame->TryGet<Transform3D>(hitData.owner, ownerTransform) &&
        frame->TryGet<DamageSource>(hitData.source, damageSource) &&
        frame->TryFindAssetInternal<AmmunitionDamageSpec>(damageSource.AmmunitionSpec.Id, true, ammunition) &&
        ammunition->UseRangeToDamageMultCurve && frame->TryGet<Weapon>(hitData.weapon, weapon) &&
        frame->TryFindAssetInternal<WeaponSpec>(weapon.WeaponData.Id, true, weaponSpec)) {
        const FP distance = FPVector3::Distance(ownerTransform.Position, hitData.position);
        const FP rangeRatio = FPMath::Min(distance / weaponSpec->Range, FP::One());
        multiplier = multiplier * ammunition->RangeToDamageMultCurve.Evaluate(rangeRatio);
    }
    return multiplier * rawDamage;
}

FP DamageSpec::CalculateRawDamages(Frame* frame, const HitData& hitData) const {
    // Level tables: out-of-range levels fall back to the first entry (the original's
    // indexing throws on an empty table).
    const auto atLevel = [this](std::int32_t level) {
        if (level < 0 || static_cast<std::size_t>(level) >= AmountPerLevel.size()) {
            return AmountPerLevel.at(0);
        }
        return AmountPerLevel[static_cast<std::size_t>(level)];
    };
    switch (Mode) {
    case DamageAmountMode::FIXED:
        return Amount;
    case DamageAmountMode::USE_AVERAGE_HUMAN_HEROES_LEVEL: {
        if (MinAmount >= MaxAmount) {
            return MinAmount;
        }
        FP ratio;
        if (!BalancingLib::TryGetHumanHeroesLevelRatio(frame, ratio)) {
            return MinAmount;
        }
        return FPMath::Lerp(MinAmount, MaxAmount, ratio);
    }
    case DamageAmountMode::USE_WEAPON_LEVEL: {
        std::int32_t level = 0;
        if (!WeaponLib::TryGetWeaponLevel(frame, hitData.weapon, level)) {
            return atLevel(0);
        }
        if (level >= 0 && static_cast<std::size_t>(level) >= AmountPerLevel.size()) {
            return AmountPerLevel.at(AmountPerLevel.size() - 1); // the last level
        }
        return atLevel(level);
    }
    case DamageAmountMode::USE_HERO_LEVEL: {
        PlayerActor* actor = nullptr;
        if (!frame->TryGetPointer<PlayerActor>(hitData.owner, actor)) {
            return atLevel(0);
        }
        return atLevel(std::clamp(actor->heroLevel, 0, 10));
    }
    case DamageAmountMode::USE_PVE_SCALING:
        return frame->get_RuntimeConfig()->PveScaling * BaseAmount;
    case DamageAmountMode::USE_SELF_LEVEL: {
        if (AmountPerSelfLevel.empty()) {
            return BaseUnscaledAmount;
        }
        LevelHolder holder{};
        if (!frame->TryGet<LevelHolder>(hitData.owner, holder)) {
            return BaseUnscaledAmount;
        }
        const auto count = static_cast<std::int32_t>(AmountPerSelfLevel.size());
        const auto index = static_cast<std::uint16_t>(std::min(count, static_cast<std::int32_t>(holder.Level)) - 1);
        return AmountPerSelfLevel.at(index);
    }
    }
    return FP{};
}

void DamageSpec::GetDamageModifier(Frame* frame, const Ownership& sourceOwnership,
                                   std::vector<DamageModifier>& resultDamageModifier) const {
    resultDamageModifier.clear();
    PlayerActor* actor = nullptr;
    if (!frame->TryGetPointer<PlayerActor>(sourceOwnership.Owner, actor)) {
        return;
    }
    const std::int32_t unlockedTalents = actor->heroTalentLevel;
    HeroSpec* heroSpec = nullptr;
    if (!BalancingLib::TryGetHeroSpec(frame, static_cast<GENERATED_CHARACTER_ID>(actor->characterIndex), heroSpec) ||
        heroSpec == nullptr) {
        return;
    }
    const auto& talents = heroSpec->TalentsInUnlockOrder;
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(talents.size()) && i < unlockedTalents; ++i) {
        const AssetRef talent = talents[static_cast<std::size_t>(i)];
        if (!talent.Id.IsValid()) {
            continue;
        }
        ImprovementSpec* improvement = nullptr;
        WeaponEnhancementSpec* enhancement = nullptr;
        if (frame->TryFindAssetInternal<ImprovementSpec>(talent.Id, true, improvement)) {
            GetDamageModifiersFromHeroImprovements(frame, improvement, resultDamageModifier);
        } else if (frame->TryFindAssetInternal<WeaponEnhancementSpec>(talent.Id, true, enhancement)) {
            GetDamageModifiersFromWeaponEnhancement(frame, enhancement, sourceOwnership.Weapon, resultDamageModifier);
        }
    }
}

void DamageSpec::GetDamageModifiersFromWeaponEnhancement(Frame* frame, const WeaponEnhancementSpec* enhancementSpec,
                                                         EntityRef sourceWeapon,
                                                         std::vector<DamageModifier>& damageModifierList) const {
    SimulationWeaponTable* table = nullptr;
    Weapon weapon{};
    if (!frame->TryFindAssetInternal<SimulationWeaponTable>(frame->get_RuntimeConfig()->simulationWeaponTable.Id, true,
                                                            table) ||
        !frame->TryGet<Weapon>(sourceWeapon, weapon)) {
        return;
    }
    GENERATED_WEAPON_ID weaponId{};
    if (!table->TryGetWeaponId(weapon, weaponId) || enhancementSpec->Weapon_ID != weaponId) {
        return;
    }
    for (const DamageModifier& modifier : enhancementSpec->DamageModifiers) {
        if (modifier.DamageSpecRef.Id == Guid) {
            damageModifierList.push_back(modifier);
        }
    }
}

void DamageSpec::GetDamageModifiersFromHeroImprovements(Frame* frame, const ImprovementSpec* improvementSpec,
                                                        std::vector<DamageModifier>& damageModifierList) const {
    (void)frame;
    for (const DamageModifier& modifier : improvementSpec->DamageModifiers) {
        if (modifier.DamageSpecRef.Id == Guid) {
            damageModifierList.push_back(modifier);
        }
    }
}

FPVector3 PusherSpec::CalculateImpulse(Frame* f, FPVector3 forceDirection, FPVector3 hitPoint,
                                       EntityRef pusherEntity) const {
    Pusher* pusher = nullptr;
    Transform3D* transform = nullptr;
    if (!f->TryGetPointer<Pusher>(pusherEntity, pusher) || !f->TryGetPointer<Transform3D>(pusherEntity, transform)) {
        return FPVector3::Zero();
    }
    const FPVector3 pusherPosition = transform->Position;
    const FPVector3 pusherForward = transform->Forward();

    // In 2D the push is flat (no height difference counts towards the distance either) and
    // falls back to the pusher's facing when there is no horizontal direction.
    FP heightDifference;
    if (Is2D) {
        forceDirection.Y = FP{};
        forceDirection = FPVector3::Normalize(forceDirection);
        if (forceDirection == FPVector3::Zero()) {
            forceDirection = pusherForward;
        }
    } else {
        heightDifference = hitPoint.Y - pusherPosition.Y;
    }

    FP force = PushForce;
    FP upwardForce = PushUpwardForce;
    if (UseCurve) {
        const FP dx = hitPoint.X - pusherPosition.X;
        const FP dz = hitPoint.Z - pusherPosition.Z;
        const FP distance =
            FP::FromRaw(FPMath::SqrtRaw((dz * dz).RawValue() + (dx * dx).RawValue() + (heightDifference * heightDifference).RawValue()));
        const FP curveFactor =
            pusher->CurrentPusherRange != FP{} ? ForceCurve.Evaluate(distance / pusher->CurrentPusherRange) : FP::One();
        force = curveFactor * force;
        upwardForce = curveFactor * upwardForce;
    }

    FPVector3 impulse = forceDirection * force;
    if (Is2D) {
        impulse.Y = upwardForce;
    }
    if (IsInverse) {
        impulse = impulse * FP::FromInt(-1);
    }
    return impulse;
}

void PushableLib::AddPush(Frame* frame, Pushable* pushable, const PushData& pushData) {
    if (pushData.impulse == FPVector3::Zero()) {
        return;
    }
    PushData signalled = pushData;
    frame->Signals.Pushable_OnAddPush(pushable, &signalled);
    frame->ResolveList(pushable->get_PendingPushRequests()).Add(pushData);
}

bool DamageLib::Hit(Frame* frame, EntityRef sourceEntity, EntityRef targetEntity, FPVector3 position,
                    FPVector3 direction, const DamageSpec* damageSpec) {
    if (damageSpec == nullptr) {
        DamageSource* damageSource = nullptr;
        DamageSpec* sourceSpec = nullptr;
        if (!frame->TryGetPointer<DamageSource>(sourceEntity, damageSource) ||
            !frame->TryFindAssetInternal<DamageSpec>(damageSource->Spec.Id, true, sourceSpec)) {
            return false;
        }
        damageSpec = sourceSpec;
    }
    Damageable* damageable = nullptr;
    if (!frame->TryGetPointer<Damageable>(targetEntity, damageable) ||
        damageable->IsInvulnerableToDamage(frame, targetEntity) ||
        !EntityInfoLib::EntityHasNoTypeOrMatches(frame, targetEntity, damageSpec->EntityFilter)) {
        return false;
    }
    DamageableSpec* damageableSpec = nullptr;
    if (frame->TryFindAssetInternal<DamageableSpec>(damageable->Spec.Id, true, damageableSpec) &&
        !Contains(damageableSpec->DamageFilter, damageSpec->DamageType)) {
        return false;
    }

    HitData hit = damageSpec->CreateHitData(frame, position, direction, sourceEntity, targetEntity);
    AreaOfEffect* areaOfEffect = nullptr;
    AreaOfEffectSpec* areaSpec = nullptr;
    if (frame->TryGetPointer<AreaOfEffect>(sourceEntity, areaOfEffect) &&
        frame->TryFindAssetInternal<AreaOfEffectSpec>(areaOfEffect->Spec.Id, true, areaSpec) && areaSpec->IsContinuous) {
        hit.SetIsContinuous(true);
    }
    frame->Signals.Damageable_OnBeforeHit(targetEntity, &hit);
    damageable->Hit(frame, targetEntity, hit);
    return true;
}

bool DamageLib::Push(Frame* frame, EntityRef sourceEntity, EntityRef targetEntity, FPVector3 position,
                     FPVector3 direction, const PusherSpec* pusherSpec) {
    if (pusherSpec == nullptr) {
        Pusher* pusher = nullptr;
        PusherSpec* sourceSpec = nullptr;
        if (!frame->TryGetPointer<Pusher>(sourceEntity, pusher) ||
            !frame->TryFindAssetInternal<PusherSpec>(pusher->Spec.Id, true, sourceSpec)) {
            return false;
        }
        pusherSpec = sourceSpec;
    }
    Pushable* pushable = nullptr;
    PushableSpec* pushableSpec = nullptr;
    if (!frame->TryGetPointer<Pushable>(targetEntity, pushable) ||
        !frame->TryFindAssetInternal<PushableSpec>(pushable->Spec.Id, true, pushableSpec) ||
        IsInvulnerableToPush(frame, targetEntity)) {
        return false;
    }
    GrinderActor* grinder = nullptr;
    if (frame->TryGetPointer<GrinderActor>(targetEntity, grinder) && grinder->get_CanGrind() &&
        grinder->get_IsGrinding()) {
        return false; // riders on a rail cannot be pushed
    }
    if ((static_cast<std::int32_t>(pushableSpec->PushFilters) & static_cast<std::int32_t>(pusherSpec->PushFilters)) == 0) {
        return false;
    }

    const FPVector3 impulse = pusherSpec->CalculateImpulse(frame, direction, position, sourceEntity);
    const Ownership ownership = Ownership::GetOwnershipOrDefault(frame, sourceEntity);
    PushData push{};
    pusherSpec->SetupPushData(&push);
    push.hitpoint = position;
    push.impulse = impulse;
    push.pushed = targetEntity;
    push.pusher = sourceEntity;
    push.pusherOwner = ownership.Owner;
    frame->Signals.Pushable_OnBeforePush(targetEntity, &push);
    PushableLib::AddPush(frame, pushable, push);
    return true;
}

bool DamageLib::HitAndPush(Frame* frame, EntityRef sourceEntity, EntityRef targetEntity, FPVector3 position,
                           FPVector3 direction, const DamageSpec* damageSpec, const PusherSpec* pusherSpec) {
    if (IsInvulnerable(frame, targetEntity)) {
        return false;
    }
    direction = FPVector3::Normalize(direction);
    // Both always run (no short-circuit).
    const bool hit = Hit(frame, sourceEntity, targetEntity, position, direction, damageSpec);
    const bool pushed = Push(frame, sourceEntity, targetEntity, position, direction, pusherSpec);
    return hit || pushed;
}

} // namespace Quantum
