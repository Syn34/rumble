// deterministic_session_config.h - Photon.Deterministic.DeterministicSessionConfig.
//
// Timing and networking parameters of a session: tick rate, rollback window, input delay,
// checksum interval and clock-correction tuning. A client proposes its local config; the
// server (or, offline, DeterministicNetworkLocal) sends back the one the simulation runs
// with. The object is shared by reference between the session and those messages.
#pragma once

#include "deterministic_types.h"

#include <memory>
#include <span>
#include <vector>

namespace Photon::Deterministic {

class BitStream;

namespace Protocol {
class Serializer;
}

class DeterministicSessionConfig {
  public:
    // Backwards-compatibility fields kept in the serialised layout.
    bool _BW_COMPAT_RunInBackgroundThread = false;
    // System.Threading.ThreadPriority (2 = Normal).
    int _BW_COMPAT_BackgroundThreadPriority = 2;
    bool _BW_COMPAT_ExposeVerifiedStatusInsideSimulation = false;
    bool _BW_COMPAT_SkipRollbackWhenPossible = false;
    int _BW_COMPAT_InputPacking = 2;

    int PlayerCount = 0;
    bool ChecksumCrossPlatformDeterminism = false;
    bool LockstepSimulation = false;
    bool AggressiveSendMode = false;
    // Simulation ticks per second.
    int UpdateFPS = 60;
    // Verified frames between checksums (0 or less disables checksums).
    int ChecksumInterval = 60;
    // How many frames prediction may run ahead of the last verified frame.
    int RollbackWindow = 60;
    int InputHardTolerance = 8;
    int InputRedundancy = 3;
    // How many frames back a player's last confirmed input may be repeated as prediction.
    int InputRepeatMaxDistance = 10;
    int SessionStartTimeout = 1;
    int TimeCorrectionRate = 4;
    int MinTimeCorrectionFrames = 1;
    int MinOffsetCorrectionDiff = 1;
    int TimeScaleMin = 100;
    int TimeScalePingMin = 100;
    int TimeScalePingMax = 300;
    // Frames between polling a local input and simulating it.
    int InputDelayMin = 0;
    int InputDelayMax = 60;
    // Round trip (ms) above which extra input delay is added.
    int InputDelayPingStart = 100;
    bool InputFixedSizeEnabled = false;
    int InputFixedSize = 0;

    // 0x7a83604 - the defaults above.
    DeterministicSessionConfig() = default;

    // 0x7a7e968 - serialises a config (may be null) into a fresh byte array.
    static std::vector<std::uint8_t>
    ToByteArray(const std::shared_ptr<DeterministicSessionConfig>& instance);
    // 0x7a7f378 - the inverse; null if the data encodes a null config.
    static std::shared_ptr<DeterministicSessionConfig>
    FromByteArray(std::span<const std::uint8_t> data);
    // 0x7a83284 - reads or writes `config` through `stream`. When reading, `config` is
    // replaced by a new instance (or reset for a null config). The protocol serializer
    // argument is unused by this method and may be null.
    static void Serialize(Protocol::Serializer* serializer, BitStream& stream,
                          std::shared_ptr<DeterministicSessionConfig>& config);
};

class DeterministicSessionConfigUtils final {
  public:
    DeterministicSessionConfigUtils() = delete;

    // 0x7a8324c - true unless fixed-size input is enabled, `data` is non-empty and its
    // length differs from InputFixedSize.
    static bool VerifyFixedSize(const DeterministicSessionConfig& config, const ByteArray& data,
                                int dataLength);
};

} // namespace Photon::Deterministic
