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

#include "resource_file.h"
#include "platform/fstream.h"
#include <atomic>
#include <stdexcept>

static std::string get_temp_file(const std::string &);
static void remove_file(const std::string &);

namespace resources {

resource_file::resource_file(const unsigned char *data, size_t size, const std::string &suffix)
    : data(data),
      size(size),
      filename(get_temp_file(suffix))
{
    platform::ofstream str(filename, std::ios::binary);
    if (str.is_open())
        str.write(reinterpret_cast<const char *>(data), size);
}

resource_file::~resource_file()
{
    remove_file(filename);
}

}

static std::atomic<int> file_id(100);

#if defined(__unix__)
#include <unistd.h>

static std::string get_temp_file(const std::string &suffix)
{
    return std::string("/tmp/") + std::to_string(getpid()) + '.' + std::to_string(file_id++) + suffix;
}

void remove_file(const std::string &file)
{
    remove(file.c_str());
}

#elif defined(WIN32)
#include <cstdlib>
#include <process.h>
#include "platform/path.h"

static std::string get_temp_file(const std::string &suffix)
{
    const wchar_t * const temp = _wgetenv(L"TEMP");
    if (temp)
    {
        return platform::from_windows_path(
                    std::wstring(temp) + L'\\' +
                    std::to_wstring(_getpid()) + L'.' + std::to_wstring(file_id++) +
                    platform::to_windows_path(suffix));
    }

    throw std::runtime_error("failed to get TEMP directory");
}

void remove_file(const std::string &file)
{
    _wremove(platform::to_windows_path(file).c_str());
}
#endif
