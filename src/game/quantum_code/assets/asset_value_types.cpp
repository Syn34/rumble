// Methods of the value types stored in the gameplay assets (quantum.code.dll).
#include "quantum_code/assets/level_assets.h"

#include "photon_deterministic/fp_math.h"

#include <stdexcept>
#include <string>

namespace Quantum {

std::int32_t DifficultyIntervalInt::Get(FP difficulty_0_1) const {
    return Photon::Deterministic::FPMath::CeilToInt(
        Photon::Deterministic::FPMath::Lerp(FP::FromInt(atMinDiff), FP::FromInt(atMaxDiff), difficulty_0_1));
}

std::int32_t GameModesTableEntry::GetHashCode() const {
    std::uint32_t hash = static_cast<std::uint32_t>(id);
    hash = hash * 31u + static_cast<std::uint32_t>(prototype.GetHashCode());
    hash = hash * 31u + static_cast<std::uint32_t>(spec.GetHashCode());
    hash = hash * 31u + static_cast<std::uint32_t>(status);
    return static_cast<std::int32_t>(hash + 0x5446907Bu); // the generator's seed, times 31^4
}

void NotReimplemented(const char* method) {
    throw std::logic_error(std::string(method) + " is not reimplemented yet");
}

} // namespace Quantum
