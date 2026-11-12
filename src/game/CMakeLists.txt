# rr_game: the reimplemented game. Sources are grouped by the original assembly
# they replace, so progress can be tracked against the original layout:
#   quantum_code/          - quantum.code.dll, the deterministic gameplay simulation
#   photon_deterministic/  - PhotonDeterministic.dll, deterministic maths/runtime
add_subdirectory(photon_deterministic)

add_library(rr_game
    quantum_code/fp_math_utility.cpp
)
add_library(rr::game ALIAS rr_game)
target_include_directories(rr_game PUBLIC .)
target_link_libraries(rr_game PUBLIC rr::photon_deterministic)
rr_set_warnings(rr_game)
