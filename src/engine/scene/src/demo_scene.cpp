#include "rr/scene/demo_scene.h"

#include "rr/assets/texture_format.h"

#include <algorithm>
#include <cmath>
#include <functional>

namespace rr::scene {

namespace {

constexpr float kPi = 3.14159265358979f;

// Collects geometry in the renderer's convention and hands it out as a mesh in the
// game's convention (the inverse of ConvertMesh's mirroring, which is the same
// mirroring), so the demo goes through the real conversion.
struct MeshBuilder {
    assets::MeshAsset mesh;
    std::vector<Mat4> bindPoses; // renderer convention

    std::uint32_t Vertex(Vec3 p, Vec3 n, Vec3 t, float u, float v, std::array<float, 4> color = {1, 1, 1, 1}) {
        mesh.positions.push_back({p.x, p.y, p.z});
        mesh.normals.push_back({n.x, n.y, n.z});
        mesh.tangents.push_back({t.x, t.y, t.z, 1.0f});
        mesh.uvs[0].push_back(u);
        mesh.uvs[0].push_back(v);
        mesh.colors.push_back(color);
        return mesh.vertexCount++;
    }
    void Triangle(std::uint32_t a, std::uint32_t b, std::uint32_t c) { mesh.indices.insert(mesh.indices.end(), {a, b, c}); }
    void Quad(std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d) {
        Triangle(a, b, c);
        Triangle(a, c, d);
    }

    assets::MeshAsset Finish(const char* name) {
        assets::MeshAsset out = mesh;
        out.name = name;
        out.uvDimension[0] = 2;
        Aabb box;
        for (auto& p : out.positions) {
            box.Add(Vec3{p[0], p[1], p[2]});
            p[0] = -p[0];
        }
        for (auto& n : out.normals) n[0] = -n[0];
        for (auto& t : out.tangents) {
            t[0] = -t[0];
            t[3] = -t[3];
        }
        for (std::size_t i = 0; i + 2 < out.indices.size(); i += 3) std::swap(out.indices[i + 1], out.indices[i + 2]);
        assets::SubMesh sm;
        sm.indexCount = static_cast<std::uint32_t>(out.indices.size());
        sm.vertexCount = out.vertexCount;
        const Vec3 c = box.Center(), e = box.Extent();
        sm.bounds = {{-c.x, c.y, c.z}, {e.x, e.y, e.z}};
        out.subMeshes = {sm};
        out.localBounds = sm.bounds;
        for (const Mat4& pose : bindPoses) {
            const Mat4 game = MirrorMatrix(pose);
            std::array<float, 16> rowMajor{};
            for (int row = 0; row < 4; ++row)
                for (int col = 0; col < 4; ++col) rowMajor[static_cast<std::size_t>(row * 4 + col)] = game.At(row, col);
            out.bindPoses.push_back(rowMajor);
        }
        return out;
    }
};

assets::MeshAsset Plane(float size, float uvRepeat) {
    MeshBuilder b;
    const float h = size * 0.5f;
    const auto v0 = b.Vertex({-h, 0, h}, {0, 1, 0}, {1, 0, 0}, 0, 0);
    const auto v1 = b.Vertex({h, 0, h}, {0, 1, 0}, {1, 0, 0}, uvRepeat, 0);
    const auto v2 = b.Vertex({h, 0, -h}, {0, 1, 0}, {1, 0, 0}, uvRepeat, uvRepeat);
    const auto v3 = b.Vertex({-h, 0, -h}, {0, 1, 0}, {1, 0, 0}, 0, uvRepeat);
    b.Quad(v0, v1, v2, v3);
    return b.Finish("demo_plane");
}

assets::MeshAsset Quad(float w, float h) {
    MeshBuilder b;
    const auto v0 = b.Vertex({-w / 2, -h / 2, 0}, {0, 0, 1}, {1, 0, 0}, 0, 0);
    const auto v1 = b.Vertex({w / 2, -h / 2, 0}, {0, 0, 1}, {1, 0, 0}, 1, 0);
    const auto v2 = b.Vertex({w / 2, h / 2, 0}, {0, 0, 1}, {1, 0, 0}, 1, 1);
    const auto v3 = b.Vertex({-w / 2, h / 2, 0}, {0, 0, 1}, {1, 0, 0}, 0, 1);
    b.Quad(v0, v1, v2, v3);
    return b.Finish("demo_quad");
}

assets::MeshAsset Cube(float size) {
    MeshBuilder b;
    const float h = size * 0.5f;
    // Each face: normal, tangent (u direction) and bitangent (v direction).
    const Vec3 faces[6][3] = {{{1, 0, 0}, {0, 0, -1}, {0, 1, 0}},  {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
                              {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},  {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
                              {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},   {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}}};
    for (const auto& f : faces) {
        const Vec3 n = f[0], t = f[1], bt = f[2];
        const auto corner = [&](float u, float v) {
            return b.Vertex(n * h + t * ((u - 0.5f) * size) + bt * ((v - 0.5f) * size), n, t, u, v);
        };
        b.Quad(corner(0, 0), corner(1, 0), corner(1, 1), corner(0, 1));
    }
    return b.Finish("demo_cube");
}

assets::MeshAsset Sphere(float radius, int rings, int segments, bool rainbow) {
    MeshBuilder b;
    for (int r = 0; r <= rings; ++r) {
        const float v = static_cast<float>(r) / static_cast<float>(rings);
        const float phi = v * kPi;
        for (int s = 0; s <= segments; ++s) {
            const float u = static_cast<float>(s) / static_cast<float>(segments);
            const float theta = u * 2 * kPi;
            const Vec3 n{std::sin(phi) * std::cos(theta), std::cos(phi), -std::sin(phi) * std::sin(theta)};
            const Vec3 t{-std::sin(theta), 0, -std::cos(theta)};
            std::array<float, 4> color{1, 1, 1, 1};
            if (rainbow) color = {0.5f + 0.5f * n.x, 0.5f + 0.5f * n.y, 0.5f + 0.5f * n.z, 1};
            b.Vertex(n * radius, n, t, u, 1 - v, color);
        }
    }
    const auto at = [&](int r, int s) { return static_cast<std::uint32_t>(r * (segments + 1) + s); };
    for (int r = 0; r < rings; ++r)
        for (int s = 0; s < segments; ++s) b.Quad(at(r, s), at(r + 1, s), at(r + 1, s + 1), at(r, s + 1));
    return b.Finish("demo_sphere");
}

// A tube along +Y over three bones (at y = 0, 1, 2), weights blending linearly
// between neighbouring bones.
assets::MeshAsset SkinnedTube(float radius, float height, int rings, int segments) {
    MeshBuilder b;
    for (int r = 0; r <= rings; ++r) {
        const float v = static_cast<float>(r) / static_cast<float>(rings);
        const float y = v * height;
        const float boneSpace = std::clamp(y, 0.0f, 2.0f);
        const int lower = std::min(static_cast<int>(boneSpace), 1);
        const float blend = boneSpace - static_cast<float>(lower);
        for (int s = 0; s <= segments; ++s) {
            const float u = static_cast<float>(s) / static_cast<float>(segments);
            const float theta = u * 2 * kPi;
            const Vec3 n{std::cos(theta), 0, -std::sin(theta)};
            b.Vertex(Vec3{n.x * radius, y, n.z * radius}, n, {0, 1, 0}, u, v, {1, 1, 1, 1});
            b.mesh.boneIndices.push_back({static_cast<std::uint32_t>(lower), static_cast<std::uint32_t>(lower + 1), 0, 0});
            b.mesh.boneWeights.push_back({1 - blend, blend, 0, 0});
        }
    }
    const auto at = [&](int r, int s) { return static_cast<std::uint32_t>(r * (segments + 1) + s); };
    for (int r = 0; r < rings; ++r)
        for (int s = 0; s < segments; ++s) b.Quad(at(r, s), at(r, s + 1), at(r + 1, s + 1), at(r + 1, s));
    // Bind poses: inverse of each bone's bind transform relative to the renderer.
    for (int bone = 0; bone < 3; ++bone) b.bindPoses.push_back(Translation({0, -static_cast<float>(bone), 0}));
    return b.Finish("demo_tube");
}

assets::Texture2DAsset MakeTexture(const char* name, int size, bool srgb,
                                   const std::function<std::array<std::uint8_t, 4>(int, int, int)>& texel) {
    assets::Texture2DAsset t;
    t.name = name;
    t.width = t.height = size;
    t.format = static_cast<std::int32_t>(assets::TextureFormat::RGBA32);
    t.srgb = srgb;
    t.filterMode = 2;
    t.mipCount = 0;
    for (int level = 0; (size >> level) > 0; ++level) {
        const int s = size >> level;
        for (int y = 0; y < s; ++y)
            for (int x = 0; x < s; ++x) {
                const auto c = texel(x << level, y << level, level);
                t.data.insert(t.data.end(), c.begin(), c.end());
            }
        ++t.mipCount;
    }
    return t;
}

int AddNode(Scene& scene, const char* name, int parent, Vec3 position, Quat rotation = {}, Vec3 scale = {1, 1, 1}) {
    Node n;
    n.name = name;
    n.parent = parent;
    n.localPosition = position;
    n.localRotation = rotation;
    n.localScale = scale;
    return scene.AddNode(std::move(n));
}

int AddMeshInstance(Scene& scene, int node, MeshId mesh, MaterialId material) {
    RenderInstance inst;
    inst.node = node;
    inst.mesh = mesh;
    inst.materials = {material};
    return scene.AddInstance(std::move(inst));
}

} // namespace

void DemoScene::Animate(float t) {
    for (std::size_t i = 0; i < bendBones.size(); ++i) {
        const float angle = 0.55f * std::sin(t * 1.3f + static_cast<float>(i) * 0.7f);
        scene.Nodes()[static_cast<std::size_t>(bendBones[i])].localRotation = AxisAngle({0, 0, 1}, angle);
    }
    scene.UpdateTransforms();
    scene.UpdateBounds(resources);
}

std::unique_ptr<DemoScene> BuildDemoScene() {
    auto demo = std::make_unique<DemoScene>();
    ResourceLibrary& res = demo->resources;
    Scene& scene = demo->scene;

    // Textures.
    const TextureId checker = res.AddTexture(MakeTexture("demo_checker", 64, true, [](int x, int y, int) {
        const bool dark = ((x / 8) + (y / 8)) % 2 == 1;
        return dark ? std::array<std::uint8_t, 4>{70, 80, 95, 255} : std::array<std::uint8_t, 4>{200, 205, 210, 255};
    }));
    const TextureId bumps = res.AddTexture(MakeTexture("demo_bumps", 64, false, [](int x, int y, int) {
        // Normals of a grid of domes, RGB-encoded.
        const float fx = static_cast<float>(x % 16) / 8.0f - 1.0f, fy = static_cast<float>(y % 16) / 8.0f - 1.0f;
        const float d2 = fx * fx + fy * fy;
        Vec3 n{0, 0, 1};
        if (d2 < 0.8f) n = Normalize(Vec3{fx, fy, 0.7f});
        const auto enc = [](float c) { return static_cast<std::uint8_t>(std::lround((c * 0.5f + 0.5f) * 255.0f)); };
        return std::array<std::uint8_t, 4>{enc(n.x), enc(n.y), enc(n.z), 255};
    }));
    const TextureId holes = res.AddTexture(MakeTexture("demo_holes", 64, true, [](int x, int y, int) {
        const float fx = static_cast<float>(x % 32) - 15.5f, fy = static_cast<float>(y % 32) - 15.5f;
        const bool hole = fx * fx + fy * fy < 90.0f;
        return std::array<std::uint8_t, 4>{240, 200, 90, static_cast<std::uint8_t>(hole ? 0 : 255)};
    }));

    // Materials.
    const auto material = [&](const char* name, ShadingModel shading, SurfaceType surface, std::array<float, 4> color,
                              TextureId albedo = kNoResource, TextureId normal = kNoResource) {
        MaterialDesc m;
        m.name = name;
        m.match = ShaderMatch::Known;
        m.shading = shading;
        m.surface = surface;
        m.color = color;
        m.albedo = albedo;
        m.normal = normal;
        if (surface == SurfaceType::AlphaTested) {
            m.alphaCutoff = 0.5f;
            m.renderQueue = 2450;
            m.cull = render::CullMode::None;
        }
        if (m.Transparent()) {
            m.renderQueue = 3000;
            m.depthWrite = false;
            m.cull = render::CullMode::None;
        }
        return res.AddMaterial(std::move(m));
    };
    const MaterialId ground = material("ground", ShadingModel::Lit, SurfaceType::Opaque, {1, 1, 1, 1}, checker);
    const MaterialId bumpy = material("bumpy", ShadingModel::LitNormalMap, SurfaceType::Opaque, {0.45f, 0.6f, 0.9f, 1}, kNoResource, bumps);
    const MaterialId orange = material("tube", ShadingModel::Lit, SurfaceType::Opaque, {0.95f, 0.45f, 0.12f, 1});
    const MaterialId cutout = material("cutout", ShadingModel::Lit, SurfaceType::AlphaTested, {1, 1, 1, 1}, holes);
    const MaterialId red = material("glass_red", ShadingModel::Unlit, SurfaceType::Transparent, {0.9f, 0.1f, 0.1f, 0.5f});
    const MaterialId blue = material("glass_blue", ShadingModel::Unlit, SurfaceType::Transparent, {0.1f, 0.2f, 0.9f, 0.5f});
    const MaterialId glow = material("glow", ShadingModel::Unlit, SurfaceType::Additive, {0.05f, 0.3f, 0.1f, 1});
    MaterialDesc rainbowDesc;
    rainbowDesc.name = "rainbow";
    rainbowDesc.shading = ShadingModel::Unlit;
    rainbowDesc.vertexColor = true;
    const MaterialId rainbow = res.AddMaterial(rainbowDesc);
    const MaterialId grey = material("grey", ShadingModel::Lit, SurfaceType::Opaque, {0.55f, 0.55f, 0.55f, 1});
    const MaterialId lodHigh = material("lod0", ShadingModel::Lit, SurfaceType::Opaque, {0.3f, 0.85f, 0.4f, 1});
    const MaterialId lodLow = material("lod1", ShadingModel::Lit, SurfaceType::Opaque, {0.85f, 0.3f, 0.8f, 1});

    // Meshes.
    const MeshId plane = res.AddMesh(ConvertMesh(Plane(20.0f, 5.0f)));
    const MeshId cube = res.AddMesh(ConvertMesh(Cube(1.0f)));
    const MeshId quad = res.AddMesh(ConvertMesh(Quad(2.0f, 2.0f)));
    const MeshId sphere = res.AddMesh(ConvertMesh(Sphere(0.6f, 16, 24, true)));
    const MeshId sphereHigh = res.AddMesh(ConvertMesh(Sphere(0.5f, 20, 32, false)));
    const MeshId tube = res.AddMesh(ConvertMesh(SkinnedTube(0.25f, 3.0f, 24, 16)));

    const int root = AddNode(scene, "demo", -1, {});
    AddMeshInstance(scene, AddNode(scene, "ground", root, {0, 0, 0}), plane, ground);
    AddMeshInstance(scene, AddNode(scene, "bumpy_cube", root, {-2.6f, 0.8f, 0.5f}, AxisAngle({0, 1, 0}, 0.6f), {1.5f, 1.5f, 1.5f}),
                    cube, bumpy);
    AddMeshInstance(scene, AddNode(scene, "rainbow_sphere", root, {2.8f, 0.8f, 0.8f}), sphere, rainbow);
    AddMeshInstance(scene, AddNode(scene, "cutout", root, {0.4f, 1.2f, -2.0f}, AxisAngle({0, 1, 0}, -0.3f)), quad, cutout);
    AddMeshInstance(scene, AddNode(scene, "glass_far", root, {1.3f, 1.3f, 1.2f}, AxisAngle({0, 1, 0}, 0.2f), {0.7f, 0.7f, 0.7f}),
                    quad, blue);
    AddMeshInstance(scene, AddNode(scene, "glass_near", root, {1.8f, 1.0f, 2.4f}, AxisAngle({0, 1, 0}, -0.2f), {0.7f, 0.7f, 0.7f}),
                    quad, red);
    AddMeshInstance(scene, AddNode(scene, "glow", root, {-1.2f, 1.6f, 2.6f}, {}, {0.6f, 0.6f, 0.6f}), quad, glow);

    // Two skinned tubes: renderer nodes with a three-bone chain below each, posed
    // independently (the second one also exercises several bone uploads a frame).
    const Vec3 tubePositions[] = {{-0.4f, 0.0f, 0.6f}, {-4.2f, 0.0f, -2.2f}};
    for (const Vec3& at : tubePositions) {
        const int tubeNode = AddNode(scene, "tube", root, at);
        const int bone0 = AddNode(scene, "bone0", tubeNode, {0, 0, 0});
        const int bone1 = AddNode(scene, "bone1", bone0, {0, 1, 0});
        const int bone2 = AddNode(scene, "bone2", bone1, {0, 1, 0});
        RenderInstance skinned;
        skinned.node = tubeNode;
        skinned.kind = RendererKind::Skinned;
        skinned.mesh = tube;
        skinned.materials = {orange};
        skinned.bones = {bone0, bone1, bone2};
        skinned.rootBone = bone0;
        skinned.skinBounds = Aabb::FromCenterExtent({0, 1.5f, 0}, {2.0f, 2.0f, 2.0f});
        scene.AddInstance(skinned);
        demo->bendBones.push_back(bone1);
        demo->bendBones.push_back(bone2);
    }

    // LOD group: a detailed sphere close up, a cube further away, nothing far off.
    const int lodNode = AddNode(scene, "lod", root, {4.2f, 0.6f, -1.5f});
    const int hi = AddMeshInstance(scene, AddNode(scene, "lod0", lodNode, {}), sphereHigh, lodHigh);
    const int lo = AddMeshInstance(scene, AddNode(scene, "lod1", lodNode, {}), cube, lodLow);
    LodGroup group;
    group.node = lodNode;
    group.size = 1.0f;
    group.screenRelativeHeights = {0.08f, 0.01f};
    group.levels = {{hi}, {lo}};
    demo->lodGroup = scene.AddLodGroup(group);
    scene.Instances()[static_cast<std::size_t>(hi)].lodGroup = demo->lodGroup;
    scene.Instances()[static_cast<std::size_t>(hi)].lodLevel = 0;
    scene.Instances()[static_cast<std::size_t>(lo)].lodGroup = demo->lodGroup;
    scene.Instances()[static_cast<std::size_t>(lo)].lodLevel = 1;

    // A ring of pillars around the origin; those behind the camera get culled.
    for (int i = 0; i < 16; ++i) {
        const float a = static_cast<float>(i) / 16.0f * 2 * kPi;
        AddMeshInstance(scene, AddNode(scene, "pillar", root, {std::cos(a) * 8.5f, 0.75f, std::sin(a) * 8.5f}, {}, {0.5f, 1.5f, 0.5f}),
                        cube, grey);
    }

    demo->camera.view = LookAt({0.5f, 3.2f, 8.5f}, {0.3f, 1.0f, 0.0f}, {0, 1, 0});
    demo->camera.fovY = 0.9f;
    demo->camera.nearPlane = 0.1f;
    demo->camera.farPlane = 100.0f;
    demo->Animate(0.0f);
    return demo;
}

} // namespace rr::scene
