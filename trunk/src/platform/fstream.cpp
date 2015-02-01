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

#include "fstream.h"

#ifdef WIN32
#include "path.h"
#include "string.h"
#include <ext/stdio_filebuf.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

namespace platform {

template <class _type, class _traits>
static __gnu_cxx::stdio_filebuf<_type, _traits> * create_filebuf(const std::string &filename, std::ios_base::openmode mode)
{
    int flags = _O_NOINHERIT, pmode = 0;
    if ((mode & std::ios_base::in) && (mode & std::ios_base::out))
    {
        flags |= _O_RDWR | _O_CREAT;
        pmode |= _S_IREAD | _S_IWRITE;
    }
    else if (mode & std::ios_base::out)
    {
        flags |= _O_WRONLY | _O_CREAT;
        pmode |= _S_IREAD | _S_IWRITE;
    }
    else if (mode & std::ios_base::in)
        flags |= _O_RDONLY;

    if (mode & std::ios_base::app)
        flags |= _O_APPEND;

    if (mode & std::ios_base::binary)
        flags |= _O_BINARY;

    if (mode & std::ios_base::trunc)
        flags |= _O_TRUNC;

    int handle;
    if (pmode)
        handle = _wopen(to_windows_path(filename).c_str(), flags, pmode);
    else
        handle = _wopen(to_windows_path(filename).c_str(), flags);

    return new __gnu_cxx::stdio_filebuf<_type, _traits>(handle, mode);
}

template <class _type, class _traits>
basic_ifstream<_type, _traits>::basic_ifstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_istream<_type, _traits>(create_filebuf<_type, _traits>(filename.c_str(), mode | std::ios_base::in))
{
}

template <class _type, class _traits>
basic_ifstream<_type, _traits>::~basic_ifstream()
{
    delete this->rdbuf();
}

template <class _type, class _traits>
bool basic_ifstream<_type, _traits>::is_open() const
{
    return static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template <class _type, class _traits>
void basic_ifstream<_type, _traits>::close()
{
    static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->close();
}

template class basic_ifstream<char>;


template <class _type, class _traits>
basic_ofstream<_type, _traits>::basic_ofstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_ostream<_type, _traits>(create_filebuf<_type, _traits>(filename, mode | std::ios_base::out))
{
}

template <class _type, class _traits>
basic_ofstream<_type, _traits>::~basic_ofstream()
{
    delete this->rdbuf();
}

template <class _type, class _traits>
bool basic_ofstream<_type, _traits>::is_open() const
{
    return static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template <class _type, class _traits>
void basic_ofstream<_type, _traits>::close()
{
     static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->close();
}

template class basic_ofstream<char>;


template <class _type, class _traits>
basic_fstream<_type, _traits>::basic_fstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_iostream<_type, _traits>(create_filebuf<_type, _traits>(filename, mode))
{
}

template <class _type, class _traits>
basic_fstream<_type, _traits>::~basic_fstream()
{
    delete this->rdbuf();
}

template <class _type, class _traits>
bool basic_fstream<_type, _traits>::is_open() const
{
    return static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template <class _type, class _traits>
void basic_fstream<_type, _traits>::close()
{
    static_cast<__gnu_cxx::stdio_filebuf<_type, _traits> *>(this->rdbuf())->close();
}

template class basic_fstream<char>;

} // End of namespace

#endif
