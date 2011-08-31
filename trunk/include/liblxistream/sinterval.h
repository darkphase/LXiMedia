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

#ifndef LXSTREAM_SINTERVAL_H
#define LXSTREAM_SINTERVAL_H

#include <QtGlobal>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

/*! Represents an interval between two events. The interval is stored as a
    fraction of one second: 1/4 = 250ms, 1/25 = 40 ms, 1/30 = 33.333333... ms.
    An SInterval can be created by manually specifying a numerator and
    denomerator or by invoking the static method fromFrequency().

    \note This class can be serialized.
    \sa STime
 */
class LXISTREAM_PUBLIC SInterval
{
public:
  inline                        SInterval(void)                                 { Q_ASSERT(sizeof(*this) == sizeof(d)); d.num = d.den = 0; }
  inline                        SInterval(qint64 num, qint64 den)               { Q_ASSERT(sizeof(*this) == sizeof(d)); d.num = num; d.den = den; }
  inline                        SInterval(const SInterval &from)                { Q_ASSERT(sizeof(*this) == sizeof(d)); d.num = from.d.num; d.den = from.d.den; }

  inline bool                   isValid(void) const                             { return (d.num > 0) && (d.den > 0); }

  inline SInterval            & operator=(const SInterval &from)                { d.num = from.d.num; d.den = from.d.den; return *this; }
  inline bool                   operator==(const SInterval &c) const            { return comp(*this, c) == 0; }
  inline bool                   operator!=(const SInterval &c) const            { return comp(*this, c) != 0; }
  inline bool                   operator> (const SInterval &c) const            { return comp(*this, c) > 0; }
  inline bool                   operator>=(const SInterval &c) const            { return comp(*this, c) >= 0; }
  inline bool                   operator< (const SInterval &c) const            { return comp(*this, c) < 0; }
  inline bool                   operator<=(const SInterval &c) const            { return comp(*this, c) <= 0; }

  inline SInterval              operator+(const SInterval &f) const             { return SInterval((d.num * f.d.den) + (f.d.num * d.den), d.den * f.d.den); }
  inline SInterval              operator-(const SInterval &f) const             { return SInterval((d.num * f.d.den) - (f.d.num * d.den), d.den * f.d.den); }
  inline SInterval              operator*(const SInterval &f) const             { return SInterval(d.num * f.d.num, d.den * f.d.den); }
  SInterval                     operator*(int m) const;
  SInterval                     operator/(int d) const;

  inline SInterval            & operator+=(const SInterval &f)                  { return *this = *this + f; }
  inline SInterval            & operator-=(const SInterval &f)                  { return *this = *this - f; }
  inline SInterval            & operator*=(const SInterval &f)                  { return *this = *this * f; }
  inline SInterval            & operator*=(int m)                               { return *this = *this * m; }
  inline SInterval            & operator/=(int d)                               { return *this = *this / d; }

  inline qint64                 num(void) const                                 { return d.num; }
  inline qint64                 den(void) const                                 { return d.den; }

  SInterval                     simplified(void) const;

  inline double                 toTime(void) const                              { return isValid() ? (double(d.num) / double(d.den)) : 0.0; }
  inline int                    toTime(int count) const                         { return isValid() ? (count * d.num / d.den) : 0; }
  inline qint64                 toTime(qint64 count) const                      { return isValid() ? (count * d.num / d.den) : Q_INT64_C(0); }
  inline float                  toTime(float count) const                       { return isValid() ? (count * float(d.num) / float(d.den)) : 0.0f; }
  inline double                 toTime(double count) const                      { return isValid() ? (count * double(d.num) / double(d.den)) : 0.0; }
  inline double                 toFrequency(void) const                         { return isValid() ? (1.0 / toTime()) : 0.0; }

  static inline SInterval       fromFrequency(int hz)                           { return SInterval(1, hz); }
  static SInterval              fromFrequency(float hz);
  static SInterval              fromFrequency(double hz);
  static inline SInterval       ntscFrequency(int hz)                           { return SInterval(1001, hz * 1000); }

private:
  static qint64                 comp(const SInterval &, const SInterval &);

private:
  // Ensure all these struct members are serializable.
  struct
  {
    qint64                      num, den;
  }                             d;
};

} // End of namespace

#endif
