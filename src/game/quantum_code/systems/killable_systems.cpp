// killable_systems.cpp
#include "quantum_code/systems/killable_systems.h"

#include "photon_deterministic/log.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/combat_libs.h"
#include "quantum_code/systems/libs.h"

namespace Quantum {

// --- KillableSystem -----------------------------------------------------------------------

void KillableSystem::Update(Frame* f, Filter& filter) {
    const ::Quantum::KillNextFrame* mark = filter.killNextFrame;
    if (f->get_DeltaTime() * (f->Number - mark->MarkedForKillFrame) < mark->KillDelay) {
        return;
    }
    const KillType killType = mark->killType;
    const EntityRef killer = mark->killer;
    f->Remove<::Quantum::KillNextFrame>(filter.entity);
    if (f->Has<PlayerActor>(filter.entity)) {
        KillableLib::Kill(f, filter.entity, killType, killer);
    } else {
        f->Destroy(filter.entity);
    }
}

void KillableSystem::OnAdded(Frame* f, EntityRef entity, Killable* component) {
    component->IsAlive = true;
    f->Events.KillableSpawnEvent(entity);
}

void KillableSystem::Reward_OnCollected(Frame* frame, EntityRef collectorEntity, DroppableType type,
                                        std::uint16_t amount) {
    if (type == DroppableType::ExtraLife) {
        KillableLib::AddExtraLives(frame, collectorEntity, amount);
    }
}

void KillableSystem::ParabolaFollower_OnReachedEnd(Frame* f, EntityRef entity) {
    Killable* killable = nullptr;
    if (f->TryGetPointer<Killable>(entity, killable) && killable->deathAnimationState == DeathAnimationState{1}) {
        killable->deathAnimationState = DeathAnimationState{2};
    }
}

void KillableSystem::Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                                       const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    Killable* killable = nullptr;
    if (f->TryGetPointer<Killable>(toRevive, killable)) {
        killable->deathAnimationState = DeathAnimationState{0};
    }
}

// --- OnKillCallbacksSystem --------------------------------------------------------------------

void OnKillCallbacksSystem::Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) {
    if (!f->Has<PlayerActor>(toKill)) {
        return;
    }
    OnKillCallbacks callbacks{};
    if (!f->TryGet<OnKillCallbacks>(killedBy.owner, callbacks)) {
        return;
    }
    Collections::QList<AssetRefCallbackAction> actions;
    if (!f->TryResolveList(callbacks.get_onPlayerKilled(), actions)) {
        return;
    }
    for (int i = 0; i < actions.get_Count(); ++i) {
        CallbackActionLib::Execute(actions.get_Item(i), f, killedBy.owner);
    }
}

// --- LifetimeLib / LifetimeSystem / LifetimeInitSystem ----------------------------------------

void LifetimeLib::TryInitialiseStats(Frame* frame, EntityRef entity, Lifetime* lifetime, LifetimeSpec* lifetimeSpec) {
    if (lifetime->Stats.initialised) {
        return;
    }
    lifetime->Stats.initialised = true;
    lifetime->Stats.lifetimeDuration = lifetimeSpec->LifetimeDuration;
    if (!lifetimeSpec->UseOwnerWeaponLevelForLifetime) {
        return;
    }
    Ownership* ownership = nullptr;
    WeaponUser* weaponUser = nullptr;
    if (!frame->TryGetPointer<Ownership>(entity, ownership) ||
        !frame->TryGetPointer<WeaponUser>(ownership->Owner, weaponUser)) {
        return;
    }
    const std::int32_t level = WeaponLib::GetWeaponLevel(frame, ownership->Owner, weaponUser->EquippedWeapon);
    if (level >= 0 && level < static_cast<std::int32_t>(lifetimeSpec->LeveledLifetimeDurations.size())) {
        lifetime->Stats.lifetimeDuration = lifetimeSpec->LeveledLifetimeDurations[static_cast<std::size_t>(level)];
    }
}

void LifetimeSystem::Update(Frame* frame, Filter& filter) {
    Lifetime* lifetime = filter.lifetime;
    LifetimeSpec* spec = frame->FindAsset<LifetimeSpec>(lifetime->Spec.Id);
    LifetimeLib::TryInitialiseStats(frame, filter.entity, lifetime, spec);
    if (lifetime->StartFrame == 0) {
        return;
    }
    if (frame->get_DeltaTime() * (frame->Number - lifetime->StartFrame) >= lifetime->Stats.lifetimeDuration) {
        KillableLib::Kill(frame, filter.entity, KillType::Undefined, EntityRef{});
    }
}

void LifetimeSystem::OnAdded(Frame* frame, EntityRef entity, Lifetime* lifetime) {
    (void)entity;
    Collections::QList<AssetRefEntityPrototype> toSpawn = frame->AllocateList<AssetRefEntityPrototype>(8);
    LifetimeSpec* spec = nullptr;
    if (frame->TryFindAssetInternal(lifetime->Spec.Id, true, spec)) {
        for (const AssetRefEntityPrototype& prototype : spec->EntitiesToSpawnOnDeath) {
            toSpawn.Add(prototype);
        }
    }
    lifetime->set_EntitiesToSpawnOnDeath(toSpawn);
}

void LifetimeSystem::OnRemoved(Frame* f, EntityRef entity, Lifetime* lifetime) {
    (void)entity;
    if (f->TryFreeList(lifetime->get_EntitiesToSpawnOnDeath())) {
        lifetime->set_EntitiesToSpawnOnDeath(Collections::QListPtr<AssetRefEntityPrototype>{});
    }
}

void LifetimeInitSystem::OnAdded(Frame* f, EntityRef entity, Lifetime* lifetime) {
    LifetimeSpec* spec = nullptr;
    if (f->TryFindAssetInternal(lifetime->Spec.Id, true, spec)) {
        lifetime->StartFrame = 0;
        if (spec->StartLifetimeOnSpawn) {
            lifetime->StartFrame = f->Number;
        }
        return;
    }
    Photon::Deterministic::Log::Error("LifetimeInitSystem: a lifetime without a spec was removed");
    f->Remove<Lifetime>(entity);
}

void ProjectileInitSystem::OnAdded(Frame* f, EntityRef entity, Projectile* component) {
    ProjectileSpec* spec = nullptr;
    if (f->TryFindAssetInternal(component->Spec.Id, true, spec)) {
        component->SpawnedFrame = f->Number;
        return;
    }
    Photon::Deterministic::Log::Error("ProjectileInitSystem: a projectile without a spec was removed");
    f->Remove<Projectile>(entity);
}

} // namespace Quantum
