#include "deterministic_session_config.h"

#include "bit_stream.h"

namespace Photon::Deterministic {

std::vector<std::uint8_t> DeterministicSessionConfig::ToByteArray(
    const std::shared_ptr<DeterministicSessionConfig>& instance) {
    // The original also creates a Protocol.Serializer for the current protocol version,
    // which Serialize does not use.
    BitStream stream(0x2000);
    stream.SetWriting(true);
    std::shared_ptr<DeterministicSessionConfig> config = instance;
    Serialize(nullptr, stream, config);
    return stream.ToArray();
}

std::shared_ptr<DeterministicSessionConfig>
DeterministicSessionConfig::FromByteArray(std::span<const std::uint8_t> data) {
    std::vector<std::uint8_t> copy(data.begin(), data.end());
    BitStream stream(copy);
    stream.SetWriting(false);
    std::shared_ptr<DeterministicSessionConfig> config;
    Serialize(nullptr, stream, config);
    return config;
}

void DeterministicSessionConfig::Serialize(Protocol::Serializer*, BitStream& stream,
                                           std::shared_ptr<DeterministicSessionConfig>& config) {
    // Null marker, then the thread priority as a raw 32-bit value.
    if (stream.Writing()) {
        stream.WriteBool(config == nullptr);
        if (!config) {
            return;
        }
        stream.WriteUInt(static_cast<std::uint32_t>(config->_BW_COMPAT_BackgroundThreadPriority),
                         32);
    } else {
        if (stream.ReadBool()) {
            config.reset();
            return;
        }
        config = std::make_shared<DeterministicSessionConfig>();
        config->_BW_COMPAT_BackgroundThreadPriority = static_cast<int>(stream.ReadUInt(32));
    }

    DeterministicSessionConfig& c = *config;
    stream.Serialize(c.PlayerCount, 32);
    stream.Serialize(c._BW_COMPAT_SkipRollbackWhenPossible);
    stream.Serialize(c._BW_COMPAT_RunInBackgroundThread);
    stream.Serialize(c._BW_COMPAT_ExposeVerifiedStatusInsideSimulation);
    stream.Serialize(c.LockstepSimulation);
    stream.Serialize(c.AggressiveSendMode);
    stream.Serialize(c.InputDelayMin, 32);
    stream.Serialize(c.InputDelayMax, 32);
    stream.Serialize(c.InputDelayPingStart, 32);
    stream.Serialize(c.UpdateFPS, 32);
    stream.Serialize(c.ChecksumInterval, 32);
    stream.Serialize(c.RollbackWindow, 32);
    stream.Serialize(c._BW_COMPAT_InputPacking, 32);
    stream.Serialize(c.InputHardTolerance, 32);
    stream.Serialize(c.InputRedundancy, 32);
    stream.Serialize(c.InputRepeatMaxDistance, 32);
    stream.Serialize(c.SessionStartTimeout, 32);
    stream.Serialize(c.TimeCorrectionRate, 32);
    stream.Serialize(c.MinTimeCorrectionFrames, 32);
    stream.Serialize(c.MinOffsetCorrectionDiff, 32);
    stream.Serialize(c.TimeScaleMin, 32);
    stream.Serialize(c.TimeScalePingMin, 32);
    stream.Serialize(c.TimeScalePingMax, 32);
    stream.Serialize(c.ChecksumCrossPlatformDeterminism);
    stream.Serialize(c.InputFixedSizeEnabled);
    stream.Serialize(c.InputFixedSize, 32);
}

bool DeterministicSessionConfigUtils::VerifyFixedSize(const DeterministicSessionConfig& config,
                                                      const ByteArray& data, int dataLength) {
    if (!data || !config.InputFixedSizeEnabled || data->empty()) {
        return true;
    }
    return config.InputFixedSize == dataLength;
}

} // namespace Photon::Deterministic
