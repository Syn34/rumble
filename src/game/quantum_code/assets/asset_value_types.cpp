// Methods of the value types stored in the gameplay assets (quantum.code.dll).
#include "quantum_code/assets/level_assets.h"

#include <stdexcept>
#include <string>

namespace Quantum {

void NotReimplemented(const char* method) {
    throw std::logic_error(std::string(method) + " is not reimplemented yet");
}

} // namespace Quantum
