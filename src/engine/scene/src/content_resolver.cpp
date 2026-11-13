#include "rr/scene/content_resolver.h"

#include "rr/assets/mesh.h"
#include "rr/assets/prefab.h"
#include "rr/assets/texture2d.h"

#include <exception>

namespace rr::scene {

namespace {

// The internal file name a pointer leads to (whether or not the file is available).
std::optional<std::string> TargetCab(assets::ContentLibrary& library, const std::string& fromCab,
                                     const assets::ObjectPointer& pointer) {
    if (pointer.fileId == 0) return fromCab;
    const assets::SerializedFile* from = library.FindFile(fromCab);
    if (!from) return std::nullopt;
    const auto index = static_cast<std::size_t>(pointer.fileId - 1);
    if (pointer.fileId < 0 || index >= from->Externals().size()) return std::nullopt;
    const std::string& path = from->Externals()[index].path;
    const auto slash = path.find_last_of('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

} // namespace

std::optional<ObjectKey> ContentResolver::Key(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    if (pointer.pathId == 0) return std::nullopt;
    auto cab = TargetCab(library_, fromCab, pointer);
    if (!cab) return std::nullopt;
    return ObjectKey{*cab, pointer.pathId};
}

std::optional<assets::ObjectRef> ContentResolver::Resolve(const std::string& fromCab,
                                                          const assets::ObjectPointer& pointer) {
    if (pointer.pathId == 0) return std::nullopt;
    auto ref = library_.Resolve(fromCab, pointer);
    if (!ref) {
        ++stats_.unresolvedPointers;
        if (auto cab = TargetCab(library_, fromCab, pointer); cab && !library_.FindFile(*cab)) {
            stats_.missingFiles.insert(*cab);
        }
    }
    return ref;
}

MeshId ContentResolver::Mesh(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    const auto key = Key(fromCab, pointer);
    if (!key) return kNoResource;
    if (auto it = meshes_.find(*key); it != meshes_.end()) return it->second;
    MeshId id = kNoResource;
    if (auto ref = Resolve(fromCab, pointer); ref && ref->object->classId == static_cast<std::int32_t>(assets::ClassId::Mesh)) {
        try {
            const assets::MeshAsset mesh = assets::ReadMesh(*library_.Decode(*ref), store_.Fetcher());
            id = resources_.AddMesh(ConvertMesh(mesh));
            ++stats_.meshesLoaded;
        } catch (const std::exception&) {
            ++stats_.meshesFailed;
        }
    }
    meshes_[*key] = id;
    return id;
}

TextureId ContentResolver::Texture(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    const auto key = Key(fromCab, pointer);
    if (!key) return kNoResource;
    if (auto it = textures_.find(*key); it != textures_.end()) return it->second;
    TextureId id = kNoResource;
    if (auto ref = Resolve(fromCab, pointer);
        ref && ref->object->classId == static_cast<std::int32_t>(assets::ClassId::Texture2D)) {
        try {
            id = resources_.AddTexture(assets::ReadTexture2D(*library_.Decode(*ref), store_.Fetcher()));
            ++stats_.texturesLoaded;
        } catch (const std::exception&) {
            ++stats_.texturesFailed;
        }
    }
    textures_[*key] = id;
    return id;
}

const assets::ShaderInfo* ContentResolver::Shader(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    const auto key = Key(fromCab, pointer);
    if (!key) return nullptr;
    if (auto it = shaders_.find(*key); it != shaders_.end()) return it->second.get();
    std::unique_ptr<assets::ShaderInfo> info;
    if (auto ref = Resolve(fromCab, pointer); ref && ref->object->classId == static_cast<std::int32_t>(assets::ClassId::Shader)) {
        try {
            info = std::make_unique<assets::ShaderInfo>(assets::ReadShaderInfo(*library_.Decode(*ref)));
        } catch (const std::exception&) {
            info.reset();
        }
    }
    return (shaders_[*key] = std::move(info)).get();
}

MaterialId ContentResolver::Material(const std::string& fromCab, const assets::ObjectPointer& pointer) {
    const auto key = Key(fromCab, pointer);
    if (!key) return kNoResource;
    if (auto it = materials_.find(*key); it != materials_.end()) return it->second;
    MaterialId id = kNoResource;
    auto ref = Resolve(fromCab, pointer);
    if (ref && ref->object->classId == static_cast<std::int32_t>(assets::ClassId::Material)) {
        try {
            const assets::MaterialAsset material = assets::ReadMaterial(*library_.Decode(*ref));
            const assets::ShaderInfo* shader = Shader(ref->cab, material.shader);
            MaterialDesc desc = MapMaterial(material, shader);
            TextureId albedo = kNoResource, normal = kNoResource;
            if (!desc.albedoProperty.empty()) albedo = Texture(ref->cab, material.textures.at(desc.albedoProperty).texture);
            if (!desc.normalProperty.empty()) normal = Texture(ref->cab, material.textures.at(desc.normalProperty).texture);
            FinishMaterial(desc, albedo, normal);
            ++stats_.shaderUse[desc.shaderName];
            ++stats_.matchKinds[desc.match];
            ++stats_.programUse[std::string(ToString(desc.shading)) + "/" + ToString(desc.surface) +
                                (desc.visible ? "" : " (hidden)")];
            id = resources_.AddMaterial(std::move(desc));
            ++stats_.materialsLoaded;
        } catch (const std::exception&) {
            ++stats_.materialsFailed;
        }
    }
    materials_[*key] = id;
    return id;
}

std::vector<assets::ObjectRef> FindRootGameObjects(assets::ContentLibrary& library, const std::string& cab) {
    std::vector<assets::ObjectRef> roots;
    const assets::SerializedFile* file = library.FindFile(cab);
    if (!file) return roots;
    for (const assets::ObjectInfo& object : file->Objects()) {
        if (object.classId != static_cast<std::int32_t>(assets::ClassId::Transform) && object.classId != 224) continue;
        const assets::ObjectRef transform{file, &object, cab};
        assets::ValuePtr value;
        try {
            value = library.Decode(transform);
        } catch (const std::exception&) {
            continue;
        }
        const auto father = assets::AsObjectPointer(value->Get("m_Father"));
        if (father && father->pathId != 0) continue;
        const auto go = assets::AsObjectPointer(value->Get("m_GameObject"));
        if (!go) continue;
        if (auto ref = library.Resolve(cab, *go)) roots.push_back(*ref);
    }
    return roots;
}

} // namespace rr::scene
