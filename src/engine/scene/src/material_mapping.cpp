#include "rr/scene/material_mapping.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <initializer_list>

namespace rr::scene {

namespace {

// Engine blend factor numbers (as stored in shader pass state).
constexpr int kBlendZero = 0;
constexpr int kBlendOne = 1;
constexpr int kBlendSrcAlpha = 5;
constexpr int kBlendOneMinusSrcAlpha = 10;

constexpr int kQueueAlphaTest = 2450;
constexpr int kQueueTransparent = 3000;
constexpr int kQueueLastOpaque = 2500;

std::string Lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool StartsWith(const std::string& s, const char* prefix) { return s.rfind(prefix, 0) == 0; }
bool Contains(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }

bool HasKeyword(const assets::MaterialAsset& m, const char* keyword) {
    return std::find(m.keywords.begin(), m.keywords.end(), keyword) != m.keywords.end();
}

// Whether the shader declares a property (always true without a shader, so the
// material's own properties are all considered then).
bool Declares(const assets::ShaderInfo* shader, const std::string& property) {
    if (!shader || shader->properties.empty()) return true;
    return std::any_of(shader->properties.begin(), shader->properties.end(),
                       [&](const assets::ShaderProperty& p) { return p.name == property; });
}

bool Assigned(const assets::MaterialAsset& m, const std::string& property) {
    auto it = m.textures.find(property);
    return it != m.textures.end() && it->second.texture.pathId != 0;
}

const float* FindFloat(const assets::MaterialAsset& m, const assets::ShaderInfo* shader, const char* property) {
    if (!Declares(shader, property)) return nullptr;
    auto it = m.floats.find(property);
    return it != m.floats.end() ? &it->second : nullptr;
}

// Texture property names that hold albedo, in order of preference.
constexpr const char* kAlbedoProperties[] = {
    "_BaseMap", "_MainTex", "_Diffuse", "_BaseColorMap", "_Base_Color", "_MainTexture", "Base_Map",
    "_Albedo", "_AlbedoMap", "_BaseTexture", "_TextureMain", "_BaseColor_01", "_Texture2D",
};
constexpr const char* kNormalProperties[] = {"_BumpMap", "_NormalMap", "_Normal", "_NormalTex", "_Normal_Map"};
constexpr const char* kColorProperties[] = {"_BaseColor", "_Color", "_DiffuseColor", "_MainColor", "_TintColor"};
constexpr const char* kNormalScaleProperties[] = {"_BumpScale", "_NormalScale", "_NormalStrength", "_Normal_Strenght",
                                                  "_Normal_Strength"};

// Textures that are clearly not albedo even if nothing better is assigned.
bool LooksLikeNonAlbedo(const std::string& property) {
    const std::string p = Lower(property);
    for (const char* part : {"normal", "bump", "mask", "masque", "emissi", "noise", "mrao", "msao", "metal", "gloss",
                             "occlusion", "parallax", "detail", "lightmap", "shadowmask", "cubemap", "matcap",
                             "dissolve", "gradient", "unity_"}) {
        if (Contains(p, part)) return true;
    }
    return false;
}

std::string PickAlbedo(const assets::MaterialAsset& m, const assets::ShaderInfo* shader) {
    for (const char* p : kAlbedoProperties) {
        if (Assigned(m, p) && Declares(shader, p)) return p;
    }
    for (const auto& [property, texture] : m.textures) {
        if (texture.texture.pathId != 0 && Declares(shader, property) && !LooksLikeNonAlbedo(property)) {
            return property;
        }
    }
    return {};
}

std::string PickNormal(const assets::MaterialAsset& m, const assets::ShaderInfo* shader) {
    for (const char* p : kNormalProperties) {
        if (Assigned(m, p) && Declares(shader, p)) return p;
    }
    return {};
}

std::array<float, 4> PickColor(const assets::MaterialAsset& m, const assets::ShaderInfo* shader) {
    for (const char* p : kColorProperties) {
        if (!Declares(shader, p)) continue;
        if (auto it = m.colors.find(p); it != m.colors.end()) return it->second;
    }
    return {1, 1, 1, 1};
}

SurfaceType SurfaceFromBlend(int src, int dst) {
    if (src == kBlendOne && dst == kBlendZero) return SurfaceType::Opaque;
    if (dst == kBlendOne) return SurfaceType::Additive;
    if (src == kBlendOne && dst == kBlendOneMinusSrcAlpha) return SurfaceType::Premultiplied;
    if (src == kBlendSrcAlpha && dst == kBlendOneMinusSrcAlpha) return SurfaceType::Transparent;
    return SurfaceType::Transparent; // other combinations (multiply...) approximated
}

render::CullMode CullFromValue(float v) {
    switch (static_cast<int>(std::lround(v))) {
    case 0: return render::CullMode::None;
    case 1: return render::CullMode::Front;
    default: return render::CullMode::Back;
    }
}

// The pass that draws the object in forward rendering.
const assets::ShaderPassState* ForwardPass(const assets::ShaderInfo& shader) {
    for (const assets::ShaderPassState& p : shader.passes) {
        auto mode = p.tags.find("LightMode");
        if (mode != p.tags.end() && (mode->second == "UniversalForward" || mode->second == "UniversalForwardOnly" ||
                                     mode->second == "SRPDefaultUnlit" || mode->second == "ForwardBase" ||
                                     mode->second == "Always" || mode->second == "Vertex")) {
            return &p;
        }
    }
    for (const char* name : {"Universal Forward", "ForwardLit", "Unlit", "FORWARD"}) {
        if (const auto* p = shader.FindPass(name)) return p;
    }
    // Skip passes that only feed other render stages.
    for (const assets::ShaderPassState& p : shader.passes) {
        auto mode = p.tags.find("LightMode");
        const std::string m = mode != p.tags.end() ? mode->second : std::string{};
        if (m != "ShadowCaster" && m != "DepthOnly" && m != "DepthNormals" && m != "DepthNormalsOnly" &&
            m != "Meta" && m != "MotionVectors" && m != "Universal2D" && m != "UniversalGBuffer" && m != "SceneSelectionPass" &&
            m != "Picking") {
            return &p;
        }
    }
    return shader.passes.empty() ? nullptr : &shader.passes.front();
}

struct Family {
    bool known = false;
    bool visible = true;
    bool unlit = false;
    bool vertexColor = false;
};

// Shader families recognised by name. Everything else is decided from tags/state.
Family IdentifyFamily(const std::string& name) {
    Family f;
    const auto set = [&](bool unlit, bool vertexColor = false) {
        f.known = true;
        f.unlit = unlit;
        f.vertexColor = vertexColor;
    };
    if (name == "Universal Render Pipeline/Lit" || name == "Universal Render Pipeline/Simple Lit" ||
        name == "Universal Render Pipeline/Complex Lit" || name == "Universal Render Pipeline/Baked Lit" ||
        name == "Legacy Shaders/VertexLit" || name == "Standard" || name == "Legacy Shaders/Diffuse") {
        set(false);
    } else if (name == "Universal Render Pipeline/Unlit" || name == "Unlit/Texture" || name == "Unlit/Color" ||
               name == "Unlit/Transparent" || name == "Unlit/Transparent Cutout" ||
               name == "Unlit/Transparent Additive" || name == "Shader Graphs/RS_Unlit_NoFog") {
        set(true);
    } else if (StartsWith(name, "Universal Render Pipeline/Particles/") || StartsWith(name, "Particles/") ||
               StartsWith(name, "Legacy Shaders/Particles/") || name == "Sprites/Default" ||
               StartsWith(name, "UI/")) {
        set(true, true);
    } else if (name == "Hidden/Shader Graph/FallbackError" || name == "Hidden/InternalErrorShader") {
        set(true);
    } else if (StartsWith(name, "Skybox/") || StartsWith(name, "Custom/Outline") ||
               StartsWith(name, "RattleSnake/UI/PostProcess/") || name == "Shader Graphs/RS_InvisibleWall") {
        // Drawn by stages the PC renderer doesn't have (sky, outlines, post
        // effects) or never visible in game.
        set(true);
        f.visible = false;
    } else if (name == "Shader Graphs/RS_ToonShading_SimulatedUniverse" ||
               name == "Shader Graphs/SH_Enviro_Simulated_Universe_01" || name == "Shader Graphs/RS_Skyscrapper") {
        set(false);
    } else if (name == "Shader Graphs/RS_Hologram3D" || name == "Shader Graphs/SH_TrajectoryPreview" ||
               name == "Shader Graphs/RS_KOTH_Zone" || name == "Shader Graphs/RS_AdventureTrack_Transition" ||
               StartsWith(name, "RattleSnake/UI/")) {
        set(true);
    }
    return f;
}

float AlphaCutoff(const assets::MaterialAsset& m, const assets::ShaderInfo* shader) {
    for (const char* p : {"_Cutoff", "_AlphaClipThreshold", "_AlphaCutoff"}) {
        if (const float* v = FindFloat(m, shader, p); v && *v > 0.0f) return *v;
    }
    return 0.5f;
}

} // namespace

const char* ToString(ShadingModel shading) {
    switch (shading) {
    case ShadingModel::Unlit: return "unlit";
    case ShadingModel::Lit: return "lit";
    case ShadingModel::LitNormalMap: return "lit+normal";
    }
    return "?";
}

const char* ToString(SurfaceType surface) {
    switch (surface) {
    case SurfaceType::Opaque: return "opaque";
    case SurfaceType::AlphaTested: return "alpha-tested";
    case SurfaceType::Transparent: return "transparent";
    case SurfaceType::Additive: return "additive";
    case SurfaceType::Premultiplied: return "premultiplied";
    }
    return "?";
}

int ParseRenderQueue(const std::string& tag, int fallback) {
    if (tag.empty()) return fallback;
    struct Named {
        const char* name;
        int queue;
    };
    static constexpr Named kNamed[] = {{"Background", 1000}, {"Geometry", 2000}, {"AlphaTest", 2450},
                                       {"Transparent", 3000}, {"Overlay", 4000}};
    const std::string lower = Lower(tag);
    for (const Named& n : kNamed) {
        const std::string name = Lower(n.name);
        if (StartsWith(lower, name.c_str())) {
            const std::string rest = tag.substr(name.size());
            int offset = 0;
            if (!rest.empty()) offset = std::atoi(rest.c_str()); // "+1" / "-10"
            return n.queue + offset;
        }
    }
    char* end = nullptr;
    const long v = std::strtol(tag.c_str(), &end, 10);
    return end && *end == '\0' ? static_cast<int>(v) : fallback;
}

float SrgbToLinear(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

MaterialDesc MapMaterial(const assets::MaterialAsset& m, const assets::ShaderInfo* shader) {
    MaterialDesc d;
    d.name = m.name;

    bool unlit = false;
    int src = kBlendOne, dst = kBlendZero;
    float zWrite = 1.0f, cull = 2.0f;
    int queue = 2000;

    if (shader) {
        d.shaderName = shader->name;
        const Family family = IdentifyFamily(shader->name);
        d.match = family.known ? ShaderMatch::Known : ShaderMatch::Generic;
        d.visible = family.visible;
        d.vertexColor = family.vertexColor;
        const std::string target = shader->Tag("ShaderGraphTargetId");
        unlit = family.known ? family.unlit
                             : (Contains(shader->name, "Unlit") || shader->Tag("UniversalMaterialType") == "Unlit" ||
                                Contains(target, "Unlit"));
        queue = ParseRenderQueue(shader->Tag("Queue"), 2000);
        if (const assets::ShaderPassState* pass = ForwardPass(*shader)) {
            src = static_cast<int>(std::lround(pass->srcBlend.Resolve(m)));
            dst = static_cast<int>(std::lround(pass->destBlend.Resolve(m)));
            zWrite = pass->zWrite.Resolve(m);
            cull = pass->culling.Resolve(m);
            if (!family.known && pass->tags.count("LightMode") && pass->tags.at("LightMode") == "SRPDefaultUnlit") {
                unlit = true;
            }
        }
    } else {
        // No shader: use what the material says about itself. Materials carry the
        // render pipeline's standard surface properties even when their shader is
        // a custom one, so these are only a best guess.
        d.match = ShaderMatch::Unresolved;
        const auto value = [&](const char* p, float fallback) {
            auto it = m.floats.find(p);
            return it != m.floats.end() ? it->second : fallback;
        };
        const bool transparent = HasKeyword(m, "_SURFACE_TYPE_TRANSPARENT") || value("_Surface", 0) > 0.5f ||
                                 m.customRenderQueue >= kQueueTransparent;
        if (transparent) {
            src = static_cast<int>(value("_SrcBlend", kBlendSrcAlpha));
            dst = static_cast<int>(value("_DstBlend", kBlendOneMinusSrcAlpha));
            if (src == kBlendOne && dst == kBlendZero) { // stale opaque factors on a transparent material
                src = kBlendSrcAlpha;
                dst = kBlendOneMinusSrcAlpha;
            }
            zWrite = value("_ZWrite", 0);
            queue = kQueueTransparent;
        } else {
            zWrite = value("_ZWrite", 1);
        }
        cull = value("_Cull", 2);
        const std::string lowerName = Lower(m.name);
        unlit = Contains(lowerName, "unlit") || Contains(lowerName, "_ui_") || Contains(lowerName, "fx_");
    }

    if (m.customRenderQueue >= 0) queue = m.customRenderQueue;
    d.renderQueue = queue;
    d.surface = SurfaceFromBlend(src, dst);
    d.depthWrite = zWrite > 0.5f;
    d.cull = CullFromValue(cull);

    // Alpha testing: the render pipeline's keyword/toggle, or an alpha-test queue.
    const float* clip = FindFloat(m, shader, "_AlphaClip");
    const bool alphaTest = HasKeyword(m, "_ALPHATEST_ON") || (clip && *clip > 0.5f) ||
                           (queue >= kQueueAlphaTest && queue < kQueueLastOpaque);
    if (d.surface == SurfaceType::Opaque && alphaTest) {
        d.surface = SurfaceType::AlphaTested;
    }
    if (alphaTest) d.alphaCutoff = AlphaCutoff(m, shader);
    if (HasKeyword(m, "_ALPHAPREMULTIPLY_ON") && d.surface == SurfaceType::Transparent) {
        d.surface = SurfaceType::Premultiplied;
    }

    d.albedoProperty = PickAlbedo(m, shader);
    if (!d.albedoProperty.empty()) {
        const assets::MaterialTexture& t = m.textures.at(d.albedoProperty);
        d.uvTransform = {t.scale[0], t.scale[1], t.offset[0], t.offset[1]};
    }
    const std::array<float, 4> c = PickColor(m, shader);
    d.color = {SrgbToLinear(c[0]), SrgbToLinear(c[1]), SrgbToLinear(c[2]), std::clamp(c[3], 0.0f, 1.0f)};

    if (unlit) {
        d.shading = ShadingModel::Unlit;
    } else {
        d.normalProperty = PickNormal(m, shader);
        d.shading = d.normalProperty.empty() ? ShadingModel::Lit : ShadingModel::LitNormalMap;
        for (const char* p : kNormalScaleProperties) {
            if (const float* v = FindFloat(m, shader, p)) {
                d.normalScale = *v;
                break;
            }
        }
    }
    return d;
}

void FinishMaterial(MaterialDesc& material, TextureId albedo, TextureId normal) {
    material.albedo = albedo;
    material.normal = normal;
    if (material.shading == ShadingModel::LitNormalMap && normal == kNoResource) {
        material.shading = ShadingModel::Lit;
    }
}

} // namespace rr::scene
