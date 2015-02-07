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
        int major = 0, minor = 0, patch = 0;
        test_assert(sscanf(libvlc_get_version(), "%d.%d.%d", &major, &minor, &patch) == 3);

        test_assert(instance::compare_version(major, minor, patch) == 0);
        test_assert(instance::compare_version(major, minor) == 0);
        test_assert(instance::compare_version(major) == 0);

        test_assert(instance::compare_version(major, minor, patch + 1) < 0);
        test_assert(instance::compare_version(major, minor + 1) < 0);
        test_assert(instance::compare_version(major + 1) < 0);

        if (patch > 0) test_assert(instance::compare_version(major, minor, patch - 1) > 0);
        if (minor > 0) test_assert(instance::compare_version(major, minor - 1) > 0);
        if (major > 0) test_assert(instance::compare_version(major - 1) > 0);
    }
} instance_test;

} // End of namespace
