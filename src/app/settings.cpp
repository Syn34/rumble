#include "settings.h"

#include <fstream>
#include <string>

namespace rr::app {
namespace {

std::string Trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r");
    return s.substr(first, last - first + 1);
}

bool ParseBool(const std::string& v) { return v == "1" || v == "true" || v == "yes" || v == "on"; }

} // namespace

Settings LoadSettings(const std::filesystem::path& file) {
    Settings settings;
    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = Trim(line.substr(0, eq));
        const std::string value = Trim(line.substr(eq + 1));
        try {
            if (key == "renderer") {
                settings.renderer = platform::ParseGraphicsApi(value);
            } else if (key == "width") {
                settings.windowWidth = std::stoi(value);
            } else if (key == "height") {
                settings.windowHeight = std::stoi(value);
            } else if (key == "fullscreen") {
                settings.fullscreen = ParseBool(value);
            } else if (key == "vsync") {
                settings.vsync = ParseBool(value);
            } else if (key == "content_dir") {
                settings.contentDir = value;
            } else if (key == "engine_lib") {
                settings.engineLib = value;
            } else if (key == "data_dirs") {
                settings.dataDirs.clear();
                std::size_t start = 0;
                while (start <= value.size()) {
                    const auto end = value.find(';', start);
                    const std::string item = Trim(value.substr(start, end == std::string::npos ? std::string::npos : end - start));
                    if (!item.empty()) settings.dataDirs.emplace_back(item);
                    if (end == std::string::npos) break;
                    start = end + 1;
                }
            } else if (key == "metadata") {
                settings.metadata = value;
            } else if (key == "lut_dir") {
                settings.lutDir = value;
            }
        } catch (const std::exception&) {
            // A malformed number keeps the default.
        }
    }
    return settings;
}

bool SaveSettings(const Settings& settings, const std::filesystem::path& file) {
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    std::ofstream out(file);
    if (!out) return false;
    out << "# Ranger Rumble settings\n"
        << "# renderer: vulkan or opengl\n"
        << "renderer=" << (settings.renderer ? platform::ToString(*settings.renderer) : "auto") << '\n'
        << "width=" << settings.windowWidth << '\n'
        << "height=" << settings.windowHeight << '\n'
        << "fullscreen=" << (settings.fullscreen ? "true" : "false") << '\n'
        << "vsync=" << (settings.vsync ? "true" : "false") << '\n'
        << "# your game files (see README)\n"
        << "content_dir=" << settings.contentDir.string() << '\n'
        << "engine_lib=" << settings.engineLib.string() << '\n';
    out << "data_dirs=";
    for (std::size_t i = 0; i < settings.dataDirs.size(); ++i) out << (i ? ";" : "") << settings.dataDirs[i].string();
    out << '\n'
        << "metadata=" << settings.metadata.string() << '\n'
        << "lut_dir=" << settings.lutDir.string() << '\n';
    return static_cast<bool>(out);
}

} // namespace rr::app
