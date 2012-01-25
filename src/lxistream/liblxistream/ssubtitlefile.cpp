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

#include "ssubtitlefile.h"
#include <LXiCore>
#include "smediafilesystem.h"
#include "smediainfo.h"

namespace LXiStream {

struct SSubtitleFile::Data
{
  inline Data(const QUrl &filePath)
    : filePath(filePath), ioDevice(NULL), language(NULL), utf8(false)
  {
  }

  QUrl                          filePath;
  QIODevice                   * ioDevice;
  SDataCodec                    dataCodec;
  const char                  * language;
  bool                          utf8;
};

SSubtitleFile::SSubtitleFile(const QUrl &filePath)
  : d(new Data(filePath))
{
}

SSubtitleFile::~SSubtitleFile()
{
  delete d->ioDevice;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QUrl SSubtitleFile::filePath(void) const
{
  return d->filePath;
}

bool SSubtitleFile::open(void)
{
  delete d->ioDevice;
  d->ioDevice = SMediaFilesystem::open(d->filePath);

  if (d->ioDevice)
  {
    const QByteArray sample = d->ioDevice->read(262144);
    d->ioDevice->seek(0);

    if (!sample.isEmpty())
    {
      d->utf8 = SStringParser::isUtf8(sample);
      d->dataCodec = SDataCodec(d->utf8 ? "SUB/RAWUTF8" : "SUB/RAW8BIT");
      d->language = SStringParser::languageOf(sample);

      if (!d->utf8)
      {
        const char * const codepage = SStringParser::codepageFor(d->language);
        if (codepage)
          d->dataCodec.setCodepage(codepage);
      }

      return true;
    }
  }

  return false;
}

void SSubtitleFile::close(void)
{
  delete d->ioDevice;
  d->ioDevice = NULL;
}

void SSubtitleFile::reset(void)
{
  d->ioDevice->seek(0);
}

/*! Returns the ISO 639-2 language code of the subtitles, or an empty string if
    it can't be determined. The file has to be opened to determine the language.

    \sa SStringParser::languageOf()
 */
const char * SSubtitleFile::language(void) const
{
  return d->language;
}

SDataCodec SSubtitleFile::codec(void) const
{
  return d->dataCodec;
}

SEncodedDataBuffer SSubtitleFile::readSubtitle(STime timeStamp)
{
  int phase = 0;
  STime startTime = STime::null, stopTime = STime::null;
  QByteArray data;
  QByteArray line;

  while ((line = d->ioDevice->readLine()).length() > 0)
  {
    if (phase == 0)
    { // Get ID
      bool ok = false;
      line.trimmed().toInt(&ok);

      if (ok)
        phase++;
    }
    else if (phase == 1)
    { // Get times
      const QStringList times = QString(line.trimmed()).split("-->", QString::SkipEmptyParts);
      if (times.count() >= 2)
      {
        QTime start = QTime::fromString(times[0].trimmed());
        QTime stop = QTime::fromString(times[1].trimmed());

        if (start.isValid() && stop.isValid())
        {
          startTime = STime::fromMSec(QTime(0, 0).msecsTo(start));
          stopTime = STime::fromMSec(QTime(0, 0).msecsTo(stop));
          phase++;
        }
      }
    }
    else if (phase == 2)
    { // Read subtitles
      const QByteArray t = line.trimmed();

      if (t.length() > 0)
      {
        if (data.isEmpty())
          data = t;
        else
          data += '\n' + t;
      }
      else if (stopTime > timeStamp)
      { // We're finished
        if (!data.isEmpty())
        {
          SEncodedDataBuffer buffer(d->dataCodec, data);
          buffer.setPresentationTimeStamp(startTime);
          buffer.setDuration(stopTime - startTime);
          return buffer;
        }
        else
          return SEncodedDataBuffer();
      }
      else // Parse the next subtitle
      {
        data.clear();
        phase = 0;
      }
    }
  }

  return SEncodedDataBuffer();
}

QList<QUrl> SSubtitleFile::findSubtitleFiles(const QUrl &file)
{
  struct T
  {
    static void exactMatches(QList<QUrl> &result, const SMediaFilesystem &dir, const QString &baseName)
    {
      foreach (const QString &fileName, dir.entryList(QDir::Files | QDir::Readable))
      if (fileName.startsWith(baseName, Qt::CaseInsensitive) &&
          fileName.toLower().endsWith(".srt"))
      {
        const QUrl url(dir.filePath(fileName));
        if (!result.contains(url))
          result += url;
      }
    }

    static void fuzzyMatches(QList<QUrl> &result, const SMediaFilesystem &dir, const QString &baseName)
    {
      foreach (const QString &fileName, dir.entryList(QDir::Files | QDir::Readable))
      if (SStringParser::toRawName(fileName).startsWith(baseName) &&
          fileName.toLower().endsWith(".srt"))
      {
        const QUrl url(dir.filePath(fileName));
        if (!result.contains(url))
          result += url;
      }
    }
  };

  const SMediaInfo mediaInfo(file);
  const SMediaFilesystem directory(mediaInfo.directory());
  const QString baseName = mediaInfo.baseName();
  const QString rawBaseName = SStringParser::toRawName(baseName);

  QList<QUrl> result;
  T::exactMatches(result, directory, baseName);

  foreach (const QString &dirName, directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
  if (dirName.contains("sub", Qt::CaseInsensitive))
    T::exactMatches(result, SMediaFilesystem(directory.filePath(dirName)), baseName);

  T::fuzzyMatches(result, directory, rawBaseName);

  foreach (const QString &dirName, directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
  if (dirName.contains("sub", Qt::CaseInsensitive))
    T::fuzzyMatches(result, SMediaFilesystem(directory.filePath(dirName)), rawBaseName);

  return result;
}

} // End of namespace
