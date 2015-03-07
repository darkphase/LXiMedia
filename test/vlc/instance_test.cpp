#include "test.h"
#include "vlc/instance.cpp"
#include <cstdio>

namespace vlc {

static const struct instance_test
{
    instance_test()
        : compare_version_test(this, "vlc::instance::compare_version", &instance_test::compare_version)
    {
    }

    struct test compare_version_test;
    void compare_version()
    {
        test_assert(::compare_version(instance::version(), libvlc_get_version()) == 0);
    }
} instance_test;

} // End of namespace
