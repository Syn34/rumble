// prototype_support.h - code shared by the generated component prototypes (quantum.code.dll).
//
// The original repeats these few lines in every prototype; here they are written once.
#pragma once

#include "quantum_code/frame.h"
#include "quantum_code/prototypes/component_prototype_visitor.h"
#include "quantum_core/component_prototype.h"

#include <stdexcept>
#include <string>

namespace Quantum::Prototypes {

// The game's frame behind a framework frame pointer (AddToEntity casts it in the original
// and fails with an invalid cast otherwise).
inline Frame* AsGameFrame(Core::FrameBase* f, const ComponentPrototype& prototype) {
    auto* frame = dynamic_cast<Frame*>(f);
    if (frame == nullptr) {
        throw std::logic_error(prototype.PrototypeTypeName() + ": not a game frame");
    }
    return frame;
}

// The visitor of the game's prototypes behind a framework visitor (Dispatch casts it).
inline ComponentPrototypeVisitor& AsGameVisitor(ComponentPrototypeVisitorBase& visitor,
                                                const ComponentPrototype& prototype) {
    auto* gameVisitor = dynamic_cast<ComponentPrototypeVisitor*>(&visitor);
    if (gameVisitor == nullptr) {
        throw std::logic_error(prototype.PrototypeTypeName() +
                               ": visitor does not handle game prototypes");
    }
    return *gameVisitor;
}

// The body of every AddToEntity: materialise into a zeroed component and Set it on the
// entity (replacing any existing one). True when the component was new.
template <class TComponent, class TPrototype>
bool MaterializeAndSet(const TPrototype& prototype, Core::FrameBase* f, EntityRef entity,
                       const PrototypeMaterializationContext& context) {
    Frame* frame = AsGameFrame(f, prototype);
    TComponent component{};
    prototype.Materialize(frame, component, context);
    return frame->Set(entity, component) == SetResult::ComponentAdded;
}

} // namespace Quantum::Prototypes
