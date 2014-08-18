#include "test.h"
#include "lximcbackend/messageloop.cpp"
#include <thread>

static const struct messageloop_test
{
    messageloop_test()
        : post_test("messageloop::post", &messageloop_test::post),
          send_test("messageloop::send", &messageloop_test::send),
          process_events_test("messageloop::process_events", &messageloop_test::process_events),
          timer_test("messageloop::timer", &messageloop_test::timer)
    {
    }

    struct test post_test;
    static void post()
    {
        class messageloop messageloop;
        messageloop.post([&messageloop] { messageloop.stop(123); });
        test_assert(messageloop.run() == 123);
    }

    struct test send_test;
    static void send()
    {
        class messageloop messageloop;

        int exitcode = -1;
        std::thread looper([&messageloop, &exitcode] { exitcode = messageloop.run(); });

        int test = 0;
        messageloop.send([&test] { test = 456; });
        test_assert(test == 456);

        messageloop.stop(123);
        looper.join();
        test_assert(exitcode == 123);
    }

    struct test process_events_test;
    static void process_events()
    {
        class messageloop messageloop;

        int test = 0;
        messageloop.post([&test] { test = 456; });
        test_assert(test == 0);
        messageloop.process_events(std::chrono::seconds(0));
        test_assert(test == 456);
    }

    struct test timer_test;
    static void timer()
    {
        class messageloop messageloop;

        int test = 0;
        class timer timer_1(messageloop, [&test] { test++; });
        class timer timer_2(messageloop, [&test, &timer_1] { test++; timer_1.stop(); });
        class timer timer_3(messageloop, [&messageloop] { messageloop.stop(123); });

        timer_1.start(std::chrono::milliseconds(16));
        timer_2.start(std::chrono::milliseconds(40), true);
        timer_3.start(std::chrono::milliseconds(96), true);

        std::chrono::steady_clock clock;
        const auto start = clock.now();
        test_assert(messageloop.run() == 123);
        const auto stop = clock.now();

        test_assert(test == 3);

        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        test_assert(duration.count() > 80);
        test_assert(duration.count() < 112);
    }
} messageloop_test;
