#include "test.h"
#include "platform/string.cpp"

static const char base64_src[] =
        "Man is distinguished, not only by his reason, but by this singular passion from "
        "other animals, which is a lust of the mind, that by a perseverance of delight "
        "in the continued and indefatigable generation of knowledge, exceeds the short "
        "vehemence of any carnal pleasure.";

static const char base64_ref[] =
        "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz"
        "IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg"
        "dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu"
        "dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo"
        "ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

static const char percent_src[] = "Dogs, Cats & Mice";
static const char percent_ref[] = "Dogs%2C%20Cats%20%26%20Mice";

static const char escape_xml_src[] = "\"Hello <&> World\"";
static const char escape_xml_ref[] = "&quot;Hello &lt;&amp;> World&quot;";

static const struct string_test
{
    string_test()
        : starts_with_test(this, "string::starts_with", &string_test::starts_with),
          ends_with_test(this, "string::ends_with", &string_test::ends_with),
          to_upper_test(this, "string::to_upper", &string_test::to_upper),
          to_lower_test(this, "string::to_lower", &string_test::to_lower),
          to_base64_test(this, "string::to_base64", &string_test::to_base64),
          from_base64_test(this, "string::from_base64", &string_test::from_base64),
          to_percent_test(this, "string::to_percent", &string_test::to_percent),
          from_percent_test(this, "string::from_percent", &string_test::from_percent),
          escape_xml_test(this, "string::escape_xml", &string_test::escape_xml),
          compare_version_test(this, "string::compare_version", &string_test::compare_version)
    {
    }

    struct test starts_with_test;
    void starts_with()
    {
        test_assert(::starts_with("hello world", "hello"));
        test_assert(!::starts_with("hello", "hello world"));
    }

    struct test ends_with_test;
    void ends_with()
    {
        test_assert(::ends_with("hello world", "world"));
        test_assert(!::ends_with("world", "hello world"));
    }

    struct test to_upper_test;
    void to_upper()
    {
        test_assert(::to_upper("hello world") == "HELLO WORLD");
    }

    struct test to_lower_test;
    void to_lower()
    {
        test_assert(::to_lower("HELLO WORLD") == "hello world");
    }

    struct test to_base64_test;
    void to_base64()
    {
        const std::string result = ::to_base64(base64_src, true);
        test_assert(result == base64_ref);
    }

    struct test from_base64_test;
    void from_base64()
    {
        const std::string result = ::from_base64(base64_ref);
        test_assert(result == base64_src);
    }

    struct test to_percent_test;
    void to_percent()
    {
        const std::string result = ::to_percent(percent_src);
        test_assert(result == percent_ref);
    }

    struct test from_percent_test;
    void from_percent()
    {
        const std::string result = ::from_percent(percent_ref);
        test_assert(result == percent_src);
    }

    struct test escape_xml_test;
    void escape_xml()
    {
        const std::string result = ::escape_xml(escape_xml_src);
        test_assert(result == escape_xml_ref);
    }

    struct test compare_version_test;
    void compare_version()
    {
        test_assert(::compare_version("1.2.3", "1.2.3") == 0);
        test_assert(::compare_version("1.2.3", "1.2") == 0);
        test_assert(::compare_version("1.2.3", "1") == 0);

        test_assert(::compare_version("1.2.3", "1.2.4") < 0);
        test_assert(::compare_version("1.2.3", "1.3") < 0);
        test_assert(::compare_version("1.2.3", "2") < 0);

        test_assert(::compare_version("1.2.3", "1.2.2") > 0);
        test_assert(::compare_version("1.2.3", "1.1") > 0);
        test_assert(::compare_version("1.2.3", "0") > 0);
    }
} string_test;
