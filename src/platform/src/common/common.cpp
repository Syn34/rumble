#include "common.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>

namespace rr::platform {

const char* ToString(GraphicsApi api) {
    switch (api) {
    case GraphicsApi::None: return "none";
    case GraphicsApi::OpenGL: return "opengl";
    case GraphicsApi::Vulkan: return "vulkan";
    }
    return "unknown";
}

std::optional<GraphicsApi> ParseGraphicsApi(const std::string& text) {
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower == "opengl" || lower == "gl") return GraphicsApi::OpenGL;
    if (lower == "vulkan" || lower == "vk") return GraphicsApi::Vulkan;
    if (lower == "none" || lower == "headless") return GraphicsApi::None;
    return std::nullopt;
}

namespace detail {

DirectoryFileSystem::DirectoryFileSystem(std::filesystem::path contentRoot,
                                         std::filesystem::path userRoot)
    : contentRoot_(std::move(contentRoot)), userRoot_(std::move(userRoot)) {}

std::optional<std::vector<std::uint8_t>>
DirectoryFileSystem::ReadContent(const std::string& path) const {
    std::ifstream file(contentRoot_ / path, std::ios::binary);
    if (!file) {
        return std::nullopt;
    }
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(file), {});
}

} // namespace detail
} // namespace rr::platform
