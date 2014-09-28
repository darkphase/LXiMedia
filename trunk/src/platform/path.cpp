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

static const size_t min_file_size = 65536;

namespace platform {

std::string clean_path(const std::string &path)
{
    std::string result;

    bool ls = false;
    for (char i : path)
    {
        if ((i != '/') || !ls)
            result.push_back(i);

        ls = i == '/';
    }

    if ((result.size() > 1) && (result[result.size() - 1] == '/'))
        result.pop_back();

    return result;
}

} // End of namespace

#if defined(__unix__)
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>

namespace platform {

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

static const std::unordered_set<std::string> & hidden_suffixes()
{
    static std::unordered_set<std::string> hidden_suffixes;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        hidden_suffixes.insert(".db" );
        hidden_suffixes.insert(".nfo");
        hidden_suffixes.insert(".sub");
        hidden_suffixes.insert(".idx");
        hidden_suffixes.insert(".srt");
        hidden_suffixes.insert(".txt");
    });

    return hidden_suffixes;
}

static std::string suffix_of(const std::string &name)
{
    const size_t ldot = name.find_last_of('.');
    if (ldot != name.npos)
        return name.substr(ldot);

    return name;
}

std::vector<std::string> list_files(
        const std::string &path,
        bool directories_only,
        size_t max_count)
{
    std::string cpath = path;
    while (!cpath.empty() && (cpath[cpath.length() - 1] == '/')) cpath.pop_back();
    cpath.push_back('/');

    auto &hidden_dirs = platform::hidden_dirs();
    for (auto &i : hidden_dirs)
        if (starts_with(cpath, i + '/'))
            return std::vector<std::string>();

    std::vector<std::string> result;

    auto dir = ::opendir(cpath.c_str());
    if (dir)
    {
        auto &hidden_names = platform::hidden_names();
        auto &hidden_suffixes = platform::hidden_suffixes();

        for (auto dirent = ::readdir(dir);
             dirent && (result.size() < max_count);
             dirent = ::readdir(dir))
        {
            struct stat stat;
            if ((dirent->d_name[0] != '.') &&
                (::stat((cpath + dirent->d_name).c_str(), &stat) == 0))
            {
                std::string name = dirent->d_name, lname = to_lower(name);
                if (S_ISDIR(stat.st_mode) &&
                    (hidden_names.find(lname) == hidden_names.end()) &&
                    (hidden_dirs.find(cpath + name) == hidden_dirs.end()))
                {
                    result.emplace_back(name + '/');
                }
                else if (!directories_only && (size_t(stat.st_size) >= min_file_size) &&
                         (hidden_suffixes.find(suffix_of(lname)) == hidden_suffixes.end()))
                {
                    result.emplace_back(std::move(name));
                }
            }
        }

        ::closedir(dir);
    }

    return result;
}

std::string home_dir()
{
    const char *home = getenv("HOME");
    if (home)
        return clean_path(home);

    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir)
        return clean_path(pw->pw_dir);

    return std::string();
}

std::string config_dir()
{
    const std::string home = home_dir();
    if (!home.empty())
    {
        const std::string result = home + "/.config/lximediaserver/";
        mkdir(result.c_str(), S_IRWXU);

        return result;
    }

    return std::string();
}

} // End of namespace

#elif defined(WIN32)
#include <windows.h>
#include <iostream>

namespace platform {

std::vector<std::string> list_root_directories()
{
    std::vector<std::string> result;

    wchar_t drives[4096];
    if (GetLogicalDriveStrings(sizeof(drives) / sizeof(*drives), drives) != 0)
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

std::string volume_name(const std::string &path)
{
    wchar_t volume_name[MAX_PATH + 1];
    if (GetVolumeInformation(
                to_windows_path(path).c_str(),
                volume_name,
                sizeof(volume_name) / sizeof(*volume_name),
                NULL, NULL, NULL, NULL, 0))
    {
        return from_utf16(volume_name);
    }

    return std::string();
}

bool starts_with(const std::wstring &text, const std::wstring &find)
{
    if (text.length() >= find.length())
        return wcsncmp(&text[0], &find[0], find.length()) == 0;

    return false;
}

static const std::wstring clean_path(const std::wstring &input)
{
    std::wstring result = input;
    while (!result.empty() && (result[result.length() - 1] == L'\\')) result.pop_back();
    return result;
}

static const std::wstring to_lower(const std::wstring &input)
{
    std::wstring result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}

static const std::unordered_set<std::wstring> & hidden_dirs()
{
    static std::unordered_set<std::wstring> hidden_dirs;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        const wchar_t *dir = _wgetenv(L"SystemDrive");
        if (dir == nullptr) dir = L"C:";
        hidden_dirs.insert(clean_path(dir) + L"\\program files");
        hidden_dirs.insert(clean_path(dir) + L"\\program files (x86)");
        hidden_dirs.insert(clean_path(dir) + L"\\windows");
        hidden_dirs.insert(clean_path(dir) + L"\\temp");
        hidden_dirs.insert(clean_path(dir) + L"\\recycler");

        dir = _wgetenv(L"ProgramFiles");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
        dir = _wgetenv(L"ProgramFiles(x86)");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
        dir = _wgetenv(L"SystemRoot");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
        dir = _wgetenv(L"TEMP");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
        dir = _wgetenv(L"TMP");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
        dir = _wgetenv(L"windir");
        if (dir) hidden_dirs.insert(to_lower(clean_path(dir)));
    });

    return hidden_dirs;
}

static const std::unordered_set<std::wstring> & hidden_names()
{
    static std::unordered_set<std::wstring> hidden_names;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        hidden_names.insert(L"@eadir");
        hidden_names.insert(L"lost+found");
    });

    return hidden_names;
}

static const std::unordered_set<std::wstring> & hidden_suffixes()
{
    static std::unordered_set<std::wstring> hidden_suffixes;
    static std::once_flag flag;
    std::call_once(flag, []
    {
        hidden_suffixes.insert(L".db" );
        hidden_suffixes.insert(L".nfo");
        hidden_suffixes.insert(L".sub");
        hidden_suffixes.insert(L".idx");
        hidden_suffixes.insert(L".srt");
        hidden_suffixes.insert(L".txt");
    });

    return hidden_suffixes;
}

static std::wstring suffix_of(const std::wstring &name)
{
    const size_t ldot = name.find_last_of(L'.');
    if (ldot != name.npos)
        return name.substr(ldot);

    return name;
}

std::vector<std::string> list_files(
        const std::string &path,
        bool directories_only,
        size_t max_count)
{
    const std::wstring wpath = clean_path(to_windows_path(to_lower(path))) + L'\\';

    auto &hidden_dirs = platform::hidden_dirs();
    for (auto &i : hidden_dirs)
        if (starts_with(wpath, i + L'\\'))
            return std::vector<std::string>();

    std::vector<std::string> result;

    WIN32_FIND_DATA find_data;
    HANDLE handle = FindFirstFile((wpath + L'*').c_str(), &find_data);
    if (handle != INVALID_HANDLE_VALUE)
    {
        auto &hidden_names = platform::hidden_names();
        auto &hidden_suffixes = platform::hidden_suffixes();

        do
        {
            struct __stat64 stat;
            if ((find_data.cFileName[0] != L'.') &&
                (::_wstat64((wpath + find_data.cFileName).c_str(), &stat) == 0))
            {
                std::wstring name = find_data.cFileName, lname = to_lower(name);
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (hidden_names.find(lname) == hidden_names.end()) &&
                    (hidden_dirs.find(wpath + lname) == hidden_dirs.end()))
                {
                    result.emplace_back(from_utf16(name) + '/');
                }
                else if (!directories_only && (size_t(stat.st_size) >= min_file_size) &&
                         (hidden_suffixes.find(suffix_of(lname)) == hidden_suffixes.end()))
                {
                    result.emplace_back(from_utf16(name));
                }
            }
        } while((result.size() < max_count) && (FindNextFile(handle, &find_data) != 0));

        CloseHandle(handle);
    }

    return result;
}

std::string home_dir()
{
    const wchar_t * const appdata = _wgetenv(L"APPDATA");
    if (appdata)
        return from_windows_path(appdata);

    return std::string();
}

std::string config_dir()
{
    const std::string home = home_dir();
    if (!home.empty())
    {
        const std::string result = home + "/LXiMediaServer/";
        _wmkdir(to_windows_path(result).c_str());
        return result;
    }

    return std::string();
}

std::wstring to_windows_path(const std::string &src)
{
    std::wstring dst = to_utf16(src);
    std::replace(dst.begin(), dst.end(), L'/', L'\\');

    return dst;
}

std::string from_windows_path(const std::wstring &src)
{
    std::string dst = from_utf16(src);
    std::replace(dst.begin(), dst.end(), '\\', '/');

    return dst;
}

} // End of namespace

#endif
