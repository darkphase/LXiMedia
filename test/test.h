#ifndef TEST_H
#define TEST_H

struct test
{
    test(const char *name, void(* func)())
        : name(name), func(func), previous(current)
    {
        current = this;
    }

    static void assert_fail(const char *, const char *, unsigned int, const char *);

    const char * const name;
    void(* func)();
    struct test *previous;
    static struct test *current;
};

#define test_assert(expr) ((expr) ? (void)0 : test::assert_fail(#expr, __FILE__, __LINE__, nullptr))

#endif
