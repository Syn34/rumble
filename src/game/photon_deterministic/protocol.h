// protocol.h - Photon.Deterministic.Protocol messages, as data.
//
// The session talks to its network layer exclusively through these messages. Only their
// fields (and Joined.IsConfirmed) are reconstructed here: the offline PC build passes them
// between DeterministicSession and DeterministicNetworkLocal in memory and never encodes
// them.
//
// TODO(Photon.Deterministic.Protocol.*): online-only; not used by the offline PC build.
// The wire format is not reimplemented: Protocol.Serializer (5 methods), every message's
// Serialize/ToString/Clone, FrameSnapshot.Encode/Decode and TickChecksumErrorFrameDump
// Encode/Decode/CopyBlock.
#pragma once

#include "deterministic_types.h"

#include <memory>
#include <string>
#include <vector>

namespace Photon::Deterministic {

class DeterministicSessionConfig;
struct DeterministicTickChecksumError;

namespace Protocol {

// Protocol.Message: base of every protocol message.
class Message {
  public:
    virtual ~Message() = default;
};

// Protocol.JoinStatus
enum class JoinStatus : int {
    Failed = 0,
    Confirmed = 1,
    WaitForSnapshot = 2,
};

// Client -> server: join request.
class Join final : public Message {
  public:
    std::string Id;
    std::string ProtocolVersion;
    int InitialTick = 0;
    int PlayerCount = 0;
    int PlayerSlots = 0;
    bool Rejoin = false;
};

// Server -> client: join result and the player slots assigned to this client.
class Joined final : public Message {
  public:
    JoinStatus Status = JoinStatus::Failed;
    bool Confirmed = false;
    std::vector<int> PlayerSlots;

    // 0x7a8f74c - Confirmed or WaitForSnapshot.
    bool IsConfirmed() const {
        return Status == JoinStatus::Confirmed || Status == JoinStatus::WaitForSnapshot;
    }
};

// Server -> client: start of the simulation with the authoritative configs.
class SimulationStart final : public Message {
  public:
    bool Reconnect = false;
    double ServerTime = 0.0;
    ByteArray RuntimeConfig;
    std::shared_ptr<DeterministicSessionConfig> SessionConfig;
};

class SimulationStop final : public Message {
  public:
    int FinalFrame = 0;
};

class RttUpdate final : public Message {
  public:
    int Rtt = 0;
};

class Disconnect final : public Message {
  public:
    std::string Reason;
};

// Server -> client asks for (Requested) or client -> server sends the runtime config.
class RuntimeConfig final : public Message {
  public:
    bool Requested = false;
    ByteArray Config;
};

class SessionConfig final : public Message {
  public:
    bool Requested = false;
    std::shared_ptr<DeterministicSessionConfig> Config;
};

class ClockCorrect final : public Message {
  public:
    double ServerSeconds = 0.0;
    double ServerTimeScale = 0.0;
};

class TickChecksum final : public Message {
  public:
    int Tick = 0;
    std::uint64_t Checksum = 0;
};

class TickChecksumError final : public Message {
  public:
    std::shared_ptr<DeterministicTickChecksumError> Error;
};

class TickChecksumErrorFrameDump final : public Message {
  public:
    int Frame = 0;
    int Block = 0;
    int BlockCount = 0;
    int ActorId = 0;
    ByteArray Data;
};

class FrameSnapshotRequest final : public Message {
  public:
    int ReferenceTick = 0;
};

class FrameSnapshot final : public Message {
  public:
    int Tick = 0;
    int TotalSize = 0;
    bool Last = false;
    ByteArray Data;
};

class Command final : public Message {
  public:
    int Index = 0;
    ByteArray Data;
};

class SetPlayerData final : public Message {
  public:
    int Index = 0;
    ByteArray Data;
};

class StartRequest final : public Message {};

class InputMissing final : public Message {};

} // namespace Protocol

} // namespace Photon::Deterministic
