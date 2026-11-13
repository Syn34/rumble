// prefab.h - reads object hierarchies (prefabs) out of the game's content files.
//
// A prefab is a tree of game objects. Each game object has a name, an active flag, a
// layer and a list of components; one of the components is its transform (local
// position, rotation and scale plus the parent and child transforms), the others are
// engine components (colliders, renderers...) or script components whose data is
// described by the file's embedded type descriptions. Pointers between these objects
// may cross into other files; the content library resolves them.
//
// Nothing here knows about any particular game: script components are returned with
// their script class name and their decoded data, for game-side readers to interpret.
#pragma once

#include "rr/assets/content_library.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rr::assets {

// The engine's vector and quaternion types as stored in content files (single precision).
struct Vector3f {
    float x = 0, y = 0, z = 0;
};

struct Quaternionf {
    float x = 0, y = 0, z = 0, w = 1;
};

Vector3f operator+(Vector3f a, Vector3f b);
Vector3f Scale(Vector3f a, Vector3f b); // component-wise product
Quaternionf operator*(Quaternionf a, Quaternionf b);
// Rotates `v` by `q`.
Vector3f Rotate(Quaternionf q, Vector3f v);

// The rotation as Euler angles in radians, in the engine's convention: applied about Z,
// then X, then Y.
Vector3f ToEulerRadians(Quaternionf q);
// Brings Euler angles in degrees into [0, 360), leaving values within a small tolerance
// below 0 (or above 360) as they are, the way the engine reports them.
Vector3f MakePositiveDegrees(Vector3f euler);
// Euler angles in degrees in [0, 360) (the engine's `eulerAngles`).
Vector3f EulerAnglesDegrees(Quaternionf q);

struct TransformData {
    Vector3f localPosition;
    Quaternionf localRotation;
    Vector3f localScale{1, 1, 1};
    std::optional<ObjectPointer> parent; // m_Father (absent for a root)
    std::vector<ObjectPointer> children; // m_Children, in order
};

// One component of a game object.
struct ComponentData {
    ObjectRef ref;
    std::int32_t classId = 0;
    ValuePtr value;                   // decoded data (null if it could not be decoded)
    std::optional<ScriptName> script; // for script components
};

struct GameObjectData {
    ObjectRef ref;
    std::string name;
    bool active = true;
    std::int32_t layer = 0;
    std::uint32_t tag = 0;
    std::vector<ComponentData> components; // in the game object's order
    std::optional<TransformData> transform;

    // Position of this object within a Prefab (-1 for none).
    int parent = -1;
    std::vector<int> children;

    // The first script component whose class name is `className`, or null.
    const ComponentData* FindScript(const std::string& className) const;
    // Every script component, in order.
    std::vector<const ComponentData*> Scripts() const;
    // The first component of an engine class, or null.
    const ComponentData* FindClass(std::int32_t classId) const;
};

// A game object and everything below it. objects[0] is the root; parents come before
// their children.
struct Prefab {
    std::vector<GameObjectData> objects;

    const GameObjectData& Root() const { return objects.front(); }
    // Position and rotation of objects[index] relative to the root's parent space (so
    // the root's own local transform is included), composing local transforms upward.
    Vector3f WorldPosition(int index) const;
    Quaternionf WorldRotation(int index) const;
};

// Interprets decoded objects (exposed for tests and for readers of other object kinds).
TransformData ParseTransform(const Value& value);
// Name, flags and the component pointers ({component pointer} list) of a game object.
GameObjectData ParseGameObject(const Value& value, std::vector<ObjectPointer>& components);

class PrefabReader {
  public:
    explicit PrefabReader(ContentLibrary& library) : library_(library) {}

    // One game object with its components decoded, without its hierarchy.
    std::optional<GameObjectData> ReadGameObject(const ObjectRef& gameObject);
    // The game object a component belongs to.
    std::optional<ObjectRef> GameObjectOf(const ObjectRef& component);
    // A game object and all of its descendants (following the transforms' child lists).
    std::optional<Prefab> ReadHierarchy(const ObjectRef& gameObject);

    ContentLibrary& Library() { return library_; }

  private:
    ContentLibrary& library_;
};

} // namespace rr::assets
