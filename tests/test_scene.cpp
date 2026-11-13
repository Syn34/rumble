// Engine scene layer: maths, handedness conversion, scene hierarchy, prefab
// instantiation and material mapping, all on data built here.
#include "test_framework.h"

#include "rr/assets/prefab.h"
#include "rr/scene/material_mapping.h"
#include "rr/scene/math.h"
#include "rr/scene/prefab_instancer.h"
#include "rr/scene/resources.h"
#include "rr/scene/scene.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>

using namespace rr;
using namespace rr::scene;

namespace {

bool Near(float a, float b, float tolerance = 1e-4f) { return std::fabs(a - b) <= tolerance; }
bool Near(Vec3 a, Vec3 b, float tolerance = 1e-4f) {
    return Near(a.x, b.x, tolerance) && Near(a.y, b.y, tolerance) && Near(a.z, b.z, tolerance);
}

// --- value builders (decoded component data) --------------------------------------

using assets::Value;
using assets::ValuePtr;

ValuePtr Int(std::int64_t v) {
    auto p = std::make_shared<Value>();
    p->type = "int";
    p->data = v;
    return p;
}
ValuePtr Real(double v) {
    auto p = std::make_shared<Value>();
    p->type = "float";
    p->data = v;
    return p;
}
ValuePtr Struct(std::vector<std::pair<std::string, ValuePtr>> fields) {
    auto p = std::make_shared<Value>();
    Value::Struct s;
    for (auto& [name, value] : fields) s.push_back({name, value});
    p->data = std::move(s);
    return p;
}
ValuePtr Array(std::vector<ValuePtr> items) {
    auto p = std::make_shared<Value>();
    p->data = Value::Array(std::move(items));
    return p;
}
ValuePtr Pointer(std::int64_t pathId, std::int64_t fileId = 0) {
    return Struct({{"m_FileID", Int(fileId)}, {"m_PathID", Int(pathId)}});
}
ValuePtr Vec(float x, float y, float z) { return Struct({{"x", Real(x)}, {"y", Real(y)}, {"z", Real(z)}}); }

// A Unity-convention transform step: p' = R(S p) + T, with the asset reader's maths.
assets::Vector3f UnityTransform(const assets::Vector3f& t, const assets::Quaternionf& r, const assets::Vector3f& s,
                                assets::Vector3f p) {
    return assets::Rotate(r, assets::Scale(s, p)) + t;
}

} // namespace

TEST_CASE("scene maths: TRS, inverse, look-at and box transforms") {
    const Quat r = AxisAngle({0.3f, 1.0f, -0.2f}, 0.9f);
    const Mat4 m = Trs({1, 2, 3}, r, {2, 0.5f, 1.5f});
    const Mat4 i = m * Inverse(m);
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col) CHECK(Near(i.At(row, col), row == col ? 1.0f : 0.0f));
    CHECK(Near(TransformPoint(m, {0, 0, 0}), {1, 2, 3}));
    CHECK(Near(TransformVector(RotationMatrix(r), {1, 0, 0}), Rotate(r, {1, 0, 0})));

    const Mat4 view = LookAt({0, 0, 5}, {0, 0, 0}, {0, 1, 0});
    CHECK(Near(TransformPoint(view, {0, 0, 0}), {0, 0, -5}));

    Aabb box = Aabb::FromCenterExtent({0, 0, 0}, {1, 2, 3});
    const Aabb moved = TransformAabb(Trs({10, 0, 0}, AxisAngle({0, 1, 0}, 1.5707963f), {1, 1, 1}), box);
    CHECK(Near(moved.Center(), {10, 0, 0}));
    CHECK(Near(moved.Extent(), {3, 2, 1}, 1e-3f));
}

TEST_CASE("scene maths: frustum culling") {
    const Mat4 vp = Perspective(1.0f, 1.0f, 0.1f, 100.0f) * LookAt({0, 0, 0}, {0, 0, -1}, {0, 1, 0});
    const Frustum f = Frustum::FromViewProjection(vp);
    CHECK(f.Intersects(Aabb::FromCenterExtent({0, 0, -10}, {1, 1, 1})));   // ahead
    CHECK(!f.Intersects(Aabb::FromCenterExtent({0, 0, 10}, {1, 1, 1})));   // behind
    CHECK(!f.Intersects(Aabb::FromCenterExtent({50, 0, -10}, {1, 1, 1}))); // far right
    CHECK(!f.Intersects(Aabb::FromCenterExtent({0, 0, -200}, {1, 1, 1}))); // past the far plane
    CHECK(f.Intersects(Aabb::FromCenterExtent({6, 0, -10}, {1.5f, 1, 1}))); // straddles the right plane
}

TEST_CASE("handedness: mirrored transforms map mirrored points") {
    // Composing two Unity transforms then mirroring equals composing the mirrored ones.
    const assets::Quaternionf ru1{0.1f, 0.7f, -0.2f, 0.68f}, ru2{-0.4f, 0.1f, 0.3f, 0.86f};
    const auto norm = [](assets::Quaternionf q) {
        const float l = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
        return assets::Quaternionf{q.x / l, q.y / l, q.z / l, q.w / l};
    };
    const assets::Quaternionf q1 = norm(ru1), q2 = norm(ru2);
    const assets::Vector3f t1{1, -2, 3}, t2{0.5f, 0.25f, -4}, s1{1, 2, 1}, s2{0.5f, 0.5f, 3};
    const assets::Vector3f p{0.3f, -0.7f, 1.1f};
    const assets::Vector3f unity = UnityTransform(t1, q1, s1, UnityTransform(t2, q2, s2, p));

    const auto mirrorT = [](assets::Vector3f v) { return MirrorPosition({v.x, v.y, v.z}); };
    const auto mirrorQ = [](assets::Quaternionf q) { return MirrorRotation({q.x, q.y, q.z, q.w}); };
    const Mat4 world = Trs(mirrorT(t1), mirrorQ(q1), {s1.x, s1.y, s1.z}) * Trs(mirrorT(t2), mirrorQ(q2), {s2.x, s2.y, s2.z});
    CHECK(Near(TransformPoint(world, mirrorT(p)), mirrorT(unity)));

    // MirrorMatrix agrees with mirroring the TRS parts.
    const Mat4 unityMatrix = Trs({t1.x, t1.y, t1.z}, {q1.x, q1.y, q1.z, q1.w}, {s1.x, s1.y, s1.z});
    const Mat4 mirrored = MirrorMatrix(unityMatrix);
    const Mat4 expected = Trs(mirrorT(t1), mirrorQ(q1), {s1.x, s1.y, s1.z});
    for (std::size_t k = 0; k < 16; ++k) CHECK(Near(mirrored.m[k], expected.m[k]));
}

TEST_CASE("mesh conversion: mirror, winding, base vertex, tangents, bind poses") {
    assets::MeshAsset mesh;
    mesh.name = "tri";
    mesh.vertexCount = 4;
    // Left-handed, clockwise front face seen from -Z (towards +Z): normal -Z.
    mesh.positions = {{9, 9, 9}, {0, 0, 0}, {0, 1, 0}, {1, 0, 0}};
    mesh.normals = {{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}};
    mesh.tangents = {{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}};
    mesh.indices = {0, 1, 2};
    assets::SubMesh sm;
    sm.firstIndex = 0;
    sm.indexCount = 3;
    sm.baseVertex = 1;
    mesh.subMeshes = {sm};
    mesh.boneWeights = {{1, 0, 0, 0}, {1, 0, 0, 0}, {0.5f, 0.5f, 0, 0}, {1, 0, 0, 0}};
    mesh.boneIndices = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}};
    std::array<float, 16> pose{};
    pose[0] = pose[5] = pose[10] = pose[15] = 1;
    pose[3] = 2; // row 0, column 3: translation x = 2
    mesh.bindPoses = {pose};
    mesh.localBounds = {{0.5f, 0.5f, 0}, {0.5f, 0.5f, 0}};

    const MeshData out = ConvertMesh(mesh);
    CHECK(out.vertices.size() == 4);
    CHECK(out.indices.size() == 3);
    CHECK(out.indices[0] == 1 && out.indices[1] == 3 && out.indices[2] == 2); // base vertex + reversed
    CHECK(Near(out.vertices[3].position[0], -1.0f));
    CHECK(Near(out.vertices[1].normal[2], -1.0f));
    CHECK(Near(out.vertices[1].tangent[0], -1.0f) && Near(out.vertices[1].tangent[3], -1.0f));

    // The counter-clockwise face normal of the converted triangle matches its vertex normal.
    const auto pos = [&](std::uint32_t i) {
        const auto& v = out.vertices[i].position;
        return Vec3{v[0], v[1], v[2]};
    };
    const Vec3 a = pos(out.indices[0]), b = pos(out.indices[1]), c = pos(out.indices[2]);
    const Vec3 faceNormal = Normalize(Cross(b - a, c - a));
    CHECK(Near(faceNormal, {0, 0, -1}));

    // Bitangent (cross(n, t) * w) is the mirror of the original one.
    const Vec3 n{out.vertices[1].normal[0], out.vertices[1].normal[1], out.vertices[1].normal[2]};
    const Vec3 t{out.vertices[1].tangent[0], out.vertices[1].tangent[1], out.vertices[1].tangent[2]};
    const Vec3 bitangent = Cross(n, t) * out.vertices[1].tangent[3];
    const Vec3 originalBitangent = Cross(Vec3{0, 0, -1}, Vec3{1, 0, 0}) * 1.0f; // in the original's algebra
    CHECK(Near(bitangent, MirrorPosition(originalBitangent)));

    CHECK(out.HasSkin());
    CHECK(Near(out.skin[2].boneIndices[1], 1.0f) && Near(out.skin[2].boneWeights[0], 0.5f));
    CHECK(out.bindPoses.size() == 1);
    CHECK(Near(out.bindPoses[0].Translation(), {-2, 0, 0}));
    CHECK(Near(out.bounds.Center(), {-0.5f, 0.5f, 0}));
}

TEST_CASE("scene: world transforms, active flags and bounds") {
    ResourceLibrary resources;
    MeshData cube;
    cube.bounds = Aabb::FromCenterExtent({0, 0, 0}, {1, 1, 1});
    cube.subMeshes.push_back({0, 0, cube.bounds});
    const MeshId mesh = resources.AddMesh(cube);

    Scene scene;
    Node root;
    root.name = "r";
    root.localPosition = {10, 0, 0};
    root.localScale = {2, 2, 2};
    const int r = scene.AddNode(root);
    Node child;
    child.parent = r;
    child.localPosition = {1, 0, 0};
    child.localRotation = AxisAngle({0, 1, 0}, 3.14159265f);
    const int c = scene.AddNode(child);
    Node hidden;
    hidden.name = "h";
    hidden.parent = r;
    hidden.active = false;
    const int h = scene.AddNode(hidden);
    Node below;
    below.name = "b";
    below.parent = h;
    const int b = scene.AddNode(below);

    RenderInstance inst;
    inst.node = c;
    inst.mesh = mesh;
    scene.AddInstance(inst);
    inst.node = b;
    scene.AddInstance(inst);

    scene.UpdateTransforms();
    scene.UpdateBounds(resources);
    CHECK(Near(scene.Nodes()[c].world.Translation(), {12, 0, 0}));
    CHECK(Near(TransformPoint(scene.Nodes()[c].world, {1, 0, 0}), {10, 0, 0}, 1e-3f));
    CHECK(!scene.Nodes()[b].activeInHierarchy);
    CHECK(scene.Drawable(scene.Instances()[0]));
    CHECK(!scene.Drawable(scene.Instances()[1]));
    CHECK(Near(scene.Instances()[0].worldBounds.Extent(), {2, 2, 2}, 1e-3f));
    CHECK(Near(scene.Bounds().Center(), {12, 0, 0}, 1e-3f));
    CHECK(scene.Path(b) == "r/h/b");

    bool threw = false;
    try {
        Node bad;
        bad.parent = 99;
        scene.AddNode(bad);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

namespace {

// Resolves pointers by path id to resources set up by the test.
class FakeResolver final : public AssetResolver {
public:
    std::map<std::int64_t, MeshId> meshes;
    std::map<std::int64_t, MaterialId> materials;
    MeshId Mesh(const std::string&, const assets::ObjectPointer& p) override {
        auto it = meshes.find(p.pathId);
        return it != meshes.end() ? it->second : kNoResource;
    }
    MaterialId Material(const std::string&, const assets::ObjectPointer& p) override {
        auto it = materials.find(p.pathId);
        return it != materials.end() ? it->second : kNoResource;
    }
};

assets::ComponentData Component(const assets::ObjectInfo& info, std::int32_t classId, ValuePtr value) {
    assets::ComponentData c;
    c.ref = assets::ObjectRef{nullptr, &info, "CAB-test"};
    c.classId = classId;
    c.value = std::move(value);
    return c;
}

} // namespace

TEST_CASE("prefab instancer: renderers, skinning bones, LOD groups, static batching") {
    // Object table entries the component refs point at (path ids only matter).
    static const assets::ObjectInfo rootTransform{100, 0, 0, 0, 4}, childTransform{101, 0, 0, 0, 4},
        skinTransform{102, 0, 0, 0, 4}, filter{200, 0, 0, 0, 33}, renderer{201, 0, 0, 0, 23}, skinned{202, 0, 0, 0, 137},
        lod{203, 0, 0, 0, 205}, particles{204, 0, 0, 0, 199}, batchedRenderer{205, 0, 0, 0, 23};

    ResourceLibrary resources;
    MeshData mesh;
    mesh.bounds = Aabb::FromCenterExtent({0, 0, 0}, {1, 1, 1});
    mesh.subMeshes = {{0, 3, mesh.bounds}, {3, 3, mesh.bounds}};
    FakeResolver resolver;
    resolver.meshes[500] = resources.AddMesh(mesh);
    resolver.materials[600] = resources.AddMaterial(MaterialDesc{});

    assets::Prefab prefab;
    assets::GameObjectData root;
    root.name = "root";
    root.transform = assets::TransformData{};
    root.transform->localPosition = {1, 2, 3};
    root.transform->localRotation = {0, 0.7071068f, 0, 0.7071068f};
    root.components.push_back(Component(rootTransform, 4, nullptr));
    root.components.push_back(Component(
        lod, 205,
        Struct({{"m_LocalReferencePoint", Vec(1, 0, 0)},
                {"m_Size", Real(2.0)},
                {"m_LODs", Array({Struct({{"screenRelativeHeight", Real(0.5)},
                                          {"renderers", Array({Struct({{"renderer", Pointer(201)}})})}}),
                                  Struct({{"screenRelativeHeight", Real(0.1)},
                                          {"renderers", Array({Struct({{"renderer", Pointer(999)}})})}})})},
                {"m_Enabled", Int(1)}})));
    root.children = {1, 2};
    prefab.objects.push_back(root);

    assets::GameObjectData child;
    child.name = "child";
    child.parent = 0;
    child.transform = assets::TransformData{};
    child.transform->localPosition = {0, 1, 0};
    child.components.push_back(Component(childTransform, 4, nullptr));
    child.components.push_back(Component(renderer, 23,
                                         Struct({{"m_Enabled", Int(1)},
                                                 {"m_Materials", Array({Pointer(600), Pointer(601)})},
                                                 {"m_StaticBatchInfo", Struct({{"firstSubMesh", Int(0)},
                                                                               {"subMeshCount", Int(0)}})},
                                                 {"m_StaticBatchRoot", Pointer(0)}})));
    child.components.push_back(Component(filter, 33, Struct({{"m_Mesh", Pointer(500)}})));
    child.components.push_back(Component(particles, 199, Struct({})));
    child.components.push_back(Component(batchedRenderer, 23,
                                         Struct({{"m_Materials", Array({Pointer(600)})},
                                                 {"m_StaticBatchInfo", Struct({{"firstSubMesh", Int(1)},
                                                                               {"subMeshCount", Int(1)}})},
                                                 {"m_StaticBatchRoot", Pointer(100)}})));
    prefab.objects.push_back(child);

    assets::GameObjectData skin;
    skin.name = "skin";
    skin.parent = 0;
    skin.active = false;
    skin.transform = assets::TransformData{};
    skin.components.push_back(Component(skinTransform, 4, nullptr));
    skin.components.push_back(Component(
        skinned, 137,
        Struct({{"m_Enabled", Int(1)},
                {"m_Materials", Array({Pointer(600)})},
                {"m_Mesh", Pointer(500)},
                {"m_Bones", Array({Pointer(100), Pointer(101), Pointer(777)})},
                {"m_RootBone", Pointer(101)},
                {"m_AABB", Struct({{"m_Center", Vec(1, 0, 0)}, {"m_Extent", Vec(1, 2, 3)}})}})));
    prefab.objects.push_back(skin);

    Scene scene;
    PrefabInstancer instancer(scene, resolver, resources);
    const int top = instancer.Instantiate(prefab);
    scene.UpdateTransforms();
    scene.UpdateBounds(resources);
    const InstantiateStats& s = instancer.Stats();

    CHECK(top == 0);
    CHECK(s.nodes == 3);
    CHECK(s.meshRenderers == 2);
    CHECK(s.skinnedRenderers == 1);
    CHECK(s.otherRenderers == 1);
    CHECK(s.staticBatched == 1);
    CHECK(s.missingMaterials == 1); // pointer 601
    CHECK(s.missingBones == 1);     // pointer 777
    CHECK(s.lodGroups == 1);
    CHECK(s.lodRenderersMissing == 1);

    // Transforms were mirrored into the right-handed convention.
    CHECK(Near(scene.Nodes()[0].localPosition, {-1, 2, 3}));
    CHECK(Near(scene.Nodes()[0].localRotation.y, -0.7071068f));
    // The child sits one unit above the root.
    CHECK(Near(scene.Nodes()[1].world.Translation(), {-1, 3, 3}));

    const auto& instances = scene.Instances();
    CHECK(instances.size() == 3);
    CHECK(instances[0].materials.size() == 2);
    CHECK(instances[0].materials[1] == resources.DefaultMaterial());
    CHECK(instances[0].lodGroup == 0 && instances[0].lodLevel == 0);
    CHECK(instances[1].StaticBatched() && instances[1].firstSubMesh == 1 && instances[1].staticBatchRoot == 0);
    CHECK(instances[2].kind == RendererKind::Skinned);
    CHECK(instances[2].bones.size() == 3 && instances[2].bones[0] == 0 && instances[2].bones[1] == 1 &&
          instances[2].bones[2] == -1);
    CHECK(instances[2].rootBone == 1);
    CHECK(Near(instances[2].skinBounds.Center(), {-1, 0, 0}));
    CHECK(!scene.Drawable(instances[2])); // its game object is inactive
    const LodGroup& group = scene.LodGroups()[0];
    CHECK(group.levels.size() == 2 && group.levels[0].size() == 1 && group.levels[1].empty());
    CHECK(Near(group.localReferencePoint, {-1, 0, 0}));
    CHECK(Near(group.size, 2.0f));
}

namespace {

assets::ShaderInfo MakeShader(const std::string& name, std::map<std::string, std::string> tags, float src, float dst,
                              std::string srcProperty = {}, std::string dstProperty = {}) {
    assets::ShaderInfo s;
    s.name = name;
    s.tags = std::move(tags);
    assets::ShaderPassState pass;
    pass.name = "Universal Forward";
    pass.tags["LightMode"] = "UniversalForward";
    pass.srcBlend = {src, std::move(srcProperty)};
    pass.destBlend = {dst, std::move(dstProperty)};
    s.passes.push_back(pass);
    assets::ShaderPassState shadow;
    shadow.name = "ShadowCaster";
    shadow.tags["LightMode"] = "ShadowCaster";
    s.passes.insert(s.passes.begin(), shadow);
    return s;
}

assets::MaterialTexture Tex(std::int64_t pathId) {
    assets::MaterialTexture t;
    t.texture.pathId = pathId;
    return t;
}

} // namespace

TEST_CASE("material mapping: shader tags and pass state") {
    // A transparent unlit shader graph (alpha blending from its forward pass).
    const assets::ShaderInfo graph =
        MakeShader("Shader Graphs/Custom_Glow", {{"Queue", "Transparent"}, {"UniversalMaterialType", "Unlit"}}, 5, 10);
    assets::MaterialAsset m;
    m.name = "glow";
    m.textures["_MainTex"] = Tex(7);
    m.textures["_MainTex"].scale = {2, 3};
    m.colors["_BaseColor"] = {1, 0.5f, 0, 0.5f};
    MaterialDesc d = MapMaterial(m, &graph);
    CHECK(d.match == ShaderMatch::Generic);
    CHECK(d.shading == ShadingModel::Unlit);
    CHECK(d.surface == SurfaceType::Transparent);
    CHECK(d.renderQueue == 3000);
    CHECK(d.albedoProperty == "_MainTex");
    CHECK(Near(d.uvTransform[0], 2) && Near(d.uvTransform[1], 3));
    CHECK(Near(d.color[1], SrgbToLinear(0.5f)) && Near(d.color[3], 0.5f));

    // A lit shader whose blend factors come from material properties.
    assets::ShaderInfo lit = MakeShader("Universal Render Pipeline/Lit", {{"RenderType", "Opaque"}}, 1, 0, "_SrcBlend", "_DstBlend");
    for (const char* p : {"_BaseMap", "_BumpMap", "_BaseColor", "_SrcBlend", "_DstBlend", "_BumpScale"}) {
        lit.properties.push_back({p, "", 0});
    }
    assets::MaterialAsset additive;
    additive.floats["_SrcBlend"] = 5;
    additive.floats["_DstBlend"] = 1;
    additive.floats["_BumpScale"] = 0.5f;
    additive.textures["_BaseMap"] = Tex(1);
    additive.textures["_BumpMap"] = Tex(2);
    additive.textures["_EmissionMap"] = Tex(3); // not declared by the shader
    d = MapMaterial(additive, &lit);
    CHECK(d.match == ShaderMatch::Known);
    CHECK(d.surface == SurfaceType::Additive);
    CHECK(d.shading == ShadingModel::LitNormalMap);
    CHECK(d.normalProperty == "_BumpMap" && Near(d.normalScale, 0.5f));
    FinishMaterial(d, 4, kNoResource);
    CHECK(d.shading == ShadingModel::Lit && d.albedo == 4);

    // Outline helpers aren't drawn.
    const assets::ShaderInfo outline = MakeShader("Custom/Outline Fill", {}, 1, 0);
    CHECK(!MapMaterial(assets::MaterialAsset{}, &outline).visible);
}

TEST_CASE("material mapping: materials without their shader") {
    assets::MaterialAsset cut;
    cut.keywords = {"_ALPHATEST_ON"};
    cut.floats["_Cutoff"] = 0.3f;
    cut.floats["_Cull"] = 0;
    cut.textures["_Masque"] = Tex(5); // a mask, not albedo
    cut.textures["_Diffuse"] = Tex(6);
    MaterialDesc d = MapMaterial(cut, nullptr);
    CHECK(d.match == ShaderMatch::Unresolved);
    CHECK(d.surface == SurfaceType::AlphaTested && Near(d.alphaCutoff, 0.3f));
    CHECK(d.cull == render::CullMode::None);
    CHECK(d.albedoProperty == "_Diffuse");

    assets::MaterialAsset glass;
    glass.floats["_Surface"] = 1;
    glass.floats["_SrcBlend"] = 1;
    glass.floats["_DstBlend"] = 10;
    glass.floats["_ZWrite"] = 0;
    glass.textures["_Normal"] = Tex(9);
    d = MapMaterial(glass, nullptr);
    CHECK(d.surface == SurfaceType::Premultiplied);
    CHECK(!d.depthWrite);
    CHECK(d.renderQueue == 3000);
    CHECK(d.shading == ShadingModel::LitNormalMap);

    assets::MaterialAsset queued;
    queued.customRenderQueue = 2450;
    CHECK(MapMaterial(queued, nullptr).surface == SurfaceType::AlphaTested);

    CHECK(ParseRenderQueue("Geometry") == 2000);
    CHECK(ParseRenderQueue("AlphaTest+1") == 2451);
    CHECK(ParseRenderQueue("Transparent-10") == 2990);
    CHECK(ParseRenderQueue("2100") == 2100);
    CHECK(ParseRenderQueue("bogus", 7) == 7);
}

// --- draw lists, LOD selection, skinning ------------------------------------------

#include "rr/scene/demo_scene.h"
#include "rr/scene/scene_renderer.h"

TEST_CASE("draw list: culling, sorting and LOD selection on the demo scene") {
    auto demo = BuildDemoScene();
    const Mat4 projection = demo->camera.Projection(16.0f / 9.0f);
    DrawListStats stats;
    const auto items = BuildDrawList(demo->scene, demo->resources, demo->camera.view, projection, {}, &stats);
    CHECK(stats.instances == demo->scene.Instances().size());
    CHECK(stats.lodRejected == 1); // one of the two LOD levels
    CHECK(stats.culled > 0);       // pillars behind the camera
    CHECK(stats.visible + stats.culled + stats.lodRejected == stats.instances);
    CHECK(stats.transparentItems == 3);
    CHECK(items.size() == stats.opaqueItems + stats.transparentItems);

    // Queue order; front to back within opaque queues, back to front within transparent ones.
    for (std::size_t i = 1; i < items.size(); ++i) {
        const DrawItem& a = items[i - 1];
        const DrawItem& b = items[i];
        CHECK(a.renderQueue <= b.renderQueue);
        if (a.renderQueue == b.renderQueue) {
            if (a.renderQueue > 2500) {
                CHECK(a.depth >= b.depth);
            } else {
                CHECK(a.depth <= b.depth);
            }
        }
    }
    CHECK(items.back().transparent);

    // Close up the detailed level is used, from far away the coarse one, and from
    // very far away neither.
    const LodGroup& group = demo->scene.LodGroups()[static_cast<std::size_t>(demo->lodGroup)];
    const Vec3 center = demo->scene.Nodes()[static_cast<std::size_t>(group.node)].world.Translation();
    const auto lodAt = [&](float distance) {
        const Mat4 view = LookAt(center + Vec3{0, 0, distance}, center, {0, 1, 0});
        return SelectLods(demo->scene, view, projection, {})[static_cast<std::size_t>(demo->lodGroup)];
    };
    CHECK(lodAt(3.0f) == 0);
    CHECK(lodAt(30.0f) == 1);
    CHECK(lodAt(300.0f) == -1);
    DrawListOptions forced;
    forced.forceLod = 0;
    CHECK(SelectLods(demo->scene, demo->camera.view, projection, forced)[0] == 0);

    // Disabling culling keeps every instance of the selected LODs.
    DrawListOptions noCull;
    noCull.frustumCull = false;
    BuildDrawList(demo->scene, demo->resources, demo->camera.view, projection, noCull, &stats);
    CHECK(stats.culled == 0);
}

TEST_CASE("skinning: bind pose reproduces the mesh, bent bones move vertices") {
    auto demo = BuildDemoScene();
    const auto& instances = demo->scene.Instances();
    const auto it = std::find_if(instances.begin(), instances.end(),
                                 [](const RenderInstance& i) { return i.kind == RendererKind::Skinned; });
    CHECK(it != instances.end());
    if (it == instances.end()) return;
    const MeshData& mesh = demo->resources.Mesh(it->mesh);
    CHECK(mesh.HasSkin() && mesh.bindPoses.size() == 3);

    const auto skinnedPosition = [&](const std::vector<Mat4>& bones, std::size_t v) {
        const MeshVertex& mv = mesh.vertices[v];
        const SkinVertex& sv = mesh.skin[v];
        Vec3 sum;
        for (int k = 0; k < 4; ++k) {
            const auto bone = static_cast<std::size_t>(sv.boneIndices[k]);
            sum = sum + TransformPoint(bones[bone], {mv.position[0], mv.position[1], mv.position[2]}) * sv.boneWeights[k];
        }
        return sum;
    };

    demo->Animate(0.0f); // sin(0) = 0 for the first bone; force an exact bind pose
    for (int b : demo->bendBones) demo->scene.Nodes()[static_cast<std::size_t>(b)].localRotation = {};
    demo->scene.UpdateTransforms();
    const Mat4& rendererWorld = demo->scene.Nodes()[static_cast<std::size_t>(it->node)].world;
    auto bones = SceneRenderer::BoneMatrices(demo->scene, *it, mesh);
    for (std::size_t v = 0; v < mesh.vertices.size(); v += 37) {
        const MeshVertex& mv = mesh.vertices[v];
        CHECK(Near(skinnedPosition(bones, v), TransformPoint(rendererWorld, {mv.position[0], mv.position[1], mv.position[2]}), 1e-3f));
    }

    // Bend: the top vertex moves sideways, the bottom one stays.
    demo->Animate(1.2f);
    bones = SceneRenderer::BoneMatrices(demo->scene, *it, mesh);
    const std::size_t top = mesh.vertices.size() - 1;
    const MeshVertex& tv = mesh.vertices[top];
    CHECK(!Near(skinnedPosition(bones, top), TransformPoint(rendererWorld, {tv.position[0], tv.position[1], tv.position[2]}), 1e-2f));
    CHECK(Near(skinnedPosition(bones, 0), TransformPoint(rendererWorld, {mesh.vertices[0].position[0], mesh.vertices[0].position[1],
                                                                         mesh.vertices[0].position[2]}), 1e-3f));
}
