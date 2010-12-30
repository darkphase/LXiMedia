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

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &data)
{
  QList<Format> result;

  if ((data[0] == 'B') && (data[1] == 'M'))
    result += Format("bmp", -1);
  else if ((data[0] == char(0xFF)) && (data[1] == char(0xD8)))
    result += Format("jpeg", -1);
  else if ((data[0] == char(0x89)) && (data[1] == 'P') && (data[2] == 'N') && (data[3] == 'G'))
    result += Format("png", -1);

  return result;
}

void FormatProber::probeName(ProbeInfo &, const QString &)
{
}

void FormatProber::probeFile(ProbeInfo &pi, QIODevice *file)
{
  if ((file->size() > 0) && (file->size() <= (16384 * 1024)))
  {
    const SImage image = SImage::fromData(file->readAll());
    if (!image.isNull())
    {
      pi.imageCodec = SVideoCodec(pi.imageCodec.codec(), image.size());
      pi.isProbed = true;
      pi.isReadable = true;

      if ((pi.imageCodec.size().width() >= 256) && (pi.imageCodec.size().height() >= 256))
      {
        SImage thumbnail;
        if ((pi.imageCodec.size().width() >= 1024) || (pi.imageCodec.size().height() >= 1024))
          thumbnail = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::FastTransformation);
        else
          thumbnail = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QBuffer b;
        if (thumbnail.save(&b, "JPEG", 50))
          pi.thumbnails += b.data();
      }
    }
  }
}

} } // End of namespaces
