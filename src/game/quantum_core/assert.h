// assert.h - Quantum.Assert and Quantum.AssertException (quantum.core.dll).
//
// Always-on assertions of the simulation framework: unlike Debug asserts they are kept in
// release builds and throw instead of aborting, so a broken invariant surfaces as an
// exception the session can report.
#pragma once

#include <stdexcept>
#include <string>

namespace Quantum {

class AssertException : public std::logic_error {
  public:
    // 0x7d3e428 - no message.
    AssertException();
    // 0x7d3e480
    explicit AssertException(const std::string& message);
};

struct Assert {
    // 0x7d3e4e8 - throws unconditionally.
    [[noreturn]] static void Fail();
    // 0x7d3e51c - throws AssertException when the condition is false.
    static void Always(bool condition) {
        if (!condition) {
            Fail();
        }
    }
    // 0x7d3e558 - the same with a message.
    static void Always(bool condition, const std::string& message) {
        if (!condition) {
            throw AssertException(message);
        }
    }
};

} // namespace Quantum
