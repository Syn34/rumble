add_executable(assetinfo assetinfo/main.cpp)
target_link_libraries(assetinfo PRIVATE rr::assets)
rr_set_warnings(assetinfo)

add_executable(contentindex contentindex/main.cpp)
target_link_libraries(contentindex PRIVATE rr::content)
# Uses the tests' derived maths tables (tests/math_tables.h) to rebuild triangle data.
target_include_directories(contentindex PRIVATE ${PROJECT_SOURCE_DIR}/tests)
rr_set_warnings(contentindex)

# Mesh/texture viewer and decoder checks (needs a renderer for viewing).
add_executable(meshview meshview/main.cpp)
target_link_libraries(meshview PRIVATE rr::assets rr::render rr::platform)
rr_set_warnings(meshview)

# Prefab/scene viewer on the engine scene renderer.
add_executable(sceneview sceneview/main.cpp)
target_link_libraries(sceneview PRIVATE rr::scene rr::assets rr::render rr::platform)
rr_set_warnings(sceneview)
