#include "test.h"
#include "platform/path.cpp"
#include <algorithm>

static const struct path_test
{
    path_test()
        : list_root_directories_test("path::list_root_directories", &path_test::list_root_directories),
          list_files_test("path::list_files", &path_test::list_files)
    {
    }

    struct test list_root_directories_test;
    static void list_root_directories()
    {
        const auto dirs = platform::list_root_directories();
#if defined(__unix__)
        test_assert(dirs.size() == 1);
        test_assert(dirs[0] == "/");
#endif
    }

    struct test list_files_test;
    static void list_files()
    {
        for (const auto &i : platform::list_root_directories())
        {
            const auto files = platform::list_files(i);
#if defined(__unix__)
            test_assert(!files.empty());

            test_assert(std::find(files.begin(), files.end(), "bin/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "boot/"   ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "dev/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "etc/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "lib/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "proc/"   ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "sbin/"   ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "sys/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "tmp/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "usr/"    ) == files.end());
            test_assert(std::find(files.begin(), files.end(), "var/"    ) == files.end());
#endif
        }
    }
} path_test;
