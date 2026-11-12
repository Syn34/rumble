// settings.h - user settings persisted between runs (userdata/settings.ini).
#pragma once

#include "rr/platform/platform.h"

#include <filesystem>
#include <optional>

namespace rr::app {

struct Settings {
    // Which renderer to use. Unset means "pick the best available".
    std::optional<platform::GraphicsApi> renderer;
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
};

// Missing or unreadable files give defaults; unknown keys are ignored.
Settings LoadSettings(const std::filesystem::path& file);
bool SaveSettings(const Settings& settings, const std::filesystem::path& file);

} // namespace rr::app
