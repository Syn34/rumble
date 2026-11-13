#include "test_framework.h"

#include "rr/assets/bundle_file.h"
#include "rr/assets/lz4.h"
#include "rr/assets/type_tree_value.h"

#include <string>
#include <vector>

using namespace rr::assets;

namespace {

void PutBE32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    for (int i = 3; i >= 0; --i) out.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
}
void PutBE64(std::vector<std::uint8_t>& out, std::uint64_t v) {
    for (int i = 7; i >= 0; --i) out.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
}
void PutBE16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>(v >> 8));
    out.push_back(static_cast<std::uint8_t>(v));
}
void PutString(std::vector<std::uint8_t>& out, const std::string& s) {
    out.insert(out.end(), s.begin(), s.end());
    out.push_back(0);
}
template <typename T> void PutLE(std::vector<std::uint8_t>& out, T v) {
    for (std::size_t i = 0; i < sizeof(T); ++i)
        out.push_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(v) >> (8 * i)));
}

} // namespace

TEST_CASE("LZ4 block decoder handles literals and overlapping matches") {
    // "hello": one sequence of five literals.
    const std::vector<std::uint8_t> literals = {0x50, 'h', 'e', 'l', 'l', 'o'};
    std::vector<std::uint8_t> out(5);
    Lz4DecompressBlock(literals, out);
    CHECK(std::string(out.begin(), out.end()) == "hello");

    // "abcabcabcX": 3 literals, then a 6-byte match 3 back (overlapping), then "X".
    const std::vector<std::uint8_t> overlap = {0x32, 'a', 'b', 'c', 3, 0, 0x10, 'X'};
    std::vector<std::uint8_t> out2(10);
    Lz4DecompressBlock(overlap, out2);
    CHECK(std::string(out2.begin(), out2.end()) == "abcabcabcX");

    bool threw = false;
    try {
        std::vector<std::uint8_t> wrong(4);
        Lz4DecompressBlock(literals, wrong);
    } catch (const ReadError&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("BundleFile reads an uncompressed bundle with one entry") {
    const std::string payload = "payload!";
    std::vector<std::uint8_t> info(16, 0); // data hash
    PutBE32(info, 1);                       // one block
    PutBE32(info, static_cast<std::uint32_t>(payload.size()));
    PutBE32(info, static_cast<std::uint32_t>(payload.size()));
    PutBE16(info, 0); // stored
    PutBE32(info, 1); // one entry
    PutBE64(info, 0);
    PutBE64(info, payload.size());
    PutBE32(info, 4); // serialized-file flag
    PutString(info, "CAB-test");

    std::vector<std::uint8_t> header;
    PutString(header, "UnityFS");
    PutBE32(header, 8);
    PutString(header, "5.x.x");
    PutString(header, "6000.3.10f1");
    const std::size_t sizeField = header.size();
    PutBE64(header, 0); // patched below
    PutBE32(header, static_cast<std::uint32_t>(info.size()));
    PutBE32(header, static_cast<std::uint32_t>(info.size()));
    PutBE32(header, 0); // uncompressed block table directly after the header
    while (header.size() % 16) header.push_back(0);

    std::vector<std::uint8_t> file = header;
    file.insert(file.end(), info.begin(), info.end());
    file.insert(file.end(), payload.begin(), payload.end());
    const std::uint64_t total = file.size();
    for (int i = 0; i < 8; ++i) file[sizeField + static_cast<std::size_t>(i)] =
        static_cast<std::uint8_t>(total >> (8 * (7 - i)));

    CHECK(BundleFile::LooksLikeBundle(file));
    BundleFile bundle(file);
    CHECK(bundle.EngineVersion() == "6000.3.10f1");
    CHECK(bundle.Entries().size() == 1);
    const BundleEntry* entry = bundle.FindEntry("CAB-test");
    CHECK(entry != nullptr && entry->IsSerializedFile());
    if (entry) {
        const auto data = bundle.EntryData(*entry);
        CHECK(std::string(data.begin(), data.end()) == payload);
    }
}

TEST_CASE("Type-tree values decode structs, aligned strings and arrays") {
    // struct Sample { int count; string name; vector<int> values; bool flag; }
    const std::vector<TypeTreeNode> nodes = {
        {"Sample", "Base", 0, -1, 0},
        {"int", "count", 1, 4, 0},
        {"string", "name", 1, -1, 0x4000},
        {"Array", "Array", 2, -1, 0x4000},
        {"int", "size", 3, 4, 0},
        {"char", "data", 3, 1, 0},
        {"vector", "values", 1, -1, 0},
        {"Array", "Array", 2, -1, 0x4000},
        {"int", "size", 3, 4, 0},
        {"int", "data", 3, 4, 0},
        {"bool", "flag", 1, 1, 0},
    };
    std::vector<std::uint8_t> bytes;
    PutLE(bytes, std::int32_t{7});
    PutLE(bytes, std::int32_t{3});
    bytes.insert(bytes.end(), {'a', 'b', 'c', 0}); // 3 chars + 1 byte of alignment
    PutLE(bytes, std::int32_t{2});
    PutLE(bytes, std::int32_t{10});
    PutLE(bytes, std::int32_t{-20});
    bytes.push_back(1);

    BinaryReader reader(bytes);
    const ValuePtr root = ReadTypeTreeValue(nodes, reader, nullptr);
    CHECK(root->Get("count")->AsInt() == 7);
    CHECK(*root->Get("name")->AsString() == "abc");
    const auto* values = root->Get("values")->AsArray();
    CHECK(values && values->size() == 2 && (*values)[1]->AsInt() == -20);
    CHECK(root->Get("flag")->AsInt() == 1);
    CHECK(reader.Remaining() == 0);
}
