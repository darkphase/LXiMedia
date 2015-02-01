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

#include "vlc/subtitles.h"
#include "platform/path.h"
#include "platform/fstream.h"
#include "platform/string.h"
#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <map>
#include <mutex>
#include <unordered_set>

namespace vlc {

namespace subtitles {

file::file()
    : own(false)
{
}

file::file(const std::string &path, const std::string &encoding)
    : path(path),
      own(false)
{
    const size_t ldot = path.find_last_of('.');
    if ((ldot != path.npos) && !encoding.empty() && (encoding != "UTF-8"))
    {
        // Binary needed to support UTF-8 on Windows
        platform::ifstream ifile(path, std::ios_base::binary);
        if (ifile.is_open())
        {
            ifile.seekg(0, std::ios_base::end);
            const auto length = ifile.tellg();
            ifile.seekg(0, std::ios_base::beg);

            std::string buffer;
            buffer.resize(length);
            if (ifile.read(&buffer[0], buffer.size()))
            {
                auto tmp_file = platform::temp_file_path(path.substr(ldot + 1));
                platform::ofstream ofile(tmp_file, std::ios_base::binary);
                if (ofile.is_open())
                {
                    buffer = to_utf8(buffer, encoding);
                    if (ofile.write(&buffer[0], buffer.size()))
                    {
                        this->path = tmp_file;
                        this->own = true;
                    }
                    else
                        platform::remove_file(tmp_file);
                }
            }
        }
    }
}

file::file(file &&other)
{
    path = std::move(other.path);
    own = other.own;
    other.own = false;
}

file::~file()
{
    if (own)
        platform::remove_file(path);
}

file & file::operator=(file &&other)
{
    if (own)
        platform::remove_file(path);

    path = std::move(other.path);
    own = other.own;
    other.own = false;

    return *this;
}


static std::string to_bare_name(const std::string &input, const std::string &suffix)
{
    const size_t base_len = input.length() - suffix.length();

    std::string result;
    result.reserve(base_len);
    for (size_t i = 0; i < base_len; i++)
    {
        const char c = input[i];
        if ((c < 0) ||
            ((c >= '0') && (c <= '9')) ||
            ((c >= 'a') && (c <= 'z')))
        {
            result.push_back(c);
        }
        else if ((c >= 'A') && (c <= 'Z'))
            result.push_back(c + ('a' - 'A'));
    }

    return result;
}

static std::string suffix_of(const std::string &name)
{
    const size_t ldot = name.find_last_of('.');
    if ((ldot != name.npos) && ((name.length() - ldot) <= 9))
        return name.substr(ldot);

    return std::string();
}

static const std::unordered_set<std::string> & subtitle_suffixes()
{
    static std::unordered_set<std::string> subtitle_suffixes;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        subtitle_suffixes.insert(".aqt" );
        subtitle_suffixes.insert(".cvd" );
        subtitle_suffixes.insert(".dks" );
        subtitle_suffixes.insert(".jss" );
        subtitle_suffixes.insert(".sub" );
        subtitle_suffixes.insert(".ttxt");
        subtitle_suffixes.insert(".mpl" );
        subtitle_suffixes.insert(".pjs" );
        subtitle_suffixes.insert(".psb" );
        subtitle_suffixes.insert(".rt"  );
        subtitle_suffixes.insert(".smi" );
        subtitle_suffixes.insert(".srt" );
        subtitle_suffixes.insert(".ssa" );
        subtitle_suffixes.insert(".svcd");
        subtitle_suffixes.insert(".usf" );
    });

    return subtitle_suffixes;
}

std::vector<std::string> find_subtitle_files(const std::string &path)
{
    std::vector<std::string> result;
    auto &subtitle_suffixes = subtitles::subtitle_suffixes();

    const size_t lsl = path.find_last_of('/');
    if (lsl != path.npos)
    {
        const auto dir = path.substr(0, lsl);
        const auto filename = path.substr(lsl + 1);
        const auto suffix = suffix_of(filename);
        const auto bare_name = to_bare_name(filename, suffix);

        for (auto &name : platform::list_files(dir, platform::file_filter::all))
        {
            const auto lname = to_lower(name);
            if (starts_with(lname, "sub") && ends_with(lname, "/"))
            {
                for (auto &subname : platform::list_files(dir + '/' + name, platform::file_filter::all))
                {
                    const auto suffix = to_lower(suffix_of(subname));
                    if ((subtitle_suffixes.find(suffix) != subtitle_suffixes.end()) &&
                        starts_with(to_bare_name(subname, suffix), bare_name))
                    {
                        result.emplace_back(dir + '/' + name + subname);
                    }
                }
            }
            else
            {
                const auto suffix = suffix_of(lname);
                if ((subtitle_suffixes.find(suffix) != subtitle_suffixes.end()) &&
                    starts_with(to_bare_name(lname, suffix), bare_name))
                {
                    result.emplace_back(dir + '/' + name);
                }
            }
        }
    }

    return result;
}

static bool language_of(
        const std::string &input,
        const char *&language,
        const char *&encoding)
{
    static const struct
    {
        const char *language, *encoding;
        const char32_t *freq;
    } frequencies[] =
    {
        { "Afrikaans"   , "ISO-8859-15" , U"eainrsotdlkgvmpubwfhjcêzëö" },
        { "Catalan"     , "ISO-8859-15" , U"easirtocnlumdpgvfbqóàòhíèjwxyzl" },
        { "Czech"       , "ISO-8859-2"  , U"oeantivlsrdkupímcházyjbřêéĉžýŝũgfúňwďóxťq" },
        { "Danish"      , "ISO-8859-15" , U"enadtrslgiohmkvufbpøjycéxqwèzüàóêç" },
        { "Dutch"       , "ISO-8859-15" , U"enatirodslghvkmubpwjczfxyëéóq" },
        { "English"     , "ISO-8859-15" , U"etaoinsrhldcumfpgwybvkxjqz" },
        { "Finnish"     , "ISO-8859-15" , U"aintesloukämrvjhpydögbcfwzxqå" },
        { "French"      , "ISO-8859-15" , U"esaitnrulodcmpévqfbghjàxèyêzçôùâûîœwkïëüæñ" },
        { "German"      , "ISO-8859-15" , U"enisratdhulcgmobwfkzvüpäßjöyqx" },
        { "Greek"       , "ISO-8859-7"  , U"αοιετσνηυρπκμλωδγχθφβξζψ" },
        { "Hebrew"      , "ISO-8859-8"  , U"יהולארתבמשעםנכדחקפסזגצטןךףץ" },
        { "Hungarian"   , "ISO-8859-2"  , U"eatlnskomzrigáéydbvhjfupöócíúüxwq" },
        { "Icelandic"   , "ISO-8859-15" , U"anriestudhlgmkfhvoáthídjóbyæúöpéýcxwzq" },
        { "Italian"     , "ISO-8859-15" , U"eaionlrtscdupmvghfbqzòàùìéèóykwxjô" },
        { "Norwegian"   , "ISO-8859-15" , U"erntsilakodgmvfupbhøjyæcwzxq" },
        { "Polish"      , "ISO-8859-2"  , U"iaeoznscrwyłdkmtpujlgębąhżśóćńfźvqx" },
        { "Portuguese"  , "ISO-8859-15" , U"aeosrinmdutlcphvqgfbãzçjáéxóõêôàyíèú" },
        { "Romanian"    , "ISO-8859-2"  , U"eaitnurcomăsdlpşvbţîfghzâjykxwq" },
        { "Russian"     , "ISO-8859-3"  , U"oeaинтсвлркдмпуëягбзчйхжшюцщeф" },
        { "Slovak"      , "ISO-8859-2"  , U"aoesnitrvlkdmcupzyhjgfbqwx" },
        { "Spanish"     , "ISO-8859-15" , U"eaosrnidlctumpbgyívqóhfzjéáñxúüwk" },
        { "Swedish"     , "ISO-8859-15" , U"eantrslidomgkvähfupåöbcjyxwzéq" },
        { "Turkish"     , "ISO-8859-9"  , U"aeinrlıdkmuytsboüşzgçhğvcöpfjwxq" },
        { nullptr       , nullptr       , nullptr }
    };

    const bool utf8 = is_utf8(input);
    auto text = utf8 ? to_utf32(input) : std::u32string();

    std::map<int, float> score;
    for (int i=0; frequencies[i].language; i++)
    {
        if (!utf8)
            text = to_utf32(input, frequencies[i].encoding);

        if (!text.empty())
        {
            // Count the number of characters.
            std::map<char32_t, int> count;
            for (char32_t c : text)
            {
                if ((c >= U'A') && (c <= U'Z'))
                    c += (U'a' - U'A');

                if (((c >= U'a') && (c <= U'z')) || (c >= 128))
                {
                    auto j = count.find(c);
                    if (j != count.end())
                        j->second++;
                    else
                        count[c] = 1;
                }
            }

            // Sort the characters by frequency.
            std::multimap<int, char32_t> freq;
            for (auto &j : count)
                freq.emplace(-j.second, j.first);

            // And determine the language.
            int n = 0;
            for (auto &j : freq)
            {
                const float weight = 1.0f / (1.0f + (float(n) / 2.0f));

                float d = 25.0f * weight;
                for (int k=0; frequencies[i].freq[k]; k++)
                    if (frequencies[i].freq[k] == j.second)
                    {
                        d = float(std::abs(k - n)) * weight;
                        break;
                    }

                auto k = score.find(i);
                if (k != score.end())
                    k->second += d;
                else
                    score[i] = d;

                n++;
            }
        }
    }

    // Find the best score
    int best = -1;
    float bestValue = 10000000.0f;
    for (auto &i : score)
        if (i.second < bestValue)
        {
            best = i.first;
            bestValue = i.second;
        }

    if (best >= 0)
    {
        language = frequencies[best].language;
        if (!utf8)
            encoding = frequencies[best].encoding;
        else
            encoding = "UTF-8";

        return true;
    }

    return false;
}

bool determine_subtitle_language(
        const std::string &path,
        const char *&language,
        const char *&encoding)
{
    // Binary needed to support UTF-8 on Windows
    platform::ifstream file(path, std::ios_base::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios_base::end);
        const auto length = std::min(uint64_t(file.tellg()), uint64_t(131072));
        file.seekg(0, std::ios_base::beg);

        std::string buffer;
        buffer.resize(length);
        if (file.read(&buffer[0], buffer.size()) &&
            (buffer.find_first_of('\0') == buffer.npos))
        {
            return language_of(buffer, language, encoding);
        }
    }

    return false;
}

} // End of namespace

} // End of namespace
