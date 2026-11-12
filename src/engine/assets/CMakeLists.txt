# rr_assets: readers for the original game's content files (engine layer).
add_library(rr_assets src/serialized_file.cpp)
add_library(rr::assets ALIAS rr_assets)
target_include_directories(rr_assets PUBLIC include)
rr_set_warnings(rr_assets)
