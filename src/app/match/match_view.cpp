#include "match_view.h"

#include "quantum_core/transform.h"
#include "quantum_core/view.h"
#include "quantum_code/components.h"
#include "rr/scene/math.h"

#include <cmath>

namespace rr::app {

namespace {

using Quantum::EntityRef;

std::int64_t Key(EntityRef e) { return static_cast<std::int64_t>(static_cast<std::uint32_t>(e.Index)) | (std::int64_t{e.Version} << 32); }

scene::Vec3 ToVec3(const Photon::Deterministic::FPVector3& v) { return {v.X.AsFloat(), v.Y.AsFloat(), v.Z.AsFloat()}; }
scene::Quat ToQuat(const Photon::Deterministic::FPQuaternion& q) {
    return {q.X.AsFloat(), q.Y.AsFloat(), q.Z.AsFloat(), q.W.AsFloat()};
}

scene::Vec3 Lerp(scene::Vec3 a, scene::Vec3 b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }
scene::Quat Nlerp(scene::Quat a, scene::Quat b, float t) {
    const float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    const float s = dot < 0 ? -1.0f : 1.0f;
    return scene::Normalize(scene::Quat{a.x + (s * b.x - a.x) * t, a.y + (s * b.y - a.y) * t, a.z + (s * b.z - a.z) * t,
                                        a.w + (s * b.w - a.w) * t});
}

// The prefab of an EntityView asset: its script object points at the EntityView component
// on the prefab's root ("View"); some point at the game object directly.
std::optional<assets::ObjectRef> ViewPrefabRoot(GameContent& content, assets::PrefabReader& reader, Quantum::AssetGuid guid) {
    auto source = content.Provider().SourceOf(guid);
    if (!source) return std::nullopt;
    const auto value = content.Library().Decode(*source);
    if (!value) return std::nullopt;
    const assets::Value::Struct* fields = value->AsStruct();
    if (!fields) return std::nullopt;
    std::vector<assets::ObjectPointer> candidates;
    if (auto view = assets::AsObjectPointer(value->Get("View"))) candidates.push_back(*view);
    for (const auto& field : *fields) {
        if (field.name == "m_GameObject" || field.name == "m_Script" || field.name == "View") continue;
        if (field.value && field.value->type.rfind("PPtr<", 0) == 0) {
            if (auto pointer = assets::AsObjectPointer(field.value.get())) candidates.push_back(*pointer);
        }
    }
    for (const auto& pointer : candidates) {
        if (pointer.pathId == 0) continue;
        auto target = content.Library().Resolve(source->cab, pointer);
        if (!target) continue;
        if (target->object->classId == static_cast<std::int32_t>(assets::ClassId::GameObject)) return target;
        if (auto gameObject = reader.GameObjectOf(*target)) return gameObject;
    }
    return std::nullopt;
}

} // namespace

MatchView::MatchView(GameContent& content)
    : content_(content), store_(content.Library()), resolver_(content.Library(), store_, resources_),
      reader_(content.Library()) {}

const assets::Prefab* MatchView::PrefabOf(Quantum::AssetGuid view) {
    auto it = prefabs_.find(view.Value);
    if (it == prefabs_.end()) {
        std::optional<assets::Prefab> prefab;
        try {
            if (auto root = ViewPrefabRoot(content_, reader_, view)) prefab = reader_.ReadHierarchy(*root);
        } catch (const std::exception&) {
            prefab.reset();
        }
        ++stats_.viewAssets;
        if (!prefab) ++stats_.viewsWithoutPrefab;
        it = prefabs_.emplace(view.Value, std::move(prefab)).first;
    }
    return it->second ? &*it->second : nullptr;
}

int MatchView::Spawn(Quantum::AssetGuid view) {
    auto& pool = pooled_[view.Value];
    if (!pool.empty()) {
        const int node = pool.back();
        pool.pop_back();
        scene_.Nodes()[static_cast<std::size_t>(node)].active = true;
        return node;
    }
    const assets::Prefab* prefab = PrefabOf(view);
    if (prefab == nullptr) return -1;
    scene::PrefabInstancer instancer(scene_, resolver_, resources_);
    const int node = instancer.Instantiate(*prefab);
    ++stats_.instancesCreated;
    return node;
}

void MatchView::Sync(Quantum::Frame& current, Quantum::Frame* previous, float alpha) {
    for (auto& [key, instance] : instances_) instance.seen = false;
    stats_.viewedEntities = 0;
    stats_.entitiesWithoutPrefab = 0;
    scene::Vec3 sum{0, 0, 0};
    int counted = 0;
    std::optional<scene::Vec3> playerFocus;

    for (const auto& pair : current.GetComponentIterator<Quantum::View>().Range()) {
        const Quantum::EntityRef entity = pair.Entity;
        const Quantum::AssetGuid viewGuid = pair.Component.Current.Id;
        if (!viewGuid.IsValid()) continue;
        Quantum::Transform3D* transform = nullptr;
        if (!current.TryGetPointer<Quantum::Transform3D>(entity, transform)) continue;
        ++stats_.viewedEntities;

        auto it = instances_.find(Key(entity));
        if (it != instances_.end() && it->second.viewGuid != viewGuid.Value) {
            // The entity changed its view: put the old instance back in its pool.
            scene_.Nodes()[static_cast<std::size_t>(it->second.node)].active = false;
            pooled_[it->second.viewGuid].push_back(it->second.node);
            instances_.erase(it);
            it = instances_.end();
        }
        if (it == instances_.end()) {
            const int node = Spawn(viewGuid);
            if (node < 0) {
                ++stats_.entitiesWithoutPrefab;
                continue;
            }
            it = instances_.emplace(Key(entity), Instance{node, viewGuid.Value, false}).first;
        }
        it->second.seen = true;

        scene::Vec3 position = ToVec3(transform->Position);
        scene::Quat rotation = ToQuat(transform->Rotation);
        Quantum::Transform3D* before = nullptr;
        if (previous != nullptr && previous->Exists(entity) && previous->TryGetPointer<Quantum::Transform3D>(entity, before)) {
            position = Lerp(ToVec3(before->Position), position, alpha);
            rotation = Nlerp(ToQuat(before->Rotation), rotation, alpha);
        }
        scene::Node& node = scene_.Nodes()[static_cast<std::size_t>(it->second.node)];
        node.localPosition = scene::MirrorPosition(position);
        node.localRotation = scene::MirrorRotation(rotation);
        sum = sum + position;
        ++counted;

        Quantum::PlayerActor* actor = nullptr;
        if (current.TryGetPointer<Quantum::PlayerActor>(entity, actor) && actor->playerRef._index == 1) playerFocus = position;
    }

    // Instances whose entity is gone go back to their pools.
    for (auto it = instances_.begin(); it != instances_.end();) {
        if (!it->second.seen) {
            scene_.Nodes()[static_cast<std::size_t>(it->second.node)].active = false;
            pooled_[it->second.viewGuid].push_back(it->second.node);
            it = instances_.erase(it);
        } else {
            ++it;
        }
    }
    stats_.instances = instances_.size();
    if (playerFocus) {
        focus_ = *playerFocus;
        haveFocus_ = true;
    } else if (counted > 0 && !haveFocus_) {
        focus_ = {sum.x / static_cast<float>(counted), sum.y / static_cast<float>(counted), sum.z / static_cast<float>(counted)};
        haveFocus_ = true;
    }
    scene_.UpdateTransforms();
    scene_.UpdateBounds(resources_);
}

scene::Camera MatchView::Camera(const CameraAngles& angles) const {
    // Orbit in the game's coordinates, then mirror into the engine's.
    const float yaw = angles.yawDegrees * 3.14159265f / 180.0f;
    const float pitch = angles.pitchDegrees * 3.14159265f / 180.0f;
    const scene::Vec3 forward{std::sin(yaw) * std::cos(pitch), -std::sin(pitch), std::cos(yaw) * std::cos(pitch)};
    const scene::Vec3 target = focus_ + scene::Vec3{0, 1.2f, 0};
    const scene::Vec3 eye{target.x - forward.x * angles.distance, target.y - forward.y * angles.distance,
                          target.z - forward.z * angles.distance};
    scene::Camera camera;
    camera.view = scene::LookAt(scene::MirrorPosition(eye), scene::MirrorPosition(target), {0, 1, 0});
    camera.fovY = 0.9f;
    camera.nearPlane = 0.1f;
    camera.farPlane = 500.0f;
    return camera;
}

} // namespace rr::app
