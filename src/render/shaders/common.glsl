// Shared preamble for every shader. The build prepends the #version line:
// "#version 450" + RR_VULKAN for Vulkan, "#version 410 core" for OpenGL.
//
// Conventions (identical on both APIs):
//   * clip space is Y-up with depth in [0, 1]; rr_ToClip() adapts it per API
//   * per-draw constants live in one block of at most 128 bytes, declared with
//     RR_DRAW_CONSTANTS and using only vec4/mat4 members so std140 and the
//     Vulkan push-constant layout agree

#ifdef RR_VULKAN
#define RR_DRAW_CONSTANTS layout(push_constant, std140) uniform
#else
#define RR_DRAW_CONSTANTS layout(std140) uniform
#endif

vec4 rr_ToClip(vec4 p) {
#ifdef RR_VULKAN
    p.y = -p.y;             // Vulkan's framebuffer Y points down
#else
    p.z = p.z * 2.0 - p.w;  // OpenGL expects depth in [-1, 1]
#endif
    return p;
}
