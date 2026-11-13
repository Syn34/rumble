// character_controller3d.h - the kinematic character controller's movement in 3D
// (Quantum.CharacterController3D.Move / ComputeRawMovement / ComputeRawSteer): the
// component (physics_components.h) overlaps a sphere around the character, works out the
// contacts' normals, the ground and the correction out of penetration, then steers its
// velocity by the config and moves the transform.
#pragma once

#include "quantum_core/entity_ref.h"
#include "quantum_core/physics3d/hit3d.h"

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector3.h"

#include <cstdint>
#include <optional>

namespace Quantum {
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;
struct CharacterController3D;
struct Transform3D;
class CharacterController3DConfig;
namespace Core {
class FrameBase;
}

// Quantum.CharacterMovementType
enum class CharacterMovementType : std::int32_t {
    None = 0,
    FreeFall = 1,
    SlopeFall = 2,
    Horizontal = 3,
};

// Quantum.CharacterController3DMovement: what ComputeRawMovement found.
struct CharacterController3DMovement {
    CharacterMovementType Type = CharacterMovementType::None;
    FPVector3 NearestNormal; // normal of the first (nearest) contact
    FPVector3 AvgNormal;     // contact normals averaged
    FPVector3 GroundNormal;
    FPVector3 Tangent;      // the requested direction along the ground
    FPVector3 SlopeTangent; // downhill along a too-steep slope
    FPVector3 Correction;   // out of the penetration
    FP Penetration;         // depth of the nearest contact
    bool Grounded = false;
    std::int32_t Contacts = 0;
};

// Quantum.IKCCCallbacks3D: lets the game filter the controller's contacts.
class IKCCCallbacks3D {
  public:
    virtual ~IKCCCallbacks3D() = default;
    // False ignores the contact.
    virtual bool OnCharacterCollision3D(Core::FrameBase* f, EntityRef character, Physics3D::Hit3D hit) = 0;
    virtual void OnCharacterTrigger3D(Core::FrameBase* f, EntityRef character, Physics3D::Hit3D hit) = 0;
};

} // namespace Quantum
