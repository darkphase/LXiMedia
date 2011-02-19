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

#include "sinterval.h"
#include <QtCore>

namespace LXiStream {

/*! Multiplies the interval by m. For example 1/4 times 2 becomes 1/2 and 1/4
    times 3 becomes 3/4.
 */
SInterval SInterval::operator*(int m) const
{
  if ((d.den % m) == 0)
    return SInterval(d.num, d.den / m);
  else
    return SInterval(d.num * m, d.den);
}

/*! Divides the interval by v. For example 1/2 divided by 2 becomes 1/4 and 3/4
    divided by 3 becomes 1/4.
 */
SInterval SInterval::operator/(int v) const
{
  if ((d.num % v) == 0)
    return SInterval(d.num / v, d.den);
  else
    return SInterval(d.num, d.den * v);
}

/*! Simplifies the fraction by finding the greatest common divider. For example
    100/400 becomes 1/4.
 */
SInterval SInterval::simplified(void) const
{
  struct T
  {
    static qint32 gcd(qint32 a, qint32 b)
    {
      if (b != 0)
        return gcd(b, a % b);
      else
        return a;
    }
  };

  const qint32 gcd = T::gcd(qAbs(d.num), d.den);
  return SInterval(d.num / gcd, d.den / gcd);
}

SInterval SInterval::fromFrequency(float hz)
{
  if (qFuzzyCompare(hz, float(qint32(hz))))
    return SInterval(1, qint32(hz));
  else
    return SInterval(1000, qint32(hz * 1000.0f)).simplified();
}

SInterval SInterval::fromFrequency(double hz)
{
  if (qFuzzyCompare(hz, double(qint32(hz))))
    return SInterval(1, qint32(hz));
  else
    return SInterval(1000, qint32(hz * 1000.0)).simplified();
}

qint64 SInterval::comp(const SInterval &a, const SInterval &b)
{
  const bool av = a.isValid(), bv = b.isValid();
  if (!av && !bv) // Both not valid
    return 0;
  else if (av != bv) // One of both not valid
    return av ? 1 : -1;
  else if ((a.d.num == b.d.num) && (a.d.den == b.d.den))
    return 0;
  else
    return (a.d.num * b.d.den) - (b.d.num * a.d.den);
}

} // End of namespace
