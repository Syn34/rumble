// physics_components.h - the physics components of quantum.core: Quantum.PhysicsBody2D/3D,
// PhysicsCollider2D/3D, PhysicsCallbacks2D/3D, PhysicsJoints2D/3D and
// CharacterController2D/3D, with the original field layouts (they live in the frame's
// component buffers and are serialised/checksummed byte for byte).
//
// Rarely large vectors of 3D bodies are stored as 32-bit raw values (FPVector3RawInt) and
// the drags as 16-bit values with 10 fractional bits (DRAG_PRECISION).
#pragma once

#include "quantum_core/builtin_assets.h"
#include "quantum_core/entity_ref.h"
#include "quantum_core/physics3d/collision_manifold3d.h" // FPVector3RawInt, FPQuaternionRawInt
#include "quantum_core/ptr.h"
#include "quantum_core/shape2d.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/transform.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_matrix3x3.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <optional>

namespace Quantum {
struct CharacterController3DMovement;
class IKCCCallbacks3D;
class CharacterController3DConfig;

using Photon::Deterministic::FPMatrix3x3;

namespace Core {
class FrameBase;
}
class FrameSerializer;

// Quantum.PhysicsBody2D/3D.PhysicsBodyStatus
enum class PhysicsBodyStatus : std::uint8_t {
    None = 0, // disabled
    Active = 1,
    Asleep = 2,
};

// Quantum.RotationFreezeFlags
enum class RotationFreezeFlags : std::uint8_t {
    None = 0,
    FreezeX = 1,
    FreezeY = 2,
    FreezeZ = 4,
    FreezeAll = 7,
};

// Quantum.CallbackFlags
enum class CallbackFlags : std::int32_t {
    None = 0,
    OnDynamicCollision = 1,
    OnDynamicCollisionEnter = 2,
    OnDynamicCollisionExit = 4,
    OnStaticCollision = 8,
    OnStaticCollisionEnter = 16,
    OnStaticCollisionExit = 32,
    OnDynamicTrigger = 64,
    OnDynamicTriggerEnter = 128,
    OnDynamicTriggerExit = 256,
    OnStaticTrigger = 512,
    OnStaticTriggerEnter = 1024,
    OnStaticTriggerExit = 2048,
};
constexpr CallbackFlags operator|(CallbackFlags a, CallbackFlags b) {
    return static_cast<CallbackFlags>(static_cast<std::int32_t>(a) | static_cast<std::int32_t>(b));
}
constexpr bool HasFlag(CallbackFlags value, CallbackFlags flag) {
    return (static_cast<std::int32_t>(value) & static_cast<std::int32_t>(flag)) != 0;
}

// Drags are stored with 10 fractional bits: FP raw >> (16 - 10).
constexpr int PhysicsDragShift = FP::FractionBits - 10;

struct PhysicsBody3D {
    static constexpr int SIZE = 184;
    static constexpr int DRAG_PRECISION = 10;
    static constexpr const char* ComponentTypeName = "PhysicsBody3D";

    // Quantum.PhysicsBody3D.ConfigFlags
    struct ConfigFlags {
        enum : std::uint8_t {
        IsKinematic = 1,
        IsAllowedToSleep = 2,
        IsAwakenedByKinematics = 4,
        ResetInertiaOnAdded = 8,
        ResetCenterOfMassOnAdded = 16,
        IsAwakenedByForces = 32,
        UseContinuousCollisionDetection = 64,
        Default = 58,
    };
    };

    std::uint8_t Config = 0; // ConfigFlags
    RotationFreezeFlags RotationFreeze = RotationFreezeFlags::None;
    PhysicsBodyStatus Status = PhysicsBodyStatus::None;
    std::uint8_t _padding1 = 0;
    std::uint16_t _drag = 0;
    std::uint16_t _angularDrag = 0;
    std::int32_t _sleepTimeRawInt = 0;
    std::int32_t _massInverseRawInt = 0; // zero: infinite mass
    std::int32_t _gravityScaleRawInt = 0;
    std::int32_t _padding2 = 0;
    Physics3D::FPVector3RawInt _inertiaTensorInverse; // diagonal, local space
    Physics3D::FPVector3RawInt _centerOfMass;
    Physics3D::FPVector3RawInt _angularImpulse;
    Physics3D::FPVector3RawInt _lastPosition;
    Physics3D::FPQuaternionRawInt _lastRotation;
    FPVector3 _force;
    FPVector3 _torque;
    FPVector3 Velocity;
    FPVector3 AngularVelocity;

    FP MassInverse() const { return FP::FromRaw(_massInverseRawInt); }
    FP GravityScale() const { return FP::FromRaw(_gravityScaleRawInt); }
    FP SleepTime() const { return FP::FromRaw(_sleepTimeRawInt); }
    FPVector3 InertiaTensorInverse() const { return FPVector3(_inertiaTensorInverse); }
    FPVector3 AngularImpulse() const { return FPVector3(_angularImpulse); }

    FPVector3 Force() const { return _force; }   // 0x7d33a8c
    FPVector3 Torque() const { return _torque; } // 0x7d33aa0
    // 0x7d33ab4
    bool Enabled() const { return Status != PhysicsBodyStatus::None; }
    // 0x7d33ac4
    bool AllowSleeping() const { return (Config & ConfigFlags::IsAllowedToSleep) != 0; }
    // 0x7d33ad0
    bool IsSleeping() const { return Status == PhysicsBodyStatus::Asleep; }
    // 0x7d33ae0
    bool UseContinuousCollisionDetection() const { return (Config & ConfigFlags::UseContinuousCollisionDetection) != 0; }
    // 0x7d33aec
    FP Drag() const { return FP::FromRaw(FP::Raw{_drag} << PhysicsDragShift); }
    // 0x7d33af8
    void SetDrag(FP value) { _drag = static_cast<std::uint16_t>(value.RawValue() >> PhysicsDragShift); }
    // 0x7d33b04
    FP AngularDrag() const { return FP::FromRaw(FP::Raw{_angularDrag} << PhysicsDragShift); }
    // 0x7d33b10
    void SetAngularDrag(FP value) {
        _angularDrag = static_cast<std::uint16_t>(value.RawValue() >> PhysicsDragShift);
    }
    // 0x7d33b1c
    FPVector3 CenterOfMass() const { return FPVector3(_centerOfMass); }
    // 0x7d33b84
    bool GetIsKinematic() const { return (Config & ConfigFlags::IsKinematic) != 0; }
    // 0x7d33b90 - making a body kinematic wakes it and clears force and torque; it can only
    // stop being kinematic if it has a finite mass.
    void SetIsKinematic(bool value);

    // 0x7d33be4 - mass <= 0 makes the body kinematic with infinite mass. A missing centre of
    // mass is zero, a missing gravity scale one.
    void InitBody(FP bodyMass, FP drag, FP angularDrag, std::uint8_t bodyConfig,
                  RotationFreezeFlags rotationFlags, std::optional<FPVector3> centerOfMass,
                  std::optional<FP> gravityScale);

    // 0x7d34318 - velocity change impulse * massInverse (wakes the body if awakened by
    // forces); with a point relative to the centre of mass also an angular impulse.
    void AddLinearImpulse(const FPVector3& amount, std::optional<FPVector3> relativePoint = {});
    // 0x7d34488 - `position` in world space.
    void AddLinearImpulseAtPosition(const FPVector3& impulse, FPVector3 position,
                                    const Transform3D* transform);
    // 0x7d34574 - turns a world position into one relative to the (rotated) centre of mass.
    void ComputeCenterOfMassOffset(FPVector3& position, const Transform3D* transform) const;
    // 0x7d34750
    void ClearForce() { _force = FPVector3::Zero(); }
    // 0x7d3475c - accumulates force (and torque about the centre of mass for a relative point).
    void AddForce(const FPVector3& amount, std::optional<FPVector3> relativePoint = {});
    // 0x7d348a4
    void AddForceAtPosition(const FPVector3& force, FPVector3 position, const Transform3D* transform);
    // 0x7d34990 - clears the torque and the accumulated angular impulse.
    void ClearTorque();
    // 0x7d349a4 - R * diag(inertiaTensorInverse) * R^T for the rotation `q`.
    void GetWorldInertiaTensorInverse(const FPQuaternion& q, FPMatrix3x3& m) const;
    // 0x7d34ba0
    void WakeUpInternal();
    // 0x7d34bbc - also restarts the sleep timer of an awake body.
    void WakeUp();
    // 0x7d34bdc
    void Sleep();

    // 0x7d350e8 - applies ResetCenterOfMassOnAdded / ResetInertiaOnAdded.
    static void OnAdded(Core::FrameBase* f, EntityRef entity, void* component);
    // 0x7d35148 - nothing to release.
    static void OnRemoved(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* /*component*/) {}
    // 0x7d34234 - centre of mass = the collider shape's local position.
    void ResetCenterOfMass(Core::FrameBase* f, EntityRef entity);

    // 0x7d33d60 - inverse inertia from the collider shape's inertia factor and the mass.
    void ResetInertia(Core::FrameBase* f, EntityRef entity);
    // 0x7d34bf4 - every field in declaration order.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PhysicsBody3D) == PhysicsBody3D::SIZE);

struct PhysicsBody2D {
    static constexpr int SIZE = 120;
    static constexpr int DRAG_PRECISION = 10;
    static constexpr const char* ComponentTypeName = "PhysicsBody2D";

    // Quantum.PhysicsBody2D.ConfigFlags
    struct ConfigFlags {
        enum : std::uint8_t {
        IsKinematic = 1,
        FreezeRotation = 2,
        IsAllowedToSleep = 4,
        IsAwakenedByKinematics = 8,
        ResetInertiaOnAdded = 16,
        ResetCenterOfMassOnAdded = 32,
        IsAwakenedByForces = 64,
        UseContinuousCollisionDetection = 128,
        Default = 116,
    };
    };

    std::uint8_t Config = 0; // ConfigFlags
    PhysicsBodyStatus Status = PhysicsBodyStatus::None;
    std::int16_t _padding = 0;
    std::uint16_t _drag = 0;
    std::uint16_t _angularDrag = 0;
    std::int32_t _massInverseRawInt = 0;
    std::int32_t _gravityScaleRawInt = 0;
    FP InertiaInverse;
    FP AngularVelocity;
    FP _torque;
    FP SleepTime;
    FP LastRotation;
    FPVector2 Velocity;
    FPVector2 _force;
    FPVector2 LastPosition;
    FPVector2 CenterOfMass;

    FP MassInverse() const { return FP::FromRaw(_massInverseRawInt); }
    FP GravityScale() const { return FP::FromRaw(_gravityScaleRawInt); }

    FPVector2 Force() const { return _force; } // 0x7d30e80
    FP Torque() const { return _torque; }      // 0x7d30e8c
    // 0x7d30e94
    bool Enabled() const { return Status != PhysicsBodyStatus::None; }
    // 0x7d30ea4
    bool IsSleeping() const { return Status == PhysicsBodyStatus::Asleep; }
    // 0x7d30eb4
    FP Drag() const { return FP::FromRaw(FP::Raw{_drag} << PhysicsDragShift); }
    // 0x7d30ec0
    void SetDrag(FP value) { _drag = static_cast<std::uint16_t>(value.RawValue() >> PhysicsDragShift); }
    // 0x7d30ecc
    FP AngularDrag() const { return FP::FromRaw(FP::Raw{_angularDrag} << PhysicsDragShift); }
    // 0x7d30ed8
    void SetAngularDrag(FP value) {
        _angularDrag = static_cast<std::uint16_t>(value.RawValue() >> PhysicsDragShift);
    }
    // 0x7d30ee4
    bool UseContinuousCollisionDetection() const { return (Config & ConfigFlags::UseContinuousCollisionDetection) != 0; }
    // 0x7d30ef0 - 1 / massInverse (zero for infinite mass).
    FP Mass() const;
    // 0x7d30f10
    bool GetIsKinematic() const { return (Config & ConfigFlags::IsKinematic) != 0; }
    // 0x7d30f1c
    void SetIsKinematic(bool value);
    // 0x7d30f84
    void InitBody(FP bodyMass, FP drag, FP angularDrag, std::uint8_t bodyConfig,
                  std::optional<FPVector2> centerOfMass, std::optional<FP> gravityScale);
    // 0x7d3158c
    void ClearForce() { _force = FPVector2::Zero(); }
    // 0x7d31594
    void ClearTorque() { _torque = FP::Zero(); }
    // 0x7d30f68
    void WakeUpInternal();

    // 0x7d31958 - applies ResetCenterOfMassOnAdded / ResetInertiaOnAdded.
    static void OnAdded(Core::FrameBase* f, EntityRef entity, void* component);
    // 0x7d319b8
    static void OnRemoved(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* /*component*/) {}
    // 0x7d314fc - centre of mass = the collider shape's local position.
    void ResetCenterOfMass(Core::FrameBase* f, EntityRef entity);

    // TODO(0x7d310a8, Quantum.PhysicsBody2D.ResetInertia): needs Shape2D inertia factors.
    void ResetInertia(Core::FrameBase* f, EntityRef entity);
    // 0x7d3159c - every field in declaration order.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PhysicsBody2D) == PhysicsBody2D::SIZE);

struct PhysicsCollider3D {
    static constexpr int SIZE = 112;
    static constexpr const char* ComponentTypeName = "PhysicsCollider3D";

    std::uint8_t _isTrigger = 0;
    std::uint8_t _disabled = 0;
    std::int16_t _padding = 0;
    std::int32_t _layer = 0;
    AssetRefPhysicsMaterial _material;
    Shape3D Shape;

    std::int32_t Layer() const { return _layer; } // 0x7d357a8
    // 0x7d357b0 - clamped to 0..31.
    void SetLayer(std::int32_t value) { _layer = value < 0 ? 0 : (value > 31 ? 31 : value); }
    // 0x7d357d8
    bool IsTrigger() const { return _isTrigger == 1; }
    // 0x7d357e8
    void SetIsTrigger(bool value) { _isTrigger = value ? 1 : 0; }
    AssetRefPhysicsMaterial Material() const { return _material; }        // 0x7d357f0
    void SetMaterial(AssetRefPhysicsMaterial value) { _material = value; } // 0x7d357f8
    // 0x7d35800
    bool Enabled() const { return _disabled == 0; }
    // 0x7d35810
    void SetEnabled(bool value) { _disabled = value ? 0 : 1; }

    // 0x7d3581c - an enabled collider with the shape, the layer clamped to 0..31 and the
    // material's guid (the default material, guid 1, without one). A compound shape would
    // get its own persistent copy of the children (CompoundShape3D.CopyFrom, not
    // reimplemented yet: throws std::logic_error).
    static PhysicsCollider3D Create(Core::FrameBase* f, const Shape3D& shape,
                                    const ::Quantum::PhysicsMaterial* material = nullptr, bool isTrigger = false,
                                    std::int32_t layer = 0);
    // 0x7d35bf0 - the flag bytes, padding, layer, material and shape.
    static void Serialize(void* ptr, FrameSerializer* serializer);
    // 0x7d36068
    static void OnAdded(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* /*component*/) {}
    // 0x7d3606c - a persistent compound shape gives its children back
    // (TODO(0x7d36098, Quantum.Shape3D.CompoundShape3D.FreePersistent)); nothing for other
    // shapes.
    static void OnRemoved(Core::FrameBase* f, EntityRef entity, void* component);
};
static_assert(sizeof(PhysicsCollider3D) == PhysicsCollider3D::SIZE);

struct PhysicsCollider2D {
    static constexpr int SIZE = 80;
    static constexpr const char* ComponentTypeName = "PhysicsCollider2D";

    std::uint8_t _isTrigger = 0;
    std::uint8_t _disabled = 0;
    std::int16_t _padding = 0;
    std::int32_t _layer = 0;
    AssetRefPhysicsMaterial _material;
    Shape2D Shape;

    std::int32_t Layer() const { return _layer; } // 0x7d32018
    // 0x7d32020 - clamped to 0..31.
    void SetLayer(std::int32_t value) { _layer = value < 0 ? 0 : (value > 31 ? 31 : value); }
    // 0x7d32048
    bool IsTrigger() const { return _isTrigger == 1; }
    // 0x7d32058
    void SetIsTrigger(bool value) { _isTrigger = value ? 1 : 0; }
    AssetRefPhysicsMaterial Material() const { return _material; } // 0x7d32060
    // 0x7d32068
    bool Enabled() const { return _disabled == 0; }

    // TODO(0x7d32078, Quantum.PhysicsCollider2D.Create)
    // TODO(0x7d32418, Quantum.PhysicsCollider2D.Serialize): needs Shape2D.Serialize.
    // 0x7d3285c
    static void OnAdded(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* /*component*/) {}
    // TODO(0x7d32860, Quantum.PhysicsCollider2D.OnRemoved): frees a persistent compound.
};
static_assert(sizeof(PhysicsCollider2D) == PhysicsCollider2D::SIZE);

// An entry of a PhysicsCallbacks collision list (frame heap): the other entity, whether it
// was seen this step, and for static colliders the static collider index.
struct PhysicsCallbacksEntry {
    enum Flags : std::int32_t {
        IsActive = 1,
        IsStatic = 2,
        IsTrigger = 4,
    };
    std::int32_t _flags = 0;
    std::int32_t StaticColliderIndex = 0;
    EntityRef Other;

    // 0x7d302cc (Quantum.CollisionCallbackInfo.Serialize) - flags, static index, then the
    // other side as one 64-bit value (an entity, or the map's guid for static colliders).
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PhysicsCallbacksEntry) == 16);

// Shared body of PhysicsCallbacks2D and PhysicsCallbacks3D (identical in the original).
struct PhysicsCallbacksData {
    static constexpr int SIZE = 16;
    static constexpr int DEFAULT_LIST_CAPACITY = 4;

    CallbackFlags Flags = CallbackFlags::None;
    Ptr _listBufferPtr;
    std::int32_t _listCount = 0;
    std::int32_t _listCapacity = 0;

    std::int32_t ListCount() const { return _listCount; }
    void SetListCount(std::int32_t value) { _listCount = value; }
    Ptr ListBufferPtr() const { return _listBufferPtr; }

    // Grows the list (doubling, starting at 4) when full; returns the buffer.
    PhysicsCallbacksEntry* EnsureListCapacity(Core::FrameBase* frame);
    PhysicsCallbacksEntry* Entries(Core::FrameBase* frame) const;
    bool IsCollidingWith(Core::FrameBase* frame, EntityRef other, std::int32_t& index) const;
    bool IsCollidingWith(Core::FrameBase* frame, EntityRef other, std::int32_t staticColliderIndex,
                         std::int32_t& index) const;
    void AddCollision(Core::FrameBase* frame, EntityRef other);
    void AddCollision(Core::FrameBase* frame, EntityRef other, std::int32_t staticColliderIndex,
                      bool isTrigger);
    // Throws std::out_of_range for an index past the list (IndexOutOfRangeException).
    void SetEntryIsActive(Core::FrameBase* frame, std::int32_t index, bool active);
    void FreeList(Core::FrameBase* frame);
};

struct PhysicsCallbacks3D : PhysicsCallbacksData {
    static constexpr const char* ComponentTypeName = "PhysicsCallbacks3D";
    // 0x7d35154 EnsureListCapacity, 0x7d352a8 / 0x7d35318 IsCollidingWith, 0x7d35398 /
    // 0x7d353d4 AddCollision, 0x7d3542c SetEntryIsActive, 0x7d35518 get_ListCount,
    // 0x7d35520 set_ListCount, 0x7d35528 get_ListBufferPtr: see PhysicsCallbacksData.
    // 0x7d35728 - starts with an empty list (component zeroed).
    static void OnAdded(Core::FrameBase* f, EntityRef entity, void* component);
    // 0x7d35730 - frees the list.
    static void OnRemoved(Core::FrameBase* f, EntityRef entity, void* component);
    // 0x7d35530 - flags, count and capacity, then the list buffer (by pointer key; its
    // entries are written the first time the buffer is met).
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(PhysicsCallbacks3D) == PhysicsCallbacksData::SIZE);

struct PhysicsCallbacks2D : PhysicsCallbacksData {
    static constexpr const char* ComponentTypeName = "PhysicsCallbacks2D";
    // 0x7d319c4 EnsureListCapacity, 0x7d31b18 / 0x7d31b88 IsCollidingWith, 0x7d31c08 /
    // 0x7d31c44 AddCollision, 0x7d31c9c SetEntryIsActive, 0x7d31d88 get_ListCount,
    // 0x7d31d90 set_ListCount, 0x7d31d98 get_ListBufferPtr: see PhysicsCallbacksData.
    // 0x7d31f98
    static void OnAdded(Core::FrameBase* f, EntityRef entity, void* component);
    // 0x7d31fa0
    static void OnRemoved(Core::FrameBase* f, EntityRef entity, void* component);
    // TODO(0x7d31da0, Quantum.PhysicsCallbacks2D.Serialize)
};
static_assert(sizeof(PhysicsCallbacks2D) == PhysicsCallbacksData::SIZE);

// Joint lists (frame heap). Joints themselves are not reimplemented yet.
struct PhysicsJoints3D {
    static constexpr int SIZE = 8;
    static constexpr int DEFAULT_CAPACITY = 2;
    static constexpr const char* ComponentTypeName = "PhysicsJoints3D";
    Ptr _jointsBuffer;
    std::uint16_t _count = 0;
    std::uint16_t _capacity = 0;
    // 0x7d2dc68
    std::int32_t Count() const { return _count; }
    // TODO(0x7d2dc70, Quantum.PhysicsJoints3D.AddJoint), TODO(0x7d2dcac,
    // Quantum.PhysicsJoints3D.EnsureCapacity), TODO(0x7d2de74, Quantum.PhysicsJoints3D.GetJoints),
    // TODO(0x7d2def8, Quantum.PhysicsJoints3D.Serialize), TODO(0x7d2e090,
    // Quantum.PhysicsJoints3D.OnRemoved): need Joint3D.
};
static_assert(sizeof(PhysicsJoints3D) == PhysicsJoints3D::SIZE);

struct PhysicsJoints2D {
    static constexpr int SIZE = 8;
    static constexpr int DEFAULT_CAPACITY = 2;
    static constexpr const char* ComponentTypeName = "PhysicsJoints2D";
    Ptr _jointsBuffer;
    std::uint16_t _count = 0;
    std::uint16_t _capacity = 0;
    // 0x7d2bf68
    std::int32_t Count() const { return _count; }
    // TODO(0x7d2bf70, Quantum.PhysicsJoints2D.AddJoint), TODO(0x7d2bfac,
    // Quantum.PhysicsJoints2D.EnsureCapacity), TODO(0x7d2c174, Quantum.PhysicsJoints2D.GetJoints),
    // TODO(0x7d2c1f8, Quantum.PhysicsJoints2D.Serialize), TODO(0x7d2c390,
    // Quantum.PhysicsJoints2D.OnRemoved): need Joint.
};
static_assert(sizeof(PhysicsJoints2D) == PhysicsJoints2D::SIZE);

struct CharacterController3D {
    static constexpr int SIZE = 48;
    static constexpr const char* ComponentTypeName = "CharacterController3D";
    std::uint8_t _grounded = 0;
    std::uint8_t _jumped = 0;
    std::int16_t _padding = 0;
    std::int32_t _broadphaseQueryIndexPlusOne = 0;
    AssetRefCharacterController3DConfig _config;
    FP MaxSpeed;
    FPVector3 Velocity;

    AssetRefCharacterController3DConfig Config() const { return _config; } // 0x7d2d140
    bool Grounded() const { return _grounded == 1; }                        // 0x7d2d148
    void SetGrounded(bool value) { _grounded = value ? 1 : 0; }             // 0x7d2d158
    bool Jumped() const { return _jumped == 1; }                            // 0x7d2d160
    void SetJumped(bool value) { _jumped = value ? 1 : 0; }                 // 0x7d2d170
    // 0x7d2d178 - -1 when no query is registered.
    std::int32_t BroadPhaseQueryIndex() const { return _broadphaseQueryIndexPlusOne - 1; }
    // 0x7d2d184 - negative values clear it.
    void SetBroadPhaseQueryIndex(std::int32_t value) {
        _broadphaseQueryIndexPlusOne = value < 0 ? 0 : value + 1;
    }
    // 0x7d2d194 / 0x7d2d1e0 - not grounded, at rest, with the config (the default one when
    // null).
    void Init(Core::FrameBase* frame, const CharacterController3DConfig* config);
    // 0x7d2d1f0 / 0x7d2d2c0 - stores the config's reference and its MaxSpeed.
    void SetConfig(Core::FrameBase* frame, const CharacterController3DConfig* config);
    // 0x7d2d300 - one step: movement from the contacts, steering, then the transform moves
    // by velocity * dt plus the penetration correction. Empty optionals use the config's
    // values and the frame's delta time.
    void Move(Core::FrameBase* frame, EntityRef entity, const FPVector3& direction,
              IKCCCallbacks3D* callback = nullptr, std::optional<std::int32_t> layerMask = {},
              std::optional<bool> useManifoldNormal = {}, std::optional<FP> deltaTime = {});
    // 0x7d2c40c - overlaps the config's sphere (radius + extent) and classifies the hits.
    static CharacterController3DMovement ComputeRawMovement(Core::FrameBase* frame, EntityRef entity,
                                                            const Transform3D* transform,
                                                            CharacterController3D* kcc,
                                                            const FPVector3& direction,
                                                            IKCCCallbacks3D* callback,
                                                            std::optional<std::int32_t> layerMask,
                                                            std::optional<bool> useManifoldNormal);
    // 0x7d2d56c - velocity from the movement type (gravity, acceleration, braking, limits).
    void ComputeRawSteer(Core::FrameBase* frame, const CharacterController3DMovement& movement, FP dt);
    // 0x7d2da44 - flag bytes, padding, query index, config, speed and velocity.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(CharacterController3D) == CharacterController3D::SIZE);

struct CharacterController2D {
    static constexpr int SIZE = 40;
    static constexpr const char* ComponentTypeName = "CharacterController2D";
    std::uint8_t _grounded = 0;
    std::uint8_t _jumped = 0;
    std::int16_t _padding = 0;
    std::int32_t _broadphaseQueryIndexPlusOne = 0;
    AssetRefCharacterController2DConfig _config;
    FP MaxSpeed;
    FPVector2 Velocity;

    void SetGrounded(bool value) { _grounded = value ? 1 : 0; } // 0x7d2bc18
    // TODO(0x7d2bc20, Quantum.CharacterController2D.Init) and 0x7d2bc68.
    // TODO(0x7d2bc74, Quantum.CharacterController2D.SetConfig)
    // TODO(0x7d2bd44, Quantum.CharacterController2D.Serialize)
};
static_assert(sizeof(CharacterController2D) == CharacterController2D::SIZE);

} // namespace Quantum
