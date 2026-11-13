#include "match_app.h"

#include "match_input.h"
#include "match_runner.h"
#include "match_view.h"

#include "photon_deterministic/log.h"
#include "rr/scene/scene_renderer.h"

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <memory>

namespace rr::app {

namespace {

void InstallLog(bool quiet) {
    static int errors = 0;
    const auto print = [quiet](const char* level) {
        return [level, quiet](const std::string& message) {
            // Repeated errors are counted; the first few are shown.
            if (quiet) return;
            if (++errors <= 50) std::fprintf(stderr, "%s: %s\n", level, message.c_str());
            else if (errors == 51) std::fprintf(stderr, "(further log messages suppressed)\n");
        };
    };
    Photon::Deterministic::Log::Init(nullptr, print("warning"), print("error"),
                                     [quiet](const std::exception& e) {
                                         if (!quiet && ++errors <= 50) std::fprintf(stderr, "exception: %s\n", e.what());
                                     });
}

struct Match {
    std::unique_ptr<GameContent> content;
    std::unique_ptr<MatchRunner> runner;
};

bool OpenMatch(const MatchCommand& command, Match& match) {
    std::string error;
    match.content = GameContent::Open(command.content, error);
    if (!match.content) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return false;
    }
    const MathTablesReport& math = match.content->MathTables();
    std::printf("math tables: %s (%zu from the game files, %zu approximated)\n",
                math.loaded ? (math.exact ? "exact" : "APPROXIMATE - not bit-exact with the original") : "missing",
                math.fromGame.size(), math.approximated.size());
    for (const auto& problem : math.problems) std::printf("  %s\n", problem.c_str());
    if (!math.loaded) {
        std::fprintf(stderr, "FPMath's lookup tables are required; stage the files above, export them to --lut-dir, "
                             "or run with --approx-math\n");
        return false;
    }
    for (const auto& note : match.content->Notes()) std::printf("content: %s\n", note.c_str());

    auto setup = BuildMatchSetup(*match.content, command.match, error);
    if (!setup) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return false;
    }
    std::printf("match: map %s (%" PRId64 "), mode %s, %d player slot(s), seed %d%s%s\n", setup->mapPath.c_str(),
                setup->runtime.Map.Id.Value, setup->gameModeName.c_str(), setup->session->PlayerCount, setup->runtime.Seed,
                setup->heroPath.empty() ? "" : ", hero ", setup->heroPath.c_str());
    for (const auto& note : setup->notes) std::printf("setup: %s\n", note.c_str());

    match.runner = std::make_unique<MatchRunner>(*match.content, std::move(*setup));
    if (!match.runner->Start(error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return false;
    }
    std::printf("session running: %d Hz, first tick %d\n", match.runner->Setup().session->UpdateFPS,
                match.runner->Session().FrameVerified()->Number);
    return true;
}

} // namespace

int RunMatchHeadless(const MatchCommand& command) {
    InstallLog(command.quiet);
    Match match;
    if (!OpenMatch(command, match)) return 1;
    MatchRunner& runner = *match.runner;
    std::size_t printed = 0;
    for (std::uint64_t done = 0; done < command.ticks;) {
        const int step = static_cast<int>(std::min<std::uint64_t>(60, command.ticks - done));
        runner.RunTicks(step);
        done += static_cast<std::uint64_t>(step);
        // Checksums arrive every ChecksumInterval verified ticks.
        std::size_t index = 0;
        for (const auto& [frame, checksum] : runner.Checksums()) {
            if (index++ < printed) continue;
            std::printf("tick %d checksum %016" PRIx64 " entities %d\n", frame, checksum, runner.Verified()->EntityCount());
            ++printed;
        }
    }
    runner.PrintDiagnostics(stdout);
    if (command.checkViews) {
        MatchView view(*match.content);
        const auto views = match.content->AssetsOfClass("EntityViewAsset");
        const std::size_t resolved = view.CheckViews(views);
        std::size_t renderers = 0, skinned = 0;
        for (const auto& instance : view.Scene().Instances()) {
            ++renderers;
            if (instance.kind == scene::RendererKind::Skinned) ++skinned;
        }
        const scene::ContentStats& cs = view.ContentStats();
        std::printf("views: %zu EntityView assets, %zu prefabs instantiated, %zu renderers (%zu skinned), "
                    "%zu meshes / %zu materials / %zu textures loaded, %zu meshes failed, %zu pointers into missing files\n",
                    views.size(), resolved, renderers, skinned, cs.meshesLoaded, cs.materialsLoaded, cs.texturesLoaded,
                    cs.meshesFailed, cs.unresolvedPointers);
    }
    return 0;
}

int RunMatchWindowed(const MatchCommand& command, platform::Platform& platform, render::RenderDevice& device) {
    InstallLog(command.quiet);
    Match match;
    if (!OpenMatch(command, match)) return 1;
    MatchRunner& runner = *match.runner;
    MatchView view(*match.content);
    InputMapper input;
    runner.LocalInput = [&](int) -> std::optional<Quantum::PolledInput> { return input.Poll(platform.GetInput()); };

    scene::SceneRenderer renderer(device);
    render::ClearValues clear;
    clear.color[0] = 0.30f;
    clear.color[1] = 0.42f;
    clear.color[2] = 0.55f;
    double previous = platform.GetClock().Now();
    std::uint64_t frame = 0;
    while (platform.PumpEvents()) {
        const platform::InputState& state = platform.GetInput();
        if (state.Get(platform::Key::Escape).pressed) break;
        const double now = platform.GetClock().Now();
        // Automated runs (--frames) step exactly one tick per frame.
        const double seconds = command.frames ? runner.TickSeconds() : std::min(now - previous, 0.25);
        previous = now;
        input.UpdateCamera(state, static_cast<float>(seconds));
        runner.Update(seconds);

        Quantum::QuantumGame& game = runner.Game();
        if (Quantum::Frame* predicted = game.Frames().Predicted.get()) {
            view.Sync(*predicted, game.Frames().PredictedPrevious.get(), game.InterpolationFactor());
        }
        if (!renderer.RenderFrame(view.Scene(), view.Resources(), view.Camera(input.camera), clear, nullptr, false, {}, {})) {
            continue;
        }
        if (command.frames && ++frame >= command.frames) break;
    }
    const ViewStats& vs = view.Stats();
    std::printf("view: %zu entities with a view, %zu instances (%zu created), %zu view assets, %zu without a prefab\n",
                vs.viewedEntities, vs.instances, vs.instancesCreated, vs.viewAssets, vs.viewsWithoutPrefab);
    runner.PrintDiagnostics(stdout);
    return 0;
}

} // namespace rr::app
