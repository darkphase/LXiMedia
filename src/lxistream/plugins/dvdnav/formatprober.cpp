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
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.openFile(filePath))
      result += Format(BufferReader::formatName, 0);
  }

  return result;
}

void FormatProber::probeMetadata(ProbeInfo &pi, ReadCallback *callback)
{
  if (callback)
  if (BufferReader::isDiscPath(callback->path))
  {
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.openFile(callback->path))
    {
      if (BufferReader::isExtractedDiscPath(callback->path))
        pi.path = QFileInfo(BufferReader::discPath(callback->path)).path();

      pi.format = BufferReader::formatName;
      pi.isReadable = true;
      pi.isProbed = true;

      pi.title = bufferReader.discTitle();

      pi.programs.clear();
      for (quint16 i=0, n=bufferReader.numTitles(); i<n; i++)
      if (bufferReader.selectTitle(i))
      {
        ProbeInfo fpi;
        fpi.programs.append(ProbeInfo::Program(i));

        fpi.programs.first().duration = bufferReader.duration();

        foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
        {
          if ((qobject_cast<FormatProber *>(prober) == NULL) && !fpi.isProbed)
          {
            bufferReader.setPosition(STime::null);
            prober->probeMetadata(fpi, &bufferReader);
          }

          delete prober;
        }

        if (!fpi.programs.isEmpty())
        {
          ProbeInfo::Program &program = fpi.programs.first();

          program.programId = i;
          program.duration = bufferReader.duration();
          program.chapters = bufferReader.chapters();

          program.audioStreams = bufferReader.filterAudioStreams(program.audioStreams);
          program.videoStreams = bufferReader.filterVideoStreams(program.videoStreams);
          program.dataStreams = bufferReader.filterDataStreams(program.dataStreams);

          if (program.duration.isValid() && (program.duration.toSec() >= 60))
            pi.programs.append(program);
        }
      }
    }
  }
}


} } // End of namespaces
