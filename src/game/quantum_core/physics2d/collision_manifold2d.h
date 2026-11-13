// collision_manifold2d.h - the 2D narrow phase's data: Quantum.Physics2D.ManifoldData2D,
// CollisionManifold2D, CollisionManifoldInfo2D and NarrowData2D.
//
// A manifold describes one touching pair of broad-phase entries: which entries, the
// contact normal (pointing from entry A towards entry B), the penetration depth and up
// to two contact points in world space.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "quantum_core/polygon_collider.h"
#include "quantum_core/shape2d.h"

#include <cstdint>

namespace Quantum::Physics2D {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;

// Shared part of collision and trigger manifolds.
struct ManifoldData2D {
    static constexpr int SIZE = 24;

    enum Flags : std::int32_t {
        Ignore = 1,
        IsDynamic = 2,
        IsTrigger = 4,
        UsesCcd = 8,
    };

    std::int32_t _flags = 0;
    std::int32_t _entryIdxA = 0;
    std::int32_t _entryIdxB = 0;
    std::int32_t _padding = 0;
    FP TimeOfImpact;

    std::int32_t EntryIndexA() const { return _entryIdxA; } // 0x7d18c00
    std::int32_t EntryIndexB() const { return _entryIdxB; } // 0x7d18c08

    bool GetIgnore() const { return HasFlag(Ignore); }        // 0x7d18c10
    void SetIgnore(bool value) { SetFlag(Ignore, value); }    // 0x7d18c1c
    bool GetIsDynamic() const { return HasFlag(IsDynamic); }  // 0x7d18c2c
    void SetIsDynamic(bool value) { SetFlag(IsDynamic, value); } // 0x7d18c38
    bool GetIsTrigger() const { return HasFlag(IsTrigger); }  // 0x7d18c58
    void SetIsTrigger(bool value) { SetFlag(IsTrigger, value); } // 0x7d18c64
    bool GetUsesCcd() const { return HasFlag(UsesCcd); }      // 0x7d18c84
    void SetUsesCcd(bool value) { SetFlag(UsesCcd, value); }  // 0x7d18c90

    // Scene-dependent (entry storage of Scene2D, out of scope here):
    // TODO(0x7d18cb0, Quantum.Physics2D.ManifoldData2D.SetEntries): stores both indices
    // and optionally calls UpdateCachedState.
    // TODO(0x7d18cc4, Quantum.Physics2D.ManifoldData2D.UpdateCachedState): IsDynamic =
    // both entries dynamic; UsesCcd = either entry (or its compound root) uses CCD.
    // TODO(0x7d18d90, Quantum.Physics2D.ManifoldData2D.GetEntryA) /
    // TODO(0x7d18da8, Quantum.Physics2D.ManifoldData2D.GetEntryB): scene entry lookup.

    bool HasFlag(Flags flag) const { return (_flags & flag) != 0; }
    void SetFlag(Flags flag, bool value) { _flags = value ? (_flags | flag) : (_flags & ~flag); }
};
static_assert(sizeof(ManifoldData2D) == ManifoldData2D::SIZE);

struct CollisionManifold2D {
    static constexpr int MAX_CONTACT_POINTS = 2;
    static constexpr int SIZE = 120;

    enum Flags : std::int32_t {
        Solved = 1,
        NormalInverted = 2,
    };

    ManifoldData2D _m;
    std::int32_t _flags = 0;
    std::int32_t _contactPointsCount = 0;
    std::int32_t _constraintDataIdx = 0;
    FP Penetration;
    FP CombinedRestitution;
    FP CombinedStaticFriction;
    FP CombinedDynamicFriction;
    FPVector2 Normal; // from entry A towards entry B
    FPVector2 _contactPoints[MAX_CONTACT_POINTS];

    std::int32_t EntryIndexA() const { return _m._entryIdxA; } // 0x7d18e10
    std::int32_t EntryIndexB() const { return _m._entryIdxB; } // 0x7d18e18
    bool GetIgnore() const { return _m.GetIgnore(); }           // 0x7d18e20
    void SetIgnore(bool value) { _m.SetIgnore(value); }         // 0x7d18e2c
    bool GetIsDynamic() const { return _m.GetIsDynamic(); }     // 0x7d18e3c
    bool GetUsesCcd() const { return _m.GetUsesCcd(); }         // 0x7d18e48
    FP GetTimeOfImpact() const { return _m.TimeOfImpact; }      // 0x7d18e54
    void SetTimeOfImpact(FP value) { _m.TimeOfImpact = value; } // 0x7d18e5c
    // 0x7d18e64
    void SetSolved(bool value) { _flags = (_flags & ~Solved) | (value ? Solved : 0); }
    // 0x7d18e74
    bool GetNormalInverted() const { return (_flags & NormalInverted) != 0; }
    // 0x7d18e80
    void SetNormalInverted(bool value) {
        _flags = (_flags & ~NormalInverted) | (value ? NormalInverted : 0);
    }
    std::int32_t ContactPointsCount() const { return _contactPointsCount; }   // 0x7d18ea0
    std::int32_t GetConstraintDataIndex() const { return _constraintDataIdx; } // 0x7d18ea8
    void SetConstraintDataIndex(std::int32_t value) { _constraintDataIdx = value; } // 0x7d18eb0

    // TODO(0x7d18eb8, Quantum.Physics2D.CollisionManifold2D.SetEntries),
    // TODO(0x7d18ecc, Quantum.Physics2D.CollisionManifold2D.GetEntryA),
    // TODO(0x7d18ee4, Quantum.Physics2D.CollisionManifold2D.GetEntryB): scene-dependent,
    // as for ManifoldData2D.

    // 0x7d18efc
    static FPVector2 GetContactPoint(const CollisionManifold2D* manifold, std::int32_t index);
    // 0x7d18f14 - mean of the contact points (raw sums divided by the count).
    static FPVector2 GetAverageContactPoint(const CollisionManifold2D* manifold);
    // 0x7d18f84 - appends without a bounds check (callers add at most two).
    static void AddContactPoint(CollisionManifold2D* manifold, const FPVector2& contact);
};
static_assert(sizeof(CollisionManifold2D) == CollisionManifold2D::SIZE);

// Per-entry pose cached for the narrow phase: position and the sine/cosine of the
// rotation (so checks don't evaluate trigonometry).
struct NarrowData2D {
    static constexpr int SIZE = 32;
    FPVector2 Position;
    FP RotSin;
    FP RotCos;

    // TODO(0x7d18fac, Quantum.Physics2D.NarrowData2D.InitFromTransform): fills the pose
    // from a transform, extrapolated by the body's velocity for CCD; needs Entry2D and
    // PhysicsBody2D.
};
static_assert(sizeof(NarrowData2D) == NarrowData2D::SIZE);

// Quantum.Physics2D.Entry2D.PolygonData (inside the entry): the polygon's vertices and
// edge normals in local space.
struct PolygonData {
    const FPVector2* Vertices = nullptr;
    const FPVector2* Normals = nullptr;
    std::int32_t VerticesCount = 0;
    std::int32_t NormalsCount = 0;

    // 0x7d18774 - points at the collider asset's vertex and normal arrays (the original
    // uses the asset's native copies of them).
    void Init(const PolygonCollider& polygonCollider);
};

// The parts of the entry's PhysicsBody2D component that the narrow phase reads (the
// component itself belongs to the ECS, which is not part of this module).
struct NarrowBody2D {
    FP AngularVelocity;
    FPVector2 Velocity;
};

// The fields of a broad-phase entry (Quantum.Physics2D.Entry2D) that the narrow-phase
// checks read. The scene and its entries are not reimplemented yet; whoever builds them
// fills this view (the original reads Entry2D.Shape, Polygon, Narrow, Body, Transform2D,
// HasDynamicBody and RotatedCenterOfMass).
struct NarrowEntry2D {
    const Shape2D* Shape = nullptr;
    PolygonData Polygon;
    NarrowData2D Narrow;
    // Only read when a query extrapolates motion over a time step (CCD / time of impact).
    const NarrowBody2D* Body = nullptr;
    FPVector2 TransformPosition; // Entry2D.Transform2D->Position
    bool HasDynamicBody = false;
    FPVector2 RotatedCenterOfMass;
};

// The pair handed to every narrow-phase check.
struct CollisionManifoldInfo2D {
    CollisionManifold2D* Manifold = nullptr;
    const NarrowEntry2D* EntryA = nullptr;
    const NarrowEntry2D* EntryB = nullptr;

    // 0x7d18dc0
    CollisionManifoldInfo2D(CollisionManifold2D* m, const NarrowEntry2D* entryA,
                            const NarrowEntry2D* entryB)
        : Manifold(m), EntryA(entryA), EntryB(entryB) {}

    // 0x7d18dcc - swaps A and B (in the manifold and here) and toggles NormalInverted, so
    // a check written for (X, Y) can run on a (Y, X) pair. The original also refreshes
    // the manifold's cached flags from the scene; those flags are symmetric in A and B,
    // so swapping cannot change them and the refresh is omitted.
    void InvertEntries();
};

} // namespace Quantum::Physics2D
