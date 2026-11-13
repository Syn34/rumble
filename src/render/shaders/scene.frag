// Scene materials (src/engine/scene/material_mapping.cpp picks the variant).
//
// Variants:
//   (none)         unlit: texture * colour * vertex colour
//   RR_LIT         one directional light plus an ambient term
//   RR_NORMAL_MAP  (with RR_LIT) tangent-space normal map in texture slot 1
// Alpha testing is the cut-off in params.x; transparency is the pipeline's blend
// state. Output is encoded to sRGB for the UNORM swapchain.
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec4 vTangent;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec4 vColor;
layout(location = 0) out vec4 oColor;

RR_DRAW_CONSTANTS DrawConstants {
    mat4 transform;
    vec4 lightDir;
    vec4 color;
    vec4 uvTransform;
    vec4 params;
} uDraw;

RR_TEXTURE(0, uTexture0); // albedo
#ifdef RR_NORMAL_MAP
RR_TEXTURE(1, uTexture1); // tangent-space normal map
#endif

void main() {
    vec4 albedo = texture(uTexture0, vTexCoord) * uDraw.color * vColor;
    if (uDraw.params.x > 0.0 && albedo.a < uDraw.params.x) discard;
    vec3 color = albedo.rgb;
#ifdef RR_LIT
    vec3 n = normalize(vNormal);
#ifdef RR_NORMAL_MAP
    // Accepts both RGB-encoded and AG-encoded (x in alpha) normal maps: one of
    // red or alpha is always 1 in those layouts.
    vec4 encoded = texture(uTexture1, vTexCoord);
    vec2 xy = vec2(encoded.r * encoded.a, encoded.g) * 2.0 - 1.0;
    xy *= uDraw.params.y;
    vec3 ts = vec3(xy, sqrt(clamp(1.0 - dot(xy, xy), 0.0, 1.0)));
    vec3 t = vTangent.xyz - n * dot(n, vTangent.xyz);
    if (dot(t, t) > 1e-8) {
        t = normalize(t);
        vec3 b = cross(n, t) * (vTangent.w < 0.0 ? -1.0 : 1.0);
        n = normalize(t * ts.x + b * ts.y + n * ts.z);
    }
#endif
    float diffuse = max(dot(n, normalize(uDraw.lightDir.xyz)), 0.0);
    float ambient = uDraw.lightDir.w;
    color *= ambient + (1.0 - ambient) * diffuse;
#endif
    oColor = vec4(rr_LinearToSrgb(color), albedo.a);
}
