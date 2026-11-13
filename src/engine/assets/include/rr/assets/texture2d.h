// texture2d.h - reader for Texture2D objects.
#pragma once

#include "rr/assets/streamed_data.h"
#include "rr/assets/texture_format.h"
#include "rr/assets/type_tree_value.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace rr::assets {

struct Texture2DAsset {
    std::string name;
    int width = 0;  // size of the first stored mip level
    int height = 0;
    std::int32_t format = 0; // TextureFormat value
    int mipCount = 1;        // stored mip levels
    int imageCount = 1;      // >1 for texture arrays / cube faces stored as one blob
    bool srgb = false;       // colour data authored in sRGB
    int filterMode = 1;      // 0 point, 1 bilinear, 2 trilinear
    int anisotropy = 1;
    int wrapU = 0;           // 0 repeat, 1 clamp, 2 mirror, 3 mirror once
    int wrapV = 0;
    std::vector<std::uint8_t> data; // every image's full mip chain, back to back

    int LevelWidth(int level) const { return width >> level > 0 ? width >> level : 1; }
    int LevelHeight(int level) const { return height >> level > 0 ? height >> level : 1; }
    // Bytes of one mip level of one image; empty if out of range.
    std::span<const std::uint8_t> Level(int level, int image = 0) const;
    // Decodes one mip level to RGBA8 (empty if the format isn't supported).
    std::vector<std::uint8_t> DecodeLevel(int level, int image = 0) const;
};

// Reads a decoded Texture2D object. Pixel data comes from the object or, when it
// is streamed, through `fetch`. Throws ReadError if the data is missing or short.
Texture2DAsset ReadTexture2D(const Value& texture, const ResourceFetcher& fetch);

} // namespace rr::assets
