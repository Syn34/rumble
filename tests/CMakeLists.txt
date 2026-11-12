add_executable(rr_tests
    test_main.cpp
    test_fp.cpp
    test_fp_math_utility.cpp
    test_fp_vectors.cpp
    test_fp_quaternion_matrix.cpp
)
target_link_libraries(rr_tests PRIVATE rr::game)
rr_set_warnings(rr_tests)
add_test(NAME rr_tests COMMAND rr_tests)
