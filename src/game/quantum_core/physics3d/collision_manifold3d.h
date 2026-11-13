// collision_manifold3d.h - the 3D narrow phase's data: Quantum.Physics3D.ManifoldData3D,
// CollisionManifold3D, CollisionManifoldInfo3D, NarrowData3D, the small geometric helper
// structs CustomPlane / CustomLine and the compact FPVector3RawInt / FPQuaternionRawInt.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_matrix3x3.h"
#include "photon_deterministic/fp_vector3.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/physics2d/hit.h" // HitEntityRef
#include "quantum_core/shape3d.h"
#include "quantum_core/triangle_ccw.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace Quantum {
struct PhysicsCollider3D;
struct PhysicsBody3D;
struct PhysicsJoints3D;
struct PhysicsCallbacks3D;
struct PhysicsSceneSettings;
} // namespace Quantum

namespace Quantum::Physics3D {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector3;

struct Entry3D;
class Scene3D;

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

    // 0x7d7b3c0 - stores both entry indices and the triangle index; optionally refreshes
    // the cached flags from the scene.
    void SetEntries(Scene3D* scene, std::int32_t entryA, std::int32_t entryB, std::int32_t tri,
                    bool updateCachedState);
    // 0x7d7b3d8 - IsDynamic = both entries are entities; UsesCcd = either (or its compound
    // root) uses CCD.
    void UpdateCachedState(const Scene3D* scene);
    // 0x7d7b4a4 / 0x7d7b4bc
    Entry3D* GetEntryA(Scene3D* scene) const;
    Entry3D* GetEntryB(Scene3D* scene) const;

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

    // 0x7d7b5f4 / 0x7d7b60c / 0x7d7b624 - as ManifoldData3D's.
    void SetEntries(Scene3D* scene, std::int32_t entryA, std::int32_t entryB, std::int32_t tri,
                    bool updateCachedState) {
        _m.SetEntries(scene, entryA, entryB, tri, updateCachedState);
    }
    Entry3D* GetEntryA(Scene3D* scene) const { return _m.GetEntryA(scene); }
    Entry3D* GetEntryB(Scene3D* scene) const { return _m.GetEntryB(scene); }

    // 0x7d7b63c
    static FPVector3 GetContactPoint(const CollisionManifold3D* manifold, std::int32_t index);
    // 0x7d7b664 - mean of the contact points (raw sums divided by the count).
    static FPVector3 GetAverageContactPoint(const CollisionManifold3D* manifold);
    // 0x7d79098 - appends without a bounds check (callers add at most four).
    static void AddContactPoint(CollisionManifold3D* manifold, const FPVector3& contact);
};
static_assert(sizeof(CollisionManifold3D) == CollisionManifold3D::SIZE);

struct Entry3D;

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

// Quantum.Physics3D.CustomVertexTriangle: a triangle's corners (in the box's local frame
// for the box-triangle check).
struct CustomVertexTriangle {
    FPVector3 v_0;
    FPVector3 v_1;
    FPVector3 v_2;
};

// Quantum.Physics3D.CustomEdgesBox: the twelve edges of a box (fields e_00..e_11 in the
// original). Order: the four edges of the top face (y = +extent), the four of the bottom
// face, then the four vertical edges.
struct CustomEdgesBox {
    static constexpr int COUNT = 12;
    CustomLine Edges[COUNT];
};
static_assert(sizeof(CustomEdgesBox) == 0x240);

// Quantum.Physics3D.CustomPlanesBox: the six face planes of a box (fields p_0..p_5):
// +X, -X, +Y, -Y, +Z, -Z.
struct CustomPlanesBox {
    static constexpr int COUNT = 6;
    CustomPlane Planes[COUNT];
};
static_assert(sizeof(CustomPlanesBox) == 0x150);

// Per-entry pose cached for the narrow phase. For boxes it also caches the box's edges and
// face planes, both in the box's local frame and rotated into world orientation (relative
// to Position, not translated).
struct NarrowData3D {
    FPVector3 Position;
    FPVector3 RotatedRight;
    FPVector3 RotatedUp;
    FPVector3 RotatedForward;
    FPQuaternion RotationInverse;
    CustomEdgesBox BoxEdges;
    CustomPlanesBox BoxPlanes;
    CustomEdgesBox BoxEdgesRotated;
    CustomPlanesBox BoxPlanesRotated;

    // 0x7d7c128 - pose from a world position/rotation. RotationInverse is the exact inverse
    // (conjugate over the squared length). The box caches are only written for box shapes;
    // of the local planes only the non-zero components of normal and point are written
    // (the rest keep whatever the entry held, zero in practice).
    static void Init(NarrowData3D* narrow, const Entry3D* entry, const FPVector3& pos,
                     const FPQuaternion& rot);

    // 0x7d7bdd4 - pose from the entity transform: with a time step the body's motion over
    // it is applied first (linear velocity, and the yaw/pitch/roll angular step about the
    // rotated centre of mass), then the shape's local offset. `useCachedRot` reports
    // whether the shape has no local rotation. Both transform arguments are updated in
    // place, as in the original.
    static void InitFromTransform(NarrowData3D* narrow, const Entry3D* entry, FPVector3& tPos,
                                  FPQuaternion& tRot, bool& useCachedRot,
                                  std::optional<FP> deltaTime);
};
static_assert(sizeof(NarrowData3D) == 0x7a0);

// Quantum.Physics3D.IntAABB: an entry's broad-phase bounds in raw-int units.
struct IntAABB {
    std::int32_t MinX = 0;
    std::int32_t MaxX = 0;
    std::int32_t MinY = 0;
    std::int32_t MaxY = 0;
    std::int32_t MinZ = 0;
    std::int32_t MaxZ = 0;

    // 0x7d7d5a4 - the half-diagonal's length (raw): half extents in 32 bits, squares
    // shifted down one by one.
    FP::Raw GetBroadRadiusRaw() const;
};
static_assert(sizeof(IntAABB) == 24);

struct Island3D;
struct EntrySolverImpulses;
class Scene3D;

// Quantum.Physics3D.Entry3D: one collider in the 3D scene (an entity's collider or a static
// collider of the map), with the cached state the broad and narrow phases and the solver
// work on. Component pointers point into the frame's component buffers.
struct Entry3D {
    std::int32_t Index = 0;
    alignas(8) EntityRef Ref; // invalid for static colliders (8-aligned in the original)
    bool Ignore = false;
    PhysicsCollider3D* Collider3D = nullptr;
    const Shape3D* Shape = nullptr;
    PhysicsBody3D* Body3D = nullptr;
    ::Quantum::Transform3D* Transform3D = nullptr;
    PhysicsJoints3D* Joints = nullptr;
    std::int32_t JointSolverDataIndex = 0;
    PhysicsCallbacks3D* Callbacks = nullptr;
    bool HasStaticTriggerCallbacks = false;
    bool HasDynamicTriggerCallbacks = false;
    Island3D* Island = nullptr;
    bool ShouldIntegrateVelocity = false;
    bool HasDynamicBody = false;
    bool _useCCD = false;
    FP _firstTimeOfImpact;
    std::int32_t _compoundRootIndexPlusOne = 0;
    Entry3D* _compoundRoot = nullptr;
    IntAABB AABB;
    std::int32_t BucketStart = 0;
    std::int32_t BucketEnd = 0;
    std::int32_t BucketIncrement = 0;
    std::int32_t StaticColliderIndex = -1; // negative for entity colliders
    bool UseSmoothSphereMeshCollision = false;
    NarrowData3D Narrow;
    FPVector3 RotatedCenterOfMass;
    Photon::Deterministic::FPMatrix3x3 WorldInertiaTensorInverse;
    FPVector3 AccumulatedCorrection;
    EntrySolverImpulses* SolverImpulsesAdr = nullptr; // head of the solver's impulse list

    // 0x7d79dd4 - entries of entities (as opposed to static colliders).
    bool IsDynamic() const { return Ref.Raw() != 0; }
    // 0x7d79de4 - parts of a compound use their root's setting.
    bool UsesCcd() const;
    // 0x7d79e08
    std::int32_t CompoundRootIndex() const { return _compoundRootIndexPlusOne - 1; }
    // 0x7d79e14
    void SetCompoundRootIndex(std::int32_t value) { _compoundRootIndexPlusOne = value + 1; }
    Entry3D* CompoundRoot() const { return _compoundRoot; }       // 0x7d79e20
    void SetCompoundRoot(Entry3D* value) { _compoundRoot = value; } // 0x7d79e28
    // 0x7d79e30 - resolves the compound root pointer from the scene's entry array.
    void Init(Entry3D* sceneEntries);

    // Whether the collider is a trigger (helper; the original reads Collider3D directly).
    bool IsTriggerCollider() const;

    // 0x7d7ac64 - narrow-phase pose from the transform; dynamic bodies (not compound parts)
    // also refresh their world-space body settings.
    static void InitNarrowData(Entry3D* entry, const FPVector3* pos, const FPQuaternion* rot);
    // 0x7d7a3dc - rotated centre of mass and world inverse inertia tensor, from the narrow
    // phase's rotated axes or an explicit rotation.
    static void CalculateWorldBodySettings(Entry3D* entry, std::optional<FPQuaternion> rotation);
    // 0x7d7a724 - position and rotation over dt, then linear/angular drag.
    static void IntegrateVelocity(Entry3D* entry, FP dt, const PhysicsSceneSettings& settings,
                                  bool applyMinVelThreshold, bool integrateAngVel);
    // 0x7d7aa88
    static void ApplyMinVelocityIntegration(Entry3D* entry, const PhysicsSceneSettings& settings);
    // 0x7d7ab5c
    static void ComputeEntryAabb(const Shape3D* shape, const NarrowData3D* narrow, IntAABB* aabb);
    // 0x7d7adc8
    static void InitBroadAabb(Entry3D* entry, const FPVector3* pos, const FPQuaternion* rot,
                              std::optional<FP> deltaTime);
};
static_assert(sizeof(Entry3D) == 0x8d0);
static_assert(offsetof(Entry3D, Narrow) == 0xb0 && offsetof(Entry3D, SolverImpulsesAdr) == 0x8c8);

struct CollisionManifoldInfo3D {
    CollisionManifold3D* Manifold = nullptr;
    const Entry3D* EntryA = nullptr;
    const Entry3D* EntryB = nullptr;
    const TriangleCCW* Tri = nullptr; // the mesh triangle when B (or A) is a static mesh

    // 0x7d7b4d4 - swaps A and B and toggles NormalInverted. As in 2D, the original's
    // refresh of the (symmetric) cached flags from the scene is omitted.
    void InvertEntries();
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
    // 0x7d7b6f4 - three ints through the frame serializer's stream.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer);
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
    // 0x7d7b9f8 - four ints.
    static void Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer);
};
static_assert(sizeof(FPQuaternionRawInt) == FPQuaternionRawInt::SIZE);

} // namespace Quantum::Physics3D
