add_executable(RangerRumble
    main.cpp
    settings.cpp
    test_scene.cpp
)
target_link_libraries(RangerRumble PRIVATE rr::game rr::platform rr::render)
rr_set_warnings(RangerRumble)
