// mesh.h - reader for Mesh objects.
//
// A mesh stores its vertices either as interleaved vertex streams (VertexData:
// up to 14 channels, each with a stream, byte offset, component format and
// dimension; the bytes are in the object or streamed from a resource blob) or,
// when mesh compression is enabled at build time, as quantised bit-packed arrays
// (CompressedMesh). The game's meshes all use the compressed form. Both are
// decoded here into plain float arrays.
//
// Data is returned in the engine's original convention (left-handed, Y up,
// clockwise front faces); converting to the renderer's convention is up to the
// caller.
#pragma once

#include "rr/assets/streamed_data.h"
#include "rr/assets/type_tree_value.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace rr::assets {

using Float2 = std::array<float, 2>;
using Float3 = std::array<float, 3>;
using Float4 = std::array<float, 4>;

struct Bounds {
    Float3 center{};
    Float3 extent{};
};

enum class MeshTopology { Triangles = 0, TriangleStrip = 1, Quads = 2, Lines = 3, LineStrip = 4, Points = 5 };

struct SubMesh {
    std::uint32_t firstIndex = 0; // index into MeshAsset::indices
    std::uint32_t indexCount = 0;
    std::int32_t baseVertex = 0;  // added to every index of this submesh
    std::uint32_t firstVertex = 0;
    std::uint32_t vertexCount = 0;
    MeshTopology topology = MeshTopology::Triangles;
    Bounds bounds;
};

struct BlendShapeVertex {
    Float3 position{}; // deltas
    Float3 normal{};
    Float3 tangent{};
    std::uint32_t index = 0; // vertex the deltas apply to
};

struct BlendShapeFrame {
    std::uint32_t firstVertex = 0; // into MeshAsset::blendShapeVertices
    std::uint32_t vertexCount = 0;
    bool hasNormals = false;
    bool hasTangents = false;
};

struct BlendShapeChannel {
    std::string name;
    std::uint32_t nameHash = 0;
    std::int32_t frameIndex = 0;
    std::int32_t frameCount = 0;
};

// Vertex channels in the order the engine numbers them.
enum class VertexChannel {
    Position = 0,
    Normal = 1,
    Tangent = 2,
    Color = 3,
    TexCoord0 = 4, // TexCoord0..7 = 4..11
    BlendWeight = 12,
    BlendIndices = 13,
};
inline constexpr int kVertexChannelCount = 14;
inline constexpr int kMaxTexCoords = 8;

// Component formats of VertexData channels.
enum class VertexComponentFormat : std::uint8_t {
    Float = 0,
    Float16 = 1,
    UNorm8 = 2,
    SNorm8 = 3,
    UNorm16 = 4,
    SNorm16 = 5,
    UInt8 = 6,
    SInt8 = 7,
    UInt16 = 8,
    SInt16 = 9,
    UInt32 = 10,
    SInt32 = 11,
};

struct ChannelInfo {
    std::uint8_t stream = 0;
    std::uint8_t offset = 0;
    std::uint8_t format = 0;    // VertexComponentFormat
    std::uint8_t dimension = 0; // low 4 bits: component count (0 = channel absent)
};

struct MeshAsset {
    std::string name;
    std::uint32_t vertexCount = 0;
    std::vector<Float3> positions;
    std::vector<Float3> normals;   // empty if the mesh has none
    std::vector<Float4> tangents;  // w = bitangent sign
    std::vector<Float4> colors;
    std::array<std::vector<float>, kMaxTexCoords> uvs{}; // uvDimension[i] floats per vertex
    std::array<int, kMaxTexCoords> uvDimension{};
    std::vector<Float4> boneWeights;
    std::vector<std::array<std::uint32_t, 4>> boneIndices;

    std::vector<std::uint32_t> indices;
    bool wideIndices = false; // the original index buffer was 32-bit
    std::vector<SubMesh> subMeshes;

    std::vector<std::array<float, 16>> bindPoses; // row-major (element [row * 4 + col])
    std::vector<std::uint32_t> boneNameHashes;
    std::uint32_t rootBoneNameHash = 0;
    Bounds localBounds;

    std::vector<BlendShapeVertex> blendShapeVertices;
    std::vector<BlendShapeFrame> blendShapeFrames;
    std::vector<BlendShapeChannel> blendShapeChannels;
    std::vector<float> blendShapeFullWeights;

    bool wasCompressed = false; // decoded from the CompressedMesh form

    Float2 Uv(int channel, std::uint32_t vertex) const;
};

// Reads a decoded Mesh object. Streamed vertex data is loaded through `fetch`.
// Throws ReadError on inconsistent data.
MeshAsset ReadMesh(const Value& mesh, const ResourceFetcher& fetch);

// --- building blocks (exposed for tests) ------------------------------------------

// Size in bytes of one component of the given format (0 if unknown).
int VertexComponentSize(std::uint8_t format);

// Decodes interleaved vertex streams into `out` (positions, normals, ...).
// Streams follow each other in stream order, each starting at a 16-byte boundary.
void DecodeVertexData(std::span<const ChannelInfo> channels, std::span<const std::uint8_t> data,
                      std::uint32_t vertexCount, MeshAsset& out);

// A bit-packed array of quantised values.
struct PackedBitVector {
    std::uint32_t numItems = 0;
    float range = 0.0f;
    float start = 0.0f;
    std::vector<std::uint8_t> data;
    std::uint8_t bitSize = 0;
};

// Unpacks numItems values of bitSize bits each (LSB-first).
std::vector<std::uint32_t> UnpackInts(const PackedBitVector& packed);
// Unpacks numItems values and maps them linearly onto [start, start + range].
std::vector<float> UnpackFloats(const PackedBitVector& packed);

struct CompressedMesh {
    PackedBitVector vertices, uv, normals, tangents, weights, normalSigns, tangentSigns, floatColors,
        boneIndices, triangles;
    std::uint32_t uvInfo = 0;
};

// Decodes the compressed vertex arrays and triangle list into `out`.
void DecodeCompressedMesh(const CompressedMesh& compressed, MeshAsset& out);

} // namespace rr::assets
