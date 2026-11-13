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
//   * textures: rows are uploaded in memory order and texture coordinate (0, 0)
//     addresses the first texel in memory on both APIs; shaders declare up to
//     kMaxTextureSlots textures with RR_TEXTURE(slot, uTexture<slot>)
#pragma once

#include "rr/platform/platform.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rr::render {

using platform::GraphicsApi;

inline constexpr std::size_t kMaxDrawConstantsSize = 128;
inline constexpr std::uint32_t kMaxTextureSlots = 4;

struct BufferHandle {
    std::uint32_t id = 0;
    explicit operator bool() const { return id != 0; }
};

struct PipelineHandle {
    std::uint32_t id = 0;
    explicit operator bool() const { return id != 0; }
};

struct TextureHandle {
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

// Texture formats. Compressed formats are only usable where
// SupportsTextureFormat() says so; RGBA8 always works.
enum class TextureFormat {
    RGBA8,
    RGBA8_sRGB,
    ASTC_4x4,
    ASTC_4x4_sRGB,
    ASTC_5x5,
    ASTC_5x5_sRGB,
    ASTC_6x6,
    ASTC_6x6_sRGB,
    ASTC_8x8,
    ASTC_8x8_sRGB,
    ASTC_10x10,
    ASTC_10x10_sRGB,
    ASTC_12x12,
    ASTC_12x12_sRGB,
};

struct TextureFormatLayout {
    int blockWidth = 1;
    int blockHeight = 1;
    int bytesPerBlock = 4;
    bool compressed = false;
    bool srgb = false;
};
TextureFormatLayout DescribeTextureFormat(TextureFormat format);
// Bytes of one mip level of the given size.
std::size_t TextureLevelBytes(TextureFormat format, int width, int height);
const char* ToString(TextureFormat format);

struct TextureDesc {
    int width = 0;
    int height = 0;
    std::uint32_t mipCount = 1; // levels supplied to CreateTexture, largest first
    TextureFormat format = TextureFormat::RGBA8;
};

struct TextureLevelData {
    const void* data = nullptr;
    std::size_t size = 0; // must equal TextureLevelBytes() for the level
};

enum class FilterMode { Nearest, Linear };
enum class WrapMode { Repeat, ClampToEdge, MirroredRepeat };

struct SamplerDesc {
    FilterMode filter = FilterMode::Linear;    // magnification and minification
    FilterMode mipFilter = FilterMode::Linear; // between mip levels
    bool useMips = true;                       // false: sample level 0 only
    WrapMode wrapU = WrapMode::Repeat;
    WrapMode wrapV = WrapMode::Repeat;

    bool operator==(const SamplerDesc&) const = default;
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

    virtual bool SupportsTextureFormat(TextureFormat format) const = 0;
    // Creates a 2D texture from `levels` (desc.mipCount entries). Returns an empty
    // handle if the format is unsupported or the data doesn't match the size.
    virtual TextureHandle CreateTexture(const TextureDesc& desc, std::span<const TextureLevelData> levels) = 0;
    virtual void DestroyTexture(TextureHandle texture) = 0;
    // Reads one mip level back as RGBA8 (decoded if the texture is compressed), for
    // tests and tools. Returns nothing where the backend can't read the format back.
    virtual std::optional<Image> ReadTexture(TextureHandle texture, std::uint32_t mipLevel) = 0;

    // Frame ----------------------------------------------------------------
    // Returns false when there is nothing to render into this frame (for
    // example while the window is minimised); skip drawing and EndFrame then.
    virtual bool BeginFrame() = 0;
    virtual void BeginRenderPass(const ClearValues& clear) = 0;
    virtual void BindPipeline(PipelineHandle pipeline) = 0;
    virtual void BindVertexBuffer(BufferHandle buffer) = 0;
    virtual void BindIndexBuffer(BufferHandle buffer, IndexType type) = 0;
    virtual void SetDrawConstants(const void* data, std::size_t size) = 0;
    // Binds a texture to slot 0..kMaxTextureSlots-1 for the following draws. Slots
    // without a texture sample opaque white.
    virtual void BindTexture(std::uint32_t slot, TextureHandle texture, const SamplerDesc& sampler) = 0;
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
