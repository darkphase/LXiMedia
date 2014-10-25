#include "test.h"
#include "platform/sha1.cpp"
#include <cstring>

static const char sha1_src[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
static const unsigned char sha1_ref[]
{
    0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
    0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1
};

static const struct sha1_test
{
    sha1_test()
        : calc_test("sha1::calc", &sha1_test::calc)
    {
    }

    struct test calc_test;
    static void calc()
    {
        unsigned char hash[20];
        sha1::calc(sha1_src, sizeof(sha1_src) - 1, hash);
        test_assert(memcmp(hash, sha1_ref, sizeof(hash)) == 0);
    }
} sha1_test;
