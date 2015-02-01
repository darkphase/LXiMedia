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

#include "inifile.h"
#include "platform/fstream.h"
#include "platform/path.h"
#include <cassert>

namespace platform {

inifile::inifile(const std::string &filename)
    : filename(filename),
      file(),
      touched(none)
{
    load_file();
}

inifile::~inifile()
{
    save_file();
}

void inifile::save()
{
    save_file();
    if (!file)
        load_file();
}

std::set<std::string> inifile::sections() const
{
    std::set<std::string> result;
    for (auto &i : values)
        result.insert(i.first);

    return result;
}

bool inifile::has_section(const std::string &name) const
{
    return values.find(name) != values.end();
}

class inifile::const_section inifile::open_section(const std::string &name) const
{
    return inifile::const_section(*this, name);
}

class inifile::section inifile::open_section(const std::string &name)
{
    return inifile::section(*this, name);
}

void inifile::erase_section(const std::string &name)
{
    auto i = values.find(name);
    if (i != values.end())
    {
        values.erase(i);
        hard_touch();
    }
}

void inifile::soft_touch()
{
    if (touched == none)
        touched = soft;

    if (on_touched) on_touched();
}

void inifile::hard_touch()
{
    touched = hard;

    if (on_touched) on_touched();
}

static std::string unescape(const std::string &input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0, n = input.length(); i < n; i++)
    {
        const char c = input[i];
        if (c != '\\')
        {
            result.push_back(c);
        }
        else if (i + 1 < n)
        {
            switch (input[++i])
            {
            case 'r': result.push_back('\r'); break;
            case 'n': result.push_back('\n'); break;
            default : result.push_back(input[i]); break;
            }
        }
        else
            break;
    }

    return result;
}

bool is_escaped(const std::string &str, size_t offset)
{
    int n = 0;
    for (size_t i = offset; i > 1; i--)
        if (str[i - 1] == '\\')
            n++;
        else
            break;

    return n & 1;
}

void inifile::load_file()
{
    values.clear();

    // Binary needed to support UTF-8 on Windows
    file.reset(new platform::fstream(
                   filename,
                   std::ios_base::binary | std::ios_base::in | std::ios_base::out));

    for (std::string section; *file;)
    {
        const size_t line_begin = file->tellg();
        std::string line;
        if (std::getline(*file, line))
        {
            const size_t line_end = file->tellg();
            if (!line.empty() && (line[0] != ';') && (line[0] != '#'))
                {
                    if (line[0] == '[')
                    {
                        auto c = line.find_first_of(']');
                        while ((c != line.npos) && is_escaped(line, c))
                            c = line.find_first_of(']', c + 1);

                        if (c != line.npos)
                            section = unescape(line.substr(1, c - 1));

                        continue;
                    }

                    auto e = line.find_first_of('=');
                    while ((e != line.npos) && is_escaped(line, e))
                        e = line.find_first_of('=', e + 1);

                    if (e != line.npos)
                    {
                        values[section][unescape(line.substr(0, e))] = string_ref(
                                    *file,
                                    line_begin + e + 1,
                                    line_end - 1 - (line_begin + e + 1));
                    }
                }
        }
        else
            break;
    }
}

static std::string escape(const std::string &input)
{
    std::string result;
    result.reserve(input.size());

    for (auto c : input)
        switch (c)
        {
        case '\r':
            result.push_back('\\');
            result.push_back('r');
            break;

        case '\n':
            result.push_back('\\');
            result.push_back('n');
            break;

        case '\\':
        case '=' :
        case '[' :
        case ']' :
        case ';' :
        case '#' :
            result.push_back('\\');

        default:
            result.push_back(c);
            break;
        }

    return result;
}

void inifile::save_file()
{
    switch (touched)
    {
    case none:
        break;

    case soft:
        file->clear();

        for (auto &section : values)
        {
            for (auto &value : section.second)
                if (value.second.dirty())
                {
                    if (value.second.offset() >
                            std::istream::pos_type(value.first.length()))
                    {
                        if (file->seekp(value.second.offset()))
                        {
                            const auto data = value.second.escaped_value();
                            assert(value.second.length() == data.length());
                            if (value.second.length() == data.length())
                            {
                                file->write(data.data(), data.length());

                                value.second = string_ref(
                                            *file,
                                            value.second.offset(),
                                            value.second.length());
                            }
                        }
                    }
                    else if (file->seekp(0, std::ios_base::end))
                    {
                        assert(values.size() == 1);

                        const size_t line_begin = file->tellp();

                        const auto escaped_name = escape(value.first);
                        const auto escaped_value = value.second.escaped_value();
                        *file << escaped_name << '='
                              << escaped_value << std::endl;

                        value.second = string_ref(
                                    *file,
                                    line_begin + escaped_name.length() + 1,
                                    escaped_value.length());
                    }
                }
        }

        file->flush();
        break;

    case hard:
        {
            // Binary needed to support UTF-8 on Windows
            platform::fstream file(
                        filename + '~',
                        std::ios_base::binary | std::ios_base::trunc |
                        std::ios_base::in | std::ios_base::out);

            if (file.is_open())
            {
                bool first = true;
                for (auto &section : values)
                {
                    if (!first)
                        file << std::endl;
                    else
                        first = false;

                    if (!section.first.empty())
                        file << '[' << escape(section.first) << ']' << std::endl;

                    for (auto &value : section.second)
                    {
                        file << escape(value.first) << '='
                             << value.second.escaped_value() << std::endl;
                    }
                }

                file.flush();
                this->file = nullptr;

                platform::ofstream ofile(
                            filename,
                            std::ios_base::binary | std::ios_base::trunc);
                if (ofile.is_open())
                {
                    file.seekg(0, std::ios_base::beg);
                    ofile << file.rdbuf();
                    file.close();
                    ofile.close();
                    remove_file(filename + '~');
                }
            }
        }

        break;
    }

    touched = none;
}


inifile::string_ref::string_ref()
    : istream(nullptr),
      off(0), len(0),
      escval()
{
}

inifile::string_ref::string_ref(const std::string &value)
    : istream(nullptr),
      off(0), len(0),
      escval(escape(value))
{
}

inifile::string_ref::string_ref(
        std::istream &istream,
        std::istream::pos_type off,
        size_t len)
    : istream(&istream),
      off(off), len(len),
      escval()
{
}

inifile::string_ref & inifile::string_ref::operator=(const string_ref &other)
{
    istream = other.istream;
    off = other.off;
    len = other.len;
    escval = other.escval;

    return *this;
}

inifile::string_ref & inifile::string_ref::operator=(const std::string &other)
{
    istream = nullptr;
    escval = escape(other);

    return *this;
}

inifile::string_ref::operator std::string() const
{
    return unescape(escaped_value());
}

std::string inifile::string_ref::escaped_value() const
{
    if (istream)
    {
        istream->clear();
        if (istream->seekg(off))
        {
            std::string result;
            result.resize(len);
            if (istream->read(&result[0], result.size()))
                return result;
        }

        return std::string();
    }

    return escval;
}


inifile::const_section::const_section(const class inifile &inifile, const std::string &section_name)
    : inifile(inifile),
      section_name(section_name)
{
}

inifile::const_section::const_section(const const_section &from)
    : inifile(from.inifile),
      section_name(from.section_name)
{
}

std::set<std::string> inifile::const_section::names() const
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        std::set<std::string> result;
        for (auto &j : i->second)
            result.insert(j.first);

        return result;
    }

    return std::set<std::string>();
}

bool inifile::const_section::has_value(const std::string &name) const
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
        return i->second.find(name) != i->second.end();

    return false;
}

std::string inifile::const_section::read(const std::string &name, const std::string &default_) const
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        auto j = i->second.find(name);
        if (j != i->second.end())
            return j->second;
    }

    return default_;
}

std::string inifile::const_section::read(const std::string &name, const char *default_) const
{
    return read(name, default_ ? std::string(default_) : std::string());
}

bool inifile::const_section::read(const std::string &name, bool default_) const
{
    return read(name, std::string(default_ ? "true" : "false")) == "true";
}

int inifile::const_section::read(const std::string &name, int default_) const
{
    try { return std::stoi(read(name, std::to_string(default_))); }
    catch (const std::invalid_argument &) { return default_; }
    catch (const std::out_of_range &) { return default_; }
}

long inifile::const_section::read(const std::string &name, long default_) const
{
    try { return std::stol(read(name, std::to_string(default_))); }
    catch (const std::invalid_argument &) { return default_; }
    catch (const std::out_of_range &) { return default_; }
}

long long inifile::const_section::read(const std::string &name, long long default_) const
{
    try { return std::stoll(read(name, std::to_string(default_))); }
    catch (const std::invalid_argument &) { return default_; }
    catch (const std::out_of_range &) { return default_; }
}


inifile::section::section(class inifile &inifile, const std::string &section_name)
    : const_section(inifile, section_name),
      inifile(inifile)
{
}

inifile::section::section(const section &from)
    : const_section(from),
      inifile(from.inifile)
{
}

void inifile::section::write(const std::string &name, const std::string &value)
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        auto j = i->second.find(name);
        if (j != i->second.end())
        {
            const std::string old_value = j->second;
            if (old_value != value)
            {
                j->second = value;

                if (j->second.escaped_value().length() == j->second.length())
                    inifile.soft_touch();
                else
                    inifile.hard_touch();
            }
        }
        else
        {
            i->second.emplace(name, value);

            if (inifile.values.size() == 1)
                inifile.soft_touch();
            else
                inifile.hard_touch();
        }
    }
    else
    {
        inifile.values[section_name].emplace(name, value);
        inifile.hard_touch();
    }
}

void inifile::section::write(const std::string &name, const char *value)
{
    return write(name, value ? std::string(value) : std::string());
}

void inifile::section::write(const std::string &name, bool value)
{
    return write(name, std::string(value ? "true" : "false"));
}

void inifile::section::write(const std::string &name, int value)
{
    return write(name, std::to_string(value));
}

void inifile::section::write(const std::string &name, long value)
{
    return write(name, std::to_string(value));
}

void inifile::section::write(const std::string &name, long long value)
{
    return write(name, std::to_string(value));
}

void inifile::section::erase(const std::string &name)
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        auto j = i->second.find(name);
        if (j != i->second.end())
        {
            i->second.erase(j);
            if (i->second.empty())
                inifile.values.erase(i);

            inifile.hard_touch();
        }
    }
}

} // End of namespace
