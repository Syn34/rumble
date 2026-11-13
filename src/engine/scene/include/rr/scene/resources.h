// resources.h - meshes, textures and materials in the renderer's convention.
//
// The scene layer converts the game's assets once, when they are loaded: meshes
// are mirrored into the right-handed convention (see rr/scene/math.h) and
// interleaved, materials are mapped onto the PC shader programs
// (rr/scene/material_mapping.h). The library owns the converted data; scenes and
// the renderer refer to entries by index.
#pragma once

#include "rr/assets/mesh.h"
#include "rr/assets/texture2d.h"
#include "rr/scene/ids.h"
#include "rr/scene/material_mapping.h"
#include "rr/scene/math.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace rr::scene {

// One vertex as the scene shaders read it (locations 0..4).
struct MeshVertex {
    float position[3];
    float normal[3];
    float tangent[4]; // w: bitangent sign
    float uv[2];
    std::uint8_t color[4];
};

// Skinning inputs (locations 5 and 6), stored after MeshVertex for skinned meshes.
struct SkinVertex {
    float boneIndices[4];
    float boneWeights[4];
};

struct SubMeshRange {
    std::uint32_t firstIndex = 0;
    std::uint32_t indexCount = 0; // 0 for submeshes that can't be drawn (non-triangle topology)
    Aabb bounds;
};

struct MeshData {
    std::string name;
    std::vector<MeshVertex> vertices;
    std::vector<SkinVertex> skin;       // empty, or one per vertex
    std::vector<std::uint32_t> indices; // absolute (base vertex applied), counter-clockwise
    std::vector<SubMeshRange> subMeshes;
    std::vector<Mat4> bindPoses;        // mirrored
    std::vector<std::uint32_t> boneNameHashes;
    Aabb bounds;                        // mirrored local bounds
    std::size_t blendShapeCount = 0;    // not applied yet
    std::size_t skippedSubMeshes = 0;   // non-triangle topologies

    bool HasSkin() const { return !skin.empty(); }
};

// Converts a mesh from the game's convention: mirrors X, flips the bitangent sign
// and every triangle's winding, applies base vertices, transposes and mirrors the
// bind poses. Vertex colours default to white, missing normals to +Y.
MeshData ConvertMesh(const assets::MeshAsset& mesh);

class ResourceLibrary {
public:
    MeshId AddMesh(MeshData mesh);
    TextureId AddTexture(assets::Texture2DAsset texture);
    MaterialId AddMaterial(MaterialDesc material);

    const MeshData& Mesh(MeshId id) const { return *meshes_[id]; }
    const assets::Texture2DAsset& Texture(TextureId id) const { return *textures_[id]; }
    const MaterialDesc& Material(MaterialId id) const { return materials_[id]; }
    std::size_t MeshCount() const { return meshes_.size(); }
    std::size_t TextureCount() const { return textures_.size(); }
    std::size_t MaterialCount() const { return materials_.size(); }

    // A plain lit grey material for renderers whose material is missing.
    MaterialId DefaultMaterial();

private:
    // Held through pointers so references stay valid while the library grows.
    std::vector<std::unique_ptr<MeshData>> meshes_;
    std::vector<std::unique_ptr<assets::Texture2DAsset>> textures_;
    std::vector<MaterialDesc> materials_;
    MaterialId defaultMaterial_ = kNoResource;
};

} // namespace rr::scene
