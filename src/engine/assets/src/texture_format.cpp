#include "rr/assets/texture_format.h"

#include "rr/assets/astc_decoder.h"

#include <algorithm>

namespace rr::assets {

TextureFormatInfo DescribeTextureFormat(std::int32_t format) {
    switch (static_cast<TextureFormat>(format)) {
    case TextureFormat::Alpha8: return {"Alpha8", 1, 1, 1, false};
    case TextureFormat::ARGB4444: return {"ARGB4444", 1, 1, 2, false};
    case TextureFormat::RGB24: return {"RGB24", 1, 1, 3, false};
    case TextureFormat::RGBA32: return {"RGBA32", 1, 1, 4, false};
    case TextureFormat::ARGB32: return {"ARGB32", 1, 1, 4, false};
    case TextureFormat::RGB565: return {"RGB565", 1, 1, 2, false};
    case TextureFormat::R16: return {"R16", 1, 1, 2, false};
    case TextureFormat::RGBA4444: return {"RGBA4444", 1, 1, 2, false};
    case TextureFormat::BGRA32: return {"BGRA32", 1, 1, 4, false};
    case TextureFormat::RG16: return {"RG16", 1, 1, 2, false};
    case TextureFormat::R8: return {"R8", 1, 1, 1, false};
    case TextureFormat::ASTC_4x4: return {"ASTC_4x4", 4, 4, 16, true};
    case TextureFormat::ASTC_5x5: return {"ASTC_5x5", 5, 5, 16, true};
    case TextureFormat::ASTC_6x6: return {"ASTC_6x6", 6, 6, 16, true};
    case TextureFormat::ASTC_8x8: return {"ASTC_8x8", 8, 8, 16, true};
    case TextureFormat::ASTC_10x10: return {"ASTC_10x10", 10, 10, 16, true};
    case TextureFormat::ASTC_12x12: return {"ASTC_12x12", 12, 12, 16, true};
    }
    return {};
}

std::size_t TextureLevelSize(std::int32_t format, int width, int height) {
    const TextureFormatInfo info = DescribeTextureFormat(format);
    if (!info.Supported() || width <= 0 || height <= 0) return 0;
    const auto bx = static_cast<std::size_t>((width + info.blockWidth - 1) / info.blockWidth);
    const auto by = static_cast<std::size_t>((height + info.blockHeight - 1) / info.blockHeight);
    return bx * by * static_cast<std::size_t>(info.bytesPerBlock);
}

std::size_t TextureChainSize(std::int32_t format, int width, int height, int mipCount) {
    std::size_t total = 0;
    for (int level = 0; level < mipCount; ++level) {
        total += TextureLevelSize(format, std::max(1, width >> level), std::max(1, height >> level));
    }
    return total;
}

namespace {

std::uint8_t Expand4(unsigned v) { return static_cast<std::uint8_t>((v & 0xF) * 0x11); }

} // namespace

std::vector<std::uint8_t> DecodeTextureToRgba8(std::int32_t format, std::span<const std::uint8_t> data,
                                               int width, int height, bool srgb) {
    const TextureFormatInfo info = DescribeTextureFormat(format);
    if (!info.Supported() || data.size() < TextureLevelSize(format, width, height)) return {};
    if (info.isAstc) {
        return astc::DecodeImage(data, width, height, info.blockWidth, info.blockHeight,
                                 srgb ? astc::Profile::Srgb : astc::Profile::Linear);
    }

    const auto count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<std::uint8_t> out(count * 4);
    const std::uint8_t* src = data.data();
    for (std::size_t i = 0; i < count; ++i) {
        std::uint8_t* d = &out[i * 4];
        switch (static_cast<TextureFormat>(format)) {
        case TextureFormat::Alpha8:
            d[0] = d[1] = d[2] = 255;
            d[3] = src[i];
            break;
        case TextureFormat::R8:
            d[0] = src[i];
            d[1] = d[2] = 0;
            d[3] = 255;
            break;
        case TextureFormat::RGB24:
            d[0] = src[i * 3];
            d[1] = src[i * 3 + 1];
            d[2] = src[i * 3 + 2];
            d[3] = 255;
            break;
        case TextureFormat::RGBA32:
            std::copy_n(src + i * 4, 4, d);
            break;
        case TextureFormat::ARGB32:
            d[0] = src[i * 4 + 1];
            d[1] = src[i * 4 + 2];
            d[2] = src[i * 4 + 3];
            d[3] = src[i * 4];
            break;
        case TextureFormat::BGRA32:
            d[0] = src[i * 4 + 2];
            d[1] = src[i * 4 + 1];
            d[2] = src[i * 4];
            d[3] = src[i * 4 + 3];
            break;
        case TextureFormat::RG16:
            d[0] = src[i * 2];
            d[1] = src[i * 2 + 1];
            d[2] = 0;
            d[3] = 255;
            break;
        case TextureFormat::R16: {
            const unsigned v = src[i * 2] | (src[i * 2 + 1] << 8);
            d[0] = static_cast<std::uint8_t>((v * 255 + 32767) / 65535);
            d[1] = d[2] = 0;
            d[3] = 255;
            break;
        }
        case TextureFormat::RGB565: {
            const unsigned v = src[i * 2] | (src[i * 2 + 1] << 8);
            d[0] = static_cast<std::uint8_t>((((v >> 11) & 0x1F) * 255 + 15) / 31);
            d[1] = static_cast<std::uint8_t>((((v >> 5) & 0x3F) * 255 + 31) / 63);
            d[2] = static_cast<std::uint8_t>(((v & 0x1F) * 255 + 15) / 31);
            d[3] = 255;
            break;
        }
        case TextureFormat::RGBA4444: { // 16-bit word, R in the top nibble
            const unsigned v = src[i * 2] | (src[i * 2 + 1] << 8);
            d[0] = Expand4(v >> 12);
            d[1] = Expand4(v >> 8);
            d[2] = Expand4(v >> 4);
            d[3] = Expand4(v);
            break;
        }
        case TextureFormat::ARGB4444: { // 16-bit word, A in the top nibble
            const unsigned v = src[i * 2] | (src[i * 2 + 1] << 8);
            d[3] = Expand4(v >> 12);
            d[0] = Expand4(v >> 8);
            d[1] = Expand4(v >> 4);
            d[2] = Expand4(v);
            break;
        }
        default: return {};
        }
    }
    return out;
}

} // namespace rr::assets
