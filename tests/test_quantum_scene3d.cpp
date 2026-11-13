// Tests for the 3D physics scene: falling bodies resting on static colliders, line casts and
// trigger callbacks.
#include "test_framework.h"
#include "math_tables.h"

#include "photon_deterministic/system_allocator.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_checksumer_bit_stream.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"
#include "quantum_core/transform.h"
#include "photon_deterministic/fp_math.h"

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
    RegistryScope registry;
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    std::unique_ptr<SceneTestFrame> frame;
    std::unique_ptr<PhysicsEngine3D> engine;

    World() {
        rr::test::InstallMathDerivedTables();
        platform.Allocator = &allocator;
        FrameContext::Args args;
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

    HitCollection3D first(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &first, V3(0, 0, 0), V3(0, 0, 20), true, -1, QueryOptions::HitAll);
    CHECK(first.Count() == 1 && first[0].Entity().Index == near.Index);
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
