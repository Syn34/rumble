// bundle_file.h - reader for the game's content bundles ("UnityFS" archives).
//
// The game ships its downloadable content as bundles: a header, a table of
// compressed storage blocks, and a directory of named entries (usually one
// serialized file plus resource blobs such as texture and mesh data). This reader
// decompresses the blocks and exposes each entry's bytes.
#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rr::assets {

struct BundleEntry {
    std::string path;      // e.g. "CAB-1234abcd" or "CAB-1234abcd.resS"
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
    std::uint32_t flags = 0; // bit 2: entry is a serialized file

    bool IsSerializedFile() const { return (flags & 4) != 0; }
};

class BundleFile {
public:
    // Parses and decompresses the whole bundle. Throws ReadError on malformed or
    // unsupported input (only uncompressed and LZ4/LZ4HC blocks are supported,
    // which is what the game's bundles use).
    explicit BundleFile(std::span<const std::uint8_t> bytes);

    static bool LooksLikeBundle(std::span<const std::uint8_t> bytes);

    std::uint32_t FormatVersion() const { return version_; }
    const std::string& EngineVersion() const { return engineVersion_; }
    const std::vector<BundleEntry>& Entries() const { return entries_; }

    const BundleEntry* FindEntry(const std::string& path) const;
    std::span<const std::uint8_t> EntryData(const BundleEntry& entry) const;
    std::vector<std::uint8_t> CopyEntry(const BundleEntry& entry) const;

private:
    std::uint32_t version_ = 0;
    std::string engineVersion_;
    std::vector<BundleEntry> entries_;
    std::vector<std::uint8_t> data_; // all storage blocks, decompressed and concatenated
};

} // namespace rr::assets
