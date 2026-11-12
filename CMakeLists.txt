cmake_minimum_required(VERSION 3.21)

project(RangerRumble
    VERSION 0.1.0
    DESCRIPTION "Native C++ reimplementation of Ratchet & Clank: Ranger Rumble"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RR_BUILD_TESTS "Build unit tests" ON)
option(RR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(RR_ENABLE_DESKTOP "Build the SDL desktop platform backend (window, input)" ON)
option(RR_ENABLE_OPENGL "Build the OpenGL renderer" ON)
option(RR_ENABLE_VULKAN "Build the Vulkan renderer (needs the Vulkan SDK)" ON)
option(RR_FETCH_DEPENDENCIES "Download dependencies that aren't installed (SDL2)" ON)

if(NOT RR_ENABLE_DESKTOP)
    # Without a window there is nothing for a renderer to draw into.
    set(RR_ENABLE_OPENGL OFF)
    set(RR_ENABLE_VULKAN OFF)
endif()

include(cmake/Warnings.cmake)
include(cmake/Dependencies.cmake)

message(STATUS "Ranger Rumble: desktop=${RR_ENABLE_DESKTOP} opengl=${RR_ENABLE_OPENGL} "
               "vulkan=${RR_ENABLE_VULKAN}")

add_subdirectory(src/platform)
add_subdirectory(src/render)
add_subdirectory(src/game)
add_subdirectory(src/app)

if(RR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
