// texture_format.h - pixel formats of the game's textures and decoders to RGBA8.
//
// Texture2D objects store their pixels in one of the engine's texture formats
// (the numeric values below are part of the file format). The Android build uses
// ASTC for nearly everything plus a few uncompressed formats; each is decoded
// here to plain RGBA8 so any renderer can use it. Renderers that support a
// compressed format natively can upload the original blocks instead.
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace rr::assets {

enum class TextureFormat : std::int32_t {
    Alpha8 = 1,
    ARGB4444 = 2,
    RGB24 = 3,
    RGBA32 = 4,
    ARGB32 = 5,
    RGB565 = 7,
    R16 = 9,
    RGBA4444 = 13,
    BGRA32 = 14,
    RG16 = 62,
    R8 = 63,
    ASTC_4x4 = 48,
    ASTC_5x5 = 49,
    ASTC_6x6 = 50,
    ASTC_8x8 = 51,
    ASTC_10x10 = 52,
    ASTC_12x12 = 53,
};

struct TextureFormatInfo {
    const char* name = "unknown";
    int blockWidth = 1;   // texels per block (1 for uncompressed formats)
    int blockHeight = 1;
    int bytesPerBlock = 0; // bytes per block, or per texel when uncompressed; 0 = unsupported
    bool isAstc = false;

    bool Supported() const { return bytesPerBlock > 0; }
};

// Describes a format; Supported() is false for formats this module can't decode.
TextureFormatInfo DescribeTextureFormat(std::int32_t format);

// Size in bytes of one mip level / of a full mip chain for one image.
std::size_t TextureLevelSize(std::int32_t format, int width, int height);
std::size_t TextureChainSize(std::int32_t format, int width, int height, int mipCount);

// Decodes one image (one mip level) to RGBA8, rows in the same order as the input.
// `srgb` selects the ASTC sRGB decode profile (colour endpoints expanded for 8-bit
// sRGB output); it doesn't change uncompressed formats. Returns an empty vector if
// the format is unsupported or the data is too short.
std::vector<std::uint8_t> DecodeTextureToRgba8(std::int32_t format, std::span<const std::uint8_t> data,
                                               int width, int height, bool srgb);

} // namespace rr::assets
