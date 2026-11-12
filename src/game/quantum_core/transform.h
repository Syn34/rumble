// transform.h - Quantum.Transform2D, Quantum.Transform2DVertical and Quantum.Transform3D:
// the position/rotation components of simulation entities (also used as the local
// offset of collider shapes).
//
// The C# structs carry more members (Forward, Right, TransformPoint for 2D, Create...),
// but the build this targets kept only the methods listed here; the rest were stripped
// as unused or inlined into callers.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>

namespace Photon::Deterministic {
class IDeterministicFrameSerializer;
}

namespace Quantum {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;

// Serialisation callbacks receive the frame serializer (Quantum.FrameSerializer). It
// implements IDeterministicFrameSerializer and only that interface is used here.
using FrameSerializer = Photon::Deterministic::IDeterministicFrameSerializer;

// 2D position + rotation (radians, counter-clockwise).
struct Transform2D {
    static constexpr int SIZE = 24;

    FPVector2 Position;
    FP Rotation;

    // 0x7d3e2f4 get_Up - the local +Y axis in world space: (-sin, cos).
    FPVector2 Up() const;

    // 0x7d3e3e8
    std::int32_t GetHashCode() const;

    // 0x7d3e3b0 - Position, then Rotation.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Transform2D) == Transform2D::SIZE);

// Height data that turns a 2D entity into a vertical extent (for 2.5D queries).
struct Transform2DVertical {
    static constexpr int SIZE = 16;

    FP Position; // vertical offset of the base
    FP Height;

    // 0x7d3c8b4 - note the order: Height first, then Position.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Transform2DVertical) == Transform2DVertical::SIZE);

// 3D position + rotation.
struct Transform3D {
    static constexpr int SIZE = 56;

    FPVector3 Position;
    FPQuaternion Rotation;

    // 0x7d3c8f0 get_EulerAngles - Rotation.AsEuler (degrees).
    FPVector3 EulerAngles() const;

    // 0x7d3c950 - direction from world into local space (rotation only).
    FPVector3 InverseTransformDirection(const FPVector3& direction) const;
    // 0x7d3c9e4 - direction from local into world space (rotation only).
    FPVector3 TransformDirection(const FPVector3& direction) const;
    // 0x7d3ca64 - point from world into local space.
    FPVector3 InverseTransformPoint(const FPVector3& position) const;
    // 0x7d3cb14 - point from local into world space.
    FPVector3 TransformPoint(const FPVector3& position) const;

    // 0x7d3cba8 - applies Euler angles (degrees) in local space, optionally renormalising.
    void Rotate(FP xAngle, FP yAngle, FP zAngle, bool normalize = true);
    // 0x7d3cc6c - applies a rotation of `angle` degrees about `axis` in local space.
    void Rotate(const FPVector3& axis, FP angle, bool normalize = true);

    // Local axes in world space, built from the rotation matrix of Rotation.
    FPVector3 Right() const;   // 0x7d3cd84
    FPVector3 Up() const;      // 0x7d3cdd8
    FPVector3 Forward() const; // 0x7d3ce2c
    FPVector3 Left() const;    // 0x7d3ce84 - exactly -Right()
    FPVector3 Back() const;    // 0x7d3ced8 - exactly -Forward()

    // 0x7d3cf6c
    std::int32_t GetHashCode() const;

    // 0x7d3cf34 - Position, then Rotation.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};
static_assert(sizeof(Transform3D) == Transform3D::SIZE);

} // namespace Quantum
