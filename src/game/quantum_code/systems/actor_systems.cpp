// actor_systems.cpp
#include "quantum_code/systems/actor_systems.h"

#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/enhancement_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/assets/weapon_specs.h"
#include "quantum_code/systems/libs.h"

namespace Quantum {

void ActorIcedSystem::OnAdded(Frame* f, EntityRef entity, ActorIced* component) {
    (void)component;
    f->Events.ActorIcedChange(entity, true);
}

void ActorIcedSystem::OnRemoved(Frame* f, EntityRef entity, ActorIced* component) {
    (void)component;
    f->Events.ActorIcedChange(entity, false);
}

void ActorSheepedSystem::OnAdded(Frame* f, EntityRef entity, ActorSheeped* component) {
    (void)component;
    f->Events.ActorSheepedChange(entity, true);
}

void ActorSheepedSystem::OnRemoved(Frame* f, EntityRef entity, ActorSheeped* component) {
    (void)component;
    f->Events.ActorSheepedChange(entity, false);
}

void ActorStunnedSystem::OnAdded(Frame* f, EntityRef entity, ActorStunned* component) {
    (void)component;
    f->Events.ActorStunnedChange(entity, true);
}

void ActorStunnedSystem::OnRemoved(Frame* f, EntityRef entity, ActorStunned* component) {
    (void)component;
    f->Events.ActorStunnedChange(entity, false);
}

void ActorInputInitialiserSystem::OnAdded(Frame* f, EntityRef entity, ActorInputInitialiser* component) {
    (void)component;
    f->Add<ActorInput>(entity);
    f->Remove<ActorInputInitialiser>(entity);
}

void EntityInfoSystem::OnAdded(Frame* f, EntityRef entity, Code::EntityInfo* component) {
    constexpr std::uint32_t NeutralTypes = (1u << 2) | (1u << 3) | (1u << 4) | (1u << 8) | (1u << 12);
    const auto type = static_cast<std::uint32_t>(component->type);
    if (((1u << (type & 31u)) & NeutralTypes) != 0) {
        Team::JoinTeam(f, entity, Team::NeutralTeamID);
    }
}

void CharacterColliderScalerSystem::OnAdded(Frame* f, EntityRef entity, CharacterColliderScaler* component) {
    (void)f;
    (void)entity;
    component->CurrentScaleFactor = FP::One();
}

void ActorInitialisedSystem::Update(Frame* f, Filter& filter) {
    f->Add<PlayerActorInitialized>(filter.entity);
    f->Signals.PlayerActor_OnInitialized(filter.entity, filter.actor);
    f->Signals.PlayerActor_OnAfterInitialized(filter.entity, filter.actor);
}

void EntityGroupSystem::OnAdded(Frame* f, EntityRef entity, EntityGroup* component) {
    EntityGroupSpec* spec = nullptr;
    if (f->TryFindAssetInternal(component->Spec.Id, true, spec) && spec->GroupMaxMembers > 0) {
        component->Stats.EntityGroupMaxMembers = spec->GroupMaxMembers;
        component->Stats.IsInitialised = true;
        return;
    }
    f->Remove<EntityGroup>(entity);
}

void ActorCombatActivitySystem::Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                                                  const FPQuaternion& rotation) {
    (void)position;
    (void)rotation;
    PlayerActor* actor = nullptr;
    if (f->TryGetPointer<PlayerActor>(toRevive, actor)) {
        actor->timeWithoutCombatActivity = FP{};
    }
}

void ActorCombatActivitySystem::Pushable_OnPushed(Frame* f, const PushData& pushData,
                                                  const OnPushedData& onPushedData) {
    (void)onPushedData;
    PlayerActor* pushed = nullptr;
    if (f->TryGetPointer<PlayerActor>(pushData.pushed, pushed)) {
        pushed->timeWithoutCombatActivity = FP{};
    }
    PlayerActor* pusherOwner = nullptr;
    if (f->TryGetPointer<PlayerActor>(pushData.pusherOwner, pusherOwner) &&
        (f->Has<CombatParticipant>(pushData.pushed) || f->Has<PlayerActor>(pushData.pushed))) {
        pusherOwner->timeWithoutCombatActivity = FP{};
    }
}

void ActorCombatActivitySystem::Damageable_HitReceived(Frame* f, EntityRef entity, const HitData& hitData,
                                                       const HitResult& hitResult) {
    (void)hitResult;
    if (!f->Has<PlayerActor>(entity)) {
        return;
    }
    PlayerActor* attacker = nullptr;
    if (f->TryGetPointer<PlayerActor>(hitData.owner, attacker)) {
        attacker->inCombatWith = entity;
        attacker->combatStartFrame = f->Number;
    }
}

void ActorCombatActivitySystem::Update(Frame* f, Filter& filter) {
    if (!GameModeHandlerLib::IsInActionPhase(f)) {
        return;
    }
    PlayerActor* actor = filter.pActor;
    if (actor->combatStartFrame >= 1) {
        if (f->get_DeltaTime() * (f->Number - actor->combatStartFrame) >= actor->durationBeforeCombatReset) {
            actor->inCombatWith = EntityRef{};
            actor->combatStartFrame = 0;
        }
    } else {
        actor->timeWithoutCombatActivity = f->get_DeltaTime() + actor->timeWithoutCombatActivity;
    }
}

void ActorKillsHistorySystem::Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) {
    PlayerActor* victim = nullptr;
    if (!f->TryGetPointer<PlayerActor>(toKill, victim)) {
        return;
    }
    victim->killStreak = 0;
    std::int32_t killerTeam = -1;
    std::int32_t victimTeam = -1;
    if (Team::TryGetTeamID(f, killedBy.owner, killerTeam) && Team::TryGetTeamID(f, toKill, victimTeam) &&
        killerTeam == victimTeam) {
        return;
    }
    PlayerActor* killer = nullptr;
    if (f->TryGetPointer<PlayerActor>(killedBy.owner, killer)) {
        ++killer->killStreak;
        killer->killCount = static_cast<std::int16_t>(killer->killCount + 1);
        killer->lastKillFrame = f->Number;
    }
}

void ActorStatusLib::RequestStatusChange(Frame* f, EntityRef actorEntity, ActorStatusType type, FP duration) {
    ActorStatus* status = nullptr;
    if (f->TryGetPointer<ActorStatus>(actorEntity, status)) {
        RequestStatusChange(f, actorEntity, status, type, duration);
    }
}

void ActorStatusLib::RequestStatusChange(Frame* f, EntityRef actorEntity, ActorStatus* actorStatus,
                                         ActorStatusType type, FP duration) {
    if (actorStatus->currentType == type) {
        return;
    }
    ActorStatusChangeRequested* request = nullptr;
    f->AddOrGet<ActorStatusChangeRequested>(actorEntity, request);
    request->wantedType = type;
    request->wantedDuration = duration;
}

} // namespace Quantum
