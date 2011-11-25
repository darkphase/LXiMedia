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

SImage::SImage(const SVideoBuffer &inBuffer, bool fast)
  : QImage()
{
  d.aspectRatio = 1.0f;

  if (!inBuffer.isNull())
  {
    SVideoBuffer videoBuffer = inBuffer;
    const SSize size = videoBuffer.format().size();
    switch(videoBuffer.format().format())
    {
    default:
      videoBuffer = SVideoFormatConvertNode::convert(videoBuffer, SVideoFormat::Format_RGB32);
      if (videoBuffer.isNull())
        return;

      // Deliberately no break.

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
}

SImage::SImage(const QString &fileName, const QSize &maxsize, const char *format)
  : QImage()
{
  *this = fromFile(fileName, maxsize, format);
}

#ifndef QT_NO_CAST_FROM_ASCII
SImage::SImage(const char *fileName, const QSize &maxsize, const char *format)
  : QImage()
{
  *this = fromFile(fileName, maxsize, format);
}
#endif

SVideoBuffer SImage::toVideoBuffer(SInterval frameRate) const
{
  if (!isNull())
  {
    SVideoFormat bufferFormat;
    switch (format())
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
      bufferFormat.setFormat(SVideoFormat::Format_RGB32, size(), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB888:
      bufferFormat.setFormat(SVideoFormat::Format_RGB24, size(), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB16:
      bufferFormat.setFormat(SVideoFormat::Format_RGB565, size(), frameRate, SVideoFormat::FieldMode_Progressive);
      break;

    case QImage::Format_RGB555:
      bufferFormat.setFormat(SVideoFormat::Format_RGB555, size(), frameRate, SVideoFormat::FieldMode_Progressive);
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

SImage SImage::fromData(const char *data, int size, const QSize &maxsize, const char *format)
{
  QByteArray byteArray = QByteArray::fromRawData(data, size);
  QBuffer buffer(&byteArray);
  if (buffer.open(QBuffer::ReadOnly))
  {
    QImageReader imageReader(&buffer, format ? QByteArray(format) : QByteArray());
    if (imageReader.canRead())
    {
      ExifData * exifData = NULL;
      if (imageReader.format() == "jpeg")
        exifData = exif_data_new_from_data(reinterpret_cast<const uchar *>(data), size);

      return handleFile(imageReader, maxsize, exifData);
    }
  }

  return SImage();
}

SImage SImage::fromData(QIODevice *ioDevice, const QSize &maxsize, const char *format)
{
  QImageReader imageReader(ioDevice, format ? QByteArray(format) : QByteArray());
  if (imageReader.canRead())
  {
    QByteArray data;
    ExifData * exifData = NULL;
    if (imageReader.format() == "jpeg")
    {
      const qint64 pos = ioDevice->pos();
      ioDevice->seek(0);
      data = ioDevice->readAll();
      ioDevice->seek(pos);

      exifData = exif_data_new_from_data(reinterpret_cast<const uchar *>(data.data()), data.size());
    }

    return handleFile(imageReader, maxsize, exifData);
  }

  return SImage();
}

SImage SImage::fromFile(const QString &fileName, const QSize &maxsize, const char *format)
{
  QFile file(fileName);
  if (file.open(QBuffer::ReadOnly))
  {
    QImageReader imageReader(&file, format ? QByteArray(format) : QByteArray());
    if (imageReader.canRead())
    {
      ExifData * exifData = NULL;
      if (imageReader.format() == "jpeg")
        exifData = exif_data_new_from_file(fileName.toUtf8());

      return handleFile(imageReader, maxsize, exifData);
    }
  }

  return SImage();
}

SImage SImage::handleFile(QImageReader &imageReader, QSize maxsize, void *exifDataVoid)
{
  ExifData * const exifData = (ExifData *)exifDataVoid;

  SImage image;

  if (exifData)
  {
    exif_data_ref(exifData);

    if (maxsize.isValid() && !maxsize.isNull() && (maxsize.width() <= 256) && (maxsize.height() < 256))
    if (exifData->data && (exifData->size > 0))
      image = QImage::fromData(exifData->data, exifData->size, "jpeg");

    ExifEntry * const orientation = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
    if (orientation)
    {
      exif_entry_ref(orientation);

      switch (exif_get_short(orientation->data, exif_data_get_byte_order(exifData)))
      {
      case 6:
      case 8:
        maxsize = QSize(maxsize.height(), maxsize.width());

      default:
        break;
      }

      exif_entry_unref(orientation);
    }
  }

  if (image.isNull())
  {
    if (maxsize.isValid() && !maxsize.isNull())
    {
      QSize size = imageReader.size();
      size.scale(maxsize, Qt::KeepAspectRatio);
      imageReader.setScaledSize(size);
    }

    image = imageReader.read();
  }

  if (!image.isNull() && maxsize.isValid() && !maxsize.isNull())
  if ((image.width() > maxsize.width()) || (image.height() > maxsize.height()))
    image = image.scaled(maxsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  if (exifData)
  {
    if (!image.isNull())
    {
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
    }

    exif_data_unref(exifData);
  }

  image.d.originalSize = imageReader.size();

  return image;
}

} // End of namespace
