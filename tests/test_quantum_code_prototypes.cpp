// Tests for the gameplay component prototypes, their flat container, the prefab reader's
// object model and the engine-side entity prototype logic. All data is built in code.
#include "test_framework.h"

#include "content/prototype_readers.h"
#include "content/unity_entity_prototype.h"
#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/log.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/frame.h"
#include "quantum_code/prototypes/flat_entity_prototype_container.h"
#include "quantum_code/prototypes/prototypes.h"
#include "quantum_core/frame_context.h"
#include "quantum_core/map.h"
#include "rr/assets/prefab.h"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Quantum;
using namespace Quantum::Prototypes;
using Photon::Deterministic::FP;
using Photon::Deterministic::FPVector3;
using Photon::Deterministic::Log;
using Photon::Deterministic::SystemAllocator;
using Quantum::Core::FrameContext;
using rr::assets::Value;
using rr::assets::ValuePtr;

namespace {

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

// A game frame without systems.
struct GameFrame {
    SystemAllocator allocator;
    Photon::Deterministic::DeterministicPlatformInfo platform;
    std::unique_ptr<FrameContext> context;
    Photon::Deterministic::DeterministicSessionConfig session;
    RuntimeConfig runtime;
    SimulationConfig simulation;
    std::unique_ptr<Frame> frame;

    GameFrame() {
        ComponentTypeId<>::ResetForTests();
        platform.Allocator = &allocator;
        FrameContext::Args args;
        args.PlatformInfo = &platform;
        args.HeapConfig = Allocator::Heap::Config(12, 256, 4);
        context = std::make_unique<FrameContext>(args);
        session.PlayerCount = 2;
        simulation.Entities.DefaultEntityCapacity = 32;
        simulation.Entities.DefaultComponentBlockCapacity = 8;
        frame = std::make_unique<Frame>(context.get(), std::vector<SystemBase*>{}, std::vector<SystemBase*>{},
                                        &session, &runtime, &simulation, FP::FromRaw(1092));
    }
    ~GameFrame() {
        frame->Free();
        frame.reset();
        context->Dispose();
        ComponentTypeId<>::ResetForTests();
    }
};

// --- decoded-value builders --------------------------------------------------------------------

ValuePtr Int(std::int64_t v) {
    auto value = std::make_shared<Value>();
    value->type = "int";
    value->data = v;
    return value;
}
ValuePtr Float(double v) {
    auto value = std::make_shared<Value>();
    value->type = "float";
    value->data = v;
    return value;
}
ValuePtr Text(const std::string& v) {
    auto value = std::make_shared<Value>();
    value->type = "string";
    value->data = v;
    return value;
}
ValuePtr Struct(const std::string& type, Value::Struct fields) {
    auto value = std::make_shared<Value>();
    value->type = type;
    value->data = std::move(fields);
    return value;
}
ValuePtr List(Value::Array items) {
    auto value = std::make_shared<Value>();
    value->type = "vector";
    value->data = std::move(items);
    return value;
}
ValuePtr Fp(std::int64_t raw) { return Struct("FP", {{"RawValue", Int(raw)}}); }
ValuePtr Guid(std::int64_t id) { return Struct("AssetRef", {{"Id", Struct("AssetGuid", {{"Value", Int(id)}})}}); }
ValuePtr Pointer(std::int64_t pathId) {
    return Struct("PPtr<Component>", {{"m_FileID", Int(0)}, {"m_PathID", Int(pathId)}});
}
ValuePtr Vector3(double x, double y, double z) {
    return Struct("Vector3f", {{"x", Float(x)}, {"y", Float(y)}, {"z", Float(z)}});
}

bool Near(float a, float b) { return std::fabs(a - b) < 1e-4f; }

} // namespace

// --- engine-side object model ------------------------------------------------------------------

TEST_CASE("prefab: transforms and game objects parse from decoded data") {
    const ValuePtr transform = Struct(
        "Transform",
        {{"m_GameObject", Pointer(5)},
         {"m_LocalRotation", Struct("Quaternionf", {{"x", Float(0)}, {"y", Float(0.7071068)}, {"z", Float(0)}, {"w", Float(0.7071068)}})},
         {"m_LocalPosition", Vector3(1, 2, 3)},
         {"m_LocalScale", Vector3(2, 2, 2)},
         {"m_Children", List({Pointer(11), Pointer(12)})},
         {"m_Father", Pointer(0)}});
    const rr::assets::TransformData t = rr::assets::ParseTransform(*transform);
    CHECK(t.localPosition.x == 1 && t.localPosition.y == 2 && t.localPosition.z == 3);
    CHECK(t.localScale.x == 2 && Near(t.localRotation.y, 0.7071068f));
    CHECK(!t.parent);
    CHECK(t.children.size() == 2 && t.children[1].pathId == 12);

    const ValuePtr gameObject = Struct(
        "GameObject",
        {{"m_Component", List({Struct("ComponentPair", {{"component", Pointer(7)}}),
                               Struct("ComponentPair", {{"component", Pointer(8)}})})},
         {"m_Layer", Int(3)},
         {"m_Name", Text("Root")},
         {"m_Tag", Int(0)},
         {"m_IsActive", Int(0)}});
    std::vector<rr::assets::ObjectPointer> components;
    const rr::assets::GameObjectData go = rr::assets::ParseGameObject(*gameObject, components);
    CHECK(go.name == "Root" && go.layer == 3 && !go.active);
    CHECK(components.size() == 2 && components[0].pathId == 7 && components[1].pathId == 8);
}

TEST_CASE("prefab: Euler angles follow the engine's Z-X-Y convention") {
    const float s = std::sqrt(0.5f);
    rr::assets::Vector3f e = rr::assets::EulerAnglesDegrees({0, s, 0, s}); // 90 degrees about Y
    CHECK(Near(e.x, 0) && Near(e.y, 90) && Near(e.z, 0));
    e = rr::assets::EulerAnglesDegrees({0, -s, 0, s}); // -90 about Y reads as 270
    CHECK(Near(e.y, 270));
    e = rr::assets::EulerAnglesDegrees({std::sin(0.25f), 0, 0, std::cos(0.25f)}); // 0.5 rad about X
    CHECK(Near(e.x, 28.64789f) && Near(e.y, 0) && Near(e.z, 0));
    // Yaw 90 then pitch 30: q = qY * qX.
    const rr::assets::Quaternionf qY{0, s, 0, s};
    const rr::assets::Quaternionf qX{std::sin(0.2617994f), 0, 0, std::cos(0.2617994f)};
    e = rr::assets::EulerAnglesDegrees(qY * qX);
    CHECK(Near(e.x, 30) && Near(e.y, 90) && Near(e.z, 0));
    // Tiny negative angles stay as they are instead of wrapping to almost 360.
    e = rr::assets::MakePositiveDegrees({-0.001f, -10, 365});
    CHECK(Near(e.x, -0.001f) && Near(e.y, 350) && Near(e.z, 5));
}

TEST_CASE("prefab: world transforms compose up the hierarchy") {
    rr::assets::Prefab prefab;
    prefab.objects.resize(2);
    const float s = std::sqrt(0.5f);
    prefab.objects[0].transform = rr::assets::TransformData{{10, 0, 0}, {0, s, 0, s}, {2, 2, 2}, std::nullopt, {}};
    prefab.objects[1].transform = rr::assets::TransformData{{1, 0, 0}, {}, {1, 1, 1}, std::nullopt, {}};
    prefab.objects[1].parent = 0;
    // The child sits one unit along the parent's x axis, scaled by 2 and turned 90 degrees
    // about y: x becomes -z.
    const rr::assets::Vector3f p = prefab.WorldPosition(1);
    CHECK(Near(p.x, 10) && Near(p.y, 0) && Near(p.z, -2));
    const rr::assets::Quaternionf q = prefab.WorldRotation(1);
    CHECK(Near(q.y, s) && Near(q.w, s));
}

// --- gameplay prototypes -----------------------------------------------------------------------

TEST_CASE("quantum.code prototypes: simple, linked and user-extended components materialise") {
    GameFrame world;
    Frame& f = *world.frame;
    const EntityRef self = f.Create();
    const EntityRef other = f.Create();
    const std::vector<EntityRef> mapEntities{self, other};
    EntityPrototypeContainer container;
    PrototypeMaterializationContext context(self, EntityPrototypeRef{AssetRef{AssetGuid(900)}, 3, 0}, &container, 7,
                                            &mapEntities);

    Health_Prototype health;
    health.Spec.Id = AssetGuid(55);
    CHECK(health.AddToEntity(&f, self, context));
    CHECK(f.Get<Health>(self).Spec.Id == AssetGuid(55));
    CHECK(!health.AddToEntity(&f, self, context)); // replaced, not new

    Hierarchy_Prototype hierarchy;
    hierarchy.Parent = MapEntityId::Create(1);
    hierarchy.Offset = FPVector3(FP::FromInt(1), FP::FromInt(2), FP::FromInt(3));
    CHECK(hierarchy.AddToEntity(&f, self, context));
    CHECK(f.Get<Hierarchy>(self).Parent == other);
    CHECK(f.Get<Hierarchy>(self).Offset == hierarchy.Offset);

    // Killable keeps its extra fields on the prototype and a reference to it on the component.
    Killable_Prototype killable;
    CHECK(killable.OnKillDisablePhysics && killable.OnKillDisableCollider);
    killable.deathAnimationSpec.Id = AssetGuid(12);
    CHECK(killable.AddToEntity(&f, other, context));
    const Killable stored = f.Get<Killable>(other);
    CHECK(stored.deathAnimationSpec.Id == AssetGuid(12));
    CHECK(stored.Prototype.Asset.Id == AssetGuid(900) && stored.Prototype.InnerAssetId == 3 &&
          stored.Prototype.ComponentTypeIndex == 7);

    // Prototypes dispatch to the game's visitor only.
    struct Counter final : ComponentPrototypeVisitor {
        using ComponentPrototypeVisitor::Visit;
        int healths = 0;
        void Visit(Health_Prototype&) override { ++healths; }
    } counter;
    health.Dispatch(counter);
    CHECK(counter.healths == 1);
    bool threw = false;
    try {
        killable.Dispatch(counter); // not handled: falls back and throws
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("quantum.code prototypes: arrays become heap collections or fixed buffers") {
    LogCapture log;
    GameFrame world;
    Frame& f = *world.frame;
    const EntityRef self = f.Create();
    const EntityRef target = f.Create();
    const std::vector<EntityRef> mapEntities{self, target};
    PrototypeMaterializationContext context(self, EntityPrototypeRef{}, nullptr, 0, &mapEntities);

    OnTriggerEnter_Prototype trigger;
    CHECK(trigger.EntityFilter == static_cast<ENTITY_INFO_FILTER>(-1));
    trigger.onTriggerEnter = {AssetRefCallbackAction{AssetGuid(1)}, AssetRefCallbackAction{AssetGuid(2)}};
    CHECK(trigger.AddToEntity(&f, self, context));
    const auto list = f.ResolveList(f.Get<OnTriggerEnter>(self).get_onTriggerEnter());
    CHECK(list.get_Count() == 2 && list.Get(1).Id == AssetGuid(2));

    // A dictionary of entries whose union value links to another map entity; the second
    // entry repeats a key and is reported.
    AIBlackboardComponent_Prototype blackboard;
    blackboard.Board.Id = AssetGuid(33);
    blackboard.Entries.resize(3);
    blackboard.Entries[0].Key = 4;
    blackboard.Entries[0].Value.Value._field_used_ = "ENTITYREFVALUE";
    blackboard.Entries[0].Value.Value.EntityRefValue = MapEntityId::Create(1);
    blackboard.Entries[1].Key = 9;
    blackboard.Entries[1].Value.Value._field_used_ = "INTEGERVALUE";
    blackboard.Entries[1].Value.Value.IntegerValue = 42;
    blackboard.Entries[2].Key = 4;
    CHECK(blackboard.AddToEntity(&f, self, context));
    const AIBlackboardComponent board = f.Get<AIBlackboardComponent>(self);
    CHECK(board.Board.Id == AssetGuid(33));
    auto entries = f.ResolveDictionary(board.get_Entries());
    CHECK(entries.get_Count() == 2);
    BlackboardEntry entry{};
    CHECK(entries.TryGetValue(4, entry) && *entry.Value.get_EntityRefValue() == target);
    CHECK(entries.TryGetValue(9, entry) && *entry.Value.get_IntegerValue() == 42);
    CHECK(log.errors.size() == 1);

    // Fixed-size buffers take as many values as fit and warn about the difference.
    UnlockedFeaturesManager_Prototype features;
    CHECK(features.unlockedFeatures.size() == 19);
    features.unlockedFeatures.assign(21, QBoolean{});
    features.unlockedFeatures[18].Value = 1;
    CHECK(features.AddToEntity(&f, self, context));
    CHECK(f.Get<UnlockedFeaturesManager>(self).get_unlockedFeatures()[18].Value == 1);
    CHECK(log.warnings.size() == 1);

    // An unknown union field is an error and leaves the union unset.
    BlackboardValue_Prototype unknown;
    unknown._field_used_ = "NOPE";
    BlackboardValue value{};
    unknown.Materialize(&f, value, context);
    CHECK(value.get_Field() == 0 && log.errors.size() == 2);
}

TEST_CASE("quantum.code prototypes: flat containers store and collect by type") {
    FlatEntityPrototypeContainer flat;
    auto health = std::make_shared<Health_Prototype>();
    auto transform = std::make_shared<Transform3D_Prototype>();
    auto killable = std::make_shared<Killable_Prototype>();
    auto health2 = std::make_shared<Health_Prototype>();
    flat.Store({health, transform, killable, health2});
    CHECK(flat.Health.size() == 2 && flat.Health[1] == health2);
    CHECK(flat.Transform3D.size() == 1 && flat.Killable.size() == 1);

    // Framework prototypes first, then the game's in list order.
    std::vector<std::shared_ptr<ComponentPrototype>> all;
    flat.Collect(all);
    CHECK(all.size() == 4);
    CHECK(all[0] == transform && all[1] == health && all[2] == health2 && all[3] == killable);

    // A map turns each flat container into one entity prototype container.
    Map map;
    std::vector<FlatEntityPrototypeContainer> entities(2);
    entities[1].Store({health});
    rr::content::unity::PrepareMapAsset(map, &entities);
    CHECK(map.MapEntities.size() == 2);
    CHECK(map.MapEntities[0]->Components.empty() && map.MapEntities[1]->Components.size() == 1);
}

TEST_CASE("quantum.code prototypes: readers build prototypes from decoded data") {
    const ValuePtr killable = Struct("Killable_Prototype", {{"deathAnimationSpec", Guid(8)},
                                                             {"OnKillBehaviour", Int(1)},
                                                             {"DelayBeforeDestruction", Fp(1 << 16)},
                                                             {"OnKillDisablePhysics", Int(0)},
                                                             {"OnKillDisableCollider", Int(1)},
                                                             {"EntityToSpawnOnDeath", Guid(9)}});
    auto prototype = rr::content::ReadComponentPrototype(killable->type, *killable);
    auto* typed = dynamic_cast<Killable_Prototype*>(prototype.get());
    CHECK(typed != nullptr);
    CHECK(typed->deathAnimationSpec.Id == AssetGuid(8) && typed->EntityToSpawnOnDeath.Id == AssetGuid(9));
    CHECK(typed->DelayBeforeDestruction == FP::_1() && !typed->OnKillDisablePhysics);
    CHECK(rr::content::ReadComponentPrototype("NotAPrototype", *killable) == nullptr);

    const ValuePtr hierarchy = Struct("Hierarchy_Prototype",
                                      {{"Parent", Struct("MapEntityId", {{"SceneIndexPlusOne", Int(3)}})},
                                       {"Offset", Struct("FPVector3", {{"X", Fp(1)}, {"Y", Fp(2)}, {"Z", Fp(3)}})}});
    const ValuePtr transform = Struct("Transform3D_Prototype", {{"Position", Struct("FPVector3", {{"X", Fp(65536)}})}});
    const ValuePtr flatValue =
        Struct("FlatEntityPrototypeContainer", {{"Hierarchy", List({hierarchy})}, {"Transform3D", List({transform})}});
    FlatEntityPrototypeContainer flat;
    rr::content::ReadFlatEntityPrototypeContainer(*flatValue, flat);
    CHECK(flat.Hierarchy.size() == 1 && flat.Hierarchy[0]->Parent.SceneIndex() == 2);
    CHECK(flat.Hierarchy[0]->Offset.Z.RawValue() == 3);
    CHECK(flat.Transform3D.size() == 1 && flat.Transform3D[0]->Position.X == FP::_1());
}

// --- engine-side entity prototype logic ------------------------------------------------------

TEST_CASE("entity prototype script: implied framework components") {
    namespace unity = rr::content::unity;
    using unity::ComponentPrototypeList;
    unity::EntityPrototype script;
    script.TransformMode = unity::EntityPrototypeTransformMode::Transform3D;
    script.Position = {1, 2, 3};
    const float s = std::sqrt(0.5f);
    script.Rotation = {0, s, 0, s};
    script.PhysicsCollider.IsEnabled = true;
    script.PhysicsCollider.Layer = 5;
    script.PhysicsCollider.CallbackFlags = 128;
    script.PhysicsCollider.Shape3D.ShapeType = Shape3DType::Sphere;
    script.PhysicsBody.IsEnabled = true;
    script.PhysicsBody.Config3D = 0x18;
    script.NavMeshPathfinder.IsEnabled = true;
    script.NavMeshPathfinder.NavMeshAgentConfig.Id = AssetGuid(6);
    script.NavMeshSteeringAgent.IsEnabled = true;
    script.NavMeshSteeringAgent.MaxSpeed = {true, FP::FromInt(4)};
    script.NavMeshAvoidanceAgent.IsEnabled = true;
    script.View.Id = AssetGuid(77);

    script.PreSerialize();
    // Version 0 configs gain 0x20 (v1) and lose 0x40 (v2).
    CHECK(script.PhysicsBody.Version3D == 2 && script.PhysicsBody.Config3D == 0x38);

    ComponentPrototypeList prototypes;
    bool selfView = true;
    script.SerializeImplicitComponents(prototypes, selfView);
    CHECK(!selfView);
    CHECK(prototypes.size() == 8);
    auto* transform = dynamic_cast<Transform3D_Prototype*>(prototypes[0].get());
    CHECK(transform && transform->Position == FPVector3(FP::FromInt(1), FP::FromInt(2), FP::FromInt(3)));
    CHECK(transform && transform->Rotation.Y.AsFloat() > 89.99f && transform->Rotation.Y.AsFloat() < 90.01f);
    auto* collider = dynamic_cast<PhysicsCollider3D_Prototype*>(prototypes[1].get());
    CHECK(collider && collider->Layer == 5 && collider->ShapeConfig->ShapeType == Shape3DType::Sphere);
    auto* callbacks = dynamic_cast<PhysicsCallbacks3D_Prototype*>(prototypes[2].get());
    CHECK(callbacks && callbacks->CallbackFlags == 128);
    auto* body = dynamic_cast<PhysicsBody3D_Prototype*>(prototypes[3].get());
    CHECK(body && body->Config == 0x38 && body->Version == 2);
    CHECK(dynamic_cast<NavMeshPathfinder_Prototype*>(prototypes[4].get()) != nullptr);
    auto* steering = dynamic_cast<NavMeshSteeringAgent_Prototype*>(prototypes[5].get());
    CHECK(steering && steering->OverrideMaxSpeed && steering->MaxSpeed == FP::FromInt(4));
    CHECK(dynamic_cast<NavMeshAvoidanceAgent_Prototype*>(prototypes[6].get()) != nullptr);
    auto* view = dynamic_cast<View_Prototype*>(prototypes[7].get());
    CHECK(view && view->Current.Id == AssetGuid(77));

    // A trigger collider has no body; with its own view script the View field is ignored.
    script.PhysicsCollider.IsTrigger = true;
    script.HasEntityView = true;
    script.NavMeshPathfinder.IsEnabled = false;
    prototypes.clear();
    script.SerializeImplicitComponents(prototypes, selfView);
    CHECK(selfView && prototypes.size() == 3);
}

TEST_CASE("entity prototype script: source colliders and the converter") {
    namespace unity = rr::content::unity;
    using unity::ComponentPrototypeList;
    unity::EntityPrototype script;
    script.Position = {0, 1, 0};
    unity::SourceColliderInfo box;
    box.kind = unity::SourceColliderInfo::Kind::Box;
    box.size = {1, 2, 1};
    box.lossyScale = {2, 1, 4};
    box.worldCenter = {0, 1, 0};
    box.isTrigger = true;
    box.layer = 9;
    script.PhysicsCollider.SourceCollider = box;
    script.PreSerialize();
    CHECK(script.PhysicsCollider.Shape3D.ShapeType == Shape3DType::Box);
    CHECK(script.PhysicsCollider.Shape3D.BoxExtents ==
          FPVector3(FP::FromInt(1), FP::FromInt(1), FP::FromInt(2)));
    CHECK(script.PhysicsCollider.Shape3D.IsSetFromSourceCollider);
    CHECK(script.PhysicsCollider.IsTrigger && script.PhysicsCollider.Layer == 9);

    script.PhysicsCollider.SourceCollider->worldCenter = {0, 2, 0}; // off-centre
    bool threw = false;
    try {
        script.PreSerialize();
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);

    const unity::ScriptKey self{"CAB-a", 10}, other{"CAB-a", 11};
    const unity::EntityPrototypeConverter asset(self);
    MapEntityId id;
    asset.Convert(self, id);
    CHECK(id == MapEntityId::Create(0));
    asset.Convert(other, id);
    CHECK(!id.IsValid());
    const unity::EntityPrototypeConverter map(std::vector<unity::ScriptKey>{other, self});
    map.Convert(self, id);
    CHECK(id == MapEntityId::Create(1));

    unity::RailCollider_Prototype rail;
    rail.railEntity = self;
    rail.nearbyRailColLeft = other;
    auto converted = rail.Convert(asset);
    CHECK(converted->railEntity == MapEntityId::Create(0) && !converted->nearbyRailColLeft.IsValid());
}

TEST_CASE("entity prototype asset: containers are filled from the prefab") {
    namespace unity = rr::content::unity;
    using unity::ComponentPrototypeList;
    LogCapture log;
    struct Stored final : unity::EntityComponentBase {
        std::shared_ptr<ComponentPrototype> prototype;
        std::shared_ptr<ComponentPrototype> CreatePrototype(const unity::EntityPrototypeConverter&) const override {
            return prototype;
        }
    };
    Quantum::EntityPrototype asset;
    asset.Path = "Prefabs/Thing|EntityPrototype";
    unity::EntityPrototype script;
    script.PhysicsCollider.IsEnabled = false;
    script.HasEntityView = true;
    std::vector<std::unique_ptr<unity::EntityComponentBase>> components;
    auto health = std::make_shared<Health_Prototype>();
    auto stored = std::make_unique<Stored>();
    stored->prototype = health;
    components.push_back(std::move(stored));

    std::string asked;
    unity::PrepareEntityPrototypeAsset(asset, &script, unity::ScriptKey{"CAB-b", 1}, components, [&](const std::string& path) {
        asked = path;
        return AssetGuid(5);
    });
    CHECK(asked == "Prefabs/Thing|EntityView");
    CHECK(asset.Container && asset.Container->Components.size() == 3); // transform, view, health
    auto* view = dynamic_cast<View_Prototype*>(asset.Container->Components[1].get());
    CHECK(view && view->Current.Id == AssetGuid(5));
    CHECK(asset.Container->Components[2] == health);
    CHECK(log.errors.empty());

    // A missing nested view is reported and skipped.
    unity::PrepareEntityPrototypeAsset(asset, &script, unity::ScriptKey{"CAB-b", 1}, components,
                                [](const std::string&) { return AssetGuid(); });
    CHECK(asset.Container->Components.size() == 2 && log.errors.size() == 1);
}
