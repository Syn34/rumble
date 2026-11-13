# rr_game: the reimplemented game. Sources are grouped by the original assembly
# they replace, so progress can be tracked against the original layout:
#   quantum_code/          - quantum.code.dll, the deterministic gameplay simulation
#   photon_deterministic/  - PhotonDeterministic.dll, deterministic maths/runtime
#   quantum_core/          - quantum.core.dll, simulation framework
add_subdirectory(photon_deterministic)
add_subdirectory(quantum_core)

add_library(rr_game
    quantum_code/fp_math_utility.cpp
    quantum_code/core_types.cpp
    quantum_code/components_1.cpp
    quantum_code/components_2.cpp
    quantum_code/components_3.cpp
    quantum_code/components_4.cpp
    quantum_code/component_pointers.cpp
)
add_library(rr::game ALIAS rr_game)
target_include_directories(rr_game PUBLIC .)
target_link_libraries(rr_game PUBLIC rr::photon_deterministic rr::quantum_core)
rr_set_warnings(rr_game)
