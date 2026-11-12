// transform.cpp - Quantum.Transform2D / Transform2DVertical / Transform3D.
#include "transform.h"

#include "photon_deterministic/deterministic_frame_serializer.h"
#include "photon_deterministic/fp_internal.h"
#include "photon_deterministic/fp_math.h"

#include <cassert>

namespace Quantum {

namespace detail = Photon::Deterministic::detail;
namespace FPMath = Photon::Deterministic::FPMath;

namespace {

// 2 * value with the doubling applied to the raw integer, as the rotation-matrix
// terms below do it before multiplying.
constexpr FP Twice(FP value) {
    return FP::FromRaw(detail::WrapMul(value.RawValue(), 2));
}

} // namespace

// --- Transform2D ---------------------------------------------------------------------

// 0x7d3e2f4
FPVector2 Transform2D::Up() const {
    FP sin;
    FP cos;
    FPMath::SinCos(Rotation, sin, cos);
    return {-sin, cos};
}

// 0x7d3e3e8
std::int32_t Transform2D::GetHashCode() const {
    std::int32_t hash = detail::HashCombine(detail::HashSeed, Position.GetHashCode());
    return detail::HashCombine(hash, Rotation.GetHashCode());
}

// 0x7d3e3b0
void Transform2D::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* transform = static_cast<Transform2D*>(ptr);
    assert(transform != nullptr && serializer != nullptr);
    FPVector2::Serialize(&transform->Position, serializer);
    FP::Serialize(&transform->Rotation, *serializer);
}

// --- Transform2DVertical -------------------------------------------------------------

// 0x7d3c8b4
void Transform2DVertical::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* transform = static_cast<Transform2DVertical*>(ptr);
    assert(transform != nullptr && serializer != nullptr);
    FP::Serialize(&transform->Height, *serializer);
    FP::Serialize(&transform->Position, *serializer);
}

// --- Transform3D ---------------------------------------------------------------------

// 0x7d3c8f0
FPVector3 Transform3D::EulerAngles() const {
    return Rotation.AsEuler();
}

// 0x7d3c950
FPVector3 Transform3D::InverseTransformDirection(const FPVector3& direction) const {
    return FPQuaternion::Conjugate(Rotation) * direction;
}

// 0x7d3c9e4
FPVector3 Transform3D::TransformDirection(const FPVector3& direction) const {
    return Rotation * direction;
}

// 0x7d3ca64
FPVector3 Transform3D::InverseTransformPoint(const FPVector3& position) const {
    return FPQuaternion::Conjugate(Rotation) * (position - Position);
}

// 0x7d3cb14
FPVector3 Transform3D::TransformPoint(const FPVector3& position) const {
    return Position + Rotation * position;
}

// 0x7d3cba8
void Transform3D::Rotate(FP xAngle, FP yAngle, FP zAngle, bool normalize) {
    Rotation = Rotation * FPQuaternion::Euler(xAngle, yAngle, zAngle);
    if (normalize) {
        Rotation = FPQuaternion::Normalize(Rotation);
    }
}

// 0x7d3cc6c
void Transform3D::Rotate(const FPVector3& axis, FP angle, bool normalize) {
    // The original also computes Rotation * axis here and discards it (no effect).
    Rotation = Rotation * FPQuaternion::AngleAxis(angle, axis);
    if (normalize) {
        Rotation = FPQuaternion::Normalize(Rotation);
    }
}

// 0x7d3cd84
FPVector3 Transform3D::Right() const {
    const FPQuaternion& q = Rotation;
    return {FP::One() - (Twice(q.Y) * q.Y + Twice(q.Z) * q.Z), q.W * Twice(q.Z) + q.X * Twice(q.Y),
            Twice(q.Z) * q.X - q.W * Twice(q.Y)};
}

// 0x7d3cdd8
FPVector3 Transform3D::Up() const {
    const FPQuaternion& q = Rotation;
    return {Twice(q.X) * q.Y - Twice(q.Z) * q.W, FP::One() - (Twice(q.X) * q.X + Twice(q.Z) * q.Z),
            q.W * Twice(q.X) + Twice(q.Z) * q.Y};
}

// 0x7d3ce2c
FPVector3 Transform3D::Forward() const {
    const FPQuaternion& q = Rotation;
    return {q.W * Twice(q.Y) + Twice(q.Z) * q.X, Twice(q.Z) * q.Y - q.W * Twice(q.X),
            FP::One() - (Twice(q.X) * q.X + Twice(q.Y) * q.Y)};
}

// 0x7d3ce84
FPVector3 Transform3D::Left() const {
    return -Right();
}

// 0x7d3ced8
FPVector3 Transform3D::Back() const {
    return -Forward();
}

// 0x7d3cf6c
std::int32_t Transform3D::GetHashCode() const {
    std::int32_t hash = detail::HashCombine(detail::HashSeed, Position.GetHashCode());
    return detail::HashCombine(hash, Rotation.GetHashCode());
}

// 0x7d3cf34
void Transform3D::Serialize(void* ptr, FrameSerializer* serializer) {
    auto* transform = static_cast<Transform3D*>(ptr);
    assert(transform != nullptr && serializer != nullptr);
    FPVector3::Serialize(&transform->Position, serializer);
    FPQuaternion::Serialize(&transform->Rotation, serializer);
}

} // namespace Quantum
