# rr_content: connects the game's content bundles to the simulation's asset system
# (PC-side glue; new code).
add_library(rr_content
    quantum_content_provider.cpp
    builtin_asset_factories.cpp
    quantum_code_factories.cpp
)
add_library(rr::content ALIAS rr_content)
target_include_directories(rr_content PUBLIC ..)
target_link_libraries(rr_content PUBLIC rr::assets rr::quantum_core rr::game)
rr_set_warnings(rr_content)

# Component prototypes on prefabs and in maps.
target_sources(rr_content PRIVATE
    prototype_readers.cpp
    framework_prototype_readers.cpp
    unity_entity_prototype.cpp
    entity_prototype_loader.cpp
)
