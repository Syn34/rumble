// deterministic_tick_input.h - Photon.Deterministic.DeterministicTickInput and friends.
//
// A DeterministicTickInput is one player's serialised input for one tick as it travels
// over the network (or out of a replay): tick, player, input bytes, optional rpc, flags.
// Instances are recycled through DeterministicTickInput::Pool. Also here: the checksum
// error records the server sends when clients desync.
#pragma once

#include "deterministic_types.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace Photon::Deterministic {

class BitStream;

namespace Protocol {
class Serializer;
class TickChecksumErrorFrameDump;
} // namespace Protocol

struct DeterministicTickInputEncodeHeader;

class DeterministicTickInput {
  public:
    // Thrown when a serialised input is malformed.
    class CorruptedException : public std::runtime_error {
      public:
        // 0x7a7d210
        explicit CorruptedException(const std::string& message) : std::runtime_error(message) {}
    };

    // DeterministicTickInput.Pool: a thread-safe free list.
    class Pool {
      public:
        // 0x7a7d628
        Pool() = default;
        // 0x7a7d350 - a recycled instance if available, otherwise a new one.
        std::shared_ptr<DeterministicTickInput> Acquire();
        // 0x7a7d4cc - resets the instance (tick, player, data length, rpc, link, sent flag,
        // data bytes; Flags are kept) and stores it for reuse.
        void Release(const std::shared_ptr<DeterministicTickInput>& input);

      private:
        std::mutex sync_;
        std::vector<std::shared_ptr<DeterministicTickInput>> pool_;
    };

    bool Sent = false;
    bool Pooled = false;
    std::shared_ptr<DeterministicTickInput> Next;
    int Tick = 0;
    int PlayerIndex = 0;
    ByteArray Rpc;
    // Holds at least DataLength bytes (may be larger when reused).
    ByteArray DataArray;
    int DataLength = 0;
    DeterministicInputFlags Flags = DeterministicInputFlags::None;

    // 0x7a7d348
    DeterministicTickInput() = default;

    // 0x7a7ca7c - copies `length` bytes into DataArray, growing it if needed.
    void CopyToDataArray(std::span<const std::uint8_t> array, int length);
    // 0x7a7d054 - player (8 bits), tick (32), data, rpc (or a single clear bit when
    // `ignoreRpc`), flags (8). Reading a set rpc bit with `ignoreRpc` throws
    // CorruptedException. The protocol serializer is not used and may be null.
    void SimpleSerialize(Protocol::Serializer* serializer, BitStream& stream, bool ignoreRpc);
    // TODO(0x7a7cb1c, Photon.Deterministic.DeterministicTickInput.SerializeForDecoder):
    // online-only; not used by the offline PC build. Delta-encodes an input against the
    // tick header for the server's input stream.
    void SerializeForDecoder(Protocol::Serializer* serializer, BitStream& stream,
                             const DeterministicTickInputEncodeHeader& header);
    // 0x7a7d2b4
    std::string ToString() const;
};

// Photon.Deterministic.DeterministicTickInputEncodeHeader: per-packet header of the
// server's input stream.
struct DeterministicTickInputEncodeHeader {
    int MaxPing = 0;
    int PlayerCount = 0;
    int InputFixedSize = 0;
    double ServerTime = 0.0;
    double ServerTimeScale = 0.0;
    bool InputFixedSizeEnabled = false;

    // TODO(0x7a84478, Photon.Deterministic.DeterministicTickInputEncodeHeader.Serialize):
    // online-only; not used by the offline PC build.
    void Serialize(BitStream& stream);
};

// Photon.Deterministic.DeterministicTickInputSet
struct DeterministicTickInputSet {
    int Tick = 0;
    std::vector<std::shared_ptr<DeterministicTickInput>> Inputs;
};

// Photon.Deterministic.DeterministicChecksumResult: one client's checksum for a tick.
struct DeterministicChecksumResult {
    std::uint8_t Client = 0;
    std::uint64_t Checksum = 0;
};

// Photon.Deterministic.DeterministicTickChecksumError (ctor 0x7a83758).
struct DeterministicTickChecksumError {
    int Tick = 0;
    std::vector<DeterministicChecksumResult> Checksums;
};

// Photon.Deterministic.ChecksumErrorFrameDump (ctor 0x7a7f270): a frame dump being
// reassembled from blocks.
struct ChecksumErrorFrameDump {
    int Frame = 0;
    std::vector<std::shared_ptr<Protocol::TickChecksumErrorFrameDump>> Blocks;
};

} // namespace Photon::Deterministic
