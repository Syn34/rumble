add_executable(RangerRumble main.cpp)
target_link_libraries(RangerRumble PRIVATE rr::game rr::platform)
rr_set_warnings(RangerRumble)
