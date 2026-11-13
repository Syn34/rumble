// character_controller3d.cpp - Quantum.CharacterController3D movement.
#include "character_controller3d.h"

#include "quantum_core/builtin_assets.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics3d/scene3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/transform.h"

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/fp_vector2.h"

#include <algorithm>

namespace Quantum {

using Core::FrameBase;
using Photon::Deterministic::FPMath::Clamp;
using Photon::Deterministic::FPMath::Lerp;
using Physics3D::Hit3D;
using Physics3D::HitCollection3D;

namespace {
FP::Raw Raw(FP v) { return v.RawValue(); }
FP FromRaw(FP::Raw v) { return FP::FromRaw(v); }
FP::Raw Mul(FP::Raw a, FP::Raw b) { return (a * b) >> 16; }
FP::Raw Dot(const FPVector3& a, const FPVector3& b) {
    return Mul(Raw(a.Y), Raw(b.Y)) + Mul(Raw(a.X), Raw(b.X)) + Mul(Raw(a.Z), Raw(b.Z));
}

const CharacterController3DConfig* ConfigOf(FrameBase* frame, const AssetRefCharacterController3DConfig& ref) {
    return frame->FindAsset<CharacterController3DConfig>(ref.Id);
}

// Hit3D.GetStaticCollider (0x7d6d870) reduced to what the controller asks: whether the
// static collider of a hit is a mesh.
bool IsMeshStatic(FrameBase* frame, const Hit3D& hit) {
    const Map* map = hit._mapGuid.IsValid() ? frame->FindAsset<Map>(hit._mapGuid) : frame->get_Map();
    return map->StaticColliders3D.at(static_cast<std::size_t>(hit._staticColliderIndex)).ShapeType ==
           Shape3DType::Mesh;
}
} // namespace

// 0x7d2d194 / 0x7d2d1e0
void CharacterController3D::Init(FrameBase* frame, const CharacterController3DConfig* config) {
    _grounded = 0;
    Velocity = FPVector3::Zero();
    SetConfig(frame, config);
}

// 0x7d2d1f0 / 0x7d2d2c0
void CharacterController3D::SetConfig(FrameBase* frame, const CharacterController3DConfig* config) {
    if (config == nullptr) {
        config = frame->FindAsset<CharacterController3DConfig>(AssetGuid{CharacterController3DConfig::DEFAULT_ID});
    }
    _config.Id = config->Guid;
    MaxSpeed = config->MaxSpeed;
}

// 0x7d2c40c - the hits nearest first, up to MaxContacts solid ones. Per hit: the normal (the
// manifold normal, or from the hit point to the sphere's centre, flipped when the centre is
// behind the surface) and the depth against the radius (without the extent). Hits within
// the extent only find the ground; triggers go to the callback; solid hits give the nearest
// contact, the ground (walkable slope) or the slope to slide down, and grow the correction
// so that it covers every contact's depth.
CharacterController3DMovement CharacterController3D::ComputeRawMovement(
    FrameBase* frame, EntityRef entity, const Transform3D* transform, CharacterController3D* kcc,
    const FPVector3& direction, IKCCCallbacks3D* callback, std::optional<std::int32_t> layerMask,
    std::optional<bool> useManifoldNormal) {
    const CharacterController3DConfig* config = ConfigOf(frame, kcc->_config);
    CharacterController3DMovement movement{};
    movement.Tangent = FPVector3::Normalize(direction);
    const FPVector3 center{FromRaw(Raw(transform->Position.X) + Raw(config->Offset.X)),
                           FromRaw(Raw(transform->Position.Y) + Raw(config->Offset.Y)),
                           FromRaw(Raw(transform->Position.Z) + Raw(config->Offset.Z))};
    const bool manifoldNormal = useManifoldNormal.has_value() ? *useManifoldNormal : config->UseManifoldNormal;
    const FPVector3 gravity = config->GravityNormalized();

    // TODO(0x7d48afc, Quantum.Physics3D.PhysicsEngine3D.TryGetQueryHits): with a broad-phase
    // query registered (BroadPhaseQueryIndex) its hits are used; those queries are not
    // reimplemented, so the sphere is always overlapped here.
    Core::FrameContext* context = frame->get_Context();
    HitCollection3D hits(8, HitCollection3D::Temp, context);
    const Shape3D sphere = Shape3D::CreateSphere(FromRaw(Raw(config->Extent) + Raw(config->Radius)));
    context->get_Physics3D()->Scene()->OverlapShape(
        frame, &hits, center, FPQuaternion::Identity(), sphere, layerMask.has_value() ? *layerMask : config->LayerMask,
        Physics3D::QueryOptions::HitAll | Physics3D::QueryOptions::ComputeDetailedInfo);
    kcc->_broadphaseQueryIndexPlusOne = 0;

    movement.Type = CharacterMovementType::FreeFall;
    if (hits.Count() < 1) {
        return movement;
    }
    hits.Sort(center);
    std::int32_t remaining = std::min(config->MaxContacts, hits.Count());
    if (remaining < 1) {
        return movement;
    }
    const FPVector3 up{FromRaw(-Raw(gravity.X)), FromRaw(-Raw(gravity.Y)), FromRaw(-Raw(gravity.Z))};
    const FP::Raw radius = Raw(config->Radius);
    bool any = false; // a solid contact was counted
    bool grounded = false;
    FP::Raw nearestPenetration = 0;
    FPVector3 nearestNormal;
    FPVector3 groundNormal;
    FPVector3 correction;
    FP::Raw sumX = 0, sumY = 0, sumZ = 0;
    std::int32_t contacts = 0;
    CharacterMovementType type = CharacterMovementType::FreeFall;
    for (std::int32_t i = 0; i < hits.Count() && remaining > 0; ++i) {
        const Hit3D& hit = hits.HitsBuffer()[i];
        if (hit._staticColliderIndex < 0 && EntityRef{hit._entity.Index, hit._entity.Version} == entity) {
            continue; // the character itself
        }
        FPVector3 normal;
        FP::Raw depth = 0;
        if (manifoldNormal) {
            normal = hit.Normal;
            depth = Raw(hit.OverlapPenetration) - Raw(config->Extent);
        } else {
            const FPVector3 toCenter{FromRaw(Raw(center.X) - Raw(hit.Point.X)), FromRaw(Raw(center.Y) - Raw(hit.Point.Y)),
                                     FromRaw(Raw(center.Z) - Raw(hit.Point.Z))};
            FP length;
            normal = FPVector3::Normalize(toCenter, length);
            if (hit._staticColliderIndex >= 0 && IsMeshStatic(frame, hit)) {
                depth = radius - Raw(length);
            } else if (Dot(hit.Normal, toCenter) < 0) {
                normal = -normal; // the centre went through the surface
                depth = radius + Raw(length);
            } else {
                depth = radius - Raw(length);
            }
        }
        if (hit._isTrigger == 1) {
            if (callback != nullptr && depth >= 0) {
                callback->OnCharacterTrigger3D(frame, entity, hit);
            }
            continue;
        }
        if (depth < 0) {
            // Only near: the ground if walkable (that ends the search), else used up.
            if (grounded) {
                break;
            }
            if (Raw(FPVector3::Angle(normal, up)) <= Raw(config->MaxSlope)) {
                groundNormal = normal;
                grounded = true;
                break;
            }
            --remaining;
            continue;
        }
        if (callback != nullptr && !callback->OnCharacterCollision3D(frame, entity, hit)) {
            continue;
        }
        --remaining;
        if (!any) {
            nearestPenetration = depth;
            nearestNormal = normal;
        }
        if (Raw(FPVector3::Angle(normal, up)) > Raw(config->MaxSlope)) {
            if (!any) {
                // Too steep: slide along the slope, downhill.
                const FP::Raw along = Mul(Raw(normal.Y), Raw(up.Y)) + Mul(Raw(normal.Z), Raw(up.Z)) +
                                      Mul(Raw(normal.X), Raw(up.X));
                if (along >= 1) {
                    movement.SlopeTangent = FPVector3::Normalize(
                        FPVector3{FromRaw(Raw(gravity.X) + Mul(along, Raw(normal.X))),
                                  FromRaw(Raw(gravity.Y) + Mul(along, Raw(normal.Y))),
                                  FromRaw(Raw(gravity.Z) + Mul(along, Raw(normal.Z)))});
                    type = CharacterMovementType::SlopeFall;
                }
            }
        } else {
            if (!(any && grounded)) {
                // The requested direction projected on the ground.
                const FP::Raw projection = Dot(direction, normal);
                movement.Tangent = FPVector3::Normalize(
                    FPVector3{FromRaw(Raw(direction.X) - Mul(projection, Raw(normal.X))),
                              FromRaw(Raw(direction.Y) - Mul(projection, Raw(normal.Y))),
                              FromRaw(Raw(direction.Z) - Mul(projection, Raw(normal.Z)))});
                groundNormal = normal;
            }
            const bool idle = (Raw(direction.X) | Raw(direction.Y) | Raw(direction.Z)) == 0;
            type = idle ? CharacterMovementType::None : CharacterMovementType::Horizontal;
            grounded = true;
        }
        if (!any) {
            correction = FPVector3{FromRaw(Mul(depth, Raw(normal.X))), FromRaw(Mul(depth, Raw(normal.Y))),
                                   FromRaw(Mul(depth, Raw(normal.Z)))};
        } else {
            // Extend the correction so this contact is resolved too: along the correction
            // when the contact lies off its plane, otherwise perpendicular to it.
            FP length;
            const FPVector3 c = FPVector3::Normalize(correction, length);
            const FP::Raw cx = Raw(c.X), cy = Raw(c.Y), cz = Raw(c.Z);
            const FP::Raw nx = Raw(normal.X), ny = Raw(normal.Y), nz = Raw(normal.Z);
            const FP::Raw ax = Mul(cy, nz) - Mul(cz, ny);
            const FP::Raw ay = Mul(cz, nx) - Mul(cx, nz);
            const FP::Raw az = Mul(cx, ny) - Mul(cy, nx);
            const FPVector3 v = FPVector3::Normalize(FPVector3{FromRaw(Mul(ay, cz) - Mul(az, cy)),
                                                               FromRaw(Mul(az, cx) - Mul(ax, cz)),
                                                               FromRaw(Mul(ax, cy) - Mul(ay, cx))});
            const FP::Raw vDotN = Mul(Raw(v.Y), ny) + Mul(Raw(v.X), nx) + Mul(Raw(v.Z), nz);
            FPVector3 axis;
            FP::Raw amount = 0;
            if (vDotN + 0x8000 <= 0x10000 && vDotN + 0x8000 >= 0) {
                axis = c;
                amount = depth - Raw(length);
            } else {
                const FP::Raw covered = Mul(nx, Raw(correction.X)) + Mul(ny, Raw(correction.Y)) +
                                        Mul(nz, Raw(correction.Z));
                axis = v;
                amount = ((depth - covered) << 16) / vDotN;
            }
            if (amount >= 1) {
                correction = FPVector3{FromRaw(Raw(correction.X) + Mul(amount, Raw(axis.X))),
                                       FromRaw(Raw(correction.Y) + Mul(amount, Raw(axis.Y))),
                                       FromRaw(Raw(correction.Z) + Mul(amount, Raw(axis.Z)))};
            }
        }
        ++contacts;
        sumX += Raw(normal.X);
        sumY += Raw(normal.Y);
        sumZ += Raw(normal.Z);
        any = true;
    }
    if (contacts >= 2) {
        sumX /= contacts;
        sumY /= contacts;
        sumZ /= contacts;
    }
    movement.Type = type;
    movement.NearestNormal = nearestNormal;
    movement.AvgNormal = FPVector3{FromRaw(sumX), FromRaw(sumY), FromRaw(sumZ)};
    movement.GroundNormal = groundNormal;
    movement.Correction = correction;
    movement.Penetration = FromRaw(nearestPenetration);
    movement.Grounded = grounded;
    movement.Contacts = contacts;
    return movement;
}

// 0x7d2d56c
void CharacterController3D::ComputeRawSteer(FrameBase* frame, const CharacterController3DMovement& movement, FP dt) {
    _grounded = movement.Grounded ? 1 : 0;
    const CharacterController3DConfig* config = ConfigOf(frame, _config);
    const FP::Raw step = Raw(dt);
    const auto accelerate = [&](const FPVector3& direction) {
        const FP::Raw a = Raw(config->Acceleration);
        Velocity.X = FromRaw(Raw(Velocity.X) + Mul(Mul(a, Raw(direction.X)), step));
        Velocity.Y = FromRaw(Raw(Velocity.Y) + Mul(Mul(a, Raw(direction.Y)), step));
        Velocity.Z = FromRaw(Raw(Velocity.Z) + Mul(Mul(a, Raw(direction.Z)), step));
    };
    const auto brake = [](FP value, FP t) {
        if (Raw(value) == 0) {
            return value;
        }
        const FP braked = Lerp(value, FP::Zero(), t);
        const FP::Raw magnitude = Raw(braked) < 0 ? -Raw(braked) : Raw(braked);
        return magnitude < 0x1999 ? FP::Zero() : braked; // below 0.1 stops
    };
    FP::Raw minVertical = -(FP::Raw{100} << 16);
    switch (movement.Type) {
    case CharacterMovementType::FreeFall: {
        Velocity.Y = FromRaw(Raw(Velocity.Y) - Mul(Raw(config->GravityStrength()), step));
        const bool steering = (Raw(movement.Tangent.X) | Raw(movement.Tangent.Y) | Raw(movement.Tangent.Z)) != 0;
        if (config->AirControl && steering) {
            accelerate(movement.Tangent);
        } else {
            const FP t = FromRaw(Mul(Raw(config->Braking), step));
            Velocity.X = Lerp(Velocity.X, FP::Zero(), t);
            Velocity.Z = Lerp(Velocity.Z, FP::Zero(), t);
        }
        break;
    }
    case CharacterMovementType::None: {
        const FP t = FromRaw(Mul(Raw(config->Braking), step));
        Velocity.X = brake(Velocity.X, t);
        Velocity.Z = brake(Velocity.Z, t);
        if (Raw(Velocity.Y) != 0 && _jumped != 1) {
            Velocity.Y = brake(Velocity.Y, t);
        }
        minVertical = 0;
        break;
    }
    case CharacterMovementType::SlopeFall:
        accelerate(movement.SlopeTangent);
        minVertical = -Raw(config->MaxSlopeSpeed);
        break;
    case CharacterMovementType::Horizontal: {
        accelerate(movement.Tangent);
        const FP speed = FPVector3::Dot(Velocity, movement.Tangent);
        const FP t = FromRaw(Mul(Raw(config->Braking), step));
        Velocity.X = Lerp(Velocity.X, FromRaw(Mul(Raw(movement.Tangent.X), Raw(speed))), t);
        Velocity.Z = Lerp(Velocity.Z, FromRaw(Mul(Raw(movement.Tangent.Z), Raw(speed))), t);
        if (_jumped != 1) {
            Velocity.Y = Lerp(Velocity.Y, FromRaw(Mul(Raw(movement.Tangent.Y), Raw(speed))), t);
        }
        const FP::Raw magnitude = Raw(speed) < 0 ? -Raw(speed) : Raw(speed);
        if (magnitude > Raw(MaxSpeed)) {
            const FP::Raw sign = Raw(Photon::Deterministic::FPMath::Sign(speed));
            const FP::Raw excess = magnitude - Raw(MaxSpeed);
            Velocity.X = FromRaw(Raw(Velocity.X) - Mul(excess, Mul(Raw(movement.Tangent.X), sign)));
            Velocity.Y = FromRaw(Raw(Velocity.Y) - Mul(excess, Mul(Raw(movement.Tangent.Y), sign)));
            Velocity.Z = FromRaw(Raw(Velocity.Z) - Mul(Mul(Raw(movement.Tangent.Z), sign), excess));
        }
        _jumped = 0;
        return; // no horizontal or vertical limit while walking
    }
    }
    // Horizontal speed limited to MaxSpeed, vertical speed clamped.
    FP x = Velocity.X;
    FP z = Velocity.Z;
    const FP::Raw horizontalSq = Mul(Raw(z), Raw(z)) + Mul(Raw(x), Raw(x));
    if (horizontalSq > Mul(Raw(MaxSpeed), Raw(MaxSpeed))) {
        const Photon::Deterministic::FPVector2 n = Photon::Deterministic::FPVector2::Normalize({x, z});
        x = FromRaw(Mul(Raw(n.X), Raw(MaxSpeed)));
        z = FromRaw(Mul(Raw(n.Y), Raw(MaxSpeed)));
    }
    Velocity.X = x;
    Velocity.Y = Clamp(Velocity.Y, FromRaw(minVertical), FP::FromInt(100));
    Velocity.Z = z;
    _jumped = 0;
}

// 0x7d2d300
void CharacterController3D::Move(FrameBase* frame, EntityRef entity, const FPVector3& direction,
                                 IKCCCallbacks3D* callback, std::optional<std::int32_t> layerMask,
                                 std::optional<bool> useManifoldNormal, std::optional<FP> deltaTime) {
    Transform3D* transform = frame->GetPointer<Transform3D>(entity);
    const FP dt = deltaTime.has_value() ? *deltaTime : frame->get_DeltaTime();
    const CharacterController3DMovement movement =
        ComputeRawMovement(frame, entity, transform, this, direction, callback, layerMask, useManifoldNormal);
    ComputeRawSteer(frame, movement, dt);
    FP::Raw dx = Mul(Raw(Velocity.X), Raw(dt));
    FP::Raw dy = Mul(Raw(Velocity.Y), Raw(dt));
    FP::Raw dz = Mul(Raw(Velocity.Z), Raw(dt));
    if (Raw(movement.Penetration) > 0x41) {
        const CharacterController3DConfig* config = ConfigOf(frame, _config);
        if (movement.Penetration > config->MaxPenetration) {
            dx += Raw(movement.Correction.X);
            dy += Raw(movement.Correction.Y);
            dz += Raw(movement.Correction.Z);
        } else {
            const FP::Raw share = Raw(config->PenetrationCorrection);
            dx += Mul(share, Raw(movement.Correction.X));
            dy += Mul(share, Raw(movement.Correction.Y));
            dz += Mul(share, Raw(movement.Correction.Z));
        }
    }
    transform->Position.X = FromRaw(Raw(transform->Position.X) + dx);
    transform->Position.Y = FromRaw(Raw(transform->Position.Y) + dy);
    transform->Position.Z = FromRaw(Raw(transform->Position.Z) + dz);
}

} // namespace Quantum
