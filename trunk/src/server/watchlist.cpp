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
#include "platform/string.h"

watchlist::watchlist(class platform::messageloop &messageloop)
    : messageloop(messageloop),
      inifile(platform::config_dir() + "/watchlist"),
      timer(messageloop, std::bind(&platform::inifile::save, &inifile)),
      save_delay(250)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };
}

watchlist::~watchlist()
{
}

bool watchlist::has_entry(const std::string &mrl)
{
    return inifile.has_section(from_percent(mrl));
}

std::chrono::milliseconds watchlist::last_position(const std::string &mrl)
{
    auto section = inifile.open_section(from_percent(mrl));

    return std::chrono::milliseconds(section.read("last_position", 0));
}

void watchlist::set_last_position(const std::string &mrl, std::chrono::milliseconds position)
{
    auto section = inifile.open_section(from_percent(mrl));

    section.write("last_position", position.count());
}

std::chrono::system_clock::time_point watchlist::last_seen(const std::string &mrl)
{
    auto section = inifile.open_section(from_percent(mrl));

    const std::chrono::minutes last_seen(section.read("last_seen", 0));
    return std::chrono::system_clock::time_point(last_seen);
}

void watchlist::set_last_seen(const std::string &mrl)
{
    auto section = inifile.open_section(from_percent(mrl));

    auto now = std::chrono::system_clock::now().time_since_epoch();
    section.write("last_seen", std::chrono::duration_cast<std::chrono::minutes>(now).count());
}
