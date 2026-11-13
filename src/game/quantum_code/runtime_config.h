// runtime_config.h - Quantum.RuntimeConfig (quantum.code.dll): the per-match settings the
// client hands to the simulation (map, game mode, bots, unlocked features...).
//
// It travels as bytes (ToByteArray / FromByteArray) and must produce the same simulation
// on every peer, so its serialisation is part of the deterministic setup.
#pragma once

#include "quantum_code/components.h"
#include "quantum_core/map.h"

#include "photon_deterministic/bit_stream.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace Quantum {

// Values of the event game modifier (RuntimeConfig.EVENT_GAME_MODIFIER_ID).
enum class EVENT_GAME_MODIFIER_ID : std::int32_t {
    None = 0,
    DropTokens = 1,
    RandomWeapon = 2,
};

// RuntimeConfig.EventGameModifier
struct alignas(4) EventGameModifier {
    EVENT_GAME_MODIFIER_ID modifier;
    std::int32_t value;
};
static_assert(sizeof(EventGameModifier) == 8);

// Quantum.BotLoadoutPreset
struct BotLoadoutPreset {
    std::int32_t character = 0;
    std::int32_t skin = 0;
    std::int32_t weapon1 = 0;
    std::int32_t weapon2 = 0;
    std::int32_t weapon3 = 0;
};

class RuntimeConfig {
  public:
    // 0x7ca3440 - PveScaling starts at 1, everything else zero.
    RuntimeConfig() = default;

    std::int32_t Seed = 0;
    AssetRefMap Map;
    AssetRefSimulationConfig SimulationConfig{};
    GameModeSettings gameModeSettings{};
    bool waitForPlayersNeeded = false;
    bool isDebugRunnerSession = false;
    std::uint32_t nbExpectedHumanPlayers = 0;
    AssetRefGameManagerConfig gameManagerConfig{};
    AssetRefSimulationWeaponTable simulationWeaponTable{};
    AssetRefSimulationUltimateTable simulationUltimateTable{};
    AssetRefSimulationGadgetTable simulationGadgetTable{};
    AssetRefSimulationMeleeWeaponsTable simulationMeleeWeaponTable{};
    AssetRefSimulationHeroesTable simulationHeroesTable{};
    AssetRefElementalDamageTable elementalDamageTable{};
    std::int32_t modificator = 0;
    std::int32_t FTUEStepId = 0;
    std::optional<std::string> sequenceId;
    bool skipPlayerPresentation = false;
    std::int32_t eventId = 0;
    EventGameModifier eventGameModifier{};
    AssetRefAIBotSDKConfig AIConfig{};
    ::Quantum::BotsSettings BotsSettings{};
    UnlockedFeatures BotsUnlockedFeatures{};
    FP PveScaling = FP::One();
    // Not kept: HeroImprovementSimulationTable, a client-side table (List<HeroImprovementData>)
    // that is not serialised; its type is not reimplemented yet.

    // 0x7ca30a4 - Seed, map and simulation config, then the user data.
    void Serialize(Photon::Deterministic::BitStream& stream);
    // 0x7ca3100 - The game's own settings, in declaration order; structs travel as raw
    // bytes.
    void SerializeUserData(Photon::Deterministic::BitStream& stream);
    // 0x7ca32dc - Serialised into a stream of `arrayCapacity` bytes.
    static std::vector<std::uint8_t> ToByteArray(RuntimeConfig& config, int arrayCapacity);
    // 0x7ca3390
    static RuntimeConfig FromByteArray(std::vector<std::uint8_t> data);

    // 0x7ca3450 - modificator 1.
    bool get_IsTutorial() const { return modificator == 1; }
    // 0x7ca3460
    std::int32_t get_FTUEStep() const { return FTUEStepId; }
    // 0x7ca3468 - A step other than 0 (none) and 9 (done).
    bool get_IsInFTUE() const { return FTUEStepId != 0 && FTUEStepId != 9; }
    // 0x7ca347c - Neither tutorial nor training (modificator 1 or 3), and not game modes
    // 6 or 7.
    bool get_IsRealGame() const;
    // 0x7ca34a8 - modificator 3.
    bool get_IsTraining() const { return modificator == 3; }
    // 0x7ca34b8
    bool get_IsOffline() const { return !waitForPlayersNeeded; }
    // 0x7ca34c8
    bool HasGameModifier(EVENT_GAME_MODIFIER_ID modifier) const { return eventGameModifier.modifier == modifier; }
    // 0x7ca34d8
    std::uint8_t get_BotsDifficulty_0_100() const { return BotsSettings.BotsDifficulty_0_100; }
    // 0x7ca34e0 - Preset `presetIndex` from the packed preset bytes (4 bytes apart; the
    // fifth value is the next preset's first byte, as in the original). False and a
    // zeroed preset when the index is out of range.
    bool TryGetBotLoadoutPreset(int presetIndex, BotLoadoutPreset& preset) const;
};

} // namespace Quantum
