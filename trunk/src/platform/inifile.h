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

#ifndef PLATFORM_INIFILE_H
#define PLATFORM_INIFILE_H

#include <string>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <set>

namespace platform {

class inifile
{
private:
    class string_ref
    {
    public:
        string_ref();
        string_ref(const std::string &);
        string_ref(
                std::istream &,
                std::istream::pos_type off,
                size_t len);

        string_ref & operator=(const string_ref &);
        string_ref & operator=(const std::string &);

        operator std::string() const;

        bool dirty() const { return istream == nullptr; }
        std::string escaped_value() const;
        std::istream::pos_type offset() const { return off; }
        size_t length() const { return len; }

    private:
        std::istream *istream;
        std::istream::pos_type off;
        size_t len;
        std::string escval;
    };

public:
    class const_section
    {
    friend class inifile;
    public:
        const_section(const const_section &);

        std::set<std::string> names() const;
        bool has_value(const std::string &name) const;

        std::string read(const std::string &name, const std::string &default_ = std::string()) const;
        std::string read(const std::string &name, const char *default_) const;
        bool read(const std::string &name, bool default_) const;
        int read(const std::string &name, int default_) const;
        long read(const std::string &name, long default_) const;
        long long read(const std::string &name, long long default_) const;

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
        void write(const std::string &name, long long value);

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

    std::set<std::string> sections() const;
    bool has_section(const std::string &name) const;

    class const_section open_section(const std::string &name = std::string()) const;
    class section open_section(const std::string &name = std::string());
    void erase_section(const std::string &name = std::string());

private:
    void soft_touch();
    void hard_touch();
    void load_file();
    void save_file();

private:
    const std::string filename;
    std::unique_ptr<std::iostream> file;
    std::map<std::string, std::map<std::string, string_ref>> values;
    enum { none, soft, hard } touched;
};

} // End of namespace

#endif
