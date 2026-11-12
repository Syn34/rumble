# PhotonDeterministic.dll - deterministic maths and simulation runtime (third-party).
add_library(rr_photon_deterministic
    bit_stream.cpp
    byte_utils.cpp
    fp.cpp
    rng_session.cpp
    fp_lut.cpp
    fp_math.cpp
    nullable_fp.cpp
    fp_vector2.cpp
    fp_vector3.cpp
    fp_quaternion.cpp
    fp_matrix2x2.cpp
    fp_matrix3x3.cpp
    nullable_fp_vector3.cpp
    dotnet_number_format.cpp
    fp_collision.cpp
)
add_library(rr::photon_deterministic ALIAS rr_photon_deterministic)
target_include_directories(rr_photon_deterministic PUBLIC ..)
rr_set_warnings(rr_photon_deterministic)
