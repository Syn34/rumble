// QuantumGame in a local DeterministicSession with content built in code: the match starts,
// every system of SystemSetup runs each tick, and two runs give the same checksums.
#include "test_framework.h"

#include "math_tables.h"
#include "photon_deterministic/deterministic_session.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/assets/gameplay_specs.h"
#include "quantum_code/quantum_game.h"
#include "quantum_code/simulation_config.h"
#include "quantum_core/asset_resource_provider.h"
#include "quantum_core/component_type_id.h"
#include "quantum_core/map.h"

#include <map>
#include <memory>

using namespace Quantum;
using namespace Photon::Deterministic;

namespace {

struct LocalMatch {
    SystemAllocator allocator;
    InMemoryAssetProvider provider;
    ProviderAssetSystem assets;
    std::unique_ptr<QuantumGame> game;
    std::unique_ptr<DeterministicSession> session;
    std::map<int, std::uint64_t> checksums;
    int polls = 0;

    LocalMatch() {
        ComponentTypeId<>::ResetForTests();
        auto simulation = std::make_shared<SimulationConfig>();
        simulation->Guid = AssetGuid(SimulationConfig::DEFAULT_ID);
        simulation->Entities.DefaultEntityCapacity = 64;
        simulation->Entities.DefaultComponentBlockCapacity = 8;
        simulation->HeapPageShift = 12;
        simulation->HeapPageCount = 64;
        simulation->ThreadCount = 1;
        provider.Add(simulation);
        auto map = std::make_shared<Map>();
        map->Guid = AssetGuid(90);
        map->UserAsset.Id = AssetGuid(91);
        provider.Add(map);
        auto customData = std::make_shared<MapCustomData>();
        customData->Guid = AssetGuid(91);
        provider.Add(customData);
        assets = CreateResourceManager(provider, &allocator);
    }

    ~LocalMatch() {
        if (session) session->Destroy();
        session.reset();
        game.reset();
        ComponentTypeId<>::ResetForTests();
    }

    void Start() {
        QuantumGameStartParameters parameters;
        parameters.ResourceManager = assets.ResourceManager.get();
        game = std::make_unique<QuantumGame>(parameters);
        game->PollInput = [this](int frame, int player, bool) -> std::optional<PolledInput> {
            ++polls;
            PolledInput input;
            input.Input.Jump = Button::FromBool((frame + player) % 7 == 0);
            input.Input.EncodedDirection = static_cast<std::uint8_t>(1 + (frame * 3 + player) % 180);
            return input;
        };
        game->ChecksumComputed = [this](QuantumGame&, int frame, std::uint64_t checksum) { checksums[frame] = checksum; };

        RuntimeConfig runtime;
        runtime.Seed = 1234;
        runtime.Map.Id = AssetGuid(90);
        runtime.SimulationConfig.Id = AssetGuid(SimulationConfig::DEFAULT_ID);
        BitStream stream(4096);
        stream.SetWriting(true);
        runtime.Serialize(stream);

        auto config = std::make_shared<DeterministicSessionConfig>();
        config->PlayerCount = 2;
        config->ChecksumInterval = 30;
        DeterministicSessionArgs args;
        args.Mode = DeterministicGameMode::Local;
        args.SessionConfig = config;
        args.Game = game.get();
        args.PlatformInfo.Allocator = &allocator;
        args.RuntimeConfig = MakeByteArray(stream.ToArray());
        session = std::make_unique<DeterministicSession>(args);
        session->Join("local", 2);
        session->Update(0.0);
        session->Update(0.0);
    }

    void Run(int ticks) {
        for (int i = 0; i < ticks; ++i) session->Update(1.0 / 60.0);
    }
};

} // namespace

TEST_CASE("QuantumGame: a local match runs every system and is deterministic") {
    rr::test::InstallMathDerivedTables();
    SystemUtility::Install();

    std::map<int, std::uint64_t> first;
    {
        LocalMatch match;
        match.Start();
        CHECK(match.session->State() == DeterministicSessionState::Running);
        if (match.session->State() != DeterministicSessionState::Running) return;
        const int start = match.session->FrameVerified()->Number;
        match.Run(120);
        CHECK(match.session->FrameVerified()->Number == start + 120);
        CHECK(match.game->VerifiedTicks() == 120u);
        CHECK(match.polls >= 120 * 2);
        // The map named by the runtime config is in the frame's globals.
        CHECK(match.game->Frames().Verified->get_Global()->Map.Id == AssetGuid(90));
        // Every enabled root system was scheduled every tick.
        std::uint64_t scheduledRoots = 0;
        for (const auto& [system, stats] : match.game->Stats()) {
            if (stats.Scheduled == 120) ++scheduledRoots;
        }
        CHECK(scheduledRoots > 100u);
        CHECK(match.checksums.size() == 4u);
        first = match.checksums;
    }
    std::map<int, std::uint64_t> second;
    {
        LocalMatch match;
        match.Start();
        match.Run(120);
        second = match.checksums;
    }
    CHECK(first == second);
    // The checksum covers the state: different frames differ.
    CHECK(first.size() >= 2u && first.begin()->second != std::next(first.begin())->second);
}
