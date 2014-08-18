#ifndef TEST_H
#define TEST_H

#include <stdexcept>

struct test
{
    test(const char *name, void(* func)())
        : name(name), func(func), previous(current)
    {
        current = this;
    }

    static void assert_fail(const char *condition, const char *file, int line)
    {
        throw std::runtime_error(std::string(file) + ':' + std::to_string(line) + ' ' + condition);
    }

    const char * const name;
    void(* func)();
    struct test *previous;
    static struct test *current;
};

#define test_assert(expr) ((expr) ? (void)0 : test::assert_fail(#expr, __FILE__, __LINE__))

#endif
