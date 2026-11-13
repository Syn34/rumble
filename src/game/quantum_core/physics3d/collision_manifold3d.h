// collision_manifold3d.h - the 3D narrow phase's data: Quantum.Physics3D.ManifoldData3D,
// CollisionManifold3D, CollisionManifoldInfo3D, NarrowData3D, the small geometric helper
// structs CustomPlane / CustomLine and the compact FPVector3RawInt / FPQuaternionRawInt.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/physics2d/hit.h" // HitEntityRef
#include "quantum_core/shape3d.h"
#include "quantum_core/triangle_ccw.h"

#include <cstdint>

namespace Quantum::Physics3D {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector3;

struct ManifoldData3D {
    static constexpr int SIZE = 32;

    enum Flags : std::int32_t {
        Ignore = 1,
        IsDynamic = 2,
        IsTrigger = 4,
        UsesCcd = 8,
    };

    std::int32_t _flags = 0;
    std::int32_t _entryIdxA = 0;
    std::int32_t _entryIdxB = 0;
    std::int32_t _triIdx = 0;     // triangle of a static mesh, when B is a mesh
    std::int32_t NextInMesh = 0;  // next manifold of the same mesh pair
    std::int32_t BucketIndex = 0;
    FP TimeOfImpact;

    std::int32_t EntryIndexA() const { return _entryIdxA; } // 0x7d7b308
    std::int32_t EntryIndexB() const { return _entryIdxB; } // 0x7d7b310
    std::int32_t TriIndex() const { return _triIdx; }       // 0x7d7b318

    bool GetIgnore() const { return HasFlag(Ignore); }           // 0x7d7b320
    void SetIgnore(bool value) { SetFlag(Ignore, value); }       // 0x7d7b32c
    bool GetIsDynamic() const { return HasFlag(IsDynamic); }     // 0x7d7b33c
    void SetIsDynamic(bool value) { SetFlag(IsDynamic, value); } // 0x7d7b348
    bool GetIsTrigger() const { return HasFlag(IsTrigger); }     // 0x7d7b368
    void SetIsTrigger(bool value) { SetFlag(IsTrigger, value); } // 0x7d7b374
    bool GetUsesCcd() const { return HasFlag(UsesCcd); }         // 0x7d7b394
    void SetUsesCcd(bool value) { SetFlag(UsesCcd, value); }     // 0x7d7b3a0

    // Scene-dependent (Scene3D entry storage, out of scope here):
    // TODO(0x7d7b3c0, Quantum.Physics3D.ManifoldData3D.SetEntries): stores both entry
    // indices and the triangle index, optionally refreshing the cached state.
    // TODO(0x7d7b3d8, Quantum.Physics3D.ManifoldData3D.UpdateCachedState): IsDynamic = both
    // entries dynamic; UsesCcd = either entry (or its compound root) uses CCD.
    // TODO(0x7d7b4a4, Quantum.Physics3D.ManifoldData3D.GetEntryA),
    // TODO(0x7d7b4bc, Quantum.Physics3D.ManifoldData3D.GetEntryB).

    bool HasFlag(Flags flag) const { return (_flags & flag) != 0; }
    void SetFlag(Flags flag, bool value) { _flags = value ? (_flags | flag) : (_flags & ~flag); }
};
static_assert(sizeof(ManifoldData3D) == ManifoldData3D::SIZE);

struct CollisionManifold3D {
    static constexpr int MAX_CONTACT_POINTS = 4;
    static constexpr int SIZE = 200;

    enum Flags : std::int32_t {
        Solved = 1,
        NormalInverted = 2,
    };

    ManifoldData3D _m;
    std::int32_t _flags = 0;
    std::int32_t _padding = 0;
    std::int32_t _contactPointsCount = 0;
    std::int32_t _constraintDataIdx = 0;
    FP Penetration;
    FP CombinedRestitution;
    FP CombinedStaticFriction;
    FP CombinedDynamicFriction;
    FPVector3 Normal; // from entry A towards entry B
    FPVector3 _contactPoints[MAX_CONTACT_POINTS];

    std::int32_t EntryIndexA() const { return _m._entryIdxA; }               // 0x7d7b518
    std::int32_t EntryIndexB() const { return _m._entryIdxB; }               // 0x7d7b520
    std::int32_t GetBucketIndex() const { return _m.BucketIndex; }           // 0x7d7b528
    void SetBucketIndex(std::int32_t value) { _m.BucketIndex = value; }      // 0x7d7b530
    std::int32_t TriIndex() const { return _m._triIdx; }                     // 0x7d7b538
    std::int32_t GetNextInMesh() const { return _m.NextInMesh; }             // 0x7d7b540
    void SetNextInMesh(std::int32_t value) { _m.NextInMesh = value; }        // 0x7d7b548
    bool GetIgnore() const { return _m.GetIgnore(); }                        // 0x7d7b550
    void SetIgnore(bool value) { _m.SetIgnore(value); }                      // 0x7d7b55c
    bool GetIsDynamic() const { return _m.GetIsDynamic(); }                  // 0x7d7b56c
    bool GetUsesCcd() const { return _m.GetUsesCcd(); }                      // 0x7d7b578
    FP GetTimeOfImpact() const { return _m.TimeOfImpact; }                   // 0x7d7b584
    void SetTimeOfImpact(FP value) { _m.TimeOfImpact = value; }              // 0x7d7b58c
    bool GetSolved() const { return (_flags & Solved) != 0; }                // 0x7d7b594
    // 0x7d7b5a0
    void SetSolved(bool value) { _flags = (_flags & ~Solved) | (value ? Solved : 0); }
    bool GetNormalInverted() const { return (_flags & NormalInverted) != 0; } // 0x7d7b5b0
    // 0x7d7b5bc
    void SetNormalInverted(bool value) {
        _flags = (_flags & ~NormalInverted) | (value ? NormalInverted : 0);
    }
    std::int32_t ContactPointsCount() const { return _contactPointsCount; }        // 0x7d7b5dc
    std::int32_t GetConstraintDataIndex() const { return _constraintDataIdx; }     // 0x7d7b5e4
    void SetConstraintDataIndex(std::int32_t value) { _constraintDataIdx = value; } // 0x7d7b5ec

    // TODO(0x7d7b5f4, Quantum.Physics3D.CollisionManifold3D.SetEntries),
    // TODO(0x7d7b60c, Quantum.Physics3D.CollisionManifold3D.GetEntryA),
    // TODO(0x7d7b624, Quantum.Physics3D.CollisionManifold3D.GetEntryB): scene-dependent.

    // 0x7d7b63c
    static FPVector3 GetContactPoint(const CollisionManifold3D* manifold, std::int32_t index);
    // 0x7d7b664 - mean of the contact points (raw sums divided by the count).
    static FPVector3 GetAverageContactPoint(const CollisionManifold3D* manifold);
    // 0x7d79098 - appends without a bounds check (callers add at most four).
    static void AddContactPoint(CollisionManifold3D* manifold, const FPVector3& contact);
};
static_assert(sizeof(CollisionManifold3D) == CollisionManifold3D::SIZE);

// Per-entry pose cached for the narrow phase. The original also caches the box's edges and
// planes (CustomEdgesBox / CustomPlanesBox, local and rotated) used by the box-box and
// box-triangle checks; those are not reproduced yet (see NarrowData3D.Init below).
// TODO(0x7d7bdd4, Quantum.Physics3D.NarrowData3D.InitFromTransform): pose extrapolated by
// the body's velocity for CCD; needs Entry3D and PhysicsBody3D.
// TODO(0x7d7c128, Quantum.Physics3D.NarrowData3D.Init): fills the rotated axes, inverse
// rotation and the box edge/plane caches.
struct NarrowData3D {
    FPVector3 Position;
    FPVector3 RotatedRight;
    FPVector3 RotatedUp;
    FPVector3 RotatedForward;
    FPQuaternion RotationInverse;
};

// The parts of the entry's PhysicsBody3D component that the narrow phase reads.
struct NarrowBody3D {
    FPVector3 Velocity;
    FPVector3 AngularVelocity;
};

// The fields of a broad-phase entry (Quantum.Physics3D.Entry3D) that the narrow-phase
// checks read; filled by the scene once it exists (original: Entry3D.Shape, Narrow, Body3D,
// Transform3D, HasDynamicBody, RotatedCenterOfMass, UseSmoothSphereMeshCollision).
struct NarrowEntry3D {
    const Shape3D* Shape = nullptr;
    NarrowData3D Narrow;
    // Only read when a query extrapolates motion over a time step (CCD / time of impact).
    const NarrowBody3D* Body = nullptr;
    FPVector3 TransformPosition; // Entry3D.Transform3D->Position
    bool HasDynamicBody = false;
    bool UseSmoothSphereMeshCollision = false;
    FPVector3 RotatedCenterOfMass;
    // Reported by queries (line casts): Entry3D.StaticColliderIndex (negative for dynamic
    // entries), Entry3D.Ref and the collider's trigger flag.
    std::int32_t StaticColliderIndex = -1;
    Physics2D::HitEntityRef Ref;
    bool IsTrigger = false;
};

struct CollisionManifoldInfo3D {
    CollisionManifold3D* Manifold = nullptr;
    const NarrowEntry3D* EntryA = nullptr;
    const NarrowEntry3D* EntryB = nullptr;
    const TriangleCCW* Tri = nullptr; // the mesh triangle when B (or A) is a static mesh

    // 0x7d7b4d4 - swaps A and B and toggles NormalInverted. As in 2D, the original's
    // refresh of the (symmetric) cached flags from the scene is omitted.
    void InvertEntries();
};

// A plane through `pointOnPlane` with unit `normal`. The distance field is left at zero by
// the constructor; the box checks fill it in when they need it.
struct CustomPlane {
    FPVector3 normal;
    FP distance;
    FPVector3 pointOnPlane;

    CustomPlane() = default;
    // 0x7d70ea0
    CustomPlane(const FPVector3& normalIn, const FPVector3& pointOnPlaneIn)
        : normal(normalIn), distance(), pointOnPlane(pointOnPlaneIn) {}
};

// A segment.
struct CustomLine {
    FPVector3 start;
    FPVector3 end;

    CustomLine() = default;
    // 0x7d70ec8
    CustomLine(const FPVector3& startIn, const FPVector3& endIn) : start(startIn), end(endIn) {}
};

// FPVector3 packed into three 32-bit raw values (physics bodies store rarely-large vectors
// this way). Conversion keeps the low 32 bits of each raw value and sign-extends back.
struct FPVector3RawInt {
    static constexpr int SIZE = 12;
    static constexpr int ALIGNMENT = 4;

    std::int32_t RawIntX = 0;
    std::int32_t RawIntY = 0;
    std::int32_t RawIntZ = 0;

    // 0x7d7b9a8
    std::int32_t GetHashCode() const;
    // 0x7d7b9cc op_Explicit(FPVector3RawInt) -> FPVector3
    explicit operator FPVector3() const;
    // 0x7d7b9e4 op_Explicit(FPVector3) -> FPVector3RawInt
    static FPVector3RawInt FromFPVector3(const FPVector3& value);
    // TODO(0x7d7b6f4, Quantum.Physics3D.FPVector3RawInt.Serialize): three ints through the
    // frame serializer's stream.
};
static_assert(sizeof(FPVector3RawInt) == FPVector3RawInt::SIZE);

struct FPQuaternionRawInt {
    static constexpr int SIZE = 16;

    std::int32_t RawIntX = 0;
    std::int32_t RawIntY = 0;
    std::int32_t RawIntZ = 0;
    std::int32_t RawIntW = 0;

    // 0x7d7bd70
    std::int32_t GetHashCode() const;
    // 0x7d7bd9c op_Explicit(FPQuaternionRawInt) -> FPQuaternion
    explicit operator FPQuaternion() const;
    // 0x7d7bdb8 op_Explicit(FPQuaternion) -> FPQuaternionRawInt
    static FPQuaternionRawInt FromFPQuaternion(const FPQuaternion& value);
    // TODO(0x7d7b9f8, Quantum.Physics3D.FPQuaternionRawInt.Serialize)
};
static_assert(sizeof(FPQuaternionRawInt) == FPQuaternionRawInt::SIZE);

} // namespace Quantum::Physics3D
