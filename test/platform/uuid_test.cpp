#include "test.h"
#include "platform/uuid.cpp"

static const struct uuid_test
{
    uuid_test()
        : generate_test("uuid::generate", &uuid_test::generate),
          string_test("uuid::string", &uuid_test::string)
    {
    }

    struct test generate_test;
    static void generate()
    {
        const auto uuid1 = platform::uuid::generate();
        test_assert(!uuid1.is_null());

        const auto uuid2 = platform::uuid::generate();
        test_assert(!uuid2.is_null());

        test_assert(uuid1 != uuid2);
        test_assert((uuid1 < uuid2) || (uuid2 < uuid1));
    }

    struct test string_test;
    static void string()
    {
        const auto uuid1 = platform::uuid::generate();
        test_assert(!uuid1.is_null());

        const auto uuid2 = platform::uuid(std::string(uuid1));
        test_assert(!uuid2.is_null());

        test_assert(uuid1 == uuid2);
        test_assert(!(uuid1 < uuid2) && !(uuid2 < uuid1));
    }
} uuid_test;
