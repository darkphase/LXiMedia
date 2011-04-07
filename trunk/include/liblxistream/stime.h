/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef LXSTREAM_STIME_H
#define LXSTREAM_STIME_H

#include <QtGlobal>
#include <LXiCore>
#include "sinterval.h"

namespace LXiStream {

/*! Represents a time in the form of n times an interval. For example 60 times
    1/30 represents 60 ticks of 1/30 of a second or two seconds. This class
    supports adding and comparing different times of different intervals, for
    example: (10 * 1/30) + (4 * 1/15) = (18 * 1/30).

    \sa SInterval
 */
class S_DSO_PUBLIC STime
{
public:
  inline                        STime(void)                                     { d.count = 0; }
  inline                        STime(const STime &from)                        { d.count = from.d.count; d.interval = from.d.interval; }
  inline                        STime(qint64 count, SInterval interval)         { d.count = count; d.interval = interval; }

  inline bool                   isValid(void) const                             { return d.interval.isValid(); }
  inline bool                   isNull(void) const                              { return isValid() && (d.count == 0); }
  inline bool                   isPositive(void) const                          { return isValid() && (d.count > 0); }
  inline bool                   isNegative(void) const                          { return isValid() && (d.count < 0); }

  inline STime                & operator=(const STime &from)                    { d.count = from.d.count; d.interval = from.d.interval; return *this; }

  inline bool                   operator==(const STime &c) const                { return comp(*this, c) == 0; }
  inline bool                   operator!=(const STime &c) const                { return comp(*this, c) != 0; }
  inline bool                   operator> (const STime &c) const                { return comp(*this, c) > 0; }
  inline bool                   operator>=(const STime &c) const                { return comp(*this, c) >= 0; }
  inline bool                   operator< (const STime &c) const                { return comp(*this, c) < 0; }
  inline bool                   operator<=(const STime &c) const                { return comp(*this, c) <= 0; }

  inline STime                  operator-(void) const                           { return STime(-d.count, d.interval); }
  inline STime                  operator+(const STime &t) const                 { return add(*this, t); }
  inline STime                  operator-(const STime &t) const                 { return add(*this, -t); }
  inline STime                  operator*(int m) const                          { return STime(d.count * m, d.interval); }
  inline STime                  operator/(int d) const                          { return div(*this, d); }

  inline STime                & operator+=(const STime &t)                      { return *this = *this + t; }
  inline STime                & operator-=(const STime &t)                      { return *this = *this - t; }
  inline STime                & operator*=(int m)                               { return *this = *this * m; }
  inline STime                & operator/=(int d)                               { return *this = *this / d; }

  inline qint64                 count(void) const                               { return d.count; }
  inline const SInterval      & interval(void) const                            { return d.interval; }

  inline int                    toHour(void) const                              { return toMin() / 60; }
  inline int                    toMin(void) const                               { return toSec() / 60; }
  inline int                    toSec(void) const                               { return d.interval.toTime(d.count); }
  inline qint64                 toMSec(void) const                              { return d.interval.toTime(d.count * 1000); }
  inline qint64                 toUSec(void) const                              { return d.interval.toTime(d.count * 1000000); }
  inline qint64                 toClock(SInterval i) const                      { return d.interval.toTime(d.count * i.den() / i.num()); }
  inline qint64                 toClock(int hz) const                           { return d.interval.toTime(d.count * hz); }
  inline qint64                 toClock(float hz) const                         { return d.interval.toTime(d.count * hz); }
  inline qint64                 toClock(double hz) const                        { return d.interval.toTime(d.count * hz); }
  inline qint64                 toClock(qint64 num, qint64 den) const           { return d.interval.toTime(d.count * den / num); }

  static inline STime           fromHour(int t)                                 { return fromMin(t * 60); }
  static inline STime           fromMin(int t)                                  { return fromSec(t * 60); }
  static inline STime           fromSec(int t)                                  { return STime(t, SInterval(1, 1)); }
  static inline STime           fromMSec(qint64 t)                              { return STime(t, SInterval(1, 1000)); }
  static inline STime           fromUSec(qint64 t)                              { return STime(t, SInterval(1, 1000000)); }
  static inline STime           fromClock(qint64 t, SInterval i)                { return STime(t, i); }
  static inline STime           fromClock(qint64 t, int hz)                     { return STime(t, SInterval(1, hz)); }
  static inline STime           fromClock(qint64 t, float hz)                   { return STime(t, SInterval::fromFrequency(hz)); }
  static inline STime           fromClock(qint64 t, double hz)                  { return STime(t, SInterval::fromFrequency(hz)); }
  static inline STime           fromClock(qint64 t, qint64 num, qint64 den)     { return STime(t, SInterval(num, den)); }

  static const STime            null;

private:
  static qint64                 comp(const STime &, const STime &);
  static STime                  add(const STime &, const STime &);
  static STime                  div(const STime &, int);

private:
  struct
  {
    qint64                      count;
    SInterval                   interval;
  }                             d;
};


} // End of namespace


QT_BEGIN_NAMESPACE

template <>
inline LXiStream::STime qAbs(const LXiStream::STime &t)
{
  return t.isPositive() ? t : -t;
}

QT_END_NAMESPACE


#endif
