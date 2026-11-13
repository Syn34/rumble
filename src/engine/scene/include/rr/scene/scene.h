// scene.h - engine-side scene: a transform hierarchy with renderable instances.
//
// A Scene is what the game's object hierarchies (prefabs, scene files) become
// once instantiated: nodes with local transforms (already converted to the
// renderer's right-handed convention), mesh/skinned-mesh renderer instances
// attached to nodes, and LOD groups selecting between instances. Everything is
// stored in flat arrays; nodes always come after their parent so world
// transforms are computed in one forward pass.
#pragma once

#include "rr/scene/ids.h"
#include "rr/scene/math.h"

#include <cstdint>
#include <string>
#include <vector>

namespace rr::scene {

class ResourceLibrary;

struct Node {
    std::string name;
    int parent = -1;
    Vec3 localPosition;
    Quat localRotation;
    Vec3 localScale{1, 1, 1};
    bool active = true; // the object's own flag
    int layer = 0;

    // Derived by Scene::UpdateTransforms.
    Mat4 world = Mat4::Identity();
    bool activeInHierarchy = true;
};

enum class RendererKind { Mesh, Skinned };

struct RenderInstance {
    int node = -1;
    RendererKind kind = RendererKind::Mesh;
    MeshId mesh = kNoResource;
    std::vector<MaterialId> materials; // one per submesh; the last repeats for extra submeshes
    bool enabled = true;

    // Static batching: the mesh is a combined mesh already in world space (or in
    // the space of staticBatchRoot) and only these submeshes belong to this
    // renderer. subMeshCount 0 = not batched, draw every submesh.
    std::uint32_t firstSubMesh = 0;
    std::uint32_t subMeshCount = 0;
    int staticBatchRoot = -1;

    // Skinned renderers: bone nodes in the mesh's bind-pose order (-1 = missing),
    // the root bone and the bounds relative to it (renderer-local when absent).
    std::vector<int> bones;
    int rootBone = -1;
    Aabb skinBounds;

    int lodGroup = -1; // LOD group this instance belongs to, if any
    int lodLevel = -1;

    // Derived by Scene::UpdateBounds.
    Aabb worldBounds;

    bool StaticBatched() const { return subMeshCount > 0; }
};

struct LodGroup {
    int node = -1;
    bool enabled = true;
    Vec3 localReferencePoint;
    float size = 1.0f; // object-space size the screen heights refer to
    std::vector<float> screenRelativeHeights; // per level, decreasing
    std::vector<std::vector<int>> levels;     // instance indices per level
};

class Scene {
public:
    // Adds a node; its parent must already exist (or be -1). Returns its index.
    int AddNode(Node node);
    int AddInstance(RenderInstance instance);
    int AddLodGroup(LodGroup group);

    std::vector<Node>& Nodes() { return nodes_; }
    const std::vector<Node>& Nodes() const { return nodes_; }
    std::vector<RenderInstance>& Instances() { return instances_; }
    const std::vector<RenderInstance>& Instances() const { return instances_; }
    std::vector<LodGroup>& LodGroups() { return lodGroups_; }
    const std::vector<LodGroup>& LodGroups() const { return lodGroups_; }

    // Recomputes world matrices and active flags from the local transforms.
    void UpdateTransforms();
    // Recomputes each instance's world bounds (after UpdateTransforms).
    void UpdateBounds(const ResourceLibrary& resources);
    // Union of the enabled, active instances' world bounds.
    Aabb Bounds() const;

    // The matrix an instance's vertices are transformed with: its node's world
    // matrix, the static batch root's for batched instances (identity without one).
    Mat4 InstanceMatrix(const RenderInstance& instance) const;
    // Whether an instance would be drawn at all (enabled, node active, has a mesh).
    bool Drawable(const RenderInstance& instance) const;
    // Node path from the root, "a/b/c" (for tools and diagnostics).
    std::string Path(int node) const;

private:
    std::vector<Node> nodes_;
    std::vector<RenderInstance> instances_;
    std::vector<LodGroup> lodGroups_;
};

} // namespace rr::scene
