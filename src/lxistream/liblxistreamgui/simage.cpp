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

#include "simage.h"
#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>
#include <libexif/exif-ifd.h>
#include <libexif/exif-tag.h>
#include <liblxistream/nodes/svideoformatconvertnode.h>

namespace LXiStreamGui {

using namespace LXiStream;

SImage::SImage(const SVideoBuffer &inbuffer, bool fast)
  : QImage()
{
  SVideoBuffer videoBuffer;
  if (videoBuffer.format().isYUV())
    videoBuffer = SVideoFormatConvertNode::convertYUVtoRGB(inbuffer);
  else if (videoBuffer.format().isBayerArray())
    videoBuffer = SVideoFormatConvertNode::demosaic(inbuffer);
  else if (videoBuffer.format() == SVideoFormat::Format_BGR32)
    videoBuffer = SVideoFormatConvertNode::convertBGRtoRGB(inbuffer);
  else
    videoBuffer = inbuffer;

  const SSize size = videoBuffer.format().size();

  switch(videoBuffer.format().format())
  {
  default:
    return;

  case SVideoFormat::Format_RGB32:
    *this = QImage(size.size(), QImage::Format_RGB32);
    for (int y=0; y<size.height(); y++)
      memcpy(scanLine(y), videoBuffer.scanLine(y, 0), size.width() * 4);

    break;

  case SVideoFormat::Format_RGB24:
    *this = QImage(size.size(), QImage::Format_RGB888);
    for (int y=0; y<size.height(); y++)
      memcpy(scanLine(y), videoBuffer.scanLine(y, 0), size.width() * 3);

    break;

  case SVideoFormat::Format_RGB565:
    *this = QImage(size.size(), QImage::Format_RGB16);
    for (int y=0; y<size.height(); y++)
      memcpy(scanLine(y), videoBuffer.scanLine(y, 0), size.width() * 2);

    break;

  case SVideoFormat::Format_RGB555:
    *this = QImage(size.size(), QImage::Format_RGB555);
    for (int y=0; y<size.height(); y++)
      memcpy(scanLine(y), videoBuffer.scanLine(y, 0), size.width() * 2);

    break;
  }

  // Compensate the aspect ratio if needed.
  if (!qFuzzyCompare(size.aspectRatio(), 1.0f))
    *this = scaled(size.absoluteSize(), Qt::IgnoreAspectRatio,
                   fast ? Qt::FastTransformation : Qt::SmoothTransformation);
}

SImage::SImage(const QString &fileName, const char *format)
  :QImage()
{
  QFile file(fileName);
  if (file.open(QFile::ReadOnly))
    *this = fromData(file.readAll(), format);
}

#ifndef QT_NO_CAST_FROM_ASCII
SImage::SImage(const char *fileName, const char *format)
  :QImage()
{
  QFile file(fileName);
  if (file.open(QFile::ReadOnly))
    *this = fromData(file.readAll(), format);
}
#endif

SVideoBuffer SImage::toVideoBuffer(float aspectRatio, SInterval frameRate) const
{
  if (!isNull())
  {
    SVideoFormat bufferFormat;
    switch (format())
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
      bufferFormat.setFormat(SVideoFormat::Format_RGB32, SSize(size(), aspectRatio), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB888:
      bufferFormat.setFormat(SVideoFormat::Format_RGB24, SSize(size(), aspectRatio), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB16:
      bufferFormat.setFormat(SVideoFormat::Format_RGB565, SSize(size(), aspectRatio), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB555:
      bufferFormat.setFormat(SVideoFormat::Format_RGB555, SSize(size(), aspectRatio), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    default:
      break;
    }

    if (!bufferFormat.isNull())
    {
      if (((bytesPerLine() & ~(SBuffer::minimumAlignVal - 1)) == 0) &&
          ((intptr_t(bits()) & ~intptr_t(SBuffer::minimumAlignVal - 1)) == 0))
      { // Data is aligned; it can be reused.
        class Image : public SBuffer::Memory
        {
        public:
          Image(const QImage &image)
            : SBuffer::Memory(image.bytesPerLine() * image.height(),
                              const_cast<char *>(reinterpret_cast<const char *>(image.bits())),
                              image.bytesPerLine() * image.height()),
              image(image)
          {
          }

          virtual ~Image()
          {
          }

        private:
          const QImage image;
        };

        const int offset[4] = { 0, 0, 0, 0 };
        const int lineSize[4] = { bytesPerLine(), 0, 0, 0 };
        return SVideoBuffer(bufferFormat,
                            SBuffer::MemoryPtr(new Image(*this)),
                            offset,
                            lineSize);
      }
      else // Unaligned data; has to be copied.
      {
        SVideoBuffer videoBuffer(bufferFormat);
        for (int y=0; y<height(); y++)
        {
          memcpy(videoBuffer.scanLine(y, 0),
                 scanLine(y),
                 qMin(videoBuffer.lineSize(0), bytesPerLine()));
        }

        return videoBuffer;
      }
    }
  }

  return SVideoBuffer();
}

SImage SImage::fromData(const uchar *data, int size, const char *format)
{
  SImage image = QImage::fromData(data, size, format);
  if (!image.isNull())
  {
    ExifData * const exifData = exif_data_new_from_data(data, size);
    if (exifData)
    {
      exif_data_ref(exifData);

      ExifEntry * const orientation = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
      if (orientation)
      {
        exif_entry_ref(orientation);

        QMatrix matrix;
        switch (exif_get_short(orientation->data, exif_data_get_byte_order(exifData)))
        {
        case 2: matrix.scale(-1.0, 1.0);
        default:
        case 1:                             break;
        case 4: matrix.scale(-1.0, 1.0);
        case 3: matrix.rotate(180);         break;
        case 5: matrix.scale(1.0, -1.0);
        case 6: matrix.rotate( 90);         break;
        case 7: matrix.scale(1.0, -1.0);
        case 8: matrix.rotate(-90);         break;
        }

        if (!matrix.isIdentity())
          image = image.transformed(matrix);

        exif_entry_unref(orientation);
      }

      exif_data_unref(exifData);
    }
  }

  return image;
}

SImage SImage::fromData(const QByteArray &data, const char *format)
{
  return fromData(reinterpret_cast<const uchar *>(data.data()), data.size(), format);
}

} // End of namespace
