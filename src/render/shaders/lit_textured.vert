// Textured geometry lit by one directional light.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

RR_DRAW_CONSTANTS DrawConstants {
    mat4 transform; // model -> clip
    vec4 lightDir;  // xyz: direction towards the light in model space; w: ambient level
    vec4 tint;      // multiplies the albedo texture
    vec4 params;    // x: 1 = encode the output as sRGB; y: alpha cut-off (0 = off)
} uDraw;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vTexCoord;

void main() {
    vNormal = aNormal;
    vTexCoord = aTexCoord;
    gl_Position = rr_ToClip(uDraw.transform * vec4(aPosition, 1.0));
}
