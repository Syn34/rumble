// physics_components.cpp - behaviour of the physics components (bodies, colliders,
// callbacks lists).
#include "physics_components.h"

#include "quantum_core/frame_serializer.h"

#include "quantum_core/frame_base.h"
#include "quantum_core/frame_heap.h"

#include "photon_deterministic/fp_internal.h"

#include <cassert>
#include <stdexcept>
#include <string>

namespace Quantum {

namespace {

// int32 wrapping addition, as the original's raw-int fields accumulate.
std::int32_t AddWrapped(std::int32_t a, std::int32_t b) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}

// 1 / mass as a raw int (unsigned 64-by-64 division of 2^32 by the raw mass).
std::int32_t InverseMassRaw(FP mass) {
    return static_cast<std::int32_t>((std::uint64_t{1} << 32) / static_cast<std::uint64_t>(mass.RawValue()));
}

} // namespace

// ---------------------------------------------------------------------------------------
// PhysicsBody3D
// ---------------------------------------------------------------------------------------

// 0x7d33b90
void PhysicsBody3D::SetIsKinematic(bool value) {
    if (value) {
        Config |= ConfigFlags::IsKinematic;
        WakeUpInternal();
        _force = FPVector3::Zero();
        _torque = FPVector3::Zero();
    } else if (_massInverseRawInt != 0) {
        Config &= static_cast<std::uint8_t>(~ConfigFlags::IsKinematic);
    }
}

// 0x7d33be4
void PhysicsBody3D::InitBody(FP bodyMass, FP drag, FP angularDrag, std::uint8_t bodyConfig,
                             RotationFreezeFlags rotationFlags, std::optional<FPVector3> centerOfMass,
                             std::optional<FP> gravityScale) {
    Config = bodyConfig;
    if (bodyMass > 0) {
        _massInverseRawInt = InverseMassRaw(bodyMass);
    } else {
        // Infinite mass: kinematic.
        _massInverseRawInt = 0;
        SetIsKinematic(true);
    }
    Status = PhysicsBodyStatus::Active;
    _lastRotation = Physics3D::FPQuaternionRawInt::FromFPQuaternion(FPQuaternion::Identity());
    RotationFreeze = rotationFlags;
    SetDrag(drag);
    SetAngularDrag(angularDrag);
    _centerOfMass = Physics3D::FPVector3RawInt::FromFPVector3(centerOfMass.value_or(FPVector3::Zero()));
    _gravityScaleRawInt = static_cast<std::int32_t>(gravityScale.value_or(FP::One()).RawValue());
}

// 0x7d34318
void PhysicsBody3D::AddLinearImpulse(const FPVector3& amount, std::optional<FPVector3> relativePoint) {
    if (GetIsKinematic()) {
        return;
    }
    Velocity = Velocity + amount * MassInverse();
    if ((Config & ConfigFlags::IsAwakenedByForces) != 0) {
        WakeUpInternal();
    }
    if (relativePoint.has_value()) {
        // Accumulated in the 32-bit raw angular impulse (wrapping).
        const FPVector3 torqueImpulse = FPVector3::Cross(*relativePoint, amount);
        const Physics3D::FPVector3RawInt delta = Physics3D::FPVector3RawInt::FromFPVector3(torqueImpulse);
        _angularImpulse.RawIntX = AddWrapped(_angularImpulse.RawIntX, delta.RawIntX);
        _angularImpulse.RawIntY = AddWrapped(_angularImpulse.RawIntY, delta.RawIntY);
        _angularImpulse.RawIntZ = AddWrapped(_angularImpulse.RawIntZ, delta.RawIntZ);
    }
}

// 0x7d34488
void PhysicsBody3D::AddLinearImpulseAtPosition(const FPVector3& impulse, FPVector3 position,
                                               const Transform3D* transform) {
    if (GetIsKinematic()) {
        return;
    }
    ComputeCenterOfMassOffset(position, transform);
    AddLinearImpulse(impulse, position);
}

// 0x7d34574
void PhysicsBody3D::ComputeCenterOfMassOffset(FPVector3& position, const Transform3D* transform) const {
    assert(transform != nullptr);
    position = position - transform->Position;
    const FPVector3 com = CenterOfMass();
    if (com == FPVector3::Zero()) {
        return;
    }
    if (transform->Rotation == FPQuaternion::Identity()) {
        position = position - com;
    } else {
        position = position - transform->Rotation * com;
    }
}

// 0x7d3475c
void PhysicsBody3D::AddForce(const FPVector3& amount, std::optional<FPVector3> relativePoint) {
    if (GetIsKinematic()) {
        return;
    }
    _force = amount + _force;
    if ((Config & ConfigFlags::IsAwakenedByForces) != 0) {
        WakeUpInternal();
    }
    if (relativePoint.has_value()) {
        _torque = _torque + FPVector3::Cross(*relativePoint, amount);
    }
}

// 0x7d348a4
void PhysicsBody3D::AddForceAtPosition(const FPVector3& force, FPVector3 position,
                                       const Transform3D* transform) {
    if (GetIsKinematic()) {
        return;
    }
    ComputeCenterOfMassOffset(position, transform);
    AddForce(force, position);
}

// 0x7d34990
void PhysicsBody3D::ClearTorque() {
    _angularImpulse = {};
    _torque = FPVector3::Zero();
}

// 0x7d349a4
void PhysicsBody3D::GetWorldInertiaTensorInverse(const FPQuaternion& q, FPMatrix3x3& m) const {
    // Rotation matrix columns (the doubled terms of the quaternion formula).
    const FPVector3 columns[3] = {q * FPVector3::Right(), q * FPVector3::Up(), q * FPVector3::Forward()};
    const auto r = [&columns](int row, int column) {
        const FPVector3& c = columns[column];
        return row == 0 ? c.X : (row == 1 ? c.Y : c.Z);
    };
    const FPVector3 inertia = InertiaTensorInverse();
    const FP diagonal[3] = {inertia.X, inertia.Y, inertia.Z};
    // A = R * diag(I), then M[j][i] = sum_k R[j][k] * A[i][k].
    FP a[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int k = 0; k < 3; ++k) {
            a[i][k] = r(i, k) * diagonal[k];
        }
    }
    FP result[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[j][i] = a[i][0] * r(j, 0) + a[i][1] * r(j, 1) + a[i][2] * r(j, 2);
        }
    }
    m = FPMatrix3x3::FromRows(result[0][0], result[0][1], result[0][2], result[1][0], result[1][1],
                              result[1][2], result[2][0], result[2][1], result[2][2]);
}

// 0x7d34ba0
void PhysicsBody3D::WakeUpInternal() {
    if (Status == PhysicsBodyStatus::Asleep) {
        _sleepTimeRawInt = 0;
        Status = PhysicsBodyStatus::Active;
    }
}

// 0x7d34bbc
void PhysicsBody3D::WakeUp() {
    _sleepTimeRawInt = 0;
    WakeUpInternal();
}

// 0x7d34bdc
void PhysicsBody3D::Sleep() {
    if (Status == PhysicsBodyStatus::Active) {
        Status = PhysicsBodyStatus::Asleep;
    }
}

// 0x7d350e8
void PhysicsBody3D::OnAdded(Core::FrameBase* f, EntityRef entity, void* component) {
    auto* body = static_cast<PhysicsBody3D*>(component);
    assert(body != nullptr);
    if ((body->Config & ConfigFlags::ResetCenterOfMassOnAdded) != 0) {
        body->ResetCenterOfMass(f, entity);
    }
    if ((body->Config & ConfigFlags::ResetInertiaOnAdded) != 0) {
        body->ResetInertia(f, entity);
    }
}

// 0x7d34bf4
void PhysicsBody3D::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* body = static_cast<PhysicsBody3D*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(&body->Config);
    stream.Serialize(reinterpret_cast<std::uint8_t*>(&body->RotationFreeze));
    stream.Serialize(reinterpret_cast<std::uint8_t*>(&body->Status));
    stream.Serialize(&body->_padding1);
    stream.Serialize(&body->_drag);
    stream.Serialize(&body->_angularDrag);
    stream.Serialize(&body->_sleepTimeRawInt);
    stream.Serialize(&body->_massInverseRawInt);
    stream.Serialize(&body->_gravityScaleRawInt);
    stream.Serialize(&body->_padding2);
    Physics3D::FPVector3RawInt::Serialize(&body->_inertiaTensorInverse, serializer);
    Physics3D::FPVector3RawInt::Serialize(&body->_centerOfMass, serializer);
    Physics3D::FPVector3RawInt::Serialize(&body->_angularImpulse, serializer);
    Physics3D::FPVector3RawInt::Serialize(&body->_lastPosition, serializer);
    Physics3D::FPQuaternionRawInt::Serialize(&body->_lastRotation, serializer);
    FPVector3::Serialize(&body->_force, serializer);
    FPVector3::Serialize(&body->_torque, serializer);
    FPVector3::Serialize(&body->Velocity, serializer);
    FPVector3::Serialize(&body->AngularVelocity, serializer);
}

// 0x7d34234
void PhysicsBody3D::ResetCenterOfMass(Core::FrameBase* f, EntityRef entity) {
    assert(f != nullptr);
    const PhysicsCollider3D* collider = f->GetPointer<PhysicsCollider3D>(entity);
    _centerOfMass = Physics3D::FPVector3RawInt::FromFPVector3(collider->Shape.LocalTransform.Position);
}

// 0x7d33d60 - inverse inertia = massInverse / factor per axis (zero for a non-positive
// factor, and all zero for an infinite mass).
void PhysicsBody3D::ResetInertia(Core::FrameBase* f, EntityRef entity) {
    if (_massInverseRawInt == 0) {
        _inertiaTensorInverse = {};
        return;
    }
    const PhysicsCollider3D* collider = f->GetPointer<PhysicsCollider3D>(entity);
    const FPVector3 factor = collider->Shape.CalculateInertiaTensorFactor(CenterOfMass());
    const FP::Raw scaled = FP::Raw{_massInverseRawInt} << 16;
    const auto axis = [scaled](FP value) {
        return value.RawValue() >= 1 ? static_cast<std::int32_t>(scaled / value.RawValue()) : 0;
    };
    _inertiaTensorInverse.RawIntX = axis(factor.X);
    _inertiaTensorInverse.RawIntY = axis(factor.Y);
    _inertiaTensorInverse.RawIntZ = axis(factor.Z);
}

// ---------------------------------------------------------------------------------------
// PhysicsBody2D
// ---------------------------------------------------------------------------------------

// 0x7d30ef0
FP PhysicsBody2D::Mass() const {
    if (_massInverseRawInt < 1) {
        return FP::Zero();
    }
    return FP::FromRaw(static_cast<FP::Raw>((std::uint64_t{1} << 32) /
                                            static_cast<std::uint64_t>(_massInverseRawInt)));
}

// 0x7d30f1c
void PhysicsBody2D::SetIsKinematic(bool value) {
    if (value) {
        Config |= ConfigFlags::IsKinematic;
        WakeUpInternal();
        _force = FPVector2::Zero();
        _torque = FP::Zero();
    } else if (_massInverseRawInt != 0) {
        Config &= static_cast<std::uint8_t>(~ConfigFlags::IsKinematic);
    }
}

// 0x7d30f84
void PhysicsBody2D::InitBody(FP bodyMass, FP drag, FP angularDrag, std::uint8_t bodyConfig,
                             std::optional<FPVector2> centerOfMass, std::optional<FP> gravityScale) {
    Config = bodyConfig;
    if (bodyMass > 0) {
        _massInverseRawInt = InverseMassRaw(bodyMass);
    } else {
        _massInverseRawInt = 0;
        SetIsKinematic(true);
    }
    Status = PhysicsBodyStatus::Active;
    SetDrag(drag);
    SetAngularDrag(angularDrag);
    SleepTime = FP::Zero();
    LastRotation = FP::Zero();
    CenterOfMass = centerOfMass.value_or(FPVector2::Zero());
    _gravityScaleRawInt = static_cast<std::int32_t>(gravityScale.value_or(FP::One()).RawValue());
}

// 0x7d30f68
void PhysicsBody2D::WakeUpInternal() {
    if (Status == PhysicsBodyStatus::Asleep) {
        SleepTime = FP::Zero();
        Status = PhysicsBodyStatus::Active;
    }
}

// 0x7d31958
void PhysicsBody2D::OnAdded(Core::FrameBase* f, EntityRef entity, void* component) {
    auto* body = static_cast<PhysicsBody2D*>(component);
    assert(body != nullptr);
    if ((body->Config & ConfigFlags::ResetCenterOfMassOnAdded) != 0) {
        body->ResetCenterOfMass(f, entity);
    }
    if ((body->Config & ConfigFlags::ResetInertiaOnAdded) != 0) {
        body->ResetInertia(f, entity);
    }
}

// 0x7d3159c
void PhysicsBody2D::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* body = static_cast<PhysicsBody2D*>(ptr);
    Photon::Deterministic::IBitStream& stream = serializer->Stream();
    stream.Serialize(&body->Config);
    stream.Serialize(reinterpret_cast<std::uint8_t*>(&body->Status));
    stream.Serialize(&body->_padding);
    stream.Serialize(&body->_drag);
    stream.Serialize(&body->_angularDrag);
    stream.Serialize(&body->_massInverseRawInt);
    stream.Serialize(&body->_gravityScaleRawInt);
    FP::Serialize(&body->InertiaInverse, *serializer);
    FP::Serialize(&body->AngularVelocity, *serializer);
    FP::Serialize(&body->_torque, *serializer);
    FP::Serialize(&body->SleepTime, *serializer);
    FP::Serialize(&body->LastRotation, *serializer);
    FPVector2::Serialize(&body->Velocity, serializer);
    FPVector2::Serialize(&body->_force, serializer);
    FPVector2::Serialize(&body->LastPosition, serializer);
    FPVector2::Serialize(&body->CenterOfMass, serializer);
}

// 0x7d314fc
void PhysicsBody2D::ResetCenterOfMass(Core::FrameBase* f, EntityRef entity) {
    assert(f != nullptr);
    CenterOfMass = f->GetPointer<PhysicsCollider2D>(entity)->Shape.LocalTransform.Position;
}

void PhysicsBody2D::ResetInertia(Core::FrameBase* /*f*/, EntityRef /*entity*/) {
    // See the TODO in the header; the inertia is left unchanged.
}

// ---------------------------------------------------------------------------------------
// PhysicsCallbacks2D / 3D
// ---------------------------------------------------------------------------------------

PhysicsCallbacksEntry* PhysicsCallbacksData::Entries(Core::FrameBase* frame) const {
    assert(frame != nullptr);
    return static_cast<PhysicsCallbacksEntry*>(frame->get_TrackedHeap()->Void(_listBufferPtr));
}

// 0x7d35154 (2D: 0x7d319c4)
PhysicsCallbacksEntry* PhysicsCallbacksData::EnsureListCapacity(Core::FrameBase* frame) {
    assert(frame != nullptr);
    Allocator::FrameHeap* heap = frame->get_TrackedHeap();
    if (_listCount == _listCapacity) {
        if (_listBufferPtr.Offset == 0) {
            _listCapacity = DEFAULT_LIST_CAPACITY;
            _listBufferPtr = heap->AllocateAndClearPtr(
                static_cast<int>(sizeof(PhysicsCallbacksEntry)) * DEFAULT_LIST_CAPACITY);
        } else {
            PhysicsCallbacksEntry* grown =
                heap->ExpandArray(Entries(frame), _listCount, _listCount * 2);
            _listCapacity *= 2;
            _listBufferPtr = heap->Void(grown);
        }
    }
    return Entries(frame);
}

// 0x7d352a8 (2D: 0x7d31b18)
bool PhysicsCallbacksData::IsCollidingWith(Core::FrameBase* frame, EntityRef other,
                                           std::int32_t& index) const {
    if (_listCount >= 1) {
        const PhysicsCallbacksEntry* entries = Entries(frame);
        for (std::int32_t i = 0; i < _listCount; ++i) {
            if (entries[i].Other == other) {
                index = i;
                return true;
            }
        }
    }
    index = -1;
    return false;
}

// 0x7d35318 (2D: 0x7d31b88)
bool PhysicsCallbacksData::IsCollidingWith(Core::FrameBase* frame, EntityRef other,
                                           std::int32_t staticColliderIndex, std::int32_t& index) const {
    if (_listCount >= 1) {
        const PhysicsCallbacksEntry* entries = Entries(frame);
        for (std::int32_t i = 0; i < _listCount; ++i) {
            const PhysicsCallbacksEntry& e = entries[i];
            if ((e._flags & PhysicsCallbacksEntry::IsStatic) != 0 &&
                e.StaticColliderIndex == staticColliderIndex && e.Other == other) {
                index = i;
                return true;
            }
        }
    }
    index = -1;
    return false;
}

// 0x7d35398 (2D: 0x7d31c08)
void PhysicsCallbacksData::AddCollision(Core::FrameBase* frame, EntityRef other) {
    PhysicsCallbacksEntry* entries = EnsureListCapacity(frame);
    PhysicsCallbacksEntry& e = entries[_listCount++];
    e = PhysicsCallbacksEntry{};
    e.Other = other;
    e._flags = PhysicsCallbacksEntry::IsActive;
}

// 0x7d353d4 (2D: 0x7d31c44)
void PhysicsCallbacksData::AddCollision(Core::FrameBase* frame, EntityRef other,
                                        std::int32_t staticColliderIndex, bool isTrigger) {
    PhysicsCallbacksEntry* entries = EnsureListCapacity(frame);
    PhysicsCallbacksEntry& e = entries[_listCount++];
    e.Other = other;
    e._flags = PhysicsCallbacksEntry::IsActive | PhysicsCallbacksEntry::IsStatic |
               (isTrigger ? PhysicsCallbacksEntry::IsTrigger : 0);
    e.StaticColliderIndex = staticColliderIndex;
}

// 0x7d3542c (2D: 0x7d31c9c)
void PhysicsCallbacksData::SetEntryIsActive(Core::FrameBase* frame, std::int32_t index, bool active) {
    if (index >= _listCount) {
        throw std::out_of_range("Index " + std::to_string(index) + " is out of range (count " +
                                std::to_string(_listCount) + ")");
    }
    PhysicsCallbacksEntry& e = Entries(frame)[index];
    e._flags = (e._flags & ~PhysicsCallbacksEntry::IsActive) | (active ? 1 : 0);
}

void PhysicsCallbacksData::FreeList(Core::FrameBase* frame) {
    if (_listBufferPtr.Offset != 0) {
        frame->get_TrackedHeap()->Free(_listBufferPtr);
    }
    *this = PhysicsCallbacksData{};
}

// 0x7d35728
void PhysicsCallbacks3D::OnAdded(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* component) {
    *static_cast<PhysicsCallbacksData*>(static_cast<PhysicsCallbacks3D*>(component)) = PhysicsCallbacksData{};
}

// 0x7d35730
void PhysicsCallbacks3D::OnRemoved(Core::FrameBase* f, EntityRef /*entity*/, void* component) {
    static_cast<PhysicsCallbacks3D*>(component)->FreeList(f);
}

// 0x7d31f98
void PhysicsCallbacks2D::OnAdded(Core::FrameBase* /*f*/, EntityRef /*entity*/, void* component) {
    *static_cast<PhysicsCallbacksData*>(static_cast<PhysicsCallbacks2D*>(component)) = PhysicsCallbacksData{};
}

// 0x7d31fa0
void PhysicsCallbacks2D::OnRemoved(Core::FrameBase* f, EntityRef /*entity*/, void* component) {
    static_cast<PhysicsCallbacks2D*>(component)->FreeList(f);
}

} // namespace Quantum
