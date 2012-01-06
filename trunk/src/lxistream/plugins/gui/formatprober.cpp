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

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &data, const QUrl &filePath)
{
  QList<Format> result;

  if (data.size() >= 4)
  {
    if ((data[0] == 'B') && (data[1] == 'M'))
      result += Format("bmp", -1);
    else if ((data[0] == char(0xFF)) && (data[1] == char(0xD8)))
      result += Format("jpeg", -1);
    else if ((data[0] == char(0x89)) && (data[1] == 'P') && (data[2] == 'N') && (data[3] == 'G'))
      result += Format("png", -1);
  }

  const QString path = filePath.path();
  const QString fileName = path.mid(path.lastIndexOf('/') + 1);
  const int lastdot = fileName.lastIndexOf('.');
  const QString suffix = lastdot >= 0 ? fileName.mid(lastdot + 1).toLower() : QString::null;
  if (SImage::rawImageSuffixes().contains(suffix))
    result += Format(suffix, -1);

  return result;
}

void FormatProber::probeFormat(ProbeInfo &pi, QIODevice *ioDevice)
{
  if (ioDevice)
  foreach (
      const Format &format,
      FormatProber::probeFormat(ioDevice->peek(4), pi.filePath))
  {
    pi.format.format = format.name;
    pi.format.fileType = ProbeInfo::FileType_Image;

    if (pi.format.fileTypeName.isEmpty())
      pi.format.fileTypeName = SImage::rawImageDescription(format.name);

    pi.isFormatProbed = true;
    break;
  }
}

void FormatProber::probeContent(ProbeInfo &pi, QIODevice *ioDevice, const QSize &thumbSize)
{
  if (ioDevice)
  foreach (
      const Format &format,
      FormatProber::probeFormat(ioDevice->peek(4), pi.filePath))
  {
    const SImage thumbnail = SImage::fromData(ioDevice, thumbSize, format.name.toAscii());
    if (!thumbnail.isNull())
    {
      if (pi.content.titles.isEmpty())
        pi.content.titles += ProbeInfo::Title();

      ProbeInfo::Title &mainTitle = pi.content.titles.first();

      mainTitle.imageCodec = SVideoCodec(format.name.toUpper(), thumbnail.originalSize());
      mainTitle.thumbnail = thumbnail.toVideoBuffer();

      pi.isContentProbed = true;
    }

    break;
  }
}

} } // End of namespaces
