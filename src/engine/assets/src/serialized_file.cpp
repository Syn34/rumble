#include "rr/assets/serialized_file.h"

#include <algorithm>

namespace rr::assets {

namespace {

// The only container version the game's files use. Older versions differ in
// many small ways; supporting them isn't needed for this game.
constexpr std::uint32_t kSupportedVersion = 22;

std::array<std::uint8_t, 16> ReadHash(BinaryReader& r) {
    std::array<std::uint8_t, 16> hash{};
    const auto bytes = r.Bytes(hash.size());
    std::copy(bytes.begin(), bytes.end(), hash.begin());
    return hash;
}

// Strings in an embedded type tree are offsets either into the file's own
// string buffer or (high bit set) into the engine's built-in common-string list.
// The built-in list isn't shipped with the files; such names are reported by offset.
std::string ResolveTypeTreeString(std::uint32_t offset, std::span<const std::uint8_t> local) {
    if (offset & 0x80000000u) {
        return "@common:" + std::to_string(offset & 0x7FFFFFFFu);
    }
    if (offset >= local.size()) return {};
    const auto* begin = reinterpret_cast<const char*>(local.data() + offset);
    const auto* end = static_cast<const char*>(std::memchr(begin, 0, local.size() - offset));
    return end ? std::string(begin, end) : std::string(begin, local.size() - offset);
}

} // namespace

const char* ClassName(std::int32_t classId) {
    switch (static_cast<ClassId>(classId)) {
    case ClassId::GameObject: return "GameObject";
    case ClassId::Transform: return "Transform";
    case ClassId::Material: return "Material";
    case ClassId::Texture2D: return "Texture2D";
    case ClassId::Mesh: return "Mesh";
    case ClassId::Shader: return "Shader";
    case ClassId::TextAsset: return "TextAsset";
    case ClassId::AudioClip: return "AudioClip";
    case ClassId::MonoBehaviour: return "MonoBehaviour";
    case ClassId::MonoScript: return "MonoScript";
    case ClassId::AssetBundle: return "AssetBundle";
    case ClassId::Sprite: return "Sprite";
    }
    return "Unknown";
}

SerializedFile::SerializedFile(std::vector<std::uint8_t> bytes) : bytes_(std::move(bytes)) {
    BinaryReader r(bytes_, Endian::Big); // the fixed header is always big-endian
    ReadHeader(r);
    r.SetEndian(endian_);
    ReadMetadata(r);
}

void SerializedFile::ReadHeader(BinaryReader& r) {
    // Legacy 32-bit header fields, unused (zero) from version 22 on.
    r.U32(); // metadata size
    r.U32(); // file size
    version_ = r.U32();
    r.U32(); // data offset
    if (version_ != kSupportedVersion) {
        throw ReadError("unsupported serialized file version " + std::to_string(version_));
    }
    endian_ = r.U8() != 0 ? Endian::Big : Endian::Little;
    r.Skip(3); // reserved

    r.U32();                // metadata size
    const auto fileSize = r.U64();
    dataOffset_ = r.U64();
    r.U64();                // reserved
    if (fileSize != bytes_.size() || dataOffset_ > bytes_.size()) {
        throw ReadError("serialized file is truncated or has an inconsistent header");
    }
}

void SerializedFile::ReadMetadata(BinaryReader& r) {
    engineVersion_ = r.CString();
    targetPlatform_ = r.I32();
    typeTreesEnabled_ = r.Bool();

    const std::int32_t typeCount = r.I32();
    types_.reserve(static_cast<std::size_t>(typeCount));
    for (std::int32_t i = 0; i < typeCount; ++i) types_.push_back(ReadType(r, false));

    const std::int32_t objectCount = r.I32();
    objects_.reserve(static_cast<std::size_t>(objectCount));
    for (std::int32_t i = 0; i < objectCount; ++i) {
        r.Align(4);
        ObjectInfo object;
        object.pathId = r.I64();
        object.byteStart = r.U64();
        object.byteSize = r.U32();
        object.typeIndex = r.I32();
        if (object.typeIndex < 0 || static_cast<std::size_t>(object.typeIndex) >= types_.size()) {
            throw ReadError("object refers to an unknown type");
        }
        object.classId = types_[static_cast<std::size_t>(object.typeIndex)].classId;
        if (dataOffset_ + object.byteStart + object.byteSize > bytes_.size()) {
            throw ReadError("object data lies outside the file");
        }
        objects_.push_back(object);
    }

    const std::int32_t scriptCount = r.I32();
    for (std::int32_t i = 0; i < scriptCount; ++i) {
        ObjectPointer ptr;
        ptr.fileId = r.I32();
        r.Align(4);
        ptr.pathId = r.I64();
        scriptTypes_.push_back(ptr);
    }

    const std::int32_t externalCount = r.I32();
    for (std::int32_t i = 0; i < externalCount; ++i) {
        ExternalReference ext;
        r.CString(); // historically an asset path, always empty now
        ext.guid = ReadHash(r);
        ext.type = r.I32();
        ext.path = r.CString();
        externals_.push_back(std::move(ext));
    }

    const std::int32_t refTypeCount = r.I32();
    for (std::int32_t i = 0; i < refTypeCount; ++i) referenceTypes_.push_back(ReadType(r, true));

    r.CString(); // user information, unused

    // Keep objects sorted by path ID for lookup.
    std::sort(objects_.begin(), objects_.end(),
              [](const ObjectInfo& a, const ObjectInfo& b) { return a.pathId < b.pathId; });
}

SerializedType SerializedFile::ReadType(BinaryReader& r, bool isReferenceType) {
    SerializedType type;
    type.classId = r.I32();
    type.stripped = r.Bool();
    type.scriptTypeIndex = r.I16();
    // Script-backed types carry the hash of the script that defines them.
    if ((isReferenceType && type.scriptTypeIndex >= 0) ||
        type.classId == static_cast<std::int32_t>(ClassId::MonoBehaviour)) {
        type.scriptHash = ReadHash(r);
    }
    type.typeHash = ReadHash(r);

    if (typeTreesEnabled_) {
        type.typeTree = ReadTypeTree(r);
        if (isReferenceType) {
            type.className = r.CString();
            type.classNamespace = r.CString();
            type.assemblyName = r.CString();
        } else {
            const std::int32_t dependencyCount = r.I32();
            r.Skip(static_cast<std::size_t>(dependencyCount) * 4);
        }
    }
    return type;
}

std::vector<TypeTreeNode> SerializedFile::ReadTypeTree(BinaryReader& r) {
    const std::int32_t nodeCount = r.I32();
    const std::int32_t stringBytes = r.I32();
    struct RawNode {
        std::uint32_t typeOffset, nameOffset;
        int level;
        std::int32_t byteSize;
        std::uint32_t metaFlags;
    };
    std::vector<RawNode> raw;
    raw.reserve(static_cast<std::size_t>(nodeCount));
    for (std::int32_t i = 0; i < nodeCount; ++i) {
        RawNode n{};
        r.U16(); // node version
        n.level = r.U8();
        r.U8(); // type flags
        n.typeOffset = r.U32();
        n.nameOffset = r.U32();
        n.byteSize = r.I32();
        r.I32(); // index
        n.metaFlags = r.U32();
        r.U64(); // reference type hash
        raw.push_back(n);
    }
    const auto strings = r.Bytes(static_cast<std::size_t>(stringBytes));

    std::vector<TypeTreeNode> nodes;
    nodes.reserve(raw.size());
    for (const RawNode& n : raw) {
        nodes.push_back({ResolveTypeTreeString(n.typeOffset, strings),
                         ResolveTypeTreeString(n.nameOffset, strings), n.level, n.byteSize, n.metaFlags});
    }
    return nodes;
}

const ObjectInfo* SerializedFile::FindObject(std::int64_t pathId) const {
    auto it = std::lower_bound(objects_.begin(), objects_.end(), pathId,
                               [](const ObjectInfo& o, std::int64_t id) { return o.pathId < id; });
    return it != objects_.end() && it->pathId == pathId ? &*it : nullptr;
}

std::span<const std::uint8_t> SerializedFile::ObjectData(const ObjectInfo& object) const {
    return std::span<const std::uint8_t>(bytes_).subspan(dataOffset_ + object.byteStart, object.byteSize);
}

BinaryReader SerializedFile::OpenObject(const ObjectInfo& object) const {
    return BinaryReader(ObjectData(object), endian_);
}

} // namespace rr::assets
