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

void FormatProber::probeFormat(ProbeInfo &pi, QIODevice *)
{
  if (BufferReader::isDiscPath(pi.filePath))
  {
    pi.format = BufferReader::formatName;
    pi.fileType = ProbeInfo::FileType_Disc;
    pi.fileTypeName = "Digital Versatile Disc";
    pi.isFormatProbed = true;
  }
}

void FormatProber::probeContent(ProbeInfo &pi, QIODevice *, const QSize &)
{
  if (BufferReader::isDiscPath(pi.filePath))
  {
    BufferReader bufferReader(QString::null, this);
    if (bufferReader.openFile(pi.filePath))
    {
//      pi.programs.clear();
//      for (quint16 i=0, n=bufferReader.numTitles(); i<n; i++)
//      if (bufferReader.selectTitle(i))
//      {
//        ProbeInfo fpi;
//        fpi.programs.append(ProbeInfo::Program(i));
//
//        fpi.programs.first().duration = bufferReader.duration();
//
//        foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
//        {
//          if ((qobject_cast<FormatProber *>(prober) == NULL) && !fpi.isContentProbed)
//          {
//            bufferReader.setPosition(STime::null);
//            prober->probeContent(fpi, &bufferReader);
//          }
//
//          delete prober;
//        }
//
//        if (!fpi.programs.isEmpty())
//        {
//          ProbeInfo::Program &program = fpi.programs.first();
//
//          program.programId = i;
//          program.duration = bufferReader.duration();
//          program.chapters = bufferReader.chapters();
//
//          program.audioStreams = bufferReader.filterAudioStreams(program.audioStreams);
//          program.videoStreams = bufferReader.filterVideoStreams(program.videoStreams);
//          program.dataStreams = bufferReader.filterDataStreams(program.dataStreams);
//
//          if (program.duration.isValid() && (program.duration.toSec() >= 60))
//            pi.programs.append(program);
//        }
//      }

      pi.metadata.insert("title", bufferReader.discTitle());

      pi.isContentProbed = true;
    }
  }
}


} } // End of namespaces
