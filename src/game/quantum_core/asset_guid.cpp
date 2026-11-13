// Original: Quantum.AssetGuid / Quantum.AssetRef (quantum.core.dll).
#include "asset_guid.h"

#include "photon_deterministic/bit_stream.h"
#include "photon_deterministic/deterministic_frame_serializer.h"

#include <cstdio>
#include <random>

namespace Quantum {

// 0x7ce0508
AssetGuid AssetGuid::NewGuid() {
    // The original draws 8 random bytes from a shared, non-deterministic random
    // generator; dynamic GUIDs never feed into the simulation's own randomness,
    // so any good source of randomness preserves the behaviour.
    static std::mt19937_64 generator{std::random_device{}()};
    std::uint64_t value = 0;
    // Values 1..255 are re-rolled: they're kept free for built-in assets.
    do {
        value = generator();
    } while (value >= 1 && value <= 255);
    // Clear the dynamic and reserved bits; callers set the dynamic bit themselves.
    return AssetGuid(static_cast<std::int64_t>(value & 0x3FFFFFFFFFFFFFFFull));
}

void AssetGuid::Serialize(void* ptr, Photon::Deterministic::IDeterministicFrameSerializer* serializer) {
    serializer->Stream().Serialize(&static_cast<AssetGuid*>(ptr)->Value);
}

std::int32_t AssetGuid::GetHashCode() const {
    const auto bits = static_cast<std::uint64_t>(Value);
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(bits) ^
                                     static_cast<std::uint32_t>(bits >> 32));
}

std::string AssetGuid::ToString(bool brackets) const {
    char buffer[24];
    std::snprintf(buffer, sizeof(buffer), brackets ? "[%016llX]" : "%016llX",
                  static_cast<unsigned long long>(static_cast<std::uint64_t>(Value)));
    return buffer;
}

std::string AssetRef::ToString(AssetGuid id) { return "Id: " + id.ToString(true); }

} // namespace Quantum
