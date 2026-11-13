#include "rr/assets/common_strings.h"

#include <algorithm>
#include <cstring>

namespace rr::assets {

namespace {

// The table always begins with these two names (it is sorted alphabetically and
// these are the first entries), which is enough to locate it unambiguously.
constexpr char kTableStart[] = "AABB\0AnimationClip\0";
constexpr std::string_view kCommonPrefix = "@common:";

} // namespace

std::optional<CommonStrings> CommonStrings::FromEngineLibrary(std::span<const std::uint8_t> library) {
    const auto* begin = library.data();
    const auto* end = begin + library.size();
    const auto* needleEnd = reinterpret_cast<const std::uint8_t*>(kTableStart) + sizeof(kTableStart) - 1;
    const auto* hit = std::search(begin, end, reinterpret_cast<const std::uint8_t*>(kTableStart), needleEnd);
    if (hit == end) return std::nullopt;

    CommonStrings table;
    // Consecutive null-terminated names; an empty name ends the table.
    std::uint32_t offset = 0;
    const auto* p = hit;
    while (p < end && *p != 0) {
        const auto* nul = static_cast<const std::uint8_t*>(std::memchr(p, 0, static_cast<std::size_t>(end - p)));
        if (!nul) break;
        const std::string name(reinterpret_cast<const char*>(p), static_cast<std::size_t>(nul - p));
        table.names_.emplace(offset, name);
        offset += static_cast<std::uint32_t>(name.size() + 1);
        p = nul + 1;
    }
    return table;
}

std::optional<std::string> CommonStrings::Lookup(std::uint32_t offset) const {
    auto it = names_.find(offset);
    if (it == names_.end()) return std::nullopt;
    return it->second;
}

std::string CommonStrings::Resolve(const std::string& name) const {
    if (name.compare(0, kCommonPrefix.size(), kCommonPrefix) != 0) return name;
    const auto offset = static_cast<std::uint32_t>(std::stoul(name.substr(kCommonPrefix.size())));
    return Lookup(offset).value_or(name);
}

} // namespace rr::assets
