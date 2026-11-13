// framework_prototype_readers.cpp - reads quantum.core's component prototypes (transforms,
// view, navigation, character controllers, physics) from decoded content data (PC-side
// glue; new code).
//
// Shapes: wrapper structs store their value under `Value` (ConfigFlagsWrapper,
// RotationFreezeFlags_Wrapper, CallbackFlags_Wrapper); nullable values are
// { _hasValue, _value }.
#include "content/prototype_readers.h"

#include "photon_deterministic/nullable_fp.h"
#include "photon_deterministic/nullable_fp_vector3.h"
#include "quantum_code/components.h"
#include "quantum_core/component_prototypes.h"
#include "quantum_core/shape3d_config.h"

#include <unordered_map>

namespace rr::content {

using namespace Quantum::Prototypes;

namespace {

// Reads `{ Value: n }` wrappers into the integer they wrap.
template <class T> void ReadWrapped(const Value& parent, std::string_view name, T& out) {
    if (const Value* wrapper = Child(parent, name))
        Read(*wrapper, "Value", out);
}

} // namespace

void ReadFields(const Value& value, Quantum::MapEntityId& out, ReaderTag) {
    Read(value, "SceneIndexPlusOne", out.SceneIndexPlusOne);
}

void ReadFields(const Value& value, Photon::Deterministic::NullableFP& out, ReaderTag) {
    Read(value, "_hasValue", out._hasValue);
    Read(value, "_value", out._value);
}

void ReadFields(const Value& value, Photon::Deterministic::NullableFPVector3& out, ReaderTag) {
    Read(value, "_hasValue", out._hasValue);
    Read(value, "_value", out._value);
}

void ReadFields(const Value& v, Quantum::SpawnConfig& out, ReaderTag) {
    Read(v, "findPositionAttempts", out.findPositionAttempts);
    Read(v, "useRandomSpawnTransform", out.useRandomSpawnTransform);
    Read(v, "fallbackYOffset", out.fallbackYOffset);
    Read(v, "freePositionRadius", out.freePositionRadius);
    Read(v, "spawnPositionOffsetDistance", out.spawnPositionOffsetDistance);
}

void ReadFields(const Value& v, Transform2D_Prototype& out, ReaderTag) {
    Read(v, "Position", out.Position);
    Read(v, "Rotation", out.Rotation);
}

void ReadFields(const Value& v, Transform2DVertical_Prototype& out, ReaderTag) {
    Read(v, "Position", out.Position);
    Read(v, "Height", out.Height);
}

void ReadFields(const Value& v, Transform3D_Prototype& out, ReaderTag) {
    Read(v, "Position", out.Position);
    Read(v, "Rotation", out.Rotation);
}

void ReadFields(const Value& v, View_Prototype& out, ReaderTag) {
    Read(v, "Current", out.Current);
}

void ReadFields(const Value&, NavMeshAvoidanceAgent_Prototype&, ReaderTag) {}

void ReadFields(const Value& v, NavMeshAvoidanceObstacle_Prototype& out, ReaderTag) {
    Read(v, "AvoidanceLayer", out.AvoidanceLayer);
    Read(v, "Radius", out.Radius);
    Read(v, "IsActive", out.IsActive);
}

void ReadFields(const Value& v, NavMeshPathfinder_Prototype& out, ReaderTag) {
    Read(v, "AgentConfig", out.AgentConfig);
    Read(v, "InitialTarget", out.InitialTarget);
    Read(v, "InitialTargetNavMesh", out.InitialTargetNavMesh);
    Read(v, "InitialTargetNavMeshName", out.InitialTargetNavMeshName);
}

void ReadFields(const Value& v, NavMeshSteeringAgent_Prototype& out, ReaderTag) {
    Read(v, "OverrideMaxSpeed", out.OverrideMaxSpeed);
    Read(v, "OverrideAcceleration", out.OverrideAcceleration);
    Read(v, "MaxSpeed", out.MaxSpeed);
    Read(v, "Acceleration", out.Acceleration);
}

void ReadFields(const Value& v, CharacterController2D_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}

void ReadFields(const Value& v, CharacterController3D_Prototype& out, ReaderTag) {
    Read(v, "Config", out.Config);
}

void ReadFields(const Value& v, PhysicsBody2D_Prototype& out, ReaderTag) {
    Read(v, "Version", out.Version);
    ReadWrapped(v, "Config", out.Config);
    Read(v, "Mass", out.Mass);
    Read(v, "Drag", out.Drag);
    Read(v, "AngularDrag", out.AngularDrag);
    Read(v, "CenterOfMass", out.CenterOfMass);
    Read(v, "GravityScale", out.GravityScale);
}

void ReadFields(const Value& v, PhysicsBody3D_Prototype& out, ReaderTag) {
    Read(v, "Version", out.Version);
    ReadWrapped(v, "Config", out.Config);
    Read(v, "Mass", out.Mass);
    Read(v, "Drag", out.Drag);
    Read(v, "AngularDrag", out.AngularDrag);
    Read(v, "CenterOfMass", out.CenterOfMass);
    ReadWrapped(v, "RotationFreeze", out.RotationFreeze);
    Read(v, "GravityScale", out.GravityScale);
}

void ReadFields(const Value& v, PhysicsCollider2D_Prototype& out, ReaderTag) {
    // The 2D shape configuration has no C++ type yet (the game's maps and prefabs use 3D
    // physics only), so ShapeConfig stays null.
    Read(v, "Layer", out.Layer);
    Read(v, "IsTrigger", out.IsTrigger);
    Read(v, "PhysicsMaterial", out.PhysicsMaterial);
}

void ReadFields(const Value& v, PhysicsCollider3D_Prototype& out, ReaderTag) {
    if (const Value* shape = Child(v, "ShapeConfig")) {
        out.ShapeConfig = std::make_shared<Quantum::Shape3DConfig>();
        ReadValue(*shape, *out.ShapeConfig);
    }
    Read(v, "Layer", out.Layer);
    Read(v, "IsTrigger", out.IsTrigger);
    Read(v, "PhysicsMaterial", out.PhysicsMaterial);
}

void ReadFields(const Value& v, PhysicsCallbacks2D_Prototype& out, ReaderTag) {
    ReadWrapped(v, "CallbackFlags", out.CallbackFlags);
}

void ReadFields(const Value& v, PhysicsCallbacks3D_Prototype& out, ReaderTag) {
    ReadWrapped(v, "CallbackFlags", out.CallbackFlags);
}

void ReadFields(const Value& v, Joint2D_Prototype& out, ReaderTag) {
    Read(v, "StartDisabled", out.StartDisabled);
    Read(v, "JointType", out.JointType);
    Read(v, "UserTag", out.UserTag);
    Read(v, "ConnectedEntity", out.ConnectedEntity);
    Read(v, "ConnectedAnchor", out.ConnectedAnchor);
    Read(v, "Anchor", out.Anchor);
    Read(v, "Frequency", out.Frequency);
    Read(v, "DampingRatio", out.DampingRatio);
    Read(v, "AutoConfigureDistance", out.AutoConfigureDistance);
    Read(v, "Distance", out.Distance);
    Read(v, "MinDistance", out.MinDistance);
    Read(v, "MaxDistance", out.MaxDistance);
    Read(v, "UseAngleLimits", out.UseAngleLimits);
    Read(v, "LowerAngle", out.LowerAngle);
    Read(v, "UpperAngle", out.UpperAngle);
    Read(v, "UseMotor", out.UseMotor);
    Read(v, "MotorSpeed", out.MotorSpeed);
    Read(v, "MaxMotorTorque", out.MaxMotorTorque);
}

void ReadFields(const Value& v, Joint3D_Prototype& out, ReaderTag) {
    Read(v, "StartDisabled", out.StartDisabled);
    Read(v, "JointType", out.JointType);
    Read(v, "UserTag", out.UserTag);
    Read(v, "ConnectedEntity", out.ConnectedEntity);
    Read(v, "ConnectedAnchor", out.ConnectedAnchor);
    Read(v, "Anchor", out.Anchor);
    Read(v, "Axis", out.Axis);
    Read(v, "Frequency", out.Frequency);
    Read(v, "DampingRatio", out.DampingRatio);
    Read(v, "AutoConfigureDistance", out.AutoConfigureDistance);
    Read(v, "Distance", out.Distance);
    Read(v, "MinDistance", out.MinDistance);
    Read(v, "MaxDistance", out.MaxDistance);
    Read(v, "UseAngleLimits", out.UseAngleLimits);
    Read(v, "LowerAngle", out.LowerAngle);
    Read(v, "UpperAngle", out.UpperAngle);
    Read(v, "UseMotor", out.UseMotor);
    Read(v, "MotorSpeed", out.MotorSpeed);
    Read(v, "MaxMotorTorque", out.MaxMotorTorque);
}

void ReadFields(const Value& v, PhysicsJoints2D_Prototype& out, ReaderTag) {
    Read(v, "JointConfigs", out.JointConfigs);
}

void ReadFields(const Value& v, PhysicsJoints3D_Prototype& out, ReaderTag) {
    Read(v, "JointConfigs", out.JointConfigs);
}

namespace {

template <class T> std::shared_ptr<Quantum::ComponentPrototype> Make(const Value& value) {
    auto prototype = std::make_shared<T>();
    ReadValue(value, *prototype);
    return prototype;
}

using Maker = std::shared_ptr<Quantum::ComponentPrototype> (*)(const Value&);

} // namespace

std::shared_ptr<Quantum::ComponentPrototype>
ReadFrameworkComponentPrototype(const std::string& typeName, const Value& value) {
    static const std::unordered_map<std::string, Maker> makers = {
        {"Transform2D_Prototype", &Make<Transform2D_Prototype>},
        {"Transform2DVertical_Prototype", &Make<Transform2DVertical_Prototype>},
        {"Transform3D_Prototype", &Make<Transform3D_Prototype>},
        {"View_Prototype", &Make<View_Prototype>},
        {"NavMeshAvoidanceAgent_Prototype", &Make<NavMeshAvoidanceAgent_Prototype>},
        {"NavMeshAvoidanceObstacle_Prototype", &Make<NavMeshAvoidanceObstacle_Prototype>},
        {"NavMeshPathfinder_Prototype", &Make<NavMeshPathfinder_Prototype>},
        {"NavMeshSteeringAgent_Prototype", &Make<NavMeshSteeringAgent_Prototype>},
        {"CharacterController2D_Prototype", &Make<CharacterController2D_Prototype>},
        {"CharacterController3D_Prototype", &Make<CharacterController3D_Prototype>},
        {"PhysicsBody2D_Prototype", &Make<PhysicsBody2D_Prototype>},
        {"PhysicsBody3D_Prototype", &Make<PhysicsBody3D_Prototype>},
        {"PhysicsCollider2D_Prototype", &Make<PhysicsCollider2D_Prototype>},
        {"PhysicsCollider3D_Prototype", &Make<PhysicsCollider3D_Prototype>},
        {"PhysicsCallbacks2D_Prototype", &Make<PhysicsCallbacks2D_Prototype>},
        {"PhysicsCallbacks3D_Prototype", &Make<PhysicsCallbacks3D_Prototype>},
        {"PhysicsJoints2D_Prototype", &Make<PhysicsJoints2D_Prototype>},
        {"PhysicsJoints3D_Prototype", &Make<PhysicsJoints3D_Prototype>},
    };
    auto it = makers.find(typeName);
    return it != makers.end() ? it->second(value) : nullptr;
}

std::shared_ptr<Quantum::ComponentPrototype> ReadComponentPrototype(const std::string& typeName,
                                                                    const Value& value) {
    if (auto prototype = ReadGameComponentPrototype(typeName, value))
        return prototype;
    return ReadFrameworkComponentPrototype(typeName, value);
}

} // namespace rr::content
