add_executable(RangerRumble
    main.cpp
    settings.cpp
    test_scene.cpp
)
# Offline matches (content startup, session runner, input, view).
target_sources(RangerRumble PRIVATE
    match/game_content.cpp
    match/math_tables.cpp
    match/match_setup.cpp
    match/match_runner.cpp
    match/match_input.cpp
    match/match_view.cpp
    match/match_app.cpp
)
target_link_libraries(RangerRumble PRIVATE rr::game rr::content rr::platform rr::render rr::assets rr::scene)
rr_set_warnings(RangerRumble)
