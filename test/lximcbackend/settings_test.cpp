#include "test.h"
#include "../../src/lximcbackend/settings.cpp"
#include "../../src/lximcbackend/messageloop.cpp" // TODO: move to own test
#include "../../src/lximcbackend/vlc/instance.cpp" // TODO: move to own test
#include "../../src/lximcbackend/vlc/media.cpp" // TODO: move to own test
#include <cstdio>

static const struct settings_test
{
    settings_test()
        : uuid_test("settings_test::uuid", &settings_test::uuid)
    {
    }

    ~settings_test()
    {
        ::remove(filename().c_str());
    }

    struct test uuid_test;
    static void uuid()
    {
        ::remove(filename().c_str());

        class messageloop messageloop;

        std::string first_uuid;
        {
            class settings settings(messageloop);
            first_uuid = settings.uuid();
            test_assert(!first_uuid.empty());
        }
        {
            class settings settings(messageloop);
            const std::string second_uuid = settings.uuid();
            test_assert(second_uuid == first_uuid);
        }

        std::ifstream file(filename());
        std::string buffer;
        buffer.resize(4096);
        file.read(&buffer[0], buffer.size());
        buffer.resize(file.gcount());

        test_assert(buffer == ("[General]\nUUID=" + first_uuid + "\n\n"));
    }
} settings_test;

#if defined(__unix__)
static std::string filename()
{
    return "/tmp/settings_test.ini";
}
#endif
