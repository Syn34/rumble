#include "crc64.h"

#include <array>

namespace Photon::Deterministic {

namespace {

std::array<std::uint64_t, 256> BuildTable() {
    std::array<std::uint64_t, 256> table{};
    for (std::uint64_t i = 0; i < table.size(); ++i) {
        std::uint64_t value = i;
        for (int bit = 0; bit < 8; ++bit) {
            value = (value & 1) != 0 ? (value >> 1) ^ CRC64::Polynomial : value >> 1;
        }
        table[i] = value;
    }
    return table;
}

} // namespace

const std::uint64_t* CRC64::Table() {
    static const std::array<std::uint64_t, 256> table = BuildTable();
    return table.data();
}

std::uint64_t CRC64::Calculate(std::uint64_t crc, const void* data, int dataLength) {
    const std::uint64_t* table = Table();
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    for (int i = 0; i < dataLength; ++i) {
        crc = table[(crc ^ bytes[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

} // namespace Photon::Deterministic
