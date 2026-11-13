// q_boolean.h - Quantum.QBoolean: a boolean stored as a 32-bit integer.
//
// Simulation state avoids `bool` fields so that struct layouts and serialisation are the
// same on every platform; QBoolean is the stand-in. Any non-zero value reads as true, but
// assigning a bool stores exactly 0 or 1.
#pragma once

#include <cstdint>
#include <string>

namespace Quantum {

class FrameSerializer;

struct QBoolean {
    static constexpr int SIZE = 4;

    std::int32_t Value = 0;

    constexpr QBoolean() = default;
    // 0x7d3d38c - bool -> QBoolean.
    constexpr QBoolean(bool value) : Value(value ? 1 : 0) {}

    // 0x7d3d380 - QBoolean -> bool.
    constexpr operator bool() const { return Value != 0; }

    // 0x7d3d4c8 (Equals(QBoolean)); 0x7d3d4d8 is the boxed Equals(object).
    constexpr bool Equals(QBoolean other) const { return Value == other.Value; }
    friend constexpr bool operator==(QBoolean a, QBoolean b) { return a.Equals(b); }

    // 0x7d3d550
    constexpr std::int32_t GetHashCode() const { return Value; }

    // 0x7d3d448 - Debug text: whether Value is exactly 1 (wording is this project's own).
    std::string ToString() const { return Value == 1 ? "true" : "false"; }

    // 0x7d3d394 - Value as a 32-bit integer.
    static void Serialize(void* ptr, FrameSerializer* serializer);
};

static_assert(sizeof(QBoolean) == QBoolean::SIZE);

} // namespace Quantum
