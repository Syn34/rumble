#include "rr/render/render_device.h"

#include <cstdio>
#include <fstream>

namespace rr::render {

#if RR_HAS_OPENGL
std::unique_ptr<RenderDevice> CreateOpenGLDevice(platform::Platform& platform);
#endif
#if RR_HAS_VULKAN
std::unique_ptr<RenderDevice> CreateVulkanDevice(platform::Platform& platform);
#endif

std::vector<GraphicsApi> AvailableGraphicsApis() {
    std::vector<GraphicsApi> apis;
#if RR_HAS_VULKAN
    apis.push_back(GraphicsApi::Vulkan);
#endif
#if RR_HAS_OPENGL
    apis.push_back(GraphicsApi::OpenGL);
#endif
    return apis;
}

std::unique_ptr<RenderDevice> CreateRenderDevice([[maybe_unused]] platform::Platform& platform,
                                                 GraphicsApi api) {
    switch (api) {
#if RR_HAS_OPENGL
    case GraphicsApi::OpenGL: return CreateOpenGLDevice(platform);
#endif
#if RR_HAS_VULKAN
    case GraphicsApi::Vulkan: return CreateVulkanDevice(platform);
#endif
    default:
        std::fprintf(stderr, "renderer '%s' is not available in this build\n",
                     platform::ToString(api));
        return nullptr;
    }
}

TextureFormatLayout DescribeTextureFormat(TextureFormat format) {
    auto astc = [](int size, bool srgb) { return TextureFormatLayout{size, size, 16, true, srgb}; };
    switch (format) {
    case TextureFormat::RGBA8: return {1, 1, 4, false, false};
    case TextureFormat::RGBA8_sRGB: return {1, 1, 4, false, true};
    case TextureFormat::ASTC_4x4: return astc(4, false);
    case TextureFormat::ASTC_4x4_sRGB: return astc(4, true);
    case TextureFormat::ASTC_5x5: return astc(5, false);
    case TextureFormat::ASTC_5x5_sRGB: return astc(5, true);
    case TextureFormat::ASTC_6x6: return astc(6, false);
    case TextureFormat::ASTC_6x6_sRGB: return astc(6, true);
    case TextureFormat::ASTC_8x8: return astc(8, false);
    case TextureFormat::ASTC_8x8_sRGB: return astc(8, true);
    case TextureFormat::ASTC_10x10: return astc(10, false);
    case TextureFormat::ASTC_10x10_sRGB: return astc(10, true);
    case TextureFormat::ASTC_12x12: return astc(12, false);
    case TextureFormat::ASTC_12x12_sRGB: return astc(12, true);
    }
    return {};
}

std::size_t TextureLevelBytes(TextureFormat format, int width, int height) {
    const TextureFormatLayout f = DescribeTextureFormat(format);
    if (width <= 0 || height <= 0) return 0;
    const auto bx = static_cast<std::size_t>((width + f.blockWidth - 1) / f.blockWidth);
    const auto by = static_cast<std::size_t>((height + f.blockHeight - 1) / f.blockHeight);
    return bx * by * static_cast<std::size_t>(f.bytesPerBlock);
}

const char* ToString(TextureFormat format) {
    switch (format) {
    case TextureFormat::RGBA8: return "RGBA8";
    case TextureFormat::RGBA8_sRGB: return "RGBA8_sRGB";
    case TextureFormat::ASTC_4x4: return "ASTC_4x4";
    case TextureFormat::ASTC_4x4_sRGB: return "ASTC_4x4_sRGB";
    case TextureFormat::ASTC_5x5: return "ASTC_5x5";
    case TextureFormat::ASTC_5x5_sRGB: return "ASTC_5x5_sRGB";
    case TextureFormat::ASTC_6x6: return "ASTC_6x6";
    case TextureFormat::ASTC_6x6_sRGB: return "ASTC_6x6_sRGB";
    case TextureFormat::ASTC_8x8: return "ASTC_8x8";
    case TextureFormat::ASTC_8x8_sRGB: return "ASTC_8x8_sRGB";
    case TextureFormat::ASTC_10x10: return "ASTC_10x10";
    case TextureFormat::ASTC_10x10_sRGB: return "ASTC_10x10_sRGB";
    case TextureFormat::ASTC_12x12: return "ASTC_12x12";
    case TextureFormat::ASTC_12x12_sRGB: return "ASTC_12x12_sRGB";
    }
    return "unknown";
}

bool WritePpm(const Image& image, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out << "P6\n" << image.width << ' ' << image.height << "\n255\n";
    for (std::size_t i = 0; i + 3 < image.rgba.size(); i += 4) {
        out.write(reinterpret_cast<const char*>(&image.rgba[i]), 3);
    }
    return static_cast<bool>(out);
}

} // namespace rr::render
