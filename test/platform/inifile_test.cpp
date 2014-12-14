#include "test.h"
#include "platform/inifile.cpp"
#include "platform/path.h"
#include <cstdio>

static const struct inifile_test
{
    const std::string filename;

    inifile_test()
        : filename(platform::temp_file_path("ini")),
          loopback_test(this, "inifile::loopback", &inifile_test::loopback)
    {
    }

    ~inifile_test()
    {
        ::remove(filename.c_str());
    }

    struct test loopback_test;
    void loopback()
    {
        static const char data[] =
                "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor\n"
                "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud\n"
                "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n"
                "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\n"
                "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt\n"
                "mollit anim id est laborum.";

        {
            platform::inifile out(filename);
            out.open_section("section[1]\\").write("string=", data);
            out.open_section("section[2]\\").write("int=", 1234);
            out.open_section("section[2]\\").write("long=", 12345678L);
            out.open_section("section[2]\\").write("long long=", 123456789101112LL);
        }
        {
            const platform::inifile in(filename);
            test_assert(in.open_section("section[1]\\").names().size() == 1);
            test_assert(in.open_section("section[1]\\").read("string=") == data);
            test_assert(in.open_section("section[2]\\").names().size() == 3);
            test_assert(in.open_section("section[2]\\").read("int=", 0) == 1234);
            test_assert(in.open_section("section[2]\\").read("long=", 0L) == 12345678L);
            test_assert(in.open_section("section[2]\\").read("long long=", 0LL) == 123456789101112LL);
        }
    }
} inifile_test;
