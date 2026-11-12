# PhotonDeterministic.dll - deterministic maths and simulation runtime (third-party).
add_library(rr_photon_deterministic
    bit_stream.cpp
    byte_utils.cpp
    fp.cpp
    system_allocator.cpp
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
    native.cpp
    log.cpp
    deterministic_command.cpp
    deterministic_frame.cpp
    deterministic_frame_ring_buffer.cpp
    deterministic_session_config.cpp
    deterministic_platform_info.cpp
    deterministic_tick_input.cpp
    deterministic_simulator_utils.cpp
    deterministic_simulator.cpp
    deterministic_session.cpp
    deterministic_network_local.cpp
    deterministic_network.cpp
    local_input_provider.cpp
    fp_collision.cpp
)
add_library(rr::photon_deterministic ALIAS rr_photon_deterministic)
target_include_directories(rr_photon_deterministic PUBLIC ..)
rr_set_warnings(rr_photon_deterministic)
