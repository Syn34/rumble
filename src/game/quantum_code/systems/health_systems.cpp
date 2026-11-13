// health_systems.cpp - HealthLib, HealthSystem and the balancing/weapon lookups health
// initialisation depends on.
#include "quantum_code/systems/health_systems.h"

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/log.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"

#include <algorithm>
#include <stdexcept>

namespace Quantum {

namespace FPMath = Photon::Deterministic::FPMath;

// --- BalancingLib / WeaponLib / CallbackActionLib -----------------------------------------

bool BalancingLib::TryGetHeroSpec(Frame* f, GENERATED_CHARACTER_ID heroID, HeroSpec*& heroSpec) {
    heroSpec = nullptr;
    SimulationHeroesTable* table = nullptr;
    if (!f->TryFindAssetInternal(f->get_RuntimeConfig()->simulationHeroesTable.Id, true, table)) {
        Photon::Deterministic::Log::Error("BalancingLib: the session has no heroes table");
        return false;
    }
    for (const SimulationHeroesTable::Entry& entry : table->entries) {
        if (entry.uniqueID != heroID) {
            continue;
        }
        if (!f->TryFindAssetInternal(entry.spec.Id, true, heroSpec)) {
            // Uncertain: the original logs the hero id and spec GUID here; what it returns
            // afterwards could not be confirmed, success is assumed.
            Photon::Deterministic::Log::Error("BalancingLib: hero {} names a missing spec",
                                              static_cast<int>(heroID));
        }
        return true;
    }
    return true;
}

bool BalancingLib::TryGetHumanWeaponsLevelRatio(Frame* f, FP& ratio) {
    ratio = FP{};
    GameModeHandler* handler = nullptr;
    if (!f->TryGetPointerSingleton<GameModeHandler>(handler)) {
        return false;
    }
    ratio = handler->averageHumanWeaponLevel / 9;
    return true;
}

std::int32_t WeaponLib::GetWeaponLevel(Frame* frame, EntityRef owner, EntityRef weapon) {
    if (!frame->Exists(owner) || !frame->Exists(weapon)) {
        return 0;
    }
    WeaponLoadoutActor* loadoutActor = nullptr;
    if (!frame->TryGetPointer<WeaponLoadoutActor>(owner, loadoutActor)) {
        return 0;
    }
    for (const WeaponLoadoutItem& item : loadoutActor->loadout.get_slots()) {
        if (item.spawnedWeapon == weapon) {
            return item.data.weaponLevel;
        }
    }
    return 0;
}

void CallbackActionLib::Execute(AssetRefCallbackAction callbackActionAssetRef, Frame* frame, EntityRef entity) {
    CallbackAction* action = nullptr;
    if (frame->TryFindAssetInternal(callbackActionAssetRef.Id, true, action)) {
        action->Execute(frame, entity);
    }
}

// --- Health / Overhealth (component helpers) ------------------------------------------------

FP Overhealth::get_DecreaseInterval() const { return FP::One() / DecreaseTickRate; }

FP Overhealth::get_DecreaseByTick() const {
    return Value / (FPMath::CeilToInt(DecreaseDuration * DecreaseTickRate) + 1);
}

void Health::DecreaseOverhealth(FP amount) {
    MaxOverHP -= amount;
    const FP cap = Stats.MaxHP + MaxOverHP;
    if (CurrentHP + CurrentOverHP > cap) {
        CurrentOverHP = cap - CurrentHP;
    }
}

// --- HealthLib ---------------------------------------------------------------------------------

void HealthLib::InitialiseStatsIfNeeded(Frame* f, EntityRef healthEntity, Health* health, HealthSpec*& healthSpec) {
    healthSpec = f->FindAsset<HealthSpec>(health->Spec.Id);
    if (health->Stats.Initialised) {
        return;
    }
    FP hp = healthSpec->MaxHP;
    bool averageLevelMode = false;
    switch (healthSpec->Mode) {
    case HealthSpecMode::HERO:
        InitHeroHealth(f, healthEntity, health, healthSpec, hp);
        break;
    case HealthSpecMode::USE_OWNER_WEAPON_LEVEL: {
        Ownership* ownership = nullptr;
        WeaponUser* weaponUser = nullptr;
        if (!f->TryGetPointer<Ownership>(healthEntity, ownership) ||
            !f->TryGetPointer<WeaponUser>(ownership->Owner, weaponUser)) {
            averageLevelMode = true;
            break;
        }
        const std::int32_t level = WeaponLib::GetWeaponLevel(f, ownership->Owner, weaponUser->EquippedWeapon);
        if (level >= 0 && level < static_cast<std::int32_t>(healthSpec->LeveledMaxHP.size())) {
            hp = healthSpec->LeveledMaxHP[static_cast<std::size_t>(level)];
        }
        break;
    }
    case HealthSpecMode::USE_PVE_SCALING:
        hp = f->get_RuntimeConfig()->PveScaling * healthSpec->BaseHP;
        break;
    case HealthSpecMode::FIXED:
        hp = healthSpec->FixedHP;
        break;
    case HealthSpecMode::USE_SELF_LEVEL: {
        LevelHolder levelHolder{};
        if (healthSpec->AmountPerSelfLevel.empty() || !f->TryGet<LevelHolder>(healthEntity, levelHolder)) {
            hp = healthSpec->BaseHP;
            break;
        }
        // Level 1 is the first entry (level 0 wraps to the 65535th, as in the original).
        const int level = std::min(static_cast<int>(healthSpec->AmountPerSelfLevel.size()),
                                   static_cast<int>(levelHolder.Level));
        hp = healthSpec->AmountPerSelfLevel.at(static_cast<std::uint16_t>(level - 1));
        break;
    }
    default:
        averageLevelMode = true;
        break;
    }
    if (averageLevelMode) {
        hp = healthSpec->MaxHP;
        if (healthSpec->MinHP < healthSpec->MaxHP) {
            FP ratio;
            if (!BalancingLib::TryGetHumanWeaponsLevelRatio(f, ratio)) {
                return;
            }
            hp = FPMath::Lerp(healthSpec->MinHP, healthSpec->MaxHP, ratio);
        }
    }
    health->Stats.InitialMaxHP = hp;
    health->Stats.MaxHP = hp;
    health->CurrentHP = hp;
    health->StartRegenHP = hp;
    UpdateMaxHp(f, health);
    health->Stats.Initialised = true;
}

void HealthLib::InitHeroHealth(Frame* f, EntityRef healthEntity, Health* health, HealthSpec*& healthSpec, FP& hp) {
    PlayerActor actor{};
    if (!f->TryGet<PlayerActor>(healthEntity, actor)) {
        throw std::runtime_error("HealthLib: hero health on an entity that is not a player");
    }
    HeroSpec* heroSpec = nullptr;
    BalancingLib::TryGetHeroSpec(f, static_cast<GENERATED_CHARACTER_ID>(actor.characterIndex), heroSpec);
    AssetRefHealthSpec specRef = heroSpec->HealthSpec;
    ReplaceHealthSpecHeroImprovementSpec* replacement = nullptr;
    if (TryGetReplaceHealthHeroImprovementSpec(f, heroSpec, replacement)) {
        specRef = replacement->HealthSpec;
    }
    healthSpec = f->FindAsset<HealthSpec>(specRef.Id);
    health->Spec = specRef;
    const int level = std::max(std::min(actor.heroLevel, 10), 0);
    hp = heroSpec->LeveledHP.at(static_cast<std::size_t>(level));
}

bool HealthLib::TryGetReplaceHealthHeroImprovementSpec(Frame* f, HeroSpec* heroSpec,
                                                       ReplaceHealthSpecHeroImprovementSpec*& replaceHealthSpec) {
    for (const AssetRefImprovementSpec& improvementRef : heroSpec->HeroImprovementSpecs) {
        ImprovementSpec* improvement = nullptr;
        if (!f->TryFindAssetInternal(improvementRef.Id, true, improvement)) {
            continue;
        }
        if (auto* replacement = dynamic_cast<ReplaceHealthSpecHeroImprovementSpec*>(improvement)) {
            replaceHealthSpec = replacement;
            return true;
        }
    }
    replaceHealthSpec = nullptr;
    return false;
}

FP HealthLib::GetHPRatio(Frame* frame, EntityRef entity) {
    Health* health = nullptr;
    if (!frame->TryGetPointer<Health>(entity, health)) {
        return FP{};
    }
    return health->CurrentHP / health->Stats.MaxHP;
}

void HealthLib::SetMaxHpMultiplier(Frame* frame, EntityRef entity, HEALTH_MULTIPLIER_SOURCE source, FP multiplier,
                                   bool overrideExisting) {
    Health* health = nullptr;
    if (!frame->TryGetPointer<Health>(entity, health)) {
        return;
    }
    FP& slot = health->get_MaxHpMultArray()[static_cast<std::size_t>(source)];
    if (overrideExisting || slot == FP::One()) {
        slot = multiplier;
    } else {
        slot = slot * multiplier;
    }
    UpdateMaxHp(frame, health);
}

FP HealthLib::GetMaxHpMultiplier(Frame* frame, EntityRef entity, HEALTH_MULTIPLIER_SOURCE source) {
    Health* health = nullptr;
    if (!frame->TryGetPointer<Health>(entity, health)) {
        return FP::One();
    }
    return health->get_MaxHpMultArray()[static_cast<std::size_t>(source)];
}

void HealthLib::UpdateMaxHp(Frame* frame, Health* health) {
    (void)frame;
    FP maxHP = health->Stats.InitialMaxHP;
    for (FP multiplier : health->get_MaxHpMultArray()) {
        maxHP = multiplier * maxHP;
    }
    if (health->CurrentHP == health->Stats.MaxHP) {
        health->CurrentHP = maxHP;
    }
    health->Stats.MaxHP = maxHP;
}

void HealthLib::FullRegen(Frame* frame, EntityRef entity) {
    Health* health = nullptr;
    if (frame->TryGetPointer<Health>(entity, health)) {
        health->CurrentHP = health->Stats.MaxHP;
    }
}

// --- HealthSystem ------------------------------------------------------------------------------

void HealthSystem::Update(Frame* f, Filter& filter) {
    GameModeHandler* handler = nullptr;
    if (f->TryGetPointerSingleton<GameModeHandler>(handler) && handler->state != GAME_MODE_HANDLER_STATE::MainLoop) {
        return;
    }
    PlayerActor* actor = nullptr;
    if (f->TryGetPointer<PlayerActor>(filter.entity, actor) && actor->gameModeFinished) {
        return;
    }
    HealthSpec* spec = nullptr;
    HealthLib::InitialiseStatsIfNeeded(f, filter.entity, filter.health, spec);
    Regenerate(f, filter.entity, filter.health, spec);
    UpdateActiveOverhealths(f, filter.health);
}

void HealthSystem::UpdateActiveOverhealths(Frame* f, Health* health) {
    Collections::QList<Overhealth> overhealths = f->ResolveList(health->get_ActiveOverhealths());
    // Overhealth the entity no longer has room for (taken by damage) is dropped, newest
    // first.
    FP unused = health->MaxOverHP - health->CurrentOverHP;
    for (int i = overhealths.get_Count() - 1; i >= 0; --i) {
        const Overhealth overhealth = overhealths.get_Item(i);
        if (unused >= overhealth.Value) {
            health->MaxOverHP -= overhealth.Value;
            overhealths.RemoveAt(i);
            unused -= overhealth.Value;
        }
        if (overhealth.Duration < FP{}) {
            continue; // permanent
        }
        const FP elapsed = f->get_DeltaTime() * (f->Number - overhealth.StartFrame);
        const FP decaying = elapsed - overhealth.Duration;
        if (decaying < FP{}) {
            continue;
        }
        if (elapsed < overhealth.Duration + overhealth.DecreaseDuration) {
            // One tick per decrease interval: when the interval wraps within this frame.
            if (decaying % overhealth.get_DecreaseInterval() < f->get_DeltaTime()) {
                health->DecreaseOverhealth(overhealth.get_DecreaseByTick());
            }
        } else {
            health->DecreaseOverhealth(overhealth.get_DecreaseByTick());
            overhealths.Remove(overhealth);
        }
    }
}

void HealthSystem::Regenerate(Frame* frame, EntityRef healthEntity, Health* health, HealthSpec* spec) {
    if (!spec->CanRegenerate) {
        return;
    }
    const FP maxHP = health->Stats.MaxHP;
    if (health->CurrentHP >= maxHP || KillableLib::IsKillableDead(frame, healthEntity)) {
        return;
    }
    if (spec->OnlyRegenerateIfShieldIsUp) {
        ShieldedActor* shielded = nullptr;
        if (frame->TryGetPointer<ShieldedActor>(healthEntity, shielded) && shielded->shieldIsBroken) {
            return;
        }
    }
    const FP sinceDamage = frame->get_DeltaTime() * (frame->Number - health->regenFrameReference);
    const FP regenerating = sinceDamage - spec->TimeWithoutDamageBeforeStartRegen;
    if (regenerating < FP{}) {
        return;
    }
    const FP missing = maxHP - health->StartRegenHP;
    const FP duration = FPMath::Max(spec->TimeToRegenerate * FPMath::Clamp01(missing / maxHP), FP::FromRaw(655));
    health->CurrentHP =
        FPMath::Clamp(health->StartRegenHP + regenerating / duration * missing, health->CurrentHP, maxHP);
}

void HealthSystem::Shield_OnFull(Frame* frame, EntityRef entity) {
    Health* health = nullptr;
    HealthSpec* spec = nullptr;
    if (frame->TryGetPointer<Health>(entity, health) && frame->TryFindAssetInternal(health->Spec.Id, true, spec) &&
        spec->OnlyRegenerateIfShieldIsUp) {
        health->regenFrameReference = frame->Number;
    }
}

void HealthSystem::Killable_OnRevive(Frame* frame, EntityRef toRevive, const FPVector3& position,
                                     const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    Health* health = nullptr;
    if (frame->TryGetPointer<Health>(toRevive, health)) {
        health->Stats.Initialised = false;
    }
}

void HealthSystem::Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type,
                                      std::uint16_t amount) {
    if (type != DroppableType::RegenHP) {
        return;
    }
    Health* health = nullptr;
    if (!frame->TryGetPointer<Health>(collectorEntity, health) || health->CurrentHP >= health->Stats.MaxHP) {
        return;
    }
    HealthSpec* spec = nullptr;
    if (!frame->TryFindAssetInternal(health->Spec.Id, true, spec) || spec->HealthRegenPerPickup.RawValue() < 1) {
        return;
    }
    health->CurrentHP = FPMath::Clamp(health->CurrentHP + spec->HealthRegenPerPickup * static_cast<std::int32_t>(amount),
                                      FP{}, health->Stats.MaxHP);
}

void HealthSystem::OnAdded(Frame* f, EntityRef entity, Health* component) {
    (void)f;
    (void)entity;
    for (FP& multiplier : component->get_MaxHpMultArray()) {
        multiplier = FP::One();
    }
}

void HealthSystem::BlastBallGame_StateChanged(Frame* f, BlastBallGameState newState) {
    if (newState != BlastBallGameState::Restarting) {
        return;
    }
    for (const auto& pair : f->Unsafe.GetComponentBlockIterator<PlayerActor>().Range()) {
        Health* health = nullptr;
        if (f->TryGetPointer<Health>(pair.Entity, health)) {
            health->Stats.Initialised = false;
        }
    }
}

} // namespace Quantum
