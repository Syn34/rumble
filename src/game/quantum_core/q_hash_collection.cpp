#include "q_hash_collection.h"
#include "q_dictionary.h"

#include "photon_deterministic/native_allocator.h"

#include <algorithm>

namespace Quantum::Collections {

using Photon::Deterministic::Native::GetAlignmentForArrayElement;
using Photon::Deterministic::Native::RoundUpToAlignment;

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

} // namespace Quantum::Collections
