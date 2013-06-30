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

#include "ssize.h"
#include <QtCore>

namespace LXiStream {

SSize::SSize(void)
{
  Q_ASSERT(sizeof(*this) == sizeof(d));

  d.width = 0;
  d.height = 0;
  d.aspectRatio = 1.0f;
}

SSize::SSize(const SSize &c)
{
  Q_ASSERT(sizeof(*this) == sizeof(d));

  d.width = c.d.width;
  d.height = c.d.height;
  d.aspectRatio = c.d.aspectRatio;
}

SSize::SSize(const QSize &c, float aspectRatio)
{
  Q_ASSERT(sizeof(*this) == sizeof(d));

  d.width = c.width();
  d.height = c.height();
  d.aspectRatio = aspectRatio;
}

SSize::SSize(int width, int height, float aspectRatio)
{
  Q_ASSERT(sizeof(SSize) == sizeof(d));

  d.width = width;
  d.height = height;
  d.aspectRatio = aspectRatio;
}

bool SSize::isValid(void) const
{
  return
      (d.width > 0) &&
      (d.height > 0) &&
      (d.aspectRatio > 0.0f);
}

bool SSize::isNull(void) const
{
  return
      (d.width == 0) ||
      (d.height == 0) ||
      qFuzzyCompare(d.aspectRatio, 0.0f);
}

SSize & SSize::operator=(const SSize &c)
{
  d.width = c.d.width;
  d.height = c.d.height;
  d.aspectRatio = c.d.aspectRatio;

  return *this;
}

bool SSize::operator==(const SSize &c) const
{
  return
      (d.width == c.d.width) &&
      (d.height == c.d.height) &&
      (d.aspectRatio == c.d.aspectRatio);
}

int SSize::absoluteWidth(void) const
{
  return
      (d.aspectRatio > 1.0f)
      ? int((d.width * d.aspectRatio) + 0.5f)
      : d.width;
}

int SSize::absoluteHeight(void) const
{
  return
      (d.aspectRatio < 1.0f)
      ? int((d.height * (1.0f / d.aspectRatio)) + 0.5f)
      : d.height;
}

QString SSize::toString(void) const
{
  QString result = QString::number(d.width) + 'x' + QString::number(d.height);
  if (!qFuzzyCompare(d.aspectRatio, 1.0f))
    result += 'x' + QString::number(d.aspectRatio);

  return result;
}

SSize SSize::fromString(const QString &str)
{
  const QStringList l = str.split('x');
  return SSize(
      l.count() >= 1 ? l[0].trimmed().toInt() : 0,
      l.count() >= 2 ? l[1].trimmed().toInt() : (l.count() >= 1 ? l[0].trimmed().toInt() : 0),
      l.count() >= 3 ? l[2].trimmed().toFloat() : 1.0f);
}

SSize SSize::scaled(int nw, int nh) const
{
  return SSize(
      nw, nh,
      (float(absoluteWidth() * nh) / float(absoluteHeight())) / float(nw));
}

} // End of namespace
