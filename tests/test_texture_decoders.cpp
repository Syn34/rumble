// Texture decoders and the Texture2D reader, on hand-built data.
#include "test_framework.h"

#include "rr/assets/astc_decoder.h"
#include "rr/assets/texture2d.h"
#include "rr/assets/texture_format.h"

#include <array>
#include <cstring>
#include <memory>
#include <vector>

using namespace rr::assets;

namespace {

// Sets `count` bits of a 128-bit block, LSB first, starting at bit `start`.
void PutBits(std::array<std::uint8_t, 16>& block, int start, int count, std::uint32_t value) {
    for (int i = 0; i < count; ++i) {
        const int bit = start + i;
        if ((value >> i) & 1) {
            block[static_cast<std::size_t>(bit / 8)] |= static_cast<std::uint8_t>(1 << (bit % 8));
        } else {
            block[static_cast<std::size_t>(bit / 8)] &= static_cast<std::uint8_t>(~(1 << (bit % 8)));
        }
    }
}

std::array<std::uint8_t, 16> VoidExtentBlock(std::uint16_t r, std::uint16_t g, std::uint16_t b, std::uint16_t a,
                                             bool hdr = false) {
    std::array<std::uint8_t, 16> block{};
    PutBits(block, 0, 9, 0x1FC);   // void-extent marker
    PutBits(block, 9, 1, hdr ? 1 : 0);
    PutBits(block, 10, 2, 3);      // reserved, must be set
    for (int i = 0; i < 4; ++i) PutBits(block, 12 + 13 * i, 13, 0x1FFF); // no extent
    PutBits(block, 64, 16, r);
    PutBits(block, 80, 16, g);
    PutBits(block, 96, 16, b);
    PutBits(block, 112, 16, a);
    return block;
}

bool AllTexels(const std::vector<std::uint8_t>& texels, std::array<std::uint8_t, 4> expected) {
    for (std::size_t i = 0; i < texels.size(); i += 4) {
        if (std::memcmp(&texels[i], expected.data(), 4) != 0) return false;
    }
    return !texels.empty();
}

ValuePtr Leaf(std::int64_t v) {
    auto p = std::make_shared<Value>();
    p->data = v;
    return p;
}

ValuePtr Text(std::string s) {
    auto p = std::make_shared<Value>();
    p->data = std::move(s);
    return p;
}

ValuePtr Struct(std::vector<Value::Field> fields) {
    auto p = std::make_shared<Value>();
    p->data = Value::Struct(std::move(fields));
    return p;
}

} // namespace

TEST_CASE("astc: void-extent block is a solid colour") {
    const auto block = VoidExtentBlock(0xFFFF, 0x8000, 0x0000, 0xFFFF);
    std::vector<std::uint8_t> out(6 * 6 * 4);
    CHECK(astc::DecodeBlock(block.data(), 6, 6, astc::Profile::Linear, out.data()));
    CHECK(AllTexels(out, {255, 128, 0, 255}));
    CHECK(astc::DecodeBlock(block.data(), 6, 6, astc::Profile::Srgb, out.data()));
    CHECK(AllTexels(out, {255, 128, 0, 255}));

    const auto dim = VoidExtentBlock(0x0101 * 7, 0x0101 * 200, 0x1234, 0x0101 * 9);
    std::vector<std::uint8_t> small(4 * 4 * 4);
    CHECK(astc::DecodeBlock(dim.data(), 4, 4, astc::Profile::Linear, small.data()));
    CHECK(AllTexels(small, {7, 200, 18, 9}));
    CHECK(astc::DecodeBlock(dim.data(), 4, 4, astc::Profile::Srgb, small.data()));
    CHECK(AllTexels(small, {7, 200, 0x12, 9})); // sRGB colour keeps the top byte; alpha stays linear
}

TEST_CASE("astc: HDR void-extent and reserved block modes give the error colour") {
    const auto hdr = VoidExtentBlock(0x3C00, 0x3C00, 0x3C00, 0x3C00, true);
    std::vector<std::uint8_t> out(8 * 8 * 4);
    CHECK(!astc::DecodeBlock(hdr.data(), 8, 8, astc::Profile::Linear, out.data()));
    CHECK(AllTexels(out, {255, 0, 255, 255}));

    std::array<std::uint8_t, 16> reserved{}; // block mode with all low four bits clear
    CHECK(!astc::DecodeBlock(reserved.data(), 4, 4, astc::Profile::Linear, out.data()));
    CHECK(out[0] == 255 && out[1] == 0 && out[2] == 255 && out[3] == 255);
}

TEST_CASE("astc: single-partition RGB block with a 4x4 weight grid") {
    // Block mode 0x42: 4x4 weights, 4 levels (2 bits each), one plane.
    std::array<std::uint8_t, 16> block{};
    PutBits(block, 0, 11, 0x42);
    PutBits(block, 11, 2, 0); // one partition
    PutBits(block, 13, 4, 8); // CEM 8: RGB, direct
    // 6 endpoint values fit at 256 levels: plain bytes r0 r1 g0 g1 b0 b1.
    const std::uint32_t values[6] = {10, 250, 20, 200, 30, 100};
    for (int i = 0; i < 6; ++i) PutBits(block, 17 + 8 * i, 8, values[i]);
    // Weights from the top of the block, bit-reversed: weight i bit k at 127 - (2i + k).
    for (int i = 0; i < 16; ++i) {
        const int w = i % 4; // column index -> weights 0, 21, 43, 64
        for (int k = 0; k < 2; ++k) PutBits(block, 127 - (2 * i + k), 1, (w >> k) & 1);
    }

    std::vector<std::uint8_t> out(4 * 4 * 4);
    CHECK(astc::DecodeBlock(block.data(), 4, 4, astc::Profile::Linear, out.data()));
    const std::uint8_t expected[4][3] = {{10, 20, 30}, {89, 79, 53}, {171, 141, 77}, {250, 200, 100}};
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            const std::uint8_t* t = &out[static_cast<std::size_t>((y * 4 + x) * 4)];
            CHECK(t[0] == expected[x][0] && t[1] == expected[x][1] && t[2] == expected[x][2] && t[3] == 255);
        }
    }

    // A 4x4 weight grid stretched over an 8x8 block: corners keep the corner weights.
    std::vector<std::uint8_t> big(8 * 8 * 4);
    CHECK(astc::DecodeBlock(block.data(), 8, 8, astc::Profile::Linear, big.data()));
    CHECK(big[0] == 10 && big[1] == 20 && big[2] == 30);
    CHECK(big[7 * 4] == 250 && big[7 * 4 + 1] == 200 && big[7 * 4 + 2] == 100);
}

TEST_CASE("astc: trit-encoded weights") {
    // Block mode for a 4x4 grid at 6 levels (one trit + one bit per weight): R = 6.
    // Layout (mode & 3) != 0, case 0: W = B + 4, H = A + 2, R0 = bit 4, R2:R1 = bits 1:0.
    // R = 6 = 0b110 -> R0 = 0, R1 = 1, R2 = 1 -> bits 1:0 = 0b11; A = 2 (H = 4); B = 0 (W = 4).
    const std::uint32_t mode = 0b11u | (2u << 5);
    std::array<std::uint8_t, 16> block{};
    PutBits(block, 0, 11, mode);
    PutBits(block, 13, 4, 0);                // CEM 0: luminance, direct
    // 16 weights * (1 bit) + ceil(16 * 8 / 5) = 42 weight bits; endpoints get the rest.
    // Two endpoint values at the finest fit (256 levels): 0 and 255.
    PutBits(block, 17, 8, 0);
    PutBits(block, 25, 8, 255);
    // First trit group (weights 0-4): bits m0, T1:0, m1, T3:2, m2, T4, m3, T6:5, m4, T7.
    // T = 0x25 decodes to trits (1, 1, 0, 1, 0). Low bits m = 0, 1, 0, 1, 1.
    std::uint32_t stream = 0;
    int pos = 0;
    auto put = [&](std::uint32_t v, int n) {
        stream |= v << pos;
        pos += n;
    };
    const std::uint32_t t = 0x25;
    put(0, 1);
    put(t & 3, 2);
    put(1, 1);
    put((t >> 2) & 3, 2);
    put(0, 1);
    put((t >> 4) & 1, 1);
    put(1, 1);
    put((t >> 5) & 3, 2);
    put(1, 1);
    put((t >> 7) & 1, 1);
    for (int i = 0; i < pos; ++i) PutBits(block, 127 - i, 1, (stream >> i) & 1);

    std::vector<std::uint8_t> out(4 * 4 * 4);
    CHECK(astc::DecodeBlock(block.data(), 4, 4, astc::Profile::Linear, out.data()));
    // Weights (trit, bit) -> 0..64: (1,0)=12, (1,1)=52, (0,0)=0, (1,1)=52, (0,1)=64.
    // Luminance 0..255 interpolated: 255 * 257 * w / 64, then to 8 bits.
    auto lum = [](int w) { return ((255 * 257 * w + 32) / 64 * 255 + 32767) / 65535; };
    const int weights[5] = {12, 52, 0, 52, 64};
    for (int i = 0; i < 5; ++i) CHECK(out[static_cast<std::size_t>(i * 4)] == lum(weights[i]));
}

TEST_CASE("astc: image decode handles partial edge blocks") {
    const auto red = VoidExtentBlock(0xFFFF, 0, 0, 0xFFFF);
    const auto blue = VoidExtentBlock(0, 0, 0xFFFF, 0xFFFF);
    // 10x5 texels with 8x8 blocks: two blocks side by side.
    std::vector<std::uint8_t> data(red.begin(), red.end());
    data.insert(data.end(), blue.begin(), blue.end());
    const auto image = astc::DecodeImage(data, 10, 5, 8, 8, astc::Profile::Linear);
    CHECK(image.size() == 10 * 5 * 4);
    CHECK(image[0] == 255 && image[2] == 0);
    CHECK(image[(4 * 10 + 7) * 4] == 255);
    CHECK(image[(4 * 10 + 8) * 4] == 0 && image[(4 * 10 + 8) * 4 + 2] == 255);
    CHECK(astc::DecodeImage(std::span(data).first(16), 10, 5, 8, 8, astc::Profile::Linear).empty());
}

TEST_CASE("texture formats: sizes and uncompressed decoders") {
    CHECK(TextureLevelSize(static_cast<std::int32_t>(TextureFormat::ASTC_8x8), 100, 60) == 13 * 8 * 16);
    CHECK(TextureLevelSize(static_cast<std::int32_t>(TextureFormat::RGB24), 3, 2) == 18);
    // 20x12 RGBA32 with mips 20x12, 10x6, 5x3, 2x1, 1x1.
    CHECK(TextureChainSize(static_cast<std::int32_t>(TextureFormat::RGBA32), 20, 12, 5) ==
          (240 + 60 + 15 + 2 + 1) * 4);
    CHECK(!DescribeTextureFormat(9999).Supported());

    const std::vector<std::uint8_t> rgb = {1, 2, 3, 4, 5, 6};
    auto out = DecodeTextureToRgba8(static_cast<std::int32_t>(TextureFormat::RGB24), rgb, 2, 1, false);
    CHECK((out == std::vector<std::uint8_t>{1, 2, 3, 255, 4, 5, 6, 255}));

    const std::vector<std::uint8_t> argb = {9, 1, 2, 3};
    out = DecodeTextureToRgba8(static_cast<std::int32_t>(TextureFormat::ARGB32), argb, 1, 1, false);
    CHECK((out == std::vector<std::uint8_t>{1, 2, 3, 9}));

    const std::vector<std::uint8_t> rgba4444 = {0x4F, 0x8C}; // word 0x8C4F: R=8 G=C B=4 A=F
    out = DecodeTextureToRgba8(static_cast<std::int32_t>(TextureFormat::RGBA4444), rgba4444, 1, 1, false);
    CHECK((out == std::vector<std::uint8_t>{0x88, 0xCC, 0x44, 0xFF}));

    const std::vector<std::uint8_t> rgb565 = {0x1F, 0xF8}; // word 0xF81F: R=31 G=0 B=31
    out = DecodeTextureToRgba8(static_cast<std::int32_t>(TextureFormat::RGB565), rgb565, 1, 1, false);
    CHECK((out == std::vector<std::uint8_t>{255, 0, 255, 255}));

    CHECK(DecodeTextureToRgba8(static_cast<std::int32_t>(TextureFormat::RGBA32), rgb, 2, 1, false).empty());
}

TEST_CASE("texture2d: reader loads streamed mips and decodes them") {
    // 8x4 RGBA32 texture with 3 mips (8x4, 4x2, 2x1), streamed from a resource blob.
    std::vector<std::uint8_t> blob(5, 0xEE); // unrelated leading bytes
    const std::size_t chain = (32 + 8 + 2) * 4;
    for (std::size_t i = 0; i < chain; ++i) blob.push_back(static_cast<std::uint8_t>(i));

    auto stream = Struct({{"offset", Leaf(5)}, {"size", Leaf(static_cast<std::int64_t>(chain))},
                          {"path", Text("archive:/CAB-test/CAB-test.resS")}});
    auto settings = Struct({{"m_FilterMode", Leaf(2)}, {"m_Aniso", Leaf(4)}, {"m_WrapU", Leaf(1)}, {"m_WrapV", Leaf(0)}});
    auto texture = Struct({
        {"m_Name", Text("synthetic")},
        {"m_Width", Leaf(8)},
        {"m_Height", Leaf(4)},
        {"m_TextureFormat", Leaf(static_cast<std::int64_t>(TextureFormat::RGBA32))},
        {"m_MipCount", Leaf(3)},
        {"m_ImageCount", Leaf(1)},
        {"m_ColorSpace", Leaf(1)},
        {"m_TextureSettings", settings},
        {"image data", std::make_shared<Value>()},
        {"m_StreamData", stream},
    });

    int fetches = 0;
    const ResourceFetcher fetch = [&](const StreamingInfo& info) {
        ++fetches;
        CHECK(info.path == "archive:/CAB-test/CAB-test.resS");
        return std::vector<std::uint8_t>(blob.begin() + static_cast<std::ptrdiff_t>(info.offset),
                                         blob.begin() + static_cast<std::ptrdiff_t>(info.offset + info.size));
    };
    const Texture2DAsset t = ReadTexture2D(*texture, fetch);
    CHECK(fetches == 1);
    CHECK(t.name == "synthetic" && t.width == 8 && t.height == 4 && t.mipCount == 3 && t.srgb);
    CHECK(t.filterMode == 2 && t.anisotropy == 4 && t.wrapU == 1 && t.wrapV == 0);
    CHECK(t.Level(0).size() == 128 && t.Level(1).size() == 32 && t.Level(2).size() == 8);
    CHECK(t.Level(1)[0] == 128 && t.Level(2)[0] == 160);
    CHECK(t.Level(3).empty());
    const auto mip2 = t.DecodeLevel(2);
    CHECK(mip2.size() == 8 && mip2[0] == 160 && mip2[7] == 167);

    // A header claiming more mips than the data holds keeps only whole levels.
    blob.resize(5 + 32 * 4 + 8 * 4);
    stream->data = Value::Struct{{"offset", Leaf(5)}, {"size", Leaf(32 * 4 + 8 * 4)}, {"path", Text("archive:/CAB-test/CAB-test.resS")}};
    const Texture2DAsset shorter = ReadTexture2D(*texture, fetch);
    CHECK(shorter.mipCount == 2);
}
