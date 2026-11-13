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

void NotReimplemented(const char* method) {
    throw std::logic_error(std::string(method) + " is not reimplemented yet");
}

} // namespace Quantum
