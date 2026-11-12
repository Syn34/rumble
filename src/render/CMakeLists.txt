# rr_render: API-neutral rendering interface with OpenGL and Vulkan backends.
include(${PROJECT_SOURCE_DIR}/cmake/Shaders.cmake)

add_library(rr_render src/render_device.cpp)
add_library(rr::render ALIAS rr_render)
target_include_directories(rr_render PUBLIC include PRIVATE src)
target_link_libraries(rr_render PUBLIC rr::platform)
rr_set_warnings(rr_render)

rr_add_shader_programs(rr_render
    DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    COMMON ${CMAKE_CURRENT_SOURCE_DIR}/shaders/common.glsl
    PROGRAMS
        unlit_color
)

if(RR_ENABLE_OPENGL)
    target_sources(rr_render PRIVATE src/gl/gl_device.cpp)
    target_compile_definitions(rr_render PRIVATE RR_HAS_OPENGL=1)
endif()

if(RR_ENABLE_VULKAN)
    target_sources(rr_render PRIVATE src/vk/vk_device.cpp)
    target_compile_definitions(rr_render PRIVATE RR_HAS_VULKAN=1)
    target_link_libraries(rr_render PRIVATE Vulkan::Vulkan)
    # Vulkan structs are initialised as `VkFoo info{VK_STRUCTURE_TYPE_FOO};`,
    # which value-initialises the remaining members on purpose.
    if(NOT MSVC)
        set_source_files_properties(src/vk/vk_device.cpp PROPERTIES
            COMPILE_OPTIONS "-Wno-missing-field-initializers")
    endif()
endif()
