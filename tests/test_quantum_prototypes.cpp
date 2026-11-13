// Tests for quantum.core's prototype system: entity prototypes built in code and
// materialised into frames.
#include "test_framework.h"

#include "math_tables.h"
#include "photon_deterministic/fp_quaternion.h"
#include "photon_deterministic/log.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_core/assert.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/component_prototype.h"
#include "quantum_core/component_prototypes.h"
#include "quantum_core/frame_base.h"
#include "quantum_core/frame_checksumer_bit_stream.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/frame_serializer.h"
#include "quantum_core/map.h"
#include "quantum_core/map_entity_id.h"
#include "quantum_core/resource_manager.h"
#include "quantum_core/transform.h"
#include "quantum_core/view.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Quantum;
using namespace Quantum::Prototypes;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPQuaternion;
using Photon::Deterministic::FPVector2;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::Log;
using Photon::Deterministic::SystemAllocator;
using Quantum::Core::FrameBase;
using Quantum::Core::FrameContext;

namespace {

template <class E, class F> bool Throws(F&& f) {
    try {
        f();
    } catch (const E&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// The component registry is process-wide and set up once. These tests only need the
// framework components; when they are the first to set it up, they hand it back
// afterwards so other tests can register their own components.
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

// Collects error/warning log lines while alive.
struct LogCapture {
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
    LogCapture() {
        Log::Init(
            nullptr, [this](const std::string& m) { warnings.push_back(m); },
            [this](const std::string& m) { errors.push_back(m); }, nullptr);
    }
    ~LogCapture() { Log::Init(nullptr, nullptr, nullptr, nullptr); }
};

class PrototypeTestFrame final : public FrameBase {
  public:
    explicit PrototypeTestFrame(FrameContext* context) : FrameBase(context) {}
    FrameMetaData* get_FrameMetaData() override { return &_meta; }
    CommitCommandsModes get_CommitCommandsMode() const override {
        return CommitCommandsModes::OnFrameSimulationBegin;
    }
    FP get_DeltaTime() const override { return FP::_1(); }
    void set_DeltaTime(FP) override {}
    std::string DumpFrame(int) override { return {}; }
    std::uint64_t CalculateChecksum() override {
        FrameChecksumerBitStream stream;
        stream.SetWriting(true);
        FrameSerializer serializer(
            Photon::Deterministic::DeterministicFrameSerializeMode::Serialize, this, &stream);
        serializer.Reset();
        stream.SetWriting(true);
        FrameBase::Serialize(this, &serializer);
        return stream.get_Checksum();
    }
    std::vector<std::uint8_t>
    Serialize(Photon::Deterministic::DeterministicFrameSerializeMode) override {
        return {};
    }
    void Deserialize(std::span<const std::uint8_t>) override {}
    void Copy(const Photon::Deterministic::DeterministicFrame&) override {}

  private:
    FrameMetaData _meta{};
};

// A frame with a static asset database holding `assets`.
struct PrototypeWorld {
    RegistryScope registry;
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<ResourceManagerStatic> manager;
    std::unique_ptr<FrameContext> context;
    std::unique_ptr<PrototypeTestFrame> frame;

    explicit PrototypeWorld(std::vector<std::shared_ptr<AssetObject>> assets) {
        platform.Allocator = &allocator;
        manager = std::make_unique<ResourceManagerStatic>(std::move(assets), nullptr);
        FrameContext::Args args;
        args.PlatformInfo = &platform;
        args.HeapConfig = Quantum::Allocator::Heap::Config(12, 256, 4);
        args.AssetDatabase = manager->CreateAssetDatabase();
        context = std::make_unique<FrameContext>(args);
        frame = std::make_unique<PrototypeTestFrame>(context.get());
        Core::EntitiesConfig config;
        config.DefaultEntityCapacity = 16;
        config.DefaultComponentBlockCapacity = 4;
        FrameBase::Initialize(frame.get(), config);
    }
    ~PrototypeWorld() {
        frame->Free();
        frame.reset();
        context->Dispose();
    }
};

// A test prototype (of the Transform2DVertical component) that records its context and
// listens for the finished entity.
class RecordingPrototype final : public TypedComponentPrototype<Transform2DVertical>,
                                 public IEntityPrototypeMaterializedListener {
  public:
    MapEntityId Link;
    EntityRef LinkedEntity;
    PrototypeMaterializationContext LastContext;
    int Notifications = 0;
    EntityRef NotifiedEntity;
    EntityPrototypeRef NotifiedRef;

    bool AddToEntity(FrameBase* f, EntityRef entity,
                     const PrototypeMaterializationContext& context) override {
        LastContext = context;
        PrototypeValidator::FindMapEntity(Link, context, LinkedEntity);
        return f->Set(entity, Transform2DVertical{FP::FromInt(7), FP::FromInt(8)}) ==
               SetResult::ComponentAdded;
    }
    void Dispatch(ComponentPrototypeVisitorBase&) override {}
    void OnEntityPrototypeMaterialized(FrameBase*, EntityRef entity,
                                       EntityPrototypeRef prototypeRef) override {
        ++Notifications;
        NotifiedEntity = entity;
        NotifiedRef = prototypeRef;
    }
};

std::shared_ptr<Transform3D_Prototype> MakeTransform3D(int x, int y, int z, int yawDegrees) {
    auto prototype = std::make_shared<Transform3D_Prototype>();
    prototype->Position = FPVector3(FP::FromInt(x), FP::FromInt(y), FP::FromInt(z));
    prototype->Rotation = FPVector3(FP::_0(), FP::FromInt(yawDegrees), FP::_0());
    return prototype;
}

} // namespace

TEST_CASE("Prototype references and map entity ids") {
    const EntityPrototypeRef standalone =
        EntityPrototypeRef::FromPrototypeAsset(AssetRefEntityPrototype{AssetGuid(40)});
    CHECK(standalone.Asset.Id == AssetGuid(40) && standalone.InnerAssetId == -1);
    CHECK(standalone.IsValid() && !EntityPrototypeRef::Invalid().IsValid());
    CHECK(standalone.AssetId() == AssetGuid(40));

    Map map;
    map.Guid = AssetGuid(0x100000002ll);
    const EntityPrototypeRef inMap = EntityPrototypeRef::FromMasterAsset(&map, 3);
    CHECK(inMap.Asset.Id == map.Guid && inMap.InnerAssetId == 3);
    CHECK(inMap.GetHashCode() == static_cast<int>(((1u ^ 2u) * 33u) ^ 3u));
    EntityPrototypeRef padded = inMap;
    padded._padding = 9;
    CHECK(padded == inMap);
    CHECK(!(standalone == inMap));
    CHECK(Throws<std::invalid_argument>([] { EntityPrototypeRef::FromMasterAsset(nullptr, 0); }));

    EntityPrototype prototype;
    prototype.Guid = AssetGuid(41);
    CHECK(EntityPrototypeRef::FromPrototypeAsset(&prototype).Asset.Id == AssetGuid(41));
    CHECK(AssetRefEntityPrototype::FromAsset(&prototype).Id == AssetGuid(41));
    CHECK(!AssetRefEntityPrototype::FromAsset(nullptr).Id.IsValid());

    ComponentPrototypeRef componentRef;
    componentRef.Asset = AssetRef{AssetGuid(41)};
    componentRef.InnerAssetId = 2;
    componentRef.ComponentTypeIndex = 5;
    const EntityPrototypeRef entityPart = componentRef.get_EntityPrototypeRef();
    CHECK(entityPart.Asset.Id == AssetGuid(41) && entityPart.InnerAssetId == 2);

    const MapEntityId id = MapEntityId::Create(4);
    CHECK(id.SceneIndexPlusOne == 5 && id.SceneIndex() == 4 && id.IsValid());
    CHECK(!MapEntityId::Invalid().IsValid() && MapEntityId::Invalid().SceneIndex() == -1);
    CHECK(id.ToString() == "4" && id.GetHashCode() == 5);
}

TEST_CASE("Materialisation context resolves map entities") {
    const EntityRef self{3, 7};
    PrototypeMaterializationContext single(self, EntityPrototypeRef{}, nullptr, 2, nullptr);
    EntityRef found;
    // Without a map entity list, scene index 0 is the entity itself.
    CHECK(single.TryGetEntityRef(MapEntityId::Create(0), found) && found == self);
    CHECK(!single.TryGetEntityRef(MapEntityId::Create(1), found) && found == EntityRef::None());
    CHECK(!single.TryGetEntityRef(MapEntityId::Invalid(), found));

    const std::vector<EntityRef> entities{EntityRef{1, 1}, EntityRef::None(), EntityRef{5, 2}};
    EntityPrototypeRef ref;
    ref.Asset = AssetRef{AssetGuid(9)};
    ref.InnerAssetId = 1;
    PrototypeMaterializationContext mapped(self, ref, nullptr, 4, &entities);
    CHECK(mapped.TryGetEntityRef(MapEntityId::Create(2), found) && found == (EntityRef{5, 2}));
    CHECK(!mapped.TryGetEntityRef(MapEntityId::Create(1), found)); // not created
    CHECK(!mapped.TryGetEntityRef(MapEntityId::Create(3), found) && found == EntityRef::None());
    const ComponentPrototypeRef componentRef = mapped.get_ComponentPrototypeRef();
    CHECK(componentRef.Asset.Id == AssetGuid(9) && componentRef.InnerAssetId == 1 &&
          componentRef.ComponentTypeIndex == 4);

    LogCapture log;
    PrototypeValidator::FindMapEntity(MapEntityId::Invalid(), mapped, found);
    CHECK(found == EntityRef::None() && log.errors.empty());
    PrototypeValidator::FindMapEntity(MapEntityId::Create(0), mapped, found);
    CHECK(found == (EntityRef{1, 1}) && log.errors.empty());
    PrototypeValidator::FindMapEntity(MapEntityId::Create(7), mapped, found);
    CHECK(found == EntityRef::None() && log.errors.size() == 1);

    CHECK(PrototypeValidator::CheckLength(3, 3, mapped) == 3 && log.warnings.empty());
    CHECK(PrototypeValidator::CheckLength(5, 2, mapped) == 2 && log.warnings.size() == 1);
    CHECK(PrototypeValidator::CheckLength(0, 2, mapped) == 0 && log.warnings.size() == 2);
}

TEST_CASE("Entity prototype containers sort by component index") {
    RegistryScope registry;
    EntityPrototypeContainer container;
    auto view = std::make_shared<View_Prototype>();
    auto transform3D = MakeTransform3D(1, 2, 3, 0);
    auto transform2D = std::make_shared<Transform2D_Prototype>();
    container.Components = {view, transform3D, transform2D};

    CHECK(transform3D->get_ComponentTypeId() == ComponentTypeId<Transform3D>::Id);
    CHECK(view->get_ComponentTypeId() == ComponentTypeId<>::GetComponentIndex("View"));
    CHECK(container.GetComponentPrototype(typeid(Transform3D)) == transform3D.get());
    CHECK(container.Components[0] == transform2D && container.Components[1] == transform3D &&
          container.Components[2] == view);
    CHECK(!container.HasAnyListeners);
    CHECK(container.GetComponentPrototype(ComponentTypeId<Transform2DVertical>::Id) == nullptr);
    CHECK(EntityPrototypeContainer::InternalBinarySearch(
              container.Components, ComponentTypeId<Transform2DVertical>::Id) == ~2);

    // A listener is noticed after the container is marked dirty.
    auto recorder = std::make_shared<RecordingPrototype>();
    container.Components.push_back(recorder);
    container.SetDirty();
    CHECK(container.GetComponentPrototype(ComponentTypeId<Transform2DVertical>::Id) ==
          recorder.get());
    CHECK(container.HasAnyListeners);
    CHECK(container.Components[2] == recorder);

    // Typed and pending component types both resolve to the registered index.
    PhysicsBody3D_Prototype body;
    CHECK(body.get_ComponentType() == &typeid(PhysicsBody3D));
    CHECK(body.get_ComponentTypeId() == ComponentTypeId<>::GetComponentIndex("PhysicsBody3D"));
    PhysicsCallbacks2D_Prototype callbacks;
    CHECK(callbacks.get_ComponentTypeId() ==
          ComponentTypeId<>::GetComponentIndex("PhysicsCallbacks2D"));
    CHECK(&ComponentPrototype::PrototypeTypeToComponentType<Transform3D_Prototype>() ==
          &typeid(Transform3D));

    EntityPrototypeContainer broken;
    broken.Components = {transform3D, nullptr};
    CHECK(Throws<std::invalid_argument>([&] { broken.EnsureIsSorted(); }));
}

TEST_CASE("Framework prototypes keep the original defaults") {
    PhysicsBody3D_Prototype body3D;
    CHECK(body3D.Config == 0x3A && body3D.Mass == FP::_1());
    CHECK(body3D.Drag.RawValue() == 32768 && body3D.AngularDrag.RawValue() == 32768);
    CHECK(!body3D.GravityScale.HasValue() && body3D.GravityScale._value == FP::_1());
    PhysicsBody2D_Prototype body2D;
    CHECK(body2D.Config == 0x74 && body2D.Version == 0);
    NavMeshAvoidanceObstacle_Prototype obstacle;
    CHECK(obstacle.Radius.RawValue() == 16384 && obstacle.IsActive);
    Joint2D_Prototype joint2D;
    Joint3D_Prototype joint3D;
    CHECK(joint2D.AutoConfigureDistance && joint3D.AutoConfigureDistance);
    PhysicsJoints2D_Prototype joints;
    CHECK(joints.JointConfigs.empty());

    // The default visitor refuses everything; overrides take their prototypes.
    struct TransformVisitor final : ComponentPrototypeVisitorBase {
        int transforms = 0;
        void Visit(Transform3D_Prototype&) override { ++transforms; }
    } visitor;
    Transform3D_Prototype transform;
    transform.Dispatch(visitor);
    CHECK(visitor.transforms == 1);
    CHECK(Throws<std::logic_error>([&] { body3D.Dispatch(visitor); }));
    CHECK(Throws<std::logic_error>([&] { obstacle.Dispatch(visitor); }));
}

TEST_CASE("An entity prototype materialises into a frame") {
    rr::test::InstallMathDerivedTables();

    auto view = std::make_shared<EntityView>();
    view->Guid = AssetGuid(70);

    auto prototype = std::make_shared<EntityPrototype>();
    prototype->Guid = AssetGuid(71);
    prototype->Container = std::make_shared<EntityPrototypeContainer>();
    auto transform = MakeTransform3D(1, 2, 3, 90);
    auto viewPrototype = std::make_shared<View_Prototype>();
    viewPrototype->Current = AssetRefEntityView{view->Guid};
    auto obstacle = std::make_shared<NavMeshAvoidanceObstacle_Prototype>();
    obstacle->AvoidanceLayer = 3;
    auto steering = std::make_shared<NavMeshSteeringAgent_Prototype>();
    steering->OverrideMaxSpeed = true;
    steering->MaxSpeed = FP::FromInt(6);
    auto avoidance = std::make_shared<NavMeshAvoidanceAgent_Prototype>();
    prototype->Container->Components = {viewPrototype, steering, transform, obstacle, avoidance};

    PrototypeWorld world({view, prototype});
    PrototypeTestFrame& frame = *world.frame;

    int signals = 0;
    EntityPrototypeRef signalled;
    frame._SignalOnEntityPrototypeMaterialized = [&](EntityRef, EntityPrototypeRef ref) {
        ++signals;
        signalled = ref;
    };

    const EntityRef entity = frame.Create(prototype.get());
    CHECK(frame.Exists(entity));
    Transform3D* stored = nullptr;
    CHECK(frame.TryGetPointer(entity, stored));
    CHECK(stored->Position == FPVector3(FP::FromInt(1), FP::FromInt(2), FP::FromInt(3)));
    const FPQuaternion expected =
        FPQuaternion::Euler(FPVector3(FP::_0(), FP::FromInt(90), FP::_0()));
    CHECK(stored->Rotation == expected);
    CHECK(frame.Get<View>(entity).Current.Id == AssetGuid(70));
    const NavMeshAvoidanceObstacle storedObstacle = frame.Get<NavMeshAvoidanceObstacle>(entity);
    CHECK(storedObstacle._disabled == 0 && storedObstacle._avoidanceLayer == 3 &&
          storedObstacle.Radius().RawValue() == 16384);
    CHECK(frame.Get<NavMeshSteeringAgent>(entity).MaxSpeed() == FP::FromInt(6));
    CHECK(frame.Has<NavMeshAvoidanceAgent>(entity));
    CHECK(signals == 1 && signalled.Asset.Id == AssetGuid(71) && signalled.InnerAssetId == -1);

    // Through the asset reference: a second, independent entity.
    const EntityRef second = frame.Create(AssetRefEntityPrototype{AssetGuid(71)});
    CHECK(second != entity && frame.Get<Transform3D>(second).Position == stored->Position);
    CHECK(signals == 2);
    CHECK(frame.ComponentCount<Transform3D>() == 2);

    // The prototype system finds its pieces again.
    CHECK(frame.FindPrototype(EntityPrototypeRef::FromPrototypeAsset(prototype.get())) ==
          prototype->Container.get());
    ComponentPrototypeRef componentRef;
    componentRef.Asset = AssetRef{AssetGuid(71)};
    componentRef.InnerAssetId = -1;
    componentRef.ComponentTypeIndex = ComponentTypeId<Transform3D>::Id;
    CHECK(frame.FindPrototype(componentRef) == transform.get());
    CHECK(frame.FindPrototype<Transform3D_Prototype>(componentRef) == transform.get());
    CHECK(frame.FindPrototype<View_Prototype>(componentRef) == nullptr);
    {
        LogCapture log; // a missing asset only warns
        CHECK(frame.FindPrototype(EntityPrototypeRef::FromPrototypeAsset(
                  AssetRefEntityPrototype{AssetGuid(99)})) == nullptr);
    }
    CHECK(Throws<std::logic_error>([&] {
        frame.FindPrototype(
            EntityPrototypeRef::FromPrototypeAsset(AssetRefEntityPrototype{view->Guid}));
    }));

    // Materialising onto an entity that has the components already: the overwritten ones
    // are recorded when a set is given, reported otherwise. The steering agent is kept.
    frame.GetPointer<NavMeshSteeringAgent>(entity)->SetVelocity(FPVector2(FP::_1(), FP::_0()));
    ComponentSet overwritten;
    const EntityPrototypeRef ref = EntityPrototypeRef::FromPrototypeAsset(prototype.get());
    CHECK(frame.MaterializePrototypeInternal(entity, prototype->Container.get(), ref, nullptr,
                                             &overwritten));
    CHECK(overwritten.IsSet(ComponentTypeId<Transform3D>::Id) &&
          overwritten.IsSet(ComponentTypeId<NavMeshSteeringAgent>::Id) &&
          !overwritten.IsSet(ComponentTypeId<Transform2D>::Id));
    CHECK(frame.Get<NavMeshSteeringAgent>(entity).Velocity() == FPVector2(FP::_1(), FP::_0()));
    {
        LogCapture log;
        CHECK(frame.MaterializePrototypeInternal(entity, prototype->Container.get(), ref, nullptr,
                                                 nullptr));
        CHECK(log.errors.size() == prototype->Container->Components.size());
    }

    // The registered serialisers cover every component used here.
    CHECK(frame.CalculateChecksum() != 0);

    // A prototype without a container (or no prototype) fails after creating the entity.
    auto empty = std::make_shared<EntityPrototype>();
    const int before = frame.EntityCount();
    CHECK(Throws<std::invalid_argument>([&] { frame.Create(empty.get()); }));
    CHECK(frame.EntityCount() == before + 1);
}

TEST_CASE("2D transforms convert degrees and pending prototypes refuse to materialise") {
    RegistryScope registry;
    PrototypeWorld world({});
    PrototypeTestFrame& frame = *world.frame;

    auto prototype = std::make_shared<EntityPrototype>();
    prototype->Container = std::make_shared<EntityPrototypeContainer>();
    auto transform2D = std::make_shared<Transform2D_Prototype>();
    transform2D->Position = FPVector2(FP::FromInt(4), FP::FromInt(-2));
    transform2D->Rotation = FP::FromInt(90);
    auto vertical = std::make_shared<Transform2DVertical_Prototype>();
    vertical->Position = FP::FromInt(1);
    vertical->Height = FP::FromInt(3);
    prototype->Container->Components = {vertical, transform2D};

    const EntityRef entity = frame.Create(prototype.get());
    const Transform2D stored = frame.Get<Transform2D>(entity);
    CHECK(stored.Position == transform2D->Position);
    CHECK(stored.Rotation.RawValue() == 90 * 1143); // 90 * Deg2Rad, truncated
    CHECK(frame.Get<Transform2DVertical>(entity).Height == FP::FromInt(3));
    CHECK(frame.Get<Transform2DVertical>(entity).Position == FP::FromInt(1));

    auto physics = std::make_shared<EntityPrototype>();
    physics->Container = std::make_shared<EntityPrototypeContainer>();
    physics->Container->Components = {std::make_shared<PhysicsBody3D_Prototype>()};
    CHECK(Throws<std::logic_error>([&] { frame.Create(physics.get()); }));
}

TEST_CASE("Map entities are created together and linked") {
    RegistryScope registry;

    auto map = std::make_shared<Map>();
    map->Guid = AssetGuid(80);
    auto first = std::make_shared<EntityPrototypeContainer>();
    first->Components = {MakeTransform3D(5, 0, 0, 0)};
    auto second = std::make_shared<EntityPrototypeContainer>();
    auto recorder = std::make_shared<RecordingPrototype>();
    recorder->Link = MapEntityId::Create(0); // refers to the first map entity
    second->Components = {recorder, MakeTransform3D(6, 0, 0, 0)};
    map->MapEntities = {first, second};

    PrototypeWorld world({map});
    PrototypeTestFrame& frame = *world.frame;
    std::vector<EntityRef> signalled;
    std::vector<int> signalledInner;
    frame._SignalOnEntityPrototypeMaterialized = [&](EntityRef entity, EntityPrototypeRef ref) {
        signalled.push_back(entity);
        signalledInner.push_back(ref.InnerAssetId);
    };

    frame.Create(map->MapEntities, map.get());
    CHECK(frame.EntityCount() == 2);
    CHECK(signalled.size() == 2 && signalledInner == (std::vector<int>{0, 1}));
    const EntityRef a = signalled[0];
    const EntityRef b = signalled[1];
    CHECK(frame.Get<MapEntityLink>(a).Index == 0 && frame.Get<MapEntityLink>(b).Index == 1);
    CHECK(frame.Get<Transform3D>(a).Position.X == FP::FromInt(5));
    CHECK(frame.Get<Transform3D>(b).Position.X == FP::FromInt(6));
    CHECK(frame.Has<Transform2DVertical>(b) && !frame.Has<Transform2DVertical>(a));

    // The second entity resolved its link to the first one and was notified once.
    CHECK(recorder->LinkedEntity == a);
    CHECK(recorder->LastContext.Entity == b &&
          recorder->LastContext.EntityPrototypeRef.InnerAssetId == 1);
    CHECK(recorder->LastContext.EntityPrototypeContainer == second.get());
    CHECK(recorder->Notifications == 1 && recorder->NotifiedEntity == b);
    CHECK(recorder->NotifiedRef.Asset.Id == AssetGuid(80) &&
          recorder->NotifiedRef.InnerAssetId == 1);

    // Map entries are found by their index; a bad index is an error.
    CHECK(frame.FindPrototype(EntityPrototypeRef::FromMasterAsset(map.get(), 1)) == second.get());
    CHECK(Throws<std::logic_error>(
        [&] { frame.FindPrototype(EntityPrototypeRef::FromMasterAsset(map.get(), 2)); }));
    CHECK(frame.CalculateChecksum() != 0);
}
