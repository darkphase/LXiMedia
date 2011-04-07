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

#ifndef LXISTREAM_SVIDEOFORMAT_H
#define LXISTREAM_SVIDEOFORMAT_H

#include <QtCore>
#include <QtGlobal>
#include <LXiCore>
#include "sinterval.h"
#include "ssize.h"

namespace LXiStream {

class S_DSO_PUBLIC SVideoFormat
{
public:
  /*! Specifies the format in which data is stored.
   */
  enum Format
  {
    Format_Invalid = 0,

    Format_RGB555 = 0x2000, Format_BGR555, Format_RGB565, Format_BGR565,
    Format_RGB24, Format_BGR24, Format_RGB32, Format_BGR32, Format_GRAY8,
    Format_GRAY16BE, Format_GRAY16LE,
    Format_YUYV422 = 0x2100, Format_UYVY422, Format_YUV410P, Format_YUV411P,
    Format_YUV420P, Format_YUV422P, Format_YUV444P,
    Format_BGGR8 = 0x2200, Format_GBRG8, Format_GRBG8, Format_RGGB8,
    Format_BGGR10, Format_GBRG10, Format_GRBG10, Format_RGGB10,
    Format_BGGR16, Format_GBRG16, Format_GRBG16, Format_RGGB16
  };

  /*! Specifies the field-mode of image data.
   */
  enum FieldMode
  {
    FieldMode_Invalid = 0,                                                      //!< No field-mode specified.
    FieldMode_Progressive,                                                      //!< Progressive (non-interlaced) video data.
    FieldMode_TopField,                                                         //!< Top field of interlaced video data only.
    FieldMode_BottomField,                                                      //!< Bottom field of interlaced video data only.
    FieldMode_InterlacedTopFirst,                                               //!< Interlaced video data, top field first.
    FieldMode_InterlacedBottomFirst,                                            //!< Interlaced video data, bottom field first.
    FieldMode_SequentialTopFirst,                                               //!< Interlaced video data in sequential fields, top field in top half of picture.
    FieldMode_SequentialBottomFirst,                                            //!< Interlaced video data in sequential fields, bottom field in top half of picture.
    FieldMode_Alternating                                                       //!< Interlaced video data, fields alternating at double frame-speed.
  };

public:
                                SVideoFormat(void);
                                SVideoFormat(Format format, SSize = SSize(), SInterval = SInterval(), FieldMode = FieldMode_Invalid);

  inline                        operator Format() const                         { return format(); }
  inline                        operator const char *() const                   { return formatName(); }

  bool                          operator==(const SVideoFormat &other) const;
  inline bool                   operator!=(const SVideoFormat &other) const     { return !operator==(other); }
  bool                          operator==(Format other) const                  { return d.format == other; }
  inline bool                   operator!=(Format other) const                  { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.format == Format_Invalid; }
  inline Format                 format(void) const                              { return d.format; }
  void                          setFormat(Format format, SSize = SSize(), SInterval = SInterval(), FieldMode = FieldMode_Invalid);

  inline SSize                  size(void) const                                { return d.size; }
  inline void                   setSize(SSize s)                                { d.size = s; }
  inline SInterval              frameRate(void) const                           { return d.frameRate; }
  inline void                   setFrameRate(SInterval r)                       { d.frameRate = r; }
  inline FieldMode              fieldMode(void) const                           { return d.fieldMode; }
  inline void                   setFieldMode(FieldMode m)                       { d.fieldMode = m; }

  inline bool                   isRGB(void) const                               { return isRGB(format()); }
  inline bool                   isYUV(void) const                               { return isYUV(format()); }
  inline bool                   isBayerArray(void) const                        { return isBayerArray(format()); }

  inline const char           * formatName(void) const                          { return formatName(format()); }
  inline int                    numPlanes(void) const                           { return numPlanes(format()); }
  inline int                    sampleSize(void) const                          { return sampleSize(format()); }
  inline bool                   planarYUVRatio(int &w, int &h) const            { return planarYUVRatio(format(), w, h); }
  inline quint32                nullPixelValue(void) const                      { return nullPixelValue(format()); }

  static inline bool            isRGB(Format f)                                 { return (f >= 0x2000) && (f < 0x2100); }
  static inline bool            isYUV(Format f)                                 { return (f >= 0x2100) && (f < 0x2200); }
  static inline bool            isBayerArray(Format f)                          { return (f >= 0x2200) && (f < 0x2300); }

  static int                    sampleSize(Format) __attribute__((pure));
  static int                    numPlanes(Format) __attribute__((pure));
  static bool                   planarYUVRatio(Format, int &w, int &h);
  static quint32                nullPixelValue(Format) __attribute__((pure));
  static const char           * formatName(Format) __attribute__((pure));

private:
  struct
  {
    Format                      format;
    SSize                       size;
    SInterval                   frameRate;
    FieldMode                   fieldMode;
  }                             d;
};

} // End of namespace

#endif
