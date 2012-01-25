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

    \note Most methods have been inlined and there is no penalty of invoking
          them in loops over first copying them to a local variable.
    \note This class can be serialized.
 */
class LXISTREAM_PUBLIC SSize
{
public:
                                SSize(void);
                                SSize(const SSize &c);
                                SSize(const QSize &c, float aspectRatio = 1.0f);
                                SSize(int width, int height, float aspectRatio = 1.0f);

  bool                          isValid(void) const;
  bool                          isNull(void) const;

  SSize                       & operator=(const SSize &c);
  bool                          operator==(const SSize &c) const;
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

  int                           absoluteWidth(void) const;
  int                           absoluteHeight(void) const;
  inline QSize                  absoluteSize(void) const                        { return QSize(absoluteWidth(), absoluteHeight()); }

  /*! Serializes the size to a string in the form of "WxHxA".
   */
  QString                       toString(void) const;

  /*! Parses a size string in the form of "WxHxA". For example:
      "32" -> { 32, 32, 1.0f },
      "32x64" -> { 32, 64, 1.0f },
      "32x64x1.5" -> { 32, 64, 1.5f }.
   */
  static SSize                  fromString(const QString &);

  /*! Scales the size to the specified size retaining the correct aspect ratio.
   */
  SSize                         scaled(int nw, int nh) const;

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
