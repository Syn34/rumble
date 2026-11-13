// meshview - development viewer and checker for the game's meshes and textures.
//
//   meshview <bundle|bundle-dir> --engine-lib <libunity.so> [mode] [options]
//
// Modes:
//   (default)            view one mesh: --mesh <name> or --path-id <id>
//   --list               list the meshes and textures in the bundle
//   --check-meshes       decode every mesh and run numeric consistency checks
//   --check-textures     decode every texture mip; where the renderer can sample a
//                        format natively, compare the GPU's decode with ours
//   --demo               view a mesh and texture generated in code (no game files)
//   --astc-selftest <n>  decode n random ASTC blocks per block size with the GPU
//                        and with our decoder and compare (no game files)
// Options:
//   --renderer <vulkan|opengl>, --frames <n>, --screenshot <file.ppm>,
//   --software-decode    always decode compressed textures on the CPU
//   --verbose            with --astc-selftest: print the first differing blocks
//
// Viewer controls: drag with the left mouse button to orbit, wheel to zoom,
// Escape to quit. Output derived from your game files stays on your machine:
// don't commit or share it.
#include "rr/assets/astc_decoder.h"
#include "rr/assets/content_library.h"
#include "rr/assets/material.h"
#include "rr/assets/mesh.h"
#include "rr/assets/streamed_data.h"
#include "rr/assets/texture2d.h"
#include "rr/platform/platform.h"
#include "rr/render/render_device.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace rr;
using assets::ContentLibrary;
using assets::ObjectRef;

// --- small maths ------------------------------------------------------------------

struct Mat4 { // column-major, like GLSL
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

Mat4 RotationX(float a) {
    Mat4 r = Mat4::Identity();
    r.At(1, 1) = std::cos(a);
    r.At(1, 2) = -std::sin(a);
    r.At(2, 1) = std::sin(a);
    r.At(2, 2) = std::cos(a);
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

// --- options ----------------------------------------------------------------------

struct Options {
    std::string input;
    std::string engineLib;
    std::string meshName;
    std::optional<std::int64_t> pathId;
    std::string mode = "view";
    int selftestBlocks = 2000;
    std::optional<platform::GraphicsApi> renderer;
    std::uint64_t frames = 0;
    std::string screenshot;
    bool softwareDecode = false;
    bool verbose = false;
};

void PrintUsage() {
    std::fprintf(stderr,
                 "usage: meshview <bundle|dir> --engine-lib <libunity.so> [--mesh <name>|--path-id <id>]\n"
                 "                [--list|--check-meshes|--check-textures] [--renderer vulkan|opengl]\n"
                 "                [--frames n] [--screenshot out.ppm] [--software-decode]\n"
                 "       meshview --demo [options]\n"
                 "       meshview --astc-selftest <blocks> [--renderer ...]\n");
}

bool ParseOptions(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto next = [&]() -> const char* { return i + 1 < argc ? argv[++i] : nullptr; };
        if (arg == "--engine-lib") {
            const char* v = next();
            if (!v) return false;
            o.engineLib = v;
        } else if (arg == "--mesh") {
            const char* v = next();
            if (!v) return false;
            o.meshName = v;
        } else if (arg == "--path-id") {
            const char* v = next();
            if (!v) return false;
            o.pathId = std::stoll(v);
        } else if (arg == "--list" || arg == "--check-meshes" || arg == "--check-textures" || arg == "--demo") {
            o.mode = arg.substr(2);
        } else if (arg == "--astc-selftest") {
            const char* v = next();
            if (!v) return false;
            o.mode = "astc-selftest";
            o.selftestBlocks = std::stoi(v);
        } else if (arg == "--renderer") {
            const char* v = next();
            o.renderer = v ? platform::ParseGraphicsApi(v) : std::nullopt;
            if (!o.renderer || *o.renderer == platform::GraphicsApi::None) return false;
        } else if (arg == "--frames") {
            const char* v = next();
            if (!v) return false;
            o.frames = std::stoull(v);
        } else if (arg == "--screenshot") {
            const char* v = next();
            if (!v) return false;
            o.screenshot = v;
        } else if (arg == "--software-decode") {
            o.softwareDecode = true;
        } else if (arg == "--verbose") {
            o.verbose = true;
        } else if (!arg.empty() && arg[0] != '-' && o.input.empty()) {
            o.input = arg;
        } else {
            return false;
        }
    }
    const bool needsGameFiles = o.mode != "demo" && o.mode != "astc-selftest";
    return !needsGameFiles || (!o.input.empty() && !o.engineLib.empty());
}

// --- content access ------------------------------------------------------------------

std::vector<std::uint8_t> ReadWholeFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

struct Content {
    std::unique_ptr<ContentLibrary> library;
    std::unique_ptr<assets::ResourceStore> resources;
    std::vector<fs::path> bundles; // the bundles the user asked about
};

std::optional<Content> OpenContent(const Options& o) {
    auto strings = assets::CommonStrings::FromEngineLibrary(ReadWholeFile(o.engineLib));
    if (!strings) {
        std::fprintf(stderr, "no field-name table found in %s\n", o.engineLib.c_str());
        return std::nullopt;
    }
    const fs::path input(o.input);
    const fs::path dir = fs::is_directory(input) ? input : input.parent_path();
    Content c;
    c.library = std::make_unique<ContentLibrary>(dir, *strings);
    c.resources = std::make_unique<assets::ResourceStore>(*c.library);
    if (fs::is_directory(input)) {
        c.bundles = c.library->Bundles();
    } else {
        c.bundles.push_back(input);
    }
    return c;
}

// Calls fn(ref) for every object of `classId` in the selected bundles.
template <typename Fn> void ForEachObject(Content& c, std::int32_t classId, Fn&& fn) {
    for (const fs::path& bundle : c.bundles) {
        for (const std::string& cab : c.library->FilesInBundle(bundle)) {
            const assets::SerializedFile* file = c.library->FindFile(cab);
            if (!file) continue;
            for (const assets::ObjectInfo& object : file->Objects()) {
                if (object.classId == classId) fn(ObjectRef{file, &object, cab});
            }
        }
    }
}

// Materials of the renderer that draws `mesh` (one per submesh), found through a
// SkinnedMeshRenderer or a MeshFilter + MeshRenderer pair in the mesh's file.
std::vector<std::optional<ObjectRef>> FindMaterials(Content& c, const ObjectRef& mesh) {
    constexpr std::int32_t kMeshFilter = 33, kMeshRenderer = 23, kSkinnedMeshRenderer = 137;
    std::vector<std::optional<ObjectRef>> result;
    auto pointsAtMesh = [&](const assets::Value& v) {
        const auto p = assets::AsObjectPointer(v.Get("m_Mesh"));
        return p && p->fileId == 0 && p->pathId == mesh.object->pathId;
    };
    auto materialsOf = [&](const ObjectRef& renderer) {
        const auto value = c.library->Decode(renderer);
        if (const auto* list = value->Get("m_Materials"); list && list->AsArray()) {
            for (const auto& m : *list->AsArray()) {
                const auto pointer = assets::AsObjectPointer(m.get());
                result.push_back(pointer ? c.library->Resolve(renderer.cab, *pointer) : std::nullopt);
            }
        }
    };
    for (const assets::ObjectInfo& object : mesh.file->Objects()) {
        if (!result.empty()) break;
        const ObjectRef ref{mesh.file, &object, mesh.cab};
        if (object.classId == kSkinnedMeshRenderer) {
            if (pointsAtMesh(*c.library->Decode(ref))) materialsOf(ref);
        } else if (object.classId == kMeshFilter) {
            const auto filter = c.library->Decode(ref);
            if (!pointsAtMesh(*filter)) continue;
            const auto go = assets::AsObjectPointer(filter->Get("m_GameObject"));
            const auto goRef = go ? c.library->Resolve(mesh.cab, *go) : std::nullopt;
            if (!goRef) continue;
            const auto goValue = c.library->Decode(*goRef);
            const auto* components = goValue->Get("m_Component");
            if (!components || !components->AsArray()) continue;
            for (const auto& pair : *components->AsArray()) {
                const auto cp = assets::AsObjectPointer(pair->Get("component"));
                const auto compRef = cp ? c.library->Resolve(goRef->cab, *cp) : std::nullopt;
                if (compRef && compRef->object->classId == kMeshRenderer) materialsOf(*compRef);
            }
        }
    }
    return result;
}

std::optional<assets::Texture2DAsset> MainTextureOf(Content& c, const ObjectRef& material,
                                                    assets::MaterialAsset* outMaterial) {
    assets::MaterialAsset m = assets::ReadMaterial(*c.library->Decode(material));
    if (outMaterial) *outMaterial = m;
    const assets::MaterialTexture* main = m.MainTexture();
    if (!main) return std::nullopt;
    const auto texRef = c.library->Resolve(material.cab, main->texture);
    if (!texRef || texRef->object->classId != static_cast<std::int32_t>(assets::ClassId::Texture2D)) {
        return std::nullopt;
    }
    return assets::ReadTexture2D(*c.library->Decode(*texRef), c.resources->Fetcher());
}

// --- renderer helpers ------------------------------------------------------------------

struct Window {
    std::unique_ptr<platform::Platform> platform;
    std::unique_ptr<render::RenderDevice> device;
};

std::optional<Window> OpenWindow(const Options& o, const char* title) {
    std::vector<platform::GraphicsApi> candidates = render::AvailableGraphicsApis();
    if (o.renderer) {
        auto it = std::find(candidates.begin(), candidates.end(), *o.renderer);
        if (it != candidates.end()) std::rotate(candidates.begin(), it, it + 1);
    }
    platform::PlatformConfig config;
    config.frameLimit = o.frames;
    for (platform::GraphicsApi api : candidates) {
        Window w;
        w.platform = platform::CreateDesktopPlatform(config);
        if (!w.platform) break;
        platform::WindowDesc desc;
        desc.title = title;
        desc.api = api;
        if (w.platform->CreateMainWindow(desc)) w.device = render::CreateRenderDevice(*w.platform, api);
        if (w.device) {
            std::printf("renderer: %s - %s\n", platform::ToString(api), w.device->DeviceName().c_str());
            return w;
        }
    }
    std::fprintf(stderr, "no renderer could be started\n");
    return std::nullopt;
}

std::optional<render::TextureFormat> NativeFormat(std::int32_t assetFormat, bool srgb) {
    using AF = assets::TextureFormat;
    using RF = render::TextureFormat;
    switch (static_cast<AF>(assetFormat)) {
    case AF::ASTC_4x4: return srgb ? RF::ASTC_4x4_sRGB : RF::ASTC_4x4;
    case AF::ASTC_5x5: return srgb ? RF::ASTC_5x5_sRGB : RF::ASTC_5x5;
    case AF::ASTC_6x6: return srgb ? RF::ASTC_6x6_sRGB : RF::ASTC_6x6;
    case AF::ASTC_8x8: return srgb ? RF::ASTC_8x8_sRGB : RF::ASTC_8x8;
    case AF::ASTC_10x10: return srgb ? RF::ASTC_10x10_sRGB : RF::ASTC_10x10;
    case AF::ASTC_12x12: return srgb ? RF::ASTC_12x12_sRGB : RF::ASTC_12x12;
    case AF::RGBA32: return srgb ? RF::RGBA8_sRGB : RF::RGBA8;
    default: return std::nullopt;
    }
}

// Uploads a texture: original blocks where the device samples the format,
// otherwise every mip decoded to RGBA8.
render::TextureHandle UploadTexture(render::RenderDevice& device, const assets::Texture2DAsset& t,
                                    bool forceSoftware, bool* usedNative = nullptr) {
    render::TextureDesc desc;
    desc.width = t.width;
    desc.height = t.height;
    desc.mipCount = static_cast<std::uint32_t>(t.mipCount);
    const auto native = NativeFormat(t.format, t.srgb);
    if (native && !forceSoftware && device.SupportsTextureFormat(*native)) {
        desc.format = *native;
        std::vector<render::TextureLevelData> levels;
        for (int l = 0; l < t.mipCount; ++l) levels.push_back({t.Level(l).data(), t.Level(l).size()});
        if (usedNative) *usedNative = true;
        return device.CreateTexture(desc, levels);
    }
    desc.format = t.srgb ? render::TextureFormat::RGBA8_sRGB : render::TextureFormat::RGBA8;
    std::vector<std::vector<std::uint8_t>> decoded;
    std::vector<render::TextureLevelData> levels;
    for (int l = 0; l < t.mipCount; ++l) {
        decoded.push_back(t.DecodeLevel(l));
        if (decoded.back().empty()) return {};
    }
    for (const auto& d : decoded) levels.push_back({d.data(), d.size()});
    if (usedNative) *usedNative = false;
    return device.CreateTexture(desc, levels);
}

render::SamplerDesc SamplerFor(const assets::Texture2DAsset& t) {
    render::SamplerDesc s;
    s.filter = t.filterMode == 0 ? render::FilterMode::Nearest : render::FilterMode::Linear;
    s.mipFilter = t.filterMode == 2 ? render::FilterMode::Linear : render::FilterMode::Nearest;
    s.useMips = t.mipCount > 1;
    auto wrap = [](int w) {
        return w == 1 ? render::WrapMode::ClampToEdge : w >= 2 ? render::WrapMode::MirroredRepeat : render::WrapMode::Repeat;
    };
    s.wrapU = wrap(t.wrapU);
    s.wrapV = wrap(t.wrapV);
    return s;
}

// --- viewer ---------------------------------------------------------------------------

struct LitVertex {
    float position[3];
    float normal[3];
    float uv[2];
};

struct LitConstants {
    Mat4 transform;
    float lightDir[4];
    float tint[4];
    float params[4];
};
static_assert(sizeof(LitConstants) <= render::kMaxDrawConstantsSize);

struct ViewPart {
    std::uint32_t firstIndex = 0;
    std::uint32_t indexCount = 0;
    std::int32_t baseVertex = 0;
    std::optional<assets::Texture2DAsset> texture;
    std::array<float, 4> tint{1, 1, 1, 1};
};

// Draws a mesh with one texture per part until the window closes or the frame
// limit is reached.
int RunViewer(const Options& o, const assets::MeshAsset& mesh, std::vector<ViewPart> parts) {
    if (mesh.positions.empty() || mesh.indices.empty()) {
        std::fprintf(stderr, "mesh has no triangles to draw\n");
        return 1;
    }
    // The original data is left-handed with clockwise front faces. Mirroring X
    // gives the right-handed view of the same object; mirroring also flips the
    // apparent winding, so each triangle's order is reversed as well to keep
    // front faces counter-clockwise for the renderer.
    std::vector<LitVertex> vertices(mesh.vertexCount);
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    for (std::uint32_t v = 0; v < mesh.vertexCount; ++v) {
        LitVertex& out = vertices[v];
        const auto& p = mesh.positions[v];
        out.position[0] = -p[0];
        out.position[1] = p[1];
        out.position[2] = p[2];
        if (!mesh.normals.empty()) {
            out.normal[0] = -mesh.normals[v][0];
            out.normal[1] = mesh.normals[v][1];
            out.normal[2] = mesh.normals[v][2];
        } else {
            out.normal[1] = 1.0f;
        }
        const auto uv = mesh.Uv(0, v);
        out.uv[0] = uv[0];
        out.uv[1] = uv[1];
        for (int i = 0; i < 3; ++i) {
            lo[i] = std::min(lo[i], out.position[i]);
            hi[i] = std::max(hi[i], out.position[i]);
        }
    }
    const float center[3] = {(lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2, (lo[2] + hi[2]) / 2};
    const float radius = std::max(1e-3f, 0.5f * std::sqrt((hi[0] - lo[0]) * (hi[0] - lo[0]) +
                                                          (hi[1] - lo[1]) * (hi[1] - lo[1]) +
                                                          (hi[2] - lo[2]) * (hi[2] - lo[2])));

    auto window = OpenWindow(o, "meshview");
    if (!window) return 1;
    render::RenderDevice& device = *window->device;
    std::vector<std::uint32_t> indices = mesh.indices;
    for (std::size_t t = 0; t + 2 < indices.size(); t += 3) std::swap(indices[t + 1], indices[t + 2]);
    const bool wide = mesh.vertexCount > 0xFFFF;
    std::vector<std::uint16_t> narrow;
    if (!wide) {
        for (std::uint32_t i : indices) narrow.push_back(static_cast<std::uint16_t>(i));
    }
    const render::BufferHandle vb =
        device.CreateBuffer(render::BufferUsage::Vertex, vertices.data(), vertices.size() * sizeof(LitVertex));
    const render::BufferHandle ib =
        wide ? device.CreateBuffer(render::BufferUsage::Index, indices.data(), indices.size() * 4)
             : device.CreateBuffer(render::BufferUsage::Index, narrow.data(), narrow.size() * 2);

    std::vector<render::TextureHandle> textures;
    std::vector<render::SamplerDesc> samplers;
    for (const ViewPart& part : parts) {
        bool native = false;
        textures.push_back(part.texture ? UploadTexture(device, *part.texture, o.softwareDecode, &native)
                                        : render::TextureHandle{});
        samplers.push_back(part.texture ? SamplerFor(*part.texture) : render::SamplerDesc{});
        if (part.texture) {
            std::printf("texture %dx%d, %d mips, %s, %s upload\n", part.texture->width, part.texture->height,
                        part.texture->mipCount, assets::DescribeTextureFormat(part.texture->format).name,
                        native ? "native" : "decoded");
        }
    }

    render::PipelineDesc pd;
    pd.shaderProgram = "lit_textured";
    pd.vertexLayout.stride = sizeof(LitVertex);
    pd.vertexLayout.attributes = {{0, render::VertexFormat::Float3, offsetof(LitVertex, position)},
                                  {1, render::VertexFormat::Float3, offsetof(LitVertex, normal)},
                                  {2, render::VertexFormat::Float2, offsetof(LitVertex, uv)}};
    const render::PipelineHandle pipeline = device.CreatePipeline(pd);
    if (!pipeline || !vb || !ib) return 1;

    float yaw = 0.6f, pitch = 0.25f, distance = radius * 2.6f;
    float lastX = 0, lastY = 0;
    std::uint64_t frame = 0;
    while (window->platform->PumpEvents()) {
        const platform::InputState& input = window->platform->GetInput();
        if (input.Get(platform::Key::Escape).pressed) break;
        if (input.Get(platform::MouseButton::Left).down && !input.Get(platform::MouseButton::Left).pressed) {
            yaw += (input.mouseX - lastX) * 0.01f;
            pitch = std::clamp(pitch + (input.mouseY - lastY) * 0.01f, -1.5f, 1.5f);
        }
        lastX = input.mouseX;
        lastY = input.mouseY;
        distance = std::clamp(distance * std::pow(0.9f, input.mouseWheel), radius * 0.2f, radius * 50.0f);

        if (!device.BeginFrame()) continue;
        int width = 0, height = 0;
        device.GetRenderSize(width, height);
        const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        const Mat4 view = Translation(0, 0, -distance) * RotationX(pitch) * RotationY(yaw) *
                          Translation(-center[0], -center[1], -center[2]);
        const Mat4 transform = Perspective(0.8f, aspect, distance * 0.01f, distance * 10.0f) * view;

        render::ClearValues clear;
        clear.color[0] = 0.10f;
        clear.color[1] = 0.11f;
        clear.color[2] = 0.13f;
        device.BeginRenderPass(clear);
        device.BindPipeline(pipeline);
        device.BindVertexBuffer(vb);
        device.BindIndexBuffer(ib, wide ? render::IndexType::UInt32 : render::IndexType::UInt16);
        // Light from over the viewer's shoulder (model space = world space here).
        const float light[3] = {0.4f, 0.8f, 0.5f};
        for (std::size_t i = 0; i < parts.size(); ++i) {
            LitConstants c{};
            c.transform = transform;
            std::copy(light, light + 3, c.lightDir);
            c.lightDir[3] = 0.35f;
            std::copy(parts[i].tint.begin(), parts[i].tint.end(), c.tint);
            c.params[0] = parts[i].texture && parts[i].texture->srgb ? 1.0f : 0.0f;
            device.BindTexture(0, textures[i], samplers[i]);
            device.SetDrawConstants(&c, sizeof(c));
            device.DrawIndexed(parts[i].indexCount, parts[i].firstIndex, parts[i].baseVertex);
        }
        device.EndRenderPass();
        const bool last = o.frames && frame + 1 == o.frames;
        if (last && !o.screenshot.empty()) device.RequestCapture();
        device.EndFrame();
        ++frame;
    }
    if (!o.screenshot.empty()) {
        auto image = device.TakeCapture();
        if (!image || !render::WritePpm(*image, o.screenshot)) {
            std::fprintf(stderr, "no frame was captured\n");
            return 1;
        }
        std::printf("saved %dx%d screenshot to %s\n", image->width, image->height, o.screenshot.c_str());
    }
    return 0;
}

std::optional<ObjectRef> FindMesh(Content& c, const Options& o) {
    std::optional<ObjectRef> found;
    ForEachObject(c, static_cast<std::int32_t>(assets::ClassId::Mesh), [&](const ObjectRef& ref) {
        if (found) return;
        if (o.pathId ? ref.object->pathId == *o.pathId : assets::ReadObjectName(*ref.file, *ref.object) == o.meshName) {
            found = ref;
        }
    });
    return found;
}

int ViewMesh(const Options& o) {
    if (o.meshName.empty() && !o.pathId) {
        std::fprintf(stderr, "choose a mesh with --mesh <name> or --path-id <id> (see --list)\n");
        return 2;
    }
    auto content = OpenContent(o);
    if (!content) return 1;
    const auto meshRef = FindMesh(*content, o);
    if (!meshRef) {
        std::fprintf(stderr, "mesh not found\n");
        return 1;
    }
    const assets::MeshAsset mesh = assets::ReadMesh(*content->library->Decode(*meshRef), content->resources->Fetcher());
    std::printf("mesh: %u vertices, %zu indices, %zu submeshes\n", mesh.vertexCount, mesh.indices.size(),
                mesh.subMeshes.size());

    const auto materials = FindMaterials(*content, *meshRef);
    std::vector<ViewPart> parts;
    for (std::size_t i = 0; i < mesh.subMeshes.size(); ++i) {
        const assets::SubMesh& sub = mesh.subMeshes[i];
        if (sub.topology != assets::MeshTopology::Triangles) continue;
        ViewPart part;
        part.firstIndex = sub.firstIndex;
        part.indexCount = sub.indexCount;
        part.baseVertex = sub.baseVertex;
        const auto& material = i < materials.size() ? materials[i] : (materials.empty() ? std::nullopt : materials.back());
        if (material) {
            try {
                assets::MaterialAsset m;
                part.texture = MainTextureOf(*content, *material, &m);
                part.tint = m.MainColor();
            } catch (const assets::ReadError& e) {
                std::fprintf(stderr, "material: %s\n", e.what());
            }
        }
        parts.push_back(std::move(part));
    }
    if (parts.empty()) parts.push_back({0, static_cast<std::uint32_t>(mesh.indices.size()), 0, std::nullopt, {1, 1, 1, 1}});
    return RunViewer(o, mesh, std::move(parts));
}

// --- listing and checks -------------------------------------------------------------

int List(const Options& o) {
    auto content = OpenContent(o);
    if (!content) return 1;
    ForEachObject(*content, static_cast<std::int32_t>(assets::ClassId::Mesh), [&](const ObjectRef& ref) {
        std::printf("mesh    %20lld  %s\n", static_cast<long long>(ref.object->pathId),
                    assets::ReadObjectName(*ref.file, *ref.object).c_str());
    });
    ForEachObject(*content, static_cast<std::int32_t>(assets::ClassId::Texture2D), [&](const ObjectRef& ref) {
        std::printf("texture %20lld  %s\n", static_cast<long long>(ref.object->pathId),
                    assets::ReadObjectName(*ref.file, *ref.object).c_str());
    });
    return 0;
}

// Decodes every mesh and checks it against itself: vertex bounds against the
// submesh bounds stored with it, indices in range, unit normals/tangents, skin
// weights summing to one, UVs finite.
int CheckMeshes(const Options& o) {
    auto content = OpenContent(o);
    if (!content) return 1;
    int total = 0, failed = 0;
    std::uint64_t vertices = 0, indices = 0;
    double worstBounds = 0, worstNormal = 0, worstWeight = 0;
    // Winding: the cross product (b - a) x (c - a) of a front face points along the
    // vertex normals in the original data (clockwise faces in its left-handed space).
    std::uint64_t agreeing = 0, opposing = 0;
    ForEachObject(*content, static_cast<std::int32_t>(assets::ClassId::Mesh), [&](const ObjectRef& ref) {
        ++total;
        std::string problem;
        try {
            const auto mesh = assets::ReadMesh(*content->library->Decode(ref), content->resources->Fetcher());
            vertices += mesh.vertexCount;
            indices += mesh.indices.size();
            for (std::uint32_t i : mesh.indices) {
                if (i >= mesh.vertexCount) problem = "index out of range";
            }
            float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
            float slo[3] = {1e30f, 1e30f, 1e30f}, shi[3] = {-1e30f, -1e30f, -1e30f};
            for (const auto& p : mesh.positions) {
                for (int k = 0; k < 3; ++k) {
                    lo[k] = std::min(lo[k], p[static_cast<std::size_t>(k)]);
                    hi[k] = std::max(hi[k], p[static_cast<std::size_t>(k)]);
                }
            }
            for (const auto& s : mesh.subMeshes) {
                for (int k = 0; k < 3; ++k) {
                    const auto kk = static_cast<std::size_t>(k);
                    slo[k] = std::min(slo[k], s.bounds.center[kk] - s.bounds.extent[kk]);
                    shi[k] = std::max(shi[k], s.bounds.center[kk] + s.bounds.extent[kk]);
                }
            }
            float size = 1e-6f, err = 0;
            for (int k = 0; k < 3; ++k) {
                size = std::max(size, hi[k] - lo[k]);
                err = std::max({err, std::fabs(lo[k] - slo[k]), std::fabs(hi[k] - shi[k])});
            }
            const double relative = err / size;
            worstBounds = std::max(worstBounds, relative);
            if (relative > 0.01) problem = "vertex bounds differ from submesh bounds";
            for (const auto& n : mesh.normals) {
                worstNormal = std::max(worstNormal, std::fabs(std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]) - 1.0));
            }
            for (const auto& w : mesh.boneWeights) {
                worstWeight = std::max(worstWeight, std::fabs(w[0] + w[1] + w[2] + w[3] - 1.0));
            }
            for (int ch = 0; ch < assets::kMaxTexCoords; ++ch) {
                for (float u : mesh.uvs[static_cast<std::size_t>(ch)]) {
                    if (!std::isfinite(u)) problem = "non-finite UV";
                }
            }
            if (mesh.indices.size() % 3 != 0) problem = "index count not a multiple of 3";
            if (!mesh.normals.empty() && problem.empty()) {
                for (std::size_t t = 0; t + 2 < mesh.indices.size(); t += 3) {
                    const auto& a = mesh.positions[mesh.indices[t]];
                    const auto& b = mesh.positions[mesh.indices[t + 1]];
                    const auto& cc = mesh.positions[mesh.indices[t + 2]];
                    const float e1[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
                    const float e2[3] = {cc[0] - a[0], cc[1] - a[1], cc[2] - a[2]};
                    const float n[3] = {e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2],
                                        e1[0] * e2[1] - e1[1] * e2[0]};
                    float dot = 0;
                    for (std::size_t k = 0; k < 3; ++k) {
                        const float vn = mesh.normals[mesh.indices[t]][k] + mesh.normals[mesh.indices[t + 1]][k] +
                                         mesh.normals[mesh.indices[t + 2]][k];
                        dot += n[k] * vn;
                    }
                    if (dot > 0) ++agreeing;
                    if (dot < 0) ++opposing;
                }
            }
        } catch (const assets::ReadError& e) {
            problem = e.what();
        }
        if (!problem.empty()) {
            ++failed;
            std::printf("FAIL mesh %lld: %s\n", static_cast<long long>(ref.object->pathId), problem.c_str());
        }
    });
    std::printf("%d meshes, %d failed; %llu vertices, %llu indices\n", total, failed,
                static_cast<unsigned long long>(vertices), static_cast<unsigned long long>(indices));
    std::printf("worst: bounds error %.4f of size, |normal|-1 %.4f, weight sum-1 %.4f\n", worstBounds, worstNormal,
                worstWeight);
    std::printf("winding: %llu triangles agree with their vertex normals, %llu oppose\n",
                static_cast<unsigned long long>(agreeing), static_cast<unsigned long long>(opposing));
    return failed == 0 ? 0 : 1;
}

std::uint64_t Fnv1a(const std::vector<std::uint8_t>& bytes, std::uint64_t hash = 1469598103934665603ull) {
    for (std::uint8_t b : bytes) hash = (hash ^ b) * 1099511628211ull;
    return hash;
}

struct DiffStats {
    std::uint64_t texels = 0;
    std::uint64_t exact = 0;
    int maxDiff = 0;
    void Add(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b) {
        for (std::size_t i = 0; i + 3 < a.size() && i + 3 < b.size(); i += 4) {
            int d = 0;
            for (int c = 0; c < 4; ++c) d = std::max(d, std::abs(a[i + static_cast<std::size_t>(c)] - b[i + static_cast<std::size_t>(c)]));
            ++texels;
            if (d == 0) ++exact;
            maxDiff = std::max(maxDiff, d);
        }
    }
};

// Decodes every texture mip in software; where the renderer samples the format
// natively, also uploads the original blocks and compares the GPU's decode.
int CheckTextures(const Options& o) {
    auto content = OpenContent(o);
    if (!content) return 1;
    auto window = OpenWindow(o, "meshview texture check");
    struct FormatStats {
        int textures = 0, levels = 0, decodeFailures = 0;
        bool compared = false;
        DiffStats gpu;
        std::uint64_t checksum = 1469598103934665603ull;
    };
    std::map<std::string, FormatStats> stats;
    int total = 0, failed = 0;
    ForEachObject(*content, static_cast<std::int32_t>(assets::ClassId::Texture2D), [&](const ObjectRef& ref) {
        ++total;
        assets::Texture2DAsset t;
        try {
            t = assets::ReadTexture2D(*content->library->Decode(ref), content->resources->Fetcher());
        } catch (const assets::ReadError& e) {
            std::printf("skip texture %lld: %s\n", static_cast<long long>(ref.object->pathId), e.what());
            return;
        }
        FormatStats& s = stats[std::string(assets::DescribeTextureFormat(t.format).name) + (t.srgb ? " sRGB" : "")];
        ++s.textures;
        std::vector<std::vector<std::uint8_t>> decoded;
        for (int l = 0; l < t.mipCount; ++l) {
            decoded.push_back(t.DecodeLevel(l));
            ++s.levels;
            if (decoded.back().empty()) {
                ++s.decodeFailures;
                ++failed;
                return;
            }
            s.checksum = Fnv1a(decoded.back(), s.checksum);
        }
        const auto native = NativeFormat(t.format, t.srgb);
        if (!window || !native || o.softwareDecode || !window->device->SupportsTextureFormat(*native)) return;
        bool usedNative = false;
        const render::TextureHandle handle = UploadTexture(*window->device, t, false, &usedNative);
        if (!handle) return;
        for (int l = 0; l < t.mipCount; ++l) {
            if (auto image = window->device->ReadTexture(handle, static_cast<std::uint32_t>(l))) {
                s.gpu.Add(decoded[static_cast<std::size_t>(l)], image->rgba);
                s.compared = true;
            }
        }
        window->device->DestroyTexture(handle);
    });
    for (const auto& [format, s] : stats) {
        std::printf("%-16s %4d textures %5d levels, decode failures %d, checksum %016llx", format.c_str(), s.textures,
                    s.levels, s.decodeFailures, static_cast<unsigned long long>(s.checksum));
        if (s.compared) {
            std::printf(", vs GPU: %llu texels, %.4f%% exact, max diff %d", static_cast<unsigned long long>(s.gpu.texels),
                        100.0 * static_cast<double>(s.gpu.exact) / static_cast<double>(std::max<std::uint64_t>(1, s.gpu.texels)),
                        s.gpu.maxDiff);
        }
        std::printf("\n");
    }
    std::printf("%d textures, %d failed to decode\n", total, failed);
    return failed == 0 ? 0 : 1;
}

// Random 128-bit blocks are mostly valid ASTC blocks exercising every block mode,
// partition count and endpoint mode; compare our decoder with the GPU's.
int AstcSelfTest(const Options& o) {
    auto window = OpenWindow(o, "meshview ASTC self-test");
    if (!window) return 1;
    struct Size {
        assets::TextureFormat asset;
        int block;
    };
    const Size sizes[] = {{assets::TextureFormat::ASTC_4x4, 4},   {assets::TextureFormat::ASTC_5x5, 5},
                          {assets::TextureFormat::ASTC_6x6, 6},   {assets::TextureFormat::ASTC_8x8, 8},
                          {assets::TextureFormat::ASTC_10x10, 10}, {assets::TextureFormat::ASTC_12x12, 12}};
    std::uint64_t state = 0x9E3779B97F4A7C15ull;
    auto random = [&state]() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    };
    int status = 0;
    for (bool srgb : {false, true}) {
        for (const Size& size : sizes) {
            const auto format = NativeFormat(static_cast<std::int32_t>(size.asset), srgb);
            if (!window->device->SupportsTextureFormat(*format)) {
                std::printf("%s: not supported by this renderer; skipped\n", render::ToString(*format));
                continue;
            }
            const int side = static_cast<int>(std::ceil(std::sqrt(static_cast<double>(o.selftestBlocks))));
            const int width = side * size.block;
            std::vector<std::uint8_t> blocks(static_cast<std::size_t>(side * side) * 16);
            for (std::size_t i = 0; i < blocks.size(); i += 8) {
                const std::uint64_t r = random();
                std::memcpy(&blocks[i], &r, 8);
            }
            // Void-extent blocks must have their two reserved bits (10-11) set; drivers
            // differ on blocks that don't (ours reports them as errors), so generate
            // only well-formed ones.
            for (std::size_t i = 0; i < blocks.size(); i += 16) {
                if (blocks[i] == 0xFC && (blocks[i + 1] & 1)) blocks[i + 1] = static_cast<std::uint8_t>(blocks[i + 1] | 0x0C);
            }

            const auto ours = assets::astc::DecodeImage(blocks, width, width, size.block, size.block,
                                                         srgb ? assets::astc::Profile::Srgb : assets::astc::Profile::Linear);
            render::TextureDesc desc;
            desc.width = desc.height = width;
            desc.format = *format;
            const render::TextureLevelData level{blocks.data(), blocks.size()};
            const render::TextureHandle handle = window->device->CreateTexture(desc, std::span(&level, 1));
            auto image = handle ? window->device->ReadTexture(handle, 0) : std::nullopt;
            if (!image) {
                std::printf("%s: GPU readback unavailable\n", render::ToString(*format));
                continue;
            }
            DiffStats d;
            d.Add(ours, image->rgba);
            int badBlocks = 0;
            for (int by = 0; by < side; ++by) {
                for (int bx = 0; bx < side; ++bx) {
                    bool bad = false;
                    for (int y = 0; y < size.block && !bad; ++y) {
                        for (int x = 0; x < size.block && !bad; ++x) {
                            const auto i = static_cast<std::size_t>(((by * size.block + y) * width + bx * size.block + x) * 4);
                            for (int c = 0; c < 4; ++c) {
                                if (std::abs(ours[i + static_cast<std::size_t>(c)] - image->rgba[i + static_cast<std::size_t>(c)]) > 0) bad = true;
                            }
                        }
                    }
                    badBlocks += bad ? 1 : 0;
                    if (bad && o.verbose && badBlocks <= 6) { // print the block and per-texel differences
                        const std::uint8_t* blk = &blocks[static_cast<std::size_t>(by * side + bx) * 16];
                        std::printf("  block");
                        for (int k = 0; k < 16; ++k) std::printf(" %02x", blk[k]);
                        const auto i = static_cast<std::size_t>(((by * size.block) * width + bx * size.block) * 4);
                        std::printf("  ours %d,%d,%d,%d gpu %d,%d,%d,%d\n", ours[i], ours[i + 1], ours[i + 2], ours[i + 3],
                                    image->rgba[i], image->rgba[i + 1], image->rgba[i + 2], image->rgba[i + 3]);
                        for (int y = 0; y < size.block; ++y) {
                            std::printf("   ");
                            for (int x = 0; x < size.block; ++x) {
                                const auto t = static_cast<std::size_t>(((by * size.block + y) * width + bx * size.block + x) * 4);
                                int dd[4];
                                for (int c = 0; c < 4; ++c) dd[c] = ours[t + static_cast<std::size_t>(c)] - image->rgba[t + static_cast<std::size_t>(c)];
                                std::printf(" %3d/%3d/%3d/%3d", dd[0], dd[1], dd[2], dd[3]);
                            }
                            std::printf("\n");
                        }
                    }
                }
            }
            std::printf("%-16s %6d blocks: %.4f%% texels exact, max diff %d, %d blocks differ\n", render::ToString(*format),
                        side * side, 100.0 * static_cast<double>(d.exact) / static_cast<double>(d.texels), d.maxDiff,
                        badBlocks);
            if (badBlocks > 0) status = 1;
            window->device->DestroyTexture(handle);
        }
    }
    return status;
}

// A procedurally generated torus and texture fed through the same mesh/texture
// structures and upload path as game content.
int Demo(const Options& o) {
    assets::MeshAsset mesh;
    constexpr int kRings = 48, kSides = 24;
    const float major = 1.0f, minor = 0.4f;
    for (int r = 0; r <= kRings; ++r) {
        for (int s = 0; s <= kSides; ++s) {
            const float u = static_cast<float>(r) / kRings, v = static_cast<float>(s) / kSides;
            const float a = u * 6.2831853f, b = v * 6.2831853f;
            const float nx = std::cos(a) * std::cos(b), ny = std::sin(b), nz = std::sin(a) * std::cos(b);
            mesh.positions.push_back({major * std::cos(a) + minor * nx, minor * ny, major * std::sin(a) + minor * nz});
            mesh.normals.push_back({nx, ny, nz});
            mesh.uvs[0].push_back(u * 4.0f);
            mesh.uvs[0].push_back(v);
        }
    }
    mesh.uvDimension[0] = 2;
    mesh.vertexCount = static_cast<std::uint32_t>(mesh.positions.size());
    // Left-handed winding (clockwise from outside), like the game's meshes.
    for (int r = 0; r < kRings; ++r) {
        for (int s = 0; s < kSides; ++s) {
            const auto i0 = static_cast<std::uint32_t>(r * (kSides + 1) + s);
            const auto i1 = i0 + kSides + 1;
            for (std::uint32_t i : {i0, i0 + 1, i1, i1, i0 + 1, i1 + 1}) mesh.indices.push_back(i);
        }
    }

    // 64x64 RGBA32 texture with a full mip chain, colour bands per mip level.
    assets::Texture2DAsset texture;
    texture.width = texture.height = 64;
    texture.format = static_cast<std::int32_t>(assets::TextureFormat::RGBA32);
    texture.filterMode = 2;
    texture.mipCount = 7;
    texture.srgb = true;
    for (int l = 0; l < texture.mipCount; ++l) {
        const int size = 64 >> l;
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                const bool stripe = ((x * 8 / size) + (y * 2 / size)) % 2 == 0;
                texture.data.push_back(static_cast<std::uint8_t>(stripe ? 230 : 40 + l * 30));
                texture.data.push_back(static_cast<std::uint8_t>(stripe ? 120 + l * 15 : 60));
                texture.data.push_back(static_cast<std::uint8_t>(stripe ? 40 : 200));
                texture.data.push_back(255);
            }
        }
    }
    ViewPart part;
    part.indexCount = static_cast<std::uint32_t>(mesh.indices.size());
    part.texture = texture;
    return RunViewer(o, mesh, {part});
}

} // namespace

int main(int argc, char** argv) {
    Options options;
    if (!ParseOptions(argc, argv, options)) {
        PrintUsage();
        return 2;
    }
    try {
        if (options.mode == "list") return List(options);
        if (options.mode == "check-meshes") return CheckMeshes(options);
        if (options.mode == "check-textures") return CheckTextures(options);
        if (options.mode == "astc-selftest") return AstcSelfTest(options);
        if (options.mode == "demo") return Demo(options);
        return ViewMesh(options);
    } catch (const assets::ReadError& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
