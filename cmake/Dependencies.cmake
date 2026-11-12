# Third-party libraries for the PC side (windowing, graphics APIs).
#
# SDL2 is looked up on the system first (package manager, vcpkg, or SDL2_DIR).
# If it isn't found and RR_FETCH_DEPENDENCIES is ON, a pinned release is
# downloaded and built as part of the project.

include(FetchContent)

if(RR_ENABLE_DESKTOP)
    find_package(SDL2 CONFIG QUIET)
    if(NOT SDL2_FOUND AND RR_FETCH_DEPENDENCIES)
        message(STATUS "SDL2 not found on the system; fetching SDL 2.30.x")
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(SDL_TEST OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(SDL2
            URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-2.30.9.tar.gz)
        FetchContent_MakeAvailable(SDL2)
    endif()

    add_library(rr_sdl2 INTERFACE)
    add_library(rr::sdl2 ALIAS rr_sdl2)
    if(TARGET SDL2::SDL2-static)
        target_link_libraries(rr_sdl2 INTERFACE SDL2::SDL2-static)
    elseif(TARGET SDL2::SDL2)
        target_link_libraries(rr_sdl2 INTERFACE SDL2::SDL2)
    else()
        message(FATAL_ERROR "SDL2 is required for the desktop build. Install it, set SDL2_DIR, "
                            "enable RR_FETCH_DEPENDENCIES, or configure with -DRR_ENABLE_DESKTOP=OFF.")
    endif()
    # SDL_main redefinition is not wanted; we provide a normal main().
    target_compile_definitions(rr_sdl2 INTERFACE SDL_MAIN_HANDLED)
endif()

if(RR_ENABLE_VULKAN)
    find_package(Vulkan QUIET COMPONENTS glslangValidator)
    if(NOT Vulkan_FOUND)
        message(WARNING "Vulkan SDK not found; building without the Vulkan renderer. "
                        "Install the Vulkan SDK (https://vulkan.lunarg.com) to enable it.")
        set(RR_ENABLE_VULKAN OFF CACHE BOOL "" FORCE)
    elseif(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
        find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE glslangValidator)
        if(NOT Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
            message(WARNING "glslangValidator not found; building without the Vulkan renderer.")
            set(RR_ENABLE_VULKAN OFF CACHE BOOL "" FORCE)
        endif()
    endif()
endif()
