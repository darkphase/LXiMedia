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
namespace DVDReadBackend {


FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::DiscFormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

QList<FormatProber::Format> FormatProber::probeFormat(const QString &path)
{
  QList<Format> result;

  if (isDisc(path))
  {
    DiscReader discReader(QString::null, this);
    if (discReader.openPath(DiscReader::formatName, path))
      result += Format(DiscReader::formatName, 0);
  }

  return result;
}

void FormatProber::probePath(ProbeInfo &pi, const QString &path)
{
  if (isDisc(path))
  {
    DiscReader discReader(QString::null, this);
    if (discReader.openPath(DiscReader::formatName, path))
    {
      pi.format = DiscReader::formatName;

      pi.titles.clear();
      for (unsigned i=0, n=discReader.numTitles(); i<n; i++)
      {
        SInterfaces::FileFormatProber::ProbeInfo fpi;

        SInterfaces::BufferReader::ReadCallback * const readCallback = discReader.openTitle(i);
        if (readCallback)
        {

          foreach (SInterfaces::FileFormatProber *prober, SInterfaces::FileFormatProber::create(this))
          {
            readCallback->seek(0, SEEK_SET);
            prober->probeFile(fpi, readCallback);

            delete prober;
          }

          discReader.closeTitle(readCallback);
        }

        pi.titles += fpi;
      }
    }
  }
}

bool FormatProber::isDisc(const QString &path)
{
  const QString canonicalPath = QFileInfo(path).canonicalFilePath();

  if (canonicalPath.endsWith(".iso", Qt::CaseInsensitive) ||
#ifdef Q_OS_UNIX
      canonicalPath.startsWith("/dev/") ||
#endif
      QDir(canonicalPath).entryList().contains("VIDEO_TS", Qt::CaseInsensitive))
  {
    return true;
  }

  return false;
}


} } // End of namespaces
