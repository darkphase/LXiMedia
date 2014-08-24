#include "test.h"
#include "lximcbackend/string.cpp"

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
        : starts_with_test("string::starts_with", &string_test::starts_with),
          ends_with_test("string::ends_with", &string_test::ends_with),
          to_upper_test("string::to_upper", &string_test::to_upper),
          to_lower_test("string::to_lower", &string_test::to_lower),
          to_base64_test("string::to_base64", &string_test::to_base64),
          from_base64_test("string::from_base64", &string_test::from_base64),
          to_percent_test("string::to_percent", &string_test::to_percent),
          from_percent_test("string::from_percent", &string_test::from_percent),
          escape_xml_test("string::escape_xml", &string_test::escape_xml)
    {
    }

    struct test starts_with_test;
    static void starts_with()
    {
        test_assert(::starts_with("hello world", "hello"));
        test_assert(!::starts_with("hello", "hello world"));
    }

    struct test ends_with_test;
    static void ends_with()
    {
        test_assert(::ends_with("hello world", "world"));
        test_assert(!::ends_with("world", "hello world"));
    }

    struct test to_upper_test;
    static void to_upper()
    {
        test_assert(::to_upper("hello world") == "HELLO WORLD");
    }

    struct test to_lower_test;
    static void to_lower()
    {
        test_assert(::to_lower("HELLO WORLD") == "hello world");
    }

    struct test to_base64_test;
    static void to_base64()
    {
        const std::string result = ::to_base64(base64_src, true);
        test_assert(result == base64_ref);
    }

    struct test from_base64_test;
    static void from_base64()
    {
        const std::string result = ::from_base64(base64_ref);
        test_assert(result == base64_src);
    }

    struct test to_percent_test;
    static void to_percent()
    {
        const std::string result = ::to_percent(percent_src);
        test_assert(result == percent_ref);
    }

    struct test from_percent_test;
    static void from_percent()
    {
        const std::string result = ::from_percent(percent_ref);
        test_assert(result == percent_src);
    }

    struct test escape_xml_test;
    static void escape_xml()
    {
        const std::string result = ::escape_xml(escape_xml_src);
        test_assert(result == escape_xml_ref);
    }
} string_test;
