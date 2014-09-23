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

#ifndef INIFILE_H
#define INIFILE_H

#include <string>
#include <functional>
#include <map>

class inifile
{
public:
    class const_section
    {
    friend class inifile;
    public:
        const_section(const const_section &);

        std::string read(const std::string &name, const std::string &default_ = std::string()) const;
        std::string read(const std::string &name, const char *default_) const;
        bool read(const std::string &name, bool default_) const;
        int read(const std::string &name, int default_) const;
        long read(const std::string &name, long default_) const;

    protected:
        const_section(const class inifile &, const std::string &);

        const class inifile &inifile;
        const std::string section_name;
    };

    class section : public const_section
    {
    friend class inifile;
    public:
        section(const section &);

        void write(const std::string &name, const std::string &value);
        void write(const std::string &name, const char *value);
        void write(const std::string &name, bool value);
        void write(const std::string &name, int value);
        void write(const std::string &name, long value);

        void erase(const std::string &name);

    private:
        section(class inifile &, const std::string &);

        class inifile &inifile;
    };

public:
    explicit inifile(const std::string &);
    ~inifile();

    std::function<void()> on_touched;
    void save();

    bool has_section(const std::string &name) const;
    class const_section open_section(const std::string &name = std::string()) const;
    class section open_section(const std::string &name = std::string());

private:
    const std::string filename;
    std::map<std::string, std::map<std::string, std::string>> values;
    bool touched;
};

#endif
