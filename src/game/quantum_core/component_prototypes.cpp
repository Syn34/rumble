// component_prototypes.cpp - prototypes of quantum.core's own components
// (Quantum.Prototypes, quantum.core.dll).
#include "quantum_core/component_prototypes.h"

#include "photon_deterministic/fp_quaternion.h"
#include "quantum_core/assert.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/physics3d/physics_engine3d.h"

#include <stdexcept>

namespace Quantum::Prototypes {

namespace {

// Body of the AddToEntity overrides that wait for their component types: they must not
// pretend to have added anything.
[[noreturn]] void NotImplemented(const ComponentPrototype& prototype) {
    throw std::logic_error(prototype.PrototypeTypeName() +
                           ": materialisation is not implemented yet");
}

} // namespace

// --- visitor ---------------------------------------------------------------------------------

void ComponentPrototypeVisitorBase::Visit(CharacterController2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(CharacterController3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(NavMeshAvoidanceAgent_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(NavMeshAvoidanceObstacle_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(NavMeshPathfinder_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(NavMeshSteeringAgent_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsBody2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsBody3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsCollider2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsJoints2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsJoints3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsCollider3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsCallbacks2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(PhysicsCallbacks3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(Transform2D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(Transform2DVertical_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(Transform3D_Prototype& prototype) {
    VisitFallback(prototype);
}
void ComponentPrototypeVisitorBase::Visit(View_Prototype& prototype) {
    VisitFallback(prototype);
}

void ComponentPrototypeVisitorBase::VisitFallback(ComponentPrototype& prototype) {
    throw std::logic_error("Component prototype visitor does not handle " +
                           prototype.PrototypeTypeName());
}

// --- transforms and view ---------------------------------------------------------------------

bool Transform2D_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext&) {
    Transform2D transform;
    Materialize(f, transform);
    return f->Set(entity, transform) == SetResult::ComponentAdded;
}

void Transform2D_Prototype::Materialize(Core::FrameBase*, Transform2D& result) const {
    result.Position = Position;
    result.Rotation = Rotation * FP::Deg2Rad();
}

bool Transform2DVertical_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                const PrototypeMaterializationContext&) {
    Transform2DVertical transform;
    Materialize(f, transform);
    return f->Set(entity, transform) == SetResult::ComponentAdded;
}

void Transform2DVertical_Prototype::Materialize(Core::FrameBase*,
                                                Transform2DVertical& result) const {
    result.Position = Position;
    result.Height = Height;
}

bool Transform3D_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                        const PrototypeMaterializationContext&) {
    Transform3D transform;
    Materialize(f, transform);
    return f->Set(entity, transform) == SetResult::ComponentAdded;
}

void Transform3D_Prototype::Materialize(Core::FrameBase*, Transform3D& result) const {
    result.Position = Position;
    result.Rotation = FPQuaternion::Euler(Rotation);
}

bool View_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                 const PrototypeMaterializationContext&) {
    View view;
    Materialize(f, view);
    return f->Set(entity, view) == SetResult::ComponentAdded;
}

void View_Prototype::Materialize(Core::FrameBase*, View& result) const {
    result.Current = Current;
}

// --- navigation ------------------------------------------------------------------------------

bool NavMeshAvoidanceAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                  const PrototypeMaterializationContext&) {
    return f->Set(entity, NavMeshAvoidanceAgent{}) == SetResult::ComponentAdded;
}

bool NavMeshAvoidanceObstacle_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                     const PrototypeMaterializationContext&) {
    NavMeshAvoidanceObstacle obstacle{};
    Materialize(f, obstacle);
    return f->Set(entity, obstacle) == SetResult::ComponentAdded;
}

void NavMeshAvoidanceObstacle_Prototype::Materialize(Core::FrameBase*,
                                                     NavMeshAvoidanceObstacle& result) const {
    result.SetRadius(Radius);
    result.SetIsActive(IsActive);
    result.SetAvoidanceLayer(AvoidanceLayer);
}

bool NavMeshPathfinder_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                              const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool NavMeshSteeringAgent_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                                 const PrototypeMaterializationContext&) {
    const AddResult result = f->Add<NavMeshSteeringAgent>(entity);
    Assert::Always(result != AddResult::EntityDoesNotExist);
    PostMaterialize(f, *f->Unsafe.GetPointer<NavMeshSteeringAgent>(entity));
    return result == AddResult::ComponentAdded;
}

void NavMeshSteeringAgent_Prototype::PostMaterialize(Core::FrameBase*,
                                                     NavMeshSteeringAgent& result) const {
    if (OverrideMaxSpeed) {
        result.SetMaxSpeed(MaxSpeed);
    }
    if (OverrideAcceleration) {
        result.SetAcceleration(Acceleration);
    }
}

// --- prototypes waiting for their component types -------------------------------------------

bool CharacterController2D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                                  const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool CharacterController3D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                                  const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool PhysicsBody2D_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext&) {
    PhysicsBody2D body{};
    Materialize(f, body);
    return f->Set(entity, body) == SetResult::ComponentAdded;
}

void PhysicsBody2D_Prototype::Materialize(Core::FrameBase*, PhysicsBody2D& result) const {
    result.InitBody(Mass, Drag, AngularDrag, Config, CenterOfMass,
                    GravityScale.HasValue() ? std::optional<FP>(GravityScale.Value()) : std::nullopt);
}

bool PhysicsBody3D_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                          const PrototypeMaterializationContext&) {
    PhysicsBody3D body{};
    Materialize(f, body);
    return f->Set(entity, body) == SetResult::ComponentAdded;
}

void PhysicsBody3D_Prototype::Materialize(Core::FrameBase*, PhysicsBody3D& result) const {
    result.InitBody(Mass, Drag, AngularDrag, Config, static_cast<RotationFreezeFlags>(RotationFreeze),
                    CenterOfMass,
                    GravityScale.HasValue() ? std::optional<FP>(GravityScale.Value()) : std::nullopt);
}

bool PhysicsCollider2D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                              const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool PhysicsCollider3D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                              const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool PhysicsCallbacks2D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                               const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

bool PhysicsCallbacks3D_Prototype::AddToEntity(Core::FrameBase* f, EntityRef entity,
                                               const PrototypeMaterializationContext&) {
    if (Physics3D::PhysicsEngine3D* physics = f->get_Context()->get_Physics3D()) {
        physics->SetCallbacks(f, entity, static_cast<::Quantum::CallbackFlags>(CallbackFlags));
    }
    return true;
}

bool PhysicsJoints2D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                            const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

void PhysicsJoints2D_Prototype::OnEntityPrototypeMaterialized(Core::FrameBase*, EntityRef,
                                                              EntityPrototypeRef) {
    NotImplemented(*this);
}

bool PhysicsJoints3D_Prototype::AddToEntity(Core::FrameBase*, EntityRef,
                                            const PrototypeMaterializationContext&) {
    NotImplemented(*this);
}

void PhysicsJoints3D_Prototype::OnEntityPrototypeMaterialized(Core::FrameBase*, EntityRef,
                                                              EntityPrototypeRef) {
    NotImplemented(*this);
}

} // namespace Quantum::Prototypes
