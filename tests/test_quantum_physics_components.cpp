// Tests for the physics components' own behaviour (bodies, colliders, controllers).
#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/physics_components.h"

using namespace Quantum;

namespace {
FP F(int v) { return FP::FromInt(v); }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }
} // namespace

TEST_CASE("Quantum PhysicsBody3D init, impulses and forces") {
    rr::test::InstallMathDerivedTables();
    PhysicsBody3D body{};
    body.InitBody(F(2), FP::_0_50(), FP::_0_25(), PhysicsBody3D::ConfigFlags::Default,
                  RotationFreezeFlags::None, {}, {});
    CHECK(body.Enabled() && !body.IsSleeping() && !body.GetIsKinematic());
    CHECK(body.MassInverse() == FP::_0_50());
    CHECK(body.GravityScale() == F(1));
    CHECK(body.Drag() == FP::_0_50() && body.AngularDrag() == FP::_0_25());
    CHECK(FPQuaternion(body._lastRotation) == FPQuaternion::Identity());

    body.AddLinearImpulse(V3(4, 0, 0), V3(0, 1, 0));
    CHECK(body.Velocity == V3(2, 0, 0));
    // Cross((0,1,0), (4,0,0)) = (0, 0, -4)
    CHECK(body.AngularImpulse() == V3(0, 0, -4));

    body.AddForce(V3(0, 3, 0), V3(1, 0, 0));
    CHECK(body.Force() == V3(0, 3, 0) && body.Torque() == V3(0, 0, 3));
    body.ClearForce();
    body.ClearTorque();
    CHECK(body.Force() == FPVector3::Zero() && body.Torque() == FPVector3::Zero());
    CHECK(body.AngularImpulse() == FPVector3::Zero());

    body.Sleep();
    CHECK(body.IsSleeping());
    body.AddForce(V3(1, 0, 0)); // Default config is awakened by forces
    CHECK(!body.IsSleeping());

    // Zero mass: kinematic, forces ignored.
    PhysicsBody3D kinematic{};
    kinematic.InitBody(F(0), F(0), F(0), 0, RotationFreezeFlags::None, {}, {});
    CHECK(kinematic.GetIsKinematic() && kinematic.MassInverse() == F(0));
    kinematic.AddLinearImpulse(V3(1, 0, 0));
    CHECK(kinematic.Velocity == FPVector3::Zero());
    kinematic.SetIsKinematic(false); // infinite mass stays kinematic
    CHECK(kinematic.GetIsKinematic());

    // Diagonal inertia, identity rotation: the world tensor is the diagonal.
    body._inertiaTensorInverse = Physics3D::FPVector3RawInt::FromFPVector3(V3(1, 2, 3));
    FPMatrix3x3 m;
    body.GetWorldInertiaTensorInverse(FPQuaternion::Identity(), m);
    CHECK(m.M00 == F(1) && m.M11 == F(2) && m.M22 == F(3) && m.M01 == F(0));
}

TEST_CASE("Quantum PhysicsBody2D, colliders and controllers") {
    PhysicsBody2D body{};
    body.InitBody(F(4), F(0), F(0), PhysicsBody2D::ConfigFlags::Default, {}, F(2));
    CHECK(body.MassInverse() == FP::_0_25() && body.Mass() == F(4));
    CHECK(body.GravityScale() == F(2));
    body.SetIsKinematic(true);
    CHECK(body.GetIsKinematic());
    body.SetIsKinematic(false);
    CHECK(!body.GetIsKinematic());

    PhysicsCollider3D collider{};
    collider.SetLayer(40);
    CHECK(collider.Layer() == 31);
    collider.SetIsTrigger(true);
    collider.SetEnabled(false);
    CHECK(collider.IsTrigger() && !collider.Enabled());

    CharacterController3D controller{};
    CHECK(controller.BroadPhaseQueryIndex() == -1);
    controller.SetBroadPhaseQueryIndex(5);
    CHECK(controller.BroadPhaseQueryIndex() == 5);
}
