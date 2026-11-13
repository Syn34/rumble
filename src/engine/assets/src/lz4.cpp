#include "rr/assets/lz4.h"

#include "rr/assets/binary_reader.h"

#include <cstring>

namespace rr::assets {

namespace {

// Lengths are a 4-bit nibble; 15 means "add following bytes until one is < 255".
std::size_t ReadLength(std::span<const std::uint8_t> in, std::size_t& pos, std::size_t base) {
    std::size_t length = base;
    if (base == 15) {
        std::uint8_t byte = 0;
        do {
            if (pos >= in.size()) throw ReadError("lz4: truncated length");
            byte = in[pos++];
            length += byte;
        } while (byte == 255);
    }
    return length;
}

} // namespace

void Lz4DecompressBlock(std::span<const std::uint8_t> in, std::span<std::uint8_t> out) {
    std::size_t ip = 0;
    std::size_t op = 0;
    while (ip < in.size()) {
        const std::uint8_t token = in[ip++];

        // Literal run.
        const std::size_t literals = ReadLength(in, ip, token >> 4);
        if (literals > in.size() - ip || literals > out.size() - op) {
            throw ReadError("lz4: literal run out of bounds");
        }
        std::memcpy(out.data() + op, in.data() + ip, literals);
        ip += literals;
        op += literals;

        // The last sequence ends after its literals.
        if (ip == in.size()) break;

        // Match: 16-bit little-endian distance back into the output.
        if (in.size() - ip < 2) throw ReadError("lz4: truncated match offset");
        const std::size_t distance = std::size_t{in[ip]} | (std::size_t{in[ip + 1]} << 8);
        ip += 2;
        if (distance == 0 || distance > op) throw ReadError("lz4: invalid match offset");
        const std::size_t length = ReadLength(in, ip, token & 0x0F) + 4;
        if (length > out.size() - op) throw ReadError("lz4: match out of bounds");

        // Copy byte by byte: a match may overlap the bytes it is producing.
        const std::size_t from = op - distance;
        for (std::size_t i = 0; i < length; ++i) out[op + i] = out[from + i];
        op += length;
    }
    if (op != out.size()) throw ReadError("lz4: decompressed size mismatch");
}

} // namespace rr::assets
