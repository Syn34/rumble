#include "rr/scene/resources.h"

#include <algorithm>
#include <cmath>

namespace rr::scene {

namespace {

std::uint8_t ToByte(float v) {
    return static_cast<std::uint8_t>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
}

Aabb MirrorBounds(const assets::Bounds& b) {
    const Vec3 center{-b.center[0], b.center[1], b.center[2]};
    const Vec3 extent{std::fabs(b.extent[0]), std::fabs(b.extent[1]), std::fabs(b.extent[2])};
    return Aabb::FromCenterExtent(center, extent);
}

} // namespace

MeshData ConvertMesh(const assets::MeshAsset& mesh) {
    MeshData out;
    out.name = mesh.name;
    const std::uint32_t count = std::min<std::uint32_t>(mesh.vertexCount, static_cast<std::uint32_t>(mesh.positions.size()));
    out.vertices.resize(count);
    const bool hasNormals = mesh.normals.size() >= count;
    const bool hasTangents = mesh.tangents.size() >= count;
    const bool hasColors = mesh.colors.size() >= count;
    const bool hasUv = mesh.uvDimension[0] >= 2;
    for (std::uint32_t v = 0; v < count; ++v) {
        MeshVertex& o = out.vertices[v];
        const auto& p = mesh.positions[v];
        o.position[0] = -p[0];
        o.position[1] = p[1];
        o.position[2] = p[2];
        if (hasNormals) {
            o.normal[0] = -mesh.normals[v][0];
            o.normal[1] = mesh.normals[v][1];
            o.normal[2] = mesh.normals[v][2];
        } else {
            o.normal[0] = 0;
            o.normal[1] = 1;
            o.normal[2] = 0;
        }
        if (hasTangents) {
            // Mirroring both normal and tangent turns cross(n, t) around, so the
            // bitangent sign flips to keep the same bitangent.
            o.tangent[0] = -mesh.tangents[v][0];
            o.tangent[1] = mesh.tangents[v][1];
            o.tangent[2] = mesh.tangents[v][2];
            o.tangent[3] = -mesh.tangents[v][3];
        } else {
            o.tangent[0] = -1;
            o.tangent[1] = 0;
            o.tangent[2] = 0;
            o.tangent[3] = -1;
        }
        const assets::Float2 uv = hasUv ? mesh.Uv(0, v) : assets::Float2{0, 0};
        o.uv[0] = uv[0];
        o.uv[1] = uv[1];
        for (std::size_t c = 0; c < 4; ++c) {
            o.color[c] = hasColors ? ToByte(mesh.colors[v][c]) : 255;
        }
    }

    if (count > 0 && mesh.boneWeights.size() >= count && mesh.boneIndices.size() >= count) {
        out.skin.resize(count);
        for (std::uint32_t v = 0; v < count; ++v) {
            for (std::size_t i = 0; i < 4; ++i) {
                out.skin[v].boneIndices[i] = static_cast<float>(mesh.boneIndices[v][i]);
                out.skin[v].boneWeights[i] = mesh.boneWeights[v][i];
            }
        }
    }

    for (const assets::SubMesh& sm : mesh.subMeshes) {
        SubMeshRange range;
        range.bounds = MirrorBounds(sm.bounds);
        if (sm.topology != assets::MeshTopology::Triangles ||
            static_cast<std::size_t>(sm.firstIndex) + sm.indexCount > mesh.indices.size()) {
            ++out.skippedSubMeshes;
            out.subMeshes.push_back(range);
            continue;
        }
        range.firstIndex = static_cast<std::uint32_t>(out.indices.size());
        const std::uint32_t triangles = sm.indexCount / 3;
        range.indexCount = triangles * 3;
        const auto vertex = [&](std::size_t i) {
            return static_cast<std::uint32_t>(static_cast<std::int64_t>(mesh.indices[i]) + sm.baseVertex);
        };
        for (std::uint32_t t = 0; t < triangles; ++t) {
            const std::size_t at = sm.firstIndex + t * 3u;
            // Mirroring reverses the apparent winding; swapping two corners restores it.
            out.indices.push_back(vertex(at));
            out.indices.push_back(vertex(at + 2));
            out.indices.push_back(vertex(at + 1));
        }
        out.subMeshes.push_back(range);
    }

    for (const auto& pose : mesh.bindPoses) {
        Mat4 m;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col) m.At(row, col) = pose[static_cast<std::size_t>(row * 4 + col)];
        out.bindPoses.push_back(MirrorMatrix(m));
    }
    out.boneNameHashes = mesh.boneNameHashes;
    out.bounds = MirrorBounds(mesh.localBounds);
    const Vec3 e = out.bounds.Extent();
    if (count > 0 && e.x == 0 && e.y == 0 && e.z == 0) {
        out.bounds = {};
        for (const MeshVertex& v : out.vertices) out.bounds.Add(Vec3{v.position[0], v.position[1], v.position[2]});
    }
    out.blendShapeCount = mesh.blendShapeChannels.size();
    return out;
}

MeshId ResourceLibrary::AddMesh(MeshData mesh) {
    meshes_.push_back(std::make_unique<MeshData>(std::move(mesh)));
    return static_cast<MeshId>(meshes_.size() - 1);
}

TextureId ResourceLibrary::AddTexture(assets::Texture2DAsset texture) {
    textures_.push_back(std::make_unique<assets::Texture2DAsset>(std::move(texture)));
    return static_cast<TextureId>(textures_.size() - 1);
}

MaterialId ResourceLibrary::AddMaterial(MaterialDesc material) {
    materials_.push_back(std::move(material));
    return static_cast<MaterialId>(materials_.size() - 1);
}

MaterialId ResourceLibrary::DefaultMaterial() {
    if (defaultMaterial_ == kNoResource) {
        MaterialDesc m;
        m.name = "(default)";
        m.color = {0.35f, 0.35f, 0.35f, 1.0f};
        defaultMaterial_ = AddMaterial(std::move(m));
    }
    return defaultMaterial_;
}

} // namespace rr::scene
