#include "test_framework.h"

#include "rr/assets/serialized_file.h"

#include <string>
#include <vector>

using namespace rr::assets;

namespace {

// Builds a minimal, valid version-22 container in memory: one type (class 49),
// one object whose payload is `payload`, and one external reference.
class FileBuilder {
public:
    std::vector<std::uint8_t> Build(const std::string& payload) {
        std::vector<std::uint8_t> meta;
        PutString(meta, "6000.3.10f1");
        PutLE(meta, std::int32_t{13});  // platform
        meta.push_back(0);               // no type trees
        PutLE(meta, std::int32_t{1});   // type count
        PutLE(meta, std::int32_t{49});  // class id
        meta.push_back(0);               // not stripped
        PutLE(meta, std::int16_t{-1});  // no script
        meta.insert(meta.end(), 16, 0xAB); // type hash
        PutLE(meta, std::int32_t{1});   // object count
        constexpr std::size_t kHeaderSize = 48;
        while ((kHeaderSize + meta.size()) % 4) meta.push_back(0);
        PutLE(meta, std::int64_t{42});  // path id
        PutLE(meta, std::uint64_t{0});  // byte start
        PutLE(meta, static_cast<std::uint32_t>(payload.size()));
        PutLE(meta, std::int32_t{0});   // type index
        PutLE(meta, std::int32_t{0});   // scripts
        PutLE(meta, std::int32_t{1});   // externals
        PutString(meta, "");
        meta.insert(meta.end(), 16, 0);
        PutLE(meta, std::int32_t{0});
        PutString(meta, "other.assets");
        PutLE(meta, std::int32_t{0});   // reference types
        PutString(meta, "");            // user info

        const std::uint64_t dataOffset = kHeaderSize + meta.size();
        const std::uint64_t fileSize = dataOffset + payload.size();
        std::vector<std::uint8_t> file;
        PutBE(file, std::uint32_t{0});
        PutBE(file, std::uint32_t{0});
        PutBE(file, std::uint32_t{22});
        PutBE(file, std::uint32_t{0});
        file.insert(file.end(), {0, 0, 0, 0}); // little-endian body + reserved
        PutBE(file, static_cast<std::uint32_t>(meta.size()));
        PutBE(file, fileSize);
        PutBE(file, dataOffset);
        PutBE(file, std::uint64_t{0});
        file.insert(file.end(), meta.begin(), meta.end());
        file.insert(file.end(), payload.begin(), payload.end());
        return file;
    }

private:
    template <typename T> static void PutLE(std::vector<std::uint8_t>& out, T v) {
        for (std::size_t i = 0; i < sizeof(T); ++i)
            out.push_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(v) >> (8 * i)));
    }
    template <typename T> static void PutBE(std::vector<std::uint8_t>& out, T v) {
        for (std::size_t i = sizeof(T); i-- > 0;)
            out.push_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(v) >> (8 * i)));
    }
    static void PutString(std::vector<std::uint8_t>& out, const std::string& s) {
        out.insert(out.end(), s.begin(), s.end());
        out.push_back(0);
    }
};

} // namespace

TEST_CASE("SerializedFile parses a minimal version-22 container") {
    SerializedFile file(FileBuilder().Build("hello"));
    CHECK(file.FormatVersion() == 22);
    CHECK(file.EngineVersion() == "6000.3.10f1");
    CHECK(!file.HasTypeTrees());
    CHECK(file.Types().size() == 1);
    CHECK(file.Externals().size() == 1);
    CHECK(file.Externals()[0].path == "other.assets");
    const ObjectInfo* obj = file.FindObject(42);
    CHECK(obj != nullptr);
    if (obj) {
        CHECK(obj->classId == static_cast<std::int32_t>(ClassId::TextAsset));
        const auto data = file.ObjectData(*obj);
        CHECK(std::string(data.begin(), data.end()) == "hello");
    }
    CHECK(file.FindObject(7) == nullptr);
}

TEST_CASE("SerializedFile rejects truncated input") {
    auto bytes = FileBuilder().Build("hello");
    bytes.pop_back();
    bool threw = false;
    try {
        SerializedFile file(std::move(bytes));
    } catch (const ReadError&) {
        threw = true;
    }
    CHECK(threw);
}
