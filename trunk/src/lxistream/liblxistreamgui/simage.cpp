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
    return fromData(&buffer, maxsize, format);

  return SImage();
}

SImage SImage::fromData(QIODevice *ioDevice, const QSize &maxsize, const char *format)
{
  if (rawImageSuffixes().contains(format))
  {
    QFile * const file = qobject_cast<QFile *>(ioDevice);
    if (file)
      return fromFile(file->fileName(), maxsize, format);

    QTemporaryFile tmpfile(QDir::temp().absoluteFilePath(QFileInfo(qApp->applicationFilePath()).baseName() + ".XXXXXX." + format));
    if (tmpfile.open())
    {
      tmpfile.write(ioDevice->readAll());
      tmpfile.close();

      return fromFile(tmpfile.fileName(), maxsize, format);
    }
  }
  else
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
  }

  return SImage();
}

SImage SImage::fromFile(const QString &fileName, const QSize &maxsize, const char *format)
{
  if (rawImageSuffixes().contains(format) ||
      rawImageSuffixes().contains(QFileInfo(fileName).suffix().toLower()))
  {
    struct T
    {
      static SImage rundcraw(const QStringList &options, const QSize &maxsize)
      {
        QProcess dcRawProcess;
#ifdef Q_OS_WIN
        const QDir appDir(qApp->applicationDirPath());
        dcRawProcess.start(appDir.absoluteFilePath("dcraw.exe"), options);
#else
        dcRawProcess.start("dcraw", options);
#endif

        if (dcRawProcess.waitForStarted())
        if (dcRawProcess.waitForFinished())
        {
          if (dcRawProcess.exitCode() == 0)
          {
            QImageReader imageReader(&dcRawProcess);
            if (imageReader.canRead())
              return handleFile(imageReader, maxsize);
          }
          else
          {
            dcRawProcess.setReadChannel(QProcess::StandardError);
            qDebug() << "dcraw:" << dcRawProcess.readAll();
          }
        }

        dcRawProcess.kill();

        return SImage();
      }
    };

    SImage result;
    if (maxsize.isValid() && !maxsize.isNull() && (maxsize.width() <= 256) && (maxsize.height() < 256))
      result = T::rundcraw(QStringList() << "-c" << "-e" << fileName, maxsize);

    if (result.isNull())
      result = T::rundcraw(QStringList() << "-c" << fileName, maxsize);

    return result;
  }
  else
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
  }

  return SImage();
}

SImage SImage::handleFile(QImageReader &imageReader, QSize maxsize, void *exifDataVoid)
{
  ExifData * const exifData = (ExifData *)exifDataVoid;

  const QSize originalSize = imageReader.size();
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
      QSize size = originalSize;
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

  image.d.originalSize = originalSize;

  return image;
}

const QSet<QString>  & SImage::rawImageSuffixes(void)
{
  static QSet<QString> suffixes;

  if (suffixes.isEmpty())
  {
    suffixes += "3fr";
    suffixes += "arw";
    suffixes += "srf";
    suffixes += "sr2";
    suffixes += "bay";
    suffixes += "crw";
    suffixes += "cr2";
    suffixes += "cap";
    suffixes += "tif";
    suffixes += "iiq";
    suffixes += "eip";
    suffixes += "dcs";
    suffixes += "dcr";
    suffixes += "drf";
    suffixes += "k25";
    suffixes += "kdc";
    suffixes += "dng";
    suffixes += "erf";
    suffixes += "fff";
    suffixes += "mos";
    suffixes += "mrw";
    suffixes += "nef";
    suffixes += "nrw";
    suffixes += "orf";
    suffixes += "ptx";
    suffixes += "pef";
    suffixes += "pxn";
    suffixes += "r3d";
    suffixes += "raf";
    suffixes += "raw";
    suffixes += "rw1";
    suffixes += "rw2";
    suffixes += "rwz";
    suffixes += "x3f";
  }

  return suffixes;
}

QString SImage::rawImageDescription(const QString &suffix)
{
  if ((suffix == "raw"))
    return "Generic Raw";
  else if (suffix == "3fr")
    return "Hasselblad Raw";
  else if ((suffix == "arw") || (suffix == "srf") || (suffix == "sr2"))
    return "Sony Raw";
  else if (suffix == "bay")
    return "Casio Raw";
  else if ((suffix == "crw") || (suffix == "cr2"))
    return "Canon Raw";
  else if ((suffix == "cap") || (suffix == "iiq") || (suffix == "eip"))
    return "Phase One Raw";
  else if ((suffix == "dcs") || (suffix == "dcr") || (suffix == "drf") || (suffix == "k25") || (suffix == "kdc"))
    return "Kodak Raw";
  else if (suffix == "dng")
    return "Adobe Raw";
  else if (suffix == "erf")
    return "Epson Raw";
  else if (suffix == "fff")
    return "Imacon Raw";
  else if (suffix == "mef")
    return "Mamiya Raw";
  else if (suffix == "mos")
    return "Leaf Raw";
  else if (suffix == "mrw")
    return "Minolta Raw";
  else if ((suffix == "nef") || (suffix == "nrw"))
    return "Nikon Raw";
  else if (suffix == "orf")
    return "Olympus Raw";
  else if ((suffix == "ptx") || (suffix == "pef"))
    return "Pentax Raw";
  else if (suffix == "pxn")
    return "Logitech Raw";
  else if (suffix == "r3d")
    return "Red Raw";
  else if (suffix == "raf")
    return "Fuji Raw";
  else if (suffix == "rw2")
    return "Panasonic Raw";
  else if (suffix == "rw1")
    return "Leica Raw";
  else if (suffix == "rwz")
    return "Rawzor Raw";
  else if (suffix == "x3f")
    return "Signa Raw";

  return "Raw Image";
}

} // End of namespace
