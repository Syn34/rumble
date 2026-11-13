// Original: Quantum.Assert / Quantum.AssertException (quantum.core.dll).
#include "assert.h"

namespace Quantum {

AssertException::AssertException() : std::logic_error("assertion failed") {}

AssertException::AssertException(const std::string& message) : std::logic_error(message) {}

void Assert::Fail() { throw AssertException(); }

} // namespace Quantum
