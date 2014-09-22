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
#include "platform/string.h"

inifile::inifile(const std::string &filename)
    : filename(filename),
      touched(false)
{
    ifstream file(filename, std::ios_base::binary); // Binary needed to support UTF-8 on Windows
    for (std::string line, section; std::getline(file, line); )
        if (!line.empty() && (line[0] != ';') && (line[0] != '#'))
            {
                auto o = line.find_first_of('[');
                if (o == 0)
                {
                    auto c = line.find_first_of(']', o);
                    if (c != line.npos)
                        section = line.substr(o + 1, c - 1);

                    continue;
                }

                auto e = line.find_first_of('=');
                if (e != line.npos)
                    values[section][line.substr(0, e)] = line.substr(e + 1);
            }
}

inifile::~inifile()
{
    save();
}

void inifile::save()
{
    if (touched)
    {
        ofstream file(filename, std::ios_base::binary); // Binary needed to support UTF-8 on Windows
        for (auto &section : values)
        {
            if (!section.first.empty())
                file << '[' << section.first << ']' << std::endl;

            for (auto &value : section.second)
                file << value.first << '=' << value.second << std::endl;

            file << std::endl;
        }

        touched = false;
    }
}

class inifile::const_section inifile::open_section(const std::string &name) const
{
    return inifile::const_section(*this, name);
}

class inifile::section inifile::open_section(const std::string &name)
{
    return inifile::section(*this, name);
}


inifile::const_section::const_section(const class inifile &inifile, const std::string &section_name)
    : inifile(inifile),
      section_name(to_percent(section_name))
{
}

inifile::const_section::const_section(const const_section &from)
    : inifile(from.inifile),
      section_name(from.section_name)
{
}

std::string inifile::const_section::read(const std::string &name, const std::string &default_) const
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        auto j = i->second.find(to_percent(name));
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
    try { return int(std::stoi(read(name, std::to_string(default_)))); }
    catch (const std::invalid_argument &) { return default_; }
    catch (const std::out_of_range &) { return default_; }
}

long inifile::const_section::read(const std::string &name, long default_) const
{
    try { return int(std::stol(read(name, std::to_string(default_)))); }
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
    inifile.values[section_name][to_percent(name)] = value;

    inifile.touched = true;
    if (inifile.on_touched) inifile.on_touched();
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

void inifile::section::erase(const std::string &name)
{
    auto i = inifile.values.find(section_name);
    if (i != inifile.values.end())
    {
        auto j = i->second.find(to_percent(name));
        if (j != i->second.end())
        {
            i->second.erase(j);
            if (i->second.empty())
                inifile.values.erase(i);

            inifile.touched = true;
            if (inifile.on_touched) inifile.on_touched();
        }
    }
}
