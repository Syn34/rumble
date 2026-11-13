#include "q_hash_collection.h"
#include "q_dictionary.h"

#include "quantum_core/frame_serializer.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/native.h"

#include <algorithm>

namespace Quantum::Collections {

const auto GetAlignmentForArrayElement = &Photon::Deterministic::Native::GetAlignmentForArrayElement;
const auto RoundUpToAlignment = &Photon::Deterministic::Native::RoundUpToAlignment;

void QHashCollection::CalculateStrides(int& keyStride, int& entryStride) {
    entryStride = static_cast<int>(sizeof(Entry));
    const int alignment = std::max(4, GetAlignmentForArrayElement(keyStride));
    keyStride = RoundUpToAlignment(keyStride, alignment);
    entryStride = RoundUpToAlignment(entryStride, alignment);
}

void QDictionaryInternal::CalculateStrides(int& entryStride, int& keyStride, int& valStride) {
    entryStride = static_cast<int>(sizeof(QHashCollection::Entry));
    const int alignment = std::max(4, std::max(GetAlignmentForArrayElement(keyStride),
                                               GetAlignmentForArrayElement(valStride)));
    keyStride = RoundUpToAlignment(keyStride, alignment);
    valStride = RoundUpToAlignment(valStride, alignment);
    entryStride = RoundUpToAlignment(entryStride, alignment);
}

void QDictionaryInternal::Serialize(Quantum::Ptr* ptr, FrameSerializer* serializer, int keyStride,
                                    int valStride, QBuffer::ItemSerializer keySerializer,
                                    QBuffer::ItemSerializer valueSerializer) {
    FrameSerializer::PtrKey key;
    if (!serializer->PointerSerialize(ptr, key)) {
        return;
    }
    auto* map = static_cast<QDictionaryInternal*>(
        serializer->PointerResolve(key, static_cast<int>(sizeof(QDictionaryInternal))));
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    QHashCollection& collection = map->_collection;
    stream.Serialize(&collection.UsedCount);
    stream.Serialize(&collection.FreeCount);
    serializer->PointerSerialize(&collection.FreeHead);

    int entryStride = 0;
    CalculateStrides(entryStride, keyStride, valStride);
    if (serializer->Reading()) {
        collection.KeyOffset = entryStride;
        map->_valueOffset = keyStride + entryStride;
    }

    const int stride = entryStride + keyStride + valStride;
    FrameSerializer::PtrKey entriesKey;
    if (QBuffer::Serialize(&collection.Entries, stride, serializer, entriesKey)) {
        auto* entries = static_cast<std::byte*>(
            serializer->PointerResolve(entriesKey, collection.Entries.Length * stride));
        for (int i = 0; i < collection.Entries.Length; ++i) {
            std::byte* slot = entries + static_cast<std::ptrdiff_t>(stride) * i;
            // Entries are linked by Ptr, so each slot's address takes part in the key
            // mapping.
            serializer->PointerSerializeVoid(slot);
            auto* entry = reinterpret_cast<QHashCollection::Entry*>(slot);
            serializer->PointerSerialize(&entry->Next);
            stream.Serialize(&entry->Hash);
            if (serializer->Writing()) {
                stream.WriteInt(static_cast<int>(entry->State));
            } else {
                entry->State = static_cast<QHashCollection::EntryState>(stream.ReadInt());
            }
            if (entry->State == QHashCollection::EntryState::Used) {
                keySerializer(slot + entryStride, serializer);
                valueSerializer(slot + entryStride + keyStride, serializer);
            }
        }
    }

    FrameSerializer::PtrKey bucketsKey;
    if (serializer->PointerSerialize(&collection.Buckets, bucketsKey)) {
        auto* buckets = static_cast<Quantum::Ptr*>(serializer->PointerResolve(
            bucketsKey, collection.Entries.Length * Quantum::Ptr::SIZE));
        for (int i = 0; i < collection.Entries.Length; ++i) {
            serializer->PointerSerialize(&buckets[i]);
        }
    }
}

} // namespace Quantum::Collections
