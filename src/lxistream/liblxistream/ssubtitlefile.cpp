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

#include "ssubtitlefile.h"
#include <LXiCore>

namespace LXiStream {

struct SSubtitleFile::Data
{
  inline Data(const QString &name) : file(name), language(NULL), utf8(false) { }

  QFile                         file;
  SDataCodec                    dataCodec;
  const char                  * language;
  bool                          utf8;
};

SSubtitleFile::SSubtitleFile(const QString &name)
  : d(new Data(name))
{
}

SSubtitleFile::~SSubtitleFile()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SSubtitleFile::exists(void) const
{
  return d->file.exists();
}

QString SSubtitleFile::fileName(void) const
{
  return d->file.fileName();
}

bool SSubtitleFile::open(void)
{
  if (d->file.open(QFile::ReadOnly))
  {
    const QByteArray sample = d->file.read(262144);
    d->file.seek(0);

    if (!sample.isEmpty())
    {
      d->utf8 = SStringParser::isUtf8(sample);
      if (d->utf8)
      {
        d->dataCodec = SDataCodec("SUB/RAWUTF8");
        d->language = SStringParser::languageOf(QString::fromUtf8(sample));
      }
      else
      {
        d->dataCodec = SDataCodec("SUB/RAWLOCAL8BIT");
        d->language = SStringParser::languageOf(QString::fromLocal8Bit(sample));
      }

      return true;
    }
  }

  return false;
}

void SSubtitleFile::close(void)
{
  d->file.close();
}

void SSubtitleFile::reset(void)
{
  d->file.seek(0);
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

  while ((line = d->file.readLine()).length() > 0)
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

QStringList SSubtitleFile::findSubtitleFiles(const QString &file)
{
  const QFileInfo mediaFileInfo(file);
  const QString baseName = SStringParser::toRawName(mediaFileInfo.completeBaseName());

  struct T
  {
    static void checkFiles(QStringList &result, const QDir &dir, const QString &baseName)
    {
      foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.srt", QDir::Files | QDir::Readable))
      if (SStringParser::toRawName(info.completeBaseName()).startsWith(baseName))
        result += info.absoluteFilePath();
    }
  };

  QStringList result;
  T::checkFiles(result, mediaFileInfo.absoluteDir(), baseName);

  foreach (const QFileInfo &info, mediaFileInfo.absoluteDir().entryInfoList(QDir::Dirs))
  if ((info.fileName().toLower() == "subtitles") || (info.fileName().toLower() == "subs"))
    T::checkFiles(result, info.absoluteFilePath(), baseName);

  return result;
}

} // End of namespace
