// match_runner.h - runs an offline match: the game (Quantum::QuantumGame) in a local
// DeterministicSession stepped at the fixed tick rate (PC glue in place of the original's
// Unity-side QuantumRunner).
#pragma once

#include "game_content.h"
#include "match_setup.h"

#include "photon_deterministic/deterministic_platform_info.h"
#include "photon_deterministic/deterministic_session.h"
#include "photon_deterministic/system_allocator.h"
#include "quantum_code/quantum_game.h"

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace rr::app {

class MatchRunner {
public:
    MatchRunner(GameContent& content, MatchSetup setup);
    ~MatchRunner();

    // Joins the local session and runs it until the simulation has started.
    bool Start(std::string& error);
    // One host frame of `seconds` (the session turns elapsed time into ticks).
    void Update(double seconds);
    // Runs exactly `ticks` more verified ticks.
    void RunTicks(int ticks);

    Quantum::QuantumGame& Game() { return *game_; }
    Photon::Deterministic::DeterministicSession& Session() { return *session_; }
    const MatchSetup& Setup() const { return setup_; }
    Quantum::Frame* Verified() const { return game_->Frames().Verified.get(); }
    Quantum::Frame* Predicted() const { return game_->Frames().Predicted.get(); }
    double TickSeconds() const { return 1.0 / setup_.session->UpdateFPS; }

    // The local player's input for each tick (slot 0); other slots get no input.
    std::function<std::optional<Quantum::PolledInput>(int frame)> LocalInput;

    // Checksums the session computed (every ChecksumInterval verified ticks).
    const std::map<int, std::uint64_t>& Checksums() const { return checksums_; }

    // Numbers only: entities per component type, systems scheduled and failing, events,
    // checksums.
    void PrintDiagnostics(std::FILE* out) const;

private:
    GameContent& content_;
    MatchSetup setup_;
    Photon::Deterministic::SystemAllocator allocator_;
    Photon::Deterministic::DeterministicPlatformInfo platform_;
    std::unique_ptr<Quantum::QuantumGame> game_;
    std::unique_ptr<Photon::Deterministic::DeterministicSession> session_;
    std::map<int, std::uint64_t> checksums_;
};

// A readable name for a system ("Quantum::HealthSystem").
std::string SystemName(const Quantum::SystemBase& system);

} // namespace rr::app
