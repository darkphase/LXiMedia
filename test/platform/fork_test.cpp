#include "test.h"
#include "platform/fork.cpp"
#include <algorithm>
#include <cstring>

static const struct fork_test
{
    fork_test()
        : run_forked_test("fstream::run_forked", &fork_test::run_forked)
    {
    }

    ~fork_test()
    {
    }

    struct test run_forked_test;
    static void run_forked()
    {
        const std::string test = "hello_world";
        const auto result = platform::run_forked([&test] { return test; }, false);

        test_assert(result == test);
    }
} fork_test;
