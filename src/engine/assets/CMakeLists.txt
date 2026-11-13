# rr_assets: readers for the original game's content files (engine layer).
add_library(rr_assets
    src/serialized_file.cpp
    src/bundle_file.cpp
    src/lz4.cpp
    src/common_strings.cpp
    src/type_tree_value.cpp
    src/content_library.cpp
)
add_library(rr::assets ALIAS rr_assets)
target_include_directories(rr_assets PUBLIC include)
rr_set_warnings(rr_assets)

# Prefab (game object hierarchy) reader.
target_sources(rr_assets PRIVATE src/prefab.cpp)
