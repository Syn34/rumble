#include "deterministic_simulator_utils.h"

#include "deterministic_frame.h"
#include "deterministic_interfaces.h"
#include "log.h"

#include <exception>

namespace Photon::Deterministic {

namespace {

void ReportGameException(const std::exception& e) {
    Log::Error("The game threw an exception");
    Log::Exception(e);
}

} // namespace

void DeterministicSimulatorUtils::CallChecksumError(
    IDeterministicGame& game, const DeterministicTickChecksumError& error,
    std::span<const std::shared_ptr<DeterministicFrame>> frames) {
    Log::Error("The server detected a checksum mismatch; the simulation is halting");
    try {
        game.OnChecksumError(error, frames);
    } catch (const std::exception& e) {
        Log::Error("The checksum error callback threw an exception");
        Log::Exception(e);
    }
}

std::uint64_t DeterministicSimulatorUtils::CallChecksum(DeterministicFrame& f) {
    try {
        return f.CalculateChecksum();
    } catch (const std::exception& e) {
        Log::Error("Calculating the frame checksum threw an exception");
        Log::Exception(e);
        return 0;
    }
}

void DeterministicSimulatorUtils::CallSimulate(DeterministicFrame& f, IDeterministicGame& game) {
    try {
        game.OnSimulate(f);
    } catch (const std::exception& e) {
        ReportGameException(e);
    }
}

void DeterministicSimulatorUtils::CallSimulateFinished(DeterministicFrame& f,
                                                       IDeterministicGame& game) {
    try {
        game.OnSimulateFinished(f);
    } catch (const std::exception& e) {
        ReportGameException(e);
    }
}

void DeterministicSimulatorUtils::CallChecksumComputed(IDeterministicGame& game, int frame,
                                                       std::uint64_t checksum) {
    try {
        game.OnChecksumComputed(frame, checksum);
    } catch (const std::exception& e) {
        ReportGameException(e);
    }
}

} // namespace Photon::Deterministic
