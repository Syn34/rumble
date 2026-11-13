// entity_prototype_loader.cpp - builds entity prototype and map assets from the user's
// content files, including the component prototypes that live on prefabs and in maps
// (PC-side glue; new code).
//
// EntityPrototypeAsset: `Settings` holds only the identifier; `Parent` points at the
// EntityPrototype script on the prefab root. The loader reads that script and the game
// object around it (transform, entity component scripts, an entity view script, the source
// collider if one is set) and hands them to PrepareEntityPrototypeAsset.
// MapAsset: `Settings` is the map; `Prototypes` lists one flat prototype container per
// baked map entity, turned into Map::MapEntities by PrepareMapAsset.
#include "content/entity_prototype_loader.h"

#include "content/prototype_readers.h"
#include "content/quantum_content_provider.h"
#include "content/unity_entity_prototype.h"
#include "photon_deterministic/log.h"
#include "quantum_code/prototypes/flat_entity_prototype_container.h"
#include "quantum_code/prototypes/prototypes.h"
#include "quantum_core/builtin_assets.h"
#include "quantum_core/map.h"
#include "rr/assets/prefab.h"

#include <algorithm>
#include <set>
#include <stdexcept>

namespace rr::content {

using rr::assets::ComponentData;
using rr::assets::ObjectRef;

namespace {

// The game's entity view script and the scripts derived from it (Assembly-CSharp): a
// prefab with any of them has its own view (GetComponent<EntityView>).
const std::set<std::string>& EntityViewScripts() {
    static const std::set<std::string> names = {
        "EntityView",
        "AreaOfEffectEntityView",
        "BigBoomMineView",
        "BombardierDroneView",
        "PlayerActorEntityView",
        "ProjectileView",
        "StreamEffectEntityView",
        "TeamBasedAreaOfEffectEntityView",
        "UltimateTankView",
        "WeaponEntityView",
        "WeaponInstantiatedAmmoViewBase",
    };
    return names;
}

// Entity component scripts are named after this prefix (EntityComponentBase's expected
// type name prefix) and keep their prototype in `Prototype`.
constexpr std::string_view kEntityComponentPrefix = "EntityComponent";

// Reads the fields of the editor-side prototypes, resolving references to prototype
// scripts into ScriptKeys.
class ScriptRefReader {
  public:
    ScriptRefReader(rr::assets::ContentLibrary& library, std::string cab)
        : library_(library), cab_(std::move(cab)) {}

    std::optional<unity::ScriptKey> Ref(const Value& parent, std::string_view name) const {
        const auto pointer = rr::assets::AsObjectPointer(Child(parent, name));
        if (!pointer || pointer->pathId == 0)
            return std::nullopt;
        auto ref = library_.Resolve(cab_, *pointer);
        if (!ref)
            return std::nullopt;
        return unity::ScriptKey{ref->cab, ref->object->pathId};
    }

    void Read(const Value& v, unity::BlackboardValue_Prototype& out) const {
        content::Read(v, "_field_used_", out._field_used_);
        content::Read(v, "BooleanValue", out.BooleanValue);
        content::Read(v, "ByteValue", out.ByteValue);
        content::Read(v, "IntegerValue", out.IntegerValue);
        content::Read(v, "FPValue", out.FPValue);
        content::Read(v, "FPVector2Value", out.FPVector2Value);
        content::Read(v, "FPVector3Value", out.FPVector3Value);
        out.EntityRefValue = Ref(v, "EntityRefValue");
        content::Read(v, "AssetRefValue", out.AssetRefValue);
    }
    void Read(const Value& v, unity::BlackboardEntry_Prototype& out) const {
        if (const Value* value = Child(v, "Value"))
            Read(*value, out.Value);
        content::Read(v, "ReactiveDecorators", out.ReactiveDecorators);
    }
    void Read(const Value& v, unity::AIBlackboardComponent_Prototype& out) const {
        content::Read(v, "Board", out.Board);
        if (const Value* entries = Child(v, "Entries"); entries && entries->AsArray()) {
            for (const auto& item : *entries->AsArray()) {
                auto& entry = out.Entries.emplace_back();
                content::Read(*item, "Key", entry.Key);
                if (const Value* value = Child(*item, "Value"))
                    Read(*value, entry.Value);
            }
        }
    }
    void Read(const Value& v, unity::BlastBallGoal_Prototype& out) const {
        content::Read(v, "OwnerTeam", out.OwnerTeam);
        out.ballEntity = Ref(v, "ballEntity");
        content::Read(v, "ballInside", out.ballInside);
    }
    void Read(const Value& v, unity::Hierarchy_Prototype& out) const {
        out.Parent = Ref(v, "Parent");
        content::Read(v, "Offset", out.Offset);
    }
    void Read(const Value& v, unity::LevelButton_Prototype& out) const {
        out.gameplayElementEntity = Ref(v, "gameplayElementEntity");
        content::Read(v, "Config", out.Config);
    }
    void Read(const Value& v, unity::RailCollider_Prototype& out) const {
        out.railEntity = Ref(v, "railEntity");
        content::Read(v, "distanceOnRail", out.distanceOnRail);
        out.nearbyRailColLeft = Ref(v, "nearbyRailColLeft");
        out.nearbyRailColRight = Ref(v, "nearbyRailColRight");
    }

  private:
    rr::assets::ContentLibrary& library_;
    std::string cab_;
};

// EntityComponentBase<TPrototype>: the prototype is stored as is.
class StoredPrototypeComponent final : public unity::EntityComponentBase {
  public:
    explicit StoredPrototypeComponent(std::shared_ptr<Quantum::ComponentPrototype> prototype)
        : prototype_(std::move(prototype)) {}
    std::shared_ptr<Quantum::ComponentPrototype>
    CreatePrototype(const unity::EntityPrototypeConverter&) const override {
        return prototype_;
    }

  private:
    std::shared_ptr<Quantum::ComponentPrototype> prototype_;
};

// EntityComponentBase<TPrototype, TEditorPrototype>: the editor-side twin is converted.
template <class TEditorPrototype>
class ConvertedPrototypeComponent final : public unity::EntityComponentBase {
  public:
    explicit ConvertedPrototypeComponent(TEditorPrototype prototype)
        : prototype_(std::move(prototype)) {}
    std::shared_ptr<Quantum::ComponentPrototype>
    CreatePrototype(const unity::EntityPrototypeConverter& converter) const override {
        return prototype_.Convert(converter);
    }

  private:
    TEditorPrototype prototype_;
};

template <class TEditorPrototype>
std::unique_ptr<unity::EntityComponentBase> MakeConverted(const ScriptRefReader& reader,
                                                          const Value& value) {
    TEditorPrototype prototype;
    reader.Read(value, prototype);
    return std::make_unique<ConvertedPrototypeComponent<TEditorPrototype>>(std::move(prototype));
}

// The entity component script `component`, or null if it is not one / cannot be read.
std::unique_ptr<unity::EntityComponentBase> ReadEntityComponent(const ComponentData& component,
                                                                rr::assets::ContentLibrary& library,
                                                                const std::string& owner) {
    if (!component.script || !component.value)
        return nullptr;
    const std::string& className = component.script->className;
    if (className.rfind(kEntityComponentPrefix, 0) != 0)
        return nullptr;
    const Value* prototype = component.value->Get("Prototype");
    if (!prototype)
        return nullptr;

    // Scripts whose prototype links to other entities store the editor-side twin.
    const ScriptRefReader reader(library, component.ref.cab);
    if (className == "EntityComponentAIBlackboardComponent")
        return MakeConverted<unity::AIBlackboardComponent_Prototype>(reader, *prototype);
    if (className == "EntityComponentBlastBallGoal")
        return MakeConverted<unity::BlastBallGoal_Prototype>(reader, *prototype);
    if (className == "EntityComponentHierarchy")
        return MakeConverted<unity::Hierarchy_Prototype>(reader, *prototype);
    if (className == "EntityComponentLevelButton")
        return MakeConverted<unity::LevelButton_Prototype>(reader, *prototype);
    if (className == "EntityComponentRailCollider")
        return MakeConverted<unity::RailCollider_Prototype>(reader, *prototype);
    if (className == "EntityComponentPhysicsJoints2D" ||
        className == "EntityComponentPhysicsJoints3D") {
        // TODO(0x7aff304, Quantum.Prototypes.Unity.PhysicsJoints2D_Prototype.Convert): and the
        // 3D one (0x7aff190 Joint3D, 0x7afef78 PhysicsJoints3D): each joint config with its
        // connected entity converted; the game's prefabs have no joints.
        Photon::Deterministic::Log::Warn("{0}: joint prototypes on prefabs are not supported yet",
                                         owner);
        return nullptr;
    }

    auto read = ReadComponentPrototype(prototype->type, *prototype);
    if (!read) {
        Photon::Deterministic::Log::Warn("{0}: unknown component prototype {1}", owner,
                                         prototype->type);
        return nullptr;
    }
    return std::make_unique<StoredPrototypeComponent>(std::move(read));
}

rr::assets::Vector3f ReadVector(const Value* value) {
    if (!value)
        return {};
    const auto component = [&](const char* name) {
        const Value* c = value->Get(name);
        return c ? static_cast<float>(c->AsDouble()) : 0.0f;
    };
    return {component("x"), component("y"), component("z")};
}

// What PreSerialize needs to know about an engine collider used as a shape source: its
// kind and size, and the centre of its bounds, rotation and scale within the prefab.
unity::SourceColliderInfo ReadSourceCollider(const ObjectRef& collider,
                                             const rr::assets::GameObjectData& root,
                                             rr::assets::PrefabReader& reader) {
    constexpr std::int32_t kBoxCollider = 65, kSphereCollider = 135;
    unity::SourceColliderInfo info;
    info.className = rr::assets::ClassName(collider.object->classId);
    if (collider.object->classId == kBoxCollider)
        info.kind = unity::SourceColliderInfo::Kind::Box;
    if (collider.object->classId == kSphereCollider)
        info.kind = unity::SourceColliderInfo::Kind::Sphere;
    const auto data = reader.Library().Decode(collider);
    info.size = ReadVector(data->Get("m_Size"));
    if (const Value* radius = data->Get("m_Radius"))
        info.radius = static_cast<float>(radius->AsDouble());
    if (const Value* trigger = data->Get("m_IsTrigger"))
        info.isTrigger = trigger->AsInt() != 0;
    const rr::assets::Vector3f center = ReadVector(data->Get("m_Center"));

    const auto prefab = reader.ReadHierarchy(root.ref);
    if (!prefab)
        return info;
    for (std::size_t i = 0; i < prefab->objects.size(); ++i) {
        const auto& object = prefab->objects[i];
        const bool owns = std::any_of(
            object.components.begin(), object.components.end(), [&](const ComponentData& c) {
                return c.ref.cab == collider.cab && c.ref.object->pathId == collider.object->pathId;
            });
        if (!owns)
            continue;
        const int index = static_cast<int>(i);
        // The engine's lossy scale: the product of the local scales up the hierarchy.
        rr::assets::Vector3f scale{1, 1, 1};
        for (int j = index; j >= 0; j = prefab->objects[static_cast<std::size_t>(j)].parent) {
            if (const auto& t = prefab->objects[static_cast<std::size_t>(j)].transform) {
                scale = rr::assets::Scale(scale, t->localScale);
            }
        }
        info.lossyScale = scale;
        info.worldRotation = prefab->WorldRotation(index);
        info.worldCenter = prefab->WorldPosition(index) +
                           rr::assets::Rotate(info.worldRotation, rr::assets::Scale(center, scale));
        info.layer = object.layer;
        break;
    }
    return info;
}

// Fills the EntityPrototype script from its decoded data and its prefab.
void ReadEntityPrototypeScript(const Value& v, const rr::assets::GameObjectData& gameObject,
                               rr::assets::PrefabReader& reader, const std::string& cab,
                               unity::EntityPrototype& out) {
    rr::assets::ContentLibrary& library = reader.Library();
    std::int32_t mode = static_cast<std::int32_t>(out.TransformMode);
    Read(v, "TransformMode", mode);
    out.TransformMode = static_cast<unity::EntityPrototypeTransformMode>(mode);

    if (const Value* t = Child(v, "Transform2DVertical")) {
        Read(*t, "IsEnabled", out.Transform2DVertical.IsEnabled);
        Read(*t, "Height", out.Transform2DVertical.Height);
        Read(*t, "PositionOffset", out.Transform2DVertical.PositionOffset);
    }
    if (const Value* c = Child(v, "PhysicsCollider")) {
        auto& collider = out.PhysicsCollider;
        Read(*c, "IsTrigger", collider.IsTrigger);
        Read(*c, "Material", collider.Material);
        Read(*c, "IsEnabled", collider.IsEnabled);
        Read(*c, "Shape3D", collider.Shape3D);
        Read(*c, "Layer", collider.Layer);
        if (const Value* flags = Child(*c, "CallbackFlags"))
            Read(*flags, "Value", collider.CallbackFlags);
        const auto source = rr::assets::AsObjectPointer(Child(*c, "SourceCollider"));
        if (source && source->pathId != 0) {
            if (auto ref = library.Resolve(cab, *source)) {
                collider.SourceCollider = ReadSourceCollider(*ref, gameObject, reader);
            }
        }
    }
    if (const Value* b = Child(v, "PhysicsBody")) {
        auto& body = out.PhysicsBody;
        Read(*b, "IsEnabled", body.IsEnabled);
        Read(*b, "Version2D", body.Version2D);
        Read(*b, "Version3D", body.Version3D);
        Read(*b, "Config2D", body.Config2D);
        Read(*b, "Config3D", body.Config3D);
        Read(*b, "RotationFreeze", body.RotationFreeze);
        Read(*b, "Mass", body.Mass);
        Read(*b, "Drag", body.Drag);
        Read(*b, "AngularDrag", body.AngularDrag);
        Read(*b, "CenterOfMass2D", body.CenterOfMass2D);
        Read(*b, "CenterOfMass3D", body.CenterOfMass3D);
        Read(*b, "GravityScale", body.GravityScale);
    }
    if (const Value* p = Child(v, "NavMeshPathfinder")) {
        auto& pathfinder = out.NavMeshPathfinder;
        Read(*p, "IsEnabled", pathfinder.IsEnabled);
        Read(*p, "NavMeshAgentConfig", pathfinder.NavMeshAgentConfig);
        if (const Value* target = Child(*p, "InitialTarget")) {
            Read(*target, "IsEnabled", pathfinder.InitialTarget.IsEnabled);
            Read(*target, "Position", pathfinder.InitialTarget.Position);
            const auto targetPointer = rr::assets::AsObjectPointer(Child(*target, "Target"));
            if (targetPointer && targetPointer->pathId != 0) {
                // Not resolved: target transforms (the game's prefabs set none).
                Photon::Deterministic::Log::Warn("{0}: navigation target transforms are not read",
                                                 gameObject.name);
            }
            if (const Value* navMesh = Child(*target, "NavMesh")) {
                Read(*navMesh, "Asset", pathfinder.InitialTarget.NavMesh.Asset);
                Read(*navMesh, "Name", pathfinder.InitialTarget.NavMesh.Name);
                const auto reference = rr::assets::AsObjectPointer(Child(*navMesh, "Reference"));
                if (reference && reference->pathId != 0) {
                    if (auto ref = library.Resolve(cab, *reference)) {
                        if (auto owner = reader.GameObjectOf(*ref)) {
                            const auto data = library.Decode(*owner);
                            if (const Value* name = data->Get("m_Name"); name && name->AsString()) {
                                pathfinder.InitialTarget.NavMesh.ReferenceName = *name->AsString();
                            }
                        }
                    }
                }
            }
        }
    }
    if (const Value* s = Child(v, "NavMeshSteeringAgent")) {
        auto& steering = out.NavMeshSteeringAgent;
        Read(*s, "IsEnabled", steering.IsEnabled);
        if (const Value* m = Child(*s, "MaxSpeed")) {
            Read(*m, "IsEnabled", steering.MaxSpeed.IsEnabled);
            Read(*m, "Value", steering.MaxSpeed.Value);
        }
        if (const Value* a = Child(*s, "Acceleration")) {
            Read(*a, "IsEnabled", steering.Acceleration.IsEnabled);
            Read(*a, "Value", steering.Acceleration.Value);
        }
    }
    if (const Value* a = Child(v, "NavMeshAvoidanceAgent"))
        Read(*a, "IsEnabled", out.NavMeshAvoidanceAgent.IsEnabled);
    Read(v, "View", out.View);

    // The prefab root has no parent, so its local transform is its world transform.
    if (gameObject.transform) {
        out.Position = gameObject.transform->localPosition;
        out.Rotation = gameObject.transform->localRotation;
    }
    for (const auto& component : gameObject.components) {
        if (component.script && EntityViewScripts().count(component.script->className))
            out.HasEntityView = true;
    }
}

std::shared_ptr<Quantum::AssetObject> LoadEntityPrototypeAsset(const AssetSource& source) {
    auto asset = std::make_shared<Quantum::EntityPrototype>();
    asset->Container = std::make_shared<Quantum::EntityPrototypeContainer>();
    if (source.settings) {
        if (const Value* identifier = Child(*source.settings, "Identifier")) {
            Read(*identifier, "Path", asset->Path);
            Read(*identifier, "Guid", asset->Guid);
        }
    }

    const auto parentPointer = rr::assets::AsObjectPointer(source.object.Get("Parent"));
    if (!parentPointer || parentPointer->pathId == 0)
        return asset;
    const auto parent = source.library.Resolve(source.ref.cab, *parentPointer);
    if (!parent)
        return asset;

    rr::assets::PrefabReader reader(source.library);
    const auto gameObjectRef = reader.GameObjectOf(*parent);
    const auto gameObject = gameObjectRef ? reader.ReadGameObject(*gameObjectRef) : std::nullopt;
    if (!gameObject)
        return asset;

    unity::EntityPrototype script;
    ReadEntityPrototypeScript(*source.library.Decode(*parent), *gameObject, reader, parent->cab,
                              script);

    std::vector<std::unique_ptr<unity::EntityComponentBase>> components;
    for (const auto& component : gameObject->components) {
        if (auto entityComponent = ReadEntityComponent(component, source.library, asset->Path)) {
            components.push_back(std::move(entityComponent));
        }
    }

    unity::PrepareEntityPrototypeAsset(
        *asset, &script, unity::ScriptKey{parent->cab, parent->object->pathId}, components,
        [&](const std::string& path) { return source.provider.GuidOfPath(path); });
    return asset;
}

std::shared_ptr<Quantum::AssetObject> LoadMapAsset(const AssetSource& source) {
    auto map = std::static_pointer_cast<Quantum::Map>(
        source.settings ? BuildAsset("MapAsset", *source.settings) : nullptr);
    if (!map)
        map = std::make_shared<Quantum::Map>();
    const Value* list = source.object.Get("Prototypes");
    if (!list || !list->AsArray())
        return map;
    std::vector<Quantum::Prototypes::FlatEntityPrototypeContainer> prototypes(
        list->AsArray()->size());
    for (std::size_t i = 0; i < prototypes.size(); ++i) {
        ReadFlatEntityPrototypeContainer(*(*list->AsArray())[i], prototypes[i]);
    }
    unity::PrepareMapAsset(*map, &prototypes);
    return map;
}

} // namespace

void RegisterPrefabAssetFactories() {
    RegisterSourceAssetFactory("EntityPrototypeAsset", &LoadEntityPrototypeAsset);
    RegisterSourceAssetFactory("MapAsset", &LoadMapAsset);
}

} // namespace rr::content
