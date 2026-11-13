# rr_scene: engine-side scenes (instantiated prefab/scene hierarchies), material
# mapping onto the PC shader programs, and the scene renderer.
add_library(rr_scene
    src/math.cpp
    src/resources.cpp
    src/material_mapping.cpp
    src/scene.cpp
    src/prefab_instancer.cpp
    src/content_resolver.cpp
    src/scene_renderer.cpp
    src/demo_scene.cpp
    src/camera_controller.cpp
)
add_library(rr::scene ALIAS rr_scene)
target_include_directories(rr_scene PUBLIC include)
target_link_libraries(rr_scene PUBLIC rr::assets rr::render)
rr_set_warnings(rr_scene)
