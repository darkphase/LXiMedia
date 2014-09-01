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

#ifndef MESSAGELOOP_H
#define MESSAGELOOP_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <set>

class timer;

class messageloop
{
friend class timer;
public:
    messageloop();
    ~messageloop();

    int run();
    void stop(int);
    void post(const std::function<void()> &);
    void send(const std::function<void()> &);

    void process_events(const std::chrono::milliseconds &duration);

private:
    void abort();
    void timer_add(class timer &);
    void timer_remove(class timer &);

private:
    std::mutex mutex;
    std::condition_variable message_added;
    std::condition_variable send_processed;

    bool stopped;
    int exitcode;
    std::queue<std::function<void()>> messages;

    std::set<timer *> timers;
    std::chrono::steady_clock clock;
};

class timer
{
friend class messageloop;
public:
    timer(class messageloop &, const std::function<void()> &timeout);
    ~timer();

    template<typename rep, typename period>
    void start(std::chrono::duration<rep, period> interval, bool once = false);
    void start(std::chrono::nanoseconds, bool once);
    void stop();

private:
    class messageloop &messageloop;
    const std::function<void()> timeout;

    std::chrono::nanoseconds interval;
    std::chrono::steady_clock::time_point next;
    bool once;
};

template<typename rep, typename period>
void timer::start(std::chrono::duration<rep, period> interval, bool once)
{
    return start(std::chrono::duration_cast<std::chrono::nanoseconds>(interval), once);
}

#endif
