// platform.h - the boundary between game code and the host operating system.
//
// On Android the game ran inside a closed-source engine runtime that owned the
// window, input, timing and file access. Here that role is played by a small set
// of interfaces that a backend implements. Game code only talks to these
// interfaces, never to an OS or windowing API directly.
//
// Backends:
//   null - headless; no window, no input. Used by tests and tools.
//   sdl  - desktop window, keyboard/mouse/gamepad, OpenGL or Vulkan surface.
#pragma once

#include "rr/platform/input.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::platform {

// Graphics APIs a window can be prepared for. The user chooses one at startup.
enum class GraphicsApi {
    None,   // no rendering (headless)
    OpenGL, // OpenGL 4.1 core profile
    Vulkan, // Vulkan 1.0+
};

const char* ToString(GraphicsApi api);
std::optional<GraphicsApi> ParseGraphicsApi(const std::string& text);

struct WindowDesc {
    std::string title = "Ranger Rumble";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    GraphicsApi api = GraphicsApi::None;
};

// Monotonic time source for the frame loop.
class Clock {
public:
    virtual ~Clock() = default;
    // Seconds since the clock was created.
    virtual double Now() const = 0;
};

// Read-only access to the game's content (extracted from the user's own copy of
// the game) plus a writable location for settings and save data.
class FileSystem {
public:
    virtual ~FileSystem() = default;
    virtual std::optional<std::vector<std::uint8_t>> ReadContent(const std::string& path) const = 0;
    virtual std::filesystem::path UserDataDirectory() const = 0;
};

// Hooks a renderer needs from a window prepared for OpenGL.
class OpenGLContext {
public:
    virtual ~OpenGLContext() = default;
    virtual void* GetProcAddress(const char* name) const = 0;
    virtual void SwapBuffers() = 0;
};

// Hooks a renderer needs from a window prepared for Vulkan. Handles are passed
// as opaque values so this header doesn't depend on the Vulkan SDK.
class VulkanSurfaceProvider {
public:
    virtual ~VulkanSurfaceProvider() = default;
    virtual std::vector<const char*> RequiredInstanceExtensions() const = 0;
    // `instance` is a VkInstance; on success writes a VkSurfaceKHR to `outSurface`.
    virtual bool CreateSurface(void* instance, std::uint64_t* outSurface) const = 0;
};

class Platform {
public:
    virtual ~Platform() = default;

    // Creates the main window. For OpenGL this also creates the context; for
    // Vulkan the renderer creates its own surface through GetVulkanSurfaceProvider.
    virtual bool CreateMainWindow(const WindowDesc& desc) = 0;

    // Pumps OS events and refreshes input. Returns false once the user asked to quit.
    virtual bool PumpEvents() = 0;

    // Size of the window's drawable area in pixels (may differ from the window
    // size on high-DPI displays). Zero while minimised.
    virtual void GetDrawableSize(int& width, int& height) const = 0;

    virtual const InputState& GetInput() const = 0;
    virtual const Clock& GetClock() const = 0;
    virtual const FileSystem& GetFileSystem() const = 0;

    // Null when the window wasn't created for that API.
    virtual OpenGLContext* GetOpenGLContext() = 0;
    virtual const VulkanSurfaceProvider* GetVulkanSurfaceProvider() const = 0;
};

struct PlatformConfig {
    // Folder that holds extracted game content (see tools/README.md).
    std::filesystem::path contentRoot = "gamedata";
    std::filesystem::path userDataRoot = "userdata";
    // Stop after this many frames (PumpEvents returns false). Zero = no limit.
    // Useful for automated runs of either backend.
    std::uint64_t frameLimit = 0;
};

std::unique_ptr<Platform> CreateNullPlatform(const PlatformConfig& config);

// Returns null if the desktop backend wasn't compiled in.
std::unique_ptr<Platform> CreateDesktopPlatform(const PlatformConfig& config);

} // namespace rr::platform
