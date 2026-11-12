# PhotonDeterministic.dll - deterministic maths and simulation runtime (third-party).
add_library(rr_photon_deterministic
    fp.cpp
)
add_library(rr::photon_deterministic ALIAS rr_photon_deterministic)
target_include_directories(rr_photon_deterministic PUBLIC ..)
rr_set_warnings(rr_photon_deterministic)
