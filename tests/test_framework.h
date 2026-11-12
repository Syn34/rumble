// Minimal dependency-free test harness.
#pragma once

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace rr::test {

struct Case {
    const char* name;
    std::function<void()> body;
};

inline std::vector<Case>& Registry() {
    static std::vector<Case> cases;
    return cases;
}

inline int& FailureCount() {
    static int failures = 0;
    return failures;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> body) {
        Registry().push_back({name, std::move(body)});
    }
};

inline void ReportFailure(const char* file, int line, const char* expr) {
    std::printf("  FAILED %s:%d: %s\n", file, line, expr);
    ++FailureCount();
}

} // namespace rr::test

#define RR_TEST_CONCAT2(a, b) a##b
#define RR_TEST_CONCAT(a, b) RR_TEST_CONCAT2(a, b)

#define TEST_CASE(name)                                                                            \
    static void RR_TEST_CONCAT(rr_test_fn_, __LINE__)();                                           \
    static ::rr::test::Registrar RR_TEST_CONCAT(rr_test_reg_, __LINE__)(                           \
        name, &RR_TEST_CONCAT(rr_test_fn_, __LINE__));                                             \
    static void RR_TEST_CONCAT(rr_test_fn_, __LINE__)()

#define CHECK(expr)                                                                                \
    do {                                                                                           \
        if (!(expr)) ::rr::test::ReportFailure(__FILE__, __LINE__, #expr);                         \
    } while (0)
