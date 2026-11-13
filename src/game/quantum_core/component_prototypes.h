// component_prototypes.h - the prototypes of quantum.core's own components
// (Quantum.Prototypes namespace of quantum.core.dll) and the visitor over them.
//
// Each prototype holds the editable description of one component and turns it into
// component data: Materialize fills a component value, AddToEntity stores it on an entity.
// AddToEntity returns true when the component was new; most prototypes store with Set, so
// an existing component is overwritten (the caller then reports it, see
// FrameBase::MaterializePrototypeInternal).
//
// The physics prototypes are complete as data but their Materialize/AddToEntity wait for
// the physics component types (written separately); they identify their component by its
// registered name until then.
#pragma once

#include "photon_deterministic/fp.h"
#include "photon_deterministic/fp_vector2.h"
#include "photon_deterministic/fp_vector3.h"
#include "photon_deterministic/nullable_fp.h"
#include "photon_deterministic/nullable_fp_vector3.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/entity_prototype_ref.h"
#include "quantum_core/map_entity_id.h"
#include "quantum_core/navigation/navmesh.h"
#include "quantum_core/navigation/navmesh_agent_config.h"
#include "quantum_core/navigation/navmesh_components.h"
#include "quantum_core/transform.h"
#include "quantum_core/view.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Quantum {

class Shape2DConfig;
class Shape3DConfig;

namespace Prototypes {

using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::NullableFP;
using Photon::Deterministic::NullableFPVector3;

class CharacterController2D_Prototype;
class CharacterController3D_Prototype;
class NavMeshAvoidanceAgent_Prototype;
class NavMeshAvoidanceObstacle_Prototype;
class NavMeshPathfinder_Prototype;
class NavMeshSteeringAgent_Prototype;
class PhysicsBody2D_Prototype;
class PhysicsBody3D_Prototype;
class PhysicsCollider2D_Prototype;
class PhysicsJoints2D_Prototype;
class PhysicsJoints3D_Prototype;
class PhysicsCollider3D_Prototype;
class PhysicsCallbacks2D_Prototype;
class PhysicsCallbacks3D_Prototype;
class Transform2D_Prototype;
class Transform2DVertical_Prototype;
class Transform3D_Prototype;
class View_Prototype;

// Double dispatch over the framework's component prototypes. Every Visit defaults to
// VisitFallback, which throws: a visitor overrides the overloads it handles.
class ComponentPrototypeVisitorBase {
  public:
    // 0x7d7e80c
    ComponentPrototypeVisitorBase() = default;
    virtual ~ComponentPrototypeVisitorBase() = default;

    // 0x7d7e68c
    virtual void Visit(CharacterController2D_Prototype& prototype);
    // 0x7d7e69c
    virtual void Visit(CharacterController3D_Prototype& prototype);
    // 0x7d7e6ac
    virtual void Visit(NavMeshAvoidanceAgent_Prototype& prototype);
    // 0x7d7e6bc
    virtual void Visit(NavMeshAvoidanceObstacle_Prototype& prototype);
    // 0x7d7e6cc
    virtual void Visit(NavMeshPathfinder_Prototype& prototype);
    // 0x7d7e6dc
    virtual void Visit(NavMeshSteeringAgent_Prototype& prototype);
    // 0x7d7e6ec
    virtual void Visit(PhysicsBody2D_Prototype& prototype);
    // 0x7d7e6fc
    virtual void Visit(PhysicsBody3D_Prototype& prototype);
    // 0x7d7e70c
    virtual void Visit(PhysicsCollider2D_Prototype& prototype);
    // 0x7d7e71c
    virtual void Visit(PhysicsJoints2D_Prototype& prototype);
    // 0x7d7e72c
    virtual void Visit(PhysicsJoints3D_Prototype& prototype);
    // 0x7d7e73c
    virtual void Visit(PhysicsCollider3D_Prototype& prototype);
    // 0x7d7e74c
    virtual void Visit(PhysicsCallbacks2D_Prototype& prototype);
    // 0x7d7e75c
    virtual void Visit(PhysicsCallbacks3D_Prototype& prototype);
    // 0x7d7e76c
    virtual void Visit(Transform2D_Prototype& prototype);
    // 0x7d7e77c
    virtual void Visit(Transform2DVertical_Prototype& prototype);
    // 0x7d7e78c
    virtual void Visit(Transform3D_Prototype& prototype);
    // 0x7d7e79c
    virtual void Visit(View_Prototype& prototype);

    // 0x7d7e7ac - Throws std::logic_error naming the prototype (NotImplementedException in
    // the original).
    virtual void VisitFallback(ComponentPrototype& prototype);
};

// --- transforms and view ---------------------------------------------------------------------

class Transform2D_Prototype final : public TypedComponentPrototype<Transform2D> {
  public:
    FPVector2 Position;
    FP Rotation; // degrees

    // 0x7d80564
    Transform2D_Prototype() = default;

    // 0x7d80454 - Set; true when the transform is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d8051c - the rotation converted to radians (FP multiplication by Deg2Rad).
    void Materialize(Core::FrameBase* frame, Transform2D& result) const;
    // 0x7d8053c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class Transform2DVertical_Prototype final : public TypedComponentPrototype<Transform2DVertical> {
  public:
    FP Position;
    FP Height;

    // 0x7d80104
    Transform2DVertical_Prototype() = default;

    // 0x7d80054 - Set; true when the component is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d800c8
    void Materialize(Core::FrameBase* frame, Transform2DVertical& result) const;
    // 0x7d800dc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class Transform3D_Prototype final : public TypedComponentPrototype<Transform3D> {
  public:
    FPVector3 Position;
    FPVector3 Rotation; // Euler angles in degrees

    // 0x7d8040c
    Transform3D_Prototype() = default;

    // 0x7d8026c - Set; true when the transform is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d80364 - the rotation through FPQuaternion::Euler.
    void Materialize(Core::FrameBase* frame, Transform3D& result) const;
    // 0x7d803e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class View_Prototype final : public TypedComponentPrototype<View> {
  public:
    AssetRefEntityView Current;

    // 0x7d7fd58
    View_Prototype() = default;

    // 0x7d7fcbc - Set; true when the view is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7fcb0
    void Materialize(Core::FrameBase* frame, View& result) const;
    // 0x7d7fd30
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

// --- navigation ------------------------------------------------------------------------------

class NavMeshAvoidanceAgent_Prototype final
    : public TypedComponentPrototype<NavMeshAvoidanceAgent> {
  public:
    // 0x7d80224
    NavMeshAvoidanceAgent_Prototype() = default;

    // 0x7d8014c - Sets a zeroed agent (its OnAdded callback fills it in from the
    // pathfinder's config); true when it is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d80200
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class NavMeshAvoidanceObstacle_Prototype final
    : public TypedComponentPrototype<NavMeshAvoidanceObstacle> {
  public:
    std::uint8_t AvoidanceLayer = 0;
    FP Radius = FP::FromRaw(FP::Raw{1} << 14); // 0.25
    bool IsActive = true;

    // 0x7d7e950
    NavMeshAvoidanceObstacle_Prototype() = default;

    // 0x7d7e814 - Set of a materialised obstacle (velocity zero); true when it is new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7e8f0 - radius, active flag and avoidance layer; the velocity is left as is.
    void Materialize(Core::FrameBase* frame, NavMeshAvoidanceObstacle& result) const;
    // 0x7d7e92c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class NavMeshPathfinder_Prototype final : public PendingComponentPrototype {
  public:
    AssetRefNavMeshAgentConfig AgentConfig;
    NullableFPVector3 InitialTarget;
    AssetRefNavMesh InitialTargetNavMesh;
    std::string InitialTargetNavMeshName;

    // 0x7d7f0d8 - (The component struct exists, but its frame methods are still TODO, so
    // the prototype resolves the type by name like the pending ones.)
    NavMeshPathfinder_Prototype() : PendingComponentPrototype("NavMeshPathfinder") {}

    // TODO(0x7d7ecd8, Quantum.Prototypes.NavMeshPathfinder_Prototype.AddToEntity): builds the
    // component with NavMeshPathfinder.Create (AgentConfig looked up in the frame), applies
    // Materialize and stores it with Set; true when new. Needs NavMeshPathfinder.Create.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // TODO(0x7d7edf4, Quantum.Prototypes.NavMeshPathfinder_Prototype.Materialize): with an
    // InitialTarget, finds the navmesh (by InitialTargetNavMesh, else by name in the
    // frame's map; an error is logged when missing) and calls NavMeshPathfinder.SetTarget.
    // Needs SetTarget.
    // 0x7d7f0b4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class NavMeshSteeringAgent_Prototype final : public TypedComponentPrototype<NavMeshSteeringAgent> {
  public:
    bool OverrideMaxSpeed = false;
    bool OverrideAcceleration = false;
    FP MaxSpeed;
    FP Acceleration;

    // 0x7d7ec90
    NavMeshSteeringAgent_Prototype() = default;

    // 0x7d7eb48 - Adds a zeroed agent if missing (an existing one is kept, not reset),
    // then applies the overrides to the entity's agent either way. True when the agent was
    // new. Throws AssertException for a missing entity.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7ec18 - the overridden speed/acceleration (negative values throw, see
    // NullableNonNegativeFP).
    void PostMaterialize(Core::FrameBase* frame, NavMeshSteeringAgent& result) const;
    // 0x7d7ec6c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

// --- character controllers -------------------------------------------------------------------

class CharacterController2D_Prototype final : public PendingComponentPrototype {
  public:
    AssetRefCharacterController2DConfig Config;

    // 0x7d7eb00
    CharacterController2D_Prototype() : PendingComponentPrototype("CharacterController2D") {}

    // TODO(0x7d7e9a8, Quantum.Prototypes.CharacterController2D_Prototype.Materialize): looks up
    // Config in the frame and calls CharacterController2D.Init with the thread-safe frame;
    // needs the CharacterController2D component.
    // TODO(0x7d7ea04, Quantum.Prototypes.CharacterController2D_Prototype.AddToEntity):
    // Materialize into a zeroed component, then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7eadc
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class CharacterController3D_Prototype final : public PendingComponentPrototype {
  public:
    AssetRefCharacterController3DConfig Config;

    // 0x7d7fc28
    CharacterController3D_Prototype() : PendingComponentPrototype("CharacterController3D") {}

    // TODO(0x7d7fad0, Quantum.Prototypes.CharacterController3D_Prototype.Materialize): as the
    // 2D one with CharacterController3D.Init.
    // TODO(0x7d7fb2c, Quantum.Prototypes.CharacterController3D_Prototype.AddToEntity):
    // Materialize into a zeroed component, then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7fc04
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

// --- physics bodies, colliders, callbacks ----------------------------------------------------

class PhysicsBody2D_Prototype final : public PendingComponentPrototype {
  public:
    static constexpr int BODY_PROTOTYPE_VERSION = 2;

    std::int32_t Version = 0;
    // PhysicsBody2D.ConfigFlagsWrapper (the body's ConfigFlags as a byte).
    std::uint8_t Config = 0x74;
    FP Mass = FP::_1();
    FP Drag = FP::FromRaw(FP::Raw{1} << 15);        // 0.5
    FP AngularDrag = FP::FromRaw(FP::Raw{1} << 15); // 0.5
    FPVector2 CenterOfMass;
    // Note the default: no value, but a stored value of 1.
    NullableFP GravityScale{0, FP::_1()};

    // 0x7d7fa48
    PhysicsBody2D_Prototype() : PendingComponentPrototype("PhysicsBody2D") {}

    // TODO(0x7d7f860, Quantum.Prototypes.PhysicsBody2D_Prototype.Materialize): PhysicsBody2D
    // initialised from mass, drag, angular drag, config flags, centre of mass and gravity
    // scale; needs the PhysicsBody2D component.
    // TODO(0x7d7f93c, Quantum.Prototypes.PhysicsBody2D_Prototype.AddToEntity): Materialize,
    // then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7fa24
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsBody3D_Prototype final : public PendingComponentPrototype {
  public:
    static constexpr int BODY_PROTOTYPE_VERSION = 2;

    std::int32_t Version = 0;
    // PhysicsBody3D.ConfigFlagsWrapper (the body's ConfigFlags as a byte; 0x3A is Default).
    std::uint8_t Config = 0x3A;
    FP Mass = FP::_1();
    FP Drag = FP::FromRaw(FP::Raw{1} << 15);        // 0.5
    FP AngularDrag = FP::FromRaw(FP::Raw{1} << 15); // 0.5
    FPVector3 CenterOfMass;
    // Quantum.RotationFreezeFlags_Wrapper (RotationFreezeFlags as a byte).
    std::uint8_t RotationFreeze = 0;
    // Note the default: no value, but a stored value of 1.
    NullableFP GravityScale{0, FP::_1()};

    // 0x7d7f314
    PhysicsBody3D_Prototype() : PendingComponentPrototype("PhysicsBody3D") {}

    // TODO(0x7d7f120, Quantum.Prototypes.PhysicsBody3D_Prototype.Materialize):
    // PhysicsBody3D.InitBody with mass, drag, angular drag, config flags, rotation freeze,
    // centre of mass and gravity scale; needs the PhysicsBody3D component.
    // TODO(0x7d7f210, Quantum.Prototypes.PhysicsBody3D_Prototype.AddToEntity): Materialize,
    // then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7f2f0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsCollider2D_Prototype final : public PendingComponentPrototype {
  public:
    std::shared_ptr<Shape2DConfig> ShapeConfig;
    std::int32_t Layer = 0;
    bool IsTrigger = false;
    AssetRefPhysicsMaterial PhysicsMaterial;

    // 0x7d8000c
    PhysicsCollider2D_Prototype() : PendingComponentPrototype("PhysicsCollider2D") {}

    // TODO(0x7d7fda0, Quantum.Prototypes.PhysicsCollider2D_Prototype.Materialize): creates the
    // shape (Shape2DConfig.CreateShape), looks up the material and builds the collider with
    // PhysicsCollider2D.Create (layer, trigger flag); needs the collider and shape config
    // types.
    // TODO(0x7d7ff0c, Quantum.Prototypes.PhysicsCollider2D_Prototype.AddToEntity):
    // Materialize, then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7ffe8
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsCollider3D_Prototype final : public PendingComponentPrototype {
  public:
    std::shared_ptr<Shape3DConfig> ShapeConfig;
    std::int32_t Layer = 0;
    bool IsTrigger = false;
    AssetRefPhysicsMaterial PhysicsMaterial;

    // 0x7d7f818
    PhysicsCollider3D_Prototype() : PendingComponentPrototype("PhysicsCollider3D") {}

    // TODO(0x7d7f594, Quantum.Prototypes.PhysicsCollider3D_Prototype.Materialize): creates the
    // shape (Shape3DConfig.CreateShape), looks up the material, builds the collider with
    // PhysicsCollider3D.Create and frees the temporary compound shape buffer; needs the
    // collider and shape config types.
    // TODO(0x7d7f708, Quantum.Prototypes.PhysicsCollider3D_Prototype.AddToEntity):
    // Materialize, then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7f7f0
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsCallbacks2D_Prototype final : public PendingComponentPrototype {
  public:
    // Quantum.CallbackFlags_Wrapper (CallbackFlags as an int).
    std::int32_t CallbackFlags = 0;

    // 0x7d7f4b0
    PhysicsCallbacks2D_Prototype() : PendingComponentPrototype("PhysicsCallbacks2D") {}

    // 0x7d7f3dc - PhysicsCallbacks2D (by name until its struct exists).
    const std::type_info* get_ComponentType() const override { return nullptr; }

    // TODO(0x7d7f43c, Quantum.Prototypes.PhysicsCallbacks2D_Prototype.AddToEntity): when the
    // frame has a 2D physics state, enables CallbackFlags for the entity through the
    // frame's Physics2D API; always true. Needs the physics engine.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7f488
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsCallbacks3D_Prototype final : public PendingComponentPrototype {
  public:
    // Quantum.CallbackFlags_Wrapper (CallbackFlags as an int).
    std::int32_t CallbackFlags = 0;

    // 0x7d7f58c
    PhysicsCallbacks3D_Prototype() : PendingComponentPrototype("PhysicsCallbacks3D") {}

    // 0x7d7f4b8 - PhysicsCallbacks3D (by name until its struct exists).
    const std::type_info* get_ComponentType() const override { return nullptr; }

    // TODO(0x7d7f518, Quantum.Prototypes.PhysicsCallbacks3D_Prototype.AddToEntity): as the 2D
    // one with the Physics3D API.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // 0x7d7f564
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

// --- joints ----------------------------------------------------------------------------------

// One joint of a PhysicsJoints2D prototype. JointType is Quantum.Physics2D.JointType
// (distance, spring, hinge), kept as its raw 16-bit value.
class Joint2D_Prototype final : public StructPrototype {
  public:
    bool StartDisabled = false;
    std::uint16_t JointType = 0;
    std::int32_t UserTag = 0;
    MapEntityId ConnectedEntity;
    FPVector2 ConnectedAnchor;
    FPVector2 Anchor;
    FP Frequency;
    FP DampingRatio;
    bool AutoConfigureDistance = true;
    FP Distance;
    FP MinDistance;
    FP MaxDistance;
    bool UseAngleLimits = false;
    FP LowerAngle;
    FP UpperAngle;
    bool UseMotor = false;
    FP MotorSpeed;
    NullableFP MaxMotorTorque;

    // 0x7d7da9c
    Joint2D_Prototype() = default;

    // TODO(0x7d7d6c4, Quantum.Prototypes.Joint2D_Prototype.Materialize): resolves
    // ConnectedEntity (PrototypeValidator::FindMapEntity) and builds the joint by type
    // (DistanceJoint / SpringJoint with ComputeStiffness and auto-configured distance from
    // the two transforms / HingeJoint with limits and motor); needs the Physics2D joint
    // types.
    // TODO(0x7d7d994, Quantum.Prototypes.Joint2D_Prototype.OnEntityPrototypeMaterializedJoint):
    // for spring joints, recomputes the stiffness from the now-existing bodies of both
    // entities; same dependency.
};

// One joint of a PhysicsJoints3D prototype. JointType is Quantum.Physics3D.JointType3D,
// kept as its raw 16-bit value.
class Joint3D_Prototype final : public StructPrototype {
  public:
    bool StartDisabled = false;
    std::uint16_t JointType = 0;
    std::int32_t UserTag = 0;
    MapEntityId ConnectedEntity;
    FPVector3 ConnectedAnchor;
    FPVector3 Anchor;
    FPVector3 Axis;
    FP Frequency;
    FP DampingRatio;
    bool AutoConfigureDistance = true;
    FP Distance;
    FP MinDistance;
    FP MaxDistance;
    bool UseAngleLimits = false;
    FP LowerAngle;
    FP UpperAngle;
    bool UseMotor = false;
    FP MotorSpeed;
    NullableFP MaxMotorTorque;

    // 0x7d7dfc4
    Joint3D_Prototype() = default;

    // TODO(0x7d7daac, Quantum.Prototypes.Joint3D_Prototype.Materialize): as the 2D one with
    // the Physics3D joint types (plus the hinge axis).
    // TODO(0x7d7de58, Quantum.Prototypes.Joint3D_Prototype.OnEntityPrototypeMaterializedJoint):
    // as the 2D one.
};

class PhysicsJoints2D_Prototype final : public PendingComponentPrototype,
                                        public IEntityPrototypeMaterializedListener {
  public:
    std::vector<Joint2D_Prototype> JointConfigs;

    // 0x7d7e60c - no joints.
    PhysicsJoints2D_Prototype() : PendingComponentPrototype("PhysicsJoints2D") {}

    // TODO(0x7d7e334, Quantum.Prototypes.PhysicsJoints2D_Prototype.AddToEntity): Materialize
    // into a zeroed component, then Set; true when new.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // TODO(0x7d7e3cc, Quantum.Prototypes.PhysicsJoints2D_Prototype.Materialize): materialises
    // every joint config and adds it with PhysicsJoints2D.AddJoint; needs PhysicsJoints2D.
    // TODO(0x7d7e4d4, Quantum.Prototypes.PhysicsJoints2D_Prototype.OnEntityPrototypeMaterialized):
    // walks the entity's joints alongside JointConfigs and lets each config finish its
    // joint (OnEntityPrototypeMaterializedJoint); same dependency.
    void OnEntityPrototypeMaterialized(Core::FrameBase* f, EntityRef entity,
                                       EntityPrototypeRef prototypeRef) override;
    // 0x7d7e5e4
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

class PhysicsJoints3D_Prototype final : public PendingComponentPrototype,
                                        public IEntityPrototypeMaterializedListener {
  public:
    std::vector<Joint3D_Prototype> JointConfigs;

    // 0x7d7e2b4 - no joints.
    PhysicsJoints3D_Prototype() : PendingComponentPrototype("PhysicsJoints3D") {}

    // TODO(0x7d7dfd4, Quantum.Prototypes.PhysicsJoints3D_Prototype.AddToEntity): as the 2D
    // one.
    bool AddToEntity(Core::FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override;
    // TODO(0x7d7e06c, Quantum.Prototypes.PhysicsJoints3D_Prototype.Materialize): as the 2D
    // one with PhysicsJoints3D.
    // TODO(0x7d7e17c, Quantum.Prototypes.PhysicsJoints3D_Prototype.OnEntityPrototypeMaterialized):
    // as the 2D one.
    void OnEntityPrototypeMaterialized(Core::FrameBase* f, EntityRef entity,
                                       EntityPrototypeRef prototypeRef) override;
    // 0x7d7e28c
    void Dispatch(ComponentPrototypeVisitorBase& visitor) override { visitor.Visit(*this); }
};

// --- references to component prototypes ------------------------------------------------------

// Prototype of a ComponentPrototypeRef field: the entity prototype and the component's
// short type name, resolved by the generated code of the game.
struct ComponentPrototypeRef_Prototype {
    ::Quantum::EntityPrototypeRef EntityPrototypeRef;
    std::string ComponentTypeShortName;
};

} // namespace Prototypes
} // namespace Quantum
