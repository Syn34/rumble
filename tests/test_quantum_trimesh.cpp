// Tests for static triangle meshes: the map's baked triangle data (TriangleCCW / Map
// serialisation), TriangleMesh cells and the mesh paths of the 3D scene.
#include "test_framework.h"
#include "math_tables.h"

#include "quantum_core/byte_stream.h"
#include "quantum_core/map.h"
#include "quantum_core/triangle_ccw.h"

#include <cstdio>
#include <vector>

using namespace Quantum;

namespace {

FP F(int v) { return FP::FromInt(v); }
FPVector3 V3(int x, int y, int z) { return {F(x), F(y), F(z)}; }

TriangleCCW MakeTriangle(FPVector3 a, FPVector3 b, FPVector3 c, std::int32_t staticIndex) {
    TriangleCCW tri;
    tri.A = a;
    tri.B = b;
    tri.C = c;
    tri.StaticDataIndex = staticIndex;
    tri.ComputeNormal();
    return tri;
}

bool SameTriangle(const TriangleCCW& x, const TriangleCCW& y) {
    return x.A == y.A && x.B == y.B && x.C == y.C && x.Normal == y.Normal && x.Center == y.Center &&
           x.EdgeA == y.EdgeA && x.LengthEdgeC == y.LengthEdgeC && x.BroadRadius == y.BroadRadius &&
           x.MaxY == y.MaxY && x.MinY == y.MinY && x.StaticDataIndex == y.StaticDataIndex;
}

} // namespace

TEST_CASE("TriangleCCW.Serialize writes vertices, optional metadata and the static index") {
    rr::test::InstallMathDerivedTables();
    const TriangleCCW tri = MakeTriangle(V3(0, 0, 0), V3(0, 0, 2), V3(2, 0, 0), 7);

    for (const bool metadata : {false, true}) {
        // The stream writes into a buffer of fixed size.
        std::vector<std::uint8_t> bytes(1024);
        ByteStream out(&bytes);
        TriangleCCW copy = tri;
        copy.Serialize(out, true, metadata);
        // Vertices 72 bytes; metadata 6 FPs and 9 vectors (264 bytes); static index 4 bytes.
        CHECK(out.Position() == (metadata ? 72 + 264 + 4 : 76));

        ByteStream in(&bytes);
        TriangleCCW read;
        read.Serialize(in, false, metadata);
        CHECK(SameTriangle(read, tri));
    }
}

TEST_CASE("Map.SerializeStaticColliderTriangles round-trips the triangle dictionary") {
    rr::test::InstallMathDerivedTables();
    Map map;
    map.StaticColliders3DTriangles[3] = {MakeTriangle(V3(0, 0, 0), V3(0, 0, 1), V3(1, 0, 0), 3),
                                         MakeTriangle(V3(1, 0, 0), V3(0, 0, 1), V3(1, 0, 1), 3)};
    map.StaticColliders3DTriangles[1] = {MakeTriangle(V3(0, 1, 0), V3(0, 1, 4), V3(4, 1, 0), 1)};
    map.SerializeTrianglesMetadata = false;

    std::vector<std::uint8_t> bytes(4096);
    ByteStream out(&bytes);
    map.SerializeStaticColliderTriangles(out, true);
    CHECK(map.OptionalTriangleMetadataSerializationEnabled);
    // count, then (key 1: count, 1 triangle), (key 3: count, 2 triangles)
    CHECK(out.Position() == 4 + 2 * 8 + 3 * 76);
    CHECK(bytes[0] == 2 && bytes[4] == 1); // little-endian count, then the smallest key

    Map loaded;
    loaded.OptionalTriangleMetadataSerializationEnabled = true;
    loaded.SerializeTrianglesMetadata = false;
    ByteStream in(&bytes);
    loaded.SerializeStaticColliderTriangles(in, false);
    CHECK(loaded.StaticColliders3DTriangles.size() == 2u);
    CHECK(loaded.StaticColliders3DTriangles[3].size() == 2u);
    CHECK(SameTriangle(loaded.StaticColliders3DTriangles[3][1], map.StaticColliders3DTriangles[3][1]));
    CHECK(SameTriangle(loaded.StaticColliders3DTriangles[1][0], map.StaticColliders3DTriangles[1][0]));

    // Without the optional flag the reader expects the metadata: written with it, reads back.
    map.SerializeTrianglesMetadata = true;
    std::vector<std::uint8_t> full(4096);
    ByteStream out2(&full);
    map.SerializeStaticColliderTriangles(out2, true);
    Map legacy;
    ByteStream in2(&full);
    legacy.SerializeStaticColliderTriangles(in2, false);
    CHECK(SameTriangle(legacy.StaticColliders3DTriangles[3][0], map.StaticColliders3DTriangles[3][0]));
}

#include "quantum_core/physics3d/collision_manifold3d.h"
#include "quantum_core/triangle_mesh.h"

TEST_CASE("TriangleMesh: cells, hashes, bounds and insertion order") {
    rr::test::InstallMathDerivedTables();
    CHECK(TriangleMesh::CellHash(0, 0, 0) == 0x8000800080008000ull);
    CHECK(TriangleMesh::CellHash(-1, 1, 0) == 0x800080008001'7fffull);
    CHECK(TriangleMesh::BitsRequiredForNumber(0u) == 0 && TriangleMesh::BitsRequiredForNumber(5u) == 3 &&
          TriangleMesh::BitsRequiredForNumber(-1) == 32);

    TriangleMesh mesh;
    CHECK(mesh.Predicted());
    mesh.Initialize(4, 1, 1, 2);
    CHECK(mesh.CellExtent() == 2 && mesh.TriangleCount() == 1 && mesh.TriangleCapacity() == 2);
    // sqrt(3) / 2 * 4, about 3.46
    CHECK(mesh.CellBroadRadius() > FP::FromRaw(226000) && mesh.CellBroadRadius() < FP::FromRaw(228000));
    CHECK(mesh.InsertMany({MakeTriangle(V3(1, 0, 1), V3(1, 0, 2), V3(2, 0, 1), 0)}) == TriangleReference{});
    mesh.SetPredicted(false);

    // A floor triangle spanning cells x -1..1, z -1..1 (y 0), and a small one in cell (0,0,0).
    const TriangleReference big = mesh.InsertMany({MakeTriangle(V3(-3, 0, -3), V3(-3, 0, 3), V3(3, 0, -3), 0)});
    const TriangleReference small = mesh.InsertMany({MakeTriangle(V3(1, 1, 1), V3(1, 1, 2), V3(2, 1, 1), 1),
                                                     MakeTriangle(V3(1, 2, 1), V3(1, 2, 2), V3(2, 2, 1), 1)});
    CHECK(big == TriangleReference(1, true) && small == TriangleReference(3, true));
    CHECK(mesh.TriangleCount() == 4 && mesh.TriangleCapacity() == 4);
    CHECK(mesh.GetTriangle(3)->Next == 2 && mesh.GetTriangle(2)->Next == 0 && mesh.GetTriangle(3)->Index == 3);

    const std::int32_t* tris = nullptr;
    std::int32_t count = 0;
    CHECK(mesh.TryGetTriangles(0, 0, 0, tris, count) && count == 3);
    CHECK(tris[0] == 1 && tris[1] == 2 && tris[2] == 3);
    CHECK(mesh.TryGetTriangles(-1, 0, -1, tris, count) && count == 1 && tris[0] == 1);
    CHECK(!mesh.TryGetTriangles(5, 0, 5, tris, count) && count == 0 && tris == nullptr);

    std::int32_t x = 0, y = 0, z = 0;
    // The raw value is divided (truncating) before the floor: a tiny negative value is cell 0.
    mesh.ToCell(FPVector3{FP::FromRaw(-1), F(5), -F(4)}, x, y, z);
    CHECK(x == 0 && y == 1 && z == -1);

    // Bounds clip to the cells in use (x -1..0, y 0..0, z -1..0).
    Physics3D::IntAABB box;
    box.MinX = static_cast<std::int32_t>(-F(100).RawValue());
    box.MaxX = static_cast<std::int32_t>(F(100).RawValue());
    box.MinY = 0;
    box.MaxY = static_cast<std::int32_t>(F(1).RawValue());
    box.MinZ = static_cast<std::int32_t>(F(1).RawValue());
    box.MaxZ = static_cast<std::int32_t>(F(2).RawValue());
    const TriangleMesh::CellBounds cells = mesh.GetBounds(box);
    CHECK(cells.MinX == -1 && cells.MaxX == 0 && cells.MinY == 0 && cells.MaxY == 0 && cells.MinZ == 0 &&
          cells.MaxZ == 0);
}

TEST_CASE("TriangleMesh: built from a map with immutable and toggleable colliders") {
    rr::test::InstallMathDerivedTables();
    Map map;
    map.Guid = AssetGuid(5);
    map.TriangleMeshCellSize = 8;
    map.StaticColliders3D.resize(3);
    for (auto& collider : map.StaticColliders3D) collider.ShapeType = Shape3DType::Mesh;
    map.StaticColliders3D[1].MutableMode = StaticColliderMutableMode::ToggleableStartOff;
    map.StaticColliders3D[2].MutableMode = StaticColliderMutableMode::ToggleableStartOn;
    map.StaticColliders3DTriangles[0] = {MakeTriangle(V3(0, 0, 0), V3(0, 0, 1), V3(1, 0, 0), 0)};
    map.StaticColliders3DTriangles[1] = {MakeTriangle(V3(0, 1, 0), V3(0, 1, 1), V3(1, 1, 0), 1),
                                         MakeTriangle(V3(0, 2, 0), V3(0, 2, 1), V3(1, 2, 0), 1)};
    map.StaticColliders3DTriangles[2] = {MakeTriangle(V3(0, 3, 0), V3(0, 3, 1), V3(1, 3, 0), 2)};
    CHECK(TriangleMesh::CalculateDefaultTriangleCapacityForMap(map) == 2);

    TriangleMesh mesh;
    mesh.SetPredicted(false);
    mesh.Initialize(map.TriangleMeshCellSize, 4099, 2, 3);
    mesh.InjectImmutableCollidersFromMap(map);
    mesh.LockImmutable();
    CHECK(mesh.TriangleCount() == 2);
    const std::int32_t* tris = nullptr;
    std::int32_t count = 0;
    CHECK(mesh.TryGetTriangles(0, 0, 0, tris, count) && count == 1 && mesh.GetTriangle(tris[0])->CCWTri.StaticDataIndex == 0);
}

// ---------------------------------------------------------------------------------------
// The 3D scene against static meshes

#include "photon_deterministic/fp_math.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/component_prototypes.h"
#include "quantum_core/physics3d/character_controller3d.h"
#include "quantum_core/resource_manager.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/physics3d/physics_engine3d.h"
#include "quantum_core/physics_components.h"
#include "quantum_core/physics_scene_settings.h"
#include "quantum_core/transform.h"

#include <memory>

using namespace Quantum::Core;
using namespace Quantum::Physics3D;
using Photon::Deterministic::FPMath::Abs;

namespace {

struct MeshRegistryScope {
    bool owned = false;
    MeshRegistryScope() {
        if (!ComponentTypeId<>::IsSetUp()) {
            ComponentTypeId<>::Setup({});
            owned = true;
        }
    }
    ~MeshRegistryScope() {
        if (owned) ComponentTypeId<>::ResetForTests();
    }
};

class MeshTestFrame final : public FrameBase {
  public:
    explicit MeshTestFrame(FrameContext* context) : FrameBase(context) {}
    FrameMetaData* get_FrameMetaData() override { return &_meta; }
    CommitCommandsModes get_CommitCommandsMode() const override { return CommitCommandsModes::OnFrameSimulationBegin; }
    FP get_DeltaTime() const override { return FP::FromRaw(65536 / 60); }
    void set_DeltaTime(FP) override {}
    std::string DumpFrame(int) override { return {}; }
    std::uint64_t CalculateChecksum() override { return 0; }
    std::vector<std::uint8_t> Serialize(Photon::Deterministic::DeterministicFrameSerializeMode) override { return {}; }
    void Deserialize(std::span<const std::uint8_t>) override {}
    void Copy(const Photon::Deterministic::DeterministicFrame&) override {}
    Map* get_Map() const override { return map.get(); }
    const PhysicsSceneSettings* get_PhysicsSceneSettings() const override { return &settings; }

    std::shared_ptr<Map> map = std::make_shared<Map>();
    PhysicsSceneSettings settings;

  private:
    FrameMetaData _meta{};
};

// A world whose map has a 20 x 20 floor at y = 0 made of one mesh collider (two triangles
// facing up), plus whatever mesh colliders a test adds.
struct MeshWorld {
    std::unique_ptr<ResourceManagerStatic> resources; // outlives the context
    MeshRegistryScope registry;
    Photon::Deterministic::SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    std::unique_ptr<MeshTestFrame> frame;
    std::unique_ptr<PhysicsEngine3D> engine;

    explicit MeshWorld(std::vector<std::shared_ptr<AssetObject>> assets = {}) {
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
        frame = std::make_unique<MeshTestFrame>(context.get());
        Core::EntitiesConfig config;
        config.DefaultEntityCapacity = 32;
        config.DefaultComponentBlockCapacity = 8;
        FrameBase::Initialize(frame.get(), config);
        frame->Verified = true; // the static mesh only takes triangles on verified frames
        frame->map->Guid = AssetGuid(77);
        PhysicsSceneSettings& s = frame->settings;
        s.SetAllLayersColliding();
        s.SetAllowSleeping(true);
        s.SetUseAngularVelocity(true);
        s.SolverType = ConstraintsSolverType::LegacyGaussSeidel;
        s.SolverIterations = 4;
        s.Gravity = {FP::Zero(), F(-10), FP::Zero()};
        s.PenetrationAllowance = FP::FromRaw(655);
        s.PenetrationCorrection = FP::FromRaw(26214);
        s.SleepTimeSec = FP::_1();
        s.LinearSleepTolerance = FP::FromRaw(6553);
        s.AngularSleepToleranceInRad = FP::FromRaw(6553);
        PhysicsQueryConfig queries;
        queries.ShapeCastMaxIterations = 16;
        queries.ShapeCastHardTolerance = FP::FromRaw(65);
        queries.ShapeCastSoftTolerance = FP::FromRaw(655);
        s.GJKSettingsValue = GJKSettings(queries);
        engine = std::make_unique<PhysicsEngine3D>(context.get());
        AddMesh({MakeTriangle(V3(-10, 0, -10), V3(10, 0, -10), V3(-10, 0, 10), 0),
                 MakeTriangle(V3(10, 0, -10), V3(10, 0, 10), V3(-10, 0, 10), 0)});
    }

    // A mesh collider; the triangles' static index is set to the collider's.
    std::int32_t AddMesh(std::vector<TriangleCCW> tris, bool trigger = false) {
        const auto index = static_cast<std::int32_t>(frame->map->StaticColliders3D.size());
        MapStaticCollider3D collider;
        collider.ShapeType = Shape3DType::Mesh;
        collider.StaticData.IsTrigger = trigger;
        frame->map->StaticColliders3D.push_back(collider);
        for (TriangleCCW& tri : tris) tri.StaticDataIndex = index;
        frame->map->StaticColliders3DTriangles[index] = std::move(tris);
        return index;
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

    void Init() {
        engine->OnFrameSimulationBegin(frame.get());
        engine->Init(frame.get(), AssetGuid{});
    }
    void Step(ICollisionCallbacks3D* callbacks = nullptr) {
        engine->OnFrameSimulationBegin(frame.get());
        engine->Update(frame.get(), callbacks, frame->get_DeltaTime());
        engine->OnFrameSimulationEnd();
    }
};

struct MeshCallbacks final : ICollisionCallbacks3D {
    int collisionEnter = 0, collision = 0, collisionExit = 0, triggerEnter = 0, trigger = 0, triggerExit = 0;
    FPVector3 lastNormal;
    std::int32_t lastStatic = -1;
    void OnCollision3D(FrameBase*, const CollisionInfo3D& info) override {
        ++collision;
        lastNormal = info.ContactNormal();
    }
    void OnCollisionEnter3D(FrameBase*, const CollisionInfo3D& info) override {
        ++collisionEnter;
        lastStatic = info._entryB->StaticColliderIndex;
    }
    void OnCollisionExit3D(FrameBase*, const ExitInfo3D&) override { ++collisionExit; }
    void OnTrigger3D(FrameBase*, const TriggerInfo3D&) override { ++trigger; }
    void OnTriggerEnter3D(FrameBase*, const TriggerInfo3D& info) override {
        ++triggerEnter;
        lastStatic = info._entryA->IsDynamic() ? info._entryB->StaticColliderIndex : info._entryA->StaticColliderIndex;
    }
    void OnTriggerExit3D(FrameBase*, const ExitInfo3D&) override { ++triggerExit; }
};

} // namespace

TEST_CASE("Scene3D mesh: map mesh colliders build the scene's triangle mesh") {
    MeshWorld w;
    w.AddMesh({MakeTriangle(V3(0, 5, 0), V3(1, 5, 0), V3(0, 5, 1), 0)});
    w.Init();
    TriangleMesh* mesh = w.engine->SceneMesh();
    CHECK(mesh->TriangleCount() == 4); // slot 0 + three triangles
    CHECK(w.engine->Scene()->StaticEntriesCount() == 2);
    CHECK(w.engine->Scene()->GetEntry(1)->Shape->Mesh.MeshIndex == 1);
    // Predicted frames add nothing: a second map loaded while predicted stays empty.
    TriangleMesh fresh;
    CHECK(fresh.Predicted());
}

TEST_CASE("Scene3D mesh: bodies fall and rest on a static mesh floor") {
    MeshWorld w;
    const EntityRef box = w.AddBody(V3(0, 3, 0), Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()}), true);
    const EntityRef ball = w.AddBody(V3(4, 2, 4), Shape3D::CreateSphere(FP::_0_50()), true);
    w.frame->Add(box, PhysicsCallbacks3D{});
    w.frame->GetPointer<PhysicsCallbacks3D>(box)->Flags = CallbackFlags::OnStaticCollisionEnter |
                                                         CallbackFlags::OnStaticCollision |
                                                         CallbackFlags::OnStaticCollisionExit;
    w.Init();
    MeshCallbacks recorder;
    for (int i = 0; i < 240; ++i) {
        w.Step(&recorder);
    }
    const Quantum::Transform3D* t = w.frame->GetPointer<Quantum::Transform3D>(box);
    CHECK(t->Position.Y > FP::FromRaw(26214) && t->Position.Y < FP::FromRaw(39322));
    CHECK(w.frame->GetPointer<PhysicsBody3D>(box)->IsSleeping());
    const Quantum::Transform3D* b = w.frame->GetPointer<Quantum::Transform3D>(ball);
    CHECK(b->Position.Y > FP::FromRaw(26214) && b->Position.Y < FP::FromRaw(39322));
    CHECK(recorder.collisionEnter == 1 && recorder.lastStatic == 0);
    CHECK(recorder.collision > 1);
    CHECK(recorder.lastNormal.Y > FP::FromRaw(60000)); // from the floor towards the box
    CHECK(recorder.collisionExit == 0);
}

TEST_CASE("Scene3D mesh: queries hit mesh triangles") {
    MeshWorld w;
    w.AddMesh({MakeTriangle(V3(20, 3, 20), V3(21, 3, 20), V3(20, 3, 21), 0)}); // far away
    w.Init();
    w.Step();
    const int detailed = QueryOptions::HitAll | QueryOptions::ComputeDetailedInfo;

    HitCollection3D ray(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &ray, V3(1, 5, 1), V3(1, -5, 1), false, -1, detailed);
    CHECK(ray.Count() == 1);
    if (ray.Count() == 1) {
        CHECK(ray[0]._staticColliderIndex == 0 && !ray[0].IsDynamic());
        CHECK(Abs(ray[0].Point.Y) < FP::FromRaw(64) && Abs(ray[0].CastDistanceNormalized - FP::_0_50()) < FP::FromRaw(64));
        CHECK(ray[0].Normal.Y > FP::FromRaw(65000));
    }
    // From below the floor's back face: no hit; statics excluded: no hit.
    HitCollection3D back(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &back, V3(1, -5, 1), V3(1, 5, 1), false, -1, detailed);
    CHECK(back.Count() == 0);
    w.engine->Linecast(w.frame.get(), &back, V3(1, 5, 1), V3(1, -5, 1), false, -1, QueryOptions::HitDynamics);
    CHECK(back.Count() == 0);
    // A long diagonal ray crossing many cells, first hit only.
    HitCollection3D diagonal(8, HitCollection3D::Temp, w.context.get());
    w.engine->Linecast(w.frame.get(), &diagonal, V3(-9, 9, -9), V3(9, -9, 9), true, -1, detailed);
    CHECK(diagonal.Count() == 1 && Abs(diagonal[0].Point.X) < FP::FromRaw(256));

    HitCollection3D overlap(8, HitCollection3D::Temp, w.context.get());
    w.engine->OverlapShape(w.frame.get(), &overlap, V3(2, 0, 2), FPQuaternion::Identity(), Shape3D::CreateSphere(FP::_0_50()),
                           -1, detailed);
    CHECK(overlap.Count() >= 1);
    if (overlap.Count() >= 1) {
        CHECK(overlap[0]._staticColliderIndex == 0 && overlap[0].OverlapPenetration > FP::Zero());
        CHECK(overlap[0].Normal.Y > FP::FromRaw(60000)); // from the floor towards the sphere
    }
    HitCollection3D boxOverlap(8, HitCollection3D::Temp, w.context.get());
    w.engine->OverlapShape(w.frame.get(), &boxOverlap, V3(-3, 0, 5), FPQuaternion::Identity(),
                           Shape3D::CreateBox(V3(1, 1, 1)), -1, detailed);
    CHECK(boxOverlap.Count() >= 1);
    HitCollection3D none(8, HitCollection3D::Temp, w.context.get());
    w.engine->OverlapShape(w.frame.get(), &none, V3(2, 3, 2), FPQuaternion::Identity(), Shape3D::CreateSphere(FP::_0_50()),
                           -1, detailed);
    CHECK(none.Count() == 0);

    // A sphere cast down onto the floor: contact after 4 of 10 units.
    HitCollection3D cast(8, HitCollection3D::Temp, w.context.get());
    w.engine->ShapeCast(w.frame.get(), &cast, V3(3, 5, 3), FPQuaternion::Identity(), Shape3D::CreateSphere(F(1)),
                        V3(0, -10, 0), true, -1, detailed);
    CHECK(cast.Count() >= 1);
    if (cast.Count() >= 1) {
        CHECK(cast[0]._staticColliderIndex == 0);
        CHECK(Abs(cast[0].CastDistanceNormalized - FP::FromRaw(26214)) < FP::FromRaw(700));
    }
}

TEST_CASE("Scene3D mesh: triggers against a static mesh trigger") {
    MeshWorld w;
    w.frame->settings.Gravity = FPVector3::Zero();
    // A vertical trigger wall at x = 3 (normal towards -x).
    const std::int32_t wall = w.AddMesh({MakeTriangle(V3(3, -2, -2), V3(3, -2, 2), V3(3, 2, -2), 0),
                                         MakeTriangle(V3(3, -2, 2), V3(3, 2, 2), V3(3, 2, -2), 0)},
                                        true);
    const EntityRef mover = w.AddBody(V3(0, 1, 0), Shape3D::CreateSphere(FP::_0_50()), true);
    w.frame->Add(mover, PhysicsCallbacks3D{});
    w.frame->GetPointer<PhysicsCallbacks3D>(mover)->Flags =
        CallbackFlags::OnStaticTriggerEnter | CallbackFlags::OnStaticTrigger | CallbackFlags::OnStaticTriggerExit;
    w.frame->GetPointer<PhysicsBody3D>(mover)->Velocity = V3(3, 0, 0);
    w.Init();
    MeshCallbacks recorder;
    for (int i = 0; i < 60; ++i) { // reaches x = 2.5 after 50 steps
        w.Step(&recorder);
    }
    CHECK(recorder.triggerEnter == 1 && recorder.lastStatic == wall);
    CHECK(recorder.trigger >= 1);
    for (int i = 0; i < 60; ++i) { // past x = 3.5
        w.Step(&recorder);
    }
    CHECK(recorder.triggerExit == 1);
    // The trigger did not stop the sphere.
    CHECK(w.frame->GetPointer<Quantum::Transform3D>(mover)->Position.X > F(5));
}

TEST_CASE("Scene3D mesh: the hybrid solver rests a box on a mesh") {
    MeshWorld w;
    w.frame->settings.SolverType = ConstraintsSolverType::HybridJacobiGaussSeidel;
    const EntityRef box = w.AddBody(V3(1, 2, -1), Shape3D::CreateBox(FPVector3{FP::_0_50(), FP::_0_50(), FP::_0_50()}), true);
    w.Init();
    for (int i = 0; i < 240; ++i) {
        w.Step();
    }
    const Quantum::Transform3D* t = w.frame->GetPointer<Quantum::Transform3D>(box);
    CHECK(t->Position.Y > FP::FromRaw(26214) && t->Position.Y < FP::FromRaw(39322));
}

TEST_CASE("Scene3D mesh: the character controller stands and walks on a mesh floor") {
    auto config = std::make_shared<CharacterController3DConfig>();
    config->Guid = AssetGuid(500);
    config->Loaded(nullptr, nullptr);
    MeshWorld w({config});
    const EntityRef hero = w.frame->Create();
    w.frame->Add(hero, Quantum::Transform3D{V3(0, 1, 0), FPQuaternion::Identity()});
    Prototypes::CharacterController3D_Prototype prototype;
    prototype.Config.Id = AssetGuid(500);
    PrototypeMaterializationContext materialization{};
    CHECK(prototype.AddToEntity(w.frame.get(), hero, materialization));
    PhysicsEngine3D* engine = w.context->get_Physics3D();
    engine->OnFrameSimulationBegin(w.frame.get());
    engine->Init(w.frame.get(), AssetGuid{});
    engine->Update(w.frame.get(), nullptr, w.frame->get_DeltaTime());
    CHECK(engine->SceneMesh()->TriangleCount() == 3);
    for (int i = 0; i < 90; ++i) {
        w.frame->GetPointer<CharacterController3D>(hero)->Move(w.frame.get(), hero, FPVector3::Zero());
    }
    const Quantum::Transform3D* t = w.frame->GetPointer<Quantum::Transform3D>(hero);
    CharacterController3D* kcc = w.frame->GetPointer<CharacterController3D>(hero);
    CHECK(kcc->Grounded());
    CHECK(Abs(t->Position.Y) < FP::FromRaw(6554));
    const FP startX = t->Position.X;
    for (int i = 0; i < 60; ++i) {
        w.frame->GetPointer<CharacterController3D>(hero)->Move(w.frame.get(), hero, V3(1, 0, 0));
    }
    kcc = w.frame->GetPointer<CharacterController3D>(hero);
    CHECK(kcc->Grounded());
    CHECK(t->Position.X - startX > F(2));
    CHECK(Abs(t->Position.Y) < FP::FromRaw(6554));
}
