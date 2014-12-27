#include "test.h"
#include "platform/fstream.cpp"
#include "platform/path.h"
#include <algorithm>
#include <cstring>

static const struct fstream_test
{
    const std::string filename;

    fstream_test()
        : filename(platform::temp_file_path("txt")),
          loopback_test(this, "platform::fstream::loopback", &fstream_test::loopback)
    {
    }

    ~fstream_test()
    {
        ::remove(filename.c_str());
    }

    struct test loopback_test;
    void loopback()
    {
        static const char data[] =
                "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor "
                "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
                "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
                "mollit anim id est laborum.";

        {
            platform::ofstream out(filename);
            test_assert(out.is_open());
            test_assert(out.write(data, sizeof(data) - 1));
        }
        {
            platform::ifstream in(filename);
            test_assert(in.is_open());

            std::string buffer;
            buffer.resize(sizeof(data) - 1);
            test_assert(in.read(&buffer[0], buffer.size()));
            test_assert(size_t(in.gcount()) == buffer.size());

            test_assert(memcmp(&buffer[0], data, buffer.size()) == 0);
        }
    }
} fstream_test;
