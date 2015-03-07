/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#ifndef PLATFORM_MESSAGELOOP_H
#define PLATFORM_MESSAGELOOP_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <set>

namespace platform {

class messageloop_ref;
class timer;

class messageloop
{
friend class messageloop_ref;
friend class timer;
public:
    messageloop();
    ~messageloop();

    int run();
    void stop(int);
    void send(const std::function<void()> &);

    int process_events(std::chrono::milliseconds duration);

private:
    void post(class messageloop_ref &, const std::function<void()> &);
    void post(class messageloop_ref &, std::function<void()> &&);
    void abort(class messageloop_ref &);

    void timer_add(class timer &, std::chrono::nanoseconds);
    void timer_remove(class timer &);

private:
    struct message
    {
        class messageloop_ref *ref;
        std::function<void()> func;
    };

    std::mutex mutex;
    std::condition_variable send_processed;
    std::list<message> messages;
    std::set<timer *> timers;
    std::chrono::steady_clock clock;
    int exitcode;

#if defined(__unix__) || defined(__APPLE__)
    std::condition_variable message_added;
    bool stopped;
#elif defined(WIN32)
    void *window_handle;
#endif
};

class messageloop_ref
{
friend class timer;
public:
    messageloop_ref(class messageloop &);
    messageloop_ref(class messageloop_ref &);
    messageloop_ref(const class messageloop_ref &) = delete;
    ~messageloop_ref();

    void stop(int);

    void send(const std::function<void()> &);
    void post(const std::function<void()> &);
    void post(std::function<void()> &&);

    int process_events(const std::chrono::milliseconds &duration);

private:
    class messageloop &messageloop;
};

class timer
{
friend class messageloop;
public:
    template<typename rep, typename period>
    static void single_shot(class messageloop &, std::chrono::duration<rep, period>, const std::function<void()> &);
    static void single_shot(class messageloop &, std::chrono::nanoseconds, const std::function<void()> &);
    template<typename rep, typename period>
    static void single_shot(class messageloop_ref &, std::chrono::duration<rep, period>, const std::function<void()> &);
    static void single_shot(class messageloop_ref &, std::chrono::nanoseconds, const std::function<void()> &);

public:
    timer(class messageloop &, const std::function<void()> &timeout);
    timer(class messageloop_ref &, const std::function<void()> &timeout);
    ~timer();

    template<typename rep, typename period>
    void start(std::chrono::duration<rep, period>, bool once = false);
    void start(std::chrono::nanoseconds, bool once);
    void stop();

private:
    class messageloop &messageloop;
    const std::function<void()> timeout;

#if defined(__unix__) || defined(__APPLE__)
    std::chrono::nanoseconds interval;
    std::chrono::steady_clock::time_point next;
#endif
    bool once;
};

template<typename rep, typename period>
void timer::single_shot(
        class messageloop &messageloop,
        std::chrono::duration<rep, period> interval,
        const std::function<void()> &timeout)
{
    return single_shot(
                messageloop,
                std::chrono::duration_cast<std::chrono::nanoseconds>(interval),
                timeout);
}

template<typename rep, typename period>
void timer::single_shot(
        class messageloop_ref &messageloop,
        std::chrono::duration<rep, period> interval,
        const std::function<void()> &timeout)
{
    return single_shot(
                messageloop,
                std::chrono::duration_cast<std::chrono::nanoseconds>(interval),
                timeout);
}

template<typename rep, typename period>
void timer::start(std::chrono::duration<rep, period> interval, bool once)
{
    return start(std::chrono::duration_cast<std::chrono::nanoseconds>(interval), once);
}

} // End of namespace

#endif
