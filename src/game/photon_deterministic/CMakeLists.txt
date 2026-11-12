# PhotonDeterministic.dll - deterministic maths and simulation runtime (third-party).
add_library(rr_photon_deterministic
    fp.cpp
    fp_vector2.cpp
    fp_vector3.cpp
    fp_quaternion.cpp
    fp_matrix2x2.cpp
    fp_matrix3x3.cpp
    nullable_fp_vector3.cpp
    dotnet_number_format.cpp
    # Temporary link stubs until the FPMath work lands; see fp_math_pending.h.
    fp_math_pending.cpp
)
add_library(rr::photon_deterministic ALIAS rr_photon_deterministic)
target_include_directories(rr_photon_deterministic PUBLIC ..)
rr_set_warnings(rr_photon_deterministic)
