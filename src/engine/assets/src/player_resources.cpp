#include "rr/assets/player_resources.h"

#include "rr/assets/binary_reader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>

namespace rr::assets {

namespace {

constexpr std::int32_t kResourceManagerClass = 147;
constexpr std::int32_t kTextAssetClass = 49;

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::optional<std::vector<std::uint8_t>> ReadFile(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return std::nullopt;
    return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// A length-prefixed string, padded to 4 bytes.
std::string AlignedString(BinaryReader& r) {
    const std::int32_t length = r.I32();
    if (length < 0) throw ReadError("negative string length");
    const auto bytes = r.Bytes(static_cast<std::size_t>(length));
    r.Align(4);
    return std::string(bytes.begin(), bytes.end());
}

} // namespace

std::unique_ptr<PlayerResources> PlayerResources::Open(std::vector<std::filesystem::path> dataFolders) {
    std::unique_ptr<PlayerResources> resources(new PlayerResources());
    resources->folders_ = std::move(dataFolders);
    const SerializedFile* managers = resources->OpenFile("globalgamemanagers");
    if (managers == nullptr) return nullptr;
    for (const ObjectInfo& object : managers->Objects()) {
        if (object.classId != kResourceManagerClass) continue;
        // ResourceManager.m_Container: vector<pair<string, PPtr<Object>>>.
        BinaryReader r = managers->OpenObject(object);
        const std::int32_t count = r.I32();
        for (std::int32_t i = 0; i < count; ++i) {
            const std::string path = AlignedString(r);
            const std::int32_t fileId = r.I32();
            const std::int64_t pathId = r.I64();
            std::string file;
            if (fileId == 0) {
                file = "globalgamemanagers";
            } else if (fileId > 0 && static_cast<std::size_t>(fileId) <= managers->Externals().size()) {
                file = managers->Externals()[static_cast<std::size_t>(fileId - 1)].path;
            } else {
                continue;
            }
            // Several objects can share a path (a prefab and its parts); keep the first.
            resources->entries_.emplace(Lower(path), Location{file, pathId});
        }
        break;
    }
    return resources;
}

std::optional<PlayerResources::Location> PlayerResources::Find(const std::string& path) const {
    auto it = entries_.find(Lower(path));
    if (it == entries_.end()) return std::nullopt;
    return it->second;
}

std::optional<std::filesystem::path> PlayerResources::Locate(const std::string& file) const {
    // References look like "sharedassets0.assets" or "Library/unity default resources";
    // the player stores them by their last path component.
    const std::filesystem::path name = std::filesystem::path(file).filename();
    for (const auto& folder : folders_) {
        std::error_code ec;
        const auto candidate = folder / name;
        if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
    }
    return std::nullopt;
}

const SerializedFile* PlayerResources::OpenFile(const std::string& file) {
    auto it = files_.find(file);
    if (it != files_.end()) return it->second.get();
    std::unique_ptr<SerializedFile> parsed;
    if (auto location = Locate(file)) {
        // Large data files are split into numbered parts.
        std::optional<std::vector<std::uint8_t>> bytes = ReadFile(*location);
        if (bytes) {
            try {
                parsed = std::make_unique<SerializedFile>(std::move(*bytes));
            } catch (const ReadError&) {
                parsed.reset();
            }
        }
    } else if (Locate(file + ".split0")) {
        std::vector<std::uint8_t> joined;
        for (int part = 0;; ++part) {
            auto partPath = Locate(file + ".split" + std::to_string(part));
            if (!partPath) break;
            auto bytes = ReadFile(*partPath);
            if (!bytes) break;
            joined.insert(joined.end(), bytes->begin(), bytes->end());
        }
        try {
            parsed = std::make_unique<SerializedFile>(std::move(joined));
        } catch (const ReadError&) {
            parsed.reset();
        }
    }
    const SerializedFile* result = parsed.get();
    files_.emplace(file, std::move(parsed));
    return result;
}

std::optional<std::vector<std::uint8_t>> PlayerResources::LoadTextAsset(const std::string& path, std::string* error) {
    const auto fail = [error](std::string why) -> std::optional<std::vector<std::uint8_t>> {
        if (error) *error = std::move(why);
        return std::nullopt;
    };
    const auto location = Find(path);
    if (!location) return fail("'" + path + "' is not in the Resources table");
    const SerializedFile* file = OpenFile(location->file);
    if (file == nullptr) return fail("data file '" + location->file + "' (Resources '" + path + "') not found");
    const ObjectInfo* object = file->FindObject(location->pathId);
    if (object == nullptr || object->classId != kTextAssetClass) {
        return fail("'" + path + "' is not a TextAsset in '" + location->file + "'");
    }
    try {
        // TextAsset: m_Name, then m_Script (the bytes).
        BinaryReader r = file->OpenObject(*object);
        AlignedString(r);
        const std::int32_t length = r.I32();
        if (length < 0) return fail("bad TextAsset '" + path + "'");
        const auto bytes = r.Bytes(static_cast<std::size_t>(length));
        return std::vector<std::uint8_t>(bytes.begin(), bytes.end());
    } catch (const ReadError& e) {
        return fail("bad TextAsset '" + path + "': " + e.what());
    }
}

} // namespace rr::assets
