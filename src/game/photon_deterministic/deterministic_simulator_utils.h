// deterministic_simulator_utils.h - Photon.Deterministic.DeterministicSimulatorUtils:
// guarded calls into the game. An exception thrown by the game during a simulation step
// or checksum is logged and swallowed so one bad frame does not tear down the session.
#pragma once

#include <cstdint>
#include <memory>
#include <span>

namespace Photon::Deterministic {

class DeterministicFrame;
class IDeterministicGame;
struct DeterministicTickChecksumError;

class DeterministicSimulatorUtils final {
  public:
    DeterministicSimulatorUtils() = delete;

    // 0x7a7c7f8 - logs that the server detected a desync, then reports it to the game.
    static void CallChecksumError(IDeterministicGame& game,
                                  const DeterministicTickChecksumError& error,
                                  std::span<const std::shared_ptr<DeterministicFrame>> frames);
    // 0x7a7c33c - the frame's checksum, or 0 if computing it threw.
    static std::uint64_t CallChecksum(DeterministicFrame& f);
    // 0x7a7bc04
    static void CallSimulate(DeterministicFrame& f, IDeterministicGame& game);
    // 0x7a7bd58
    static void CallSimulateFinished(DeterministicFrame& f, IDeterministicGame& game);
    // 0x7a7c410
    static void CallChecksumComputed(IDeterministicGame& game, int frame, std::uint64_t checksum);
};

} // namespace Photon::Deterministic
