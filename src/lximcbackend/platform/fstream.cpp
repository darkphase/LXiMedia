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

template <class _type, class _traits>
basic_utf8filebuf<_type, _traits>::basic_utf8filebuf(const std::string &filename, std::ios_base::openmode mode)
    : binary(mode & std::ios_base::binary),
      handle(INVALID_HANDLE_VALUE)
{
    const bool out = mode & std::ios_base::out;

    if (mode & std::ios_base::in)
    {
        handle = CreateFileW(
                    to_windows_path(filename).c_str(),
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
    }
    else if (mode & std::ios_base::out)
    {
        handle = CreateFileW(
                    to_windows_path(filename).c_str(),
                    GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    }

    if (handle != INVALID_HANDLE_VALUE)
    {
        this->setg(nullptr, nullptr, nullptr);
        if (out)
            this->setp(&buffer[0], &buffer[sizeof(buffer)]);
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
    if (ReadFile(handle, &buffer[0], sizeof(buffer), &bytesRead, NULL) &&
        (bytesRead > 0) && (bytesRead <= sizeof(buffer)))
    {
        if (!binary)
        {
            std::wstring dst;
            dst.resize(bytesRead);
            dst.resize(MultiByteToWideChar(
                    CP_ACP, 0,
                    &buffer[0], bytesRead,
                    &dst[0], dst.length()));

            text_buffer = from_utf16(dst);
            this->setg(&text_buffer[0], &text_buffer[0], &text_buffer[text_buffer.length()]);
        }
        else
            this->setg(&buffer[0], &buffer[0], &buffer[bytesRead]);

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
        std::wstring u16 = to_utf16(std::string(&buffer[0], write));

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
        if (WriteFile(handle, &buffer[pos], write - pos, &bytesWritten, NULL) && (bytesWritten <= (write - pos)))
            pos += bytesWritten;
        else
            return _traits::eof();
    }

    this->setp(&buffer[0], &buffer[sizeof(buffer)]);
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

#endif
