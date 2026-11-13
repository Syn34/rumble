#include "rr/scene/prefab_instancer.h"

#include "rr/scene/resources.h"

namespace rr::scene {

namespace {

float FloatOf(const assets::Value* v, float fallback = 0.0f) {
    return v ? static_cast<float>(v->AsDouble(fallback)) : fallback;
}

Vec3 Vec3Of(const assets::Value* v) {
    if (!v) return {};
    return {FloatOf(v->Get("x")), FloatOf(v->Get("y")), FloatOf(v->Get("z"))};
}

std::optional<assets::ObjectPointer> NonNullPointer(const assets::Value* v) {
    auto p = assets::AsObjectPointer(v);
    if (!p || p->pathId == 0) return std::nullopt;
    return p;
}

std::vector<assets::ObjectPointer> PointerList(const assets::Value* list) {
    std::vector<assets::ObjectPointer> out;
    if (!list || !list->AsArray()) return out;
    for (const assets::ValuePtr& entry : *list->AsArray()) {
        // Missing entries are kept (as null pointers) so indices stay aligned.
        out.push_back(assets::AsObjectPointer(entry.get()).value_or(assets::ObjectPointer{}));
    }
    return out;
}

ObjectKey KeyOf(const assets::ObjectRef& ref) { return {ref.cab, ref.object ? ref.object->pathId : 0}; }

bool IsClass(const assets::ComponentData& c, ComponentClass cls) { return c.classId == static_cast<std::int32_t>(cls); }

} // namespace

std::optional<ObjectKey> AssetResolver::Key(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    if (pointer.pathId == 0 || pointer.fileId != 0) return std::nullopt;
    return ObjectKey{fromCab, pointer.pathId};
}

RendererData ParseRenderer(const assets::Value& value) {
    RendererData r;
    if (const assets::Value* e = value.Get("m_Enabled")) r.enabled = e->AsInt(1) != 0;
    r.materials = PointerList(value.Get("m_Materials"));
    if (const assets::Value* info = value.Get("m_StaticBatchInfo")) {
        r.staticBatchFirstSubMesh = static_cast<std::uint32_t>(info->Get("firstSubMesh") ? info->Get("firstSubMesh")->AsInt() : 0);
        r.staticBatchSubMeshCount = static_cast<std::uint32_t>(info->Get("subMeshCount") ? info->Get("subMeshCount")->AsInt() : 0);
    }
    r.staticBatchRoot = NonNullPointer(value.Get("m_StaticBatchRoot"));
    return r;
}

SkinnedRendererData ParseSkinnedRenderer(const assets::Value& value) {
    SkinnedRendererData s;
    s.renderer = ParseRenderer(value);
    s.mesh = NonNullPointer(value.Get("m_Mesh"));
    s.bones = PointerList(value.Get("m_Bones"));
    s.rootBone = NonNullPointer(value.Get("m_RootBone"));
    if (const assets::Value* box = value.Get("m_AABB")) {
        const Vec3 center = Vec3Of(box->Get("m_Center"));
        const Vec3 extent = Vec3Of(box->Get("m_Extent"));
        if (extent.x >= 0 && extent.y >= 0 && extent.z >= 0 && (extent.x + extent.y + extent.z) > 0) {
            s.bounds = Aabb::FromCenterExtent(center, extent);
        }
    }
    if (const assets::Value* weights = value.Get("m_BlendShapeWeights"); weights && weights->AsArray()) {
        for (const assets::ValuePtr& w : *weights->AsArray()) s.blendShapeWeights.push_back(FloatOf(w.get()));
    }
    return s;
}

std::optional<assets::ObjectPointer> ParseMeshFilter(const assets::Value& value) {
    return NonNullPointer(value.Get("m_Mesh"));
}

LodGroupData ParseLodGroup(const assets::Value& value) {
    LodGroupData g;
    if (const assets::Value* e = value.Get("m_Enabled")) g.enabled = e->AsInt(1) != 0;
    g.localReferencePoint = Vec3Of(value.Get("m_LocalReferencePoint"));
    g.size = FloatOf(value.Get("m_Size"), 1.0f);
    if (const assets::Value* lods = value.Get("m_LODs"); lods && lods->AsArray()) {
        for (const assets::ValuePtr& lod : *lods->AsArray()) {
            LodGroupData::Level level;
            level.screenRelativeHeight = FloatOf(lod->Get("screenRelativeHeight"));
            if (const assets::Value* list = lod->Get("renderers"); list && list->AsArray()) {
                for (const assets::ValuePtr& entry : *list->AsArray()) {
                    if (auto p = NonNullPointer(entry->Get("renderer"))) level.renderers.push_back(*p);
                }
            }
            g.levels.push_back(std::move(level));
        }
    }
    return g;
}

InstantiateStats& InstantiateStats::operator+=(const InstantiateStats& o) {
    nodes += o.nodes;
    meshRenderers += o.meshRenderers;
    skinnedRenderers += o.skinnedRenderers;
    staticBatched += o.staticBatched;
    lodGroups += o.lodGroups;
    lodRenderersMissing += o.lodRenderersMissing;
    otherRenderers += o.otherRenderers;
    missingMeshes += o.missingMeshes;
    missingMaterials += o.missingMaterials;
    missingBones += o.missingBones;
    undecodedComponents += o.undecodedComponents;
    return *this;
}

int PrefabInstancer::Instantiate(const assets::Prefab& prefab, int parentNode) {
    if (prefab.objects.empty()) return -1;

    // 1. Nodes, and where each transform/game object ended up.
    std::vector<int> nodeOf(prefab.objects.size(), -1);
    std::map<ObjectKey, int> nodeByTransform;
    for (std::size_t i = 0; i < prefab.objects.size(); ++i) {
        const assets::GameObjectData& go = prefab.objects[i];
        Node node;
        node.name = go.name;
        node.active = go.active;
        node.layer = go.layer;
        node.parent = go.parent >= 0 ? nodeOf[static_cast<std::size_t>(go.parent)] : parentNode;
        if (go.transform) {
            const auto& t = *go.transform;
            node.localPosition = MirrorPosition({t.localPosition.x, t.localPosition.y, t.localPosition.z});
            node.localRotation =
                MirrorRotation({t.localRotation.x, t.localRotation.y, t.localRotation.z, t.localRotation.w});
            node.localScale = {t.localScale.x, t.localScale.y, t.localScale.z};
        }
        nodeOf[i] = scene_.AddNode(std::move(node));
        ++stats_.nodes;
        for (const assets::ComponentData& c : go.components) {
            if (c.classId == 4 || c.classId == 224) nodeByTransform[KeyOf(c.ref)] = nodeOf[i]; // Transform, RectTransform
        }
    }

    const auto boneNode = [&](const std::string& cab, const assets::ObjectPointer& p) {
        auto key = resolver_.Key(cab, p);
        if (!key) return -1;
        auto it = nodeByTransform.find(*key);
        return it != nodeByTransform.end() ? it->second : -1;
    };
    const auto loadMaterials = [&](const std::string& cab, const std::vector<assets::ObjectPointer>& pointers) {
        std::vector<MaterialId> out;
        for (const assets::ObjectPointer& p : pointers) {
            MaterialId id = p.pathId != 0 ? resolver_.Material(cab, p) : kNoResource;
            if (id == kNoResource) {
                ++stats_.missingMaterials;
                id = resources_.DefaultMaterial();
            }
            out.push_back(id);
        }
        return out;
    };

    // 2. Renderers.
    std::map<ObjectKey, int> instanceByRenderer;
    for (std::size_t i = 0; i < prefab.objects.size(); ++i) {
        const assets::GameObjectData& go = prefab.objects[i];
        const assets::ComponentData* filter = nullptr;
        for (const assets::ComponentData& c : go.components) {
            if (IsClass(c, ComponentClass::MeshFilter)) filter = &c;
        }
        for (const assets::ComponentData& c : go.components) {
            const bool meshRenderer = IsClass(c, ComponentClass::MeshRenderer);
            const bool skinned = IsClass(c, ComponentClass::SkinnedMeshRenderer);
            if (IsClass(c, ComponentClass::ParticleSystemRenderer) || IsClass(c, ComponentClass::LineRenderer) ||
                IsClass(c, ComponentClass::TrailRenderer) || IsClass(c, ComponentClass::SpriteRenderer)) {
                ++stats_.otherRenderers;
                continue;
            }
            if (!meshRenderer && !skinned) continue;
            if (!c.value) {
                ++stats_.undecodedComponents;
                continue;
            }
            RenderInstance inst;
            inst.node = nodeOf[i];
            const std::string& cab = c.ref.cab;
            if (meshRenderer) {
                ++stats_.meshRenderers;
                const RendererData r = ParseRenderer(*c.value);
                inst.kind = RendererKind::Mesh;
                inst.enabled = r.enabled;
                inst.materials = loadMaterials(cab, r.materials);
                std::optional<assets::ObjectPointer> meshPointer;
                if (filter && filter->value) meshPointer = ParseMeshFilter(*filter->value);
                if (meshPointer) inst.mesh = resolver_.Mesh(filter->ref.cab, *meshPointer);
                if (r.staticBatchSubMeshCount > 0) {
                    ++stats_.staticBatched;
                    inst.firstSubMesh = r.staticBatchFirstSubMesh;
                    inst.subMeshCount = r.staticBatchSubMeshCount;
                    if (r.staticBatchRoot) inst.staticBatchRoot = boneNode(cab, *r.staticBatchRoot);
                }
            } else {
                ++stats_.skinnedRenderers;
                const SkinnedRendererData s = ParseSkinnedRenderer(*c.value);
                inst.kind = RendererKind::Skinned;
                inst.enabled = s.renderer.enabled;
                inst.materials = loadMaterials(cab, s.renderer.materials);
                if (s.mesh) inst.mesh = resolver_.Mesh(cab, *s.mesh);
                for (const assets::ObjectPointer& b : s.bones) {
                    const int n = b.pathId != 0 ? boneNode(cab, b) : -1;
                    if (n < 0) ++stats_.missingBones;
                    inst.bones.push_back(n);
                }
                if (s.rootBone) inst.rootBone = boneNode(cab, *s.rootBone);
                if (s.bounds.Valid()) {
                    inst.skinBounds = Aabb::FromCenterExtent(MirrorPosition(s.bounds.Center()), s.bounds.Extent());
                }
            }
            if (inst.mesh == kNoResource) ++stats_.missingMeshes;
            instanceByRenderer[KeyOf(c.ref)] = scene_.AddInstance(std::move(inst));
        }
    }

    // 3. LOD groups.
    for (std::size_t i = 0; i < prefab.objects.size(); ++i) {
        for (const assets::ComponentData& c : prefab.objects[i].components) {
            if (!IsClass(c, ComponentClass::LODGroup) || !c.value) continue;
            const LodGroupData data = ParseLodGroup(*c.value);
            LodGroup group;
            group.node = nodeOf[i];
            group.enabled = data.enabled;
            group.localReferencePoint = MirrorPosition(data.localReferencePoint);
            group.size = data.size;
            const int groupIndex = static_cast<int>(scene_.LodGroups().size());
            for (std::size_t level = 0; level < data.levels.size(); ++level) {
                group.screenRelativeHeights.push_back(data.levels[level].screenRelativeHeight);
                std::vector<int> members;
                for (const assets::ObjectPointer& p : data.levels[level].renderers) {
                    auto key = resolver_.Key(c.ref.cab, p);
                    auto it = key ? instanceByRenderer.find(*key) : instanceByRenderer.end();
                    if (it == instanceByRenderer.end()) {
                        ++stats_.lodRenderersMissing;
                        continue;
                    }
                    RenderInstance& inst = scene_.Instances()[static_cast<std::size_t>(it->second)];
                    // A renderer listed in several levels belongs to the first.
                    if (inst.lodGroup < 0) {
                        inst.lodGroup = groupIndex;
                        inst.lodLevel = static_cast<int>(level);
                    }
                    members.push_back(it->second);
                }
                group.levels.push_back(std::move(members));
            }
            scene_.AddLodGroup(std::move(group));
            ++stats_.lodGroups;
        }
    }
    return nodeOf[0];
}

} // namespace rr::scene
