#include "rr/assets/il2cpp_metadata.h"

#include "rr/assets/binary_reader.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

namespace rr::assets {

namespace {

constexpr std::uint32_t kMagic = 0xFAB11BAF;
constexpr int kSupportedVersion = 39;

// Header sections in file order (each is offset, size, count).
enum SectionIndex : int {
    kString = 2,
    kFieldDefaultValues = 7,
    kFieldAndParameterDefaultValueData = 8,
    kFields = 11,
    kGenericContainers = 14,
    kInterfaceOffsets = 18,
    kTypeDefinitions = 19,
    kImages = 20,
    kSectionCount = 31,
};

// v38+ stores some indices in the smallest width that fits the table.
int IndexWidth(std::uint32_t count) {
    if (count <= 0xFF) return 1;
    if (count <= 0xFFFF) return 2;
    return 4;
}

} // namespace

Il2CppMetadata::Il2CppMetadata(std::vector<std::uint8_t> bytes) : bytes_(std::move(bytes)) {
    BinaryReader r(bytes_);
    if (r.U32() != kMagic) throw ReadError("not an IL2CPP metadata file");
    if (r.I32() != kSupportedVersion) throw ReadError("unsupported IL2CPP metadata version");
    Section sections[kSectionCount];
    for (auto& section : sections) {
        section.offset = r.U32();
        section.size = r.U32();
        section.count = r.U32();
        if (std::uint64_t{section.offset} + section.size > bytes_.size()) throw ReadError("metadata section out of range");
    }
    strings_ = sections[kString];
    fieldDefaultValues_ = sections[kFieldDefaultValues];
    defaultValueData_ = sections[kFieldAndParameterDefaultValueData];
    fields_ = sections[kFields];
    typeDefinitions_ = sections[kTypeDefinitions];
    images_ = sections[kImages];
    typeDefIndexWidth_ = IndexWidth(typeDefinitions_.count);
    genericContainerWidth_ = IndexWidth(sections[kGenericContainers].count);
    // The type table lives in the binary; its index width shows in the interface-offset
    // pairs (type index + int32 offset).
    const Section& interfaceOffsets = sections[kInterfaceOffsets];
    typeIndexWidth_ = interfaceOffsets.count ? static_cast<int>(interfaceOffsets.Stride()) - 4 : 4;
    if (typeIndexWidth_ != 1 && typeIndexWidth_ != 2 && typeIndexWidth_ != 4) throw ReadError("unexpected type index width");
}

std::optional<Il2CppMetadata> Il2CppMetadata::Load(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return std::nullopt;
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    try {
        return Il2CppMetadata(std::move(bytes));
    } catch (const ReadError&) {
        return std::nullopt;
    }
}

std::string Il2CppMetadata::String(std::uint32_t index) const {
    const std::size_t start = std::size_t{strings_.offset} + index;
    const std::size_t end = std::size_t{strings_.offset} + strings_.size;
    if (start >= end) return {};
    const auto* begin = reinterpret_cast<const char*>(bytes_.data() + start);
    return std::string(begin, strnlen(begin, end - start));
}

std::uint32_t Il2CppMetadata::Index(std::size_t pos, int width) const {
    BinaryReader r(bytes_);
    r.Seek(pos);
    switch (width) {
    case 1: return r.U8();
    case 2: return r.U16();
    default: return r.U32();
    }
}

std::vector<Il2CppMetadata::StaticData> Il2CppMetadata::PrivateImplementationData(const std::string& imageName) const {
    BinaryReader r(bytes_);
    // Find the image and its type range.
    std::uint32_t firstType = 0, typeCount = 0;
    bool found = false;
    for (std::uint32_t i = 0; i < images_.count && !found; ++i) {
        const std::size_t pos = std::size_t{images_.offset} + std::size_t{i} * images_.Stride();
        r.Seek(pos);
        const std::uint32_t name = r.U32();
        if (String(name) != imageName) continue;
        firstType = Index(pos + 8, typeDefIndexWidth_);
        r.Seek(pos + 8 + static_cast<std::size_t>(typeDefIndexWidth_));
        typeCount = r.U32();
        found = true;
    }
    if (!found) return {};

    // The <PrivateImplementationDetails> type's field range.
    const std::size_t fieldStartOffset = 8 + 3 * static_cast<std::size_t>(typeIndexWidth_) +
                                         static_cast<std::size_t>(genericContainerWidth_) + 4;
    const std::size_t fieldCountOffset = fieldStartOffset + 4 * 8 + 2 * 2;
    std::int32_t firstField = -1;
    std::uint32_t fieldCount = 0;
    for (std::uint32_t t = firstType; t < firstType + typeCount && t < typeDefinitions_.count; ++t) {
        const std::size_t pos = std::size_t{typeDefinitions_.offset} + std::size_t{t} * typeDefinitions_.Stride();
        r.Seek(pos);
        if (String(r.U32()) != "<PrivateImplementationDetails>") continue;
        r.Seek(pos + fieldStartOffset);
        firstField = r.I32();
        r.Seek(pos + fieldCountOffset);
        fieldCount = r.U16();
        break;
    }
    if (firstField < 0) return {};

    std::vector<StaticData> out;
    const auto first = static_cast<std::uint32_t>(firstField);
    for (std::uint32_t i = 0; i < fieldDefaultValues_.count; ++i) {
        const std::size_t pos = std::size_t{fieldDefaultValues_.offset} + std::size_t{i} * fieldDefaultValues_.Stride();
        r.Seek(pos);
        const std::uint32_t field = r.U32();
        if (field < first || field >= first + fieldCount) continue;
        r.Seek(pos + 4 + static_cast<std::size_t>(typeIndexWidth_));
        const std::uint32_t dataIndex = r.U32();
        if (dataIndex >= defaultValueData_.size) continue;
        StaticData data;
        r.Seek(std::size_t{fields_.offset} + std::size_t{field} * fields_.Stride());
        data.fieldName = String(r.U32());
        data.dataOffset = dataIndex;
        data.available = std::span<const std::uint8_t>(bytes_).subspan(std::size_t{defaultValueData_.offset} + dataIndex,
                                                                     defaultValueData_.size - dataIndex);
        out.push_back(std::move(data));
    }
    std::sort(out.begin(), out.end(), [](const StaticData& a, const StaticData& b) { return a.dataOffset < b.dataOffset; });
    return out;
}

} // namespace rr::assets
