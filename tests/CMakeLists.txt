add_executable(rr_tests
    test_main.cpp
    test_fp.cpp
    test_fp_lut.cpp
    test_fp_math.cpp
    test_fp_math_utility.cpp
    test_nullable_fp.cpp
)
target_link_libraries(rr_tests PRIVATE rr::game)
rr_set_warnings(rr_tests)
add_test(NAME rr_tests COMMAND rr_tests)
