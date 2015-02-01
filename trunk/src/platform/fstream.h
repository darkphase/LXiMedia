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

#ifndef PLATFORM_FSTREAM_H
#define PLATFORM_FSTREAM_H

#if defined(_GLIBCXX_FSTREAM)
# error Do not use <fstream> on Windows; it does not handle UTF-8 properly.
#endif

#ifdef WIN32
#include <istream>
#include <string>
#include <streambuf>

namespace platform {

template <class _type, class _traits = std::char_traits<_type>>
class basic_ifstream : public std::basic_istream<_type, _traits>
{
public:
    basic_ifstream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in);
    ~basic_ifstream();

    bool is_open() const;
    void close();
};

template <class _type, class _traits = std::char_traits<_type>>
class basic_ofstream : public std::basic_ostream<_type, _traits>
{
public:
   basic_ofstream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::out);
   ~basic_ofstream();

   bool is_open() const;
   void close();
};

template <class _type, class _traits = std::char_traits<_type>>
class basic_fstream : public std::basic_iostream<_type, _traits>
{
public:
   basic_fstream(const std::string &filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
   ~basic_fstream();

   bool is_open() const;
   void close();
};

typedef basic_ifstream<char> ifstream;
typedef basic_ofstream<char> ofstream;
typedef basic_fstream<char> fstream;

} // End of namespace

#else
# include <fstream>

namespace platform {

using std::ifstream;
using std::ofstream;
using std::fstream;

} // End of namespace

#endif

#endif
