// Entry point for the PC build.
//
// At this stage there is no renderer or game loop content yet: the executable
// brings up the platform layer and runs an empty fixed-step loop so the overall
// program structure is in place and testable.
#include "rr/platform/platform.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace {

struct Options {
    bool headless = true; // only the null backend exists so far
    std::uint64_t frames = 60;
};

Options ParseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            options.frames = std::stoull(argv[++i]);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv) {
    const Options options = ParseOptions(argc, argv);

    rr::platform::PlatformConfig config;
    config.headlessFrameLimit = options.frames;
    auto platform = rr::platform::CreateNullPlatform(config);

    if (!platform->CreateMainWindow({})) {
        std::fprintf(stderr, "failed to create the main window\n");
        return 1;
    }

    // The simulation runs on a fixed tick; rendering happens once per OS frame.
    constexpr double kTickSeconds = 1.0 / 60.0;
    double accumulator = 0.0;
    double previous = platform->GetClock().Now();
    std::uint64_t ticks = 0;
    std::uint64_t frames = 0;

    while (platform->PumpEvents()) {
        const double now = platform->GetClock().Now();
        accumulator += now - previous;
        previous = now;

        while (accumulator >= kTickSeconds) {
            // TODO: advance the gameplay simulation one tick.
            accumulator -= kTickSeconds;
            ++ticks;
        }

        // TODO: render.
        platform->Present();
        ++frames;
    }

    std::printf("Ranger Rumble (PC) - ran %llu frames, %llu simulation ticks\n",
                static_cast<unsigned long long>(frames), static_cast<unsigned long long>(ticks));
    return 0;
}
