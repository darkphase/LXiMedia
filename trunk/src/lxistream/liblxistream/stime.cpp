/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#include "stime.h"
#include <QtCore>

namespace LXiStream {

const STime STime::null = STime(0, SInterval(1, 1));

qint64 STime::comp(const STime &a, const STime &b)
{
  const bool av = a.isValid(), bv = b.isValid();
  if (!av && !bv) // Both not valid
  {
    return 0;
  }
  else if (av != bv) // One of both not valid
  {
    return av ? 1 : -1;
  }
  else if (a.d.interval == b.d.interval)
  {
    return a.d.count - b.d.count;
  }
  else
  {
    return
        (a.d.count * a.d.interval.num() * b.d.interval.den()) -
        (b.d.count * b.d.interval.num() * a.d.interval.den());
  }
}

STime STime::add(const STime &a, const STime &b)
{
  if (a.d.interval == b.d.interval)
  {
    return STime(a.d.count + b.d.count, a.d.interval);
  }
  else if (a.d.interval.den() == b.d.interval.den())
  {
    return STime((a.d.count * a.d.interval.num()) + (b.d.count * b.d.interval.num()),
                 SInterval(1, a.d.interval.den()));
  }
  else if (a.d.interval.isValid() && b.d.interval.isValid())
  {
    const qint64 num =
        (a.d.count * qint64(a.d.interval.num()) * qint64(b.d.interval.den())) +
        (b.d.count * qint64(b.d.interval.num()) * qint64(a.d.interval.den()));
    const qint64 den =
        qint64(a.d.interval.den()) * qint64(b.d.interval.den());

    if ((num % b.d.interval.den()) == 0)
    {
      return STime(num / b.d.interval.den(), SInterval(1, a.d.interval.den()));
    }
    else if ((num % a.d.interval.den()) == 0)
    {
      return STime(num / a.d.interval.den(), SInterval(1, b.d.interval.den()));
    }
    else
    {
      struct T
      {
        static qint64 gcd(qint64 a, qint64 b)
        {
          if (b != 0)
            return gcd(b, a % b);
          else
            return a;
        }
      };

      const qint64 gcd = T::gcd(qAbs(num), den);
      return STime(num / gcd, SInterval(1, den / gcd));
    }
  }
  else
    return a.d.interval.isValid() ? a : b;
}

STime STime::div(const STime &t, int d)
{
  if ((t.d.count % d) == 0)
    return STime(t.d.count / d, t.d.interval);
  else
    return STime(t.d.count, t.d.interval / d);
}

} // End of namespace
