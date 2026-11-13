// lz4.h - decoder for the LZ4 block format (the compression used by the game's
// content bundles). Implemented from the public format description.
#pragma once

#include <cstdint>
#include <span>

namespace rr::assets {

// Decompresses one LZ4 block into `out`, which must be exactly the decompressed
// size. Throws ReadError on malformed input or a size mismatch.
void Lz4DecompressBlock(std::span<const std::uint8_t> in, std::span<std::uint8_t> out);

} // namespace rr::assets
