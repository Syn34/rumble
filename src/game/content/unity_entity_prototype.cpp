// unity_entity_prototype.cpp - the engine-side half of entity prototypes (PhotonQuantum.dll):
// the EntityPrototype script, its source-collider helper, the prototype converter and the
// editor-side twins of prototypes that link to other entities.
#include "content/unity_entity_prototype.h"

#include "photon_deterministic/log.h"
#include "photon_deterministic/nullable_fp_vector3.h"
#include "quantum_code/prototypes/flat_entity_prototype_container.h"
#include "quantum_code/prototypes/prototypes.h"
#include "quantum_core/component_prototypes.h"
#include "quantum_core/map.h"

#include <algorithm>
#include <stdexcept>

namespace rr::content::unity {

using namespace Quantum::Prototypes;
using rr::assets::Quaternionf;
using rr::assets::Vector3f;

// --- FPMathUtils -----------------------------------------------------------------------------

FP ToFP(float value) {
    return FP::FromFloat_UNSAFE(value);
}

FPVector2 ToFPVector2(Vector3f v) {
    return FPVector2(ToFP(v.x), ToFP(v.z));
}

FPVector3 ToFPVector3(Vector3f v) {
    return FPVector3(ToFP(v.x), ToFP(v.y), ToFP(v.z));
}

FP ToFPVerticalPosition(Vector3f v) {
    return ToFP(v.y);
}

FP ToFPRotation2DDegrees(Quaternionf q) {
    return -ToFP(rr::assets::EulerAnglesDegrees(q).y);
}

// --- converter -------------------------------------------------------------------------------

void EntityPrototypeConverter::Convert(const std::optional<ScriptKey>& prototype,
                                       Quantum::MapEntityId& result) const {
    result = Quantum::MapEntityId::Invalid();
    if (!prototype)
        return;
    if (AssetPrototype) {
        if (*AssetPrototype == *prototype)
            result = Quantum::MapEntityId::Create(0);
        return;
    }
    const auto it = std::find(OrderedMapPrototypes.begin(), OrderedMapPrototypes.end(), *prototype);
    if (it != OrderedMapPrototypes.end()) {
        result = Quantum::MapEntityId::Create(static_cast<int>(it - OrderedMapPrototypes.begin()));
    }
}

// --- EntityPrototype script ------------------------------------------------------------------

void PhysicsBodyGeneric::EnsureVersionUpdated() {
    for (; Version2D < 2; ++Version2D) {
        if (Version2D == 0)
            Config2D |= 0x40;
        if (Version2D == 1)
            Config2D &= 0x7F;
    }
    for (; Version3D < 2; ++Version3D) {
        if (Version3D == 0)
            Config3D |= 0x20;
        if (Version3D == 1)
            Config3D &= static_cast<std::uint8_t>(~0x40);
    }
}

bool TrySetShapeConfigFromSourceCollider3D(Quantum::Shape3DConfig& shape,
                                           const EntityPrototype& reference,
                                           const std::optional<SourceColliderInfo>& source) {
    if (!source) {
        shape.IsSetFromSourceCollider = false;
        return false;
    }
    // EntityPrototypeUtils.ThrowIfDifferentWorldPosition / ThrowIfDifferentWorldRotation: the
    // engine's approximate equality of vectors (squared distance) and rotations (dot product).
    const auto checkPosition = [&] {
        const float dx = source->worldCenter.x - reference.Position.x;
        const float dy = source->worldCenter.y - reference.Position.y;
        const float dz = source->worldCenter.z - reference.Position.z;
        if (dx * dx + dy * dy + dz * dz >= 9.99999944e-11f) {
            throw std::logic_error("This collider needs to have the same position (including the "
                                   "offset) as the prototype.");
        }
    };
    const auto checkRotation = [&] {
        const Quaternionf& a = source->worldRotation;
        const Quaternionf& b = reference.Rotation;
        if (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w <= 0.999999f) {
            throw std::logic_error(
                "This collider needs to have the same rotation as the prototype.");
        }
    };
    switch (source->kind) {
    case SourceColliderInfo::Kind::Sphere:
        checkPosition();
        shape.ShapeType = Quantum::Shape3DType::Sphere;
        shape.SphereRadius = ToFP(source->radius);
        shape.IsSetFromSourceCollider = true;
        return true;
    case SourceColliderInfo::Kind::Box: {
        checkPosition();
        checkRotation();
        shape.ShapeType = Quantum::Shape3DType::Box;
        const Vector3f half{source->size.x * 0.5f, source->size.y * 0.5f, source->size.z * 0.5f};
        shape.BoxExtents = ToFPVector3(rr::assets::Scale(half, source->lossyScale));
        shape.IsSetFromSourceCollider = true;
        return true;
    }
    case SourceColliderInfo::Kind::Other:
        break;
    }
    throw std::invalid_argument("Source collider type " + source->className +
                                " is not supported for 3D shapes");
}

void EntityPrototype::PreSerialize() {
    bool fromSource = false;
    if (TransformMode == EntityPrototypeTransformMode::Transform2D) {
        // TODO(0x7abe4f8, EntityPrototypeUtils.TrySetShapeConfigFromSourceCollider2D): copies
        // a circle/box/polygon/edge collider into the 2D shape config; needs the 2D shape
        // config type (no 2D prototypes in the game's content).
        if (PhysicsCollider.SourceCollider) {
            throw std::logic_error("EntityPrototype: 2D source colliders are not supported yet");
        }
    } else if (TransformMode == EntityPrototypeTransformMode::Transform3D) {
        fromSource = TrySetShapeConfigFromSourceCollider3D(PhysicsCollider.Shape3D, *this,
                                                           PhysicsCollider.SourceCollider);
    }
    if (fromSource) {
        // EntityPrototypeUtils.IsColliderTrigger, and the collider object's layer.
        PhysicsCollider.IsTrigger = PhysicsCollider.SourceCollider->isTrigger;
        PhysicsCollider.Layer = PhysicsCollider.SourceCollider->layer;
    }

    if (PhysicsBody.IsEnabled) {
        PhysicsBody.EnsureVersionUpdated();
        if (TransformMode == EntityPrototypeTransformMode::Transform2D) {
            // A 2D body that freezes its rotation freezes it about every axis.
            PhysicsBody.RotationFreeze = (PhysicsBody.Config2D & 0x02) != 0 ? 0x07 : 0x00;
        }
    }

    if (NavMeshPathfinder.IsEnabled) {
        InitialNavMeshTargetInfo& target = NavMeshPathfinder.InitialTarget;
        if (target.TargetPosition)
            target.Position = ToFPVector3(*target.TargetPosition);
        if (target.NavMesh.ReferenceName) {
            target.NavMesh.Asset = {};
            target.NavMesh.Name = *target.NavMesh.ReferenceName;
        }
    }
}

void EntityPrototype::SerializeImplicitComponents(ComponentPrototypeList& prototypes,
                                                  bool& hasSelfView) const {
    const auto emitCollider2D = [&] {
        auto collider = std::make_shared<PhysicsCollider2D_Prototype>();
        collider->IsTrigger = PhysicsCollider.IsTrigger;
        collider->Layer = PhysicsCollider.Layer;
        collider->PhysicsMaterial = PhysicsCollider.Material;
        prototypes.push_back(collider);
        auto callbacks = std::make_shared<PhysicsCallbacks2D_Prototype>();
        callbacks->CallbackFlags = PhysicsCollider.CallbackFlags;
        prototypes.push_back(callbacks);
        if (!PhysicsCollider.IsTrigger && PhysicsBody.IsEnabled) {
            auto body = std::make_shared<PhysicsBody2D_Prototype>();
            body->Config = PhysicsBody.Config2D;
            body->Version = PhysicsBody.Version2D;
            body->AngularDrag = PhysicsBody.AngularDrag;
            body->Drag = PhysicsBody.Drag;
            body->Mass = PhysicsBody.Mass;
            body->CenterOfMass = PhysicsBody.CenterOfMass2D;
            body->GravityScale = PhysicsBody.GravityScale;
            prototypes.push_back(body);
        }
    };
    const auto emitCollider3D = [&] {
        auto collider = std::make_shared<PhysicsCollider3D_Prototype>();
        collider->IsTrigger = PhysicsCollider.IsTrigger;
        collider->Layer = PhysicsCollider.Layer;
        collider->PhysicsMaterial = PhysicsCollider.Material;
        collider->ShapeConfig = std::make_shared<Quantum::Shape3DConfig>(PhysicsCollider.Shape3D);
        prototypes.push_back(collider);
        auto callbacks = std::make_shared<PhysicsCallbacks3D_Prototype>();
        callbacks->CallbackFlags = PhysicsCollider.CallbackFlags;
        prototypes.push_back(callbacks);
        if (!PhysicsCollider.IsTrigger && PhysicsBody.IsEnabled) {
            auto body = std::make_shared<PhysicsBody3D_Prototype>();
            body->Config = PhysicsBody.Config3D;
            body->Version = PhysicsBody.Version3D;
            body->AngularDrag = PhysicsBody.AngularDrag;
            body->Drag = PhysicsBody.Drag;
            body->Mass = PhysicsBody.Mass;
            body->RotationFreeze = PhysicsBody.RotationFreeze;
            body->CenterOfMass = PhysicsBody.CenterOfMass3D;
            body->GravityScale = PhysicsBody.GravityScale;
            prototypes.push_back(body);
        }
    };

    if (TransformMode == EntityPrototypeTransformMode::Transform2D) {
        auto transform = std::make_shared<Transform2D_Prototype>();
        transform->Position = ToFPVector2(Position);
        transform->Rotation = ToFPRotation2DDegrees(Rotation);
        prototypes.push_back(transform);
        if (Transform2DVertical.IsEnabled) {
            auto vertical = std::make_shared<Transform2DVertical_Prototype>();
            vertical->Position =
                ToFPVerticalPosition(Position) + Transform2DVertical.PositionOffset;
            vertical->Height = Transform2DVertical.Height;
            prototypes.push_back(vertical);
        }
        if (PhysicsCollider.IsEnabled)
            emitCollider2D();
    } else if (TransformMode == EntityPrototypeTransformMode::Transform3D) {
        auto transform = std::make_shared<Transform3D_Prototype>();
        transform->Position = ToFPVector3(Position);
        transform->Rotation = ToFPVector3(rr::assets::EulerAnglesDegrees(Rotation));
        prototypes.push_back(transform);
        if (PhysicsCollider.IsEnabled)
            emitCollider3D();
    }

    if (NavMeshPathfinder.IsEnabled) {
        auto pathfinder = std::make_shared<NavMeshPathfinder_Prototype>();
        pathfinder->AgentConfig = NavMeshPathfinder.NavMeshAgentConfig;
        const InitialNavMeshTargetInfo& target = NavMeshPathfinder.InitialTarget;
        if (target.IsEnabled) {
            pathfinder->InitialTarget = Photon::Deterministic::NullableFPVector3(target.Position);
            pathfinder->InitialTargetNavMesh = target.NavMesh.Asset;
            pathfinder->InitialTargetNavMeshName = target.NavMesh.Name;
        }
        prototypes.push_back(pathfinder);
        if (NavMeshSteeringAgent.IsEnabled) {
            auto steering = std::make_shared<NavMeshSteeringAgent_Prototype>();
            steering->OverrideMaxSpeed = NavMeshSteeringAgent.MaxSpeed.IsEnabled;
            steering->OverrideAcceleration = NavMeshSteeringAgent.Acceleration.IsEnabled;
            steering->MaxSpeed = NavMeshSteeringAgent.MaxSpeed.Value;
            steering->Acceleration = NavMeshSteeringAgent.Acceleration.Value;
            prototypes.push_back(steering);
            if (NavMeshAvoidanceAgent.IsEnabled) {
                prototypes.push_back(std::make_shared<NavMeshAvoidanceAgent_Prototype>());
            }
        }
    }

    hasSelfView = HasEntityView;
    if (!hasSelfView && View.Id.IsValid()) {
        auto view = std::make_shared<View_Prototype>();
        view->Current = View;
        prototypes.push_back(view);
    }
}

// --- prototypes linking to other entities ----------------------------------------------------

void BlackboardValue_Prototype::Convert(
    const EntityPrototypeConverter& converter,
    Quantum::Prototypes::BlackboardValue_Prototype& result) const {
    result._field_used_ = _field_used_;
    result.BooleanValue = BooleanValue;
    result.ByteValue = ByteValue;
    result.IntegerValue = IntegerValue;
    result.FPValue = FPValue;
    result.FPVector2Value = FPVector2Value;
    result.FPVector3Value = FPVector3Value;
    converter.Convert(EntityRefValue, result.EntityRefValue);
    result.AssetRefValue = AssetRefValue;
}

void BlackboardEntry_Prototype::Convert(
    const EntityPrototypeConverter& converter,
    Quantum::Prototypes::BlackboardEntry_Prototype& result) const {
    Value.Convert(converter, result.Value);
    result.ReactiveDecorators = ReactiveDecorators;
}

void DictionaryEntry_Int32_BlackboardEntry_Prototype::Convert(
    const EntityPrototypeConverter& converter,
    Quantum::Prototypes::DictionaryEntry_Int32_BlackboardEntry_Prototype& result) const {
    result.Key = Key;
    Value.Convert(converter, result.Value);
}

std::shared_ptr<Quantum::Prototypes::AIBlackboardComponent_Prototype>
AIBlackboardComponent_Prototype::Convert(const EntityPrototypeConverter& converter) const {
    auto result = std::make_shared<Quantum::Prototypes::AIBlackboardComponent_Prototype>();
    result->Board = Board;
    result->Entries.resize(Entries.size());
    for (std::size_t i = 0; i < Entries.size(); ++i)
        Entries[i].Convert(converter, result->Entries[i]);
    return result;
}

std::shared_ptr<Quantum::Prototypes::BlastBallGoal_Prototype>
BlastBallGoal_Prototype::Convert(const EntityPrototypeConverter& converter) const {
    auto result = std::make_shared<Quantum::Prototypes::BlastBallGoal_Prototype>();
    result->OwnerTeam = OwnerTeam;
    converter.Convert(ballEntity, result->ballEntity);
    result->ballInside = ballInside;
    return result;
}

std::shared_ptr<Quantum::Prototypes::Hierarchy_Prototype>
Hierarchy_Prototype::Convert(const EntityPrototypeConverter& converter) const {
    auto result = std::make_shared<Quantum::Prototypes::Hierarchy_Prototype>();
    converter.Convert(Parent, result->Parent);
    result->Offset = Offset;
    return result;
}

std::shared_ptr<Quantum::Prototypes::LevelButton_Prototype>
LevelButton_Prototype::Convert(const EntityPrototypeConverter& converter) const {
    auto result = std::make_shared<Quantum::Prototypes::LevelButton_Prototype>();
    converter.Convert(gameplayElementEntity, result->gameplayElementEntity);
    result->Config = Config;
    return result;
}

std::shared_ptr<Quantum::Prototypes::RailCollider_Prototype>
RailCollider_Prototype::Convert(const EntityPrototypeConverter& converter) const {
    auto result = std::make_shared<Quantum::Prototypes::RailCollider_Prototype>();
    converter.Convert(railEntity, result->railEntity);
    result->distanceOnRail = distanceOnRail;
    converter.Convert(nearbyRailColLeft, result->nearbyRailColLeft);
    converter.Convert(nearbyRailColRight, result->nearbyRailColRight);
    return result;
}

// --- assets ----------------------------------------------------------------------------------

void PrepareEntityPrototypeAsset(
    Quantum::EntityPrototype& settings, EntityPrototype* parent, const ScriptKey& parentKey,
    const std::vector<std::unique_ptr<EntityComponentBase>>& components,
    const std::function<Quantum::AssetGuid(const std::string&)>& assetGuidOfPath) {
    using Photon::Deterministic::Log;
    if (parent == nullptr)
        return;
    parent->PreSerialize();

    ComponentPrototypeList prototypes;
    bool hasSelfView = false;
    parent->SerializeImplicitComponents(prototypes, hasSelfView);

    if (hasSelfView) {
        static const std::string kPrototypeSuffix = "EntityPrototype";
        const std::string& path = settings.Path;
        Quantum::AssetGuid viewGuid;
        if (path.size() >= kPrototypeSuffix.size() &&
            path.compare(path.size() - kPrototypeSuffix.size(), kPrototypeSuffix.size(),
                         kPrototypeSuffix) == 0) {
            const std::string viewPath =
                path.substr(0, path.size() - kPrototypeSuffix.size()) + "EntityView";
            viewGuid = assetGuidOfPath(viewPath);
        } else {
            Log::Error("Prototype asset paths are expected to end with {0}", kPrototypeSuffix);
        }
        if (viewGuid.IsValid()) {
            auto view = std::make_shared<View_Prototype>();
            view->Current.Id = viewGuid;
            prototypes.push_back(view);
        } else {
            Log::Error("The prefab of prototype {0} has an entity view, but its nested view asset "
                       "was not found",
                       path);
        }
    }

    const EntityPrototypeConverter converter(parentKey);
    for (const auto& component : components) {
        component->Refresh();
        prototypes.push_back(component->CreatePrototype(converter));
    }

    if (!settings.Container)
        settings.Container = std::make_shared<Quantum::EntityPrototypeContainer>();
    settings.Container->Components = std::move(prototypes);
    settings.Container->SetDirty();
}

void PrepareMapAsset(
    Quantum::Map& settings,
    const std::vector<Quantum::Prototypes::FlatEntityPrototypeContainer>* prototypes) {
    if (prototypes == nullptr)
        return;
    settings.MapEntities.clear();
    settings.MapEntities.reserve(prototypes->size());
    std::vector<std::shared_ptr<Quantum::ComponentPrototype>> buffer;
    for (const auto& flat : *prototypes) {
        flat.Collect(buffer);
        auto container = std::make_shared<Quantum::EntityPrototypeContainer>();
        container->Components = buffer;
        settings.MapEntities.push_back(std::move(container));
        buffer.clear();
    }
}

} // namespace rr::content::unity
