// hit3d.cpp - Quantum.Physics3D.Hit3D / HitCollection3D.
#include "hit3d.h"

#include "quantum_core/frame_base.h"
#include "quantum_core/map.h"

#include "quantum_core/physics2d/hit_collection_storage.h"

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

// 0x7d6dc90
HitCollection3D::HitCollection3D(std::int32_t capacity, Modes mode, Core::FrameContext* context)
    : _mode(mode), _count(0), _capacity(capacity) {
    _hits3D = Physics2D::hit_storage::AllocateHits<Hit3D>(capacity, mode, context);
}

// 0x7d6e07c
void HitCollection3D::Free(Photon::Deterministic::Native::Allocator* allocator) {
    Physics2D::hit_storage::FreeHits(_hits3D, _count, _capacity, _mode, allocator);
}

// 0x7d6e0cc
void HitCollection3D::Add(const Hit3D& hit, Core::FrameContext* context) {
    if (_count == _capacity) {
        ExpandCapacity(_count * 2, context);
    }
    _hits3D[_count++] = hit;
}

// 0x7d6e120
void HitCollection3D::ExpandCapacity(std::int32_t newCapacity, Core::FrameContext* context) {
    Physics2D::hit_storage::ExpandHits(_hits3D, _capacity, _mode, newCapacity, context);
}

namespace {
// The map a static hit names: the frame's own map when it is that one, otherwise the asset.
const Map* MapOfHit(Core::FrameBase* frame, AssetGuid guid, bool quiet) {
    const Map* map = frame->get_Map();
    if (map != nullptr && map->Guid == guid) {
        return map;
    }
    if (quiet) {
        Map* found = nullptr;
        return frame->TryFindAsset<Map>(guid, found) ? found : nullptr;
    }
    return frame->FindAsset<Map>(guid);
}
} // namespace

bool Hit3D::TryGetStaticCollider(Core::FrameBase* frame, MapStaticCollider3D& collider) const {
    collider = MapStaticCollider3D{};
    if (_staticColliderIndex < 0) {
        return false;
    }
    const Map* map = MapOfHit(frame, _mapGuid, true);
    if (map == nullptr || _staticColliderIndex >= static_cast<std::int32_t>(map->StaticColliders3D.size())) {
        return false;
    }
    collider = map->StaticColliders3D[static_cast<std::size_t>(_staticColliderIndex)];
    return true;
}

MapStaticCollider3D Hit3D::GetStaticCollider(Core::FrameBase* frame) const {
    const Map* map = MapOfHit(frame, _mapGuid, false);
    if (map == nullptr) {
        throw std::invalid_argument("Hit3D.GetStaticCollider: map not found");
    }
    return map->StaticColliders3D.at(static_cast<std::size_t>(_staticColliderIndex));
}

} // namespace Quantum::Physics3D
