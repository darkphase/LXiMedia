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

namespace platform {

using std::chrono::duration_cast;

void messageloop::abort(class messageloop_ref &ref)
{
    std::lock_guard<std::mutex> _(mutex);

    for (auto i = messages.begin(); i != messages.end(); )
        if (i->ref == &ref)
            i = messages.erase(i);
        else
            i++;
}

int messageloop::run()
{
    return process_events(std::chrono::milliseconds(-1));
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

int messageloop_ref::process_events(const std::chrono::milliseconds &duration)
{
    return messageloop.process_events(duration);
}

void timer::single_shot(
        class messageloop &messageloop,
        std::chrono::nanoseconds interval,
        const std::function<void()> &timeout)
{
    auto timer = std::make_shared<class timer>(messageloop, nullptr);
    const_cast<std::function<void()> &>(timer->timeout) = [timer, timeout]
    {
        timeout();
    };

    timer->start(interval, true);
}

void timer::single_shot(
        class messageloop_ref &messageloop,
        std::chrono::nanoseconds interval,
        const std::function<void()> &timeout)
{
    auto timer = std::make_shared<class timer>(messageloop, nullptr);
    const_cast<std::function<void()> &>(timer->timeout) = [timer, timeout]
    {
        timeout();
    };

    timer->start(interval, true);
}

timer::timer(
        class messageloop &messageloop,
        const std::function<void()> &timeout)
    : messageloop(messageloop),
      timeout(timeout),
      once(false)
{
}

timer::timer(
        class messageloop_ref &messageloop,
        const std::function<void()> &timeout)
    : timer(messageloop.messageloop, timeout)
{
}

timer::~timer()
{
    messageloop.timer_remove(*this);
}

void timer::start(std::chrono::nanoseconds interval, bool once)
{
    this->once = once;

    messageloop.timer_add(*this, interval);
}

void timer::stop()
{
    messageloop.timer_remove(*this);
}

} // End of namespace

#if defined(__unix__) || defined(__APPLE__)
#include <signal.h>

namespace platform {

static messageloop * main_instance = nullptr;

messageloop::messageloop()
    : exitcode(0),
      stopped(false)
{
    if (main_instance == nullptr)
    {
        main_instance = this;

        struct T
        {
            static void signal_handler(int signal)
            {
                if ((signal == SIGINT) || (signal == SIGTERM))
                {
                    // Can't lock from a signal
                    main_instance->stopped = true;
                    main_instance->message_added.notify_one();
                }
            }
        };

        struct sigaction act;
        act.sa_handler = &T::signal_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        sigaction(SIGHUP, &act, nullptr);
        sigaction(SIGTERM, &act, nullptr);
        sigaction(SIGINT, &act, nullptr);
    }
}

messageloop::~messageloop()
{
    assert(timers.empty());

    if (main_instance == this)
    {
        main_instance = nullptr;

        sigaction(SIGHUP, nullptr, nullptr);
        sigaction(SIGTERM, nullptr, nullptr);
        sigaction(SIGINT, nullptr, nullptr);
    }
}

void messageloop::post(
        class messageloop_ref &ref,
        const std::function<void()> &func)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.push_back(message { &ref, func });
    message_added.notify_one();
}

void messageloop::post(
        class messageloop_ref &ref,
        std::function<void()> &&func)
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

void messageloop::stop(int exitcode)
{
    std::lock_guard<std::mutex> _(mutex);

    this->exitcode = exitcode;
    this->stopped = true;
    message_added.notify_one();
}

void messageloop::timer_add(class timer &timer, std::chrono::nanoseconds iv)
{
    std::lock_guard<std::mutex> _(mutex);

    timer.interval = iv;
    timer.next = clock.now() + timer.interval;

    timers.insert(&timer);
}

void messageloop::timer_remove(class timer &timer)
{
    std::lock_guard<std::mutex> _(mutex);

    timers.erase(&timer);
}

int messageloop::process_events(std::chrono::milliseconds duration)
{
    stopped = false;

    auto start = clock.now();
    for (;;)
    {
        std::unique_lock<std::mutex> lock(mutex);

        struct message message;
        if (stopped)
        {
            return exitcode;
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
                        timers.erase(*i);
                    else
                        (*i)->next += (*i)->interval;

                    break;
                }

                i++;
            }
        }

        if (!message.func)
        {
            std::chrono::steady_clock::time_point next_timeout;
            if (duration.count() >= 0)
            {
                next_timeout = start + duration;
                if (next_timeout < clock.now())
                    return -1;
            }

            if (!timers.empty())
            {
                auto i = timers.begin();

                if (duration.count() >= 0)
                    next_timeout = std::min(next_timeout, (*i)->next);
                else
                    next_timeout = (*i)->next;

                for (i++; i != timers.end(); i++)
                    next_timeout = std::min(next_timeout, (*i)->next);

                message_added.wait_until(lock, next_timeout);
                continue;
            }
            else if (duration.count() >= 0)
            {
                message_added.wait_until(lock, next_timeout);
                continue;
            }
            else
            {
                message_added.wait(lock);
                continue;
            }
        }

        lock.unlock();
        if (message.func)
            message.func();
    }
}

} // End of namespace

#elif defined(WIN32)
#include <windows.h>

namespace platform {

static int num_instances = 0;
static const wchar_t class_name[] = L"messageloop";

messageloop::messageloop()
    : exitcode(0),
      window_handle(nullptr)
{
    struct T
    {
        static LRESULT WINAPI WndProc(
                    HWND window_handle, UINT msg,
                    WPARAM wp, LPARAM lp)
        {
            if (msg == WM_CREATE)
            {
                ::SetWindowLongPtr(
                            window_handle, GWLP_USERDATA,
                            LONG_PTR(reinterpret_cast<LPCREATESTRUCT>(
                                         lp)->lpCreateParams));
            }
            else
            {
                messageloop * const me = reinterpret_cast<messageloop *>(
                            ::GetWindowLongPtr(window_handle, GWLP_USERDATA));

                if (me)
                {
                    std::unique_lock<std::mutex> lock(me->mutex);

                    if (msg == WM_USER)
                    {
                        struct message message;
                        if (!me->messages.empty())
                        {
                            message = me->messages.front();
                            me->messages.pop_front();
                        }

                        lock.unlock();
                        if (message.func)
                            message.func();
                    }
                    else if (msg == WM_TIMER)
                    {
                        class timer * const timer =
                                reinterpret_cast<class timer *>(wp);

                        if (me->timers.find(timer) != me->timers.end())
                        {
                            lock.unlock();

                            if (timer->once)
                                me->timer_remove(*timer);

                            if (timer->timeout)
                                timer->timeout();
                        }
                    }
                    else if (msg == WM_CLOSE)
                    {
                        ::PostQuitMessage(me->exitcode);
                    }
                }
            }

            return ::DefWindowProc(window_handle, msg, wp, lp);
        }
    };

    if (num_instances++ == 0)
    {
        WNDCLASSEXW wc;
        memset(&wc, 0, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &T::WndProc;
        wc.hInstance = ::GetModuleHandle(NULL);
        wc.lpszClassName = class_name;
        ::RegisterClassEx(&wc);
    }

    window_handle = ::CreateWindowEx(
        0, class_name, NULL,
        0, 0, 0, 0, 0,
        HWND_MESSAGE, NULL,
        ::GetModuleHandle(NULL),
        this);
}

messageloop::~messageloop()
{
    assert(timers.empty());

    ::DestroyWindow(HWND(window_handle));

    if (--num_instances == 0)
        ::UnregisterClass(class_name, ::GetModuleHandle(NULL));
}

void messageloop::post(
        class messageloop_ref &ref,
        const std::function<void()> &func)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.push_back(message { &ref, func });
    ::PostMessage(HWND(window_handle), WM_USER, 0, 0);
}

void messageloop::post(
        class messageloop_ref &ref,
        std::function<void()> &&func)
{
    std::lock_guard<std::mutex> _(mutex);

    messages.emplace_back(message { &ref, std::move(func) });
    ::PostMessage(HWND(window_handle), WM_USER, 0, 0);
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

    ::PostMessage(HWND(window_handle), WM_USER, 0, 0);
    while (!finished) send_processed.wait(lock);
}

void messageloop::stop(int exitcode)
{
    std::lock_guard<std::mutex> _(mutex);

    this->exitcode = exitcode;
    ::PostMessage(HWND(window_handle), WM_CLOSE, 0, 0);
}

void messageloop::timer_add(class timer &timer, std::chrono::nanoseconds iv)
{
    std::lock_guard<std::mutex> _(mutex);

    const auto elapse =
            UINT(std::max(
                     int64_t(USER_TIMER_MINIMUM),
                     std::min(
                         duration_cast<std::chrono::milliseconds>(iv).count(),
                         int64_t(USER_TIMER_MAXIMUM))));

    if (::SetTimer(HWND(window_handle), UINT_PTR(&timer), elapse, NULL) != 0)
        timers.insert(&timer);
}

void messageloop::timer_remove(class timer &timer)
{
    std::lock_guard<std::mutex> _(mutex);

    auto i = timers.find(&timer);
    if (i != timers.end())
    {
        ::KillTimer(HWND(window_handle), UINT_PTR(&timer));
        timers.erase(i);
    }
}

int messageloop::process_events(std::chrono::milliseconds duration)
{
    auto start = clock.now();
    for (;;)
    {
        MSG msg = {0};
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return int(msg.wParam);

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        if (duration.count() >= 0)
        {
            auto remain = duration - duration_cast<std::chrono::milliseconds>(
                    clock.now() - start);

            if ((remain.count() < 0) ||
                (::MsgWaitForMultipleObjects(
                     0, NULL, FALSE,
                     DWORD(remain.count()),
                     QS_ALLEVENTS) == WAIT_TIMEOUT))
            {
                return -1;
            }
        }
        else
            ::MsgWaitForMultipleObjects(0, NULL, FALSE, INFINITE, QS_ALLEVENTS);
    }
}

} // End of namespace

#endif
