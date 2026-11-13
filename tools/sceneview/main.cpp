// sceneview - development viewer for prefabs and scenes from the game's bundles,
// drawn by the engine's scene renderer (OpenGL or Vulkan).
//
//   sceneview --demo                                      synthetic scene (no game files)
//   sceneview <bundle-dir> --engine-lib <libunity.so> --list [--bundle <text>]
//   sceneview <bundle-dir> --engine-lib <libunity.so> --prefab <name|path id> [--bundle <text>]
//   sceneview <bundle-dir> --engine-lib <libunity.so> --all --bundle <text> [--grid]
//   sceneview <bundle-dir> --engine-lib <libunity.so> --stats [--bundle <text>] [--gpu]
//
// Modes:
//   --list     root game objects with renderers in the selected bundles
//   --prefab   view one root game object (first match by name, or by path id)
//   --all      view every root game object of the selected bundles together, as
//              placed (a scene); --grid lays them out side by side instead
//   --stats    instantiate every root of the selected bundles and print numbers
//              (instances, meshes/materials/textures resolved, shader mapping,
//              missing files, draw-list counts); --gpu also uploads and draws
//              them for GPU-side counts. Nothing is shown or saved.
//   --demo     the synthetic scene from rr/scene/demo_scene.h
// Selection: --bundle <text> keeps bundles whose file name contains the text
// (default: all bundles in the folder).
// Options: --renderer <vulkan|opengl>, --list-renderers, --frames <n>,
//   --screenshot <file.ppm> (synthetic content only: don't capture game content),
//   --lod <n> (force a LOD level), --no-cull, --software-decode
//
// Controls: left drag orbits, right/middle drag pans, wheel zooms; F toggles a fly
// camera (W/A/S/D/Q/E, Shift, right drag to look); R reframes; L cycles forced LOD
// levels; C toggles frustum culling; P prints frame statistics; Escape quits.
// Output derived from your game files stays on your machine.
#include "rr/assets/content_library.h"
#include "rr/assets/prefab.h"
#include "rr/assets/streamed_data.h"
#include "rr/platform/platform.h"
#include "rr/render/render_device.h"
#include "rr/scene/camera_controller.h"
#include "rr/scene/content_resolver.h"
#include "rr/scene/demo_scene.h"
#include "rr/scene/prefab_instancer.h"
#include "rr/scene/scene_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using namespace rr;

struct Options {
    enum class Mode { None, Demo, List, Prefab, All, Stats } mode = Mode::None;
    std::string input;
    std::string engineLib;
    std::string bundleFilter;
    std::string prefab;
    bool grid = false;
    bool gpu = false;
    bool listRenderers = false;
    std::optional<platform::GraphicsApi> renderer;
    std::uint64_t frames = 0;
    std::string screenshot;
    int forceLod = -1;
    bool noCull = false;
    bool softwareDecode = false;
};

void PrintUsage() {
    std::fprintf(stderr,
                 "usage: sceneview --demo [options]\n"
                 "       sceneview <bundle-dir> --engine-lib <libunity.so> (--list | --prefab <name|id> |\n"
                 "                 --all [--grid] | --stats [--gpu]) [--bundle <text>] [options]\n"
                 "options: --renderer <vulkan|opengl> --list-renderers --frames <n> --screenshot <file.ppm>\n"
                 "         --lod <n> --no-cull --software-decode\n");
}

bool ParseOptions(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        const auto value = [&]() -> const char* { return i + 1 < argc ? argv[++i] : nullptr; };
        if (a == "--demo") {
            o.mode = Options::Mode::Demo;
        } else if (a == "--list") {
            o.mode = Options::Mode::List;
        } else if (a == "--all") {
            o.mode = Options::Mode::All;
        } else if (a == "--stats") {
            o.mode = Options::Mode::Stats;
        } else if (a == "--grid") {
            o.grid = true;
        } else if (a == "--gpu") {
            o.gpu = true;
        } else if (a == "--no-cull") {
            o.noCull = true;
        } else if (a == "--software-decode") {
            o.softwareDecode = true;
        } else if (a == "--list-renderers") {
            o.listRenderers = true;
        } else if (a == "--prefab" || a == "--engine-lib" || a == "--bundle" || a == "--renderer" || a == "--frames" ||
                   a == "--screenshot" || a == "--lod") {
            const char* v = value();
            if (!v) {
                std::fprintf(stderr, "%s needs a value\n", a.c_str());
                return false;
            }
            if (a == "--prefab") {
                o.mode = Options::Mode::Prefab;
                o.prefab = v;
            } else if (a == "--engine-lib") {
                o.engineLib = v;
            } else if (a == "--bundle") {
                o.bundleFilter = v;
            } else if (a == "--renderer") {
                o.renderer = platform::ParseGraphicsApi(v);
                if (!o.renderer) {
                    std::fprintf(stderr, "unknown renderer '%s'\n", v);
                    return false;
                }
            } else if (a == "--frames") {
                o.frames = std::strtoull(v, nullptr, 10);
            } else if (a == "--screenshot") {
                o.screenshot = v;
            } else {
                o.forceLod = std::atoi(v);
            }
        } else if (!a.empty() && a[0] != '-' && o.input.empty()) {
            o.input = a;
        } else {
            std::fprintf(stderr, "unknown argument '%s'\n", a.c_str());
            return false;
        }
    }
    if (o.listRenderers) return true;
    if (o.mode == Options::Mode::None || (o.mode != Options::Mode::Demo && (o.input.empty() || o.engineLib.empty()))) {
        PrintUsage();
        return false;
    }
    return true;
}

// --- window -----------------------------------------------------------------------

struct Window {
    std::unique_ptr<platform::Platform> platform;
    std::unique_ptr<render::RenderDevice> device;
};

std::optional<Window> OpenWindow(const Options& o) {
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
        desc.title = "sceneview";
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

void PrintFrame(const scene::FrameStats& f) {
    std::printf("frame: %zu drawable instances, %zu visible, %zu culled, %zu other LOD levels; %zu draw calls "
                "(%zu opaque, %zu transparent, %zu skinned), %zu triangles, %zu pipeline changes\n",
                f.list.instances, f.list.visible, f.list.culled, f.list.lodRejected, f.drawCalls, f.list.opaqueItems,
                f.list.transparentItems, f.skinnedDraws, f.triangles, f.pipelineChanges);
}

void PrintGpu(const scene::GpuStats& g) {
    std::printf("gpu: %zu meshes (%zu failed), %zu textures (%zu native, %zu decoded, %zu failed), %zu pipelines "
                "(%zu failed), %zu meshes over the bone limit\n",
                g.meshes, g.meshFailures, g.textures, g.texturesNative, g.texturesDecoded, g.textureFailures,
                g.pipelines, g.pipelineFailures, g.bonesClamped);
}

// Shows `scene` until the window closes or the frame limit is reached.
int RunViewer(const Options& o, scene::Scene& sceneData, scene::ResourceLibrary& resources, scene::DemoScene* demo) {
    auto window = OpenWindow(o);
    if (!window) return 1;
    scene::SceneRenderer renderer(*window->device);
    renderer.forceSoftwareTextures = o.softwareDecode;

    scene::CameraController controller;
    controller.Frame(sceneData.Bounds());
    scene::DrawListOptions drawOptions;
    drawOptions.forceLod = o.forceLod;
    drawOptions.frustumCull = !o.noCull;

    render::ClearValues clear;
    clear.color[0] = 0.10f;
    clear.color[1] = 0.11f;
    clear.color[2] = 0.13f;
    std::uint64_t frame = 0;
    scene::FrameStats stats;
    bool interacted = false;
    while (window->platform->PumpEvents()) {
        const platform::InputState& input = window->platform->GetInput();
        if (input.Get(platform::Key::Escape).pressed) break;
        if (input.Get(platform::Key::L).pressed) {
            drawOptions.forceLod = drawOptions.forceLod >= 3 ? -1 : drawOptions.forceLod + 1;
            std::printf("LOD: %s%d\n", drawOptions.forceLod < 0 ? "auto " : "", drawOptions.forceLod);
        }
        if (input.Get(platform::Key::C).pressed) {
            drawOptions.frustumCull = !drawOptions.frustumCull;
            std::printf("frustum culling %s\n", drawOptions.frustumCull ? "on" : "off");
        }
        if (input.Get(platform::Key::P).pressed) {
            PrintFrame(stats);
            PrintGpu(renderer.Stats());
        }
        controller.Update(input, 1.0f / 60.0f);
        interacted |= input.Get(platform::MouseButton::Left).down || input.Get(platform::MouseButton::Right).down ||
                      input.mouseWheel != 0.0f || input.Get(platform::Key::F).pressed;

        // The demo keeps its fixed camera (reproducible captures) until the user moves.
        scene::Camera camera = controller.GetCamera();
        if (demo) {
            demo->Animate(o.frames ? static_cast<float>(frame) * 0.05f
                                   : static_cast<float>(window->platform->GetClock().Now()));
            if (!interacted) camera = demo->camera;
        }
        const bool last = o.frames && frame + 1 == o.frames;
        if (!renderer.RenderFrame(sceneData, resources, camera, clear, &stats, last && !o.screenshot.empty(), {},
                                  drawOptions)) {
            continue;
        }
        if (frame == 0) PrintFrame(stats);
        ++frame;
    }
    PrintFrame(stats);
    PrintGpu(renderer.Stats());
    if (!o.screenshot.empty()) {
        auto image = window->device->TakeCapture();
        if (!image || !render::WritePpm(*image, o.screenshot)) {
            std::fprintf(stderr, "no frame was captured\n");
            return 1;
        }
        std::printf("saved %dx%d screenshot to %s\n", image->width, image->height, o.screenshot.c_str());
    }
    return 0;
}

// --- game content -----------------------------------------------------------------

std::vector<std::uint8_t> ReadWholeFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), {}};
}

struct Content {
    std::unique_ptr<assets::ContentLibrary> library;
    std::unique_ptr<assets::ResourceStore> store;
    std::vector<fs::path> bundles; // selected
};

std::optional<Content> OpenContent(const Options& o) {
    auto strings = assets::CommonStrings::FromEngineLibrary(ReadWholeFile(o.engineLib));
    if (!strings) {
        std::fprintf(stderr, "no field-name table found in %s\n", o.engineLib.c_str());
        return std::nullopt;
    }
    Content c;
    c.library = std::make_unique<assets::ContentLibrary>(o.input, *strings);
    c.store = std::make_unique<assets::ResourceStore>(*c.library);
    for (const fs::path& b : c.library->Bundles()) {
        if (o.bundleFilter.empty() || b.filename().string().find(o.bundleFilter) != std::string::npos) {
            c.bundles.push_back(b);
        }
    }
    if (c.bundles.empty()) std::fprintf(stderr, "no bundle matches '%s'\n", o.bundleFilter.c_str());
    return c;
}

struct Root {
    assets::ObjectRef ref;
    std::string name;
    fs::path bundle;
};

std::vector<Root> FindRoots(Content& c) {
    std::vector<Root> roots;
    for (const fs::path& bundle : c.bundles) {
        for (const std::string& cab : c.library->FilesInBundle(bundle)) {
            for (const assets::ObjectRef& ref : scene::FindRootGameObjects(*c.library, cab)) {
                Root r{ref, {}, bundle};
                try {
                    const auto value = c.library->Decode(ref);
                    if (const auto* n = value->Get("m_Name"); n && n->AsString()) r.name = *n->AsString();
                } catch (const std::exception&) {
                }
                roots.push_back(std::move(r));
            }
        }
    }
    return roots;
}

std::size_t CountRenderers(const assets::Prefab& prefab) {
    std::size_t n = 0;
    for (const auto& go : prefab.objects)
        for (const auto& comp : go.components)
            if (comp.classId == static_cast<std::int32_t>(scene::ComponentClass::MeshRenderer) ||
                comp.classId == static_cast<std::int32_t>(scene::ComponentClass::SkinnedMeshRenderer))
                ++n;
    return n;
}

int List(const Options& o) {
    auto c = OpenContent(o);
    if (!c) return 1;
    assets::PrefabReader reader(*c->library);
    for (const Root& root : FindRoots(*c)) {
        std::optional<assets::Prefab> prefab;
        try {
            prefab = reader.ReadHierarchy(root.ref);
        } catch (const std::exception&) {
        }
        if (!prefab) continue;
        const std::size_t renderers = CountRenderers(*prefab);
        if (renderers == 0) continue;
        std::printf("%-48s path id %-21lld %4zu objects %4zu renderers  (%s)\n", root.name.c_str(),
                    static_cast<long long>(root.ref.object->pathId), prefab->objects.size(), renderers,
                    root.bundle.filename().string().c_str());
    }
    return 0;
}

struct Loaded {
    scene::ResourceLibrary resources;
    scene::Scene scene;
    scene::InstantiateStats instances;
    std::size_t roots = 0;
    std::size_t rootFailures = 0;
};

// Instantiates the given roots, each under the scene root; `grid` spaces them out.
void Instantiate(Content& c, const std::vector<Root>& roots, bool grid, Loaded& out) {
    assets::PrefabReader reader(*c.library);
    scene::ContentResolver resolver(*c.library, *c.store, out.resources);
    const int columns = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(roots.size())))));
    int index = 0;
    for (const Root& root : roots) {
        std::optional<assets::Prefab> prefab;
        try {
            prefab = reader.ReadHierarchy(root.ref);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "%s: %s\n", root.name.c_str(), e.what());
        }
        if (!prefab) {
            ++out.rootFailures;
            continue;
        }
        scene::PrefabInstancer instancer(out.scene, resolver, out.resources);
        const int node = instancer.Instantiate(*prefab);
        out.instances += instancer.Stats();
        ++out.roots;
        if (grid) {
            // Spread the roots out, keeping each one's own rotation and scale.
            out.scene.Nodes()[static_cast<std::size_t>(node)].localPosition = {
                static_cast<float>(index % columns) * 6.0f, 0.0f, static_cast<float>(index / columns) * 6.0f};
        }
        ++index;
    }
    out.scene.UpdateTransforms();
    out.scene.UpdateBounds(out.resources);

    const scene::ContentStats& s = resolver.Stats();
    std::printf("roots: %zu instantiated, %zu failed\n", out.roots, out.rootFailures);
    const scene::InstantiateStats& i = out.instances;
    std::printf("nodes: %zu; mesh renderers %zu, skinned renderers %zu, static batched %zu, LOD groups %zu "
                "(%zu LOD entries outside the hierarchy), other renderers (particles/lines/trails/sprites) %zu\n",
                i.nodes, i.meshRenderers, i.skinnedRenderers, i.staticBatched, i.lodGroups, i.lodRenderersMissing,
                i.otherRenderers);
    std::printf("missing: %zu meshes, %zu material slots, %zu bones; %zu undecodable components\n", i.missingMeshes,
                i.missingMaterials, i.missingBones, i.undecodedComponents);
    std::printf("resources: %zu meshes loaded (%zu failed), %zu materials (%zu failed), %zu textures (%zu failed); "
                "%zu pointers into %zu unavailable files\n",
                s.meshesLoaded, s.meshesFailed, s.materialsLoaded, s.materialsFailed, s.texturesLoaded,
                s.texturesFailed, s.unresolvedPointers, s.missingFiles.size());
    const auto kind = [&](scene::ShaderMatch m) {
        auto it = s.matchKinds.find(m);
        return it != s.matchKinds.end() ? it->second : std::size_t{0};
    };
    std::printf("material shaders: %zu known family, %zu generic (tags/pass state), %zu unresolved (shader file "
                "missing; material properties used)\n",
                kind(scene::ShaderMatch::Known), kind(scene::ShaderMatch::Generic), kind(scene::ShaderMatch::Unresolved));
    for (const auto& [name, count] : s.shaderUse) {
        std::printf("  %4zu  %s\n", count, name.empty() ? "(shader not available)" : name.c_str());
    }
    std::printf("programs:\n");
    for (const auto& [name, count] : s.programUse) std::printf("  %4zu  %s\n", count, name.c_str());
    const scene::Aabb b = out.scene.Bounds();
    if (b.Valid()) {
        std::printf("bounds: centre (%.2f, %.2f, %.2f), extent (%.2f, %.2f, %.2f)\n", b.Center().x, b.Center().y,
                    b.Center().z, b.Extent().x, b.Extent().y, b.Extent().z);
    }
}

std::vector<Root> SelectRoots(Content& c, const Options& o) {
    std::vector<Root> all = FindRoots(c);
    if (o.mode != Options::Mode::Prefab) return all;
    char* end = nullptr;
    const long long id = std::strtoll(o.prefab.c_str(), &end, 10);
    const bool byId = end && *end == '\0' && !o.prefab.empty();
    for (const Root& r : all) {
        if (byId ? r.ref.object->pathId == id : r.name == o.prefab) return {r};
    }
    std::fprintf(stderr, "no root game object '%s' in the selected bundles\n", o.prefab.c_str());
    return {};
}

int Stats(const Options& o, Content& c, Loaded& loaded) {
    // Draw list from a camera framing everything, without culling, as a sanity check.
    scene::CameraController controller;
    controller.Frame(loaded.scene.Bounds());
    const scene::Camera camera = controller.GetCamera();
    scene::DrawListOptions options;
    options.forceLod = o.forceLod;
    options.frustumCull = !o.noCull;
    scene::DrawListStats list;
    const auto items =
        scene::BuildDrawList(loaded.scene, loaded.resources, camera.view, camera.Projection(16.0f / 9.0f), options, &list);
    std::printf("draw list (framed camera): %zu drawable instances, %zu visible, %zu culled, %zu other LOD levels, "
                "%zu items (%zu opaque, %zu transparent), %zu submeshes with hidden materials\n",
                list.instances, list.visible, list.culled, list.lodRejected, items.size(), list.opaqueItems,
                list.transparentItems, list.hiddenMaterials);
    (void)c;

    // Skinning consistency: skin every vertex on the CPU with the bone matrices the
    // renderer would upload (current pose) and compare with the renderer's stored
    // bounds (m_AABB, relative to the root bone).
    std::size_t skinnedChecked = 0, skinnedInside = 0, skinnedVertices = 0;
    double worstOutside = 0.0;
    for (const scene::RenderInstance& inst : loaded.scene.Instances()) {
        if (inst.kind != scene::RendererKind::Skinned || inst.mesh == scene::kNoResource || !inst.skinBounds.Valid()) continue;
        const scene::MeshData& mesh = loaded.resources.Mesh(inst.mesh);
        if (!mesh.HasSkin() || mesh.bindPoses.empty()) continue;
        const auto bones = scene::SceneRenderer::BoneMatrices(loaded.scene, inst, mesh);
        scene::Aabb skinned;
        for (std::size_t v = 0; v < mesh.vertices.size(); ++v) {
            scene::Vec3 p;
            const auto& mv = mesh.vertices[v];
            for (int k = 0; k < 4; ++k) {
                const auto b = static_cast<std::size_t>(mesh.skin[v].boneIndices[k]);
                if (b >= bones.size()) continue;
                p = p + scene::TransformPoint(bones[b], {mv.position[0], mv.position[1], mv.position[2]}) *
                            mesh.skin[v].boneWeights[k];
            }
            skinned.Add(p);
        }
        skinnedVertices += mesh.vertices.size();
        ++skinnedChecked;
        const scene::Aabb& box = inst.worldBounds;
        const float size = std::max(scene::Length(box.Extent()), 1e-3f);
        const scene::Vec3 over = scene::Max(scene::Max(box.min - skinned.min, skinned.max - box.max), {0, 0, 0});
        const double outside = scene::Length(over) / size;
        worstOutside = std::max(worstOutside, outside);
        if (outside < 0.05) {
            ++skinnedInside;
        } else {
            std::printf("  skinned outside its bounds by %.1f%%: %s (mesh %s, %zu bones, root bone %s)\n", outside * 100.0,
                        loaded.scene.Path(inst.node).c_str(), mesh.name.c_str(), mesh.bindPoses.size(),
                        inst.rootBone >= 0 ? "set" : "none");
        }
    }
    std::printf("skinning: %zu skinned instances (%zu vertices) skinned on the CPU at the current pose; %zu within "
                "5%% of their stored bounds, worst overshoot %.1f%% of the bounds' size\n",
                skinnedChecked, skinnedVertices, skinnedInside, worstOutside * 100.0);
    if (!o.gpu) return 0;
    auto window = OpenWindow(o);
    if (!window) return 1;
    scene::SceneRenderer renderer(*window->device);
    renderer.forceSoftwareTextures = o.softwareDecode;
    renderer.Prepare(loaded.resources);
    render::ClearValues clear;
    scene::FrameStats frame;
    for (int i = 0; i < 3; ++i) renderer.RenderFrame(loaded.scene, loaded.resources, camera, clear, &frame, false, {}, options);
    PrintFrame(frame);
    PrintGpu(renderer.Stats());
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    Options o;
    if (!ParseOptions(argc, argv, o)) return 2;
    if (o.listRenderers) {
        for (platform::GraphicsApi api : render::AvailableGraphicsApis()) std::printf("%s\n", platform::ToString(api));
        return 0;
    }
    try {
        if (o.mode == Options::Mode::Demo) {
            auto demo = scene::BuildDemoScene();
            return RunViewer(o, demo->scene, demo->resources, demo.get());
        }
        if (o.mode == Options::Mode::List) return List(o);

        auto content = OpenContent(o);
        if (!content || content->bundles.empty()) return 1;
        const std::vector<Root> roots = SelectRoots(*content, o);
        if (roots.empty()) return 1;
        Loaded loaded;
        Instantiate(*content, roots, o.grid || o.mode == Options::Mode::Stats, loaded);
        if (o.mode == Options::Mode::Stats) return Stats(o, *content, loaded);
        return RunViewer(o, loaded.scene, loaded.resources, nullptr);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
