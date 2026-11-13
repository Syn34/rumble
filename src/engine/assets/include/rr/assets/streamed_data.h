// streamed_data.h - access to bulk data kept outside the serialized objects.
//
// Large payloads (texture pixels, vertex data) are often not stored inside the
// object itself: the object carries a StreamingInfo {offset, size, path} that
// points into a resource blob (a ".resS" entry of the same bundle). The readers
// take a ResourceFetcher so they can be used with real bundles (ResourceStore)
// or with in-memory data in tests.
#pragma once

#include "rr/assets/content_library.h"
#include "rr/assets/type_tree_value.h"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace rr::assets {

struct StreamingInfo {
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::string path; // e.g. "archive:/CAB-x/CAB-x.resS"

    bool Empty() const { return size == 0; }
};

// Reads a StreamingInfo field; nullopt when the field is missing.
std::optional<StreamingInfo> ReadStreamingInfo(const Value* value);

// Returns the `size` bytes at `offset` of the named resource. Throws ReadError if
// the resource can't be found or is too short.
using ResourceFetcher = std::function<std::vector<std::uint8_t>(const StreamingInfo&)>;

// Bytes of a byte-array field (TypelessData or vector<UInt8>); empty if absent.
std::vector<std::uint8_t> ValueBytes(const Value* value);

// Finds resource entries in the bundles of a ContentLibrary and serves byte ranges
// from them. Bundles are decompressed on first use; the most recently used ones
// stay cached.
class ResourceStore {
public:
    explicit ResourceStore(const ContentLibrary& library, std::size_t cachedBundles = 2);

    std::vector<std::uint8_t> Fetch(const StreamingInfo& info);
    ResourceFetcher Fetcher() {
        return [this](const StreamingInfo& info) { return Fetch(info); };
    }

private:
    const BundleFile& OpenBundle(const std::filesystem::path& path);

    std::map<std::string, std::filesystem::path> entryToBundle_;
    std::size_t cacheLimit_;
    std::mutex mutex_;
    // Most recently used last.
    std::vector<std::pair<std::filesystem::path, std::unique_ptr<BundleFile>>> cache_;
};

// Name of a named object (Mesh, Texture2D, Material, ...), read from the first
// field of its serialized data without decoding the rest. Empty if unreadable.
std::string ReadObjectName(const SerializedFile& file, const ObjectInfo& object);

} // namespace rr::assets
