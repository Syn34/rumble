#include "test_framework.h"

int main() {
    for (const auto& c : rr::test::Registry()) {
        const int before = rr::test::FailureCount();
        c.body();
        std::printf("[%s] %s\n", rr::test::FailureCount() == before ? " ok " : "FAIL", c.name);
    }
    std::printf("%zu tests, %d failed checks\n", rr::test::Registry().size(),
                rr::test::FailureCount());
    return rr::test::FailureCount() == 0 ? 0 : 1;
}
