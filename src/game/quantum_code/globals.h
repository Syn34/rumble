// globals.h - Quantum._globals_ (quantum.code.dll): the per-frame global state.
//
// One block of native memory per frame (Frame::AllocGen), copied with the frame. It holds
// the frame-wide simulation state that is not component data: delta times, the RNG, the
// frame metadata, the physics engine states and settings, the system enable bits and the
// latest input of every player.
#pragma once

#include "quantum_code/components.h"
#include "quantum_code/input.h"
#include "quantum_core/frame_meta_data.h"
#include "quantum_core/map.h"
#include "quantum_core/navigation/navmesh_region_mask.h"

#include <array>
#include <cstdint>
#include <span>

namespace Quantum {

// Same size and alignment as quantum.core's PhysicsEngineState and PhysicsSceneSettings,
// whose reimplementation belongs to the physics engine work; until then the frame only
// keeps their bytes.
struct alignas(8) PhysicsEngineStateStorage {
    std::array<std::uint8_t, 16> Bytes;
};
struct alignas(8) PhysicsSceneSettingsStorage {
    std::array<std::uint8_t, 296> Bytes;
};

struct alignas(8) _globals_ {
    static constexpr int SIZE = 3816;
    static constexpr int ALIGNMENT = 8;

    std::int32_t nPlayerDataReceived;
    QBoolean AI_BotsInactive;
    QBoolean cheat_BotsInactive;
    QBoolean followPlayerCheatActive;
    AssetRefMap Map;
    BitSet32 PlayerLastConnectionState;
    FP DeltaTime;
    FP UnscaledDeltaTime;
    NavMeshRegionMask NavMeshRegions;
    PhysicsEngineStateStorage PhysicsState2D;
    PhysicsEngineStateStorage PhysicsState3D;
    RNGSession RngSession;
    ::Quantum::BotSDKData BotSDKData;
    FPVector3 followPlayerPos;
    ::Quantum::FrameMetaData FrameMetaData;
    std::array<Input, Input::MAX_COUNT> _input_;
    BitSet1024 Systems;
    PhysicsSceneSettingsStorage PhysicsSettings;

    // 0x7b9ea0c - The input of every player slot.
    std::span<Input> get_input() { return _input_; }
    std::span<const Input> get_input() const { return _input_; }

    // TODO(0x7b9ea74, Quantum._globals_.GetHashCode): needs the GetHashCode of quantum.core's
    // PhysicsEngineState and PhysicsSceneSettings.
    // TODO(0x7b9ed9c, Quantum._globals_.Serialize): fields in layout order; needs the
    // serialisers of PhysicsEngineState and PhysicsSceneSettings (physics engine work).
};
static_assert(sizeof(_globals_) == _globals_::SIZE);

} // namespace Quantum
