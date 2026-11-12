add_executable(assetinfo assetinfo/main.cpp)
target_link_libraries(assetinfo PRIVATE rr::assets)
rr_set_warnings(assetinfo)
