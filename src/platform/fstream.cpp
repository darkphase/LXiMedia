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
#include <windows.h>

namespace platform {

template <class _type, class _traits>
basic_utf8filebuf<_type, _traits>::basic_utf8filebuf(const std::string &filename, std::ios_base::openmode mode)
    : binary(mode & std::ios_base::binary),
      handle(INVALID_HANDLE_VALUE)
{
    DWORD desired_access = 0;
    DWORD creation_disposition = 0;

    if (mode & std::ios_base::in)
    {
        desired_access |= GENERIC_READ;
        creation_disposition = OPEN_EXISTING;
    }

    if (mode & std::ios_base::out)
    {
        desired_access |= GENERIC_WRITE;
        creation_disposition = CREATE_ALWAYS;
    }

    handle = CreateFile(
                to_windows_path(filename).c_str(),
                desired_access,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                NULL,
                creation_disposition,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

    if (handle != INVALID_HANDLE_VALUE)
    {
        this->setg(nullptr, nullptr, nullptr);
        if (mode & std::ios_base::out)
            this->setp(&write_buffer[0], &write_buffer[sizeof(write_buffer)]);
    }
}

template <class _type, class _traits>
basic_utf8filebuf<_type, _traits>::~basic_utf8filebuf()
{
    if (handle != INVALID_HANDLE_VALUE)
    {
        basic_utf8filebuf<_type, _traits>::sync();
        CloseHandle(handle);
    }
}

template <class _type, class _traits>
bool basic_utf8filebuf<_type, _traits>::is_open() const
{
    return handle != INVALID_HANDLE_VALUE;
}

template <class _type, class _traits>
int basic_utf8filebuf<_type, _traits>::underflow()
{
    if ((this->gptr() != nullptr) &&
        (this->gptr() < this->egptr())) // buffer not exhausted
    {
        return _traits::to_int_type(*this->gptr());
    }

    DWORD bytesRead = 0;
    if (ReadFile(handle, &read_buffer[0], sizeof(read_buffer), &bytesRead, NULL) &&
        (bytesRead > 0) && (bytesRead <= sizeof(read_buffer)))
    {
        if (!binary)
        {
            std::wstring dst;
            dst.resize(bytesRead);
            dst.resize(MultiByteToWideChar(
                    CP_ACP, 0,
                    &read_buffer[0], bytesRead,
                    &dst[0], dst.length()));

            text_buffer = from_utf16(dst);
            this->setg(&text_buffer[0], &text_buffer[0], &text_buffer[text_buffer.length()]);
        }
        else
            this->setg(&read_buffer[0], &read_buffer[0], &read_buffer[bytesRead]);

        return _traits::to_int_type(*this->gptr());
    }

    return _traits::eof();
}

template <class _type, class _traits>
int basic_utf8filebuf<_type, _traits>::overflow(int value)
{
    const int write = this->pptr() - this->pbase();
    if (!binary)
    {
        std::wstring u16 = to_utf16(std::string(&write_buffer[0], write));

        std::string dst;
        dst.resize(u16.length() * 4);
        dst.resize(WideCharToMultiByte(
                CP_ACP, 0,
                &u16[0], u16.length(),
                &dst[0], dst.length(),
                NULL, NULL));

        for (size_t pos = 0; pos < dst.length(); )
        {
            DWORD bytesWritten = 0;
            if (WriteFile(handle, &dst[pos], dst.length() - pos, &bytesWritten, NULL) && (bytesWritten <= (dst.length() - pos)))
                pos += bytesWritten;
            else
                return _traits::eof();
        }
    }
    else for (int pos = 0; pos < write; )
    {
        DWORD bytesWritten = 0;
        if (WriteFile(handle, &write_buffer[pos], write - pos, &bytesWritten, NULL) && (bytesWritten <= DWORD(write - pos)))
            pos += bytesWritten;
        else
            return _traits::eof();
    }

    this->setp(&write_buffer[0], &write_buffer[sizeof(write_buffer)]);
    if (!_traits::eq_int_type(value, _traits::eof()))
        this->sputc(value);

    return _traits::not_eof(value);
}

template <class _type, class _traits>
int basic_utf8filebuf<_type, _traits>::sync()
{
    int result = this->overflow(_traits::eof());
    return _traits::eq_int_type(result, _traits::eof()) ? -1 : 0;
}

template <class _type, class _traits>
std::streambuf::pos_type basic_utf8filebuf<_type, _traits>::seekoff(std::streambuf::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which)
{
    LARGE_INTEGER lmove, lpos;
    lmove.QuadPart = off;

    BOOL result;
    switch (dir)
    {
    case std::ios_base::beg:
        result = SetFilePointerEx(handle, lmove, &lpos, FILE_BEGIN);
        break;

    case std::ios_base::cur:
        result = SetFilePointerEx(handle, lmove, &lpos, FILE_CURRENT);
        break;

    case std::ios_base::end:
        result = SetFilePointerEx(handle, lmove, &lpos, FILE_END);
        break;

    default:
        result = FALSE;
        break;
    }

    if (result != FALSE)
    {
        if (which & std::ios_base::in)
            this->setg(this->eback(), this->egptr(), this->egptr());
        else if (which & std::ios_base::out)
            this->setp(this->epptr(), this->epptr());

        return std::streambuf::pos_type(lpos.QuadPart);
    }

    return std::streambuf::pos_type(std::streambuf::off_type(-1));
}

template <class _type, class _traits>
std::streambuf::pos_type basic_utf8filebuf<_type, _traits>::seekpos(std::streambuf::pos_type pos, std::ios_base::openmode which)
{
    return seekoff(std::streambuf::off_type(pos), std::ios_base::beg, which);
}

template class basic_utf8filebuf<char>;


template <class _type, class _traits>
basic_ifstream<_type, _traits>::basic_ifstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_istream<_type, _traits>(new basic_utf8filebuf<_type, _traits>(filename, mode | std::ios_base::in))
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
    return static_cast<basic_utf8filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template class basic_ifstream<char>;


template <class _type, class _traits>
basic_ofstream<_type, _traits>::basic_ofstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_ostream<_type, _traits>(new basic_utf8filebuf<_type, _traits>(filename, mode | std::ios_base::out))
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
    return static_cast<basic_utf8filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template class basic_ofstream<char>;


template <class _type, class _traits>
basic_fstream<_type, _traits>::basic_fstream(const std::string &filename, std::ios_base::openmode mode)
    : std::basic_iostream<_type, _traits>(new basic_utf8filebuf<_type, _traits>(filename, mode))
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
    return static_cast<basic_utf8filebuf<_type, _traits> *>(this->rdbuf())->is_open();
}

template class basic_fstream<char>;

} // End of namespace

#endif
