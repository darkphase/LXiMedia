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

#ifndef FSTREAM_H
#define FSTREAM_H

#if defined(_GLIBCXX_FSTREAM)
# error Do not use <fstream> on Windows; it does not handle UTF-8 properly.
#endif

#ifdef WIN32
#include <istream>
#include <string>
#include <streambuf>

namespace platform {

template <class _type, class _traits = std::char_traits<_type>>
class basic_utf8filebuf : public std::basic_streambuf<_type, _traits>
{
public:
    basic_utf8filebuf(const std::string &filename, std::ios_base::openmode mode);
    ~basic_utf8filebuf();

    bool is_open() const;

    int underflow() override;
    int overflow(int value) override;
    int sync() override;

private:
    const bool binary;
    void *handle;
    char buffer[4096];
    std::string text_buffer;
};

template <class _type, class _traits = std::char_traits<_type>>
class basic_ifstream : public std::basic_istream<_type, _traits>
{
public:
    basic_ifstream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in);
    ~basic_ifstream();

    bool is_open() const;
};

template <class _type, class _traits = std::char_traits<_type>>
class basic_ofstream : public std::basic_ostream<_type, _traits>
{
public:
   basic_ofstream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::out);
   ~basic_ofstream();

   bool is_open() const;
};

typedef basic_ifstream<char> ifstream;
typedef basic_ofstream<char> ofstream;

} // End of namespace

#else
# include <fstream>

namespace platform {

using std::ifstream;
using std::ofstream;

} // End of namespace

#endif

#endif
