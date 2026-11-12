# Build-time script: turns one shader source into a C++ file containing
#   - the OpenGL GLSL text (common preamble + body; the #version line is added at runtime)
#   - the Vulkan SPIR-V (compiled with glslangValidator), if Vulkan is enabled
#
# Invoked as: cmake -DINPUT=... -DCOMMON=... -DOUTPUT=... -DSYMBOL=... -DSTAGE=vert|frag
#                   [-DGLSLANG=path] -DWORKDIR=... -P EmbedShader.cmake

file(READ "${COMMON}" common_text)
file(READ "${INPUT}" body_text)
set(gl_text "${common_text}\n${body_text}")

function(to_byte_list text_or_file is_file out_var)
    if(is_file)
        file(READ "${text_or_file}" hex HEX)
    else()
        file(WRITE "${WORKDIR}/${SYMBOL}.tmp" "${text_or_file}")
        file(READ "${WORKDIR}/${SYMBOL}.tmp" hex HEX)
    endif()
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    set(${out_var} "${bytes}" PARENT_SCOPE)
endfunction()

to_byte_list("${gl_text}" FALSE gl_bytes)

set(spirv_bytes "")
set(has_spirv "false")
if(GLSLANG)
    set(vk_source "${WORKDIR}/${SYMBOL}.${STAGE}")
    file(WRITE "${vk_source}" "#version 450\n#define RR_VULKAN 1\n${gl_text}")
    set(spv "${WORKDIR}/${SYMBOL}.spv")
    execute_process(
        COMMAND "${GLSLANG}" -V --quiet -o "${spv}" "${vk_source}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Compiling ${INPUT} for Vulkan failed:\n${out}${err}")
    endif()
    to_byte_list("${spv}" TRUE spirv_bytes)
    set(has_spirv "true")
endif()

if(STAGE STREQUAL "vert")
    set(stage_enum "ShaderStage::Vertex")
else()
    set(stage_enum "ShaderStage::Fragment")
endif()

file(WRITE "${OUTPUT}.new"
"// Generated from ${INPUT} - do not edit.
#include \"shader_blob.h\"

namespace rr::render::shaders {
namespace {
const unsigned char kGlsl[] = {${gl_bytes} 0x00};
const unsigned char kSpirv[] = {${spirv_bytes} 0x00};
} // namespace

extern const ShaderBlob ${SYMBOL};
const ShaderBlob ${SYMBOL} = {
    ${stage_enum},
    reinterpret_cast<const char*>(kGlsl),
    ${has_spirv} ? kSpirv : nullptr,
    ${has_spirv} ? sizeof(kSpirv) - 1 : 0,
};

} // namespace rr::render::shaders
")
file(COPY_FILE "${OUTPUT}.new" "${OUTPUT}" ONLY_IF_DIFFERENT)
file(REMOVE "${OUTPUT}.new")
