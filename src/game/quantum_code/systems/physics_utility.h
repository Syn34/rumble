// physics_utility.h - the game's helpers around 3D physics queries: PhysicsUtility (static,
// ground and wall hits), the nearest-hit ray cast the movement code uses, static collider
// lookups for hits, and CharacterController3DLib (the game's own version of the kinematic
// controller's contact gathering, with the dynamic bodies it touched).
#pragma once

#include "quantum_code/frame.h"
#include "quantum_core/layer_mask.h"
#include "quantum_core/physics3d/character_controller3d.h"
#include "quantum_core/physics3d/hit3d.h"

namespace Quantum {

struct MapStaticCollider3D;
class StaticColliderConfig;
namespace Physics3D {
class PhysicsEngine3D;
}

// Quantum.CharacterController3DMovementExt: CharacterController3DLib's result, the
// controller's movement plus the layers of the dynamic bodies it collided with and the last
// such body.
struct CharacterController3DMovementExt {
    CharacterController3DMovement baseMovementPack;
    LayerMask dynamicCollisionsFound;
    EntityRef dynamicEntityHit;
};

class PhysicsUtility {
  public:
    // 0x7b7cb0c - Hits on map colliders (the index is -1 for entity hits).
    static bool IsStatic(const Physics3D::Hit3D& hit) { return hit.StaticColliderIndex() != -1; }
    // 0x7b7cb1c
    static bool IsGround(const Physics3D::Hit3D& hit) { return IsGround(hit.Normal); }
    // 0x7b7cb7c - Up, or within 40 degrees of it.
    static bool IsGround(const FPVector3& normal);
    // 0x7b7cc68
    static bool IsWall(const Physics3D::Hit3D& hit) { return !IsGround(hit.Normal); }
    // TODO(0x7b7cccc, PhysicsUtility.IsCliffAhead): probes for a drop in front of the wrench
    //   user (WrenchWeaponSpec); not reimplemented yet.
};

// The frame context's 3D physics engine (null when the session runs without 3D physics).
Physics3D::PhysicsEngine3D* GetPhysicsEngine3D(Frame* f);

// Physics3D.Hit3D.TryGetStaticCollider (0x7d6d728): the map collider a static hit is on;
// null for entity hits or an index outside the map's colliders. Written here because the
// quantum.core version is not available yet.
const MapStaticCollider3D* GetHitStaticCollider(Frame* f, const Physics3D::Hit3D& hit);
// The static collider's StaticColliderConfig asset, if it has one.
StaticColliderConfig* GetHitStaticColliderConfig(Frame* f, const Physics3D::Hit3D& hit);
// The entity of a dynamic hit (Hit3D.Entity), as an EntityRef.
EntityRef GetHitEntity(const Physics3D::Hit3D& hit);

// The game's ray cast (inlined from the physics API, e.g. at 0x7c59054): a first-hit line
// cast from `origin` along `direction` * `distance`; when several hits come back the
// nearest to the origin is kept. The collection holds at most one hit afterwards.
void RaycastNearest(Frame* f, Physics3D::HitCollection3D* hits, const FPVector3& origin, const FPVector3& direction,
                    FP distance, std::int32_t layerMask, std::int32_t options);
// The same with an end point.
void LinecastNearest(Frame* f, Physics3D::HitCollection3D* hits, const FPVector3& start, const FPVector3& end,
                     std::int32_t layerMask, std::int32_t options);

class CharacterController3DLib {
  public:
    // 0x7bda6b8 - The controller's contacts around the character, as quantum.core's
    // CharacterController3D.ComputeRawMovement, with the game's differences: steep contacts
    // never turn into a slope slide (no SlopeFall, no slope tangent), the overlap uses the
    // config's layer mask, and every accepted dynamic contact adds its collider's layer to
    // dynamicCollisionsFound and becomes dynamicEntityHit.
    static CharacterController3DMovementExt ComputeRawMovement(Frame* frame, EntityRef entity, Transform3D* transform,
                                                               CharacterController3D* kcc,
                                                               CharacterController3DConfig* config,
                                                               const FPVector3& direction, IKCCCallbacks3D* callback);
    // TODO(0x7bda588, Quantum.CharacterController3DLib.InjectOverlapQuery): registers the
    //   controller's sphere as a broad-phase query for the next physics step; quantum.core's
    //   broad-phase queries are not available yet, so ComputeRawMovement overlaps directly.
};

} // namespace Quantum
