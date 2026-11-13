// component_pointers.cpp - collection lifetime of the types whose collections sit inside
// nested structs (the rest is in components_N.cpp).
#include "quantum_code/components.h"

#include "quantum_code/frame.h"

namespace Quantum {

void RuntimeHFSM::ClearPointers(Frame* f, EntityRef entityRef) {
    blackboard.ClearPointers(f, entityRef);
}

void CompoundHFSM::ClearPointers(Frame* f, EntityRef entityRef) {
    attackHFSM.ClearPointers(f, entityRef);
    navHFSM.ClearPointers(f, entityRef);
}

void PrioritisedTargetHandler::ClearPointers(Frame* f, EntityRef entityRef) {
    for (PrioritisedTarget& target : get_targets()) {
        target.ClearPointers(f, entityRef);
    }
}

void TargetFinder::AllocatePointers(Frame* f, EntityRef entityRef) {
    (void)entityRef;
    if (PotentialTargetsPtr == Ptr::Null()) {
        PotentialTargetsPtr = AllocateListPtr<PotentialTarget>(f, 8);
    }
}

void TargetFinder::ClearPointers(Frame* f, EntityRef entityRef) {
    AutoTargetInfo.ClearPointers(f, entityRef);
    if (PotentialTargetsPtr != Ptr::Null()) {
        f->FreeList(get_PotentialTargets());
    }
    PotentialTargetsPtr = Ptr::Null();
    WrenchAutoTargetInfo.ClearPointers(f, entityRef);
}

void Weapon::AllocatePointers(Frame* f, EntityRef entityRef) { Stats.AllocatePointers(f, entityRef); }

void Weapon::ClearPointers(Frame* f, EntityRef entityRef) { Stats.ClearPointers(f, entityRef); }

void UTAgent::ClearPointers(Frame* f, EntityRef entityRef) {
    UtilityReasoner.ClearPointers(f, entityRef);
}

} // namespace Quantum
