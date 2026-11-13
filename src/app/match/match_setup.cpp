#include "match_setup.h"

#include "quantum_code/assets/asset_value_types.h"
#include "quantum_code/assets/game_mode_specs.h"
#include "quantum_code/enums.h"
#include "quantum_code/input.h"
#include "quantum_code/simulation_config.h"
#include "quantum_core/map.h"

#include "photon_deterministic/bit_stream.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <utility>

namespace rr::app {

namespace {

using Quantum::GAME_MODE;

constexpr std::array<std::pair<const char*, GAME_MODE>, 11> kModeNames{{
    {"FREE_FOR_ALL", GAME_MODE::FREE_FOR_ALL},
    {"BLAST_BALL", GAME_MODE::BLAST_BALL},
    {"BOLT_KING", GAME_MODE::BOLT_KING},
    {"SQUAD_SIEGE", GAME_MODE::SQUAD_SIEGE},
    {"TUTORIAL", GAME_MODE::TUTORIAL},
    {"WEAPON_SHOWCASE", GAME_MODE::WEAPON_SHOWCASE},
    {"TRAINING", GAME_MODE::TRAINING},
    {"TEAM_TOTAL_RUMBLE", GAME_MODE::TEAM_TOTAL_RUMBLE},
    {"QWARK_BALL", GAME_MODE::QWARK_BALL},
    {"ZONE_CONTROL", GAME_MODE::ZONE_CONTROL},
    {"ELIMINATION", GAME_MODE::ELIMINATION},
}};

std::string Upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

const char* ModeName(GAME_MODE mode) {
    for (const auto& [name, value] : kModeNames) {
        if (value == mode) return name;
    }
    return "UNDEFINED";
}

// The maps' file names start with their mode (q_ffa_, q_bb_...). The PC build's default
// when no mode is given; the original takes it from the lobby.
GAME_MODE ModeFromMapName(const std::string& path) {
    std::string leaf = path.substr(path.find_last_of('/') == std::string::npos ? 0 : path.find_last_of('/') + 1);
    std::transform(leaf.begin(), leaf.end(), leaf.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const std::pair<const char*, GAME_MODE> prefixes[] = {
        {"q_ffa_", GAME_MODE::FREE_FOR_ALL},       {"q_bb_", GAME_MODE::BLAST_BALL},
        {"q_bk_", GAME_MODE::BOLT_KING},           {"q_el_", GAME_MODE::ELIMINATION},
        {"q_ttr_", GAME_MODE::TEAM_TOTAL_RUMBLE},  {"q_zc_", GAME_MODE::ZONE_CONTROL},
        {"q_pve_", GAME_MODE::SQUAD_SIEGE},        {"q_training", GAME_MODE::TRAINING},
        {"q_tuto", GAME_MODE::TUTORIAL},
    };
    for (const auto& [prefix, mode] : prefixes) {
        if (leaf.rfind(prefix, 0) == 0) return mode;
    }
    return GAME_MODE::FREE_FOR_ALL;
}

template <class TRef> void SetSingle(GameContent& content, const char* scriptClass, TRef& ref, std::vector<std::string>& notes) {
    const auto found = content.AssetsOfClass(scriptClass);
    if (found.size() == 1) {
        ref.Id = found.front().Guid;
    } else {
        notes.push_back(std::string(scriptClass) + ": " + (found.empty() ? "not in the staged content" : "more than one") +
                        "; left unset");
    }
}

} // namespace

std::vector<std::uint8_t> SerializeRuntimeConfig(Quantum::RuntimeConfig& config) {
    Photon::Deterministic::BitStream stream(64 * 1024);
    stream.SetWriting(true);
    config.Serialize(stream);
    return stream.ToArray();
}

std::optional<MatchSetup> BuildMatchSetup(GameContent& content, const MatchOptions& options, std::string& error) {
    MatchSetup setup;
    Quantum::RuntimeConfig& rc = setup.runtime;

    const auto map = content.Find("MapAsset", options.map);
    if (!map) {
        error = "map '" + options.map + "' not found (or ambiguous); maps:";
        for (const auto& id : content.AssetsOfClass("MapAsset")) error += "\n  " + id.Path + "  (" + std::to_string(id.Guid.Value) + ")";
        return std::nullopt;
    }
    rc.Map.Id = map->Guid;
    setup.mapPath = map->Path;
    rc.Seed = options.seed;

    const auto simulation = content.AssetsOfClass("SimulationConfigAsset");
    rc.SimulationConfig.Id = simulation.empty() ? Quantum::AssetGuid(Quantum::SimulationConfig::DEFAULT_ID)
                                                : simulation.front().Guid;

    // Game mode: a name, a number or a GUID from the game modes table.
    GAME_MODE mode = ModeFromMapName(map->Path);
    if (!options.gameMode.empty()) {
        bool resolved = false;
        const std::string upper = Upper(options.gameMode);
        for (const auto& [name, value] : kModeNames) {
            if (upper == name) {
                mode = value;
                resolved = true;
            }
        }
        if (!resolved && std::all_of(options.gameMode.begin(), options.gameMode.end(),
                                     [](unsigned char c) { return std::isdigit(c) != 0 || c == '-'; })) {
            const long long number = std::stoll(options.gameMode);
            if (number > 0 && number < static_cast<long long>(GAME_MODE::COUNT)) {
                mode = static_cast<GAME_MODE>(number);
                resolved = true;
            } else {
                for (const auto& tableId : content.AssetsOfClass("GameModesTableAsset")) {
                    auto table = std::dynamic_pointer_cast<Quantum::GameModesTable>(content.Load(tableId.Guid));
                    if (!table) continue;
                    for (const auto& entry : table->prototypes) {
                        if (entry.spec.Id.Value == number || entry.prototype.Id.Value == number) {
                            mode = entry.id;
                            resolved = true;
                        }
                    }
                }
            }
        }
        if (!resolved) {
            error = "game mode '" + options.gameMode + "' not recognised (a GAME_MODE name or number, or a game mode GUID)";
            return std::nullopt;
        }
    }
    rc.gameModeSettings.gameMode = mode;
    setup.gameModeName = ModeName(mode);

    // Tables and configs that exist once in the content.
    SetSingle(content, "GameManagerConfigAsset", rc.gameManagerConfig, setup.notes);
    SetSingle(content, "SimulationWeaponTableAsset", rc.simulationWeaponTable, setup.notes);
    SetSingle(content, "SimulationUltimateTableAsset", rc.simulationUltimateTable, setup.notes);
    SetSingle(content, "SimulationGadgetTableAsset", rc.simulationGadgetTable, setup.notes);
    SetSingle(content, "SimulationMeleeWeaponsTableAsset", rc.simulationMeleeWeaponTable, setup.notes);
    SetSingle(content, "SimulationHeroesTableAsset", rc.simulationHeroesTable, setup.notes);
    SetSingle(content, "ElementalDamageTableAsset", rc.elementalDamageTable, setup.notes);
    SetSingle(content, "AIBotSDKConfigAsset", rc.AIConfig, setup.notes);

    // Offline: nobody to wait for.
    rc.waitForPlayersNeeded = false;
    rc.nbExpectedHumanPlayers = 1;
    rc.skipPlayerPresentation = true;

    if (!options.hero.empty()) {
        if (auto hero = content.Find("HeroSpecAsset", options.hero)) {
            setup.heroPath = hero->Path;
        } else {
            error = "hero '" + options.hero + "' not found";
            return std::nullopt;
        }
    }
    // TODO: the hero travels in the player's RuntimePlayer (characterIndex, CharacterPrototype),
    // which reaches the frame through Frame.UpdatePlayerData; neither is reimplemented yet.
    setup.notes.push_back("player data (RuntimePlayer, hero choice) is not passed to the simulation yet");

    auto session = std::make_shared<Photon::Deterministic::DeterministicSessionConfig>();
    session->PlayerCount = std::clamp(options.players, 1, Quantum::Input::MAX_COUNT);
    session->UpdateFPS = 60;
    session->ChecksumInterval = 60;
    session->RollbackWindow = 60;
    session->InputDelayMin = 0;
    session->LockstepSimulation = false;
    setup.session = session;
    setup.notes.push_back("DeterministicSessionConfig: class defaults (Resources deterministicconfig is not decoded)");
    return setup;
}

} // namespace rr::app
