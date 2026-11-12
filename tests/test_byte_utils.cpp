#include "test_framework.h"

#include "photon_deterministic/byte_utils.h"

#include <cstdint>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Photon::Deterministic::ByteUtils;
using Photon::Deterministic::Encoding;

namespace {

std::vector<std::uint8_t> Bytes(std::string_view s) {
    return {s.begin(), s.end()};
}

} // namespace

TEST_CASE("ByteUtils value blocks and raw writes are little endian") {
    std::vector<std::uint8_t> buffer(32, 0);
    int offset = ByteUtils::AddValueBlock(std::int32_t{0x11223344}, buffer, 0);
    CHECK(offset == 8);
    CHECK(buffer[0] == 4);
    CHECK(buffer[4] == 0x44);
    CHECK(buffer[7] == 0x11);
    offset = ByteUtils::AddValueBlock(std::int64_t{-2}, buffer, offset);
    CHECK(offset == 20);
    CHECK(buffer[8] == 8);
    CHECK(buffer[12] == 0xFE);
    CHECK(buffer[19] == 0xFF);
    offset = ByteUtils::AddValueBlock(std::uint64_t{1}, std::span<std::uint8_t>(buffer), 20);
    CHECK(offset == 32);

    CHECK(ByteUtils::WriteBytes(std::int32_t{-1}, buffer, 0) == 4);
    CHECK(ByteUtils::WriteBytes(std::int64_t{0x0102030405060708}, buffer, 4) == 8);
    CHECK(buffer[3] == 0xFF);
    CHECK(buffer[4] == 0x08);
    CHECK(buffer[11] == 0x01);

    int blockStart = -1;
    const int contentStart = ByteUtils::BeginByteBlockHeader(buffer, 12, blockStart);
    CHECK(blockStart == 12);
    CHECK(contentStart == 16);
    CHECK(ByteUtils::EndByteBlockHeader(buffer, blockStart, 5) == 21);
    CHECK(buffer[12] == 5);

    bool threw = false;
    try {
        ByteUtils::WriteBytes(std::int64_t{0}, buffer, 30);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("ByteUtils pack, read and merge byte blocks") {
    const std::vector<std::vector<std::uint8_t>> blocks = {{1, 2, 3}, {}, {0xFF}};
    const auto packed = ByteUtils::PackByteBlocks(blocks);
    CHECK(packed == (std::vector<std::uint8_t>{3, 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0xFF}));
    CHECK(ByteUtils::ReadByteBlocks(packed) == blocks);
    CHECK(ByteUtils::MergeByteBlocks(blocks) == (std::vector<std::uint8_t>{1, 2, 3, 0xFF}));
    CHECK(ByteUtils::ReadByteBlocks({}).empty());
}

TEST_CASE("ByteUtils Base64") {
    CHECK(ByteUtils::Base64Encode(Bytes("")) == "");
    CHECK(ByteUtils::Base64Encode(Bytes("f")) == "Zg==");
    CHECK(ByteUtils::Base64Encode(Bytes("fo")) == "Zm8=");
    CHECK(ByteUtils::Base64Encode(Bytes("foobar")) == "Zm9vYmFy");
    CHECK(ByteUtils::Base64Decode("Zm9vYmE=") == Bytes("fooba"));
    CHECK(ByteUtils::Base64Decode(" Zm9v\r\nYg== ") == Bytes("foob"));
    std::vector<std::uint8_t> all(256);
    for (int i = 0; i < 256; ++i) {
        all[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(i);
    }
    CHECK(ByteUtils::Base64Decode(ByteUtils::Base64Encode(all)) == all);

    int failures = 0;
    for (const char* bad : {"Zm9", "Zm=v", "Zm9v!A==", "===="}) {
        try {
            ByteUtils::Base64Decode(bad);
        } catch (const std::invalid_argument&) {
            ++failures;
        }
    }
    CHECK(failures == 4);
}

TEST_CASE("ByteUtils GZip decodes streams produced by zlib") {
    // gzip of "hello hello hello hello" (fixed Huffman block).
    const std::vector<std::uint8_t> fixed = {
        0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xcb, 0x48, 0xcd, 0xc9,
        0xc9, 0x57, 0xc8, 0x40, 0x27, 0x01, 0xe3, 0x51, 0x3d, 0x8d, 0x17, 0x00, 0x00, 0x00};
    CHECK(ByteUtils::GZipDecompressBytes(fixed) == Bytes("hello hello hello hello"));

    // gzip of a longer text (dynamic Huffman block).
    std::string text;
    for (int i = 0; i < 40; ++i) {
        char line[96];
        std::snprintf(line, sizeof line, "The quick brown fox %d jumps over the lazy dog %d. ", i,
                      i * i % 97);
        text += line;
    }
    const std::vector<std::uint8_t> dynamic = {
        0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7d, 0xd4, 0xd9, 0x15, 0xc2,
        0x20, 0x10, 0x40, 0xd1, 0x56, 0xa6, 0x82, 0x9c, 0x30, 0x03, 0x04, 0xfa, 0x48, 0x03, 0x2e,
        0x71, 0x57, 0x34, 0x1a, 0xb7, 0xea, 0xb5, 0x00, 0x5e, 0xfe, 0xf3, 0x4e, 0x80, 0x3b, 0xd0,
        0xef, 0x06, 0xb9, 0x4d, 0xfb, 0xd5, 0x51, 0x96, 0x63, 0x79, 0x5d, 0x64, 0x53, 0xde, 0xd2,
        0xca, 0x61, 0x3a, 0x5f, 0xef, 0x52, 0x9e, 0xc3, 0x28, 0x8f, 0xff, 0x07, 0xa7, 0xc5, 0xf7,
        0x23, 0xeb, 0xb2, 0x95, 0xb6, 0x91, 0xbe, 0x12, 0x38, 0x0c, 0x5c, 0x3d, 0x50, 0x0c, 0x7c,
        0x3d, 0x30, 0x0c, 0x72, 0x3d, 0xf0, 0xbc, 0xa4, 0x58, 0x2f, 0x02, 0x16, 0x1a, 0xea, 0x45,
        0xc4, 0xc2, 0xe0, 0x1f, 0x1d, 0xef, 0x1b, 0xf6, 0x91, 0xb0, 0x88, 0x70, 0x54, 0x19, 0x8b,
        0x04, 0x1a, 0x8e, 0xc1, 0x0d, 0x0a, 0x16, 0x57, 0x58, 0x96, 0x9b, 0x31, 0xef, 0x20, 0x61,
        0xf5, 0x4e, 0x21, 0x61, 0x77, 0x2a, 0xd8, 0xdd, 0xe8, 0xbc, 0x18, 0x3e, 0xd2, 0x5f, 0x58,
        0x3e, 0xc3, 0x74, 0x39, 0xa6, 0x37, 0x62, 0x61, 0xfb, 0x0e, 0xae, 0xae, 0xb2, 0xbd, 0x83,
        0xbd, 0x28, 0xe3, 0x07, 0x58, 0x98, 0x32, 0x7e, 0x86, 0xab, 0xa2, 0x8c, 0xef, 0x61, 0xc4,
        0x94, 0xf1, 0x33, 0x3d, 0x44, 0xac, 0xef, 0x69, 0x2f, 0xac, 0x9f, 0x69, 0x61, 0xac, 0x1f,
        0xc8, 0x85, 0xf5, 0x13, 0x14, 0x8c, 0x1f, 0x61, 0xc4, 0x8c, 0xf1, 0x15, 0xee, 0xa4, 0x31,
        0x7e, 0x82, 0x85, 0x19, 0xe3, 0x07, 0x7a, 0xee, 0x19, 0x5f, 0x61, 0x2a, 0x8d, 0xf1, 0x13,
        0xbc, 0xad, 0xc6, 0xf8, 0x11, 0xe6, 0xc5, 0x66, 0xde, 0x7c, 0x3a, 0x64, 0xc6, 0x77, 0xf4,
        0x97, 0x19, 0x7c, 0xb8, 0x2e, 0x36, 0xa3, 0x1f, 0x1b, 0xf9, 0x01, 0xe9, 0x8f, 0x1f, 0x22,
        0xe7, 0x07, 0x00, 0x00};
    CHECK(ByteUtils::GZipDecompressString(dynamic, Encoding::UTF8) == text);

    // Corruption is detected (CRC mismatch).
    auto corrupt = fixed;
    corrupt[corrupt.size() - 8] ^= 1;
    bool threw = false;
    try {
        ByteUtils::GZipDecompressBytes(corrupt);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("ByteUtils GZip round trips") {
    std::vector<std::uint8_t> big(200000);
    for (std::size_t i = 0; i < big.size(); ++i) {
        big[i] = static_cast<std::uint8_t>((i * 2654435761u) >> 13);
    }
    CHECK(ByteUtils::GZipDecompressBytes(ByteUtils::GZipCompressBytes(big)) == big);
    CHECK(ByteUtils::GZipDecompressBytes(ByteUtils::GZipCompressBytes({})).empty());

    const auto compressed = ByteUtils::GZipCompressString("snapshot", Encoding::UTF8);
    CHECK(compressed[0] == 0x1F && compressed[1] == 0x8B);
    CHECK(ByteUtils::GZipDecompressString(compressed, Encoding::UTF8) == "snapshot");

    std::ostringstream out;
    ByteUtils::GZipCompressBytes(Bytes("xxabcxx"), 2, 3, out);
    const std::string s1 = out.str();
    CHECK(ByteUtils::GZipDecompressBytes(std::vector<std::uint8_t>(s1.begin(), s1.end())) ==
          Bytes("abc"));

    std::istringstream in(std::string(10000, 'q'));
    std::ostringstream out2;
    ByteUtils::GZipCompressBytes(in, out2);
    const std::string s2 = out2.str();
    CHECK(ByteUtils::GZipDecompressString(std::vector<std::uint8_t>(s2.begin(), s2.end()),
                                          Encoding::UTF8) == std::string(10000, 'q'));

    std::istringstream copySource(std::string(9000, 'z'));
    std::ostringstream copyTarget;
    ByteUtils::CopyTo(copySource, copyTarget);
    CHECK(copyTarget.str() == std::string(9000, 'z'));
}
