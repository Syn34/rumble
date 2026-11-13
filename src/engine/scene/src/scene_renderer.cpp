#include "rr/scene/scene_renderer.h"

#include "rr/assets/texture_format.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <optional>

namespace rr::scene {

namespace {

constexpr int kLastOpaqueQueue = 2500;

// Matches DrawConstants in src/render/shaders/scene.vert / scene.frag.
struct SceneDrawConstants {
    float transform[16];
    float lightDir[4];
    float color[4];
    float uvTransform[4];
    float params[4]; // x: alpha cut-off, y: normal strength, z: vertex colour weight
};
static_assert(sizeof(SceneDrawConstants) <= render::kMaxDrawConstantsSize);
static_assert(sizeof(MeshVertex) == 52 && sizeof(SkinVertex) == 32);
static_assert(kMaxBones * sizeof(Mat4) <= render::kMaxDrawUniformsSize);

const char* ProgramName(ShadingModel shading, bool skinned) {
    switch (shading) {
    case ShadingModel::Unlit: return skinned ? "scene_unlit_skinned" : "scene_unlit";
    case ShadingModel::Lit: return skinned ? "scene_lit_skinned" : "scene_lit";
    case ShadingModel::LitNormalMap: return skinned ? "scene_lit_normal_skinned" : "scene_lit_normal";
    }
    return "scene_lit";
}

render::BlendMode BlendFor(SurfaceType surface) {
    switch (surface) {
    case SurfaceType::Opaque:
    case SurfaceType::AlphaTested: return render::BlendMode::Opaque;
    case SurfaceType::Transparent: return render::BlendMode::AlphaBlend;
    case SurfaceType::Additive: return render::BlendMode::Additive;
    case SurfaceType::Premultiplied: return render::BlendMode::Premultiplied;
    }
    return render::BlendMode::Opaque;
}

const MaterialDesc& FallbackMaterial() {
    static const MaterialDesc material = [] {
        MaterialDesc m;
        m.name = "(none)";
        m.color = {0.35f, 0.35f, 0.35f, 1.0f};
        return m;
    }();
    return material;
}

bool InLevel(const LodGroup& group, int level, int instance) {
    if (level < 0 || level >= static_cast<int>(group.levels.size())) return false;
    const auto& members = group.levels[static_cast<std::size_t>(level)];
    return std::find(members.begin(), members.end(), instance) != members.end();
}

std::optional<render::TextureFormat> NativeFormat(std::int32_t assetFormat, bool srgb) {
    using AF = assets::TextureFormat;
    using RF = render::TextureFormat;
    switch (static_cast<AF>(assetFormat)) {
    case AF::ASTC_4x4: return srgb ? RF::ASTC_4x4_sRGB : RF::ASTC_4x4;
    case AF::ASTC_5x5: return srgb ? RF::ASTC_5x5_sRGB : RF::ASTC_5x5;
    case AF::ASTC_6x6: return srgb ? RF::ASTC_6x6_sRGB : RF::ASTC_6x6;
    case AF::ASTC_8x8: return srgb ? RF::ASTC_8x8_sRGB : RF::ASTC_8x8;
    case AF::ASTC_10x10: return srgb ? RF::ASTC_10x10_sRGB : RF::ASTC_10x10;
    case AF::ASTC_12x12: return srgb ? RF::ASTC_12x12_sRGB : RF::ASTC_12x12;
    case AF::RGBA32: return srgb ? RF::RGBA8_sRGB : RF::RGBA8;
    default: return std::nullopt;
    }
}

} // namespace

// --- draw lists ---------------------------------------------------------------------

std::vector<int> SelectLods(const Scene& scene, const Mat4& view, const Mat4& projection,
                            const DrawListOptions& options) {
    const Vec3 eye = TransformPoint(Inverse(view), {0, 0, 0});
    const float focal = projection.At(1, 1); // 1 / tan(fovY / 2)
    std::vector<int> selected;
    selected.reserve(scene.LodGroups().size());
    for (const LodGroup& group : scene.LodGroups()) {
        const int levels = static_cast<int>(group.levels.size());
        if (levels == 0) {
            selected.push_back(-1);
            continue;
        }
        if (options.forceLod >= 0) {
            selected.push_back(std::min(options.forceLod, levels - 1));
            continue;
        }
        const Mat4& world = scene.Nodes()[static_cast<std::size_t>(group.node)].world;
        const Vec3 reference = TransformPoint(world, group.localReferencePoint);
        const float distance = Length(reference - eye);
        if (distance < 1e-4f) {
            selected.push_back(0);
            continue;
        }
        // Fraction of the screen height the group's size covers.
        const float height = group.size * MaxAxisScale(world) * focal / (2.0f * distance) * options.lodBias;
        int level = -1;
        for (int i = 0; i < levels; ++i) {
            if (height >= group.screenRelativeHeights[static_cast<std::size_t>(i)]) {
                level = i;
                break;
            }
        }
        selected.push_back(level);
    }
    return selected;
}

std::vector<DrawItem> BuildDrawList(const Scene& scene, const ResourceLibrary& resources, const Mat4& view,
                                    const Mat4& projection, const DrawListOptions& options, DrawListStats* outStats) {
    DrawListStats stats;
    std::vector<DrawItem> items;
    const Frustum frustum = Frustum::FromViewProjection(projection * view);
    const std::vector<int> lods = SelectLods(scene, view, projection, options);

    const auto& instances = scene.Instances();
    for (std::size_t i = 0; i < instances.size(); ++i) {
        const RenderInstance& inst = instances[i];
        if (!scene.Drawable(inst)) continue;
        ++stats.instances;
        if (inst.lodGroup >= 0) {
            const LodGroup& group = scene.LodGroups()[static_cast<std::size_t>(inst.lodGroup)];
            if (group.enabled && !InLevel(group, lods[static_cast<std::size_t>(inst.lodGroup)], static_cast<int>(i))) {
                ++stats.lodRejected;
                continue;
            }
        }
        if (options.frustumCull && inst.worldBounds.Valid() && !frustum.Intersects(inst.worldBounds)) {
            ++stats.culled;
            continue;
        }
        ++stats.visible;

        const MeshData& mesh = resources.Mesh(inst.mesh);
        const Vec3 center = inst.worldBounds.Valid() ? inst.worldBounds.Center() : scene.InstanceMatrix(inst).Translation();
        const float depth = -TransformPoint(view, center).z;

        const std::uint32_t base = inst.StaticBatched() ? inst.firstSubMesh : 0;
        const std::size_t subMeshes =
            inst.StaticBatched() ? std::min<std::size_t>(inst.subMeshCount, mesh.subMeshes.size() > base ? mesh.subMeshes.size() - base : 0)
                                 : mesh.subMeshes.size();
        if (subMeshes == 0) continue;
        // Materials pair with submeshes; extra materials draw the last submesh again
        // (the engine's multi-material behaviour), extra submeshes go undrawn.
        const std::size_t slots = inst.materials.empty() ? subMeshes : inst.materials.size();
        for (std::size_t k = 0; k < slots; ++k) {
            const std::uint32_t sub = base + static_cast<std::uint32_t>(std::min(k, subMeshes - 1));
            if (mesh.subMeshes[sub].indexCount == 0) continue;
            const MaterialId materialId = inst.materials.empty() ? kNoResource : inst.materials[k];
            const MaterialDesc& material = materialId != kNoResource ? resources.Material(materialId) : FallbackMaterial();
            if (!material.visible) {
                ++stats.hiddenMaterials;
                continue;
            }
            DrawItem item;
            item.instance = static_cast<int>(i);
            item.subMesh = sub;
            item.material = materialId;
            item.depth = depth;
            item.transparent = material.Transparent();
            item.renderQueue = material.renderQueue;
            (item.transparent ? stats.transparentItems : stats.opaqueItems)++;
            items.push_back(item);
        }
    }

    // The engine's order: by render queue; opaque queues front to back (less
    // overdraw), transparent queues back to front (correct blending).
    std::stable_sort(items.begin(), items.end(), [](const DrawItem& a, const DrawItem& b) {
        if (a.renderQueue != b.renderQueue) return a.renderQueue < b.renderQueue;
        if (a.renderQueue > kLastOpaqueQueue) return a.depth > b.depth;
        return a.depth < b.depth;
    });
    if (outStats) *outStats = stats;
    return items;
}

// --- textures -----------------------------------------------------------------------

render::TextureHandle UploadTexture(render::RenderDevice& device, const assets::Texture2DAsset& t, bool forceSoftware,
                                    bool* native) {
    if (t.width <= 0 || t.height <= 0 || t.mipCount <= 0) return {};
    render::TextureDesc desc;
    desc.width = t.width;
    desc.height = t.height;
    desc.mipCount = static_cast<std::uint32_t>(t.mipCount);
    const auto nativeFormat = NativeFormat(t.format, t.srgb);
    if (nativeFormat && !forceSoftware && device.SupportsTextureFormat(*nativeFormat)) {
        desc.format = *nativeFormat;
        std::vector<render::TextureLevelData> levels;
        for (int l = 0; l < t.mipCount; ++l) levels.push_back({t.Level(l).data(), t.Level(l).size()});
        if (native) *native = true;
        return device.CreateTexture(desc, levels);
    }
    desc.format = t.srgb ? render::TextureFormat::RGBA8_sRGB : render::TextureFormat::RGBA8;
    std::vector<std::vector<std::uint8_t>> decoded;
    for (int l = 0; l < t.mipCount; ++l) {
        decoded.push_back(t.DecodeLevel(l));
        if (decoded.back().empty()) return {};
    }
    std::vector<render::TextureLevelData> levels;
    for (const auto& d : decoded) levels.push_back({d.data(), d.size()});
    if (native) *native = false;
    return device.CreateTexture(desc, levels);
}

render::SamplerDesc SamplerFor(const assets::Texture2DAsset& t) {
    render::SamplerDesc s;
    s.filter = t.filterMode == 0 ? render::FilterMode::Nearest : render::FilterMode::Linear;
    s.mipFilter = t.filterMode == 2 ? render::FilterMode::Linear : render::FilterMode::Nearest;
    s.useMips = t.mipCount > 1;
    const auto wrap = [](int w) {
        return w == 1 ? render::WrapMode::ClampToEdge
                      : w >= 2 ? render::WrapMode::MirroredRepeat : render::WrapMode::Repeat;
    };
    s.wrapU = wrap(t.wrapU);
    s.wrapV = wrap(t.wrapV);
    return s;
}

// --- renderer -----------------------------------------------------------------------

SceneRenderer::~SceneRenderer() { Release(); }

void SceneRenderer::Release() {
    for (GpuMesh& m : meshes_) {
        if (m.vertices) device_.DestroyBuffer(m.vertices);
        if (m.indices) device_.DestroyBuffer(m.indices);
    }
    for (GpuTexture& t : textures_) {
        if (t.handle) device_.DestroyTexture(t.handle);
    }
    for (auto& [key, pipeline] : pipelines_) {
        if (pipeline) device_.DestroyPipeline(pipeline);
    }
    meshes_.clear();
    textures_.clear();
    pipelines_.clear();
    gpuStats_ = {};
}

const SceneRenderer::GpuMesh& SceneRenderer::Mesh(const ResourceLibrary& resources, MeshId id) {
    if (meshes_.size() <= id) meshes_.resize(static_cast<std::size_t>(id) + 1);
    GpuMesh& gpu = meshes_[id];
    if (gpu.vertices || gpu.failed) return gpu;
    const MeshData& mesh = resources.Mesh(id);
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        gpu.failed = true;
        ++gpuStats_.meshFailures;
        return gpu;
    }
    gpu.skinned = mesh.HasSkin();
    gpu.stride = static_cast<std::uint32_t>(sizeof(MeshVertex) + (gpu.skinned ? sizeof(SkinVertex) : 0));
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(gpu.stride) * mesh.vertices.size());
    for (std::size_t v = 0; v < mesh.vertices.size(); ++v) {
        std::uint8_t* out = bytes.data() + v * gpu.stride;
        std::memcpy(out, &mesh.vertices[v], sizeof(MeshVertex));
        if (gpu.skinned) std::memcpy(out + sizeof(MeshVertex), &mesh.skin[v], sizeof(SkinVertex));
    }
    gpu.vertices = device_.CreateBuffer(render::BufferUsage::Vertex, bytes.data(), bytes.size());
    if (mesh.vertices.size() <= 0xFFFF) {
        std::vector<std::uint16_t> narrow(mesh.indices.begin(), mesh.indices.end());
        gpu.indices = device_.CreateBuffer(render::BufferUsage::Index, narrow.data(), narrow.size() * 2);
        gpu.indexType = render::IndexType::UInt16;
    } else {
        gpu.indices = device_.CreateBuffer(render::BufferUsage::Index, mesh.indices.data(), mesh.indices.size() * 4);
        gpu.indexType = render::IndexType::UInt32;
    }
    if (!gpu.vertices || !gpu.indices) {
        gpu.failed = true;
        ++gpuStats_.meshFailures;
    } else {
        ++gpuStats_.meshes;
    }
    return gpu;
}

const SceneRenderer::GpuTexture& SceneRenderer::Texture(const ResourceLibrary& resources, TextureId id) {
    if (textures_.size() <= id) textures_.resize(static_cast<std::size_t>(id) + 1);
    GpuTexture& gpu = textures_[id];
    if (gpu.attempted) return gpu;
    gpu.attempted = true;
    const assets::Texture2DAsset& texture = resources.Texture(id);
    bool native = false;
    gpu.handle = UploadTexture(device_, texture, forceSoftwareTextures, &native);
    gpu.sampler = SamplerFor(texture);
    if (!gpu.handle) {
        ++gpuStats_.textureFailures;
    } else {
        ++gpuStats_.textures;
        ++(native ? gpuStats_.texturesNative : gpuStats_.texturesDecoded);
    }
    return gpu;
}

render::PipelineHandle SceneRenderer::Pipeline(const PipelineKey& key) {
    if (auto it = pipelines_.find(key); it != pipelines_.end()) return it->second;
    render::PipelineDesc desc;
    desc.shaderProgram = ProgramName(key.shading, key.skinned);
    desc.vertexLayout.stride = key.stride;
    desc.vertexLayout.attributes = {
        {0, render::VertexFormat::Float3, offsetof(MeshVertex, position)},
        {1, render::VertexFormat::Float3, offsetof(MeshVertex, normal)},
        {2, render::VertexFormat::Float4, offsetof(MeshVertex, tangent)},
        {3, render::VertexFormat::Float2, offsetof(MeshVertex, uv)},
        {4, render::VertexFormat::UByte4Norm, offsetof(MeshVertex, color)},
    };
    if (key.skinned) {
        const auto skin = static_cast<std::uint32_t>(sizeof(MeshVertex));
        desc.vertexLayout.attributes.push_back(
            {5, render::VertexFormat::Float4, skin + static_cast<std::uint32_t>(offsetof(SkinVertex, boneIndices))});
        desc.vertexLayout.attributes.push_back(
            {6, render::VertexFormat::Float4, skin + static_cast<std::uint32_t>(offsetof(SkinVertex, boneWeights))});
    }
    desc.cull = key.cull;
    desc.blend = BlendFor(key.surface);
    desc.depthWrite = key.depthWrite;
    const render::PipelineHandle handle = device_.CreatePipeline(desc);
    ++(handle ? gpuStats_.pipelines : gpuStats_.pipelineFailures);
    pipelines_[key] = handle;
    return handle;
}

std::vector<Mat4> SceneRenderer::BoneMatrices(const Scene& scene, const RenderInstance& instance, const MeshData& mesh) {
    const std::size_t count = std::min(mesh.bindPoses.size(), kMaxBones);
    std::vector<Mat4> bones(count);
    const Mat4& rendererWorld = scene.Nodes()[static_cast<std::size_t>(instance.node)].world;
    for (std::size_t b = 0; b < count; ++b) {
        const int node = b < instance.bones.size() ? instance.bones[b] : -1;
        // A bone at its bind pose gives the renderer's own transform; missing bones
        // stay there.
        bones[b] = node >= 0 ? scene.Nodes()[static_cast<std::size_t>(node)].world * mesh.bindPoses[b] : rendererWorld;
    }
    return bones;
}

bool SceneRenderer::RenderFrame(const Scene& scene, const ResourceLibrary& resources, const Camera& camera,
                                const render::ClearValues& clear, FrameStats* stats, bool capture,
                                const LightSettings& light, const DrawListOptions& options) {
    if (!device_.BeginFrame()) return false;
    device_.BeginRenderPass(clear);
    const FrameStats frame = Draw(scene, resources, camera, light, options);
    device_.EndRenderPass();
    if (capture) device_.RequestCapture();
    device_.EndFrame();
    if (stats) *stats = frame;
    return true;
}

void SceneRenderer::Prepare(const ResourceLibrary& resources) {
    for (MeshId m = 0; m < resources.MeshCount(); ++m) Mesh(resources, m);
    for (TextureId t = 0; t < resources.TextureCount(); ++t) Texture(resources, t);
}

FrameStats SceneRenderer::Draw(const Scene& scene, const ResourceLibrary& resources, const Camera& camera,
                               const LightSettings& light, const DrawListOptions& options) {
    FrameStats stats;
    // Size the caches up front so references into them stay valid below.
    if (meshes_.size() < resources.MeshCount()) meshes_.resize(resources.MeshCount());
    if (textures_.size() < resources.TextureCount()) textures_.resize(resources.TextureCount());
    int width = 0, height = 0;
    device_.GetRenderSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    const Mat4 projection = camera.Projection(aspect);
    const Mat4 viewProjection = projection * camera.view;
    const std::vector<DrawItem> items = BuildDrawList(scene, resources, camera.view, projection, options, &stats.list);

    render::PipelineHandle boundPipeline;
    MeshId boundMesh = kNoResource;
    int skinnedInstance = -1;
    bool texturesBound = false;
    render::TextureHandle boundAlbedo, boundNormal;
    render::SamplerDesc boundAlbedoSampler, boundNormalSampler;
    for (const DrawItem& item : items) {
        const RenderInstance& inst = scene.Instances()[static_cast<std::size_t>(item.instance)];
        const MeshData& mesh = resources.Mesh(inst.mesh);
        const GpuMesh& gpu = Mesh(resources, inst.mesh);
        if (gpu.failed) continue;
        const MaterialDesc& material = item.material != kNoResource ? resources.Material(item.material) : FallbackMaterial();
        const bool skinned = inst.kind == RendererKind::Skinned && gpu.skinned && !mesh.bindPoses.empty();
        if (skinned && mesh.bindPoses.size() > kMaxBones && skinnedInstance != item.instance) ++gpuStats_.bonesClamped;

        // Textures (a missing normal map falls back to plain lighting).
        const GpuTexture* albedo = material.albedo != kNoResource ? &Texture(resources, material.albedo) : nullptr;
        const GpuTexture* normal = material.normal != kNoResource ? &Texture(resources, material.normal) : nullptr;
        ShadingModel shading = material.shading;
        if (shading == ShadingModel::LitNormalMap && (!normal || !normal->handle)) shading = ShadingModel::Lit;

        const PipelineKey key{shading, material.surface, material.cull, material.depthWrite, skinned, gpu.stride};
        const render::PipelineHandle pipeline = Pipeline(key);
        if (!pipeline) continue;
        if (pipeline.id != boundPipeline.id) {
            device_.BindPipeline(pipeline);
            boundPipeline = pipeline;
            boundMesh = kNoResource; // vertex layouts are tied to the pipeline
            ++stats.pipelineChanges;
        }
        if (boundMesh != inst.mesh) {
            device_.BindVertexBuffer(gpu.vertices);
            device_.BindIndexBuffer(gpu.indices, gpu.indexType);
            boundMesh = inst.mesh;
        }
        // Rebinding unchanged textures would cost a descriptor set per draw on Vulkan.
        const render::TextureHandle albedoHandle = albedo ? albedo->handle : render::TextureHandle{};
        const render::SamplerDesc albedoSampler = albedo ? albedo->sampler : render::SamplerDesc{};
        if (!texturesBound || albedoHandle.id != boundAlbedo.id || !(albedoSampler == boundAlbedoSampler)) {
            device_.BindTexture(0, albedoHandle, albedoSampler);
            boundAlbedo = albedoHandle;
            boundAlbedoSampler = albedoSampler;
        }
        if (shading == ShadingModel::LitNormalMap &&
            (!texturesBound || normal->handle.id != boundNormal.id || !(normal->sampler == boundNormalSampler))) {
            device_.BindTexture(1, normal->handle, normal->sampler);
            boundNormal = normal->handle;
            boundNormalSampler = normal->sampler;
        }
        texturesBound = true;

        SceneDrawConstants c{};
        Vec3 lightDir = light.direction;
        if (skinned) {
            std::copy(viewProjection.m.begin(), viewProjection.m.end(), c.transform);
            if (skinnedInstance != item.instance) {
                const std::vector<Mat4> bones = BoneMatrices(scene, inst, mesh);
                device_.SetDrawUniforms(bones.data(), bones.size() * sizeof(Mat4));
                skinnedInstance = item.instance;
            }
            ++stats.skinnedDraws;
        } else {
            const Mat4 world = scene.InstanceMatrix(inst);
            const Mat4 transform = viewProjection * world;
            std::copy(transform.m.begin(), transform.m.end(), c.transform);
            // Lighting happens in object space: bring the light direction there.
            lightDir = Normalize(TransformVector(Inverse(world), light.direction));
        }
        c.lightDir[0] = lightDir.x;
        c.lightDir[1] = lightDir.y;
        c.lightDir[2] = lightDir.z;
        c.lightDir[3] = light.ambient;
        std::copy(material.color.begin(), material.color.end(), c.color);
        std::copy(material.uvTransform.begin(), material.uvTransform.end(), c.uvTransform);
        c.params[0] = material.surface == SurfaceType::AlphaTested ? std::max(material.alphaCutoff, 1e-3f) : 0.0f;
        c.params[1] = material.normalScale;
        c.params[2] = material.vertexColor ? 1.0f : 0.0f;
        device_.SetDrawConstants(&c, sizeof(c));

        const SubMeshRange& range = mesh.subMeshes[item.subMesh];
        device_.DrawIndexed(range.indexCount, range.firstIndex, 0);
        ++stats.drawCalls;
        stats.triangles += range.indexCount / 3;
    }
    return stats;
}

} // namespace rr::scene
