// physics_utility.cpp
#include "quantum_code/systems/physics_utility.h"

#include "quantum_code/assets/level_assets.h"
#include "quantum_code/fp_math_utility.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/query_options.h"
#include "quantum_core/shape3d.h"
#include "quantum_core/transform.h"

#include "photon_deterministic/fp_math.h"

#include <algorithm>

namespace Quantum {

using Physics3D::Hit3D;
using Physics3D::HitCollection3D;

bool PhysicsUtility::IsGround(const FPVector3& normal) {
    if (FPMathUtility::EqualWithEpsilon(normal, FPVector3::Up())) {
        return true;
    }
    return FPVector3::Angle(normal, FPVector3::Up()) <= FP::FromInt(40);
}

Physics3D::PhysicsEngine3D* GetPhysicsEngine3D(Frame* f) {
    return f->get_Context()->get_Physics3D();
}

const MapStaticCollider3D* GetHitStaticCollider(Frame* f, const Hit3D& hit) {
    if (hit.StaticColliderIndex() < 0) {
        return nullptr;
    }
    Map* map = nullptr;
    if (hit._mapGuid.IsValid()) {
        f->TryFindAsset<Map>(hit._mapGuid, map);
    } else {
        map = f->get_Map();
    }
    if (map == nullptr) {
        return nullptr;
    }
    const auto index = static_cast<std::size_t>(hit.StaticColliderIndex());
    if (index >= map->StaticColliders3D.size()) {
        return nullptr;
    }
    return &map->StaticColliders3D[index];
}

StaticColliderConfig* GetHitStaticColliderConfig(Frame* f, const Hit3D& hit) {
    const MapStaticCollider3D* collider = GetHitStaticCollider(f, hit);
    if (collider == nullptr) {
        return nullptr;
    }
    StaticColliderConfig* config = nullptr;
    f->TryFindAsset<StaticColliderConfig>(collider->StaticData.Asset, config);
    return config;
}

EntityRef GetHitEntity(const Hit3D& hit) {
    const Physics2D::HitEntityRef entity = hit.Entity();
    return EntityRef{entity.Index, entity.Version};
}

void LinecastNearest(Frame* f, HitCollection3D* hits, const FPVector3& start, const FPVector3& end,
                     std::int32_t layerMask, std::int32_t options) {
    GetPhysicsEngine3D(f)->Linecast(f, hits, start, end, true, layerMask, options);
    if (hits->Count() >= 2) {
        hits->Sort(start);
        hits->SetCount(1);
    }
}

void RaycastNearest(Frame* f, HitCollection3D* hits, const FPVector3& origin, const FPVector3& direction, FP distance,
                    std::int32_t layerMask, std::int32_t options) {
    LinecastNearest(f, hits, origin, origin + direction * distance, layerMask, options);
}

namespace {

bool IsMeshCollider(Frame* f, const Hit3D& hit) {
    const MapStaticCollider3D* collider = GetHitStaticCollider(f, hit);
    return collider != nullptr && collider->ShapeType == Shape3DType::Mesh;
}

} // namespace

CharacterController3DMovementExt CharacterController3DLib::ComputeRawMovement(Frame* frame, EntityRef entity,
                                                                               Transform3D* transform,
                                                                               CharacterController3D* kcc,
                                                                               CharacterController3DConfig* config,
                                                                               const FPVector3& direction,
                                                                               IKCCCallbacks3D* callback) {
    (void)kcc;
    CharacterController3DMovementExt result{};
    CharacterController3DMovement& movement = result.baseMovementPack;
    movement.Tangent = direction.Normalized();
    movement.Type = CharacterMovementType::FreeFall;

    const FPVector3 center = transform->Position + config->Offset;
    const FPVector3 gravity = config->GravityNormalized();
    const FPVector3 up = -gravity;
    const bool manifoldNormal = config->UseManifoldNormal;

    // TODO(0x7d48afc, Quantum.Physics3D.PhysicsEngine3D.TryGetQueryHits): the original first
    // takes the hits of the query InjectOverlapQuery registered (when the controller has
    // one); those queries are not available, so the sphere is always overlapped here.
    Core::FrameContext* context = frame->get_Context();
    HitCollection3D hits(8, HitCollection3D::Temp, context);
    const Shape3D sphere = Shape3D::CreateSphere(config->Radius + config->Extent);
    GetPhysicsEngine3D(frame)->OverlapShape(
        frame, &hits, center, FPQuaternion::Identity(), sphere, config->LayerMask,
        static_cast<std::int32_t>(QueryOptions::HitAll | QueryOptions::ComputeDetailedInfo));

    if (hits.Count() < 1) {
        return result;
    }
    hits.Sort(center);
    std::int32_t remaining = std::min(config->MaxContacts, hits.Count());
    if (remaining < 1) {
        return result;
    }

    bool any = false; // a solid contact has been counted
    bool grounded = false;
    FP nearestPenetration;
    FPVector3 nearestNormal;
    FPVector3 groundNormal;
    FPVector3 correction;
    FPVector3 normalSum; // summed raw, averaged by the contact count
    std::int32_t contacts = 0;
    CharacterMovementType type = CharacterMovementType::FreeFall;

    for (std::int32_t i = 0; i < hits.Count() && remaining > 0; ++i) {
        const Hit3D& hit = hits.HitsBuffer()[i];
        if (GetHitEntity(hit) == entity) {
            continue; // the character itself
        }

        FPVector3 normal;
        FP depth;
        if (manifoldNormal) {
            normal = hit.Normal;
            depth = hit.OverlapPenetration - config->Extent;
        } else {
            const FPVector3 toCenter = center - hit.Point;
            FP length;
            normal = FPVector3::Normalize(toCenter, length);
            if (hit.StaticColliderIndex() >= 0 && IsMeshCollider(frame, hit)) {
                depth = config->Radius - length;
            } else if (FPVector3::Dot(hit.Normal, toCenter) < FP{}) {
                normal = -normal; // the centre went through the surface
                depth = config->Radius + length;
            } else {
                depth = config->Radius - length;
            }
        }

        if (hit.IsTrigger()) {
            if (callback != nullptr && depth >= FP{}) {
                callback->OnCharacterTrigger3D(frame, entity, hit);
            }
            continue;
        }

        if (depth < FP{}) {
            // Only inside the extent: it can still be the ground (which ends the search).
            if (grounded) {
                break;
            }
            if (FPVector3::Angle(normal, up) <= config->MaxSlope) {
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
        if (hit.IsDynamic()) {
            const EntityRef other = GetHitEntity(hit);
            PhysicsCollider3D* collider = nullptr;
            if (frame->TryGetPointer<PhysicsCollider3D>(other, collider)) {
                result.dynamicCollisionsFound =
                    LayerMask::FromInt((1 << collider->Layer()) | result.dynamicCollisionsFound.BitMask);
                result.dynamicEntityHit = other;
            }
        }
        --remaining;
        if (!any) {
            nearestPenetration = depth;
            nearestNormal = normal;
        }

        if (FPVector3::Angle(normal, up) <= config->MaxSlope) {
            if (!(any && grounded)) {
                // The requested direction along the ground.
                movement.Tangent = (direction - normal * FPVector3::Dot(direction, normal)).Normalized();
                groundNormal = normal;
            }
            type = direction == FPVector3::Zero() ? CharacterMovementType::None : CharacterMovementType::Horizontal;
            grounded = true;
        }

        if (!any) {
            correction = normal * depth;
        } else {
            // Grow the correction so it resolves this contact too: along itself when the
            // contact's normal lies near its plane, otherwise across it.
            FP length;
            const FPVector3 axis = FPVector3::Normalize(correction, length);
            const FPVector3 across = FPVector3::Cross(FPVector3::Cross(axis, normal), axis).Normalized();
            const FP acrossDotNormal = FPVector3::Dot(across, normal);
            const FP half = FP::FromRaw(0x8000);
            FPVector3 extension;
            FP amount;
            if (acrossDotNormal + half >= FP{} && acrossDotNormal + half <= FP::One()) {
                extension = axis;
                amount = depth - length;
            } else {
                // The part of the depth the correction does not cover yet, along `across`.
                const FP covered = FPVector3::Dot(normal, correction);
                extension = across;
                amount = (depth - covered) / acrossDotNormal;
            }
            if (amount.RawValue() >= 1) {
                correction = correction + extension * amount;
            }
        }
        ++contacts;
        normalSum = FPVector3{FP::FromRaw(normalSum.X.RawValue() + normal.X.RawValue()),
                              FP::FromRaw(normalSum.Y.RawValue() + normal.Y.RawValue()),
                              FP::FromRaw(normalSum.Z.RawValue() + normal.Z.RawValue())};
        any = true;
    }

    if (contacts >= 2) {
        // The original divides the raw sums by the count (integer division).
        normalSum = FPVector3{FP::FromRaw(normalSum.X.RawValue() / contacts), FP::FromRaw(normalSum.Y.RawValue() / contacts),
                              FP::FromRaw(normalSum.Z.RawValue() / contacts)};
    }
    movement.Type = type;
    movement.NearestNormal = nearestNormal;
    movement.AvgNormal = normalSum;
    movement.GroundNormal = groundNormal;
    movement.Correction = correction;
    movement.Penetration = nearestPenetration;
    movement.Grounded = grounded;
    movement.Contacts = contacts;
    return result;
}

} // namespace Quantum
