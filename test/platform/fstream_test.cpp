#include "test.h"
#include "platform/fstream.cpp"
#include <algorithm>
#include <cstring>

static std::string filename();

static const struct fstream_test
{
    fstream_test()
        : loopback_test("fstream::loopback", &fstream_test::loopback)
    {
    }

    ~fstream_test()
    {
        ::remove(filename().c_str());
    }

    struct test loopback_test;
    static void loopback()
    {
        static const char data[] =
                "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor "
                "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
                "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
                "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
                "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
                "mollit anim id est laborum.";

        const std::string filename = ::filename();
        {
            ofstream out(filename);
            test_assert(out.is_open());
            test_assert(out.write(data, sizeof(data) - 1));
        }
        {
            ifstream in(filename);
            test_assert(in.is_open());

            std::string buffer;
            buffer.resize(sizeof(data) - 1);
            test_assert(in.read(&buffer[0], buffer.size()));
            test_assert(size_t(in.gcount()) == buffer.size());

            test_assert(memcmp(&buffer[0], data, buffer.size()) == 0);
        }
    }
} fstream_test;

#if defined(__unix__)
#include <unistd.h>

static std::string filename()
{
    return std::string("/tmp/") + std::to_string(getpid()) + ".file.txt";
}
#elif defined(WIN32)
#include <cstdlib>
#include <process.h>
#include "platform/path.h"

static std::string filename()
{
    const wchar_t * const temp = _wgetenv(L"TEMP");
    if (temp)
        return from_windows_path(std::wstring(temp) + L'\\' + std::to_wstring(_getpid()) + L".file.txt");

    throw std::runtime_error("failed to get TEMP directory");
}
#endif
