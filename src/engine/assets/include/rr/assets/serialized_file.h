// serialized_file.h - reader for the engine's serialized asset files.
//
// The game's content (scenes, prefabs, meshes, textures, game data) is stored in
// the original engine's "serialized file" container: a header, a metadata block
// describing object types and an object table, then the raw object data. This
// reader understands the container (format version 22, used by the game's
// engine version) and hands out each object's raw bytes plus its type. Decoding
// the bytes of specific object types happens in per-type readers on top.
#pragma once

#include "rr/assets/binary_reader.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rr::assets {

// Well-known engine class IDs used by the readers. Values are part of the file format.
enum class ClassId : std::int32_t {
    GameObject = 1,
    Transform = 4,
    Material = 21,
    Texture2D = 28,
    Mesh = 43,
    Shader = 48,
    TextAsset = 49,
    AudioClip = 83,
    MonoBehaviour = 114,
    MonoScript = 115,
    AssetBundle = 142,
    Sprite = 213,
};

const char* ClassName(std::int32_t classId); // "Unknown" for IDs we don't name yet

// One node of an embedded type description (present only when the file was
// written with type information; player builds usually strip it).
struct TypeTreeNode {
    std::string type;
    std::string name;
    int level = 0;
    std::int32_t byteSize = 0;
    std::uint32_t metaFlags = 0; // bit 0x4000: align to 4 bytes after this field
};

struct SerializedType {
    std::int32_t classId = 0;
    bool stripped = false;
    std::int16_t scriptTypeIndex = -1;
    std::array<std::uint8_t, 16> scriptHash{};
    std::array<std::uint8_t, 16> typeHash{};
    std::vector<TypeTreeNode> typeTree;
    // Only for reference types (managed [SerializeReference] types).
    std::string className, classNamespace, assemblyName;
};

struct ObjectInfo {
    std::int64_t pathId = 0;     // unique ID of the object within this file
    std::uint64_t byteStart = 0; // offset from the start of the data section
    std::uint32_t byteSize = 0;
    std::int32_t typeIndex = 0;  // index into SerializedFile::Types()
    std::int32_t classId = 0;
};

struct ExternalReference {
    std::array<std::uint8_t, 16> guid{};
    std::int32_t type = 0;
    std::string path; // referenced file, e.g. "globalgamemanagers.assets"
};

// A pointer to an object, possibly in another file (fileId 0 = this file,
// otherwise 1-based index into Externals()).
struct ObjectPointer {
    std::int32_t fileId = 0;
    std::int64_t pathId = 0;
};

class SerializedFile {
public:
    // Parses the container. Throws ReadError on malformed or unsupported files.
    explicit SerializedFile(std::vector<std::uint8_t> bytes);

    std::uint32_t FormatVersion() const { return version_; }
    const std::string& EngineVersion() const { return engineVersion_; }
    std::int32_t TargetPlatform() const { return targetPlatform_; }
    bool HasTypeTrees() const { return typeTreesEnabled_; }

    const std::vector<SerializedType>& Types() const { return types_; }
    const std::vector<ObjectInfo>& Objects() const { return objects_; }
    const std::vector<ExternalReference>& Externals() const { return externals_; }
    const std::vector<ObjectPointer>& ScriptTypes() const { return scriptTypes_; }

    const ObjectInfo* FindObject(std::int64_t pathId) const;

    // Raw serialized bytes of one object.
    std::span<const std::uint8_t> ObjectData(const ObjectInfo& object) const;

    // A reader over one object's bytes, using the file's byte order.
    BinaryReader OpenObject(const ObjectInfo& object) const;

private:
    void ReadHeader(BinaryReader& r);
    void ReadMetadata(BinaryReader& r);
    SerializedType ReadType(BinaryReader& r, bool isReferenceType);
    std::vector<TypeTreeNode> ReadTypeTree(BinaryReader& r);

    std::vector<std::uint8_t> bytes_;
    std::uint32_t version_ = 0;
    std::uint64_t dataOffset_ = 0;
    Endian endian_ = Endian::Little;
    std::string engineVersion_;
    std::int32_t targetPlatform_ = 0;
    bool typeTreesEnabled_ = false;
    std::vector<SerializedType> types_;
    std::vector<ObjectInfo> objects_;
    std::vector<ObjectPointer> scriptTypes_;
    std::vector<ExternalReference> externals_;
    std::vector<SerializedType> referenceTypes_;
};

} // namespace rr::assets
