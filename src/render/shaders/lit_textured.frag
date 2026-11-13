layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vTexCoord;
layout(location = 0) out vec4 oColor;

RR_DRAW_CONSTANTS DrawConstants {
    mat4 transform;
    vec4 lightDir;
    vec4 tint;
    vec4 params;
} uDraw;

RR_TEXTURE(0, uTexture0); // albedo

void main() {
    vec4 albedo = texture(uTexture0, vTexCoord) * uDraw.tint;
    if (uDraw.params.y > 0.0 && albedo.a < uDraw.params.y) discard;
    float diffuse = max(dot(normalize(vNormal), normalize(uDraw.lightDir.xyz)), 0.0);
    float ambient = uDraw.lightDir.w;
    vec3 color = albedo.rgb * (ambient + (1.0 - ambient) * diffuse);
    if (uDraw.params.x > 0.5) color = rr_LinearToSrgb(color);
    oColor = vec4(color, albedo.a);
}
