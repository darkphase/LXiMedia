/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "string.h"
#include <algorithm>
#include <climits>
#include <cstring>
#include <iomanip>
#include <sstream>

bool starts_with(const std::string &text, const std::string &find)
{
    if (text.length() >= find.length())
        return strncmp(&text[0], &find[0], find.length()) == 0;

    return false;
}

bool ends_with(const std::string &text, const std::string &find)
{
    if (text.length() >= find.length())
        return strncmp(&text[text.length() - find.length()], &find[0], find.length()) == 0;

    return false;
}

std::string to_upper(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string to_lower(const std::string &input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

static bool is_number(char32_t c)
{
    return (c >= '0') && (c <= '9');
}

static unsigned read_number(const std::u32string &s, size_t &p)
{
    unsigned n = 0;
    while (p < s.length())
    {
        const char32_t c = s[p];
        if (is_number(c))
        {
            const unsigned v = unsigned(c - '0');
            if (n <= (UINT_MAX - v) / 10)
                n = (n * 10) + v;
            else
                n = UINT_MAX;

            p++;
        }
        else
            break;
    }

    return n;
}

bool alphanum_less::operator()(const std::string &a_u8, const std::string &b_u8) const
{
    const auto a = to_utf32(a_u8);
    const auto b = to_utf32(b_u8);

    for (size_t ap = 0, bp = 0, m = 0; (ap < a.length()) && (bp < b.length()); )
        if (m == 0) // Processing strings
        {
            const char32_t ac = a[ap], bc = b[bp];
            if (is_number(ac) && is_number(bc))
                m = 1;
            else if (ac < bc)
                return true;
            else if (ac > bc)
                return false;
            else
            {
                ap++;
                bp++;
            }
        }
        else // Processing numbers
        {
            const unsigned an = read_number(a, ap);
            const unsigned bn = read_number(b, bp);
            if (an < bn)
                return true;
            else if (an > bn)
                return false;
            else
                m = 0;
        }

    return false;
}

std::string from_base64(const std::string &input)
{
    static const uint8_t table[256] =
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  62,0x00,0x00,0x00,  63,
        52,  53,  54,  55,  56,  57,  58,  59,  60,  61,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
        15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,0x00,0x00,0x00,0x00,0x00,
        0x00,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
        41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,0x00,0x00,0x00,0x00,0x00
        // Remainder will be initialized to zero.
    };

    std::string result;
    result.reserve(input.length() * 3 / 4);

    for (size_t i = 0; i < input.length(); i += 4)
    {
        const bool has[] =
        {
            input[i  ] != '=' ,
            (i+1 < input.length()) && (input[i+1] != '='),
            (i+2 < input.length()) && (input[i+2] != '='),
            (i+3 < input.length()) && (input[i+3] != '='),
        };

        uint32_t triple = 0;
        for (int j = 0; j < 4; j++)
            triple |= uint32_t(has[j] ? table[uint8_t(input[i + j])] : 0) << (18 - (j * 6));

        for (int j = 0; (j < 3) && has[j + 1]; j++)
            result.push_back(char(triple >> (16 - (j * 8)) & 0xFF));
    }

    return result;
}

std::string to_base64(const std::string &input, bool pad)
{
    static const char table[64] =
    {
        'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
        'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
        'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
    };

    std::string result;
    result.reserve(4 * ((input.length() + 2) / 3));

    for (size_t i = 0; i < input.length(); i += 3)
    {
        const bool has[] = { true, true, i + 1 < input.length(), i + 2 < input.length() };

        uint32_t triple = 0;
        for (int j = 0; j < 3; j++)
            triple |= uint32_t(has[j+1] ? uint8_t(input[i + j]) : 0) << (16 - (j * 8));

        for (int j = 0; (j < 4) && has[j]; j++)
            result.push_back(table[(triple >> (18 - (j * 6))) & 0x3F]);
    }

    if (pad)
    {
        static const int padding[] = { 0, 2, 1 };
        for (int i = 0, n = padding[input.length() % 3]; i < n; i++)
            result.push_back('=');
    }

    return result;
}

std::string from_percent(const std::string &input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0, n = input.length(); i < n; i++)
    {
        const char c = input[i];
        if (c != '%')
        {
            result.push_back((c != '+') ? c : ' ');
        }
        else if (i + 2 < n)
        {
            const char h[3] = { input[i + 1], input[i + 2], 0 };
            result.push_back(char(::strtol(h, nullptr, 16)));
            i += 2;
        }
        else
            break;
    }

    return result;
}

std::string to_percent(const std::string &input)
{
    std::ostringstream result;
    result << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0, n = input.length(); i < n; i++)
    {
        const char c = input[i];
        if (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) ||
            ((c >= '0') && (c <= '9')) || (c == '_'))
        {
            result << c;
        }
        else
            result << '%' << std::setw(2) << (unsigned(c) & 0xFF);
    }

    return result.str();
}

std::string escape_xml(const std::string &input)
{
    std::ostringstream result;

    for (auto i : input)
    {
        if      (i == '&') result << "&amp;";
        else if (i == '"') result << "&quot;";
        else if (i == '<') result << "&lt;";
        else               result << i;
    }

    return result.str();
}

bool is_utf8(const std::string &src)
{
    const auto &input =
            reinterpret_cast<const std::basic_string<uint8_t> &>(src);

    for (size_t i = 0, n = input.size(); (i + 3) < n; )
    {
        // ASCII.
        if ((input[i] == 0x09) || (input[i] == 0x0a) || (input[i] == 0x0d) ||
            ((0x20 <= input[i]) && (input[i] <= 0x7e)))
        {
            i++;
        }
        // Non-overlong 2-byte.
        else if (((0xc2 <= input[i  ]) && (input[i  ] <= 0xdf)) &&
                 ((0x80 <= input[i+1]) && (input[i+1] <= 0xbf)))
        {
            i += 2;
        }
        // Excluding overlongs.
        else if ((input[i] == 0xe0) &&
                 ((0xa0 <= input[i+1]) && (input[i+1] <= 0xbf)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)))
        {
            i += 3;
        }
        // Straight 3-byte.
        else if ((((0xe1 <= input[i]) && (input[i] <= 0xec)) ||
                  (input[i] == 0xee) || (input[i] == 0xef)) &&
                 ((0x80 <= input[i+1]) && (input[i+1] <= 0xbf)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)))
        {
            i += 3;
        }
        // Excluding surrogates.
        else if ((input[i] == 0xed) &&
                 ((0x80 <= input[i+1]) && (input[i+1] <= 0x9f)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)))
        {
            i += 3;
        }
        // Planes 1-3.
        else if ((input[i] == 0xf0) &&
                 ((0x90 <= input[i+1]) && (input[i+1] <= 0xbf)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)) &&
                 ((0x80 <= input[i+3]) && (input[i+3] <= 0xbf)))
        {
            i += 4;
        }
        // Planes 4-15.
        else if (((0xf1 <= input[i  ]) && (input[i  ] <= 0xf3)) &&
                 ((0x80 <= input[i+1]) && (input[i+1] <= 0xbf)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)) &&
                 ((0x80 <= input[i+3]) && (input[i+3] <= 0xbf)))
        {
            i += 4;
        }
        // plane 16.
        else if ((input[i] == 0xf4) &&
                 ((0x80 <= input[i+1]) && (input[i+1] <= 0x8f)) &&
                 ((0x80 <= input[i+2]) && (input[i+2] <= 0xbf)) &&
                 ((0x80 <= input[i+3]) && (input[i+3] <= 0xbf)))
        {
            i += 4;
        }
        else
            return false;
    }

    return true;
}

#if defined(__unix__) || defined(__APPLE__)
#include <iconv.h>

std::string to_utf8(const std::string &src, const std::string &encoding)
{
    std::string dst;

    auto handle = iconv_open("UTF-8", encoding.c_str());
    if (handle != iconv_t(-1))
    {
        char *inptr = const_cast<char *>(&src[0]);
        size_t insize = src.length();

        dst.resize((src.length() * 4) + 8);
        char *outptr = &dst[0];
        size_t outsize = dst.length();

        size_t rc = iconv(handle, &inptr, &insize, &outptr, &outsize);
        if (rc != size_t(-1))
            dst.resize(dst.length() - outsize);
        else
            dst.clear();

        iconv_close(handle);
    }

    return dst;
}

std::u32string to_utf32(const std::string &src, const char *encoding)
{
    std::u32string dst;

    auto handle = iconv_open("UTF-32", encoding ? encoding : "UTF-8");
    if (handle != iconv_t(-1))
    {
        char *inptr = const_cast<char *>(&src[0]);
        size_t insize = src.length();

        dst.resize(src.length() + 8);
        char *outptr = reinterpret_cast<char *>(&dst[0]);
        size_t outsize = dst.length() * sizeof(dst[0]);

        size_t rc = iconv(handle, &inptr, &insize, &outptr, &outsize);
        if (rc != size_t(-1))
            dst.resize(dst.length() - (outsize / sizeof(dst[0])));
        else
            dst.clear();

        iconv_close(handle);
    }

    return dst;
}

#elif defined(WIN32)
#include <windows.h>

std::wstring to_utf16(const std::string &src)
{
    std::wstring dst;
    dst.resize(src.length());
    dst.resize(MultiByteToWideChar(
            CP_UTF8, 0,
            &src[0], src.length(),
            &dst[0], dst.length()));

    return dst;
}

std::string from_utf16(const std::wstring &src)
{
    std::string dst;
    dst.resize(src.length() * 4);
    dst.resize(WideCharToMultiByte(
            CP_UTF8, 0,
            &src[0], src.length(),
            &dst[0], dst.length(),
            NULL, NULL));

    return dst;
}

static UINT to_codepage(const char *encoding)
{
    if      (encoding == nullptr)                   return CP_UTF8;
    else if (strcmp(encoding, "ISO-8859-1" ) == 0)  return 28591;
    else if (strcmp(encoding, "ISO-8859-2" ) == 0)  return 28592;
    else if (strcmp(encoding, "ISO-8859-3" ) == 0)  return 28593;
    else if (strcmp(encoding, "ISO-8859-4" ) == 0)  return 28594;
    else if (strcmp(encoding, "ISO-8859-5" ) == 0)  return 28595;
    else if (strcmp(encoding, "ISO-8859-6" ) == 0)  return 28596;
    else if (strcmp(encoding, "ISO-8859-7" ) == 0)  return 28597;
    else if (strcmp(encoding, "ISO-8859-8" ) == 0)  return 28598;
    else if (strcmp(encoding, "ISO-8859-9" ) == 0)  return 28599;
    else if (strcmp(encoding, "ISO-8859-13") == 0)  return 28603;
    else if (strcmp(encoding, "ISO-8859-15") == 0)  return 28605;

    return 0;
}

std::string to_utf8(const std::string &src, const std::string &encoding)
{
    std::string dst;

    const auto codepage = to_codepage(encoding.c_str());
    if (codepage && IsValidCodePage(codepage))
    {
        std::wstring tmp;
        tmp.resize(src.length());
        tmp.resize(MultiByteToWideChar(
                codepage, 0,
                &src[0], src.length(),
                &tmp[0], tmp.length()));

        dst.resize(tmp.length() * 4);
        dst.resize(WideCharToMultiByte(
                CP_UTF8, 0,
                &tmp[0], tmp.length(),
                &dst[0], dst.length(),
                NULL, NULL));
    }

    return dst;
}

static std::u32string to_utf32(const std::wstring &src)
{
    std::u32string dst;
    dst.reserve(src.size());

    for (size_t i = 0, n = src.size(); i < n; i++)
    {
        const char16_t ch = src[i];
        if ((ch >= 0xD800) && (ch <= 0xDBFF) && (i + 1 < n))
        {
            dst.push_back(
                        (char32_t(ch - 0xD800) << 10) +
                        char32_t(src[++i] - 0xDC00) +
                        0x0010000);
        }
        else
            dst.push_back(char32_t(ch));
    }

    return dst;
}

std::u32string to_utf32(const std::string &src, const char *encoding)
{
    std::u32string dst;

    const auto codepage = to_codepage(encoding);
    if (codepage && IsValidCodePage(codepage))
    {
        std::wstring tmp;
        tmp.resize(src.length());
        tmp.resize(MultiByteToWideChar(
                codepage, 0,
                &src[0], src.length(),
                &tmp[0], tmp.length()));

        dst = to_utf32(tmp);
    }

    return dst;
}
#endif
