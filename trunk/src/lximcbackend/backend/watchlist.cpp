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

#include "watchlist.h"
#include "platform/path.h"

static std::string filename();

watchlist::watchlist(class messageloop &messageloop)
    : messageloop(messageloop),
      inifile(filename()),
      timer(messageloop, std::bind(&inifile::save, &inifile)),
      save_delay(250)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };
}

watchlist::~watchlist()
{
}

std::chrono::milliseconds watchlist::last_position(const std::string &mrl)
{
    auto section = inifile.open_section(mrl);

    return std::chrono::milliseconds(section.read("last_position", 0));
}

void watchlist::set_last_position(const std::string &mrl, std::chrono::milliseconds position)
{
    auto section = inifile.open_section(mrl);

    section.write("last_position", position.count());
}

std::chrono::system_clock::time_point watchlist::last_seen(const std::string &mrl)
{
    auto section = inifile.open_section(mrl);

    const std::chrono::minutes last_seen(section.read("last_seen", 0));
    return std::chrono::system_clock::time_point(last_seen);
}

void watchlist::set_last_seen(const std::string &mrl)
{
    auto section = inifile.open_section(mrl);

    auto now = std::chrono::system_clock::now().time_since_epoch();
    section.write("last_seen", std::chrono::duration_cast<std::chrono::minutes>(now).count());
}

#if defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static std::string home_dir()
{
    const char *home = getenv("HOME");
    if (home)
        return clean_path(home);

    struct passwd *pw = getpwuid(getuid());
    if (pw && pw->pw_dir)
        return clean_path(pw->pw_dir);

    return std::string();
}

static std::string filename()
{
    return home_dir() + "/.config/LeX-Interactive/watchlist";
}

#elif defined(WIN32)
#include <cstdlib>
#include <direct.h>

static std::string filename()
{
    static const wchar_t confdir[] = L"\\LeX-Interactive\\";
    static const wchar_t conffile[] = L"watchlist";

    const wchar_t * const appdata = _wgetenv(L"APPDATA");
    if (appdata)
    {
        _wmkdir((std::wstring(appdata) + confdir).c_str());
        return from_windows_path(std::wstring(appdata) + confdir + conffile);
    }

    return std::string();
}
#endif
