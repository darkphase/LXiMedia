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

#include "stimer.h"
#include <QtCore>
#if defined(Q_OS_UNIX)
#include <sys/time.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace LXiStream {

class STimer::Init : public SApplication::Initializer
{
public:
  virtual void                  startup(void);
  virtual void                  shutdown(void);

public:
  static qint64                 baseTimestamp;

private:
  static Init                   self;
};

qint64       STimer::Init::baseTimestamp = 0;
STimer::Init STimer::Init::self;

struct STimer::Data
{
  QAtomicInt                    ref;
  STime                         localOffset;
  STime                         pausedTimeStamp;

#if defined(Q_OS_WIN)
  qint64                        timerFreq;
#endif
};

struct STimer::IntervalData
{
  STime                         sourceTime;
  STime                         lastTime;
  STime                         avgInterval;
};


STimer::STimer(void)
       :d(new Data()),
        id(NULL)
{
  d->ref = 1;
  d->localOffset = STime::null;
  d->pausedTimeStamp = STime();

#if defined(Q_OS_WIN)
  LARGE_INTEGER freq;
  if (::QueryPerformanceFrequency(&freq))
    d->timerFreq = freq.QuadPart;
  else
    d->timerFreq = 0;
#endif
}

STimer::~STimer(void)
{
  if (!d->ref.deref())
    delete d;

  delete id;
}

void STimer::sync(STimer &other)
{
  if (&other != this)
  {
    if (!d->ref.deref())
      delete d;

    d = other.d;
    d->ref.ref();
  }
}

/*! Returns the current time.
 */
STime STimer::timeStamp(void) const
{
  if (!d->pausedTimeStamp.isValid())
    return absoluteTimeStamp() + d->localOffset;
  else
    return d->pausedTimeStamp;
}

/*! Sets the current time to the specified timestamp. If the timer was paused,
    it will resume again.
 */
void STimer::setTimeStamp(STime ts)
{
  d->localOffset = ts - absoluteTimeStamp();
  d->pausedTimeStamp = STime();
}

/*! Resets the timer, i.e. sets the timestamp to 0. If the timer was paused,
    it will resume again.
 */
void STimer::reset(void)
{
  setTimeStamp(STime::null);
}

/*! Returns the current time. If the timer was paused, it may resume again. If
    the offset is latger than maxOffset, the current time is corrected to the
    specified timestamp. The timestamp is not corrected if the specified
    timestamp is 0.
 */
STime STimer::correctTimeStamp(STime ts, STime maxOffset)
{
  if (!ts.isNull())
  {
    const STime now = timeStamp();
    const STime offset = ts - now;

    if (qAbs(offset) <= maxOffset)
    {
      return now;
    }
    else
    {
      setTimeStamp(ts);
      return ts;
    }
  }

  return timeStamp();
}

/*! Returns the offset from the current time for the specified timestamp. A
    positve offset means the specified timestamp is in the future.
 */
STime STimer::offset(STime ts) const
{
  if (!ts.isNull())
    return ts - timeStamp();

  return STime::null;
}

/*! Returns the offset from the current time for the specified timestamp. If
    the offset is larger than maxOffset, the current time is corrected to the
    specified timestamp. A positve offset means the specified timestamp is in
    the future.
 */
STime STimer::correctOffset(STime ts, STime maxOffset)
{
  if (ts.isValid())
  {
    const STime offset = ts - timeStamp();

    if (qAbs(offset) <= maxOffset)
      return offset;
    else
      setTimeStamp(ts);
  }

  return STime::null;
}

/*! Pauses the timer; if true, the timer will continue to return the same
    timestamp until set to flase again.
 */
void STimer::pause(bool p)
{
  if (p)
    d->pausedTimeStamp = timeStamp();
  else
    setTimeStamp(d->pausedTimeStamp);
}

/*! Returns the current time with jitter smoothened. This method detects the
    interval in which it is invoked and smoothens out the jitter. For example,
    if this method is invoked at timestamps: 0 ms, 37 ms, 88 ms, 108 ms and 166
    ms (notice the interval of roughly 40 ms here); the smoothTimeStamp will
    return the following timestamps: 0 ms, 40 ms, 80 ms, 120 ms and 160 ms. An
    interval can be proveded, but one is estimated if it is left at 0.

    \note This method works best for intervals of 10 - 1000 ms with jitter less
          than 100 ms.
 */
STime STimer::smoothTimeStamp(STime refInterval, STime delay)
{
  if (id != NULL)
  {
    const STime currentTime = timeStamp() - delay;
    const STime interval = refInterval.isValid() ? refInterval : id->avgInterval;
    id->sourceTime += interval;

    if (qAbs(currentTime - id->sourceTime) < (interval * 4))
    {
      // Compensate for a small drift.
      if (qAbs(currentTime - id->sourceTime) >= STime::fromMSec(10))
        id->sourceTime += STime::fromUSec((currentTime - id->sourceTime).toUSec() >> 3);

      if (id->avgInterval.isPositive())
      {
        id->avgInterval = STime::fromUSec(((id->avgInterval.toUSec() << 2) +
                                           (id->avgInterval.toUSec() * 3) +
                                           (currentTime - id->lastTime).toUSec()) >> 3);
      }
      else
      {
        id->avgInterval = currentTime - id->lastTime;
      }
    }
    else // We lost some time somewhere, resync
      id->sourceTime = currentTime;

    id->lastTime = currentTime;
  }
  else
  {
    id = new IntervalData();
    id->sourceTime = timeStamp() - delay;
    id->lastTime = id->sourceTime;
    id->avgInterval = refInterval.isValid() ? refInterval : STime::null;
  }

  return id->sourceTime;
}

/*! Returns the time since the creation of the SApplication instance.
 */
STime STimer::absoluteTimeStamp(void) const
{
#if defined(Q_OS_UNIX)
  struct timeval now;
  gettimeofday(&now, NULL);

  return STime::fromSec(now.tv_sec - Init::baseTimestamp) + STime::fromUSec(now.tv_usec);
#elif defined(Q_OS_WIN)
  LARGE_INTEGER count;
  if ((d->timerFreq > 0) && ::QueryPerformanceCounter(&count))
    return STime::fromUSec(((count.QuadPart - Init::baseTimestamp) * Q_INT64_C(1000000)) / d->timerFreq);
  else
    return STime::fromUSec(0);
#endif
}

void STimer::Init::startup(void)
{
#if defined(Q_OS_UNIX)
  struct timeval now;
  gettimeofday(&now, NULL);

  baseTimestamp = now.tv_sec;
#elif defined(Q_OS_WIN)
  LARGE_INTEGER count;
  if (::QueryPerformanceCounter(&count))
    baseTimestamp = count.QuadPart;
#endif
}

void STimer::Init::shutdown(void)
{
}

} // End of namespace
