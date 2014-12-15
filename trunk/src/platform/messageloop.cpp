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

static void set_abort_handler(void(platform::messageloop::*)(), platform::messageloop *);
static void clear_abort_handler(platform::messageloop *);

namespace platform {

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
        struct message message;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (stopped)
            {
                break;
            }
            else if (!messages.empty())
            {
                message = messages.front();
                messages.pop_front();
            }
            else if (!timers.empty())
            {
                auto now = clock.now();
                for (auto i = timers.begin(); i != timers.end(); )
                {
                    if ((*i)->next <= now)
                    {
                        message = messageloop::message { nullptr, (*i)->timeout };

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

            if (!message.func)
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

        if (message.func) message.func();
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

void messageloop::post(class messageloop_ref &ref, const std::function<void()> &func)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.push_back(message { &ref, func });
    message_added.notify_one();
}

void messageloop::post(class messageloop_ref &ref, std::function<void()> &&func)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.emplace_back(message { &ref, std::move(func) });
    message_added.notify_one();
}

void messageloop::send(const std::function<void()> &func)
{
    std::unique_lock<std::mutex> lock(mutex);

    bool finished = false;
    messages.push_back(message { nullptr, [this, &func, &finished]
    {
        if (func) func();

        std::lock_guard<std::mutex> _(mutex);
        finished = true;
        send_processed.notify_all();
    }});

    message_added.notify_one();
    while (!finished) send_processed.wait(lock);
}

void messageloop::process_events(const std::chrono::milliseconds &duration)
{
    auto stop = clock.now() + duration;
    stopped = false;

    for (;;)
    {
        struct message message;
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!stopped)
            {
                if (!messages.empty())
                {
                    message = messages.front();
                    messages.pop_front();
                }
                else if (message_added.wait_until(lock, stop) == std::cv_status::no_timeout)
                    continue;
                else
                    return;
            }
            else
                return;
        }

        if (message.func) message.func();
    }
}

void messageloop::abort(class messageloop_ref &ref)
{
    std::lock_guard<std::mutex> _(mutex);

    for (auto i = messages.begin(); i != messages.end(); )
        if (i->ref == &ref)
            i = messages.erase(i);
        else
            i++;
}

void messageloop::abort()
{
    // Can't lock from a signal

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


messageloop_ref::messageloop_ref(class messageloop &messageloop)
    : messageloop(messageloop)
{
}

messageloop_ref::messageloop_ref(class messageloop_ref &messageloop_ref)
    : messageloop(messageloop_ref.messageloop)
{
}

messageloop_ref::~messageloop_ref()
{
    messageloop.abort(*this);
}

void messageloop_ref::stop(int exitcode)
{
    return messageloop.stop(exitcode);
}

void messageloop_ref::send(const std::function<void()> &func)
{
    return messageloop.send(func);
}

void messageloop_ref::post(const std::function<void()> &func)
{
    return messageloop.post(*this, func);
}

void messageloop_ref::post(std::function<void()> &&func)
{
    return messageloop.post(*this, std::move(func));
}

void messageloop_ref::process_events(const std::chrono::milliseconds &duration)
{
    return messageloop.process_events(duration);
}

void timer::single_shot(
        class messageloop &messageloop,
        std::chrono::nanoseconds interval,
        const std::function<void()> &timeout)
{
    auto timer = std::make_shared<class timer>(messageloop, nullptr);
    const_cast<std::function<void()> &>(timer->timeout) = [timer, timeout] { timeout(); };
    timer->start(interval, true);
}

void timer::single_shot(
        class messageloop_ref &messageloop,
        std::chrono::nanoseconds interval,
        const std::function<void()> &timeout)
{
    auto timer = std::make_shared<class timer>(messageloop, nullptr);
    const_cast<std::function<void()> &>(timer->timeout) = [timer, timeout] { timeout(); };
    timer->start(interval, true);
}

timer::timer(class messageloop &messageloop, const std::function<void()> &timeout)
    : messageloop(messageloop),
      timeout(timeout),
      interval(0),
      once(false)
{
}

timer::timer(class messageloop_ref &messageloop, const std::function<void()> &timeout)
    : timer(messageloop.messageloop, timeout)
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

} // End of namespace

static std::map<platform::messageloop *, void(platform::messageloop::*)()> abort_handlers;

#if defined(__unix__) || defined(__APPLE__)
#include <iostream>
#include <signal.h>

static void signal_handler(int signal)
{
    std::clog << "platform::messageloop: received \"" << strsignal(signal) << "\" signal" << std::endl;

    if ((signal == SIGINT) || (signal == SIGTERM))
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
}

static void set_abort_handler(void(platform::messageloop::*handler)(), platform::messageloop *object)
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

static void clear_abort_handler(platform::messageloop *object)
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
        std::clog << "platform::messageloop: received CTRL_C" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_BREAK_EVENT:
        std::clog << "platform::messageloop: received CTRL_BREAK" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_LOGOFF_EVENT:
        std::clog << "platform::messageloop: received CTRL_LOGOFF" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    case CTRL_SHUTDOWN_EVENT:
        std::clog << "platform::messageloop: received CTRL_SHUTDOWN" << std::endl;
        for (auto &i : abort_handlers) ((i.first)->*(i.second))();
        return TRUE;

    default:
        return FALSE;
    }
}

static void set_abort_handler(void(platform::messageloop::*handler)(), platform::messageloop *object)
{
    if (abort_handlers.empty())
        SetConsoleCtrlHandler(&console_ctrl_handler, TRUE);

    abort_handlers[object] = handler;
}

static void clear_abort_handler(platform::messageloop *object)
{
    abort_handlers.erase(object);

    if (abort_handlers.empty())
        SetConsoleCtrlHandler(&console_ctrl_handler, FALSE);
}
#endif

