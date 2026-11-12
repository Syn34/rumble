// hit3d.cpp - Quantum.Physics3D.Hit3D / HitCollection3D.
#include "hit3d.h"

#include "photon_deterministic/fp_internal.h"
#include "quantum_core/hit_quick_sort.h"

#include <stdexcept>

namespace Quantum::Physics3D {

namespace fpdetail = Photon::Deterministic::detail;
using fpdetail::HashCombine;
using fpdetail::HashSeed;

// 0x7d6db60 - same field order as the 2D Hit.
std::int32_t Hit3D::GetHashCode() const {
    std::int32_t hash = HashCombine(HashSeed, _isTrigger);
    hash = HashCombine(hash, ShapeUserTag);
    hash = HashCombine(hash, _staticColliderIndex);
    hash = HashCombine(hash, OverlapPenetration.GetHashCode());
    // EntityRef's hash, inlined by the original: Index * 59209 + Version.
    const auto entityHash = static_cast<std::int32_t>(static_cast<std::uint32_t>(_entity.Index) * 59209u +
                                                      static_cast<std::uint32_t>(_entity.Version));
    hash = HashCombine(hash, entityHash);
    const auto bits = static_cast<std::uint64_t>(sqrDistance);
    hash = HashCombine(hash, static_cast<std::int32_t>(static_cast<std::uint32_t>(bits) ^
                                                       static_cast<std::uint32_t>(bits >> 32)));
    hash = HashCombine(hash, Point.GetHashCode());
    return HashCombine(hash, Normal.GetHashCode());
}

// 0x7d6dc2c
Hit3D HitCollection3D::operator[](std::int32_t index) const {
    if (static_cast<std::uint32_t>(index) >= static_cast<std::uint32_t>(_count)) {
        throw std::out_of_range("HitCollection3D index");
    }
    return _hits3D[index];
}

// 0x7d6dcf4
void HitCollection3D::Sort(const FPVector3& point) {
    if (_count < 2) {
        return;
    }
    for (std::int32_t i = 0; i < _count; ++i) {
        _hits3D[i].sqrDistance = FPVector3::DistanceSquared(point, _hits3D[i].Point).RawValue();
    }
    QuickSort(_hits3D, 0, _count - 1);
}

// 0x7d6def0
void HitCollection3D::SortCastDistance() {
    if (_count < 2) {
        return;
    }
    QuickSortCastDistance(_hits3D, 0, _count - 1);
}

// 0x7d6dd84
void HitCollection3D::QuickSort(Hit3D* hits, std::int32_t lo, std::int32_t hi) {
    Quantum::detail::LomutoQuickSort(hits, lo, hi, [](const Hit3D& hit) { return hit.sqrDistance; });
}

// 0x7d6df10
void HitCollection3D::QuickSortCastDistance(Hit3D* hits, std::int32_t lo, std::int32_t hi) {
    Quantum::detail::LomutoQuickSort(hits, lo, hi,
                                     [](const Hit3D& hit) { return hit.CastDistanceNormalized; });
}

} // namespace Quantum::Physics3D
