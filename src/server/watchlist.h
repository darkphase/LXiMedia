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

#ifndef WATCHLIST_H
#define WATCHLIST_H

#include "platform/inifile.h"
#include "platform/messageloop.h"
#include "platform/uuid.h"
#include <chrono>
#include <vector>

class watchlist
{
public:
    struct entry
    {
        entry();
        entry(
                std::chrono::system_clock::time_point last_seen,
                std::chrono::milliseconds last_position,
                std::chrono::milliseconds duration,
                const std::string &mrl);

        ~entry();

        std::chrono::system_clock::time_point last_seen;
        std::chrono::milliseconds last_position;
        std::chrono::milliseconds duration;
        std::string mrl;
    };

public:
    explicit watchlist(class platform::messageloop_ref &);
    ~watchlist();

    std::vector<struct entry> watched_items() const;
    struct entry watched_item(const platform::uuid &) const;
    void set_watched_item(const platform::uuid &, const struct entry &);

    static bool watched_till_end(std::chrono::milliseconds last_position, std::chrono::milliseconds duration);
    static bool watched_till_end(const struct entry &);

private:
    class platform::messageloop_ref messageloop;
    class platform::inifile inifile;
    class platform::timer timer;
    const std::chrono::milliseconds save_delay;
};

#endif