// actor_systems.h - systems that react to an actor's state components appearing and
// disappearing (status effects, input initialisation, entity info, collider scaling).
#pragma once

#include "quantum_code/systems/system_main_thread.h"

namespace Quantum {

// The ActorIced / ActorSheeped / ActorStunned components mark a status effect: their
// systems announce the change to the view.
class ActorIcedSystem final : public SystemSignalsOnly,
                              public ISignalOnComponentAdded<ActorIced>,
                              public ISignalOnComponentRemoved<ActorIced> {
  public:
    // 0x7cb9c98
    ActorIcedSystem() = default;
    // 0x7cb9c10
    void OnAdded(Frame* f, EntityRef entity, ActorIced* component) override;
    // 0x7cb9c54
    void OnRemoved(Frame* f, EntityRef entity, ActorIced* component) override;
};

class ActorSheepedSystem final : public SystemSignalsOnly,
                                 public ISignalOnComponentAdded<ActorSheeped>,
                                 public ISignalOnComponentRemoved<ActorSheeped> {
  public:
    // 0x7cb9e14
    ActorSheepedSystem() = default;
    // 0x7cb9d8c
    void OnAdded(Frame* f, EntityRef entity, ActorSheeped* component) override;
    // 0x7cb9dd0
    void OnRemoved(Frame* f, EntityRef entity, ActorSheeped* component) override;
};

class ActorStunnedSystem final : public SystemSignalsOnly,
                                 public ISignalOnComponentAdded<ActorStunned>,
                                 public ISignalOnComponentRemoved<ActorStunned> {
  public:
    // 0x7cb9f90
    ActorStunnedSystem() = default;
    // 0x7cb9f08
    void OnAdded(Frame* f, EntityRef entity, ActorStunned* component) override;
    // 0x7cb9f4c
    void OnRemoved(Frame* f, EntityRef entity, ActorStunned* component) override;
};

// ActorInputInitialiser is a one-shot marker: it is replaced by an ActorInput.
class ActorInputInitialiserSystem final : public SystemSignalsOnly,
                                          public ISignalOnComponentAdded<ActorInputInitialiser> {
  public:
    // 0x7cb2018
    ActorInputInitialiserSystem() = default;
    // 0x7cb1f68
    void OnAdded(Frame* f, EntityRef entity, ActorInputInitialiser* component) override;
};

// Puts entities of the neutral kinds (see OnAdded) into the neutral team.
class EntityInfoSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<Code::EntityInfo> {
  public:
    // 0x7cdb008
    EntityInfoSystem() = default;
    // 0x7cdaf68 - Entity info types 2, 3, 4, 8 and 12 join team -2.
    void OnAdded(Frame* f, EntityRef entity, Code::EntityInfo* component) override;
};

class CharacterColliderScalerSystem final : public SystemSignalsOnly,
                                            public ISignalOnComponentAdded<CharacterColliderScaler> {
  public:
    // 0x7bda580
    CharacterColliderScalerSystem() = default;
    // 0x7bda568 - A new scaler starts at scale 1.
    void OnAdded(Frame* f, EntityRef entity, CharacterColliderScaler* component) override;
};

struct ActorInitialisedSystemFilter {
    EntityRef entity;
    PlayerActor* actor = nullptr;
    static constexpr auto FilterEntity = &ActorInitialisedSystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&ActorInitialisedSystemFilter::actor);
};

// Announces each new player once (OnInitialized, then OnAfterInitialized).
class ActorInitialisedSystem final : public SystemMainThreadFilter<ActorInitialisedSystemFilter> {
  public:
    // 0x7cb1f20
    ActorInitialisedSystem() = default;
    // 0x7cb1e20
    ComponentSet get_Without() const override { return ComponentSet::Create<PlayerActorInitialized>(); }
    // 0x7cb1ea8 - Marks the player initialised, then raises both signals.
    void Update(Frame* f, Filter& filter) override;
};

// Groups take their size from their spec; groups without one are removed.
class EntityGroupSystem final : public SystemSignalsOnly, public ISignalOnComponentAdded<EntityGroup> {
  public:
    // 0x7bc0dfc
    EntityGroupSystem() = default;
    // 0x7bc0cd8 - A spec with a positive maximum initialises the group.
    void OnAdded(Frame* f, EntityRef entity, EntityGroup* component) override;
};

struct ActorCombatActivitySystemFilter {
    EntityRef entity;
    PlayerActor* pActor = nullptr;
    static constexpr auto FilterEntity = &ActorCombatActivitySystemFilter::entity;
    static constexpr auto FilterComponents = std::make_tuple(&ActorCombatActivitySystemFilter::pActor);
};

// Tracks whether players are fighting: who they last hit and for how long nothing
// happened.
class ActorCombatActivitySystem final : public SystemMainThreadFilter<ActorCombatActivitySystemFilter>,
                                        public ISignalKillable_OnRevive,
                                        public ISignalPushable_OnPushed,
                                        public ISignalDamageable_HitReceived {
  public:
    // 0x7cb1dd8
    ActorCombatActivitySystem() = default;
    // 0x7cb1a88
    void Killable_OnRevive(Frame* f, EntityRef toRevive, const FPVector3& position,
                           const FPQuaternion& rotation) override;
    // 0x7cb1b20 - Being pushed is activity; so is pushing a player or a combat
    // participant.
    void Pushable_OnPushed(Frame* f, const PushData& pushData, const OnPushedData& onPushedData) override;
    // 0x7cb1d08 - The attacker of a player is in combat with it from now on.
    void Damageable_HitReceived(Frame* f, EntityRef entity, const HitData& hitData, const HitResult& hitResult) override;
    // 0x7cb1c58 - In the action phase: a combat ends after the actor's reset duration;
    // out of combat, inactivity time accumulates.
    void Update(Frame* f, Filter& filter) override;
};

// Kill streaks and kill counts (team kills do not count).
class ActorKillsHistorySystem final : public SystemSignalsOnly, public ISignalKillable_OnKill {
  public:
    // 0x7bb1120
    ActorKillsHistorySystem() = default;
    // 0x7bb0f9c
    void Killable_OnKill(Frame* f, EntityRef toKill, const KilledByData& killedBy) override;
};

class ActorStatusLib {
  public:
    // 0x7cb8bac - With no duration (-1).
    static void RequestStatusChange(Frame* f, EntityRef actorEntity, ActorStatusType type) {
        RequestStatusChange(f, actorEntity, type, -FP::One());
    }
    // 0x7cb93b0 - For actors with an ActorStatus.
    static void RequestStatusChange(Frame* f, EntityRef actorEntity, ActorStatusType type, FP duration);
    // 0x7cb9464 - Queues the change (ActorStatusChangeRequested, the latest request wins)
    // unless the actor already has that status.
    static void RequestStatusChange(Frame* f, EntityRef actorEntity, ActorStatus* actorStatus, ActorStatusType type,
                                    FP duration);
    // TODO(0x7cb8c78, Quantum.ActorStatusLib.ChangeStatus): switches the status: checks the
    // current status allows it, swaps the per-status actor components, pushable state and
    // events; not reimplemented yet.
    // TODO(0x7cb950c, Quantum.ActorStatusLib.GetActorStatusTypeSettings): the settings of a
    // status in an ActorStatusSpec; not reimplemented yet.
};

} // namespace Quantum
