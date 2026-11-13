// component_reference.h - Quantum.ComponentReference: (entity, component type index).
//
// Used as the key of the frame's deferred component-removal sets.
#pragma once

#include "quantum_core/entity_ref.h"

#include <functional>

namespace Quantum {

class FrameSerializer;

struct ComponentReference {
    EntityRef Entity;
    int Component = 0;
    int _padding = 0;

    ComponentReference() = default;
    // 0x7ceb63c
    ComponentReference(EntityRef entity, int component) : Entity(entity), Component(component) {}

    // 0x7ceb648 (Equals(ComponentReference)); 0x7ceb67c is the boxed Equals(object).
    bool Equals(const ComponentReference& other) const {
        return Entity == other.Entity && Component == other.Component;
    }
    friend bool operator==(const ComponentReference& a, const ComponentReference& b) {
        return a.Equals(b);
    }

    // 0x7ceb704 - entity hash * 397 XOR component index.
    int GetHashCode() const {
        return static_cast<int>(static_cast<std::uint32_t>(Entity.GetHashCode()) * 397u ^
                                static_cast<std::uint32_t>(Component));
    }

    // 0x7ceb724 - The entity, then the component index.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(ComponentReference) == 16);

} // namespace Quantum
