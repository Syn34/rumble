// q_list.cpp - serialisation of Quantum.Collections.QListInternal (quantum.core.dll).
#include "quantum_core/q_list.h"

#include "quantum_core/frame_serializer.h"

#include "photon_deterministic/bit_stream.h"

namespace Quantum::Collections {

void QListInternal::Serialize(Quantum::Ptr* ptr, FrameSerializer* serializer, int stride,
                              QBuffer::ItemSerializer itemSerializer) {
    FrameSerializer::PtrKey key;
    if (!serializer->PointerSerialize(ptr, key)) {
        return;
    }
    auto* list = static_cast<QListInternal*>(
        serializer->PointerResolve(key, static_cast<int>(sizeof(QListInternal))));
    serializer->Stream().Serialize(&list->_count);
    QBuffer::Serialize(&list->_items, stride, serializer, list->_count, itemSerializer);
}

} // namespace Quantum::Collections
