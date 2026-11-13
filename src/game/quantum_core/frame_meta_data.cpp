#include "frame_meta_data.h"

#include "quantum_core/component_reference.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/frame_serializer.h"

namespace Quantum {

void FrameMetaData::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* meta = static_cast<FrameMetaData*>(ptr);
    // 0x7cef5c0 / 0x7cef5cc (<>c.<Serialize>b__7_0 / b__7_1) are the key serializers.
    QHashCollectionList::Serialize<EntityRef>(&meta->_entityDestroyList, serializer,
                                              &EntityRef::Serialize);
    QHashCollectionList::Serialize<ComponentReference>(&meta->_componentRemovalList, serializer,
                                                       &ComponentReference::Serialize);
    Allocator::Heap::AllocationTracker::Serialize(&meta->_trackedInternal, serializer);
}

} // namespace Quantum
