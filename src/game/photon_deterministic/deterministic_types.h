// deterministic_types.h - small shared vocabulary of the PhotonDeterministic session runtime.
//
// The original is managed code, so byte arrays, configs and frames are garbage-collected
// references that several objects can point at, and some code compares them by identity
// (a deserialised command remembers the exact rpc array it came from). Those references
// are modelled with std::shared_ptr; a null pointer stands for a null reference.
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

namespace Photon::Deterministic {

// A nullable, shareable byte[].
using ByteArray = std::shared_ptr<std::vector<std::uint8_t>>;

inline ByteArray MakeByteArray(std::vector<std::uint8_t> bytes = {}) {
    return std::make_shared<std::vector<std::uint8_t>>(std::move(bytes));
}

// Photon.Deterministic.DeterministicInputFlags ([Flags] byte enum).
enum class DeterministicInputFlags : std::uint8_t {
    None = 0,
    // The input may be repeated to predict later frames of the same player.
    Repeatable = 1,
    PlayerNotPresent = 2,
    ReplacedByServer = 4,
    // The rpc slot of this input carries a serialised DeterministicCommand.
    Command = 8,
};

constexpr DeterministicInputFlags operator|(DeterministicInputFlags a, DeterministicInputFlags b) {
    return static_cast<DeterministicInputFlags>(static_cast<std::uint8_t>(a) |
                                                static_cast<std::uint8_t>(b));
}
constexpr DeterministicInputFlags operator&(DeterministicInputFlags a, DeterministicInputFlags b) {
    return static_cast<DeterministicInputFlags>(static_cast<std::uint8_t>(a) &
                                                static_cast<std::uint8_t>(b));
}
constexpr DeterministicInputFlags operator~(DeterministicInputFlags a) {
    return static_cast<DeterministicInputFlags>(
        static_cast<std::uint8_t>(~static_cast<unsigned>(a)));
}
constexpr bool HasFlag(DeterministicInputFlags value, DeterministicInputFlags flag) {
    return (value & flag) != DeterministicInputFlags::None;
}

// Photon.Deterministic.DeterministicGameMode.
enum class DeterministicGameMode : int {
    Multiplayer = 0,
    Local = 1,
    Replay = 2,
    Spectating = 3,
};

// Photon.Deterministic.DeterministicSessionState.
enum class DeterministicSessionState : int {
    Idle = 0,
    Joined = 1,
    Running = 2,
    Shutdown = 3,
    Destroyed = 4,
};

// Stand-in for System.IDisposable (frame contexts and session runners) until mscorlib
// gets its own reimplementation.
class IDisposable {
  public:
    virtual ~IDisposable() = default;
    virtual void Dispose() = 0;
};

namespace detail {

// The original converts doubles to int with the IL2CPP conversion: saturating truncation,
// NaN gives 0, and +infinity is special-cased to int.MinValue.
inline int DoubleToInt32(double value) {
    if (value == std::numeric_limits<double>::infinity()) {
        return std::numeric_limits<int>::min();
    }
    if (std::isnan(value)) {
        return 0;
    }
    if (value <= static_cast<double>(std::numeric_limits<int>::min())) {
        return std::numeric_limits<int>::min();
    }
    if (value >= static_cast<double>(std::numeric_limits<int>::max())) {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>(value);
}

// (int)Math.Ceiling(value) with the same conversion rules.
inline int CeilingToInt32(double value) {
    return DoubleToInt32(std::ceil(value));
}

} // namespace detail

} // namespace Photon::Deterministic
