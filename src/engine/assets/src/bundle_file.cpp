#include "rr/assets/bundle_file.h"

#include "rr/assets/binary_reader.h"
#include "rr/assets/lz4.h"

#include <algorithm>
#include <cstring>

namespace rr::assets {

namespace {

constexpr char kSignature[] = "UnityFS";

// Archive flags (header) and block flags (block table).
constexpr std::uint32_t kCompressionMask = 0x3F;
constexpr std::uint32_t kBlocksInfoAtEnd = 0x80;
constexpr std::uint32_t kBlocksInfoNeedsPadding = 0x200;

enum class Compression : std::uint32_t { None = 0, Lzma = 1, Lz4 = 2, Lz4HC = 3 };

std::vector<std::uint8_t> Decompress(std::span<const std::uint8_t> in, std::size_t size,
                                     std::uint32_t method) {
    std::vector<std::uint8_t> out(size);
    switch (static_cast<Compression>(method)) {
    case Compression::None:
        if (in.size() != size) throw ReadError("bundle: stored block has the wrong size");
        std::memcpy(out.data(), in.data(), size);
        break;
    case Compression::Lz4:
    case Compression::Lz4HC:
        Lz4DecompressBlock(in, out);
        break;
    default:
        throw ReadError("bundle: unsupported compression method " + std::to_string(method));
    }
    return out;
}

} // namespace

bool BundleFile::LooksLikeBundle(std::span<const std::uint8_t> bytes) {
    return bytes.size() > sizeof(kSignature) &&
           std::memcmp(bytes.data(), kSignature, sizeof(kSignature)) == 0;
}

BundleFile::BundleFile(std::span<const std::uint8_t> bytes) {
    if (!LooksLikeBundle(bytes)) throw ReadError("not a content bundle");
    BinaryReader r(bytes, Endian::Big);
    r.CString(); // signature
    version_ = r.U32();
    r.CString(); // minimum player version, always "5.x.x"
    engineVersion_ = r.CString();
    const std::uint64_t totalSize = r.U64();
    const std::uint32_t infoCompressedSize = r.U32();
    const std::uint32_t infoSize = r.U32();
    const std::uint32_t flags = r.U32();
    if (totalSize != bytes.size()) throw ReadError("bundle: size in header doesn't match the file");
    if (version_ < 6 || version_ > 8) {
        throw ReadError("bundle: unsupported format version " + std::to_string(version_));
    }
    if (version_ >= 7) r.Align(16);

    // The block table either follows the header or sits at the very end.
    std::size_t infoStart = r.Position();
    if (flags & kBlocksInfoAtEnd) infoStart = bytes.size() - infoCompressedSize;
    const auto info = Decompress(bytes.subspan(infoStart, infoCompressedSize), infoSize,
                                 flags & kCompressionMask);
    std::size_t dataStart = (flags & kBlocksInfoAtEnd) ? r.Position() : infoStart + infoCompressedSize;
    if (flags & kBlocksInfoNeedsPadding) dataStart = (dataStart + 15) / 16 * 16;

    BinaryReader table(info, Endian::Big);
    table.Skip(16); // hash of the uncompressed data
    struct Block {
        std::uint32_t size, compressedSize;
        std::uint16_t flags;
    };
    std::vector<Block> blocks(table.U32());
    std::uint64_t totalUncompressed = 0;
    for (Block& b : blocks) {
        b.size = table.U32();
        b.compressedSize = table.U32();
        b.flags = table.U16();
        totalUncompressed += b.size;
    }
    const std::uint32_t entryCount = table.U32();
    for (std::uint32_t i = 0; i < entryCount; ++i) {
        BundleEntry e;
        e.offset = table.U64();
        e.size = table.U64();
        e.flags = table.U32();
        e.path = table.CString();
        if (e.offset + e.size > totalUncompressed) throw ReadError("bundle: entry outside the data");
        entries_.push_back(std::move(e));
    }

    data_.reserve(totalUncompressed);
    std::size_t pos = dataStart;
    for (const Block& b : blocks) {
        if (pos + b.compressedSize > bytes.size()) throw ReadError("bundle: block outside the file");
        const auto block = Decompress(bytes.subspan(pos, b.compressedSize), b.size,
                                      b.flags & kCompressionMask);
        data_.insert(data_.end(), block.begin(), block.end());
        pos += b.compressedSize;
    }
}

const BundleEntry* BundleFile::FindEntry(const std::string& path) const {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [&](const BundleEntry& e) { return e.path == path; });
    return it != entries_.end() ? &*it : nullptr;
}

std::span<const std::uint8_t> BundleFile::EntryData(const BundleEntry& entry) const {
    return std::span<const std::uint8_t>(data_).subspan(entry.offset, entry.size);
}

std::vector<std::uint8_t> BundleFile::CopyEntry(const BundleEntry& entry) const {
    const auto bytes = EntryData(entry);
    return {bytes.begin(), bytes.end()};
}

} // namespace rr::assets
