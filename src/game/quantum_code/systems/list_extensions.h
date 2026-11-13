// list_extensions.h - Quantum.IListExtensions: generic helpers over frame lists. The
// original methods are generic (no address of their own); the instances used so far are
// noted on each template.
#pragma once

#include "quantum_code/frame.h"

namespace Quantum::IListExtensions {

// Swaps two items (instance for short inlined into ActorManager.HandleForcedLocalPlayerSpawnPoint).
template <class T> void Swap(Collections::QList<T>& list, std::int32_t indexA, std::int32_t indexB) {
    const T itemA = list.get_Item(indexA);
    const T itemB = list.get_Item(indexB);
    list.set_Item(indexA, itemB);
    list.set_Item(indexB, itemA);
}

// Shuffles with the frame's RNG, swapping each item from the last down to the second with
// one strictly before it (instance for short at 0x7c494b0).
template <class T> void Shuffle(Collections::QList<T>& list, Frame* f) {
    for (std::int32_t i = list.get_Count() - 1; i >= 1; --i) {
        const std::int32_t j = f->get_RNG()->Next(0, i);
        Swap(list, j, i);
    }
}

} // namespace Quantum::IListExtensions
