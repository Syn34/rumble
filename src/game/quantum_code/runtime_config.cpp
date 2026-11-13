// runtime_config.cpp - Quantum.RuntimeConfig.
#include "quantum_code/runtime_config.h"

#include <cstring>

namespace Quantum {

namespace {

using Photon::Deterministic::BitStream;

// BitStreamExtensions.Serialize(BitStream, ref AssetRefXxx): the GUID as a 64-bit value.
// One helper for all of them: 0x7c8d7c4, 0x7c8db0c, 0x7c8da64, 0x7c8d86c, 0x7c8d9bc,
// 0x7c8d914, 0x7c8d71c, 0x7c8d674.
template <class TAssetRef> void SerializeAssetRef(BitStream& stream, TAssetRef& ref) {
    stream.Serialize(ref.Id.Value);
}

// BitStreamUtility.SerializeStruct<T>: the struct's bytes as one buffer.
template <class T> void SerializeStruct(BitStream& stream, T& value) {
    stream.SerializeBuffer(reinterpret_cast<std::uint8_t*>(&value), static_cast<int>(sizeof(T)));
}

} // namespace

void RuntimeConfig::Serialize(BitStream& stream) {
    stream.Serialize(Seed);
    stream.Serialize(Map.Id.Value);
    stream.Serialize(SimulationConfig.Id.Value);
    SerializeUserData(stream);
}

void RuntimeConfig::SerializeUserData(BitStream& stream) {
    SerializeStruct(stream, gameModeSettings);
    stream.Serialize(waitForPlayersNeeded);
    stream.Serialize(isDebugRunnerSession);
    stream.Serialize(nbExpectedHumanPlayers);
    SerializeAssetRef(stream, gameManagerConfig);
    SerializeAssetRef(stream, simulationWeaponTable);
    SerializeAssetRef(stream, simulationUltimateTable);
    SerializeAssetRef(stream, simulationGadgetTable);
    SerializeAssetRef(stream, simulationMeleeWeaponTable);
    SerializeAssetRef(stream, simulationHeroesTable);
    SerializeAssetRef(stream, elementalDamageTable);
    stream.Serialize(modificator);
    stream.Serialize(FTUEStepId);
    stream.Serialize(sequenceId);
    stream.Serialize(skipPlayerPresentation);
    stream.Serialize(eventId);
    SerializeStruct(stream, eventGameModifier);
    SerializeAssetRef(stream, AIConfig);
    SerializeStruct(stream, BotsSettings);
    stream.Serialize(PveScaling);
    SerializeStruct(stream, BotsUnlockedFeatures);
}

std::vector<std::uint8_t> RuntimeConfig::ToByteArray(RuntimeConfig& config, int arrayCapacity) {
    BitStream stream(arrayCapacity);
    config.Serialize(stream);
    return stream.ToArray();
}

RuntimeConfig RuntimeConfig::FromByteArray(std::vector<std::uint8_t> data) {
    BitStream stream(data);
    stream.SetReading(true);
    RuntimeConfig config;
    config.Serialize(stream);
    return config;
}

bool RuntimeConfig::get_IsRealGame() const {
    if ((modificator | 2) == 3) {
        return false;
    }
    const auto mode = static_cast<std::int32_t>(gameModeSettings.gameMode);
    return mode != 6 && mode != 7;
}

bool RuntimeConfig::TryGetBotLoadoutPreset(int presetIndex, BotLoadoutPreset& preset) const {
    if (presetIndex < 0 || presetIndex >= BotsSettings.NumBotLoadoutPresets) {
        preset = BotLoadoutPreset{};
        return false;
    }
    const std::uint8_t* bytes = BotsSettings.BotLoadoutPresets.data() + presetIndex * 4;
    preset.character = bytes[0];
    preset.skin = bytes[1];
    preset.weapon1 = bytes[2];
    preset.weapon2 = bytes[3];
    preset.weapon3 = bytes[4];
    return true;
}

} // namespace Quantum
