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
#include "bufferreader.h"

namespace LXiStream {
namespace DVDNavBackend {


FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::FormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &, const QString &filePath)
{
  QList<Format> result;

  if (BufferReader::isDiscPath(filePath))
  {
    ::dvdnav_t * dvdHandle = NULL;
    if (::dvdnav_open(&dvdHandle, filePath.toUtf8()) == DVDNAV_STATUS_OK)
    {
      result += Format(BufferReader::formatName, 0);

      ::dvdnav_close(dvdHandle);
    }
  }

  return result;
}

void FormatProber::probeFormat(ProbeInfo &pi, QIODevice *)
{
  if (BufferReader::isDiscPath(pi.filePath))
  {
    QFile file(pi.filePath);
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.start(&file, NULL, false))
    {
      pi.format.format = BufferReader::formatName;
      pi.format.fileType = ProbeInfo::FileType_Disc;
      pi.format.fileTypeName = "Digital Versatile Disc";

      const QString discName = bufferReader.discName();
      if (!discName.isEmpty())
        pi.format.metadata.insert("title", discName);

      pi.isFormatProbed = true;

      bufferReader.stop();
    }
  }
}

void FormatProber::probeContent(ProbeInfo &pi, QIODevice *, const QSize &thumbSize)
{
  if (BufferReader::isDiscPath(pi.filePath))
  {
    QFile file(pi.filePath);
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.start(&file, NULL, false))
    {
      pi.content.titles.clear();

      for (int i=0, n=bufferReader.numTitles(); i<n; i++)
      if (bufferReader.selectTitle(i))
      {
        ProbeInfo fpi;
        fpi.content.titles += ProbeInfo::Title();
        fpi.content.titles.first().duration = bufferReader.duration();

        QIODevice * const ioDevice = bufferReader.getIODevice();
        foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
        {
          if (ioDevice->seek(0))
          {
            if (!fpi.isFormatProbed)
              prober->probeFormat(fpi, ioDevice);

            if (fpi.isFormatProbed && !fpi.isContentProbed)
              prober->probeContent(fpi, ioDevice, thumbSize);
          }

          delete prober;
        }

        if (!fpi.content.titles.isEmpty())
        {
          ProbeInfo::Title &title = fpi.content.titles.first();

          title.duration = bufferReader.duration();
          if (title.duration.isValid() && (title.duration.toSec() >= 60))
          {
            title.chapters = bufferReader.chapters();

            title.audioStreams = bufferReader.filterAudioStreams(title.audioStreams);
            title.videoStreams = bufferReader.filterVideoStreams(title.videoStreams);
            title.dataStreams = bufferReader.filterDataStreams(title.dataStreams);

            pi.content.titles += title;
          }
        }
      }

      pi.isContentProbed = true;
    }
  }
}


} } // End of namespaces
