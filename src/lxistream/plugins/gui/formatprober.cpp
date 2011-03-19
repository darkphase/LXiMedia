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

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &data, const QString &)
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

  return result;
}

void FormatProber::probeMetadata(ProbeInfo &pi, ReadCallback *readCallback)
{
  if (readCallback)
  {
    const qint64 size = readCallback->seek(0, -1);
    if ((size > 0) && (size <= (16384 * 1024)))
    {
      QByteArray data(size, 0);
      data.resize(readCallback->read(reinterpret_cast<uchar *>(data.data()), data.size()));

      const SImage image = SImage::fromData(data);
      if (!image.isNull())
      {
        pi.isProbed = true;
        pi.isReadable = true;

        if (pi.programs.isEmpty())
          pi.programs.append(ProbeInfo::Program());

        ProbeInfo::Program &program = pi.programs.first();

        program.imageCodec = SVideoCodec(program.imageCodec.codec(), image.size());

        if ((program.imageCodec.size().width() >= 256) && (program.imageCodec.size().height() >= 256))
        {
          SImage thumbnail;
          if ((program.imageCodec.size().width() >= 1024) || (program.imageCodec.size().height() >= 1024))
            thumbnail = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::FastTransformation);
          else
            thumbnail = image.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          QBuffer b;
          if (thumbnail.save(&b, "JPEG", 50))
            program.thumbnail = b.data();
        }
      }
    }
  }
}

} } // End of namespaces
