#include "test_scene.h"

#include <array>
#include <cmath>
#include <cstdint>
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

    return solid_ && lines_ && cubeVertices_ && cubeIndices_ && gridVertices_;
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

    device.EndRenderPass();
}

} // namespace rr::app
