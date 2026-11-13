#include "rr/assets/material.h"

#include "rr/assets/content_library.h"

namespace rr::assets {

namespace {

const Value::Array& ArrayField(const Value* v, const char* name) {
    static const Value::Array empty;
    const Value* f = v ? v->Get(name) : nullptr;
    const Value::Array* a = f ? f->AsArray() : nullptr;
    return a ? *a : empty;
}

std::string StringField(const Value* v, const char* name) {
    const Value* f = v ? v->Get(name) : nullptr;
    return f && f->AsString() ? *f->AsString() : std::string{};
}

float FloatField(const Value* v, const char* name, float fallback = 0.0f) {
    const Value* f = v ? v->Get(name) : nullptr;
    return f ? static_cast<float>(f->AsDouble(fallback)) : fallback;
}

// Property sheets store maps as arrays of {first: key, second: value} pairs.
template <typename Fn> void ForEachPair(const Value* sheet, const char* field, Fn&& fn) {
    for (const ValuePtr& pair : ArrayField(sheet, field)) {
        const Value* key = pair->Get("first");
        const Value* value = pair->Get("second");
        if (key && key->AsString() && value) fn(*key->AsString(), *value);
    }
}

} // namespace

const MaterialTexture* MaterialAsset::MainTexture() const {
    for (const char* property : {"_BaseMap", "_MainTex", "_BaseColorMap", "_AlbedoMap", "_Albedo", "_BaseTexture"}) {
        auto it = textures.find(property);
        if (it != textures.end() && it->second.texture.pathId != 0) return &it->second;
    }
    for (const auto& [property, texture] : textures) {
        if (texture.texture.pathId != 0) return &texture;
    }
    return nullptr;
}

std::array<float, 4> MaterialAsset::MainColor() const {
    for (const char* property : {"_BaseColor", "_Color"}) {
        if (auto it = colors.find(property); it != colors.end()) return it->second;
    }
    return {1.0f, 1.0f, 1.0f, 1.0f};
}

MaterialAsset ReadMaterial(const Value& material) {
    MaterialAsset out;
    out.name = StringField(&material, "m_Name");
    if (auto shader = AsObjectPointer(material.Get("m_Shader"))) out.shader = *shader;
    for (const ValuePtr& k : ArrayField(&material, "m_ValidKeywords")) {
        if (k->AsString()) out.keywords.push_back(*k->AsString());
    }
    if (const Value* q = material.Get("m_CustomRenderQueue")) out.customRenderQueue = static_cast<int>(q->AsInt(-1));

    const Value* sheet = material.Get("m_SavedProperties");
    ForEachPair(sheet, "m_TexEnvs", [&](const std::string& key, const Value& env) {
        MaterialTexture t;
        if (auto pointer = AsObjectPointer(env.Get("m_Texture"))) t.texture = *pointer;
        const Value* scale = env.Get("m_Scale");
        const Value* offset = env.Get("m_Offset");
        t.scale = {FloatField(scale, "x", 1.0f), FloatField(scale, "y", 1.0f)};
        t.offset = {FloatField(offset, "x"), FloatField(offset, "y")};
        out.textures[key] = t;
    });
    ForEachPair(sheet, "m_Floats", [&](const std::string& key, const Value& v) {
        out.floats[key] = static_cast<float>(v.AsDouble());
    });
    ForEachPair(sheet, "m_Ints", [&](const std::string& key, const Value& v) {
        out.ints[key] = static_cast<std::int32_t>(v.AsInt());
    });
    ForEachPair(sheet, "m_Colors", [&](const std::string& key, const Value& v) {
        out.colors[key] = {FloatField(&v, "r"), FloatField(&v, "g"), FloatField(&v, "b"), FloatField(&v, "a")};
    });
    return out;
}

ShaderInfo ReadShaderInfo(const Value& shader) {
    ShaderInfo out;
    const Value* parsed = shader.Get("m_ParsedForm");
    out.name = StringField(parsed, "m_Name");
    if (out.name.empty()) out.name = StringField(&shader, "m_Name");
    const Value* propInfo = parsed ? parsed->Get("m_PropInfo") : nullptr;
    for (const ValuePtr& p : ArrayField(propInfo, "m_Props")) {
        ShaderProperty prop;
        prop.name = StringField(p.get(), "m_Name");
        prop.description = StringField(p.get(), "m_Description");
        if (const Value* type = p->Get("m_Type")) prop.type = static_cast<int>(type->AsInt());
        out.properties.push_back(std::move(prop));
    }
    out.subShaderCount = static_cast<int>(ArrayField(parsed, "m_SubShaders").size());
    return out;
}

} // namespace rr::assets
