// Shared preamble for every shader. The build prepends the #version line:
// "#version 450" + RR_VULKAN for Vulkan, "#version 410 core" for OpenGL.
//
// Conventions (identical on both APIs):
//   * clip space is Y-up with depth in [0, 1]; rr_ToClip() adapts it per API
//   * per-draw constants live in one block of at most 128 bytes, declared with
//     RR_DRAW_CONSTANTS and using only vec4/mat4 members so std140 and the
//     Vulkan push-constant layout agree
//   * textures are declared with RR_TEXTURE(slot, uTexture<slot>), slot 0..3: a
//     combined image sampler at set 0 / binding <slot> on Vulkan, texture unit
//     <slot> on OpenGL (the backend finds the uniform by its uTexture<slot> name)
//   * a larger per-draw block (at most 16 KiB, std140) is declared as
//     `RR_DRAW_UNIFORMS DrawUniforms { ... } name;`: a dynamic uniform buffer at
//     set 0 / binding 4 on Vulkan, uniform block binding 1 on OpenGL

#ifdef RR_VULKAN
#define RR_DRAW_CONSTANTS layout(push_constant, std140) uniform
#define RR_TEXTURE(slot, name) layout(set = 0, binding = slot) uniform sampler2D name
#define RR_DRAW_UNIFORMS layout(set = 0, binding = 4, std140) uniform
#else
#define RR_DRAW_CONSTANTS layout(std140) uniform
#define RR_TEXTURE(slot, name) uniform sampler2D name
#define RR_DRAW_UNIFORMS layout(std140) uniform
#endif

vec4 rr_ToClip(vec4 p) {
#ifdef RR_VULKAN
    p.y = -p.y;             // Vulkan's framebuffer Y points down
#else
    p.z = p.z * 2.0 - p.w;  // OpenGL expects depth in [-1, 1]
#endif
    return p;
}

// Linear -> sRGB transfer function, for writing linear results to the (UNORM)
// swapchain images both backends render into.
vec3 rr_LinearToSrgb(vec3 c) {
    c = clamp(c, 0.0, 1.0);
    return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));
}
