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

#include "messageloop.h"
#include <cassert>
#include <cstring>
#include <map>

static void set_abort_handler(void(messageloop::*)(), messageloop *);
static void clear_abort_handler(messageloop *);

messageloop::messageloop()
    : stopped(false),
      exitcode(-1)
{
    set_abort_handler(&messageloop::abort, this);
}

messageloop::~messageloop()
{
    assert(timers.empty());

    clear_abort_handler(this);
}

int messageloop::run()
{
    stopped = false;

    for (;;)
    {
        std::function<void()> message;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (stopped)
            {
                break;
            }
            else if (!messages.empty())
            {
                message = messages.front();
                messages.pop();
            }
            else if (!timers.empty())
            {
                auto now = clock.now();
                for (auto i = timers.begin(); i != timers.end(); )
                {
                    if ((*i)->next <= now)
                    {
                        message = (*i)->timeout;

                        if ((*i)->once)
                        {
                            auto timer = *i;
                            timer_remove(**i);

                            i = timers.lower_bound(timer);
                            continue;
                        }
                        else
                            (*i)->next += (*i)->interval;
                    }

                    i++;
                }
            }

            if (!message)
            {
                if (!timers.empty())
                {
                    auto i = timers.begin();
                    std::chrono::steady_clock::time_point next_timeout = (*i)->next;
                    for (i++; i != timers.end(); i++)
                        next_timeout = std::min(next_timeout, (*i)->next);

                    message_added.wait_until(lock, next_timeout);
                    continue;
                }
                else
                {
                    message_added.wait(lock);
                    continue;
                }
            }
        }

        if (message) message();
    }

    return exitcode;
}

void messageloop::stop(int exitcode)
{
    std::lock_guard<std::mutex> _(mutex);

    this->exitcode = exitcode;
    stopped = true;
    message_added.notify_one();
}

void messageloop::post(const std::function<void()> &message)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.push(message);
    message_added.notify_one();
}

void messageloop::send(const std::function<void()> &message)
{
    std::unique_lock<std::mutex> lock(mutex);

    bool finished = false;
    messages.push([this, &message, &finished]
    {
        if (message) message();

        std::lock_guard<std::mutex> _(mutex);
        finished = true;
        send_processed.notify_all();
    });

    message_added.notify_one();
    while (!finished) send_processed.wait(lock);
}

void messageloop::process_events(const std::chrono::milliseconds &duration)
{
    auto stop = clock.now() + duration;
    stopped = false;

    for (;;)
    {
        std::function<void()> message;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!stopped)
            {
                if (!messages.empty())
                {
                    message = messages.front();
                    messages.pop();
                }
                else if (message_added.wait_until(lock, stop) == std::cv_status::no_timeout)
                    continue;
                else
                    return;
            }
            else
                return;
        }

        if (message) message();
    }
}

void messageloop::abort()
{
    // Can't lock from a signal

    this->exitcode = exitcode;
    stopped = true;
    message_added.notify_one();
}

void messageloop::timer_add(class timer &timer)
{
    timers.insert(&timer);
}

void messageloop::timer_remove(class timer &timer)
{
    timers.erase(&timer);
}


timer::timer(class messageloop &messageloop, const std::function<void()> &timeout)
    : messageloop(messageloop),
      timeout(timeout),
      interval(0),
      once(false)
{
}

timer::~timer()
{
    messageloop.timer_remove(*this);
}

void timer::start(std::chrono::nanoseconds interval, bool once)
{
    this->interval = interval;
    this->next = messageloop.clock.now() + interval;
    this->once = once;

    messageloop.timer_add(*this);
}

void timer::stop()
{
    messageloop.timer_remove(*this);
}

static std::map<messageloop *, void(messageloop::*)()> abort_handlers;

#if defined(__unix__)
#include <iostream>
#include <signal.h>

static void signal_handler(int signal)
{
    std::clog << "Received \"" << strsignal(signal) << "\" signal" << std::endl;

    if ((signal == SIGINT) || (signal == SIGTERM))
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
}

static void set_abort_handler(void(messageloop::*handler)(), messageloop *object)
{
    if (abort_handlers.empty())
    {
        struct sigaction act;
        act.sa_handler = &signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        sigaction(SIGHUP, &act, nullptr);
        sigaction(SIGTERM, &act, nullptr);
        sigaction(SIGINT, &act, nullptr);
    }

    abort_handlers[object] = handler;
}

static void clear_abort_handler(messageloop *object)
{
    abort_handlers.erase(object);

    if (abort_handlers.empty())
    {
        sigaction(SIGHUP, nullptr, nullptr);
        sigaction(SIGTERM, nullptr, nullptr);
        sigaction(SIGINT, nullptr, nullptr);
    }
}
#elif defined(WIN32)
#include <iostream>
#include <windows.h>

static BOOL WINAPI console_ctrl_handler(DWORD type)
{
    switch (type)
    {
    case CTRL_C_EVENT:
        std::clog << "Received CTRL_C" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_BREAK_EVENT:
        std::clog << "Received CTRL_BREAK" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_LOGOFF_EVENT:
        std::clog << "Received CTRL_LOGOFF" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_SHUTDOWN_EVENT:
        std::clog << "Received CTRL_SHUTDOWN" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    default:
        return FALSE;
    }
}

static void set_abort_handler(void(messageloop::*handler)(), messageloop *object)
{
    if (abort_handlers.empty())
        SetConsoleCtrlHandler(&console_ctrl_handler, TRUE);

    abort_handlers[object] = handler;
}

static void clear_abort_handler(messageloop *object)
{
    abort_handlers.erase(object);

    if (abort_handlers.empty())
        SetConsoleCtrlHandler(&console_ctrl_handler, FALSE);
}
#endif

