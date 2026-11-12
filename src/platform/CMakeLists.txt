# rr_platform: OS abstraction (window, input, time, files) with swappable backends.
add_library(rr_platform
    src/null/null_platform.cpp
)
add_library(rr::platform ALIAS rr_platform)
target_include_directories(rr_platform PUBLIC include)
rr_set_warnings(rr_platform)
