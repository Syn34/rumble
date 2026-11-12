// render_device.h - graphics-API-neutral rendering interface.
//
// The game's renderer is written once against this interface; OpenGL and
// Vulkan backends implement it. The user picks the backend at startup
// (command line or settings file) and can switch between runs.
//
// Conventions shared by every backend:
//   * clip space is Y-up with depth in [0, 1] (shaders call rr_ToClip)
//   * per-draw constants are a block of at most kMaxDrawConstantsSize bytes,
//     std140 layout, vec4/mat4 members only
//   * captured frames are RGBA8, rows ordered top to bottom
#pragma once

#include "rr/platform/platform.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::render {

using platform::GraphicsApi;

inline constexpr std::size_t kMaxDrawConstantsSize = 128;

struct BufferHandle {
    std::uint32_t id = 0;
    explicit operator bool() const { return id != 0; }
};

struct PipelineHandle {
    std::uint32_t id = 0;
    explicit operator bool() const { return id != 0; }
};

enum class BufferUsage { Vertex, Index };
enum class IndexType { UInt16, UInt32 };

enum class VertexFormat {
    Float2,
    Float3,
    Float4,
    UByte4Norm, // four bytes mapped to [0, 1], e.g. vertex colours
};

struct VertexAttribute {
    std::uint32_t location;
    VertexFormat format;
    std::uint32_t offset;
};

struct VertexLayout {
    std::uint32_t stride = 0;
    std::vector<VertexAttribute> attributes;
};

enum class PrimitiveTopology { TriangleList, LineList };
enum class CullMode { None, Back, Front };
enum class BlendMode { Opaque, AlphaBlend };

struct PipelineDesc {
    std::string shaderProgram; // name of a program in src/render/shaders
    VertexLayout vertexLayout;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    CullMode cull = CullMode::Back; // front faces are counter-clockwise
    BlendMode blend = BlendMode::Opaque;
    bool depthTest = true;
    bool depthWrite = true;
};

struct ClearValues {
    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    float depth = 1.0f;
};

struct Image {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> rgba;
};

class RenderDevice {
public:
    virtual ~RenderDevice() = default;

    virtual GraphicsApi Api() const = 0;
    virtual std::string DeviceName() const = 0;

    // Resources ------------------------------------------------------------
    virtual BufferHandle CreateBuffer(BufferUsage usage, const void* data, std::size_t size) = 0;
    virtual void DestroyBuffer(BufferHandle buffer) = 0;
    virtual PipelineHandle CreatePipeline(const PipelineDesc& desc) = 0;
    virtual void DestroyPipeline(PipelineHandle pipeline) = 0;

    // Frame ----------------------------------------------------------------
    // Returns false when there is nothing to render into this frame (for
    // example while the window is minimised); skip drawing and EndFrame then.
    virtual bool BeginFrame() = 0;
    virtual void BeginRenderPass(const ClearValues& clear) = 0;
    virtual void BindPipeline(PipelineHandle pipeline) = 0;
    virtual void BindVertexBuffer(BufferHandle buffer) = 0;
    virtual void BindIndexBuffer(BufferHandle buffer, IndexType type) = 0;
    virtual void SetDrawConstants(const void* data, std::size_t size) = 0;
    virtual void Draw(std::uint32_t vertexCount, std::uint32_t firstVertex = 0) = 0;
    virtual void DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex = 0,
                             std::int32_t vertexOffset = 0) = 0;
    virtual void EndRenderPass() = 0;
    // Submits the frame and presents it.
    virtual void EndFrame() = 0;

    // Size of the image being rendered this frame, in pixels.
    virtual void GetRenderSize(int& width, int& height) const = 0;

    // Frame capture (screenshots, automated backend comparison) -------------
    // Asks for a copy of the next frame passed to EndFrame.
    virtual void RequestCapture() = 0;
    // Returns the captured frame once it is available, then forgets it.
    virtual std::optional<Image> TakeCapture() = 0;
};

// Backends compiled into this build, in order of preference.
std::vector<GraphicsApi> AvailableGraphicsApis();

// Creates a device for `api` on a window that was created for the same API.
// Returns null (after logging why) if the API is unavailable or fails to start.
std::unique_ptr<RenderDevice> CreateRenderDevice(platform::Platform& platform, GraphicsApi api);

// Writes an image as binary PPM (no alpha). Handy for screenshots without
// pulling in an image library.
bool WritePpm(const Image& image, const std::string& path);

} // namespace rr::render
