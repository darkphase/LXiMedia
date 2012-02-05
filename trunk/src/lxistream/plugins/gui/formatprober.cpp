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

#include "formatprober.h"
#include "imagedecoder.h"

namespace LXiStream {
namespace GuiBackend {

FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::FormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

void FormatProber::readFormat(ProbeInfo &pi, const QByteArray &buffer)
{
  if (!pi.fileInfo.isDir)
  {
    const QString path = pi.filePath.path();
    const QString fileName = path.mid(path.lastIndexOf('/') + 1);
    const int lastdot = fileName.lastIndexOf('.');
    const QString suffix = lastdot >= 0 ? fileName.mid(lastdot + 1).toLower() : QString::null;

    if (SImage::rawImageSuffixes().contains(suffix))
    {
      pi.format.format = suffix;
      pi.format.fileType = ProbeInfo::FileType_Image;
      pi.format.fileTypeName = "RAW image";
      pi.isFormatProbed = true;
    }
    else if (buffer.size() >= 4)
    {
      if ((buffer[0] == 'B') && (buffer[1] == 'M') && suffix.contains("bmp"))
      {
        pi.format.format = "bmp";
        pi.format.fileType = ProbeInfo::FileType_Image;
        pi.format.fileTypeName = "Portable Pixmap";
        pi.isFormatProbed = true;
      }
      else if ((buffer[0] == char(0xFF)) && (buffer[1] == char(0xD8)) && suffix.contains("jp"))
      {
        pi.format.format = "jpeg";
        pi.format.fileType = ProbeInfo::FileType_Image;
        pi.format.fileTypeName = "JPEG Compressed";
        pi.isFormatProbed = true;
      }
      else if ((buffer[0] == char(0x89)) && (buffer[1] == 'P') && (buffer[2] == 'N') && (buffer[3] == 'G'))
      {
        pi.format.format = "png";
        pi.format.fileType = ProbeInfo::FileType_Image;
        pi.format.fileTypeName = "Portable Pixmap";
        pi.isFormatProbed = true;
      }
    }
  }
}

void FormatProber::readContent(ProbeInfo &pi, QIODevice *ioDevice)
{
  if (ioDevice)
  if (SImage::rawImageSuffixes().contains(pi.format.format) ||
      (pi.format.format == "bmp") || (pi.format.format == "jpeg") ||
      (pi.format.format == "png"))
  {
    if (pi.content.titles.isEmpty())
      pi.content.titles += ProbeInfo::Title();

    ProbeInfo::Title &mainTitle = pi.content.titles.first();

    mainTitle.imageCodec = SVideoCodec(pi.format.format.toUpper());

    pi.isContentProbed = true;
  }
}

SVideoBuffer FormatProber::readThumbnail(const ProbeInfo &pi, QIODevice *ioDevice, const QSize &thumbSize)
{
  if (ioDevice)
  {
    if (SImage::rawImageSuffixes().contains(pi.format.format) ||
        (pi.format.format == "bmp") || (pi.format.format == "jpeg") ||
        (pi.format.format == "png"))
    {
      const SImage thumbnail = SImage::fromData(ioDevice, thumbSize, pi.format.format.toAscii());
      if (!thumbnail.isNull())
        return thumbnail.toVideoBuffer();
    }
  }
  
  return SVideoBuffer();
}

} } // End of namespaces
