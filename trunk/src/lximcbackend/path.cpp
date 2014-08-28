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

#include "path.h"
#include "string.h"
#include <algorithm>
#include <map>
#include <mutex>
#include <unordered_set>

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

std::vector<std::string> list_root_directories()
{
    std::vector<std::string> result;
    result.emplace_back("/");

    return result;
}

static const std::unordered_set<std::string> & hidden_dirs()
{
    static std::unordered_set<std::string> hidden_dirs;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        hidden_dirs.insert("/bin"   );
        hidden_dirs.insert("/boot"  );
        hidden_dirs.insert("/dev"   );
        hidden_dirs.insert("/etc"   );
        hidden_dirs.insert("/lib"   );
        hidden_dirs.insert("/proc"  );
        hidden_dirs.insert("/sbin"  );
        hidden_dirs.insert("/sys"   );
        hidden_dirs.insert("/tmp"   );
        hidden_dirs.insert("/usr"   );
        hidden_dirs.insert("/var"   );
    });

    return hidden_dirs;
}

static const std::unordered_set<std::string> & hidden_names()
{
    static std::unordered_set<std::string> hidden_names;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        hidden_names.insert("@eadir");
        hidden_names.insert("lost+found");
    });

    return hidden_names;
}

std::vector<std::string> list_files(
        const std::string &path,
        bool directories_only,
        const size_t min_file_size)
{
    std::multimap<std::string, std::string> dirs, files;

    auto dir = ::opendir(path.c_str());
    if (dir)
    {
        auto &hidden_dirs = ::hidden_dirs();
        auto &hidden_names = ::hidden_names();

        std::string cpath = path;
        while (!cpath.empty() && (cpath[cpath.length() - 1] == '/')) cpath.pop_back();
        cpath.push_back('/');

        for (auto &i : hidden_dirs)
            if (starts_with(cpath, i + '/'))
                return std::vector<std::string>();

        for (auto dirent = ::readdir(dir); dirent; dirent = ::readdir(dir))
        {
            struct stat stat;
            if ((dirent->d_name[0] != '.') &&
                (::stat((cpath + dirent->d_name).c_str(), &stat) == 0))
            {
                std::string name = dirent->d_name, lname = to_lower(name);
                if (S_ISDIR(stat.st_mode) &&
                    (hidden_names.find(name) == hidden_names.end()) &&
                    (hidden_dirs.find(cpath + name) == hidden_dirs.end()))
                {
                    dirs.emplace(std::move(lname), name + '/');
                }
                else if (!directories_only && (size_t(stat.st_size) >= min_file_size))
                    files.emplace(std::move(lname), std::move(name));
            }
        }

        ::closedir(dir);
    }

    std::vector<std::string> result;
    result.reserve(dirs.size() + files.size());
    for (auto &i : dirs) result.emplace_back(std::move(i.second));
    for (auto &i : files) result.emplace_back(std::move(i.second));
    return result;
}
#elif defined(WIN32)
#include <windows.h>

std::vector<std::string> list_root_directories()
{
    std::vector<std::string> result;

    wchar_t drives[4096];
    if (GetLogicalDriveStringsW(sizeof(drives) / sizeof(*drives), drives) != 0)
    {
        for (wchar_t *i = drives; *i; )
        {
            const size_t len = wcslen(i);
            result.emplace_back(from_windows_path(std::wstring(i, len)));
            i += len + 1;
        }
    }

    return result;
}

std::vector<std::string> list_files(
        const std::string &path,
        bool directories_only,
        const size_t min_file_size)
{
    std::multimap<std::string, std::string> dirs, files;

    std::wstring wpath = to_windows_path(path);
    while (!wpath.empty() && (wpath[wpath.length() - 1] == L'\\')) wpath.pop_back();
    wpath.push_back(L'\\');

    WIN32_FIND_DATAW find_data;
    HANDLE handle = FindFirstFileW((wpath + L'*').c_str(), &find_data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            struct __stat64 stat;
            if ((find_data.cFileName[0] != L'.') &&
                (::_wstat64((wpath + find_data.cFileName).c_str(), &stat) == 0))
            {
                std::string name = from_windows_path(find_data.cFileName), lname = to_lower(name);
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (lname != "@eadir"))
                {
                    dirs.emplace(std::move(lname), name + '/');
                }
                else if (!directories_only && (size_t(stat.st_size) >= min_file_size))
                    files.emplace(std::move(lname), std::move(name));
            }
        } while(FindNextFileW(handle, &find_data) != 0);

        CloseHandle(handle);
    }

    std::vector<std::string> result;
    result.reserve(dirs.size() + files.size());
    for (auto &i : dirs) result.emplace_back(std::move(i.second));
    for (auto &i : files) result.emplace_back(std::move(i.second));
    return result;
}

std::wstring to_windows_path(const std::string &src)
{
    std::wstring dst;
    dst.resize(src.length());
    dst.resize(MultiByteToWideChar(
            CP_UTF8, 0,
            &src[0], src.length(),
            &dst[0], dst.length()));

    std::replace(dst.begin(), dst.end(), L'/', L'\\');

    return dst;
}

std::string from_windows_path(const std::wstring &src)
{
    std::string dst;
    dst.resize(src.length() * 4);
    dst.resize(WideCharToMultiByte(
            CP_UTF8, 0,
            &src[0], src.length(),
            &dst[0], dst.length(),
            NULL, NULL));

    std::replace(dst.begin(), dst.end(), '\\', '/');

    return dst;
}
#endif
