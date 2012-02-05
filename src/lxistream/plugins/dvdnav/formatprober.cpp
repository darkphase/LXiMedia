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

void FormatProber::readFormat(ProbeInfo &pi, const QByteArray &)
{
  if (BufferReader::isDiscPath(pi.filePath))
  {
    pi.format.format = BufferReader::formatName;
    pi.format.fileType = ProbeInfo::FileType_Disc;
    pi.format.fileTypeName = "Digital Versatile Disc";
    pi.isFormatProbed = true;
  }
}

void FormatProber::readContent(ProbeInfo &pi, QIODevice *ioDevice)
{
  QFile * const file = qobject_cast<QFile *>(ioDevice);
  if (file && (pi.format.format == BufferReader::formatName))
  {
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.start(file, NULL, false))
    {
      const QString discName = bufferReader.discName();
      if (!discName.isEmpty())
        pi.content.metadata.insert("title", discName);

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
          if (!fpi.isContentProbed)
            prober->readContent(fpi, ioDevice);

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

SVideoBuffer FormatProber::readThumbnail(const ProbeInfo &pi, QIODevice *ioDevice, const QSize &thumbSize)
{
  SVideoBuffer result;

  if (ioDevice)
  {
    QFile * const file = qobject_cast<QFile *>(ioDevice);
    if (file && BufferReader::isDiscPath(file->fileName()))
    {
      BufferReader bufferReader(QString::null, this);
      if (bufferReader.start(file, NULL, false))
      {
        for (int i=0, n=bufferReader.numTitles(); (i<n) && result.isNull(); i++)
        if (bufferReader.selectTitle(i))
        {
          QIODevice * const ioDevice = bufferReader.getIODevice();

          const QByteArray buffer =
              ioDevice->read(SInterfaces::FormatProber::defaultProbeSize);

          const QList<SInterfaces::FormatProber *> probers =
              SInterfaces::FormatProber::create(NULL);

          ProbeInfo fpi;
          foreach (SInterfaces::FormatProber *prober, probers)
          if (!fpi.isFormatProbed)
            prober->readFormat(fpi, buffer);

          foreach (SInterfaces::FormatProber *prober, probers)
          {
            if (fpi.isFormatProbed && result.isNull() && ioDevice->seek(0))
              result = prober->readThumbnail(fpi, ioDevice, thumbSize);

            delete prober;
          }
        }
      }
    }
  }

  return result;
}


} } // End of namespaces
