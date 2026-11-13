add_executable(assetinfo assetinfo/main.cpp)
target_link_libraries(assetinfo PRIVATE rr::assets)
rr_set_warnings(assetinfo)

add_executable(contentindex contentindex/main.cpp)
target_link_libraries(contentindex PRIVATE rr::content)
rr_set_warnings(contentindex)
