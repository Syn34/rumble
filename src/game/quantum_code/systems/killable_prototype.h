// killable_prototype.h - Quantum.Prototypes.Killable_Prototype: the prototype data of a
// Killable that KillableLib.Kill reads (what happens when the entity is killed).
// Declared here with the fields the systems use until the quantum.code component
// prototypes are reimplemented; abstract (no materialisation).
#pragma once

#include "quantum_code/components.h"
#include "quantum_core/component_prototype.h"

namespace Quantum {

namespace Prototypes {
// The prototype of a Killable: what happens when the entity is killed. (Abstract here:
// the prototype classes' materialisation is not part of the systems.)
class Killable_Prototype : public TypedComponentPrototype<Killable> {
  public:
    AssetRefDeathAnimationSpec deathAnimationSpec;
    KillableOnKillBehaviour OnKillBehaviour{};
    FP DelayBeforeDestruction;
    bool OnKillDisablePhysics = false;
    bool OnKillDisableCollider = false;
    AssetRefEntityPrototype EntityToSpawnOnDeath;
};
} // namespace Prototypes

} // namespace Quantum
