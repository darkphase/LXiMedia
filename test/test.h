#ifndef TEST_H
#define TEST_H

#include <functional>

struct test
{
    template <class _object>
    test(_object *parent, const char *name, void(_object::*func)())
        : name(name),
          func(std::bind(func, parent)),
          previous(current)
    {
        current = this;
    }

    static void assert_fail(const char *, const char *, unsigned int, const char *);

    const char * const name;
    std::function<void()> func;
    struct test *previous;
    static struct test *current;
};

#define test_assert(expr) ((expr) ? (void)0 : test::assert_fail(#expr, __FILE__, __LINE__, nullptr))

#endif
