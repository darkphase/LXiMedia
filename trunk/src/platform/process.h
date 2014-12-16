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

#ifndef PLATFORM_PROCESS_H
#define PLATFORM_PROCESS_H

#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#if defined(__unix__) || defined(__APPLE__)
# include <sys/types.h>
#elif defined(WIN32)
# include <thread>
#endif

namespace platform {

class process : public std::istream
{
public:
    process(
            const std::function<void(std::ostream &)> &,
            bool background_task = false);

    process(process &&);
    process & operator=(process &&);

    ~process();

    process(const process &) = delete;
    process & operator=(const process &) = delete;

    void join();

private:
    class pipe_streambuf;
    int pipe_desc[2];

#if defined(__unix__) || defined(__APPLE__)
    pid_t child;
#elif defined(WIN32)
    std::thread thread;
#endif
};

} // End of namespace

#endif
