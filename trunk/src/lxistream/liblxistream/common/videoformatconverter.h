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

#ifndef LXISTREAM_VIDEOFORMATCONVERTER_H
#define LXISTREAM_VIDEOFORMATCONVERTER_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace Common {

template <SVideoFormat::Format _srcFormat, SVideoFormat::Format _dstFormat>
class VideoFormatConverterBase : public SInterfaces::VideoFormatConverter
{
public:
  template <class _instance>
  static void registerClass(int priority = 0)
  {
    SInterfaces::VideoFormatConverter::registerClass<_instance>(_srcFormat, _dstFormat, priority);
  }

public:
  explicit                      VideoFormatConverterBase(const QString &, QObject *);
  virtual                       ~VideoFormatConverterBase();

public: // From SInterfaces::VideoFormatConverter
  virtual bool                  openFormat(const SVideoFormat &srcFormat, const SVideoFormat &dstFormat);
};

class VideoFormatConverter_Format_YUYV422_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUYV422_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_YUYV422 : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUYV422>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_YUYV422(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUYV422>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_UYVY422_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_UYVY422_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_UYVY422 : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_UYVY422>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_UYVY422(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_UYVY422>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_BGR32_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_BGR32, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_BGR32_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_BGR32, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_BGR32 : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_BGR32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_BGR32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_BGR32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_YUV420P_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_YUV420P, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUV420P_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUV420P, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_YUV420P : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV420P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_YUV420P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV420P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_YUV422P_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_YUV422P, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUV422P_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUV422P, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_YUV422P : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV422P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_YUV422P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV422P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_YUV444P_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_YUV444P, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUV444P_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUV444P, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGB32_Format_YUV444P : public VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV444P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGB32_Format_YUV444P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGB32, SVideoFormat::Format_YUV444P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};


class VideoFormatConverter_Format_GRBG8_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_GRBG8, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_GRBG8_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_GRBG8, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_GBRG8_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_GBRG8, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_GBRG8_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_GBRG8, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_RGGB8_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_RGGB8, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_RGGB8_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_RGGB8, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_BGGR8_Format_RGB32 : public VideoFormatConverterBase<SVideoFormat::Format_BGGR8, SVideoFormat::Format_RGB32>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_BGGR8_Format_RGB32(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_BGGR8, SVideoFormat::Format_RGB32>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};


class VideoFormatConverter_Format_YUYV422_Format_YUV420P : public VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV420P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUYV422_Format_YUV420P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV420P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_UYVY422_Format_YUV420P : public VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV420P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_UYVY422_Format_YUV420P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV420P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_YUYV422_Format_YUV422P : public VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV422P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_YUYV422_Format_YUV422P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_YUYV422, SVideoFormat::Format_YUV422P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

class VideoFormatConverter_Format_UYVY422_Format_YUV422P : public VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV422P>
{
Q_OBJECT
public:
  inline VideoFormatConverter_Format_UYVY422_Format_YUV422P(const QString &name, QObject *parent)
    : VideoFormatConverterBase<SVideoFormat::Format_UYVY422, SVideoFormat::Format_YUV422P>(name, parent)
  {
  }

  virtual SVideoBuffer          convertBuffer(const SVideoBuffer &);
};

} } // End of namespaces

#endif
