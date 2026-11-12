// platform.h - the boundary between game code and the host operating system.
//
// On Android the game ran inside a closed-source engine runtime that owned the
// window, input, timing and file access. Here that role is played by a small set
// of interfaces that a backend implements. Game code only talks to these
// interfaces, never to an OS or windowing API directly.
//
// Backends:
//   null  - headless; no window, scripted/no input. Used by tests and servers.
//   (planned) sdl3 - desktop window, keyboard/mouse/gamepad, audio device.
#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::platform {

struct WindowDesc {
    std::string title = "Ranger Rumble";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
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

class Platform {
public:
    virtual ~Platform() = default;

    virtual bool CreateMainWindow(const WindowDesc& desc) = 0;

    // Pumps OS events. Returns false once the user has asked to quit.
    virtual bool PumpEvents() = 0;

    // Presents the frame that was just rendered.
    virtual void Present() = 0;

    virtual const Clock& GetClock() const = 0;
    virtual const FileSystem& GetFileSystem() const = 0;
};

struct PlatformConfig {
    // Folder that holds extracted game content (see tools/README.md).
    std::filesystem::path contentRoot = "gamedata";
    std::filesystem::path userDataRoot = "userdata";
    // For the null backend: how many frames to run before PumpEvents() reports quit.
    // Zero means run until told otherwise.
    std::uint64_t headlessFrameLimit = 0;
};

std::unique_ptr<Platform> CreateNullPlatform(const PlatformConfig& config);

} // namespace rr::platform
