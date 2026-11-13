// common_strings.h - the engine's shared names for embedded type descriptions.
//
// Type descriptions inside content files refer to frequently used type and field
// names (e.g. basic number types) by offset into a string table built into the
// original engine instead of storing them. That table isn't part of the content
// files, so it is read at runtime from the engine library in the user's own copy
// of the game (lib/arm64-v8a/libunity.so in the arm64 APK split).
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>

namespace rr::assets {

class CommonStrings {
public:
    // Finds the table inside the engine library. Returns nullopt if it isn't there.
    static std::optional<CommonStrings> FromEngineLibrary(std::span<const std::uint8_t> library);

    // Name for an offset into the table, or nullopt if unknown.
    std::optional<std::string> Lookup(std::uint32_t offset) const;

    // Turns names stored as "@common:<offset>" by SerializedFile into real names;
    // other names are returned unchanged.
    std::string Resolve(const std::string& name) const;

    std::size_t Size() const { return names_.size(); }

private:
    std::unordered_map<std::uint32_t, std::string> names_;
};

} // namespace rr::assets
