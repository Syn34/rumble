// hit.h - results of 2D physics queries: Quantum.Physics2D.Hit and HitCollection.
#pragma once

#include "quantum_core/asset_guid.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"

#include <cstdint>

namespace Quantum::Physics2D {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;

// Raw contents of a Quantum.EntityRef (the ECS type is not reimplemented yet).
struct HitEntityRef {
    std::int32_t Index = 0;
    std::int32_t Version = 0;
};

struct Hit {
    static constexpr int SIZE = 64;
    static constexpr int PenetrationOffset = 8;
    static constexpr int EntityOffset = 16;
    static constexpr int FirstVector2 = 32;

    std::uint16_t _isTrigger = 0;
    std::uint16_t ShapeUserTag = 0;
    std::int32_t _staticColliderIndex = 0;
    union {
        FP OverlapPenetration{}; // overlap queries
        FP CastDistanceNormalized;   // shape casts and ray casts, in [0, 1]
    };
    union {
        HitEntityRef _entity{}; // dynamic colliders
        AssetGuid _mapGuid;    // static colliders: the map the collider belongs to
    };
    std::int64_t sqrDistance = 0; // scratch key used by HitCollection::Sort
    FPVector2 Point;
    FPVector2 Normal;

    // 0x7d11440
    void SetIsTrigger(bool value) { _isTrigger = value ? 1 : 0; }

    // 0x7d1144c
    std::int32_t GetHashCode() const;

    // Static-collider accessors (IsDynamic, Entity, StaticColliderIndex, TryGetStaticCollider)
    // were stripped from this build's 2D Hit or need the frame/map; see Hit3D.
};
static_assert(sizeof(Hit) == Hit::SIZE);

// A growable buffer of hits allocated from the frame (temporary or persistent memory).
struct HitCollection {
    enum Modes : std::int32_t {
        None = 0,
        Temp = 1,
        Persistent = 2,
    };

    Modes _mode = None;
    Hit* _hits = nullptr;
    std::int32_t _count = 0;
    std::int32_t _capacity = 0;

    std::int32_t Count() const { return _count; }          // 0x7d11508
    void SetCount(std::int32_t value) { _count = value; }  // 0x7d11510
    Hit* HitsBuffer() const { return _hits; }              // 0x7d11518

    // 0x7d1157c - orders the hits by squared distance from `point` (nearest first). The
    // distances are cached in each hit's sqrDistance.
    void Sort(FPVector2 point);
    // 0x7d11744 - orders the hits by CastDistanceNormalized (nearest first).
    void SortCastDistance();

    // 0x7d115f4 - Lomuto quicksort on sqrDistance over [lo, hi]; not stable.
    static void QuickSort(Hit* hits, std::int32_t lo, std::int32_t hi);
    // 0x7d11764 - the same on CastDistanceNormalized.
    static void QuickSortCastDistance(Hit* hits, std::int32_t lo, std::int32_t hi);

    // Need FrameContext / Allocator:
    // TODO(0x7d11520, Quantum.Physics2D.HitCollection..ctor): allocates capacity hits
    // (temp memory for Temp mode, the allocator otherwise).
    // TODO(0x7d118b4, Quantum.Physics2D.HitCollection.Free): frees persistent buffers.
    // TODO(0x7d118fc, Quantum.Physics2D.HitCollection.Add): doubles the capacity when
    // full (ExpandCapacity), then appends.
    // TODO(0x7d11958, Quantum.Physics2D.HitCollection.ExpandCapacity)
};

} // namespace Quantum::Physics2D
