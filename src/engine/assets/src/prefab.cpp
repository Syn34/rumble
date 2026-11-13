#include "rr/assets/prefab.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace rr::assets {

namespace {

// RectTransform (UI objects) is a Transform with extra layout fields.
constexpr std::int32_t kRectTransformClass = 224;

bool IsTransformClass(std::int32_t classId) {
    return classId == static_cast<std::int32_t>(ClassId::Transform) ||
           classId == kRectTransformClass;
}

float AsFloat(const Value* value, float fallback) {
    return value ? static_cast<float>(value->AsDouble(fallback)) : fallback;
}

Vector3f ReadVector3(const Value* value, Vector3f fallback) {
    if (!value)
        return fallback;
    return {AsFloat(value->Get("x"), fallback.x), AsFloat(value->Get("y"), fallback.y),
            AsFloat(value->Get("z"), fallback.z)};
}

Quaternionf ReadQuaternion(const Value* value) {
    Quaternionf q;
    if (!value)
        return q;
    q.x = AsFloat(value->Get("x"), q.x);
    q.y = AsFloat(value->Get("y"), q.y);
    q.z = AsFloat(value->Get("z"), q.z);
    q.w = AsFloat(value->Get("w"), q.w);
    return q;
}

} // namespace

// --- maths ---------------------------------------------------------------------------------

Vector3f operator+(Vector3f a, Vector3f b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3f Scale(Vector3f a, Vector3f b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

Quaternionf operator*(Quaternionf a, Quaternionf b) {
    return {a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
            a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
}

Vector3f Rotate(Quaternionf q, Vector3f v) {
    // v' = v + 2w(u x v) + 2u x (u x v), with u the vector part of q.
    const Vector3f u{q.x, q.y, q.z};
    const auto cross = [](Vector3f a, Vector3f b) {
        return Vector3f{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    };
    const Vector3f t = cross(u, v);
    const Vector3f t2{2 * t.x, 2 * t.y, 2 * t.z};
    const Vector3f c = cross(u, t2);
    return {v.x + q.w * t2.x + c.x, v.y + q.w * t2.y + c.y, v.z + q.w * t2.z + c.z};
}

Vector3f ToEulerRadians(Quaternionf q) {
    // Rotation matrix entries of R = Ry * Rx * Rz (the engine's Z, X, Y order).
    const float m00 = 1 - 2 * (q.y * q.y + q.z * q.z);
    const float m02 = 2 * (q.x * q.z + q.w * q.y);
    const float m10 = 2 * (q.x * q.y + q.w * q.z);
    const float m11 = 1 - 2 * (q.x * q.x + q.z * q.z);
    const float m12 = 2 * (q.y * q.z - q.w * q.x);
    const float m20 = 2 * (q.x * q.z - q.w * q.y);
    const float m22 = 1 - 2 * (q.x * q.x + q.y * q.y);

    const float sinX = std::clamp(-m12, -1.0f, 1.0f);
    Vector3f euler;
    euler.x = std::asin(sinX);
    if (std::fabs(sinX) < 0.99999f) {
        euler.y = std::atan2(m02, m22);
        euler.z = std::atan2(m10, m11);
    } else {
        // Gimbal lock: the Y and Z rotations share an axis; put it all into Y.
        euler.y = std::atan2(-m20, m00);
        euler.z = 0;
    }
    return euler;
}

Vector3f MakePositiveDegrees(Vector3f euler) {
    constexpr float kNegativeFlip = -0.0001f * 57.29578f;
    constexpr float kPositiveFlip = 360.0f + kNegativeFlip;
    const auto wrap = [](float a) {
        if (a < kNegativeFlip)
            return a + 360.0f;
        if (a > kPositiveFlip)
            return a - 360.0f;
        return a;
    };
    return {wrap(euler.x), wrap(euler.y), wrap(euler.z)};
}

Vector3f EulerAnglesDegrees(Quaternionf q) {
    constexpr float kRadToDeg = 57.29578f;
    const Vector3f r = ToEulerRadians(q);
    return MakePositiveDegrees({r.x * kRadToDeg, r.y * kRadToDeg, r.z * kRadToDeg});
}

// --- parsing -------------------------------------------------------------------------------

TransformData ParseTransform(const Value& value) {
    TransformData t;
    t.localPosition = ReadVector3(value.Get("m_LocalPosition"), {});
    t.localRotation = ReadQuaternion(value.Get("m_LocalRotation"));
    t.localScale = ReadVector3(value.Get("m_LocalScale"), {1, 1, 1});
    if (auto father = AsObjectPointer(value.Get("m_Father")); father && father->pathId != 0) {
        t.parent = *father;
    }
    if (const Value* children = value.Get("m_Children"); children && children->AsArray()) {
        for (const auto& child : *children->AsArray()) {
            if (auto pointer = AsObjectPointer(child.get()); pointer && pointer->pathId != 0) {
                t.children.push_back(*pointer);
            }
        }
    }
    return t;
}

GameObjectData ParseGameObject(const Value& value, std::vector<ObjectPointer>& components) {
    GameObjectData go;
    if (const Value* name = value.Get("m_Name"); name && name->AsString())
        go.name = *name->AsString();
    if (const Value* active = value.Get("m_IsActive"))
        go.active = active->AsInt(1) != 0;
    if (const Value* layer = value.Get("m_Layer"))
        go.layer = static_cast<std::int32_t>(layer->AsInt());
    if (const Value* tag = value.Get("m_Tag"))
        go.tag = static_cast<std::uint32_t>(tag->AsInt());
    components.clear();
    if (const Value* list = value.Get("m_Component"); list && list->AsArray()) {
        for (const auto& entry : *list->AsArray()) {
            // Each entry is a pair whose `component` field points at the component.
            const Value* pointerValue = entry->Get("component");
            if (auto pointer = AsObjectPointer(pointerValue ? pointerValue : entry.get())) {
                components.push_back(*pointer);
            }
        }
    }
    return go;
}

const ComponentData* GameObjectData::FindScript(const std::string& className) const {
    for (const ComponentData& c : components) {
        if (c.script && c.script->className == className)
            return &c;
    }
    return nullptr;
}

std::vector<const ComponentData*> GameObjectData::Scripts() const {
    std::vector<const ComponentData*> scripts;
    for (const ComponentData& c : components) {
        if (c.classId == static_cast<std::int32_t>(ClassId::MonoBehaviour))
            scripts.push_back(&c);
    }
    return scripts;
}

const ComponentData* GameObjectData::FindClass(std::int32_t classId) const {
    for (const ComponentData& c : components) {
        if (c.classId == classId)
            return &c;
    }
    return nullptr;
}

Vector3f Prefab::WorldPosition(int index) const {
    Vector3f position;
    for (int i = index; i >= 0; i = objects[static_cast<std::size_t>(i)].parent) {
        const auto& transform = objects[static_cast<std::size_t>(i)].transform;
        if (!transform)
            continue;
        position = Rotate(transform->localRotation, Scale(transform->localScale, position)) +
                   transform->localPosition;
    }
    return position;
}

Quaternionf Prefab::WorldRotation(int index) const {
    Quaternionf rotation;
    for (int i = index; i >= 0; i = objects[static_cast<std::size_t>(i)].parent) {
        const auto& transform = objects[static_cast<std::size_t>(i)].transform;
        if (transform)
            rotation = transform->localRotation * rotation;
    }
    return rotation;
}

// --- reading -------------------------------------------------------------------------------

std::optional<GameObjectData> PrefabReader::ReadGameObject(const ObjectRef& gameObject) {
    if (!gameObject.object ||
        gameObject.object->classId != static_cast<std::int32_t>(ClassId::GameObject)) {
        return std::nullopt;
    }
    std::vector<ObjectPointer> pointers;
    GameObjectData go = ParseGameObject(*library_.Decode(gameObject), pointers);
    go.ref = gameObject;
    for (const ObjectPointer& pointer : pointers) {
        auto ref = library_.Resolve(gameObject.cab, pointer);
        if (!ref)
            continue;
        ComponentData component;
        component.ref = *ref;
        component.classId = ref->object->classId;
        try {
            component.value = library_.Decode(*ref);
        } catch (const ReadError&) {
            component.value = nullptr; // undecodable components are listed without data
        }
        if (component.classId == static_cast<std::int32_t>(ClassId::MonoBehaviour) &&
            component.value) {
            component.script = library_.ScriptOf(*ref);
        }
        if (!go.transform && IsTransformClass(component.classId) && component.value) {
            go.transform = ParseTransform(*component.value);
        }
        go.components.push_back(std::move(component));
    }
    return go;
}

std::optional<ObjectRef> PrefabReader::GameObjectOf(const ObjectRef& component) {
    const ValuePtr value = library_.Decode(component);
    const auto pointer = AsObjectPointer(value->Get("m_GameObject"));
    if (!pointer)
        return std::nullopt;
    return library_.Resolve(component.cab, *pointer);
}

std::optional<Prefab> PrefabReader::ReadHierarchy(const ObjectRef& gameObject) {
    auto root = ReadGameObject(gameObject);
    if (!root)
        return std::nullopt;
    Prefab prefab;
    prefab.objects.push_back(std::move(*root));
    std::set<std::pair<std::string, std::int64_t>> seen{
        {gameObject.cab, gameObject.object->pathId}};

    // Breadth-first over the transforms' child lists, so parents precede children.
    for (std::size_t index = 0; index < prefab.objects.size(); ++index) {
        if (!prefab.objects[index].transform)
            continue;
        const std::string cab = prefab.objects[index].ref.cab;
        const std::vector<ObjectPointer> children = prefab.objects[index].transform->children;
        for (const ObjectPointer& childPointer : children) {
            auto childTransform = library_.Resolve(cab, childPointer);
            if (!childTransform)
                continue;
            auto childObject = GameObjectOf(*childTransform);
            if (!childObject ||
                !seen.insert({childObject->cab, childObject->object->pathId}).second)
                continue;
            auto child = ReadGameObject(*childObject);
            if (!child)
                continue;
            child->parent = static_cast<int>(index);
            prefab.objects[index].children.push_back(static_cast<int>(prefab.objects.size()));
            prefab.objects.push_back(std::move(*child));
        }
    }
    return prefab;
}

} // namespace rr::assets
