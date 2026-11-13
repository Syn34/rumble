// material_mapping.h - maps the game's materials onto the PC shader programs.
//
// The game's shaders (render-pipeline shaders, shader graphs, a few built-in and
// custom ones) only exist as compiled mobile programs, so the PC renderer draws
// every material with one of a small set of its own programs (scene_* in
// src/render/shaders). This file decides, per material, which one and with what
// state:
//
//   shading:  unlit, lit, or lit with a tangent-space normal map
//   surface:  opaque, alpha-tested (cut-off), alpha-blended, additive or
//             premultiplied
//   state:    face culling, depth writes, render queue
//   inputs:   albedo texture (+ tiling), normal map, base colour, cut-off,
//             normal strength, whether vertex colours tint the result
//
// When the material's shader object can be read, its name, sub-shader tags
// (Queue, RenderType, UniversalMaterialType...) and the forward pass's blend /
// depth / cull state (with the material's overriding properties applied) decide.
// Without it (the shader lives in a bundle that isn't available), the decision
// falls back to the material's own keywords, render queue and properties.
#pragma once

#include "rr/assets/material.h"
#include "rr/render/render_device.h"
#include "rr/scene/ids.h"

#include <array>
#include <string>

namespace rr::scene {

enum class ShadingModel { Unlit, Lit, LitNormalMap };
enum class SurfaceType { Opaque, AlphaTested, Transparent, Additive, Premultiplied };

const char* ToString(ShadingModel shading);
const char* ToString(SurfaceType surface);

// How the decision was made.
enum class ShaderMatch {
    Known,      // the shader was read and its family is known by name
    Generic,    // the shader was read; tags and pass state decided
    Unresolved, // no shader object; the material's properties decided
};

struct MaterialDesc {
    std::string name;
    std::string shaderName; // empty when unresolved
    ShaderMatch match = ShaderMatch::Unresolved;
    bool visible = true;    // false for helper materials drawn by passes we don't have (outlines, skyboxes)

    ShadingModel shading = ShadingModel::Lit;
    SurfaceType surface = SurfaceType::Opaque;
    render::CullMode cull = render::CullMode::Back;
    bool depthWrite = true;
    int renderQueue = 2000;

    std::array<float, 4> color{1, 1, 1, 1}; // linear
    std::array<float, 4> uvTransform{1, 1, 0, 0};
    float alphaCutoff = 0.0f; // > 0 for AlphaTested
    float normalScale = 1.0f;
    bool vertexColor = false;

    // Material properties of the textures used (empty: none), and the loaded ones.
    std::string albedoProperty;
    std::string normalProperty;
    TextureId albedo = kNoResource;
    TextureId normal = kNoResource;

    bool Transparent() const {
        return surface == SurfaceType::Transparent || surface == SurfaceType::Additive ||
               surface == SurfaceType::Premultiplied;
    }
};

// Decides how to draw `material`. `shader` is null when the shader object isn't
// available. Texture ids are left unset; the caller loads albedoProperty /
// normalProperty and fills them in (a normal map that can't be loaded should
// downgrade LitNormalMap to Lit, see FinishMaterial).
MaterialDesc MapMaterial(const assets::MaterialAsset& material, const assets::ShaderInfo* shader);

// Applies the loaded texture ids and downgrades the shading when the normal map is
// missing.
void FinishMaterial(MaterialDesc& material, TextureId albedo, TextureId normal);

// Parses a Queue tag ("Geometry", "AlphaTest+1", "Transparent-10", "2450").
int ParseRenderQueue(const std::string& tag, int fallback = 2000);

// sRGB-encoded colour component -> linear.
float SrgbToLinear(float c);

} // namespace rr::scene
