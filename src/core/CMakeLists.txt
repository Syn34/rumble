# rr_core: engine-independent foundation (deterministic maths, utilities).
# Replaces the third-party deterministic maths library the game shipped with.
add_library(rr_core
    src/fp.cpp
)
add_library(rr::core ALIAS rr_core)
target_include_directories(rr_core PUBLIC include)
rr_set_warnings(rr_core)
