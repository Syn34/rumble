// Embedded shader sources, generated at build time (see cmake/Shaders.cmake).
#pragma once

#include <cstddef>

namespace rr::render::shaders {

enum class ShaderStage { Vertex, Fragment };

struct ShaderBlob {
    ShaderStage stage;
    const char* glsl;                 // OpenGL source without the #version line
    const unsigned char* spirv;       // Vulkan SPIR-V, or null if Vulkan is disabled
    std::size_t spirvSize;
};

struct ShaderProgramSource {
    const char* name;
    const ShaderBlob* vertex;
    const ShaderBlob* fragment;
};

const ShaderProgramSource* FindProgram(const char* name);

} // namespace rr::render::shaders
