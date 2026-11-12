// Unlit geometry with per-vertex colour, transformed by one matrix.
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;

RR_DRAW_CONSTANTS DrawConstants {
    mat4 transform;
    vec4 tint;
} uDraw;

layout(location = 0) out vec4 vColor;

void main() {
    vColor = aColor * uDraw.tint;
    gl_Position = rr_ToClip(uDraw.transform * vec4(aPosition, 1.0));
}
