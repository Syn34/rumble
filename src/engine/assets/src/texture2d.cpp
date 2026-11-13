#include "rr/assets/texture2d.h"

#include <algorithm>

namespace rr::assets {

namespace {

std::int64_t IntField(const Value& v, const char* name, std::int64_t fallback = 0) {
    const Value* f = v.Get(name);
    return f ? f->AsInt(fallback) : fallback;
}

} // namespace

std::span<const std::uint8_t> Texture2DAsset::Level(int level, int image) const {
    if (level < 0 || level >= mipCount || image < 0 || image >= imageCount) return {};
    const std::size_t chain = TextureChainSize(format, width, height, mipCount);
    std::size_t offset = chain * static_cast<std::size_t>(image);
    offset += TextureChainSize(format, width, height, level);
    const std::size_t size = TextureLevelSize(format, LevelWidth(level), LevelHeight(level));
    if (size == 0 || offset + size > data.size()) return {};
    return std::span<const std::uint8_t>(data).subspan(offset, size);
}

std::vector<std::uint8_t> Texture2DAsset::DecodeLevel(int level, int image) const {
    const auto bytes = Level(level, image);
    if (bytes.empty()) return {};
    return DecodeTextureToRgba8(format, bytes, LevelWidth(level), LevelHeight(level), srgb);
}

Texture2DAsset ReadTexture2D(const Value& texture, const ResourceFetcher& fetch) {
    Texture2DAsset out;
    if (const Value* name = texture.Get("m_Name"); name && name->AsString()) out.name = *name->AsString();
    out.width = static_cast<int>(IntField(texture, "m_Width"));
    out.height = static_cast<int>(IntField(texture, "m_Height"));
    out.format = static_cast<std::int32_t>(IntField(texture, "m_TextureFormat"));
    out.mipCount = std::max(1, static_cast<int>(IntField(texture, "m_MipCount", 1)));
    out.imageCount = std::max(1, static_cast<int>(IntField(texture, "m_ImageCount", 1)));
    out.srgb = IntField(texture, "m_ColorSpace") == 1;
    if (const Value* settings = texture.Get("m_TextureSettings")) {
        out.filterMode = static_cast<int>(IntField(*settings, "m_FilterMode", 1));
        out.anisotropy = static_cast<int>(IntField(*settings, "m_Aniso", 1));
        out.wrapU = static_cast<int>(IntField(*settings, "m_WrapU"));
        out.wrapV = static_cast<int>(IntField(*settings, "m_WrapV"));
    }
    // Stripped top mips: the stored chain starts that many levels down.
    if (const auto stripped = static_cast<int>(IntField(texture, "m_MipsStripped")); stripped > 0) {
        out.width = std::max(1, out.width >> stripped);
        out.height = std::max(1, out.height >> stripped);
        out.mipCount = std::max(1, out.mipCount - stripped);
    }
    if (out.width <= 0 || out.height <= 0) throw ReadError("texture " + out.name + ": bad size");

    out.data = ValueBytes(texture.Get("image data"));
    if (out.data.empty()) {
        const auto stream = ReadStreamingInfo(texture.Get("m_StreamData"));
        if (stream && !stream->Empty()) {
            if (!fetch) throw ReadError("texture " + out.name + ": streamed data needs a fetcher");
            out.data = fetch(*stream);
        }
    }
    if (out.data.empty()) throw ReadError("texture " + out.name + ": no pixel data");

    // Trust the data over the header if the chain doesn't fit (keep whole levels only).
    if (DescribeTextureFormat(out.format).Supported()) {
        const std::size_t perImage = out.data.size() / static_cast<std::size_t>(out.imageCount);
        while (out.mipCount > 1 && TextureChainSize(out.format, out.width, out.height, out.mipCount) > perImage) {
            --out.mipCount;
        }
        if (TextureChainSize(out.format, out.width, out.height, 1) > perImage) {
            throw ReadError("texture " + out.name + ": pixel data shorter than the first mip level");
        }
    }
    return out;
}

} // namespace rr::assets
