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
#include <iomanip>
#include <sstream>

using std::chrono::duration_cast;

static std::string to_string(const struct watchlist::entry &entry)
{
    std::ostringstream str;
    str << std::setw(9) << std::setfill('0')
        << duration_cast<std::chrono::minutes>(entry.last_seen.time_since_epoch()).count()
        << ',' << std::setw(6) << std::setfill('0')
        << duration_cast<std::chrono::seconds>(entry.last_position).count()
        << '/' << std::setw(6) << std::setfill('0')
        << duration_cast<std::chrono::seconds>(entry.duration).count()
        << ',' << entry.mrl;

    return str.str();
}

static watchlist::entry to_entry(const std::string &str)
{
    watchlist::entry result;

    const size_t comma1 = str.find_first_of(',');
    if (comma1 != str.npos)
    {
        const size_t slash = str.find_first_of('/', comma1 + 1);
        const size_t comma2 = str.find_first_of(',', comma1 + 1);
        if (comma2 != str.npos)
        {
            const auto last_seen = str.substr(0, comma1);
            try { result.last_seen = std::chrono::system_clock::time_point(std::chrono::minutes(std::stoi(last_seen))); }
            catch (const std::invalid_argument &) { }
            catch (const std::out_of_range &) { }

            const auto last_position = str.substr(comma1 + 1, std::min(slash, comma2) - comma1 - 1);
            try { result.last_position = std::chrono::seconds(std::stoll(last_position)); }
            catch (const std::invalid_argument &) { }
            catch (const std::out_of_range &) { }

            if (slash < comma2)
            {
                const auto duration = str.substr(slash + 1, comma2 - slash - 1);
                try { result.duration = std::chrono::seconds(std::stoll(duration)); }
                catch (const std::invalid_argument &) { }
                catch (const std::out_of_range &) { }
            }

            result.mrl = str.substr(comma2 + 1);
        }
    }

    return result;
}

watchlist::watchlist(class platform::messageloop_ref &messageloop)
    : messageloop(messageloop),
      inifile(platform::config_dir() + "/watchlist"),
      timer(messageloop, std::bind(&platform::inifile::save, &inifile)),
      save_delay(250)
{
    inifile.on_touched = [this] { timer.start(save_delay, true); };

//    // Convert file.
//    auto section = inifile.open_section();
//    for (auto &uuid : section.names())
//        section.write(uuid, to_string(to_entry(section.read(uuid))));
}

watchlist::~watchlist()
{
}

std::vector<struct watchlist::entry> watchlist::watched_items() const
{
    const auto section = inifile.open_section();

    std::vector<watchlist::entry> result;
    for (auto &uuid : section.names())
        result.emplace_back(to_entry(section.read(uuid)));

    return result;
}

struct watchlist::entry watchlist::watched_item(const platform::uuid &uuid) const
{
    const auto section = inifile.open_section();

    return to_entry(section.read(uuid));
}

void watchlist::set_watched_item(const platform::uuid &uuid, const struct entry &entry)
{
    assert(!uuid.is_null());
    if (!uuid.is_null())
    {
        auto section = inifile.open_section();

        section.write(uuid, to_string(entry));
    }
}

bool watchlist::watched_till_end(std::chrono::milliseconds last_position, std::chrono::milliseconds duration)
{
    return last_position > (duration - std::max(duration / 10, std::chrono::milliseconds(60000)));
}

bool watchlist::watched_till_end(const struct entry &entry)
{
    return watched_till_end(entry.last_position, entry.duration);
}


watchlist::entry::entry()
    : last_seen(),
      last_position(0),
      duration(0),
      mrl()
{
}

watchlist::entry::entry(
        std::chrono::system_clock::time_point last_seen,
        std::chrono::milliseconds last_position,
        std::chrono::milliseconds duration,
        const std::string &mrl)
    : last_seen(last_seen),
      last_position(last_position),
      duration(duration),
      mrl(mrl)
{
}

watchlist::entry::~entry()
{
}
