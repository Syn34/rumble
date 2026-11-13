// material.h - readers for Material and Shader objects.
//
// A material names its shader and stores the values it sets: textures (with
// tiling scale/offset), floats, integers and colours, each keyed by the shader
// property name. Shaders are only identified here (name and declared
// properties); the PC renderer supplies its own shader programs.
#pragma once

#include "rr/assets/serialized_file.h"
#include "rr/assets/type_tree_value.h"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace rr::assets {

struct MaterialTexture {
    ObjectPointer texture; // pathId 0 = no texture assigned
    std::array<float, 2> scale{1.0f, 1.0f};
    std::array<float, 2> offset{0.0f, 0.0f};
};

struct MaterialAsset {
    std::string name;
    ObjectPointer shader;
    std::vector<std::string> keywords;
    int customRenderQueue = -1;
    std::map<std::string, MaterialTexture> textures;
    std::map<std::string, float> floats;
    std::map<std::string, std::int32_t> ints;
    std::map<std::string, std::array<float, 4>> colors;

    // The albedo texture: the first assigned of the usual main-texture property
    // names, else the first assigned texture. Null if none is assigned.
    const MaterialTexture* MainTexture() const;
    // The base colour (_BaseColor / _Color), white if absent.
    std::array<float, 4> MainColor() const;
};

MaterialAsset ReadMaterial(const Value& material);

struct ShaderProperty {
    std::string name;
    std::string description;
    int type = 0; // 0 colour, 1 vector, 2 float, 3 range, 4 texture, 5 int
};

// A fixed-function render state value: either a constant or the name of a
// material property that supplies it (e.g. "_SrcBlend").
struct ShaderStateValue {
    float value = 0.0f;
    std::string property; // empty: `value` is fixed

    // The value with `material`'s property applied, if it sets it.
    float Resolve(const MaterialAsset& material) const;
};

// Render state of one pass (the engine's numbering: blend factors 0 zero,
// 1 one, 5 src alpha, 10 one minus src alpha...; culling 0 off, 1 front,
// 2 back; zTest 4 less-equal, 8 always...).
struct ShaderPassState {
    std::string name; // e.g. "Universal Forward"
    std::map<std::string, std::string> tags; // e.g. LightMode
    ShaderStateValue srcBlend{1.0f, {}};
    ShaderStateValue destBlend{0.0f, {}};
    ShaderStateValue zWrite{1.0f, {}};
    ShaderStateValue zTest{4.0f, {}};
    ShaderStateValue culling{2.0f, {}};
};

struct ShaderInfo {
    std::string name; // e.g. "Universal Render Pipeline/Lit"
    std::vector<ShaderProperty> properties;
    int subShaderCount = 0;
    // Tags of the first sub-shader (Queue, RenderType, UniversalMaterialType...),
    // with upper-cased keys as the engine stores some ("QUEUE") normalised to the
    // usual spelling ("Queue").
    std::map<std::string, std::string> tags;
    std::vector<ShaderPassState> passes; // of the first sub-shader
    std::vector<std::string> keywordNames;

    const ShaderPassState* FindPass(const std::string& name) const;
    std::string Tag(const std::string& key) const;
};

ShaderInfo ReadShaderInfo(const Value& shader);

} // namespace rr::assets
