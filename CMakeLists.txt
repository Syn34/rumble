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

include(cmake/Warnings.cmake)

add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/game)
add_subdirectory(src/app)

if(RR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
