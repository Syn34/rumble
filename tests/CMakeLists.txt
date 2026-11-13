add_executable(rr_tests
    test_main.cpp
    test_fp.cpp
    test_fp_lut.cpp
    test_fp_math.cpp
    test_fp_math_utility.cpp
    test_serialized_file.cpp
    test_asset_guid.cpp
    test_bit_stream.cpp
    test_byte_utils.cpp
    test_rng_session.cpp
    test_nullable_fp.cpp
    test_fp_vectors.cpp
    test_fp_quaternion_matrix.cpp
    test_quantum_heap.cpp
    test_quantum_collections.cpp
    test_deterministic_session.cpp
    test_fp_collision.cpp
    test_quantum_collision.cpp
    test_navigation.cpp
)
target_link_libraries(rr_tests PRIVATE rr::game rr::assets)
rr_set_warnings(rr_tests)
add_test(NAME rr_tests COMMAND rr_tests)
