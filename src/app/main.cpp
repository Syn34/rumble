// Entry point for the PC build.
//
// Brings up the platform layer and the renderer the user picked, then runs a
// fixed-step loop. Until game rendering exists it draws a placeholder scene.
//
// Renderer choice, in priority order:
//   1. --renderer vulkan|opengl on the command line
//   2. renderer= in userdata/settings.ini
//   3. the first available backend (Vulkan, then OpenGL)
// If the chosen backend can't start, the other one is tried.
#include "settings.h"
#include "test_scene.h"

#include "photon_deterministic/system_allocator.h"
#include "rr/platform/platform.h"
#include "rr/render/render_device.h"
#include "rr/scene/demo_scene.h"
#include "rr/scene/scene_renderer.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

namespace {

using rr::platform::GraphicsApi;

struct Options {
    std::optional<GraphicsApi> renderer;
    bool headless = false;
    std::uint64_t frames = 0;    // 0 = run until the window is closed
    std::string screenshotPath;  // capture the last frame to this PPM file
    bool saveRenderer = false;   // remember --renderer in settings.ini
    bool listRenderers = false;
    bool sceneDemo = false;      // draw the synthetic scene through the engine's scene renderer
};

void PrintUsage() {
    std::printf(
        "usage: RangerRumble [options]\n"
        "  --renderer <vulkan|opengl>  graphics API to use\n"
        "  --save-renderer             remember the --renderer choice for future runs\n"
        "  --list-renderers            print the renderers in this build and exit\n"
        "  --frames <n>                exit after n frames\n"
        "  --screenshot <file.ppm>     save the final frame (use with --frames)\n"
        "  --headless                  no window or rendering\n"
        "  --scene-demo                draw the synthetic scene-renderer demo instead of the placeholder\n");
}

bool ParseOptions(int argc, char** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next = [&]() -> const char* { return i + 1 < argc ? argv[++i] : nullptr; };
        if (arg == "--renderer") {
            const char* value = next();
            options.renderer = value ? rr::platform::ParseGraphicsApi(value) : std::nullopt;
            if (!options.renderer || *options.renderer == GraphicsApi::None) {
                std::fprintf(stderr, "--renderer expects 'vulkan' or 'opengl'\n");
                return false;
            }
        } else if (arg == "--frames") {
            const char* value = next();
            if (!value) return false;
            options.frames = std::stoull(value);
        } else if (arg == "--screenshot") {
            const char* value = next();
            if (!value) return false;
            options.screenshotPath = value;
        } else if (arg == "--headless") {
            options.headless = true;
        } else if (arg == "--save-renderer") {
            options.saveRenderer = true;
        } else if (arg == "--scene-demo") {
            options.sceneDemo = true;
        } else if (arg == "--list-renderers") {
            options.listRenderers = true;
        } else {
            PrintUsage();
            return false;
        }
    }
    return true;
}

// Order in which to try renderers: the preferred one first, then the rest.
std::vector<GraphicsApi> RendererCandidates(std::optional<GraphicsApi> preferred) {
    std::vector<GraphicsApi> candidates = rr::render::AvailableGraphicsApis();
    if (preferred) {
        auto it = std::find(candidates.begin(), candidates.end(), *preferred);
        if (it == candidates.end()) {
            std::fprintf(stderr, "renderer '%s' is not available in this build\n",
                         rr::platform::ToString(*preferred));
        } else {
            std::rotate(candidates.begin(), it, it + 1);
        }
    }
    return candidates;
}

int RunHeadless(const Options& options) {
    rr::platform::PlatformConfig config;
    config.frameLimit = options.frames ? options.frames : 60;
    auto platform = rr::platform::CreateNullPlatform(config);
    platform->CreateMainWindow({});
    std::uint64_t frames = 0;
    while (platform->PumpEvents()) {
        // TODO: advance the gameplay simulation.
        ++frames;
    }
    std::printf("headless: ran %llu frames\n", static_cast<unsigned long long>(frames));
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    Options options;
    if (!ParseOptions(argc, argv, options)) return 2;

    // Raw-memory services the simulation runtime expects the platform to provide.
    Photon::Deterministic::SystemUtility::Install();

    if (options.listRenderers) {
        for (GraphicsApi api : rr::render::AvailableGraphicsApis()) {
            std::printf("%s\n", rr::platform::ToString(api));
        }
        return 0;
    }
    if (options.headless) return RunHeadless(options);

    rr::platform::PlatformConfig config;
    config.frameLimit = options.frames;
    const auto settingsFile = config.userDataRoot / "settings.ini";
    rr::app::Settings settings = rr::app::LoadSettings(settingsFile);
    if (options.renderer && options.saveRenderer) {
        settings.renderer = options.renderer;
        rr::app::SaveSettings(settings, settingsFile);
    }

    // Try each renderer in turn. Each attempt needs a fresh window because the
    // window is created specifically for one graphics API.
    std::unique_ptr<rr::platform::Platform> platform;
    std::unique_ptr<rr::render::RenderDevice> device;
    for (GraphicsApi api : RendererCandidates(options.renderer ? options.renderer : settings.renderer)) {
        platform = rr::platform::CreateDesktopPlatform(config);
        if (!platform) {
            std::fprintf(stderr, "no desktop platform in this build; use --headless\n");
            return 1;
        }
        rr::platform::WindowDesc window;
        window.width = settings.windowWidth;
        window.height = settings.windowHeight;
        window.fullscreen = settings.fullscreen;
        window.vsync = settings.vsync;
        window.api = api;
        if (platform->CreateMainWindow(window)) {
            device = rr::render::CreateRenderDevice(*platform, api);
        }
        if (device) break;
        std::fprintf(stderr, "renderer '%s' failed to start; trying the next one\n",
                     rr::platform::ToString(api));
        platform.reset();
    }
    if (!device) {
        std::fprintf(stderr, rr::render::AvailableGraphicsApis().empty()
                                 ? "this build has no renderers; run with --headless\n"
                                 : "no renderer could be started\n");
        return 1;
    }
    std::printf("renderer: %s - %s\n", rr::platform::ToString(device->Api()),
                device->DeviceName().c_str());

    // The engine's scene renderer on a scene built in code (the game's own scenes
    // come later; tools/sceneview shows them).
    std::unique_ptr<rr::scene::DemoScene> demo;
    std::unique_ptr<rr::scene::SceneRenderer> sceneRenderer;
    if (options.sceneDemo) {
        demo = rr::scene::BuildDemoScene();
        sceneRenderer = std::make_unique<rr::scene::SceneRenderer>(*device);
    }

    rr::app::TestScene scene;
    if (!options.sceneDemo && !scene.Create(*device)) {
        std::fprintf(stderr, "failed to create the placeholder scene\n");
        return 1;
    }

    constexpr double kTickSeconds = 1.0 / 60.0;
    double accumulator = 0.0;
    double previous = platform->GetClock().Now();
    std::uint64_t frame = 0;

    while (platform->PumpEvents()) {
        if (platform->GetInput().Get(rr::platform::Key::Escape).pressed) break;

        const double now = platform->GetClock().Now();
        accumulator += std::min(now - previous, 0.25); // don't spiral after a stall
        previous = now;
        while (accumulator >= kTickSeconds) {
            // TODO: advance the gameplay simulation one tick.
            accumulator -= kTickSeconds;
        }

        if (!device->BeginFrame()) continue;
        // Automated runs animate by frame number so output is reproducible.
        const float time = options.frames ? static_cast<float>(frame) * 0.05f
                                          : static_cast<float>(now);
        if (demo) {
            demo->Animate(time);
            rr::render::ClearValues clear;
            clear.color[0] = 0.10f;
            clear.color[1] = 0.11f;
            clear.color[2] = 0.13f;
            device->BeginRenderPass(clear);
            sceneRenderer->Draw(demo->scene, demo->resources, demo->camera);
            device->EndRenderPass();
        } else {
            scene.Draw(*device, time);
        }
        const bool lastFrame = options.frames && frame + 1 == options.frames;
        if (lastFrame && !options.screenshotPath.empty()) device->RequestCapture();
        device->EndFrame();
        ++frame;
    }

    if (!options.screenshotPath.empty()) {
        if (auto image = device->TakeCapture()) {
            if (rr::render::WritePpm(*image, options.screenshotPath)) {
                std::printf("saved %dx%d screenshot to %s\n", image->width, image->height,
                            options.screenshotPath.c_str());
            }
        } else {
            std::fprintf(stderr, "no frame was captured\n");
            return 1;
        }
    }
    return 0;
}
