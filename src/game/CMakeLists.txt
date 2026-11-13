# rr_game: the reimplemented game. Sources are grouped by the original assembly
# they replace, so progress can be tracked against the original layout:
#   quantum_code/          - quantum.code.dll, the deterministic gameplay simulation
#   photon_deterministic/  - PhotonDeterministic.dll, deterministic maths/runtime
#   quantum_core/          - quantum.core.dll, simulation framework
add_subdirectory(photon_deterministic)
add_subdirectory(quantum_core)
add_subdirectory(content)

add_library(rr_game
    quantum_code/fp_math_utility.cpp
    quantum_code/core_types.cpp
    quantum_code/input.cpp
    quantum_code/components_1.cpp
    quantum_code/components_2.cpp
    quantum_code/components_3.cpp
    quantum_code/components_4.cpp
    quantum_code/component_pointers.cpp
    quantum_code/system_base.cpp
    quantum_code/runtime_config.cpp
    quantum_code/input_provider.cpp
    quantum_code/event_types.cpp
    quantum_code/frame_events.cpp
    quantum_code/frame_signals.cpp
    quantum_code/frame_components.cpp
    quantum_code/frame.cpp
)
add_library(rr::game ALIAS rr_game)
target_include_directories(rr_game PUBLIC .)
target_link_libraries(rr_game PUBLIC rr::photon_deterministic rr::quantum_core)
rr_set_warnings(rr_game)

# Gameplay asset classes (quantum.code.dll assets).
target_sources(rr_game PRIVATE
    quantum_code/assets/asset_value_types.cpp
    quantum_code/assets/bot_sdk_assets.cpp
    quantum_code/assets/gameplay_specs.cpp
    quantum_code/assets/actor_specs.cpp
    quantum_code/assets/game_mode_specs.cpp
    quantum_code/assets/level_assets.cpp
)
