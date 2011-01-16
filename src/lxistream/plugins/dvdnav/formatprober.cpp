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
#include "discreader.h"

namespace LXiStream {
namespace DVDNavBackend {


FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::FormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

QList<FormatProber::Format> FormatProber::probeFileFormat(const QByteArray &, const QString &)
{
  return QList<Format>();
}

QList<FormatProber::Format> FormatProber::probeDiscFormat(const QString &devicePath)
{
  QList<Format> result;

  if (DiscReader::isDiscPath(devicePath))
  {
    DiscReader discReader(QString::null, this);
    if (discReader.openPath(DiscReader::formatName, devicePath))
      result += Format(DiscReader::formatName, 0);
  }

  return result;
}

void FormatProber::probeFile(ProbeInfo &, ReadCallback *)
{
}

void FormatProber::probeDisc(ProbeInfo &pi, const QString &devicePath)
{
  if (DiscReader::isDiscPath(devicePath))
  {
    DiscReader discReader(QString::null, this);
    if (discReader.openPath(DiscReader::formatName, devicePath))
    {
      if (DiscReader::isExtractedDiscPath(devicePath))
        pi.path = QFileInfo(DiscReader::discPath(devicePath)).path();

      pi.format = DiscReader::formatName;
      pi.isDisc = true;
      pi.isReadable = true;

      pi.title = discReader.title();

      pi.titles.clear();
      for (unsigned i=0, n=discReader.numTitles(); i<n; i++)
      {
        SInterfaces::FormatProber::ProbeInfo fpi;

        if (discReader.playTitle(i))
        {
          foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(this))
          {
            discReader.seek(0, SEEK_SET);
            prober->probeFile(fpi, &discReader);

            delete prober;
          }

          discReader.annotateAudioStreams(fpi.audioStreams);
          discReader.annotateVideoStreams(fpi.videoStreams);
          discReader.annotateDataStreams(fpi.dataStreams);
          discReader.annotateChapters(fpi.chapters);

          if (!fpi.duration.isValid() || fpi.duration.isNull())
            fpi.duration = discReader.duration();
        }

        pi.titles += fpi;
      }
    }
  }
}


} } // End of namespaces
