#include "test.h"
#include "resources/resource_file.cpp"
#include "resources/resources.cpp"
#include "platform/fstream.h"

namespace resources {

static const struct resource_file_test
{
    std::string filename;

    resource_file_test()
        : a440hz_flac_test(this, "resources::resource_file::a440hz_flac", &resource_file_test::a440hz_flac),
          pm5544_png_test(this, "resources::resource_file::pm5544_png", &resource_file_test::pm5544_png),
          pm5644_png_test(this, "resources::resource_file::pm5644_png", &resource_file_test::pm5644_png)
    {
    }

    ~resource_file_test()
    {
        if (!filename.empty())
            ::remove(filename.c_str());
    }

    struct test a440hz_flac_test;
    void a440hz_flac()
    {
        {
            resource_file file(resources::a440hz_flac, "flac");
            filename = file;

            platform::ifstream stream(filename, std::ios_base::binary);
            test_assert(stream.is_open());
        }

        platform::ifstream stream(filename, std::ios_base::binary);
        test_assert(!stream.is_open());

        filename.clear();
    }

    struct test pm5544_png_test;
    void pm5544_png()
    {
        {
            resource_file file(resources::pm5544_png, "png");
            filename = file;

            platform::ifstream stream(filename, std::ios_base::binary);
            test_assert(stream.is_open());
        }

        platform::ifstream stream(filename, std::ios_base::binary);
        test_assert(!stream.is_open());

        filename.clear();
    }

    struct test pm5644_png_test;
    void pm5644_png()
    {
        {
            resource_file file(resources::pm5644_png, "png");
            filename = file;

            platform::ifstream stream(filename, std::ios_base::binary);
            test_assert(stream.is_open());
        }

        platform::ifstream stream(filename, std::ios_base::binary);
        test_assert(!stream.is_open());

        filename.clear();
    }
} media_cache_test;

} // End of namespace
