#include "test.h"
#include "platform/process.cpp"
#define register_function(F) register_function(#F, F)

#include <algorithm>
#include <cstring>
#include <thread>

static const struct process_test
{
    process_test()
        : child_process_name(platform::process::register_function(&process_test::child_process)),
          run_process_test(this, "platform::process::run_process", &process_test::run_process)
    {
    }

    platform::process::function_handle child_process_name;
    static int child_process(platform::process &process)
    {
        unsigned value_ofs(-1);
        process >> value_ofs;

        std::string test;
        process >> test;

        auto &value = process.get_shared<int>(value_ofs);
        value = 1234;

        process << test << ' ' << std::flush;

        int exit_code = -1;
        process >> exit_code;

        while (process)
            std::this_thread::yield();

        value = 5678;

        return exit_code;
    }

    struct test run_process_test;
    void run_process()
    {
        platform::process process(
                    child_process_name,
                    platform::process::priority::low);

        const unsigned value_ofs = process.alloc_shared<int>();
        test_assert(value_ofs != unsigned(-1));
        process << value_ofs << ' ' << std::flush;

        // Send data to the process.
        const std::string test = "hello_world";
        process << test << ' ' << std::flush;

        // Get data from the pipe.
        std::string result;
        process >> result;
        test_assert(process);
        test_assert(result == test);

        // The shared memory value should be updated now.
        test_assert(process.get_shared<int>(value_ofs) == 1234);

        // Send the desired exit code.
        process << 123 << ' ' << std::flush;

        // Terminate process
        test_assert(process.joinable());
        process.send_term();

        // The pipe should be closed now.
        process >> result;
        test_assert(!process);

        test_assert(process.join() == 123);

        // The shared memory value should be updated again.
        test_assert(process.get_shared<int>(value_ofs) == 5678);
    }
} process_test;
