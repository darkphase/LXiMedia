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
#include <cassert>
#include <sstream>

watchlist::watchlist(class platform::messageloop_ref &messageloop)
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

std::map<std::string, std::chrono::minutes> watchlist::watched_items() const
{
    const auto section = inifile.open_section();
    const auto now = std::chrono::system_clock::now();

    std::map<std::string, std::chrono::minutes> result;
    for (auto uuid : section.names())
    {
        const auto m = mrl(uuid);
        if (!m.empty())
        {
            const auto ls = last_seen(uuid);
            result.emplace(m, std::chrono::duration_cast<std::chrono::minutes>(now - ls));
        }
    }

    return result;
}

std::chrono::milliseconds watchlist::last_position(const platform::uuid &uuid) const
{
    const auto section = inifile.open_section();

    const auto value = section.read(uuid);
    const size_t comma1 = value.find_first_of(',');
    if (comma1 != value.npos)
    {
        const size_t comma2 = value.find_first_of(',', comma1 + 1);
        if (comma2 != value.npos)
        {
            const auto last_position = value.substr(comma1 + 1, comma2 - comma1 - 1);
            try { return std::chrono::milliseconds(std::stoll(last_position)); }
            catch (const std::invalid_argument &) { }
            catch (const std::out_of_range &) { }
        }
    }

    return std::chrono::milliseconds(0);
}

std::chrono::system_clock::time_point watchlist::last_seen(const platform::uuid &uuid) const
{
    const auto section = inifile.open_section();

    const auto value = section.read(uuid);
    const size_t comma = value.find_first_of(',');
    if (comma != value.npos)
    {
        const auto last_seen = value.substr(0, comma);
        try { return std::chrono::system_clock::time_point(std::chrono::minutes(std::stoi(last_seen))); }
        catch (const std::invalid_argument &) { }
        catch (const std::out_of_range &) { }
    }

    return std::chrono::system_clock::time_point(std::chrono::minutes(0));
}

std::string watchlist::mrl(const platform::uuid &uuid) const
{
    const auto section = inifile.open_section();

    const auto value = section.read(uuid);
    const size_t comma1 = value.find_first_of(',');
    if (comma1 != value.npos)
    {
        const size_t comma2 = value.find_first_of(',', comma1 + 1);
        if (comma2 != value.npos)
            return value.substr(comma2 + 1);
    }

    return std::string();
}

void watchlist::set_last_position(const platform::uuid &uuid, std::chrono::milliseconds position, const std::string &mrl)
{
    assert(!uuid.is_null());
    if (!uuid.is_null())
    {
        auto section = inifile.open_section();

        auto now = std::chrono::system_clock::now().time_since_epoch();

        std::ostringstream str;
        str << std::chrono::duration_cast<std::chrono::minutes>(now).count()
            << ',' << position.count()
            << ',' << mrl;

        section.write(uuid, str.str());
    }
}
