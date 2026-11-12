#include "deterministic_platform_info.h"

#include <array>

namespace Photon::Deterministic {

namespace {

const char* Name(DeterministicPlatformInfo::Architectures value) {
    static constexpr std::array names{"ARMv7", "ARM64", "x86"};
    const auto index = static_cast<std::size_t>(value);
    return index < names.size() ? names[index] : "?";
}

const char* Name(DeterministicPlatformInfo::Platforms value) {
    static constexpr std::array names{"Windows", "OSX",          "Linux",  "IOS", "Android",
                                      "XboxOne", "PlayStation4", "Switch", "TVOS"};
    const auto index = static_cast<std::size_t>(value);
    return index < names.size() ? names[index] : "?";
}

const char* Name(DeterministicPlatformInfo::RuntimeHosts value) {
    static constexpr std::array names{"Unity", "UnityEditor", "PhotonServer", "NetApplication"};
    const auto index = static_cast<std::size_t>(value);
    return index < names.size() ? names[index] : "?";
}

const char* Name(DeterministicPlatformInfo::Runtimes value) {
    static constexpr std::array names{"NetFramework", "NetCore", "Mono", "IL2CPP"};
    const auto index = static_cast<std::size_t>(value);
    return index < names.size() ? names[index] : "?";
}

} // namespace

std::string DeterministicPlatformInfo::ToString() const {
    // The original lists the same six fields; the allocator is shown by its type name
    // there, which has no equivalent here, so only its presence is reported.
    std::string out;
    out += "Architecture: ";
    out += Name(Architecture);
    out += ", Platform: ";
    out += Name(Platform);
    out += ", RuntimeHost: ";
    out += Name(RuntimeHost);
    out += ", Runtime: ";
    out += Name(Runtime);
    out += ", CoreCount: ";
    out += std::to_string(CoreCount);
    out += ", Allocator: ";
    out += Allocator != nullptr ? "set" : "null";
    return out;
}

} // namespace Photon::Deterministic
