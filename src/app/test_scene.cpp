#include "test_scene.h"

#include "rr/assets/astc_decoder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

namespace rr::app {
namespace {

struct Vertex {
    float position[3];
    std::uint8_t color[4];
};

// Column-major 4x4 matrix, matching GLSL's mat4 memory layout.
struct Mat4 {
    std::array<float, 16> m{};

    static Mat4 Identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }
    float& At(int row, int col) { return m[static_cast<std::size_t>(col * 4 + row)]; }
    float At(int row, int col) const { return m[static_cast<std::size_t>(col * 4 + row)]; }

    friend Mat4 operator*(const Mat4& a, const Mat4& b) {
        Mat4 r;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) sum += a.At(row, k) * b.At(k, col);
                r.At(row, col) = sum;
            }
        return r;
    }
};

// Right-handed perspective projection producing depth in [0, 1], Y up.
Mat4 Perspective(float fovY, float aspect, float zNear, float zFar) {
    const float f = 1.0f / std::tan(fovY * 0.5f);
    Mat4 r;
    r.At(0, 0) = f / aspect;
    r.At(1, 1) = f;
    r.At(2, 2) = zFar / (zNear - zFar);
    r.At(2, 3) = zNear * zFar / (zNear - zFar);
    r.At(3, 2) = -1.0f;
    return r;
}

Mat4 Translation(float x, float y, float z) {
    Mat4 r = Mat4::Identity();
    r.At(0, 3) = x;
    r.At(1, 3) = y;
    r.At(2, 3) = z;
    return r;
}

Mat4 RotationY(float a) {
    Mat4 r = Mat4::Identity();
    r.At(0, 0) = std::cos(a);
    r.At(0, 2) = std::sin(a);
    r.At(2, 0) = -std::sin(a);
    r.At(2, 2) = std::cos(a);
    return r;
}

Mat4 RotationX(float a) {
    Mat4 r = Mat4::Identity();
    r.At(1, 1) = std::cos(a);
    r.At(1, 2) = -std::sin(a);
    r.At(2, 1) = std::sin(a);
    r.At(2, 2) = std::cos(a);
    return r;
}

struct DrawConstants {
    Mat4 transform;
    float tint[4];
};
static_assert(sizeof(DrawConstants) <= render::kMaxDrawConstantsSize);

struct LitVertex {
    float position[3];
    float normal[3];
    float uv[2];
};

struct LitConstants {
    Mat4 transform;
    float lightDir[4]; // towards the light, model space; w = ambient
    float tint[4];
    float params[4];   // x: encode sRGB, y: alpha cut-off
};
static_assert(sizeof(LitConstants) <= render::kMaxDrawConstantsSize);

render::VertexLayout LitLayout() {
    render::VertexLayout layout;
    layout.stride = sizeof(LitVertex);
    layout.attributes = {
        {0, render::VertexFormat::Float3, offsetof(LitVertex, position)},
        {1, render::VertexFormat::Float3, offsetof(LitVertex, normal)},
        {2, render::VertexFormat::Float2, offsetof(LitVertex, uv)},
    };
    return layout;
}

// Checkerboard with one tint per mip level, so mip selection shows up in the image.
std::vector<std::vector<std::uint8_t>> CheckerMips(int size) {
    const std::uint8_t tints[][3] = {{255, 255, 255}, {255, 190, 120}, {120, 220, 255}, {200, 255, 140},
                                     {255, 140, 200}, {180, 160, 255}, {255, 255, 120}, {140, 255, 220}};
    std::vector<std::vector<std::uint8_t>> levels;
    for (int level = 0; size >> level > 0; ++level) {
        const int s = size >> level;
        const int cell = std::max(1, 8 >> level);
        std::vector<std::uint8_t> texels(static_cast<std::size_t>(s * s) * 4);
        for (int y = 0; y < s; ++y) {
            for (int x = 0; x < s; ++x) {
                const bool dark = ((x / cell) + (y / cell)) % 2 == 1;
                const auto* t = tints[level % 8];
                std::uint8_t* p = &texels[static_cast<std::size_t>(y * s + x) * 4];
                for (int c = 0; c < 3; ++c) p[c] = static_cast<std::uint8_t>(dark ? t[c] / 4 : t[c]);
                p[3] = 255;
            }
        }
        levels.push_back(std::move(texels));
    }
    return levels;
}

// Builds ASTC 8x8 blocks: one partition, RGB endpoints (CEM 8, 8-bit values) and a
// 4x4 grid of 2-bit weights stretched over the block, so decoding exercises endpoint
// selection, weight unquantisation and infill. Contents follow a fixed sequence.
std::vector<std::uint8_t> AstcTestBlocks(int blocksX, int blocksY) {
    std::vector<std::uint8_t> data(static_cast<std::size_t>(blocksX * blocksY) * 16, 0);
    std::uint32_t seed = 12345;
    auto next = [&seed]() {
        seed = seed * 1664525u + 1013904223u;
        return seed >> 24;
    };
    for (int b = 0; b < blocksX * blocksY; ++b) {
        std::uint8_t* block = &data[static_cast<std::size_t>(b) * 16];
        auto put = [block](int start, int count, std::uint32_t value) {
            for (int i = 0; i < count; ++i) {
                if ((value >> i) & 1) block[(start + i) / 8] |= static_cast<std::uint8_t>(1 << ((start + i) % 8));
            }
        };
        put(0, 11, 0x42); // 4x4 weights, 4 levels
        put(13, 4, 8);    // CEM 8
        for (int i = 0; i < 6; ++i) put(17 + 8 * i, 8, next());
        for (int i = 0; i < 16; ++i) {
            const std::uint32_t w = ((i % 4) + (i / 4) + static_cast<std::uint32_t>(b)) % 4;
            put(127 - 2 * i, 1, w & 1); // weights are stored bit-reversed from the top
            put(126 - 2 * i, 1, (w >> 1) & 1);
        }
    }
    return data;
}

// Unit cube with per-face normals and UVs; faces counter-clockwise from outside.
void BuildLitCube(std::vector<LitVertex>& vertices, std::vector<std::uint16_t>& indices) {
    const float faces[6][3][3] = {
        {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},  {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}},
        {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}}, {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
        {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}}, {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
    };
    const float corners[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    for (const auto& f : faces) {
        const auto base = static_cast<std::uint16_t>(vertices.size());
        for (const auto& c : corners) {
            LitVertex v{};
            for (int i = 0; i < 3; ++i) {
                v.position[i] = 0.5f * (f[0][i] + c[0] * f[1][i] + c[1] * f[2][i]);
                v.normal[i] = f[0][i];
            }
            v.uv[0] = (c[0] + 1.0f) * 1.5f; // three checker repeats per face
            v.uv[1] = (c[1] + 1.0f) * 1.5f;
            vertices.push_back(v);
        }
        for (int i : {0, 1, 2, 0, 2, 3}) indices.push_back(static_cast<std::uint16_t>(base + i));
    }
}

// Rotation part of `m` applied transposed: world direction -> model direction for
// rigid transforms.
void ToModelSpace(const Mat4& m, const float world[3], float out[3]) {
    for (int i = 0; i < 3; ++i) out[i] = m.At(0, i) * world[0] + m.At(1, i) * world[1] + m.At(2, i) * world[2];
}

render::VertexLayout VertexFormatLayout() {
    render::VertexLayout layout;
    layout.stride = sizeof(Vertex);
    layout.attributes = {
        {0, render::VertexFormat::Float3, offsetof(Vertex, position)},
        {1, render::VertexFormat::UByte4Norm, offsetof(Vertex, color)},
    };
    return layout;
}

} // namespace

bool TestScene::Create(render::RenderDevice& device) {
    // One colour per face; four vertices per face so colours don't blend.
    struct Face {
        float normal[3];
        float u[3];
        float v[3];
        std::uint8_t color[4];
    };
    const Face faces[] = {
        {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}, {230, 80, 60, 255}},
        {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}, {60, 160, 230, 255}},
        {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}, {90, 200, 90, 255}},
        {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {240, 200, 60, 255}},
        {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}, {200, 100, 220, 255}},
        {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}, {120, 120, 130, 255}},
    };
    std::vector<Vertex> vertices;
    std::vector<std::uint16_t> indices;
    for (const Face& f : faces) {
        const auto base = static_cast<std::uint16_t>(vertices.size());
        const float corners[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}}; // counter-clockwise
        for (const auto& c : corners) {
            Vertex v{};
            for (int i = 0; i < 3; ++i) {
                v.position[i] = 0.5f * (f.normal[i] + c[0] * f.u[i] + c[1] * f.v[i]);
            }
            for (int i = 0; i < 4; ++i) v.color[i] = f.color[i];
            vertices.push_back(v);
        }
        for (int i : {0, 1, 2, 0, 2, 3}) indices.push_back(static_cast<std::uint16_t>(base + i));
    }
    cubeIndexCount_ = static_cast<std::uint32_t>(indices.size());
    cubeVertices_ = device.CreateBuffer(render::BufferUsage::Vertex, vertices.data(),
                                        vertices.size() * sizeof(Vertex));
    cubeIndices_ = device.CreateBuffer(render::BufferUsage::Index, indices.data(),
                                       indices.size() * sizeof(std::uint16_t));

    // Ground grid spanning x in [-5, 5] and z in [-5, 2]. It stops short of the
    // camera: lines crossing the near plane rendered incorrectly on the Mesa
    // software Vulkan driver used for automated checks (TODO: confirm on GPUs).
    std::vector<Vertex> grid;
    constexpr int kHalfWidth = 5;
    constexpr int kFar = -5;
    constexpr int kNear = 2;
    const auto shadeFor = [](int i) -> std::uint8_t { return i == 0 ? 200 : 90; };
    for (int i = -kHalfWidth; i <= kHalfWidth; ++i) {
        const auto x = static_cast<float>(i);
        const std::uint8_t s = shadeFor(i);
        grid.push_back({{x, 0, static_cast<float>(kFar)}, {s, s, s, 255}});
        grid.push_back({{x, 0, static_cast<float>(kNear)}, {s, s, s, 255}});
    }
    for (int i = kFar; i <= kNear; ++i) {
        const auto z = static_cast<float>(i);
        const std::uint8_t s = shadeFor(i);
        const auto w = static_cast<float>(kHalfWidth);
        grid.push_back({{-w, 0, z}, {s, s, s, 255}});
        grid.push_back({{w, 0, z}, {s, s, s, 255}});
    }
    gridVertexCount_ = static_cast<std::uint32_t>(grid.size());
    gridVertices_ = device.CreateBuffer(render::BufferUsage::Vertex, grid.data(), grid.size() * sizeof(Vertex));

    render::PipelineDesc solid;
    solid.shaderProgram = "unlit_color";
    solid.vertexLayout = VertexFormatLayout();
    solid_ = device.CreatePipeline(solid);

    render::PipelineDesc lines = solid;
    lines.topology = render::PrimitiveTopology::LineList;
    lines.cull = render::CullMode::None;
    lines_ = device.CreatePipeline(lines);

    // Lit, textured objects.
    std::vector<LitVertex> litVertices;
    std::vector<std::uint16_t> litIndices;
    BuildLitCube(litVertices, litIndices);
    litCubeVertices_ = device.CreateBuffer(render::BufferUsage::Vertex, litVertices.data(),
                                           litVertices.size() * sizeof(LitVertex));
    litCubeIndices_ = device.CreateBuffer(render::BufferUsage::Index, litIndices.data(),
                                          litIndices.size() * sizeof(std::uint16_t));
    const LitVertex quad[6] = {
        {{-0.6f, -0.6f, 0}, {0, 0, 1}, {0, 0}}, {{0.6f, -0.6f, 0}, {0, 0, 1}, {1, 0}},
        {{0.6f, 0.6f, 0}, {0, 0, 1}, {1, 1}},   {{-0.6f, -0.6f, 0}, {0, 0, 1}, {0, 0}},
        {{0.6f, 0.6f, 0}, {0, 0, 1}, {1, 1}},   {{-0.6f, 0.6f, 0}, {0, 0, 1}, {0, 1}},
    };
    quadVertices_ = device.CreateBuffer(render::BufferUsage::Vertex, quad, sizeof(quad));

    const auto mips = CheckerMips(64);
    std::vector<render::TextureLevelData> levels;
    for (const auto& m : mips) levels.push_back({m.data(), m.size()});
    render::TextureDesc checker;
    checker.width = checker.height = 64;
    checker.mipCount = static_cast<std::uint32_t>(levels.size());
    checker_ = device.CreateTexture(checker, levels);

    // 64x64 ASTC 8x8 texture: uploaded as is where the device supports ASTC,
    // otherwise decoded in software (the two paths must look the same).
    constexpr int kAstcSize = 64;
    const std::vector<std::uint8_t> blocks = AstcTestBlocks(kAstcSize / 8, kAstcSize / 8);
    render::TextureDesc astc;
    astc.width = astc.height = kAstcSize;
    astc.format = render::TextureFormat::ASTC_8x8;
    if (device.SupportsTextureFormat(astc.format)) {
        const render::TextureLevelData level{blocks.data(), blocks.size()};
        compressed_ = device.CreateTexture(astc, std::span(&level, 1));
    } else {
        const auto rgba = assets::astc::DecodeImage(blocks, kAstcSize, kAstcSize, 8, 8, assets::astc::Profile::Linear);
        astc.format = render::TextureFormat::RGBA8;
        const render::TextureLevelData level{rgba.data(), rgba.size()};
        compressed_ = device.CreateTexture(astc, std::span(&level, 1));
    }

    render::PipelineDesc lit;
    lit.shaderProgram = "lit_textured";
    lit.vertexLayout = LitLayout();
    lit_ = device.CreatePipeline(lit);

    return solid_ && lines_ && cubeVertices_ && cubeIndices_ && gridVertices_ && lit_ && litCubeVertices_ &&
           litCubeIndices_ && quadVertices_ && checker_ && compressed_;
}

void TestScene::Draw(render::RenderDevice& device, float time) {
    int width = 0, height = 0;
    device.GetRenderSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

    const Mat4 projection = Perspective(1.0f, aspect, 0.1f, 100.0f);
    const Mat4 view = Translation(0.0f, -0.6f, -4.0f) * RotationX(0.35f);
    const Mat4 viewProjection = projection * view;

    render::ClearValues clear;
    clear.color[0] = 0.08f;
    clear.color[1] = 0.10f;
    clear.color[2] = 0.16f;
    device.BeginRenderPass(clear);

    DrawConstants constants{};
    constants.tint[0] = constants.tint[1] = constants.tint[2] = constants.tint[3] = 1.0f;

    constants.transform = viewProjection;
    device.BindPipeline(lines_);
    device.BindVertexBuffer(gridVertices_);
    device.SetDrawConstants(&constants, sizeof(constants));
    device.Draw(gridVertexCount_);

    constants.transform = viewProjection * Translation(0.0f, 0.8f, 0.0f) * RotationY(time) *
                          RotationX(time * 0.6f);
    device.BindPipeline(solid_);
    device.BindVertexBuffer(cubeVertices_);
    device.BindIndexBuffer(cubeIndices_, render::IndexType::UInt16);
    device.SetDrawConstants(&constants, sizeof(constants));
    device.DrawIndexed(cubeIndexCount_);

    // Lit cube with the mipmapped checker texture (trilinear, repeating).
    const float sunWorld[3] = {0.4f, 0.8f, 0.45f};
    LitConstants lit{};
    lit.tint[0] = lit.tint[1] = lit.tint[2] = lit.tint[3] = 1.0f;
    const Mat4 cubeModel = Translation(-1.9f, 0.6f, -0.5f) * RotationY(0.7f + time * 0.5f) * RotationX(0.4f);
    lit.transform = viewProjection * cubeModel;
    ToModelSpace(cubeModel, sunWorld, lit.lightDir);
    lit.lightDir[3] = 0.25f;
    device.BindPipeline(lit_);
    device.BindTexture(0, checker_, render::SamplerDesc{});
    device.BindVertexBuffer(litCubeVertices_);
    device.BindIndexBuffer(litCubeIndices_, render::IndexType::UInt16);
    device.SetDrawConstants(&lit, sizeof(lit));
    device.DrawIndexed(static_cast<std::uint32_t>(36));

    // Quad with the block-compressed texture, sampled without filtering.
    render::SamplerDesc nearest;
    nearest.filter = render::FilterMode::Nearest;
    nearest.useMips = false;
    nearest.wrapU = nearest.wrapV = render::WrapMode::ClampToEdge;
    const Mat4 quadModel = Translation(1.9f, 0.8f, -0.3f) * RotationY(-0.5f);
    lit.transform = viewProjection * quadModel;
    ToModelSpace(quadModel, sunWorld, lit.lightDir);
    lit.lightDir[3] = 0.6f;
    device.BindTexture(0, compressed_, nearest);
    device.BindVertexBuffer(quadVertices_);
    device.SetDrawConstants(&lit, sizeof(lit));
    device.Draw(6);

    device.EndRenderPass();
}

} // namespace rr::app
