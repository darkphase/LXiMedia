#include "test.h"
#include "platform/process.cpp"
#include <algorithm>
#include <cstring>
#include <thread>

static const struct process_test
{
    process_test()
        : run_process_test(this, "fstream::run_process", &process_test::run_process)
    {
    }

    struct test run_process_test;
    void run_process()
    {
        const std::string test = "hello_world";
        platform::process process([&](platform::process &process, std::ostream &out)
        {
            out << test << ' ' << std::flush;

            while (!process.term_pending())
                std::this_thread::yield();
        }, true);

        // Get data from the pipe.
        std::string result;
        process >> result;
        test_assert(process);
        test_assert(result == test);

        // Terminate process
        test_assert(process.joinable());
        process.send_term();

        // The pipe should be closed now.
        process >> result;
        test_assert(!process);

        process.join();
    }
} fork_test;
