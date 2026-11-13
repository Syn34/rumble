// crc64.h - Photon.Deterministic.CRC64: the 64-bit CRC used for frame checksums.
//
// A table-driven, reflected (least significant bit first) CRC: for every input byte the
// low byte of the running value selects a table entry, which is XORed into the value
// shifted right by eight. There is no initial value or final XOR inside Calculate; the
// caller passes the running checksum and gets the updated one back, so data can be fed
// in pieces.
//
// The table belongs to the reflected polynomial 0x95AC9329AC4BC9B5 (the bit-reversed
// form of 0xAD93D23594C935A9, "CRC-64/Jones"). The original ships it as a precomputed
// array; here it is generated from the polynomial on first use, which gives the same
// 256 entries.
#pragma once

#include <cstdint>

namespace Photon::Deterministic {

class CRC64 final {
  public:
    CRC64() = delete;

    // Reflected generator polynomial of the table.
    static constexpr std::uint64_t Polynomial = 0x95AC9329AC4BC9B5ull;

    // 0x7a87fac - Feeds `dataLength` bytes at `data` into the running value `crc`.
    // Nothing is read for a length of zero or less.
    static std::uint64_t Calculate(std::uint64_t crc, const void* data, int dataLength);

    // 0x7a88064 (.cctor: builds crc64_tab) - The 256-entry lookup table.
    static const std::uint64_t* Table();
};

} // namespace Photon::Deterministic
