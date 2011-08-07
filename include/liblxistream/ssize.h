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

#ifndef LXSTREAM_SSIZE_H
#define LXSTREAM_SSIZE_H

#include <QtCore>
#include <QtGlobal>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class SSize; // For QtCreator autocompletion.

/*! The SSize object defines a two dimensional size with a floating point aspect
    ratio. It can be used to describe a video size, for example 720x576x1.4222
    meaning a 16:9 image at a resoltion of 720x576 pixels at an absolute size of
    1024x576.

    \note All methods have been inlined and there is no penalty of invoking them
          in loops over first copying them to a local variable, with the
          exception of the absolute* methods that do a floating point
          computation.
    \note This class can be serialized.
 */
class LXISTREAM_PUBLIC SSize
{
public:
  inline                        SSize(void)                                     { Q_ASSERT(sizeof(*this) == sizeof(d)); d.width = 0; d.height = 0; d.aspectRatio = 1.0f; }
  inline                        SSize(const SSize &c)                           { Q_ASSERT(sizeof(*this) == sizeof(d)); d.width = c.d.width; d.height = c.d.height; d.aspectRatio = c.d.aspectRatio; }
  inline                        SSize(const QSize &c, float aspectRatio = 1.0f) { Q_ASSERT(sizeof(*this) == sizeof(d)); d.width = c.width(); d.height = c.height(); d.aspectRatio = aspectRatio; }
  inline                        SSize(int width, int height, float aspectRatio = 1.0f) { Q_ASSERT(sizeof(SSize) == sizeof(d)); d.width = width; d.height = height; d.aspectRatio = aspectRatio; }

  inline bool                   isValid(void) const                             { return (d.width > 0) && (d.height > 0) && (d.aspectRatio > 0.0f); }
  inline bool                   isNull(void) const                              { return (d.width == 0) || (d.height == 0) || qFuzzyCompare(d.aspectRatio, 0.0f); }

  inline SSize                & operator=(const SSize &c)                       { d.width = c.d.width; d.height = c.d.height; d.aspectRatio = c.d.aspectRatio; return *this; }
  inline bool                   operator==(const SSize &c) const                { return (d.width == c.d.width) && (d.height == c.d.height) && (d.aspectRatio == c.d.aspectRatio); }
  inline bool                   operator!=(const SSize &c) const                { return !operator==(c); }
  inline bool                   operator<(const SSize &c) const                 { return (absoluteWidth() * absoluteHeight()) <  (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator<=(const SSize &c) const                { return (absoluteWidth() * absoluteHeight()) <= (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator>(const SSize &c) const                 { return (absoluteWidth() * absoluteHeight()) >  (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator>=(const SSize &c) const                { return (absoluteWidth() * absoluteHeight()) >= (c.absoluteWidth() * c.absoluteHeight()); }

  inline int                    width(void) const                               { return d.width; }
  inline void                   setWidth(int nw)                                { d.width = nw; }
  inline int                    height(void) const                              { return d.height; }
  inline void                   setHeight(int nh)                               { d.height = nh; }
  inline float                  aspectRatio(void) const                         { return d.aspectRatio; }
  inline void                   setAspectRatio(float na)                        { d.aspectRatio = na; }
  inline QSize                  size(void) const                                { return QSize(d.width, d.height); }

  inline int                    absoluteWidth(void) const                       { return (d.aspectRatio > 1.0f) ? int(d.width * d.aspectRatio) : d.width; }
  inline int                    absoluteHeight(void) const                      { return (d.aspectRatio < 1.0f) ? int(d.height * (1.0f / d.aspectRatio)) : d.height; }
  inline QSize                  absoluteSize(void) const                        { return QSize(absoluteWidth(), absoluteHeight()); }

  inline QString                toString(void) const                            { return QString::number(d.width) + 'x' + QString::number(d.height) + 'x' + QString::number(d.aspectRatio); }
  inline static SSize           fromString(const QString &str)                  { const QStringList l = str.split('x'); return SSize(l.count() >= 1 ? l[0].toInt() : 0, l.count() >= 2 ? l[1].toInt() : 0, l.count() >= 3 ? l[2].toFloat() : 1.0f); }

private:
  // Ensure all these struct members are serializable.
  struct
  {
    int                         width, height;
    float                       aspectRatio;
  }                             d;
};


} // End of namespace


#endif
