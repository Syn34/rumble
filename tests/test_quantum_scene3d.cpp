// Tests for the 3D physics scene: falling bodies resting on static colliders, line casts and
// trigger callbacks.
#include "test_framework.h"
#include "math_tables.h"

#include "photon_deterministic/system_allocator.h"
#include "quantum_core/component_prototypes.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_checksumer_bit_stream.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/character_controller3d.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/resource_manager.h"
#include "quantum_core/asset_db.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"
#include "quantum_core/transform.h"
#include "photon_deterministic/fp_math.h"

#include <cstdio>
#include <memory>
#include <vector>

using namespace Quantum;
using namespace Quantum::Core;
using namespace Quantum::Physics3D;
using Photon::Deterministic::SystemAllocator;

namespace {

FP F(int v) { return FP::FromInt(v); }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }

struct RegistryScope {
    bool owned = false;
    RegistryScope() {
        if (!ComponentTypeId<>::IsSetUp()) {
            ComponentTypeId<>::Setup({});
            owned = true;
        }
    }
    ~RegistryScope() {
        if (owned) {
            ComponentTypeId<>::ResetForTests();
        }
    }
};

class SceneTestFrame final : public FrameBase {
  public:
    explicit SceneTestFrame(FrameContext* context) : FrameBase(context) {}
    FrameMetaData* get_FrameMetaData() override { return &_meta; }
    CommitCommandsModes get_CommitCommandsMode() const override {
        return CommitCommandsModes::OnFrameSimulationBegin;
    }
    FP get_DeltaTime() const override { return FP::FromRaw(65536 / 60); }
    void set_DeltaTime(FP) override {}
    std::string DumpFrame(int) override { return {}; }
    std::uint64_t CalculateChecksum() override { return 0; }
    std::vector<std::uint8_t>
    Serialize(Photon::Deterministic::DeterministicFrameSerializeMode) override {
        return {};
    }
    void Deserialize(std::span<const std::uint8_t>) override {}
    void Copy(const Photon::Deterministic::DeterministicFrame&) override {}
    Map* get_Map() const override { return map.get(); }
    const PhysicsSceneSettings* get_PhysicsSceneSettings() const override { return &settings; }

    std::shared_ptr<Map> map = std::make_shared<Map>();
    PhysicsSceneSettings settings;

  private:
    FrameMetaData _meta{};
};

struct World {
    std::unique_ptr<ResourceManagerStatic> resources; // outlives the context
    RegistryScope registry;
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    std::unique_ptr<SceneTestFrame> frame;
    std::unique_ptr<PhysicsEngine3D> engine;

    explicit World(std::vector<std::shared_ptr<AssetObject>> assets = {}) {
        rr::test::InstallMathDerivedTables();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        if (!assets.empty()) {
            resources = std::make_unique<ResourceManagerStatic>(assets, nullptr);
            args.AssetDatabase = resources->CreateAssetDatabase();
            args.UsePhysics3D = true; // the controller queries the context's engine
        }
        args.PlatformInfo = &platform;
        args.HeapConfig = Quantum::Allocator::Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
        frame = std::make_unique<SceneTestFrame>(context.get());
        Core::EntitiesConfig config;
        config.DefaultEntityCapacity = 32;
        config.DefaultComponentBlockCapacity = 8;
        FrameBase::Initialize(frame.get(), config);
        PhysicsSceneSettings& s = frame->settings;
        s.SetAllLayersColliding();
        s.SetAllowSleeping(true);
        s.SetUseAngularVelocity(true);
        s.SolverType = ConstraintsSolverType::LegacyGaussSeidel;
        s.SolverIterations = 4;
        s.Gravity = {FP::Zero(), F(-10), FP::Zero()};
        s.PenetrationAllowance = FP::FromRaw(655);   // 0.01
        s.PenetrationCorrection = FP::FromRaw(26214); // 0.4
        s.SleepTimeSec = FP::_1();
        s.LinearSleepTolerance = FP::FromRaw(6553);
        s.AngularSleepToleranceInRad = FP::FromRaw(6553);
        engine = std::make_unique<PhysicsEngine3D>(context.get());
    }

    void AddStaticBox(FPVector3 position, FPVector3 extents, bool trigger = false) {
        MapStaticCollider3D collider;
        collider.Position = position;
        collider.ShapeType = Shape3DType::Box;
        collider.BoxExtents = extents;
        collider.StaticData.IsTrigger = trigger;
        frame->map->StaticColliders3D.push_back(collider);
    }

    EntityRef AddBody(FPVector3 position, Shape3D shape, bool dynamic, bool trigger = false) {
        const EntityRef e = frame->Create();
        frame->Add(e, Quantum::Transform3D{position, FPQuaternion::Identity()});
        PhysicsCollider3D collider{};
        collider.Shape = shape;
        collider.SetIsTrigger(trigger);
        frame->Add(e, collider);
        if (dynamic) {
            PhysicsBody3D body{};
            body.InitBody(F(1), FP::Zero(), FP::Zero(), PhysicsBody3D::ConfigFlags::Default,
                          RotationFreezeFlags::FreezeAll, {}, {});
            frame->Add(e, body);
        }
        return e;
    }

    void Step(ICollisionCallbacks3D* callbacks = nullptr) {
        engine->Update(frame.get(), callbacks, frame->get_DeltaTime());
    }
};

struct RecordingCallbacks final : ICollisionCallbacks3D {
    int triggerEnter = 0, trigger = 0, triggerExit = 0, collisionEnter = 0;
    void OnCollision3D(FrameBase*, const CollisionInfo3D&) override {}
    void OnCollisionEnter3D(FrameBase*, const CollisionInfo3D&) override { ++collisionEnter; }
    void OnCollisionExit3D(FrameBase*, const ExitInfo3D&) override {}
    void OnTrigger3D(FrameBase*, const TriggerInfo3D&) override { ++trigger; }
    void OnTriggerEnter3D(FrameBase*, const TriggerInfo3D&) override { ++triggerEnter; }
    void OnTriggerExit3D(FrameBase*, const ExitInfo3D&) override { ++triggerExit; }
};

} // namespace

TEST_CASE("Scene3D: active entry list and stabbing list bookkeeping") {
    ActiveEntryList list;
    list.ResetAndEnsureCapacity(5);
    CHECK(list.Capacity == 8);
    list.Add({3, 0, 1}, false);
    list.Add({1, 0, 1}, false);
    list.Add({4, 0, 1}, false);
    list.Remove(3); // last item moves into the hole
    CHECK(list.Count == 2 && list.Items[0].Index == 4 && list.IndexLookup[4] == 0);

    StabbingList stab;
    stab.ResetAndEnsureCapacity(4, 4);
    stab.GetNextEntryList(0);
    stab.GetNextEntryList(10);
    stab.GetNextEntryList(20);
    CHECK(stab.Stab(15) == 1 && stab.Stab(20) == 2 && stab.Stab(5) == 0);
}

TEST_CASE("Scene3D: a falling body comes to rest on a static box") {
    World w;
    w.AddStaticBox(V3(0, -1, 0), V3(10, 1, 10));
    const EntityRef box = w.AddBody(V3(0, 3, 0), Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()}), true);
    w.engine->Init(w.frame.get(), AssetGuid{});
    CHECK(w.engine->Scene()->StaticEntriesCount() == 1);
    for (int i = 0; i < 240; ++i) {
        w.Step();
    }
    const Quantum::Transform3D* t = w.frame->GetPointer<Quantum::Transform3D>(box);
    const PhysicsBody3D* body = w.frame->GetPointer<PhysicsBody3D>(box);
    // Resting on the surface (y = 0) with its half-height 0.5, within the allowance.
    CHECK(t->Position.Y > FP::FromRaw(26214) && t->Position.Y < FP::FromRaw(39322));
    CHECK(Photon::Deterministic::FPMath::Abs(body->Velocity.Y) < FP::FromRaw(6554));
    CHECK(body->IsSleeping());
}

TEST_CASE("Scene3D: a line cast hits the expected entity") {
    World w;
    const EntityRef near = w.AddBody(V3(0, 0, 5), Shape3D::CreateSphere(F(1)), false);
    const EntityRef far = w.AddBody(V3(0, 0, 10), Shape3D::CreateBox(V3(1, 1, 1)), false);
    w.AddBody(V3(5, 0, 5), Shape3D::CreateSphere(F(1)), false); // off the line
    w.engine->Init(w.frame.get(), AssetGuid{});
    w.Step();
    HitCollection3D hits(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &hits, V3(0, 0, 0), V3(0, 0, 20), false, -1, QueryOptions::HitAll);
    CHECK(hits.Count() == 2);
    hits.SortCastDistance();
    CHECK(hits[0].Entity().Index == near.Index && hits[1].Entity().Index == far.Index);
    CHECK(Photon::Deterministic::FPMath::Abs(hits[0].Point.Z - F(4)) < FP::FromRaw(64));

    // First hit only: the walk stops after the first bucket holding a hit, keeping every
    // hit found in it (both here: one bucket along X covers the whole segment).
    HitCollection3D first(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &first, V3(0, 0, 0), V3(0, 0, 20), true, -1, QueryOptions::HitAll);
    CHECK(first.Count() == 2);
}

TEST_CASE("Scene3D: a first-hit line cast stops at the first bucket with a hit") {
    World w;
    w.frame->map->BucketingAxis = Map::BucketAxis::Y; // buckets along Z, 16 units each
    const EntityRef near = w.AddBody(V3(0, 0, 5), Shape3D::CreateSphere(F(1)), false);
    w.AddBody(V3(0, 0, 40), Shape3D::CreateBox(V3(1, 1, 1)), false);
    w.engine->Init(w.frame.get(), AssetGuid{});
    w.Step();
    HitCollection3D all(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &all, V3(0, 0, 0), V3(0, 0, 60), false, -1, QueryOptions::HitAll);
    CHECK(all.Count() == 2);
    HitCollection3D first(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &first, V3(0, 0, 0), V3(0, 0, 60), true, -1, QueryOptions::HitAll);
    CHECK(first.Count() == 1 && first[0].Entity().Index == near.Index);
    // Walking backwards finds the far box first.
    HitCollection3D back(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &back, V3(0, 0, 60), V3(0, 0, 0), true, -1, QueryOptions::HitAll);
    CHECK(back.Count() == 1 && back[0].Entity().Index != near.Index);
    // Overlap through the buckets: the sphere only.
    HitCollection3D overlap(8, HitCollection3D::Temp, w.context.get());
    w.engine->OverlapShape(w.frame.get(), &overlap, V3(0, 0, 6), FPQuaternion::Identity(),
                           Shape3D::CreateSphere(F(1)), -1, QueryOptions::HitAll | QueryOptions::ComputeDetailedInfo);
    CHECK(overlap.Count() == 1 && overlap[0].Entity().Index == near.Index);
    CHECK(overlap[0].OverlapPenetration > FP::Zero());
    CHECK(overlap[0].Normal.Z > FP::Zero()); // from the collider towards the query shape
}

TEST_CASE("Scene3D: a trigger callback fires once on enter") {
    World w;
    w.frame->settings.Gravity = FPVector3::Zero();
    const EntityRef trigger = w.AddBody(V3(0, 0, 0), Shape3D::CreateBox(V3(2, 2, 2)), false, true);
    const EntityRef mover = w.AddBody(V3(-6, 0, 0), Shape3D::CreateSphere(FP::_0_50()), true);
    w.frame->Add(trigger, PhysicsCallbacks3D{}); // OnAdded clears the component
    w.frame->GetPointer<PhysicsCallbacks3D>(trigger)->Flags =
        CallbackFlags::OnDynamicTriggerEnter | CallbackFlags::OnDynamicTriggerExit |
        CallbackFlags::OnDynamicTrigger;
    w.frame->GetPointer<PhysicsBody3D>(mover)->Velocity = V3(6, 0, 0);
    w.engine->Init(w.frame.get(), AssetGuid{});
    RecordingCallbacks recorder;
    for (int i = 0; i < 60; ++i) { // one second: the sphere enters at x = -2.5
        w.Step(&recorder);
    }
    CHECK(recorder.triggerEnter == 1);
    CHECK(recorder.trigger > 1);
    CHECK(recorder.triggerExit == 0);
    for (int i = 0; i < 60; ++i) { // and leaves past x = 2.5
        w.Step(&recorder);
    }
    CHECK(recorder.triggerEnter == 1);
    CHECK(recorder.triggerExit == 1);
}

TEST_CASE("Scene3D: inertia from shape factors and the context's engine") {
    World w;
    const EntityRef e = w.AddBody(V3(0, 0, 0), Shape3D::CreateBox(V3(1, 2, 3)), true);
    PhysicsBody3D* body = w.frame->GetPointer<PhysicsBody3D>(e);
    body->ResetInertia(w.frame.get(), e);
    // Box with half extents (1, 2, 3), mass 1: I = ((4 + 9), (1 + 9), (1 + 4)) / 3.
    const FPVector3 inv = body->InertiaTensorInverse();
    CHECK(Photon::Deterministic::FPMath::Abs(inv.X - FP::FromRaw(65536 * 3 / 13)) < FP::FromRaw(8));
    CHECK(Photon::Deterministic::FPMath::Abs(inv.Z - FP::FromRaw(65536 * 3 / 5)) < FP::FromRaw(8));
    const FPVector3 sphere = Shape3D::CreateSphere(F(2)).CalculateInertiaTensorFactor(FPVector3::Zero());
    CHECK(sphere.X == FP::FromRaw(2 * 4 * 65536 / 5));

    FrameContext::Args args;
    args.PlatformInfo = &w.platform;
    args.HeapConfig = Quantum::Allocator::Heap::Config(12, 256, 4);
    args.UsePhysics3D = true;
    FrameContext withPhysics(args);
    CHECK(withPhysics.get_Physics3D() != nullptr && w.context->get_Physics3D() == nullptr);
}

TEST_CASE("Scene3D: physics body and callbacks prototypes materialise") {
    World w;
    const EntityRef e = w.frame->Create();
    PhysicsCollider3D collider{};
    collider.Shape = Shape3D::CreateSphere(F(1));
    w.frame->Add(e, collider); // the body's OnAdded resets inertia from the collider
    Prototypes::PhysicsBody3D_Prototype bodyPrototype;
    bodyPrototype.Mass = F(4);
    PrototypeMaterializationContext materialization{};
    CHECK(bodyPrototype.AddToEntity(w.frame.get(), e, materialization));
    const PhysicsBody3D* body = w.frame->GetPointer<PhysicsBody3D>(e);
    CHECK(body->MassInverse() == FP::_0_25() && body->GravityScale() == F(1));
    CHECK(body->Drag() == FP::_0_50());
    CHECK(body->InertiaTensorInverse().X > FP::Zero());

    PhysicsEngine3D::SetCallbacks(w.frame.get(), e, CallbackFlags::OnDynamicCollision);
    CHECK(w.frame->Get<PhysicsCallbacks3D>(e).Flags == CallbackFlags::OnDynamicCollision);
    PhysicsEngine3D::SetCallbacks(w.frame.get(), e, CallbackFlags::None);
    CHECK(!w.frame->Has<PhysicsCallbacks3D>(e));
}

namespace {
struct ContactRecorder final : ICollisionCallbacks3D {
    int enter = 0, stay = 0, exit = 0;
    bool lastStatic = false;
    EntityRef lastEntity, lastOther;
    std::int32_t lastStaticIndex = -2;
    void OnCollision3D(FrameBase*, const CollisionInfo3D&) override { ++stay; }
    void OnCollisionEnter3D(FrameBase*, const CollisionInfo3D& info) override {
        ++enter;
        lastStatic = info.IsStatic();
        lastEntity = info.Entity();
        lastOther = info.Other();
    }
    void OnCollisionExit3D(FrameBase*, const ExitInfo3D& info) override {
        ++exit;
        lastStatic = info.IsStatic();
        lastStaticIndex = info._staticIndex;
        lastEntity = info.Entity;
    }
    void OnTrigger3D(FrameBase*, const TriggerInfo3D&) override {}
    void OnTriggerEnter3D(FrameBase*, const TriggerInfo3D&) override {}
    void OnTriggerExit3D(FrameBase*, const ExitInfo3D&) override {}
};
} // namespace

TEST_CASE("Scene3D: accumulated impulses add or extend") {
    AccumulatedImpulse sum{};
    AccumulatedImpulse unit{};
    unit.Velocity = V3(1, 0, 0);
    sum.Accumulate(unit, ImpulseLinear | ImpulseAdd);
    sum.Accumulate(unit, ImpulseLinear | ImpulseAccumulate); // already covered: no change
    CHECK(sum.Velocity.X == F(1));
    AccumulatedImpulse twice{};
    twice.Velocity = V3(2, 0, 0);
    sum.Accumulate(twice, ImpulseLinear | ImpulseAccumulate); // only the share beyond 1
    CHECK(sum.Velocity.X == F(2));
    AccumulatedImpulse back{};
    back.Velocity = V3(-1, 0, 0);
    back.AngularVelocity = V3(0, 3, 0);
    sum.Accumulate(back, ImpulseLinear | ImpulseAccumulate); // opposing: added in full
    CHECK(sum.Velocity.X == F(1) && sum.AngularVelocity.Y == FP::Zero());
    sum.Accumulate(back, ImpulseAngular | ImpulseAdd);
    CHECK(sum.Velocity.X == F(1) && sum.AngularVelocity.Y == F(3));
}

TEST_CASE("Scene3D: static collision enter and exit callbacks") {
    World w;
    w.AddStaticBox(V3(0, -1, 0), V3(10, 1, 10));
    const EntityRef ball = w.AddBody(V3(0, 1, 0), Shape3D::CreateSphere(FP::_0_50()), true);
    PhysicsEngine3D::SetCallbacks(w.frame.get(), ball,
                                  CallbackFlags::OnStaticCollisionEnter | CallbackFlags::OnStaticCollisionExit |
                                      CallbackFlags::OnStaticCollision);
    w.engine->Init(w.frame.get(), AssetGuid{});
    ContactRecorder recorder;
    for (int i = 0; i < 60; ++i) {
        w.Step(&recorder);
    }
    CHECK(recorder.enter == 1);
    CHECK(recorder.stay > 1);
    CHECK(recorder.lastStatic && recorder.lastEntity == ball && !recorder.lastOther.IsValid());
    CHECK(recorder.exit == 0);
    // Lifted away: one exit from static collider 0.
    w.frame->GetPointer<Quantum::Transform3D>(ball)->Position = V3(0, 5, 0);
    w.frame->GetPointer<PhysicsBody3D>(ball)->Velocity = FPVector3::Zero();
    w.Step(&recorder);
    w.Step(&recorder);
    CHECK(recorder.exit == 1);
    CHECK(recorder.lastStatic && recorder.lastStaticIndex == 0 && recorder.lastEntity == ball);
}

TEST_CASE("Scene3D: stacked bodies share one island") {
    World w;
    w.frame->settings.SetAllowSleeping(false);
    w.AddStaticBox(V3(0, -1, 0), V3(10, 1, 10));
    const Shape3D cube = Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()});
    w.AddBody(V3(0, 0, 0) + FPVector3{FP::Zero(), FP::_0_50(), FP::Zero()}, cube, true);
    w.AddBody(V3(0, 1, 0) + FPVector3{FP::Zero(), FP::_0_50(), FP::Zero()}, cube, true);
    w.AddBody(V3(5, 0, 0) + FPVector3{FP::Zero(), FP::_0_50(), FP::Zero()}, cube, true); // apart
    w.engine->Init(w.frame.get(), AssetGuid{});
    for (int i = 0; i < 3; ++i) {
        w.Step();
    }
    const Scene3D* scene = w.engine->Scene();
    int live = 0;
    std::size_t largest = 0;
    for (const Island3D& island : scene->_islands) {
        if (!island.Ignore) {
            ++live;
            largest = std::max(largest, island.Entries.size());
        }
    }
    CHECK(live == 2);
    CHECK(largest == 2);
    CHECK(scene->_solverEntries.size() == 3);
}

TEST_CASE("Scene3D: a resting body without contacts falls asleep on its own") {
    World w;
    w.frame->settings.Gravity = FPVector3::Zero();
    const EntityRef e = w.AddBody(V3(0, 0, 0), Shape3D::CreateSphere(F(1)), true);
    w.engine->Init(w.frame.get(), AssetGuid{});
    PhysicsBody3D* body = w.frame->GetPointer<PhysicsBody3D>(e);
    for (int i = 0; i < 30 && !body->IsSleeping(); ++i) {
        w.Step();
    }
    CHECK(!body->IsSleeping()); // half a second: not yet
    for (int i = 0; i < 60 && !body->IsSleeping(); ++i) {
        w.Step();
    }
    CHECK(body->IsSleeping());
    CHECK(body->SleepTime() == FP::Zero());
}

TEST_CASE("Scene3D: the hybrid solver rests a box and a stack on the floor") {
    World w;
    w.frame->settings.SolverType = ConstraintsSolverType::HybridJacobiGaussSeidel;
    w.frame->settings.SetAllowSleeping(false);
    w.AddStaticBox(V3(0, -1, 0), V3(10, 1, 10));
    const Shape3D cube = Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()});
    const EntityRef low = w.AddBody(V3(0, 1, 0), cube, true);
    const EntityRef high = w.AddBody(V3(0, 2, 0) + FPVector3{FP::Zero(), FP::_0_50(), FP::Zero()}, cube, true);
    w.engine->Init(w.frame.get(), AssetGuid{});
    for (int i = 0; i < 240; ++i) {
        w.Step();
    }
    const FP lowY = w.frame->GetPointer<Quantum::Transform3D>(low)->Position.Y;
    const FP highY = w.frame->GetPointer<Quantum::Transform3D>(high)->Position.Y;
    // Within a few centimetres of y = 0.5 and 1.5.
    CHECK(Photon::Deterministic::FPMath::Abs(lowY - FP::_0_50()) < FP::FromRaw(6554));
    CHECK(Photon::Deterministic::FPMath::Abs(highY - FP::FromRaw(98304)) < FP::FromRaw(6554));
    // Rotation is frozen but the perceived masses still include the inertia, so each pass
    // removes only part of the approach speed: the boxes settle with a small steady
    // downward velocity that penetration correction balances (as in the original).
    const FP vHigh = w.frame->GetPointer<PhysicsBody3D>(high)->Velocity.Y;
    CHECK(vHigh < FP::Zero() && vHigh > -FP::_1());
    const FP before = w.frame->GetPointer<Quantum::Transform3D>(high)->Position.Y;
    w.Step();
    CHECK(w.frame->GetPointer<Quantum::Transform3D>(high)->Position.Y == before);
}

TEST_CASE("Scene3D: shape casts against spheres and boxes") {
    World w;
    PhysicsQueryConfig config;
    config.ShapeCastMaxIterations = 16;
    config.ShapeCastHardTolerance = FP::FromRaw(65);  // ~0.001
    config.ShapeCastSoftTolerance = FP::FromRaw(655); // ~0.01
    w.frame->settings.GJKSettingsValue = GJKSettings(config);
    const EntityRef ball = w.AddBody(V3(0, 0, 10), Shape3D::CreateSphere(F(1)), false);
    const EntityRef crate = w.AddBody(V3(5, 0, 10), Shape3D::CreateBox(V3(1, 1, 1)), false);
    w.engine->Init(w.frame.get(), AssetGuid{});
    w.Step();
    const int options = QueryOptions::HitAll | QueryOptions::ComputeDetailedInfo;
    // Sphere against sphere: touches when the centres are 2 apart, at z = 8 (half of 20).
    HitCollection3D hits(8, HitCollection3D::Temp, w.context.get());
    w.engine->ShapeCast(w.frame.get(), &hits, V3(0, 0, -2), FPQuaternion::Identity(), Shape3D::CreateSphere(F(1)),
                        V3(0, 0, 20), false, -1, options);
    CHECK(hits.Count() == 1);
    if (hits.Count() != 1) return;
    CHECK(hits[0].Entity().Index == ball.Index);
    CHECK(Photon::Deterministic::FPMath::Abs(hits[0].CastDistanceNormalized - FP::_0_50()) < FP::FromRaw(64));
    CHECK(Photon::Deterministic::FPMath::Abs(hits[0].Point.Z - F(9)) < FP::FromRaw(64));
    // Box against box through GJK.
    HitCollection3D boxHits(8, HitCollection3D::Temp, w.context.get());
    w.engine->ShapeCast(w.frame.get(), &boxHits, V3(5, 0, 0), FPQuaternion::Identity(),
                        Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()}), V3(0, 0, 20), false, -1,
                        options);
    CHECK(boxHits.Count() == 1);
    if (boxHits.Count() != 1) return;
    CHECK(boxHits[0].Entity().Index == crate.Index);
    // Front face at z = 9, the cast box's half size 0.5: 8.5 / 20.
    CHECK(Photon::Deterministic::FPMath::Abs(boxHits[0].CastDistanceNormalized - FP::FromRaw(27853)) <
          FP::FromRaw(400));
}

TEST_CASE("Scene3D: a CCD body stops at a thin wall instead of tunnelling") {
    World w;
    w.frame->settings.Gravity = FPVector3::Zero();
    PhysicsQueryConfig config;
    config.AllowCCD = true;
    config.CCDLinearVelocityThreshold = F(1);
    config.CCDAngularVelocityThreshold = F(1);
    config.CCDDistanceTolerance = FP::FromRaw(655);
    config.MaxTimeOfImpactIterations = 16;
    config.MaxRootFindingIterations = 16;
    w.frame->settings.CCDSettingsValue = CCDSettings(config);
    w.AddStaticBox(V3(0, 0, 10), FPVector3{F(5), F(5), FP::FromRaw(6554)}); // 0.2 thick
    const EntityRef bullet = w.AddBody(V3(0, 0, 0), Shape3D::CreateSphere(FP::_0_50()), true);
    PhysicsBody3D* body = w.frame->GetPointer<PhysicsBody3D>(bullet);
    body->Config |= PhysicsBody3D::ConfigFlags::UseContinuousCollisionDetection;
    body->Velocity = V3(0, 0, 600); // 10 units per step
    w.engine->Init(w.frame.get(), AssetGuid{});
    for (int i = 0; i < 3; ++i) {
        w.Step();
    }
    const FP z = w.frame->GetPointer<Quantum::Transform3D>(bullet)->Position.Z;
    CHECK(z < F(10)); // never past the wall
    CHECK(z > F(5));  // but it did move up to it
}

TEST_CASE("Scene3D: the character controller lands, stands and walks") {
    auto config = std::make_shared<CharacterController3DConfig>();
    config->Guid = AssetGuid(500);
    config->Loaded(nullptr, nullptr);
    World w({config});
    w.AddStaticBox(V3(0, -1, 0), V3(20, 1, 20));
    const EntityRef hero = w.frame->Create();
    w.frame->Add(hero, Quantum::Transform3D{V3(0, 1, 0), FPQuaternion::Identity()});
    Prototypes::CharacterController3D_Prototype prototype;
    prototype.Config.Id = AssetGuid(500);
    PrototypeMaterializationContext materialization{};
    CHECK(prototype.AddToEntity(w.frame.get(), hero, materialization));
    CharacterController3D* kcc = w.frame->GetPointer<CharacterController3D>(hero);
    CHECK(kcc->MaxSpeed == F(3) && kcc->Config().Id == AssetGuid(500));
    PhysicsEngine3D* engine = w.context->get_Physics3D();
    engine->Init(w.frame.get(), AssetGuid{});
    engine->Update(w.frame.get(), nullptr, w.frame->get_DeltaTime());
    for (int i = 0; i < 90; ++i) {
        w.frame->GetPointer<CharacterController3D>(hero)->Move(w.frame.get(), hero, FPVector3::Zero());
    }
    const Quantum::Transform3D* t = w.frame->GetPointer<Quantum::Transform3D>(hero);
    kcc = w.frame->GetPointer<CharacterController3D>(hero);
    // Standing on the floor: the sphere (radius 0.5, centre offset 0.5) rests near y = 0.
    CHECK(kcc->Grounded());
    CHECK(Photon::Deterministic::FPMath::Abs(t->Position.Y) < FP::FromRaw(6554));
    // Walking along +X reaches the controller's speed.
    const FP startX = t->Position.X;
    for (int i = 0; i < 60; ++i) {
        w.frame->GetPointer<CharacterController3D>(hero)->Move(w.frame.get(), hero, V3(1, 0, 0));
    }
    kcc = w.frame->GetPointer<CharacterController3D>(hero);
    CHECK(kcc->Grounded());
    CHECK(t->Position.X - startX > F(2));
    CHECK(kcc->Velocity.X <= F(3) && kcc->Velocity.X > F(2));
    CHECK(Photon::Deterministic::FPMath::Abs(t->Position.Y) < FP::FromRaw(6554));
}
