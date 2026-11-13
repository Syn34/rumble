// hit.cpp - Quantum.Physics2D.Hit / HitCollection.
#include "hit.h"

#include "quantum_core/physics2d/hit_collection_storage.h"

#include "photon_deterministic/fp_internal.h"
#include "quantum_core/hit_quick_sort.h"


namespace Quantum::Physics2D {

namespace detail = Photon::Deterministic::detail;
using detail::HashCombine;
using detail::HashSeed;


// 0x7d1144c
std::int32_t Hit::GetHashCode() const {
    std::int32_t hash = HashCombine(HashSeed, _isTrigger);
    hash = HashCombine(hash, ShapeUserTag);
    hash = HashCombine(hash, _staticColliderIndex);
    hash = HashCombine(hash, OverlapPenetration.GetHashCode());
    // EntityRef's hash, inlined by the original: Index * 59209 + Version.
    const auto entityHash = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(_entity.Index) * 59209u + static_cast<std::uint32_t>(_entity.Version));
    hash = HashCombine(hash, entityHash);
    hash = HashCombine(hash, static_cast<std::int32_t>(static_cast<std::uint32_t>(sqrDistance) ^
                                                       static_cast<std::uint32_t>(
                                                           static_cast<std::uint64_t>(sqrDistance) >> 32)));
    hash = HashCombine(hash, Point.GetHashCode());
    return HashCombine(hash, Normal.GetHashCode());
}

// 0x7d1157c
void HitCollection::Sort(FPVector2 point) {
    if (_count < 2) {
        return;
    }
    for (std::int32_t i = 0; i < _count; ++i) {
        _hits[i].sqrDistance = FPVector2::DistanceSquared(point, _hits[i].Point).RawValue();
    }
    QuickSort(_hits, 0, _count - 1);
}

// 0x7d11744
void HitCollection::SortCastDistance() {
    if (_count < 2) {
        return;
    }
    QuickSortCastDistance(_hits, 0, _count - 1);
}

// 0x7d115f4
void HitCollection::QuickSort(Hit* hits, std::int32_t lo, std::int32_t hi) {
    Quantum::detail::LomutoQuickSort(hits, lo, hi, [](const Hit& hit) { return hit.sqrDistance; });
}

// 0x7d11764
void HitCollection::QuickSortCastDistance(Hit* hits, std::int32_t lo, std::int32_t hi) {
    Quantum::detail::LomutoQuickSort(hits, lo, hi, [](const Hit& hit) { return hit.CastDistanceNormalized; });
}

// 0x7d11520
HitCollection::HitCollection(std::int32_t capacity, Modes mode, Core::FrameContext* context)
    : _mode(mode), _count(0), _capacity(capacity) {
    _hits = Physics2D::hit_storage::AllocateHits<Hit>(capacity, mode, context);
}

// 0x7d118b4
void HitCollection::Free(Photon::Deterministic::Native::Allocator* allocator) {
    Physics2D::hit_storage::FreeHits(_hits, _count, _capacity, _mode, allocator);
}

// 0x7d118fc
void HitCollection::Add(const Hit& hit, Core::FrameContext* context) {
    if (_count == _capacity) {
        ExpandCapacity(_count * 2, context);
    }
    _hits[_count++] = hit;
}

// 0x7d11958
void HitCollection::ExpandCapacity(std::int32_t newCapacity, Core::FrameContext* context) {
    Physics2D::hit_storage::ExpandHits(_hits, _capacity, _mode, newCapacity, context);
}

} // namespace Quantum::Physics2D
