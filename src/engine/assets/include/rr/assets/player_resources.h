// player_resources.h - the player's Resources folder (engine layer, new code).
//
// Objects the original loads with Resources.Load(path) are listed in the ResourceManager
// object of `globalgamemanagers`: a table of lower-case paths to object pointers into the
// player's data files. This reader opens that table, finds the referenced files in the data
// folders it is given and reads TextAsset objects (the only kind the PC build needs from
// Resources so far; their layout is fixed, so no type trees are needed).
#pragma once

#include "rr/assets/serialized_file.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rr::assets {

class PlayerResources {
public:
    struct Location {
        std::string file;      // data file name, as globalgamemanagers refers to it
        std::int64_t pathId = 0;
    };

    // `dataFolders` are searched in order for globalgamemanagers and the files it refers
    // to. Null if no folder has a readable globalgamemanagers.
    static std::unique_ptr<PlayerResources> Open(std::vector<std::filesystem::path> dataFolders);

    // Number of Resources paths in the table.
    std::size_t Size() const { return entries_.size(); }
    // The object behind a Resources path (case-insensitive, as Resources.Load).
    std::optional<Location> Find(const std::string& path) const;
    // Full path of a data file, if one of the folders has it.
    std::optional<std::filesystem::path> Locate(const std::string& file) const;

    // The bytes of the TextAsset at a Resources path. On failure returns nothing and, when
    // given, says why in `error` (e.g. the data file that is missing).
    std::optional<std::vector<std::uint8_t>> LoadTextAsset(const std::string& path, std::string* error = nullptr);

private:
    PlayerResources() = default;
    const SerializedFile* OpenFile(const std::string& file);

    std::vector<std::filesystem::path> folders_;
    std::map<std::string, Location> entries_;
    std::map<std::string, std::unique_ptr<SerializedFile>> files_;
};

} // namespace rr::assets
