// astc_decoder.h - software decoder for ASTC LDR textures (2D blocks).
//
// Written from the Khronos ASTC specification (Khronos Data Format Specification,
// "ASTC Compressed Texture Image Formats"). Every 128-bit block is decoded on its
// own: block mode and weight grid, partitioning, colour endpoint modes, the
// integer sequence encoding (bits / trits / quints), weight infill and dual-plane
// weights. HDR endpoint modes and HDR void-extent blocks are not part of the LDR
// profile and decode to the error colour (magenta), as the specification requires.
#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace rr::assets::astc {

inline constexpr int kBlockBytes = 16;

enum class Profile {
    Linear, // LDR, 8-bit UNORM output
    Srgb,   // LDR with sRGB endpoint expansion; output bytes are sRGB-encoded
};

// Decodes one block of blockWidth x blockHeight texels (4..12 each) into `out`
// (blockWidth * blockHeight * 4 bytes, RGBA8, row-major). Returns false if the
// block is invalid; `out` then holds the error colour.
bool DecodeBlock(const std::uint8_t* block, int blockWidth, int blockHeight, Profile profile,
                 std::uint8_t* out);

// Decodes a whole image. `data` holds ceil(w/bw) * ceil(h/bh) blocks, row by row.
// Returns an empty vector if `data` is too short or the block size is unsupported.
std::vector<std::uint8_t> DecodeImage(std::span<const std::uint8_t> data, int width, int height,
                                      int blockWidth, int blockHeight, Profile profile);

} // namespace rr::assets::astc
