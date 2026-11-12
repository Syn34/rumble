// hit_quick_sort.h - the quicksort shared by Physics2D.HitCollection and
// Physics3D.HitCollection3D (internal helper, not an original type).
#pragma once

#include <cstdint>
#include <utility>

namespace Quantum::detail {

// Lomuto partition with the last element as pivot, recursing on the left part and looping
// on the right. Not stable: equal keys may be reordered, exactly as in the original.
template <typename HitT, typename Key>
void LomutoQuickSort(HitT* hits, std::int32_t lo, std::int32_t hi, Key key) {
    while (lo < hi) {
        const auto pivot = key(hits[hi]);
        std::int32_t store = lo;
        for (std::int32_t i = lo; i < hi; ++i) {
            if (key(hits[i]) < pivot) {
                std::swap(hits[store], hits[i]);
                ++store;
            }
        }
        std::swap(hits[store], hits[hi]);
        LomutoQuickSort(hits, lo, store - 1, key);
        lo = store + 1;
    }
}

} // namespace Quantum::detail
