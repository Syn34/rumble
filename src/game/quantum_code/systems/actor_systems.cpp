// actor_systems.cpp
#include "quantum_code/systems/actor_systems.h"

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

} // namespace Quantum
