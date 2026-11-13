#include "match_runner.h"

#include "quantum_core/component_type_id.h"

#include <algorithm>
#include <cinttypes>
#include <typeinfo>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace rr::app {

using Photon::Deterministic::DeterministicGameMode;
using Photon::Deterministic::DeterministicSessionArgs;
using Photon::Deterministic::DeterministicSessionState;

std::string SystemName(const Quantum::SystemBase& system) {
    const char* mangled = typeid(system).name();
#if defined(__GNUC__) || defined(__clang__)
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    if (status == 0 && demangled != nullptr) {
        std::string name(demangled);
        std::free(demangled);
        return name;
    }
#endif
    return mangled;
}

MatchRunner::MatchRunner(GameContent& content, MatchSetup setup) : content_(content), setup_(std::move(setup)) {
    platform_.Allocator = &allocator_;
    platform_.Platform = Photon::Deterministic::DeterministicPlatformInfo::Platforms::Linux;
    platform_.Runtime = Photon::Deterministic::DeterministicPlatformInfo::Runtimes::NetCore;
    platform_.RuntimeHost = Photon::Deterministic::DeterministicPlatformInfo::RuntimeHosts::NetApplication;
    platform_.CoreCount = 1;
}

MatchRunner::~MatchRunner() {
    if (session_) session_->Destroy();
    session_.reset();
    game_.reset();
}

bool MatchRunner::Start(std::string& error) {
    Quantum::QuantumGameStartParameters parameters;
    parameters.ResourceManager = &content_.ResourceManager();
    game_ = std::make_unique<Quantum::QuantumGame>(parameters);
    game_->PollInput = [this](int frame, int player, bool) -> std::optional<Quantum::PolledInput> {
        if (player != 0 || !LocalInput) return std::nullopt;
        return LocalInput(frame);
    };
    game_->ChecksumComputed = [this](Quantum::QuantumGame&, int frame, std::uint64_t checksum) {
        checksums_[frame] = checksum;
    };

    DeterministicSessionArgs args;
    args.Mode = DeterministicGameMode::Local;
    args.SessionConfig = setup_.session;
    args.Game = game_.get();
    args.PlatformInfo = platform_;
    args.RuntimeConfig = Photon::Deterministic::MakeByteArray(SerializeRuntimeConfig(setup_.runtime));
    try {
        session_ = std::make_unique<Photon::Deterministic::DeterministicSession>(args);
        session_->Join("local", setup_.session->PlayerCount);
        for (int i = 0; i < 4 && session_->State() != DeterministicSessionState::Running; ++i) session_->Update(0.0);
    } catch (const std::exception& e) {
        error = std::string("session start failed: ") + e.what();
        return false;
    }
    if (session_->State() != DeterministicSessionState::Running) {
        error = "the local session did not start";
        return false;
    }
    return true;
}

void MatchRunner::Update(double seconds) { session_->Update(seconds); }

void MatchRunner::RunTicks(int ticks) {
    const int target = session_->FrameVerified()->Number + ticks;
    // Exactly one tick of time per update; a few extra updates guard against rounding.
    for (int guard = 0; session_->FrameVerified()->Number < target && guard < ticks * 2 + 4; ++guard) {
        session_->Update(TickSeconds());
    }
}

void MatchRunner::PrintDiagnostics(std::FILE* out) const {
    const Quantum::Frame* f = Verified();
    if (f == nullptr) return;
    std::fprintf(out, "frame %d (verified ticks simulated: %" PRIu64 ")\n", f->Number, game_->VerifiedTicks());
    std::fprintf(out, "entities: %d\n", f->EntityCount());
    std::fprintf(out, "components:\n");
    for (int type = 1; type < Quantum::ComponentTypeId<>::Count(); ++type) {
        const auto* buffer = f->GetDataBuffer(type);
        const int count = buffer->get_UsedCount();
        if (count > 0) std::fprintf(out, "  %6d  %s\n", count, Quantum::ComponentTypeId<>::GetComponentName(type).c_str());
    }

    int enabled = 0, scheduledRoots = 0, failing = 0;
    for (const Quantum::SystemBase* system : game_->SystemsAll()) {
        if (f->SystemIsEnabledInHierarchy(system)) ++enabled;
    }
    std::vector<std::pair<std::string, const Quantum::QuantumGame::SystemStats*>> failures;
    for (const auto& [system, stats] : game_->Stats()) {
        if (stats.Scheduled > 0) ++scheduledRoots;
        if (stats.Exceptions > 0) {
            ++failing;
            failures.emplace_back(SystemName(*system), &stats);
        }
    }
    std::fprintf(out, "systems: %zu (%zu roots), %d enabled, %d roots scheduled, %d threw\n", game_->SystemsAll().size(),
                 game_->SystemsRoot().size(), enabled, scheduledRoots, failing);
    std::sort(failures.begin(), failures.end());
    for (const auto& [name, stats] : failures) {
        std::fprintf(out, "  threw %" PRIu64 "x  %s  (%s)\n", stats->Exceptions, name.c_str(), stats->LastException.c_str());
    }
    std::uint64_t events = 0;
    for (const auto& [id, count] : game_->EventCounts()) events += count;
    std::fprintf(out, "events raised: %" PRIu64 " (%zu kinds)\n", events, game_->EventCounts().size());
    std::fprintf(out, "checksums:\n");
    for (const auto& [frame, checksum] : checksums_) std::fprintf(out, "  %6d  %016" PRIx64 "\n", frame, checksum);
}

} // namespace rr::app
