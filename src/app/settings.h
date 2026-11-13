// settings.h - user settings persisted between runs (userdata/settings.ini).
#pragma once

#include "rr/platform/platform.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace rr::app {

struct Settings {
    // Which renderer to use. Unset means "pick the best available".
    std::optional<platform::GraphicsApi> renderer;
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;

    // Where the user's game files are (see README: "Running a match").
    std::filesystem::path contentDir;              // content_dir: asset pack aa/Android folder
    std::filesystem::path engineLib;               // engine_lib: libunity.so (arm64 split)
    std::vector<std::filesystem::path> dataDirs;   // data_dirs: player data folders (bin/Data), ';'-separated
    std::filesystem::path metadata;                // metadata: global-metadata.dat
    std::filesystem::path lutDir;                  // lut_dir: optional folder of exported LUT/*.bytes
};

// Missing or unreadable files give defaults; unknown keys are ignored.
Settings LoadSettings(const std::filesystem::path& file);
bool SaveSettings(const Settings& settings, const std::filesystem::path& file);

} // namespace rr::app
