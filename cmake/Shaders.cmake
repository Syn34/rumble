# rr_add_shader_programs(<target> COMMON <file> PROGRAMS <name>...)
#
# Each program <name> is made of shaders/<name>.vert and shaders/<name>.frag.
# A program can also be spelled <name>=<vert>:<frag>[:<DEFINE>,<DEFINE>...] to
# build it from shaders/<vert>.vert and shaders/<frag>.frag with the given
# preprocessor symbols defined (as 1) in both stages, so one source can give
# several variants. Both stages are embedded into <target> for every enabled
# graphics API, and a registry is generated so backends can look programs up by
# name at runtime.
function(rr_add_shader_programs target)
    cmake_parse_arguments(ARG "" "COMMON;DIR" "PROGRAMS" ${ARGN})
    set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY "${gen_dir}")

    set(glslang "")
    if(RR_ENABLE_VULKAN)
        set(glslang "${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
    endif()

    set(registry_decls "")
    set(registry_entries "")
    foreach(spec IN LISTS ARG_PROGRAMS)
        set(defines "")
        if(spec MATCHES "^([A-Za-z0-9_]+)=([A-Za-z0-9_]+):([A-Za-z0-9_]+)(:([A-Za-z0-9_,]+))?$")
            set(program "${CMAKE_MATCH_1}")
            set(source_vert "${CMAKE_MATCH_2}")
            set(source_frag "${CMAKE_MATCH_3}")
            set(defines "${CMAKE_MATCH_5}")
        else()
            set(program "${spec}")
            set(source_vert "${spec}")
            set(source_frag "${spec}")
        endif()
        foreach(stage vert frag)
            set(input "${ARG_DIR}/${source_${stage}}.${stage}")
            set(symbol "${program}_${stage}")
            set(output "${gen_dir}/${symbol}.cpp")
            add_custom_command(
                OUTPUT "${output}"
                COMMAND "${CMAKE_COMMAND}"
                    "-DINPUT=${input}" "-DCOMMON=${ARG_COMMON}" "-DOUTPUT=${output}"
                    "-DSYMBOL=${symbol}" "-DSTAGE=${stage}" "-DGLSLANG=${glslang}"
                    "-DWORKDIR=${gen_dir}" "-DDEFINES=${defines}"
                    -P "${PROJECT_SOURCE_DIR}/cmake/EmbedShader.cmake"
                DEPENDS "${input}" "${ARG_COMMON}" "${PROJECT_SOURCE_DIR}/cmake/EmbedShader.cmake"
                COMMENT "Embedding shader ${program}.${stage}"
                VERBATIM)
            target_sources(${target} PRIVATE "${output}")
            string(APPEND registry_decls "extern const ShaderBlob ${symbol};\n")
        endforeach()
        string(APPEND registry_entries
               "    {\"${program}\", &${program}_vert, &${program}_frag},\n")
    endforeach()

    file(CONFIGURE OUTPUT "${gen_dir}/shader_registry.cpp" CONTENT
"// Generated - do not edit.
#include \"shader_blob.h\"

#include <cstring>

namespace rr::render::shaders {
${registry_decls}
namespace {
const ShaderProgramSource kPrograms[] = {
${registry_entries}};
} // namespace

const ShaderProgramSource* FindProgram(const char* name) {
    for (const auto& p : kPrograms) {
        if (std::strcmp(p.name, name) == 0) return &p;
    }
    return nullptr;
}

} // namespace rr::render::shaders
")
    target_sources(${target} PRIVATE "${gen_dir}/shader_registry.cpp")
endfunction()
