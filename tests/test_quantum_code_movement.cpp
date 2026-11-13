// Character movement: MovementSystem driving KccExtension.Move over a static floor.
#include "test_framework.h"

#include "math_tables.h"
#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/assets/actor_specs.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/frame.h"
#include "quantum_code/systems/libs.h"
#include "quantum_code/systems/movement_systems.h"
#include "quantum_code/systems/physics_utility.h"
#include "quantum_code/systems/system_setup.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/map.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/resource_manager.h"
#include "quantum_core/transform.h"

#include <memory>
#include <vector>

using namespace Quantum;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPMath::Abs;
using Photon::Deterministic::SystemAllocator;
using Quantum::Core::FrameContext;

namespace {

FP F(int v) { return FP::FromInt(v); }
FPVector3 V3(int x, int y, int z) { return FPVector3(F(x), F(y), F(z)); }

constexpr std::uint64_t kMapGuid = 700;
constexpr std::uint64_t kKccConfigGuid = 701;
constexpr std::uint64_t kExtConfigGuid = 702;

// A frame with MovementSystem, 3D physics and a 40 x 40 floor whose top is at y = 0.
struct MovementWorld {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<ResourceManagerStatic> resources;
    std::unique_ptr<FrameContext> context;
    Photon::Deterministic::DeterministicSessionConfig session;
    RuntimeConfig runtime;
    SimulationConfig simulation;
    SystemList roots;
    std::vector<SystemBase*> all;
    std::unique_ptr<Frame> frame;
    std::shared_ptr<KccExtensionConfig> extConfig = std::make_shared<KccExtensionConfig>();
    std::shared_ptr<CharacterController3DConfig> kccConfig = std::make_shared<CharacterController3DConfig>();

    explicit MovementWorld(SystemList extraSystems = {}, std::vector<std::shared_ptr<AssetObject>> extraAssets = {}) {
        rr::test::InstallMathDerivedTables();
        ComponentTypeId<>::ResetForTests();
        auto map = std::make_shared<Map>();
        map->Guid = AssetGuid(kMapGuid);
        MapStaticCollider3D floor;
        floor.Position = V3(0, -1, 0);
        floor.ShapeType = Shape3DType::Box;
        floor.BoxExtents = V3(20, 1, 20);
        map->StaticColliders3D.push_back(floor);
        kccConfig->Guid = AssetGuid(kKccConfigGuid);
        kccConfig->Loaded(nullptr, nullptr);
        extConfig->Guid = AssetGuid(kExtConfigGuid);
        extConfig->OrientationSharpness = F(10);
        extConfig->PushReductionSpeed = F(20);
        extConfig->Airborne_Acceleration = F(10);
        extConfig->Airborne_Braking = F(1);
        extraAssets.insert(extraAssets.end(), {map, kccConfig, extConfig});
        resources = std::make_unique<ResourceManagerStatic>(extraAssets, nullptr);

        platform.Allocator = &allocator;
        FrameContext::Args args;
        args.AssetDatabase = resources->CreateAssetDatabase();
        args.PlatformInfo = &platform;
        args.HeapConfig = Quantum::Allocator::Heap::Config(12, 256, 4);
        args.UsePhysics3D = true;
        context = std::make_unique<FrameContext>(args);
        session.PlayerCount = 4;
        simulation.Entities.DefaultEntityCapacity = 64;
        simulation.Entities.DefaultComponentBlockCapacity = 8;
        roots.push_back(std::make_unique<ActorInputUpdateSystem>());
        roots.push_back(std::make_unique<MovementSystem>());
        for (auto& system : extraSystems) {
            roots.push_back(std::move(system));
        }
        all = FlattenSystems(roots);
        std::vector<SystemBase*> rootPointers;
        for (const auto& root : roots) {
            rootPointers.push_back(root.get());
        }
        frame = std::make_unique<Frame>(context.get(), all, rootPointers, &session, &runtime, &simulation,
                                        FP::FromRaw(1092)); // 1/60 s
        frame->get_Global()->Map.Id = AssetGuid(kMapGuid);
        PhysicsSceneSettings* settings = frame->MutablePhysicsSceneSettings();
        settings->SetAllLayersColliding();
        context->get_Physics3D()->Init(frame.get(), AssetGuid(kMapGuid));
        StartSystems(frame.get());
    }

    ~MovementWorld() {
        frame->Free();
        frame.reset();
        context->Dispose();
        ComponentTypeId<>::ResetForTests();
    }

    EntityRef AddCharacter(FPVector3 position) {
        Frame& f = *frame;
        const EntityRef e = f.Create();
        f.Add(e, Quantum::Transform3D{position, FPQuaternion::Identity()});
        CharacterController3D kcc{};
        kcc.Init(&f, kccConfig.get());
        f.Add(e, kcc);
        KccExtension ext{};
        ext.Config.Id = AssetGuid(kExtConfigGuid);
        f.Add(e, ext);
        f.Add<ActorInput>(e);
        return e;
    }

    void Step(int frames = 1) {
        for (int i = 0; i < frames; ++i) {
            ++frame->Number;
            UpdateSystems(frame.get());
        }
    }
};

} // namespace

TEST_CASE("quantum.code movement: a character lands, walks, turns and jumps") {
    MovementWorld w;
    Frame& f = *w.frame;
    const EntityRef hero = w.AddCharacter(V3(0, 2, 0));
    KccExtension* ext = f.GetPointer<KccExtension>(hero);
    CHECK(ext->enabled);
    CHECK(ext->orientationSharpness == F(10));

    w.Step(120);
    const Quantum::Transform3D* t = f.GetPointer<Quantum::Transform3D>(hero);
    CHECK(f.GetPointer<CharacterController3D>(hero)->Grounded());
    CHECK(f.GetPointer<KccExtension>(hero)->grounded);
    CHECK(Abs(t->Position.Y) < FP::FromRaw(6554));
    CHECK(f.GetPointer<KccExtension>(hero)->groundedTime > F(1));

    // Walking along +X reaches the controller's speed and turns the character that way.
    f.GetPointer<ActorInput>(hero)->Direction = FPVector2(F(1), FP{});
    const FP startX = t->Position.X;
    w.Step(60);
    t = f.GetPointer<Quantum::Transform3D>(hero);
    const CharacterController3D* kcc = f.GetPointer<CharacterController3D>(hero);
    CHECK(t->Position.X - startX > F(2));
    CHECK(kcc->Velocity.X <= w.kccConfig->MaxSpeed && kcc->Velocity.X > F(2));
    CHECK(t->Forward().X > FP::FromRaw(64880)); // about 0.99
    CHECK(Abs(t->Position.Y) < FP::FromRaw(6554));

    // Jumping: the button press leaves the ground with the jump impulse.
    f.GetPointer<ActorInput>(hero)->Direction = FPVector2{};
    w.Step(30);
    f.GetPointer<ActorInput>(hero)->JumpStateWanted = true;
    w.Step();
    f.GetPointer<ActorInput>(hero)->JumpStateWanted = false;
    CHECK(f.GetPointer<ActorInput>(hero)->JumpButton.WasPressed());
    ext = f.GetPointer<KccExtension>(hero);
    CHECK(ext->jumpState == JumpState::Jumping);
    CHECK(!ext->grounded);
    w.Step(5);
    t = f.GetPointer<Quantum::Transform3D>(hero);
    CHECK(t->Position.Y > FP::FromRaw(13107)); // above 0.2
    w.Step(120);
    t = f.GetPointer<Quantum::Transform3D>(hero);
    ext = f.GetPointer<KccExtension>(hero);
    CHECK(ext->grounded);
    CHECK(ext->jumpState == JumpState::None);
    CHECK(Abs(t->Position.Y) < FP::FromRaw(6554));
}

TEST_CASE("quantum.code movement: pushes move the character and decay") {
    MovementWorld w;
    Frame& f = *w.frame;
    const EntityRef hero = w.AddCharacter(V3(0, 0, 0));
    w.Step(30);
    const FP startZ = f.GetPointer<Quantum::Transform3D>(hero)->Position.Z;
    f.GetPointer<KccExtension>(hero)->AddPush(&f, V3(0, 0, 10), false);
    w.Step();
    KccExtension* ext = f.GetPointer<KccExtension>(hero);
    CHECK(ext->push == FPVector3::Zero());
    // Bent upwards by the push lerp (0.1 x 90/100), then one frame of decay.
    CHECK(ext->pushVelocity.Z > F(8) && ext->pushVelocity.Z < F(9));
    // The upward lerp bends the horizontal push slightly upwards.
    CHECK(ext->pushVelocity.Y > FP{});
    w.Step(60);
    ext = f.GetPointer<KccExtension>(hero);
    CHECK(ext->pushVelocity == FPVector3::Zero());
    const FP travelled = f.GetPointer<Quantum::Transform3D>(hero)->Position.Z - startZ;
    CHECK(travelled > F(1) && travelled < F(5));

    // Reviving clears the pending motion.
    ext->AddPush(&f, V3(5, 0, 0), true);
    ext->AddImpulse(V3(0, 5, 0));
    MovementSystem::ResetKccMotion(&f, hero);
    ext = f.GetPointer<KccExtension>(hero);
    CHECK(!ext->hasPushToConsume && !ext->hasImpulseToConsume && !ext->resetPush);
    CHECK(ext->impulse == FPVector3::Zero() && ext->velocity == FPVector3::Zero());
}

TEST_CASE("quantum.code movement: impulses with forced input and modifiers") {
    MovementWorld w;
    Frame& f = *w.frame;
    const EntityRef hero = w.AddCharacter(V3(0, 0, 0));
    w.Step(30);
    KccExtension* ext = f.GetPointer<KccExtension>(hero);
    CharacterController3D* kcc = f.GetPointer<CharacterController3D>(hero);
    ext->AddImpulsionWithInputsAuto(&f, kcc, V3(10, 10, 0));
    CHECK(ext->hasImpulseToConsume);
    CHECK(kcc->Jumped());
    // |impulse| / 25 of a second (about 0.566).
    CHECK(ext->impulseTimer > FP::FromRaw(36700) && ext->impulseTimer < FP::FromRaw(37500));
    CHECK(ext->forceInputMaxBlend == F(1));
    CHECK(ext->forceInput.X > FP::FromRaw(46000) && ext->forceInput.X <= F(1));
    w.Step();
    ext = f.GetPointer<KccExtension>(hero);
    CHECK(!ext->hasImpulseToConsume);
    CHECK(f.GetPointer<CharacterController3D>(hero)->Velocity.Y > F(5));
    CHECK(!ext->grounded);

    // A max speed modifier scales the controller's max speed while it lasts.
    w.Step(120);
    ext = f.GetPointer<KccExtension>(hero);
    ext->AddModifier_MaxSpeed(&f, MOD_ORIGIN::SPEEDER, FP::FromRaw(-32768), F(1), false);
    w.Step();
    CHECK(f.GetPointer<CharacterController3D>(hero)->MaxSpeed == w.kccConfig->MaxSpeed * FP::FromRaw(32768));
    w.Step(70);
    CHECK(f.GetPointer<CharacterController3D>(hero)->MaxSpeed == w.kccConfig->MaxSpeed);
}

TEST_CASE("quantum.code movement: helpers") {
    CHECK(PhysicsUtility::IsGround(FPVector3::Up()));
    CHECK(PhysicsUtility::IsGround(FPVector3(FP{}, F(1), FP::FromRaw(52429)).Normalized())); // ~38.7 deg
    CHECK(!PhysicsUtility::IsGround(FPVector3(FP{}, F(1), F(1)).Normalized()));               // 45 deg
    CHECK(PlayerActorInputActionLib::FilterContains(PlayerActorInputActionFilter::Jump, PlayerActorInputAction::Jump));
    CHECK(!PlayerActorInputActionLib::FilterContains(PlayerActorInputActionFilter::Fire, PlayerActorInputAction::Jump));

    MovementWorld w;
    Frame& f = *w.frame;
    f.Number = 10;
    CHECK(KccExtension::CheckFrameSlicing(&f, EntityRef{2, 1}, 1));
    CHECK(KccExtension::CheckFrameSlicing(&f, EntityRef{2, 1}, 6));
    CHECK(!KccExtension::CheckFrameSlicing(&f, EntityRef{3, 1}, 6));

    // Ray casts keep only the nearest hit.
    Physics3D::HitCollection3D* hits = f.GetPersistentHitCollection(0);
    RaycastNearest(&f, hits, V3(0, 5, 0), FPVector3::Down(), F(10), -1, static_cast<int>(QueryOptions::HitAll));
    CHECK(hits->Count() == 1);
    CHECK(Abs((*hits)[0].Point.Y) < FP::FromRaw(66));
    CHECK(PhysicsUtility::IsStatic((*hits)[0]));
    CHECK(GetHitStaticCollider(&f, (*hits)[0]) != nullptr);
    CHECK(f.GetPersistentHitCollection(0)->Count() == 0);

    EntityMultiplierPair pair;
    pair.EntityInfoType = EntityInfoType::Crate;
    pair.Multiplier = F(3);
    w.extConfig->WalkingPushMultiplierByEntity.push_back(pair);
    FP multiplier;
    CHECK(w.extConfig->TryGetWalkingPushMultiplier(EntityInfoType::Crate, multiplier) && multiplier == F(3));
    CHECK(!w.extConfig->TryGetWalkingPushMultiplier(EntityInfoType::None, multiplier) && multiplier == FP{});
}

TEST_CASE("quantum.code movement: turning, ledge grabbing and wall running") {
    auto ledgeSpec = std::make_shared<LedgeGrabbingSpec>();
    ledgeSpec->Guid = AssetGuid(710);
    ledgeSpec->snapDuration = FP::One();
    auto wallSpec = std::make_shared<WallRunningSpec>();
    wallSpec->Guid = AssetGuid(711);
    wallSpec->Speed = F(8);
    wallSpec->Acceleration = F(4);
    wallSpec->JumpSpeed = F(10);
    wallSpec->PerpendicularThreshold = FP::FromRaw(32768);
    wallSpec->HeightCorrectionInterpolationSpeed = FP{};
    SystemList extra;
    extra.push_back(std::make_unique<TurnToSystem>());
    extra.push_back(std::make_unique<LedgeGrabbingActorSystem>());
    extra.push_back(std::make_unique<WallRunningSystem>());
    MovementWorld w(std::move(extra), {ledgeSpec, wallSpec});
    Frame& f = *w.frame;

    // TurnTo: turning towards +X; a turn speed of one full lerp per frame gets there at once
    // (slower turns stop once a step is within the epsilon of FPMathUtility).
    const EntityRef turner = f.Create();
    f.Add(turner, Quantum::Transform3D{V3(0, 0, 0), FPQuaternion::Identity()});
    TurnTo turn{};
    turn.lookAtDirection = V3(1, 0, 0);
    turn.turnSpeed = F(60);
    f.Add(turner, turn);
    f.SystemEnable<TurnToSystem>();
    w.Step(2);
    CHECK(f.GetPointer<Quantum::Transform3D>(turner)->Forward().X > FP::FromRaw(65000));

    // Ledge: the signal starts the grab, the update brings the actor to the top.
    const EntityRef climber = f.Create();
    f.Add(climber, Quantum::Transform3D{V3(0, 1, 0), FPQuaternion::Identity()});
    LedgeGrabbingActor grab{};
    grab.Spec.Id = AssetGuid(710);
    f.Add(climber, grab);
    Physics3D::Hit3D wallHit{};
    wallHit.Point = V3(0, 1, 2);
    auto* ledgeSystem = dynamic_cast<LedgeGrabbingActorSystem*>(w.all[3]);
    CHECK(ledgeSystem != nullptr);
    ledgeSystem->KccExtension_OnValidLedgeDetected(&f, climber, wallHit, V3(0, 3, 2));
    CHECK(f.GetPointer<LedgeGrabbingActor>(climber)->IsLedgeGrabbing);
    CHECK(f.GetPointer<LedgeGrabbingActor>(climber)->Destination == V3(0, 3, 2));
    w.Step(61);
    CHECK(!f.GetPointer<LedgeGrabbingActor>(climber)->IsLedgeGrabbing);
    CHECK(f.GetPointer<Quantum::Transform3D>(climber)->Position == V3(0, 3, 2));

    // Wall running along a wall facing +Z: the runner moves along +Z and speeds up.
    const EntityRef wall = f.Create();
    f.Add(wall, Quantum::Transform3D{V3(2, 1, 0), FPQuaternion::Identity()});
    const EntityRef runner = w.AddCharacter(V3(1, 1, 0));
    f.GetPointer<CharacterController3D>(runner)->Velocity = V3(0, 0, 4);
    WallRunningActor wallRunner{};
    wallRunner.Spec.Id = AssetGuid(711);
    f.Add(runner, wallRunner);
    f.Add<DashActor>(runner);
    WallRunningActor* wr = f.GetPointer<WallRunningActor>(runner);
    CHECK(wr->CanWallRun);
    wr->RunnableWallEntity = wall;
    wr->IsWallRunning = true;
    w.Step(30);
    wr = f.GetPointer<WallRunningActor>(runner);
    CHECK(wr->IsInitialized);
    CHECK(wr->RunningDirection == V3(0, 0, 1));
    CHECK(wr->RunningSpeed > F(4));
    CHECK(f.GetPointer<Quantum::Transform3D>(runner)->Position.Z > F(1));
    // The runner is snapped to the wall's right side at the snap offset (0.5).
    CHECK(f.GetPointer<Quantum::Transform3D>(runner)->Position.X == FP::FromRaw(163840));
    f.GetPointer<ActorInput>(runner)->JumpStateWanted = true;
    w.Step();
    wr = f.GetPointer<WallRunningActor>(runner);
    CHECK(!wr->IsWallRunning);
    CHECK(f.GetPointer<KccExtension>(runner)->jumpState == JumpState::Jumping);
    CHECK(f.GetPointer<KccExtension>(runner)->hasImpulseToConsume ||
          f.GetPointer<CharacterController3D>(runner)->Velocity.Y > FP{});
}
