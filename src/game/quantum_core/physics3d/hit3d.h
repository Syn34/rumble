// hit3d.h - results of 3D physics queries: Quantum.Physics3D.Hit3D and HitCollection3D.
#pragma once

#include "quantum_core/asset_guid.h"
#include "quantum_core/physics2d/hit.h" // HitEntityRef

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>

namespace Quantum::Physics3D {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;
using Physics2D::HitEntityRef;

struct Hit3D {
    static constexpr int SIZE = 80;
    static constexpr int PenetrationOffset = 8;
    static constexpr int EntityOffset = 16;
    static constexpr int FirstVector3 = 32;

    std::uint16_t _isTrigger = 0;
    std::uint16_t ShapeUserTag = 0;
    std::int32_t _staticColliderIndex = 0; // negative for dynamic (entity) hits
    union {
        FP OverlapPenetration{};
        FP CastDistanceNormalized;
    };
    union {
        HitEntityRef _entity{};
        AssetGuid _mapGuid;
    };
    std::int64_t sqrDistance = 0;
    FPVector3 Point;
    FPVector3 Normal;

    // 0x7d6d6a0
    bool IsDynamic() const { return _staticColliderIndex < 0; }
    // 0x7d6d6ac
    std::int32_t StaticColliderIndex() const { return _staticColliderIndex; }
    // 0x7d6d6b4
    bool IsTrigger() const { return _isTrigger == 1; }
    // 0x7d6d6c4
    void SetIsTrigger(bool value) { _isTrigger = value ? 1 : 0; }
    // 0x7d6d6d0 - the entity for dynamic hits, a default (invalid) reference otherwise.
    HitEntityRef Entity() const { return IsDynamic() ? _entity : HitEntityRef{}; }

    // 0x7d6db60
    std::int32_t GetHashCode() const;

    // Need the frame / map assets:
    // TODO(0x7d6d6e8, Quantum.Physics3D.Hit3D.TryGetStaticCollider), also 0x7d6d728,
    // TODO(0x7d6d840, Quantum.Physics3D.Hit3D.GetStaticCollider), also 0x7d6d870.
    // TODO(0x7d6d92c, Quantum.Physics3D.Hit3D.Serialize)
};
static_assert(sizeof(Hit3D) == Hit3D::SIZE);

struct HitCollection3D {
    enum Modes : std::int32_t {
        None = 0,
        Temp = 1,
        Persistent = 2,
    };

    Modes _mode = None;
    Hit3D* _hits3D = nullptr;
    std::int32_t _count = 0;
    std::int32_t _capacity = 0;

    std::int32_t Count() const { return _count; }         // 0x7d6dc1c
    void SetCount(std::int32_t value) { _count = value; } // 0x7d6dc24
    Hit3D* HitsBuffer() const { return _hits3D; }         // 0x7d6dc88
    // 0x7d6dc2c - copy of hit `index`; throws std::out_of_range past Count
    // (IndexOutOfRangeException in the original).
    Hit3D operator[](std::int32_t index) const;

    // 0x7d6dcf4 - nearest to `point` first (distances cached in sqrDistance).
    void Sort(const FPVector3& point);
    // 0x7d6def0 - by CastDistanceNormalized.
    void SortCastDistance();
    // 0x7d6dd84 - Lomuto quicksort on sqrDistance over [lo, hi].
    static void QuickSort(Hit3D* hits, std::int32_t lo, std::int32_t hi);
    // 0x7d6df10
    static void QuickSortCastDistance(Hit3D* hits, std::int32_t lo, std::int32_t hi);

    // 0x7d6e0c4
    void Reset() { _count = 0; }

    HitCollection3D() = default;
    // 0x7d6dc90 - see HitCollection (2D).
    HitCollection3D(std::int32_t capacity, Modes mode, Core::FrameContext* context);
    // 0x7d6e07c
    void Free(Photon::Deterministic::Native::Allocator* allocator);
    // 0x7d6e0cc
    void Add(const Hit3D& hit, Core::FrameContext* context);
    // 0x7d6e120
    void ExpandCapacity(std::int32_t newCapacity, Core::FrameContext* context);
};

} // namespace Quantum::Physics3D
