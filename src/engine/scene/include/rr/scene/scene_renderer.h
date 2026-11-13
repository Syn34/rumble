// scene_renderer.h - draws a Scene with either render backend.
//
// Each frame: LOD selection, frustum culling against the instances' world bounds,
// one draw item per (instance, submesh), opaque items sorted by render queue then
// front to back, transparent items by queue then back to front; then the items are
// drawn with the scene_* shader programs. Skinned instances are skinned on the GPU
// from bone matrices (bone node world transform * bind pose) uploaded through the
// renderer's per-draw uniform block.
//
// BuildDrawList is independent of the GPU (tests and tools use it directly);
// SceneRenderer adds GPU resource management and drawing.
#pragma once

#include "rr/render/render_device.h"
#include "rr/scene/math.h"
#include "rr/scene/resources.h"
#include "rr/scene/scene.h"

#include <cstdint>
#include <map>
#include <vector>

namespace rr::scene {

struct Camera {
    Mat4 view = Mat4::Identity();
    float fovY = 1.0f; // radians
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    Mat4 Projection(float aspect) const { return Perspective(fovY, aspect, nearPlane, farPlane); }
    Vec3 Position() const { return TransformPoint(Inverse(view), {0, 0, 0}); }
};

struct DrawListOptions {
    bool frustumCull = true;
    int forceLod = -1;     // >= 0: always use this LOD level (clamped); -1: by screen size
    float lodBias = 1.0f;  // > 1 keeps detailed levels longer
};

struct DrawItem {
    int instance = -1;
    std::uint32_t subMesh = 0;
    MaterialId material = kNoResource;
    float depth = 0.0f; // view-space distance along the view direction
    bool transparent = false;
    int renderQueue = 2000;
};

struct DrawListStats {
    std::size_t instances = 0;   // drawable instances considered
    std::size_t lodRejected = 0; // not the selected level of their LOD group (or below the last level)
    std::size_t culled = 0;      // outside the view frustum
    std::size_t visible = 0;
    std::size_t opaqueItems = 0;
    std::size_t transparentItems = 0;
    std::size_t hiddenMaterials = 0; // submeshes whose material isn't drawn by the PC renderer
};

// Selected level of every LOD group (-1 = none visible) for this camera.
std::vector<int> SelectLods(const Scene& scene, const Mat4& view, const Mat4& projection, const DrawListOptions& options);

std::vector<DrawItem> BuildDrawList(const Scene& scene, const ResourceLibrary& resources, const Mat4& view,
                                    const Mat4& projection, const DrawListOptions& options,
                                    DrawListStats* stats = nullptr);

struct LightSettings {
    Vec3 direction = Normalize(Vec3{0.4f, 0.8f, 0.45f}); // towards the light, world space
    float ambient = 0.35f;
};

struct FrameStats {
    DrawListStats list;
    std::size_t drawCalls = 0;
    std::size_t skinnedDraws = 0;
    std::size_t triangles = 0;
    std::size_t pipelineChanges = 0;
};

struct GpuStats {
    std::size_t meshes = 0;
    std::size_t meshFailures = 0;
    std::size_t textures = 0;
    std::size_t texturesNative = 0;   // uploaded in their compressed format
    std::size_t texturesDecoded = 0;  // decoded to RGBA8 on the CPU first
    std::size_t textureFailures = 0;
    std::size_t pipelines = 0;
    std::size_t pipelineFailures = 0;
    std::size_t bonesClamped = 0;     // skinned meshes with more bones than the shader takes
};

// Maximum bones per skinned draw (size of the shader's bone array).
inline constexpr std::size_t kMaxBones = 256;

class SceneRenderer {
public:
    explicit SceneRenderer(render::RenderDevice& device) : device_(device) {}
    ~SceneRenderer();
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    // Records the scene's draws; call between BeginRenderPass and EndRenderPass.
    // GPU resources are created on first use.
    FrameStats Draw(const Scene& scene, const ResourceLibrary& resources, const Camera& camera,
                    const LightSettings& light = {}, const DrawListOptions& options = {});

    // One whole frame: BeginFrame, clear, Draw, EndFrame (capturing it first if
    // `capture` is set). Returns false if there was nothing to render into.
    bool RenderFrame(const Scene& scene, const ResourceLibrary& resources, const Camera& camera,
                     const render::ClearValues& clear, FrameStats* stats = nullptr, bool capture = false,
                     const LightSettings& light = {}, const DrawListOptions& options = {});

    // Uploads every mesh, texture and material pipeline of the library now.
    void Prepare(const ResourceLibrary& resources);
    // Frees all GPU resources (they're recreated on demand).
    void Release();

    const GpuStats& Stats() const { return gpuStats_; }
    bool forceSoftwareTextures = false; // decode compressed textures even where the GPU samples them

    // Bone matrices of a skinned instance (world space), as uploaded for drawing.
    static std::vector<Mat4> BoneMatrices(const Scene& scene, const RenderInstance& instance, const MeshData& mesh);

private:
    struct GpuMesh {
        render::BufferHandle vertices;
        render::BufferHandle indices;
        render::IndexType indexType = render::IndexType::UInt32;
        std::uint32_t stride = 0;
        bool skinned = false;
        bool failed = false;
    };
    struct GpuTexture {
        render::TextureHandle handle;
        render::SamplerDesc sampler;
        bool attempted = false;
    };
    struct PipelineKey {
        ShadingModel shading;
        SurfaceType surface;
        render::CullMode cull;
        bool depthWrite;
        bool skinned;
        std::uint32_t stride;
        auto operator<=>(const PipelineKey&) const = default;
    };

    const GpuMesh& Mesh(const ResourceLibrary& resources, MeshId id);
    const GpuTexture& Texture(const ResourceLibrary& resources, TextureId id);
    render::PipelineHandle Pipeline(const PipelineKey& key);

    render::RenderDevice& device_;
    std::vector<GpuMesh> meshes_;
    std::vector<GpuTexture> textures_;
    std::map<PipelineKey, render::PipelineHandle> pipelines_;
    GpuStats gpuStats_;
};

// Uploads a texture: the original blocks where the device samples the format,
// otherwise every mip decoded to RGBA8. Returns an empty handle on failure.
render::TextureHandle UploadTexture(render::RenderDevice& device, const assets::Texture2DAsset& texture,
                                    bool forceSoftware, bool* native = nullptr);
render::SamplerDesc SamplerFor(const assets::Texture2DAsset& texture);

} // namespace rr::scene
