#include "test.h"
#include "platform/process.cpp"

#if !defined(PROCESS_USES_THREAD)
#define PROCESS_USES_THREAD
#undef PLATFORM_PROCESS_H
#define process process_thread
#include "platform/process.cpp"
#undef process_thread
#undef PROCESS_USES_THREAD
#endif

#include <algorithm>
#include <cstring>
#include <thread>

static const struct process_test
{
    process_test()
        : run_process_test(this, "platform::process::run_process", &process_test::run_process<platform::process>)
#if !defined(PROCESS_USES_THREAD)
        , run_process_thread_test(this, "platform::process::run_process_thread", &process_test::run_process<platform::process_thread>)
#endif
    {
    }

    struct test run_process_test;
#if !defined(PROCESS_USES_THREAD)
    struct test run_process_thread_test;
#endif
    template <class _Process>
    void run_process()
    {
        const std::string test = "hello_world";
        _Process process([&](_Process &process, std::ostream &out)
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
} process_test;
