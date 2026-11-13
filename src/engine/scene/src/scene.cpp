#include "rr/scene/scene.h"

#include "rr/scene/resources.h"

#include <stdexcept>

namespace rr::scene {

int Scene::AddNode(Node node) {
    if (node.parent >= static_cast<int>(nodes_.size())) {
        throw std::invalid_argument("Scene::AddNode: parent must be added before its children");
    }
    nodes_.push_back(std::move(node));
    return static_cast<int>(nodes_.size() - 1);
}

int Scene::AddInstance(RenderInstance instance) {
    instances_.push_back(std::move(instance));
    return static_cast<int>(instances_.size() - 1);
}

int Scene::AddLodGroup(LodGroup group) {
    lodGroups_.push_back(std::move(group));
    return static_cast<int>(lodGroups_.size() - 1);
}

void Scene::UpdateTransforms() {
    for (Node& n : nodes_) {
        const Mat4 local = Trs(n.localPosition, n.localRotation, n.localScale);
        if (n.parent >= 0) {
            const Node& p = nodes_[static_cast<std::size_t>(n.parent)];
            n.world = p.world * local;
            n.activeInHierarchy = n.active && p.activeInHierarchy;
        } else {
            n.world = local;
            n.activeInHierarchy = n.active;
        }
    }
}

Mat4 Scene::InstanceMatrix(const RenderInstance& instance) const {
    if (instance.StaticBatched()) {
        return instance.staticBatchRoot >= 0 ? nodes_[static_cast<std::size_t>(instance.staticBatchRoot)].world
                                             : Mat4::Identity();
    }
    return nodes_[static_cast<std::size_t>(instance.node)].world;
}

bool Scene::Drawable(const RenderInstance& instance) const {
    return instance.enabled && instance.mesh != kNoResource && instance.node >= 0 &&
           nodes_[static_cast<std::size_t>(instance.node)].activeInHierarchy;
}

void Scene::UpdateBounds(const ResourceLibrary& resources) {
    for (RenderInstance& inst : instances_) {
        inst.worldBounds = {};
        if (inst.mesh == kNoResource || inst.node < 0) continue;
        const MeshData& mesh = resources.Mesh(inst.mesh);
        if (inst.kind == RendererKind::Skinned && inst.skinBounds.Valid()) {
            const int space = inst.rootBone >= 0 ? inst.rootBone : inst.node;
            inst.worldBounds = TransformAabb(nodes_[static_cast<std::size_t>(space)].world, inst.skinBounds);
            continue;
        }
        Aabb local;
        if (inst.StaticBatched()) {
            for (std::uint32_t s = inst.firstSubMesh; s < inst.firstSubMesh + inst.subMeshCount; ++s) {
                if (s < mesh.subMeshes.size()) local.Add(mesh.subMeshes[s].bounds);
            }
        } else {
            local = mesh.bounds;
        }
        inst.worldBounds = TransformAabb(InstanceMatrix(inst), local);
    }
}

Aabb Scene::Bounds() const {
    Aabb box;
    for (const RenderInstance& inst : instances_) {
        if (Drawable(inst)) box.Add(inst.worldBounds);
    }
    return box;
}

std::string Scene::Path(int node) const {
    std::string path;
    for (int i = node; i >= 0; i = nodes_[static_cast<std::size_t>(i)].parent) {
        path = nodes_[static_cast<std::size_t>(i)].name + (path.empty() ? "" : "/" + path);
    }
    return path;
}

} // namespace rr::scene
