// deterministic_platform_info.h - Photon.Deterministic.DeterministicPlatformInfo: what the
// host platform provides to the simulation (allocator, task runner) and describes about
// itself.
#pragma once

#include "native.h"

#include <string>

namespace Photon::Deterministic {

class IDeterministicPlatformTaskRunner;

struct DeterministicPlatformInfo {
    enum class Architectures : int { ARMv7 = 0, ARM64 = 1, x86 = 2 };
    enum class Platforms : int {
        Windows = 0,
        OSX = 1,
        Linux = 2,
        IOS = 3,
        Android = 4,
        XboxOne = 5,
        PlayStation4 = 6,
        Switch = 7,
        TVOS = 8,
    };
    enum class RuntimeHosts : int {
        Unity = 0,
        UnityEditor = 1,
        PhotonServer = 2,
        NetApplication = 3
    };
    enum class Runtimes : int { NetFramework = 0, NetCore = 1, Mono = 2, IL2CPP = 3 };

    Architectures Architecture = Architectures::ARMv7;
    Platforms Platform = Platforms::Windows;
    RuntimeHosts RuntimeHost = RuntimeHosts::Unity;
    Runtimes Runtime = Runtimes::NetFramework;
    // Not owned. Used for all frame input memory.
    Native::Allocator* Allocator = nullptr;
    int CoreCount = 0;
    // Not owned; may be null.
    IDeterministicPlatformTaskRunner* TaskRunner = nullptr;

    // 0x7a7a21c
    DeterministicPlatformInfo() = default;

    // 0x7a79dc8 - "Name: value" pairs of the descriptive fields, for the session's log.
    std::string ToString() const;
};

} // namespace Photon::Deterministic
