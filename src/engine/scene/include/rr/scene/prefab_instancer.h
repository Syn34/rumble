// prefab_instancer.h - turns a game object hierarchy into scene nodes and
// renderable instances.
//
// Input is an assets::Prefab (a prefab's or a scene's object tree with decoded
// components). For each game object a node is added (transform converted to the
// right-handed convention), and for its components:
//   MeshFilter + MeshRenderer  -> a mesh instance (static batching info kept)
//   SkinnedMeshRenderer        -> a skinned instance with its bones bound to nodes
//   LODGroup                   -> a LOD group over the instances it lists
// Other renderers (particles, lines, trails, sprites) are counted and skipped.
// Meshes and materials are loaded through an AssetResolver, so the same code
// runs on the game's files (ContentResolver) and on data built in tests.
#pragma once

#include "rr/assets/prefab.h"
#include "rr/scene/scene.h"

#include <compare>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace rr::scene {

// Engine class IDs of the components handled here.
enum class ComponentClass : std::int32_t {
    MeshRenderer = 23,
    MeshFilter = 33,
    TrailRenderer = 96,
    LineRenderer = 120,
    SkinnedMeshRenderer = 137,
    ParticleSystemRenderer = 199,
    LODGroup = 205,
    SpriteRenderer = 212,
};

// Identity of an object across files.
struct ObjectKey {
    std::string cab;
    std::int64_t pathId = 0;
    auto operator<=>(const ObjectKey&) const = default;
};

class AssetResolver {
public:
    virtual ~AssetResolver() = default;
    // The object `pointer` refers to, from an object stored in file `fromCab`.
    // The default handles pointers within the same file only.
    virtual std::optional<ObjectKey> Key(const std::string& fromCab, const assets::ObjectPointer& pointer);
    // Loads (or finds already loaded) resources; kNoResource if unavailable.
    virtual MeshId Mesh(const std::string& fromCab, const assets::ObjectPointer& pointer) = 0;
    virtual MaterialId Material(const std::string& fromCab, const assets::ObjectPointer& pointer) = 0;
};

// --- component data, parsed from decoded values (exposed for tests) ---------------

struct RendererData {
    bool enabled = true;
    std::vector<assets::ObjectPointer> materials;
    std::uint32_t staticBatchFirstSubMesh = 0;
    std::uint32_t staticBatchSubMeshCount = 0;
    std::optional<assets::ObjectPointer> staticBatchRoot;
};

struct SkinnedRendererData {
    RendererData renderer;
    std::optional<assets::ObjectPointer> mesh;
    std::vector<assets::ObjectPointer> bones;
    std::optional<assets::ObjectPointer> rootBone;
    Aabb bounds; // in the game's convention (not mirrored); invalid if absent
    std::vector<float> blendShapeWeights;
};

struct LodGroupData {
    bool enabled = true;
    Vec3 localReferencePoint; // game convention
    float size = 1.0f;
    struct Level {
        float screenRelativeHeight = 0.0f;
        std::vector<assets::ObjectPointer> renderers;
    };
    std::vector<Level> levels;
};

RendererData ParseRenderer(const assets::Value& value);
SkinnedRendererData ParseSkinnedRenderer(const assets::Value& value);
std::optional<assets::ObjectPointer> ParseMeshFilter(const assets::Value& value);
LodGroupData ParseLodGroup(const assets::Value& value);

struct InstantiateStats {
    std::size_t nodes = 0;
    std::size_t meshRenderers = 0;
    std::size_t skinnedRenderers = 0;
    std::size_t staticBatched = 0;
    std::size_t lodGroups = 0;
    std::size_t lodRenderersMissing = 0; // LOD entries pointing at renderers outside the hierarchy
    std::size_t otherRenderers = 0;      // particles, lines, trails, sprites (not drawn yet)
    std::size_t missingMeshes = 0;       // renderer without a loadable mesh
    std::size_t missingMaterials = 0;    // material slots that couldn't be loaded
    std::size_t missingBones = 0;        // bones outside the hierarchy
    std::size_t undecodedComponents = 0;

    InstantiateStats& operator+=(const InstantiateStats& o);
};

class PrefabInstancer {
public:
    PrefabInstancer(Scene& scene, AssetResolver& resolver, ResourceLibrary& resources)
        : scene_(scene), resolver_(resolver), resources_(resources) {}

    // Adds the hierarchy under `parentNode` (-1: at the scene root). Returns the
    // node of prefab.objects[0]. Call Scene::UpdateTransforms/UpdateBounds after.
    int Instantiate(const assets::Prefab& prefab, int parentNode = -1);

    const InstantiateStats& Stats() const { return stats_; }

private:
    Scene& scene_;
    AssetResolver& resolver_;
    ResourceLibrary& resources_;
    InstantiateStats stats_;
};

} // namespace rr::scene
