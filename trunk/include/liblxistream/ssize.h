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
 */
class SSize
{
public:
  inline                        SSize(void) : w(0), h(0), a(1.0f)               { }
  inline                        SSize(const SSize &c) : w(c.w), h(c.h), a(c.a)  { }
  inline                        SSize(const QSize &c, float a = 1.0f) : w(c.width()), h(c.height()), a(a)  { }
  inline                        SSize(int w, int h, float a = 1.0f) : w(w), h(h), a(a) { }

  inline bool                   isValid(void) const                             { return (w > 0) && (h > 0) && (a > 0.0f); }
  inline bool                   isNull(void) const                              { return (w == 0) || (h == 0) || qFuzzyCompare(a, 0.0f); }

  inline SSize                & operator=(const SSize &c)                       { w = c.w; h = c.h; a = c.a; return *this; }
  inline bool                   operator==(const SSize &c) const                { return (w == c.w) && (h == c.h) && (a == c.a); }
  inline bool                   operator!=(const SSize &c) const                { return !operator==(c); }
  inline bool                   operator<(const SSize &c) const                 { return (absoluteWidth() * absoluteHeight()) <  (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator<=(const SSize &c) const                { return (absoluteWidth() * absoluteHeight()) <= (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator>(const SSize &c) const                 { return (absoluteWidth() * absoluteHeight()) >  (c.absoluteWidth() * c.absoluteHeight()); }
  inline bool                   operator>=(const SSize &c) const                { return (absoluteWidth() * absoluteHeight()) >= (c.absoluteWidth() * c.absoluteHeight()); }

  inline int                    width(void) const                               { return w; }
  inline void                   setWidth(int nw)                                { w = nw; }
  inline int                    height(void) const                              { return h; }
  inline void                   setHeight(int nh)                               { h = nh; }
  inline float                  aspectRatio(void) const                         { return a; }
  inline void                   setAspectRatio(float na)                        { a = na; }
  inline QSize                  size(void) const                                { return QSize(w, h); }

  inline int                    absoluteWidth(void) const                       { return (a > 1.0f) ? int(w * a) : w; }
  inline int                    absoluteHeight(void) const                      { return (a < 1.0f) ? int(h * (1.0f / a)) : h; }
  inline QSize                  absoluteSize(void) const                        { return QSize(absoluteWidth(), absoluteHeight()); }

private:
  int                           w, h;
  float                         a;
};


} // End of namespace


#endif
