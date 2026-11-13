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

} // namespace Quantum
