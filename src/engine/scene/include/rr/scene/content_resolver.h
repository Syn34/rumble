// content_resolver.h - loads meshes, materials and textures for scenes from the
// game's content bundles (through assets::ContentLibrary), converting each once.
#pragma once

#include "rr/assets/content_library.h"
#include "rr/assets/material.h"
#include "rr/assets/streamed_data.h"
#include "rr/scene/prefab_instancer.h"
#include "rr/scene/resources.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace rr::scene {

struct ContentStats {
    std::size_t meshesLoaded = 0;
    std::size_t meshesFailed = 0;
    std::size_t materialsLoaded = 0;
    std::size_t materialsFailed = 0;
    std::size_t texturesLoaded = 0;
    std::size_t texturesFailed = 0;
    std::size_t unresolvedPointers = 0; // pointers into files that aren't available
    std::set<std::string> missingFiles; // internal names of those files
    // Per shader name ("" = shader file not available): materials using it.
    std::map<std::string, std::size_t> shaderUse;
    std::map<ShaderMatch, std::size_t> matchKinds;
    std::map<std::string, std::size_t> programUse; // "lit+normal/opaque" ...
};

class ContentResolver final : public AssetResolver {
public:
    ContentResolver(assets::ContentLibrary& library, assets::ResourceStore& store, ResourceLibrary& resources)
        : library_(library), store_(store), resources_(resources) {}

    std::optional<ObjectKey> Key(const std::string& fromCab, const assets::ObjectPointer& pointer) override;
    MeshId Mesh(const std::string& fromCab, const assets::ObjectPointer& pointer) override;
    MaterialId Material(const std::string& fromCab, const assets::ObjectPointer& pointer) override;
    TextureId Texture(const std::string& fromCab, const assets::ObjectPointer& pointer);

    const ContentStats& Stats() const { return stats_; }

private:
    std::optional<assets::ObjectRef> Resolve(const std::string& fromCab, const assets::ObjectPointer& pointer);
    const assets::ShaderInfo* Shader(const std::string& fromCab, const assets::ObjectPointer& pointer);

    assets::ContentLibrary& library_;
    assets::ResourceStore& store_;
    ResourceLibrary& resources_;
    std::map<ObjectKey, MeshId> meshes_;
    std::map<ObjectKey, MaterialId> materials_;
    std::map<ObjectKey, TextureId> textures_;
    std::map<ObjectKey, std::unique_ptr<assets::ShaderInfo>> shaders_; // null: unreadable
    ContentStats stats_;
};

// Game objects of one serialized file whose transform has no parent: the roots of
// the prefabs (or of a scene) stored in it.
std::vector<assets::ObjectRef> FindRootGameObjects(assets::ContentLibrary& library, const std::string& cab);

} // namespace rr::scene
