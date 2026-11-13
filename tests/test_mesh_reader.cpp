// Mesh reader: vertex streams, quantised (bit-packed) meshes, index buffers and
// submeshes, all on data built here.
#include "test_framework.h"

#include "rr/assets/mesh.h"

#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

using namespace rr::assets;

namespace {

bool Near(float a, float b, float tolerance = 1e-4f) { return std::fabs(a - b) <= tolerance; }

// Packs values LSB-first, `bits` bits each (written independently of the reader).
PackedBitVector Pack(const std::vector<std::uint32_t>& values, int bits) {
    PackedBitVector p;
    p.numItems = static_cast<std::uint32_t>(values.size());
    p.bitSize = static_cast<std::uint8_t>(bits);
    p.data.assign((values.size() * static_cast<std::size_t>(bits) + 7) / 8, 0);
    std::size_t bit = 0;
    for (std::uint32_t v : values) {
        for (int i = 0; i < bits; ++i, ++bit) {
            if ((v >> i) & 1) p.data[bit / 8] |= static_cast<std::uint8_t>(1 << (bit % 8));
        }
    }
    return p;
}

// Quantises floats onto `bits` bits over [start, start + range].
PackedBitVector PackFloats(const std::vector<float>& values, int bits, float start, float range) {
    std::vector<std::uint32_t> q;
    const float maxValue = static_cast<float>((1u << bits) - 1);
    for (float v : values) q.push_back(static_cast<std::uint32_t>(std::lround((v - start) / range * maxValue)));
    PackedBitVector p = Pack(q, bits);
    p.start = start;
    p.range = range;
    return p;
}

void PutHalf(std::vector<std::uint8_t>& out, std::uint16_t h) {
    out.push_back(static_cast<std::uint8_t>(h & 0xFF));
    out.push_back(static_cast<std::uint8_t>(h >> 8));
}

template <typename T> void Put(std::vector<std::uint8_t>& out, T v) {
    std::uint8_t b[sizeof(T)];
    std::memcpy(b, &v, sizeof(T));
    out.insert(out.end(), b, b + sizeof(T));
}

ValuePtr Leaf(std::int64_t v) {
    auto p = std::make_shared<Value>();
    p->data = v;
    return p;
}
ValuePtr Real(double v) {
    auto p = std::make_shared<Value>();
    p->data = v;
    return p;
}
ValuePtr Text(std::string s) {
    auto p = std::make_shared<Value>();
    p->data = std::move(s);
    return p;
}
ValuePtr Bytes(std::vector<std::uint8_t> b) {
    auto p = std::make_shared<Value>();
    p->data = Value::Bytes(std::move(b));
    return p;
}
ValuePtr Struct(std::vector<Value::Field> fields) {
    auto p = std::make_shared<Value>();
    p->data = Value::Struct(std::move(fields));
    return p;
}
ValuePtr Array(std::vector<ValuePtr> items) {
    auto p = std::make_shared<Value>();
    p->data = Value::Array(std::move(items));
    return p;
}
ValuePtr Vec3(double x, double y, double z) { return Struct({{"x", Real(x)}, {"y", Real(y)}, {"z", Real(z)}}); }

} // namespace

TEST_CASE("mesh: packed bit vectors") {
    const std::vector<std::uint32_t> values = {5, 3, 7, 0, 6, 1, 2, 4, 7};
    const PackedBitVector p = Pack(values, 3);
    CHECK(p.data.size() == 4);
    CHECK(UnpackInts(p) == values);

    const std::vector<std::uint32_t> wide = {0x1ABCD, 0x00001, 0x1FFFF};
    CHECK(UnpackInts(Pack(wide, 17)) == wide);

    PackedBitVector f = Pack({0, 255, 51}, 8);
    f.start = -1.0f;
    f.range = 2.0f;
    const auto floats = UnpackFloats(f);
    CHECK(Near(floats[0], -1.0f) && Near(floats[1], 1.0f) && Near(floats[2], -0.6f));

    PackedBitVector truncated = Pack(values, 3);
    truncated.data.pop_back();
    bool threw = false;
    try {
        UnpackInts(truncated);
    } catch (const ReadError&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("mesh: compressed mesh with quantised channels") {
    CompressedMesh c;
    const std::vector<float> positions = {-1, 0, 2, 1, 0.5f, 2, 0, 1, -2};
    c.vertices = PackFloats(positions, 16, -2.0f, 4.0f);
    // Two UV channels: UV0 2D, UV1 2D (uvInfo nibbles 0b0101 each), channel-major.
    c.uv = PackFloats({0, 0, 1, 0, 0.5f, 1, /* uv1 */ 0.25f, 0.25f, 0.75f, 0.25f, 0.5f, 0.75f}, 10, 0.0f, 1.0f);
    c.uvInfo = 0x55;
    // Normals (x, y) with z signs: (0, 0, +1), (0.6, 0, -0.8), (0, -0.6, +0.8).
    c.normals = PackFloats({0, 0, 0.6f, 0, 0, -0.6f}, 8, -1.0f, 2.0f);
    c.normalSigns = Pack({1, 0, 1}, 1);
    // Tangents (x, y) with (z sign, w) per vertex.
    c.tangents = PackFloats({1, 0, 0, 1, 0.8f, 0}, 8, -1.0f, 2.0f);
    c.tangentSigns = Pack({1, 1, 1, 0, 0, 1}, 1);
    // Skin weights: v0 one bone (31), v1 two bones (10 + 21), v2 three stored + one implied.
    c.weights = Pack({31, 10, 21, 5, 6, 7}, 5);
    c.boneIndices = Pack({2, 0, 1, 3, 4, 5, 6}, 4);
    c.triangles = Pack({0, 1, 2}, 2);

    MeshAsset m;
    DecodeCompressedMesh(c, m);
    CHECK(m.wasCompressed && m.vertexCount == 3);
    const float posTolerance = 4.0f / 65535.0f;
    for (std::size_t i = 0; i < 9; ++i) CHECK(Near(m.positions[i / 3][i % 3], positions[i], posTolerance));

    CHECK(m.uvDimension[0] == 2 && m.uvDimension[1] == 2 && m.uvDimension[2] == 0);
    CHECK(Near(m.Uv(0, 1)[0], 1.0f, 1e-3f) && Near(m.Uv(0, 2)[1], 1.0f, 1e-3f));
    CHECK(Near(m.Uv(1, 0)[0], 0.25f, 1e-3f) && Near(m.Uv(1, 2)[1], 0.75f, 1e-3f));

    const float nTol = 0.02f;
    CHECK(Near(m.normals[0][2], 1.0f, nTol));
    CHECK(Near(m.normals[1][0], 0.6f, nTol) && Near(m.normals[1][2], -0.8f, nTol));
    CHECK(Near(m.normals[2][1], -0.6f, nTol) && Near(m.normals[2][2], 0.8f, nTol));

    CHECK(Near(m.tangents[0][0], 1.0f, nTol) && Near(m.tangents[0][3], 1.0f));
    CHECK(Near(m.tangents[1][1], 1.0f, nTol) && Near(m.tangents[1][3], -1.0f));
    CHECK(Near(m.tangents[2][0], 0.8f, nTol) && Near(m.tangents[2][2], -0.6f, nTol) && Near(m.tangents[2][3], 1.0f));

    CHECK(Near(m.boneWeights[0][0], 1.0f) && m.boneIndices[0][0] == 2);
    CHECK(Near(m.boneWeights[1][0], 10.0f / 31) && Near(m.boneWeights[1][1], 21.0f / 31));
    CHECK(m.boneIndices[1][0] == 0 && m.boneIndices[1][1] == 1 && Near(m.boneWeights[1][2], 0.0f));
    CHECK(Near(m.boneWeights[2][0], 5.0f / 31) && Near(m.boneWeights[2][3], 13.0f / 31));
    CHECK(m.boneIndices[2][0] == 3 && m.boneIndices[2][3] == 6);

    CHECK((m.indices == std::vector<std::uint32_t>{0, 1, 2}));
}

TEST_CASE("mesh: vertex streams with mixed component formats") {
    // Stream 0: position Float x3 @0, normal SNorm16 x3 @12 (stride 18).
    // Stream 1: colour UNorm8 x4 @0, UV0 Float16 x2 @4, bone indices UInt8 x4 @8 (stride 12).
    std::vector<ChannelInfo> channels(kVertexChannelCount);
    channels[0] = {0, 0, static_cast<std::uint8_t>(VertexComponentFormat::Float), 3};
    channels[1] = {0, 12, static_cast<std::uint8_t>(VertexComponentFormat::SNorm16), 3};
    channels[3] = {1, 0, static_cast<std::uint8_t>(VertexComponentFormat::UNorm8), 4};
    channels[4] = {1, 4, static_cast<std::uint8_t>(VertexComponentFormat::Float16), 2};
    channels[13] = {1, 8, static_cast<std::uint8_t>(VertexComponentFormat::UInt8), 4};

    const std::uint32_t vertexCount = 3;
    std::vector<std::uint8_t> data;
    for (std::uint32_t v = 0; v < vertexCount; ++v) {
        Put<float>(data, static_cast<float>(v));
        Put<float>(data, -static_cast<float>(v));
        Put<float>(data, 0.5f);
        Put<std::int16_t>(data, 32767);
        Put<std::int16_t>(data, -32767);
        Put<std::int16_t>(data, static_cast<std::int16_t>(v == 2 ? -32768 : 0));
    }
    while (data.size() % 16) data.push_back(0xCD); // stream 1 starts on a 16-byte boundary
    CHECK(data.size() == 64);
    for (std::uint32_t v = 0; v < vertexCount; ++v) {
        data.insert(data.end(), {255, 0, 51, static_cast<std::uint8_t>(v)});
        PutHalf(data, 0x3800); // 0.5
        PutHalf(data, v == 1 ? 0xC000 : 0x0001); // -2, smallest subnormal
        data.insert(data.end(), {static_cast<std::uint8_t>(v), 7, 8, 9});
    }

    MeshAsset m;
    DecodeVertexData(channels, data, vertexCount, m);
    CHECK(m.vertexCount == 3 && m.positions.size() == 3);
    CHECK(Near(m.positions[2][0], 2.0f) && Near(m.positions[2][1], -2.0f) && Near(m.positions[1][2], 0.5f));
    CHECK(Near(m.normals[0][0], 1.0f) && Near(m.normals[0][1], -1.0f) && Near(m.normals[2][2], -1.0f));
    CHECK(Near(m.colors[1][0], 1.0f) && Near(m.colors[1][2], 0.2f) && Near(m.colors[2][3], 2.0f / 255));
    CHECK(m.uvDimension[0] == 2 && Near(m.Uv(0, 0)[0], 0.5f) && Near(m.Uv(0, 1)[1], -2.0f));
    CHECK(m.Uv(0, 0)[1] > 0.0f && m.Uv(0, 0)[1] < 1e-7f);
    CHECK(m.boneIndices[2][0] == 2 && m.boneIndices[0][3] == 9);
    CHECK(m.boneWeights.size() == 3 && Near(m.boneWeights[0][0], 1.0f)); // indices without weights

    bool threw = false;
    try {
        MeshAsset shortMesh;
        DecodeVertexData(channels, std::span(data).first(80), vertexCount, shortMesh);
    } catch (const ReadError&) {
        threw = true;
    }
    CHECK(threw);
}

TEST_CASE("mesh: reader with streamed vertex data, index buffer and submeshes") {
    // Four vertices (Float3 positions only), streamed from a resource blob.
    std::vector<std::uint8_t> vertices;
    const float corners[4][3] = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    for (const auto& c : corners) {
        for (float f : c) Put<float>(vertices, f);
    }
    std::vector<std::uint8_t> blob = {1, 2, 3};
    blob.insert(blob.end(), vertices.begin(), vertices.end());

    std::vector<std::uint8_t> indexBytes;
    for (int i : {0, 1, 2, 0, 2, 3}) Put<std::uint16_t>(indexBytes, static_cast<std::uint16_t>(i));

    std::vector<ValuePtr> channels;
    for (int i = 0; i < kVertexChannelCount; ++i) {
        channels.push_back(Struct({{"stream", Leaf(0)}, {"offset", Leaf(0)}, {"format", Leaf(0)},
                                   {"dimension", Leaf(i == 0 ? 3 : 0)}}));
    }
    auto subMesh = [](int firstByte, int count) {
        return Struct({{"firstByte", Leaf(firstByte)}, {"indexCount", Leaf(count)}, {"topology", Leaf(0)},
                       {"baseVertex", Leaf(0)}, {"firstVertex", Leaf(0)}, {"vertexCount", Leaf(4)},
                       {"localAABB", Struct({{"m_Center", Vec3(0.5, 0.5, 0)}, {"m_Extent", Vec3(0.5, 0.5, 0)}})}});
    };
    std::vector<ValuePtr> bindPose;
    {
        std::vector<Value::Field> m;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                m.push_back({"e" + std::to_string(r) + std::to_string(c), Real(r == c ? 1.0 : (r == 0 && c == 3 ? 5.0 : 0.0))});
        bindPose.push_back(Struct(std::move(m)));
    }
    auto mesh = Struct({
        {"m_Name", Text("quad")},
        {"m_SubMeshes", Array({subMesh(0, 3), subMesh(6, 3)})},
        {"m_Shapes", Struct({{"vertices", Array({Struct({{"vertex", Vec3(0, 0, 1)}, {"normal", Vec3(0, 0, 0)},
                                                          {"tangent", Vec3(0, 0, 0)}, {"index", Leaf(2)}})})},
                             {"shapes", Array({Struct({{"firstVertex", Leaf(0)}, {"vertexCount", Leaf(1)},
                                                       {"hasNormals", Leaf(0)}, {"hasTangents", Leaf(0)}})})},
                             {"channels", Array({Struct({{"name", Text("bulge")}, {"nameHash", Leaf(77)},
                                                         {"frameIndex", Leaf(0)}, {"frameCount", Leaf(1)}})})},
                             {"fullWeights", Array({Real(100.0)})}})},
        {"m_BindPose", Array(bindPose)},
        {"m_BoneNameHashes", Array({Leaf(1234)})},
        {"m_RootBoneNameHash", Leaf(99)},
        {"m_IndexFormat", Leaf(0)},
        {"m_IndexBuffer", Bytes(indexBytes)},
        {"m_VertexData", Struct({{"m_VertexCount", Leaf(4)}, {"m_Channels", Array(channels)}, {"m_DataSize", Bytes({})}})},
        {"m_CompressedMesh", Struct({{"m_Vertices", Struct({{"m_NumItems", Leaf(0)}})}})},
        {"m_LocalAABB", Struct({{"m_Center", Vec3(0.5, 0.5, 0)}, {"m_Extent", Vec3(0.5, 0.5, 0)}})},
        {"m_StreamData", Struct({{"offset", Leaf(3)}, {"size", Leaf(48)}, {"path", Text("archive:/CAB-q/CAB-q.resS")}})},
    });

    const ResourceFetcher fetch = [&](const StreamingInfo& info) {
        return std::vector<std::uint8_t>(blob.begin() + static_cast<std::ptrdiff_t>(info.offset),
                                         blob.begin() + static_cast<std::ptrdiff_t>(info.offset + info.size));
    };
    const MeshAsset m = ReadMesh(*mesh, fetch);
    CHECK(m.name == "quad" && m.vertexCount == 4 && !m.wasCompressed && !m.wideIndices);
    CHECK(Near(m.positions[2][0], 1.0f) && Near(m.positions[2][1], 1.0f) && Near(m.positions[3][0], 0.0f));
    CHECK((m.indices == std::vector<std::uint32_t>{0, 1, 2, 0, 2, 3}));
    CHECK(m.subMeshes.size() == 2 && m.subMeshes[1].firstIndex == 3 && m.subMeshes[1].indexCount == 3);
    CHECK(Near(m.subMeshes[0].bounds.extent[1], 0.5f));
    CHECK(m.bindPoses.size() == 1 && Near(m.bindPoses[0][3], 5.0f) && Near(m.bindPoses[0][15], 1.0f));
    CHECK(m.boneNameHashes.size() == 1 && m.boneNameHashes[0] == 1234 && m.rootBoneNameHash == 99);
    CHECK(m.blendShapeVertices.size() == 1 && m.blendShapeVertices[0].index == 2 && Near(m.blendShapeVertices[0].position[2], 1.0f));
    CHECK(m.blendShapeChannels.size() == 1 && m.blendShapeChannels[0].name == "bulge");
    CHECK(m.blendShapeFrames.size() == 1 && m.blendShapeFullWeights.size() == 1);
    CHECK(Near(m.localBounds.center[0], 0.5f));

    // A submesh pointing past the index buffer is rejected.
    mesh->data = Value::Struct{{"m_SubMeshes", Array({subMesh(8, 6)})}, {"m_IndexBuffer", Bytes(indexBytes)}};
    bool threw = false;
    try {
        ReadMesh(*mesh, fetch);
    } catch (const ReadError&) {
        threw = true;
    }
    CHECK(threw);
}
