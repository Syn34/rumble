#include "component_reference.h"

#include "quantum_core/frame_serializer.h"

namespace Quantum {

void ComponentReference::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* reference = static_cast<ComponentReference*>(ptr);
    EntityRef::Serialize(&reference->Entity, serializer);
    serializer->Stream().Serialize(&reference->Component);
}

} // namespace Quantum
