#include "rr/assets/mesh.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace rr::assets {

namespace {

std::int64_t IntField(const Value& v, const char* name, std::int64_t fallback = 0) {
    const Value* f = v.Get(name);
    return f ? f->AsInt(fallback) : fallback;
}

float FloatField(const Value& v, const char* name) {
    const Value* f = v.Get(name);
    return f ? static_cast<float>(f->AsDouble()) : 0.0f;
}

Float3 ReadVector3(const Value* v) {
    if (!v) return {};
    return {FloatField(*v, "x"), FloatField(*v, "y"), FloatField(*v, "z")};
}

Bounds ReadBounds(const Value* v) {
    if (!v) return {};
    return {ReadVector3(v->Get("m_Center")), ReadVector3(v->Get("m_Extent"))};
}

const Value::Array& ArrayField(const Value& v, const char* name) {
    static const Value::Array empty;
    const Value* f = v.Get(name);
    const Value::Array* a = f ? f->AsArray() : nullptr;
    return a ? *a : empty;
}

PackedBitVector ReadPackedBitVector(const Value* v) {
    PackedBitVector p;
    if (!v) return p;
    p.numItems = static_cast<std::uint32_t>(IntField(*v, "m_NumItems"));
    p.range = FloatField(*v, "m_Range");
    p.start = FloatField(*v, "m_Start");
    p.data = ValueBytes(v->Get("m_Data"));
    p.bitSize = static_cast<std::uint8_t>(IntField(*v, "m_BitSize"));
    return p;
}

float HalfToFloat(std::uint16_t h) {
    const std::uint32_t sign = static_cast<std::uint32_t>(h & 0x8000) << 16;
    std::uint32_t exponent = (h >> 10) & 0x1F;
    std::uint32_t mantissa = h & 0x3FF;
    std::uint32_t bits;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else { // subnormal: normalise
            exponent = 127 - 15 + 1;
            while ((mantissa & 0x400) == 0) {
                mantissa <<= 1;
                --exponent;
            }
            mantissa &= 0x3FF;
            bits = sign | (exponent << 23) | (mantissa << 13);
        }
    } else if (exponent == 31) {
        bits = sign | 0x7F800000u | (mantissa << 13);
    } else {
        bits = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
    }
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

template <typename T> T Load(const std::uint8_t* p) {
    T value;
    std::memcpy(&value, p, sizeof(T));
    return value;
}

// Reads one component and converts it to float (normalised formats map to
// [0, 1] / [-1, 1]; integer formats keep their integer value).
float ReadComponent(const std::uint8_t* p, std::uint8_t format) {
    switch (static_cast<VertexComponentFormat>(format)) {
    case VertexComponentFormat::Float: return Load<float>(p);
    case VertexComponentFormat::Float16: return HalfToFloat(Load<std::uint16_t>(p));
    case VertexComponentFormat::UNorm8: return static_cast<float>(p[0]) / 255.0f;
    case VertexComponentFormat::SNorm8:
        return std::max(static_cast<float>(static_cast<std::int8_t>(p[0])) / 127.0f, -1.0f);
    case VertexComponentFormat::UNorm16: return static_cast<float>(Load<std::uint16_t>(p)) / 65535.0f;
    case VertexComponentFormat::SNorm16:
        return std::max(static_cast<float>(Load<std::int16_t>(p)) / 32767.0f, -1.0f);
    case VertexComponentFormat::UInt8: return static_cast<float>(p[0]);
    case VertexComponentFormat::SInt8: return static_cast<float>(static_cast<std::int8_t>(p[0]));
    case VertexComponentFormat::UInt16: return static_cast<float>(Load<std::uint16_t>(p));
    case VertexComponentFormat::SInt16: return static_cast<float>(Load<std::int16_t>(p));
    case VertexComponentFormat::UInt32: return static_cast<float>(Load<std::uint32_t>(p));
    case VertexComponentFormat::SInt32: return static_cast<float>(Load<std::int32_t>(p));
    }
    return 0.0f;
}

std::uint32_t ReadIntegerComponent(const std::uint8_t* p, std::uint8_t format) {
    switch (static_cast<VertexComponentFormat>(format)) {
    case VertexComponentFormat::UInt8:
    case VertexComponentFormat::UNorm8:
    case VertexComponentFormat::SInt8:
    case VertexComponentFormat::SNorm8: return p[0];
    case VertexComponentFormat::UInt16:
    case VertexComponentFormat::UNorm16:
    case VertexComponentFormat::SInt16:
    case VertexComponentFormat::SNorm16: return Load<std::uint16_t>(p);
    case VertexComponentFormat::UInt32:
    case VertexComponentFormat::SInt32: return Load<std::uint32_t>(p);
    case VertexComponentFormat::Float: return static_cast<std::uint32_t>(Load<float>(p));
    case VertexComponentFormat::Float16: return static_cast<std::uint32_t>(HalfToFloat(Load<std::uint16_t>(p)));
    }
    return 0;
}

} // namespace

Float2 MeshAsset::Uv(int channel, std::uint32_t vertex) const {
    if (channel < 0 || channel >= kMaxTexCoords) return {};
    const auto c = static_cast<std::size_t>(channel);
    const auto dim = static_cast<std::size_t>(uvDimension[c]);
    if (dim == 0 || (vertex + 1) * dim > uvs[c].size()) return {};
    const float* p = &uvs[c][vertex * dim];
    return {p[0], dim > 1 ? p[1] : 0.0f};
}

int VertexComponentSize(std::uint8_t format) {
    switch (static_cast<VertexComponentFormat>(format)) {
    case VertexComponentFormat::Float:
    case VertexComponentFormat::UInt32:
    case VertexComponentFormat::SInt32: return 4;
    case VertexComponentFormat::Float16:
    case VertexComponentFormat::UNorm16:
    case VertexComponentFormat::SNorm16:
    case VertexComponentFormat::UInt16:
    case VertexComponentFormat::SInt16: return 2;
    case VertexComponentFormat::UNorm8:
    case VertexComponentFormat::SNorm8:
    case VertexComponentFormat::UInt8:
    case VertexComponentFormat::SInt8: return 1;
    }
    return 0;
}

void DecodeVertexData(std::span<const ChannelInfo> channels, std::span<const std::uint8_t> data,
                      std::uint32_t vertexCount, MeshAsset& out) {
    out.vertexCount = vertexCount;
    if (vertexCount == 0) return;

    // Stride of every stream, then each stream's start offset.
    int streamCount = 0;
    std::array<std::size_t, 16> strides{};
    for (const ChannelInfo& c : channels) {
        const int dim = c.dimension & 0xF;
        if (dim == 0) continue;
        const int size = VertexComponentSize(c.format);
        if (size == 0 || c.stream >= strides.size()) throw ReadError("mesh: unknown vertex channel format");
        strides[c.stream] = std::max<std::size_t>(strides[c.stream], c.offset + static_cast<std::size_t>(size * dim));
        streamCount = std::max(streamCount, c.stream + 1);
    }
    std::array<std::size_t, 16> streamOffsets{};
    std::size_t offset = 0;
    for (int s = 0; s < streamCount; ++s) {
        streamOffsets[static_cast<std::size_t>(s)] = offset;
        offset += strides[static_cast<std::size_t>(s)] * vertexCount;
        offset = (offset + 15) & ~std::size_t{15};
    }
    if (streamCount > 0) {
        const std::size_t last = static_cast<std::size_t>(streamCount - 1);
        if (streamOffsets[last] + strides[last] * vertexCount > data.size()) {
            throw ReadError("mesh: vertex data shorter than its channel layout");
        }
    }

    // Element pointer of channel `c` for vertex `v`.
    auto element = [&](const ChannelInfo& c, std::uint32_t v) {
        return data.data() + streamOffsets[c.stream] + strides[c.stream] * v + c.offset;
    };
    auto readFloats = [&](const ChannelInfo& c, std::uint32_t v, float* dst, int maxComponents) {
        const int dim = std::min(c.dimension & 0xF, maxComponents);
        const int size = VertexComponentSize(c.format);
        const std::uint8_t* p = element(c, v);
        for (int i = 0; i < dim; ++i) dst[i] = ReadComponent(p + i * size, c.format);
        return dim;
    };

    const std::size_t n = vertexCount;
    for (std::size_t ch = 0; ch < channels.size() && ch < kVertexChannelCount; ++ch) {
        const ChannelInfo& c = channels[ch];
        const int dim = c.dimension & 0xF;
        if (dim == 0) continue;
        switch (static_cast<int>(ch)) {
        case static_cast<int>(VertexChannel::Position):
            out.positions.assign(n, Float3{});
            for (std::uint32_t v = 0; v < vertexCount; ++v) readFloats(c, v, out.positions[v].data(), 3);
            break;
        case static_cast<int>(VertexChannel::Normal):
            out.normals.assign(n, Float3{});
            for (std::uint32_t v = 0; v < vertexCount; ++v) readFloats(c, v, out.normals[v].data(), 3);
            break;
        case static_cast<int>(VertexChannel::Tangent):
            out.tangents.assign(n, Float4{0, 0, 0, 1});
            for (std::uint32_t v = 0; v < vertexCount; ++v) readFloats(c, v, out.tangents[v].data(), 4);
            break;
        case static_cast<int>(VertexChannel::Color):
            out.colors.assign(n, Float4{1, 1, 1, 1});
            for (std::uint32_t v = 0; v < vertexCount; ++v) readFloats(c, v, out.colors[v].data(), 4);
            break;
        case static_cast<int>(VertexChannel::BlendWeight):
            out.boneWeights.assign(n, Float4{});
            for (std::uint32_t v = 0; v < vertexCount; ++v) readFloats(c, v, out.boneWeights[v].data(), 4);
            break;
        case static_cast<int>(VertexChannel::BlendIndices): {
            out.boneIndices.assign(n, {});
            const int size = VertexComponentSize(c.format);
            for (std::uint32_t v = 0; v < vertexCount; ++v) {
                const std::uint8_t* p = element(c, v);
                for (int i = 0; i < std::min(dim, 4); ++i) {
                    out.boneIndices[v][static_cast<std::size_t>(i)] = ReadIntegerComponent(p + i * size, c.format);
                }
            }
            break;
        }
        default: { // texture coordinates
            const auto uv = static_cast<std::size_t>(ch - static_cast<std::size_t>(VertexChannel::TexCoord0));
            out.uvDimension[uv] = std::min(dim, 4);
            out.uvs[uv].assign(n * static_cast<std::size_t>(out.uvDimension[uv]), 0.0f);
            for (std::uint32_t v = 0; v < vertexCount; ++v) {
                readFloats(c, v, &out.uvs[uv][v * static_cast<std::size_t>(out.uvDimension[uv])], 4);
            }
            break;
        }
        }
    }
    // Single-bone skinning may store indices without weights: the weight is 1.
    // TODO: check how partial weight channels (fewer weights than indices) are meant
    // to be completed; no mesh in the game uses uncompressed skinning data.
    if (!out.boneIndices.empty() && out.boneWeights.empty()) out.boneWeights.assign(n, Float4{1, 0, 0, 0});
}

std::vector<std::uint32_t> UnpackInts(const PackedBitVector& packed) {
    std::vector<std::uint32_t> out(packed.numItems, 0);
    const int bits = packed.bitSize;
    if (bits == 0) return out;
    if (bits > 32 || (static_cast<std::uint64_t>(packed.numItems) * static_cast<std::uint64_t>(bits) + 7) / 8 > packed.data.size()) {
        throw ReadError("mesh: packed bit vector shorter than its item count");
    }
    std::size_t bitPos = 0;
    for (std::uint32_t i = 0; i < packed.numItems; ++i) {
        std::uint64_t value = 0;
        for (int b = 0; b < bits;) {
            const std::size_t byte = bitPos >> 3;
            const int shift = static_cast<int>(bitPos & 7);
            const int take = std::min(bits - b, 8 - shift);
            const std::uint64_t chunk = (packed.data[byte] >> shift) & ((1u << take) - 1);
            value |= chunk << b;
            b += take;
            bitPos += static_cast<std::size_t>(take);
        }
        out[i] = static_cast<std::uint32_t>(value);
    }
    return out;
}

std::vector<float> UnpackFloats(const PackedBitVector& packed) {
    const std::vector<std::uint32_t> ints = UnpackInts(packed);
    std::vector<float> out(ints.size(), packed.start);
    if (packed.bitSize == 0) return out;
    const double maxValue = static_cast<double>((std::uint64_t{1} << packed.bitSize) - 1);
    for (std::size_t i = 0; i < ints.size(); ++i) {
        out[i] = static_cast<float>(static_cast<double>(ints[i]) / maxValue * packed.range + packed.start);
    }
    return out;
}

void DecodeCompressedMesh(const CompressedMesh& c, MeshAsset& out) {
    out.wasCompressed = true;
    const std::uint32_t vertexCount = c.vertices.numItems / 3;
    out.vertexCount = vertexCount;
    const std::size_t n = vertexCount;

    const std::vector<float> positions = UnpackFloats(c.vertices);
    out.positions.resize(n);
    for (std::size_t v = 0; v < n; ++v) out.positions[v] = {positions[v * 3], positions[v * 3 + 1], positions[v * 3 + 2]};

    // Texture coordinates: channel-major, channels described by uvInfo (4 bits each:
    // bits 0-1 = dimension - 1, bit 2 = present). Without uvInfo, older data holds
    // UV0 and optionally UV1, both 2D.
    if (c.uv.numItems > 0) {
        const std::vector<float> uv = UnpackFloats(c.uv);
        std::size_t cursor = 0;
        for (int ch = 0; ch < kMaxTexCoords; ++ch) {
            int dim = 0;
            if (c.uvInfo != 0) {
                const std::uint32_t info = (c.uvInfo >> (ch * 4)) & 0xF;
                if (info & 4) dim = static_cast<int>(info & 3) + 1;
            } else if (ch < 2 && uv.size() >= (static_cast<std::size_t>(ch) + 1) * 2 * n) {
                dim = 2;
            }
            if (dim == 0) continue;
            const std::size_t count = n * static_cast<std::size_t>(dim);
            if (cursor + count > uv.size()) throw ReadError("mesh: compressed UVs shorter than declared");
            const auto u = static_cast<std::size_t>(ch);
            out.uvDimension[u] = dim;
            out.uvs[u].assign(uv.begin() + static_cast<std::ptrdiff_t>(cursor),
                              uv.begin() + static_cast<std::ptrdiff_t>(cursor + count));
            cursor += count;
        }
    }

    // Normals: x and y stored, z rebuilt from unit length plus a sign bit.
    if (c.normals.numItems > 0) {
        const std::vector<float> xy = UnpackFloats(c.normals);
        const std::vector<std::uint32_t> signs = UnpackInts(c.normalSigns);
        if (xy.size() < n * 2) throw ReadError("mesh: compressed normals shorter than the vertex count");
        out.normals.resize(n);
        for (std::size_t v = 0; v < n; ++v) {
            const float x = xy[v * 2], y = xy[v * 2 + 1];
            float z = std::sqrt(std::max(0.0f, 1.0f - x * x - y * y));
            if (v < signs.size() && signs[v] == 0) z = -z;
            out.normals[v] = {x, y, z};
        }
    }

    // Tangents: same scheme, with two sign bits per vertex (z sign, then w).
    if (c.tangents.numItems > 0) {
        const std::vector<float> xy = UnpackFloats(c.tangents);
        const std::vector<std::uint32_t> signs = UnpackInts(c.tangentSigns);
        if (xy.size() < n * 2) throw ReadError("mesh: compressed tangents shorter than the vertex count");
        out.tangents.resize(n);
        for (std::size_t v = 0; v < n; ++v) {
            const float x = xy[v * 2], y = xy[v * 2 + 1];
            float z = std::sqrt(std::max(0.0f, 1.0f - x * x - y * y));
            float w = 1.0f;
            if (v * 2 + 1 < signs.size()) {
                if (signs[v * 2] == 0) z = -z;
                w = signs[v * 2 + 1] != 0 ? 1.0f : -1.0f;
            }
            out.tangents[v] = {x, y, z, w};
        }
    }

    if (c.floatColors.numItems > 0) {
        const std::vector<float> rgba = UnpackFloats(c.floatColors);
        if (rgba.size() >= n * 4) {
            out.colors.resize(n);
            for (std::size_t v = 0; v < n; ++v) out.colors[v] = {rgba[v * 4], rgba[v * 4 + 1], rgba[v * 4 + 2], rgba[v * 4 + 3]};
        }
    }

    // Skin weights: 5-bit weights out of 31, a variable number per vertex. A
    // vertex's list ends when its weights reach 31, or after three stored weights,
    // in which case the fourth is whatever remains. Each weight (stored or
    // implied) has a bone index.
    if (c.weights.numItems > 0) {
        const std::vector<std::uint32_t> weights = UnpackInts(c.weights);
        const std::vector<std::uint32_t> bones = UnpackInts(c.boneIndices);
        out.boneWeights.assign(n, Float4{});
        out.boneIndices.assign(n, {});
        std::size_t vertex = 0, slot = 0, bone = 0;
        std::uint32_t sum = 0;
        auto nextBone = [&]() {
            if (bone >= bones.size()) throw ReadError("mesh: fewer bone indices than weights");
            return bones[bone++];
        };
        for (std::size_t i = 0; i < weights.size() && vertex < n; ++i) {
            out.boneWeights[vertex][slot] = static_cast<float>(weights[i]) / 31.0f;
            out.boneIndices[vertex][slot] = nextBone();
            ++slot;
            sum += weights[i];
            if (sum >= 31) {
                ++vertex;
                slot = 0;
                sum = 0;
            } else if (slot == 3) {
                out.boneWeights[vertex][3] = static_cast<float>(31 - sum) / 31.0f;
                out.boneIndices[vertex][3] = nextBone();
                ++vertex;
                slot = 0;
                sum = 0;
            }
        }
    }

    if (c.triangles.numItems > 0) out.indices = UnpackInts(c.triangles);
}

namespace {

void ReadBlendShapes(const Value* shapes, MeshAsset& out) {
    if (!shapes) return;
    for (const ValuePtr& v : ArrayField(*shapes, "vertices")) {
        BlendShapeVertex bv;
        bv.position = ReadVector3(v->Get("vertex"));
        bv.normal = ReadVector3(v->Get("normal"));
        bv.tangent = ReadVector3(v->Get("tangent"));
        bv.index = static_cast<std::uint32_t>(IntField(*v, "index"));
        out.blendShapeVertices.push_back(bv);
    }
    for (const ValuePtr& v : ArrayField(*shapes, "shapes")) {
        BlendShapeFrame f;
        f.firstVertex = static_cast<std::uint32_t>(IntField(*v, "firstVertex"));
        f.vertexCount = static_cast<std::uint32_t>(IntField(*v, "vertexCount"));
        f.hasNormals = IntField(*v, "hasNormals") != 0;
        f.hasTangents = IntField(*v, "hasTangents") != 0;
        out.blendShapeFrames.push_back(f);
    }
    for (const ValuePtr& v : ArrayField(*shapes, "channels")) {
        BlendShapeChannel ch;
        if (const Value* name = v->Get("name"); name && name->AsString()) ch.name = *name->AsString();
        ch.nameHash = static_cast<std::uint32_t>(IntField(*v, "nameHash"));
        ch.frameIndex = static_cast<std::int32_t>(IntField(*v, "frameIndex"));
        ch.frameCount = static_cast<std::int32_t>(IntField(*v, "frameCount"));
        out.blendShapeChannels.push_back(ch);
    }
    for (const ValuePtr& v : ArrayField(*shapes, "fullWeights")) {
        out.blendShapeFullWeights.push_back(static_cast<float>(v->AsDouble()));
    }
}

} // namespace

MeshAsset ReadMesh(const Value& mesh, const ResourceFetcher& fetch) {
    MeshAsset out;
    if (const Value* name = mesh.Get("m_Name"); name && name->AsString()) out.name = *name->AsString();
    out.wideIndices = IntField(mesh, "m_IndexFormat") == 1;
    out.localBounds = ReadBounds(mesh.Get("m_LocalAABB"));
    out.rootBoneNameHash = static_cast<std::uint32_t>(IntField(mesh, "m_RootBoneNameHash"));

    for (const ValuePtr& m : ArrayField(mesh, "m_BindPose")) {
        std::array<float, 16> matrix{};
        for (int r = 0; r < 4; ++r) {
            for (int col = 0; col < 4; ++col) {
                const std::string field = "e" + std::to_string(r) + std::to_string(col);
                matrix[static_cast<std::size_t>(r * 4 + col)] = FloatField(*m, field.c_str());
            }
        }
        out.bindPoses.push_back(matrix);
    }
    for (const ValuePtr& h : ArrayField(mesh, "m_BoneNameHashes")) {
        out.boneNameHashes.push_back(static_cast<std::uint32_t>(h->AsInt()));
    }
    ReadBlendShapes(mesh.Get("m_Shapes"), out);

    // Vertex streams.
    if (const Value* vd = mesh.Get("m_VertexData")) {
        const auto vertexCount = static_cast<std::uint32_t>(IntField(*vd, "m_VertexCount"));
        if (vertexCount > 0) {
            std::vector<ChannelInfo> channels;
            for (const ValuePtr& c : ArrayField(*vd, "m_Channels")) {
                channels.push_back({static_cast<std::uint8_t>(IntField(*c, "stream")),
                                    static_cast<std::uint8_t>(IntField(*c, "offset")),
                                    static_cast<std::uint8_t>(IntField(*c, "format")),
                                    static_cast<std::uint8_t>(IntField(*c, "dimension"))});
            }
            std::vector<std::uint8_t> bytes = ValueBytes(vd->Get("m_DataSize"));
            if (bytes.empty()) {
                const auto stream = ReadStreamingInfo(mesh.Get("m_StreamData"));
                if (stream && !stream->Empty()) {
                    if (!fetch) throw ReadError("mesh " + out.name + ": streamed data needs a fetcher");
                    bytes = fetch(*stream);
                }
            }
            DecodeVertexData(channels, bytes, vertexCount, out);
        }
    }

    // Index buffer.
    const std::vector<std::uint8_t> indexBytes = ValueBytes(mesh.Get("m_IndexBuffer"));
    if (!indexBytes.empty()) {
        const std::size_t size = out.wideIndices ? 4 : 2;
        out.indices.resize(indexBytes.size() / size);
        for (std::size_t i = 0; i < out.indices.size(); ++i) {
            out.indices[i] = out.wideIndices ? Load<std::uint32_t>(&indexBytes[i * 4])
                                             : Load<std::uint16_t>(&indexBytes[i * 2]);
        }
    }

    // Compressed form (replaces the vertex streams and index buffer when present).
    if (const Value* cm = mesh.Get("m_CompressedMesh"); cm && out.vertexCount == 0) {
        CompressedMesh c;
        c.vertices = ReadPackedBitVector(cm->Get("m_Vertices"));
        if (c.vertices.numItems > 0) {
            c.uv = ReadPackedBitVector(cm->Get("m_UV"));
            c.normals = ReadPackedBitVector(cm->Get("m_Normals"));
            c.tangents = ReadPackedBitVector(cm->Get("m_Tangents"));
            c.weights = ReadPackedBitVector(cm->Get("m_Weights"));
            c.normalSigns = ReadPackedBitVector(cm->Get("m_NormalSigns"));
            c.tangentSigns = ReadPackedBitVector(cm->Get("m_TangentSigns"));
            c.floatColors = ReadPackedBitVector(cm->Get("m_FloatColors"));
            c.boneIndices = ReadPackedBitVector(cm->Get("m_BoneIndices"));
            c.triangles = ReadPackedBitVector(cm->Get("m_Triangles"));
            c.uvInfo = static_cast<std::uint32_t>(IntField(*cm, "m_UVInfo"));
            DecodeCompressedMesh(c, out);
        }
    }

    // Submeshes: firstByte is a byte offset into the (16- or 32-bit) index buffer.
    const std::uint32_t indexSize = out.wideIndices ? 4 : 2;
    for (const ValuePtr& s : ArrayField(mesh, "m_SubMeshes")) {
        SubMesh sub;
        sub.firstIndex = static_cast<std::uint32_t>(IntField(*s, "firstByte")) / indexSize;
        sub.indexCount = static_cast<std::uint32_t>(IntField(*s, "indexCount"));
        sub.topology = static_cast<MeshTopology>(IntField(*s, "topology"));
        sub.baseVertex = static_cast<std::int32_t>(IntField(*s, "baseVertex"));
        sub.firstVertex = static_cast<std::uint32_t>(IntField(*s, "firstVertex"));
        sub.vertexCount = static_cast<std::uint32_t>(IntField(*s, "vertexCount"));
        sub.bounds = ReadBounds(s->Get("localAABB"));
        if (static_cast<std::uint64_t>(sub.firstIndex) + sub.indexCount > out.indices.size()) {
            throw ReadError("mesh " + out.name + ": submesh outside the index buffer");
        }
        out.subMeshes.push_back(sub);
    }
    return out;
}

} // namespace rr::assets
