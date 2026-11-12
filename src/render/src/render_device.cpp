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
