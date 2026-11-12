# rr_platform: OS abstraction (window, input, time, files) with swappable backends.
add_library(rr_platform
    src/common/common.cpp
    src/null/null_platform.cpp
)
add_library(rr::platform ALIAS rr_platform)
target_include_directories(rr_platform PUBLIC include)
rr_set_warnings(rr_platform)

if(RR_ENABLE_DESKTOP)
    target_sources(rr_platform PRIVATE src/sdl/sdl_platform.cpp)
    target_link_libraries(rr_platform PRIVATE rr::sdl2)
    target_compile_definitions(rr_platform PUBLIC RR_HAS_DESKTOP=1)
else()
    target_sources(rr_platform PRIVATE src/sdl/no_desktop_platform.cpp)
endif()
