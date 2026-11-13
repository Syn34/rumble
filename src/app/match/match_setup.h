// match_setup.h - the configs of an offline match (PC glue standing in for the client flow
// that fills them on Android: lobby, matchmaking and the game manager).
#pragma once

#include "game_content.h"

#include "photon_deterministic/deterministic_session_config.h"
#include "quantum_code/runtime_config.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::app {

struct MatchOptions {
    std::string map;       // map asset name/path or GUID
    std::string gameMode;  // GAME_MODE name/number, or the GUID of a game mode spec/prototype
    std::string hero;      // hero spec name/path or GUID (recorded; see MatchSetup::notes)
    int players = 1;       // player slots (all local; slot 0 gets the user's input)
    int seed = 1;
};

struct MatchSetup {
    Quantum::RuntimeConfig runtime;
    std::shared_ptr<Photon::Deterministic::DeterministicSessionConfig> session;
    std::string mapPath;
    std::string gameModeName;
    std::string heroPath;
    std::vector<std::string> notes; // what couldn't be set from the staged content
};

// Null (and `error` set) if the map can't be found.
std::optional<MatchSetup> BuildMatchSetup(GameContent& content, const MatchOptions& options, std::string& error);

// The runtime config as the session carries it (serialised with a writing stream).
std::vector<std::uint8_t> SerializeRuntimeConfig(Quantum::RuntimeConfig& config);

} // namespace rr::app
