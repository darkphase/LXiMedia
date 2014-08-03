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

messageloop::messageloop()
  : running(false),
    exitcode(-1)
{
}

messageloop::~messageloop()
{
  assert(timers.empty());
}

int messageloop::run()
{
  running = true;

  for (;;)
  {
    std::function<void()> message;
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!running)
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

  exitcode = exitcode;
  running = false;
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

  for (;;)
  {
    std::function<void()> message;
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!messages.empty())
      {
        message = messages.front();
        messages.pop();
      }
      else
      {
        if (message_added.wait_until(lock, stop) == std::cv_status::no_timeout)
          continue;
        else
          return;
      }
    }

    if (message) message();
  }
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
