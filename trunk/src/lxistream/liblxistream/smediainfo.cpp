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

#include "smediainfo.h"
#include "sinterfaces.h"
#include "ssubtitlefile.h"
#include <LXiCore>

namespace LXiStream {

const unsigned SMediaInfo::tvShowSeason = 100u;

SMediaInfo::SMediaInfo(void)
  : pi(NULL)
{
}

SMediaInfo::SMediaInfo(const SMediaInfo &from)
  : pi(from.pi)
{
}

SMediaInfo::SMediaInfo(const QString &path)
  : pi(new ProbeInfo())
{
  probe(path);
}

SMediaInfo::SMediaInfo(const QSharedDataPointer<ProbeInfo> &pi)
  : pi(pi)
{
}

SMediaInfo::~SMediaInfo()
{
}

SMediaInfo & SMediaInfo::operator=(const SMediaInfo &from)
{
  pi = from.pi;

  return *this;
}

bool SMediaInfo::isNull(void) const
{
  return pi == NULL;
}

QString SMediaInfo::filePath(void) const
{
  return pi->filePath;
}

QString SMediaInfo::fileName(void) const
{
  return QFileInfo(pi->filePath).fileName();
}

QString SMediaInfo::baseName(void) const
{
  return QFileInfo(pi->filePath).completeBaseName();
}

QString SMediaInfo::path(void) const
{
  if (pi->path.isEmpty())
    return QFileInfo(pi->filePath).path();
  else
    return pi->path;
}

qint64 SMediaInfo::size(void) const
{
  return pi->size;
}

QDateTime SMediaInfo::lastModified(void) const
{
  return pi->lastModified;
}

QString SMediaInfo::format(void) const
{
  return pi->format;
}

STime SMediaInfo::totalDuration(void) const
{
  STime result = STime::null;

  foreach (const Program &p, pi->programs)
  if (p.duration.isValid())
    result += p.duration;

  return result.isPositive() ? result : STime();
}

bool SMediaInfo::containsAudio(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.audioStreams.isEmpty())
    return true;

  return false;
}

bool SMediaInfo::containsVideo(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.videoStreams.isEmpty())
    return true;

  return false;
}

bool SMediaInfo::containsImage(void) const
{
  foreach (const Program &p, pi->programs)
  if (!p.imageCodec.isNull())
    return true;

  return false;
}

bool SMediaInfo::isProbed(void) const
{
  return pi->isProbed;
}

bool SMediaInfo::isReadable(void) const
{
  return pi->isReadable;
}

QString SMediaInfo::fileTypeName(void) const
{
  return pi->fileTypeName;
}

const QList<SMediaInfo::Program> & SMediaInfo::programs(void) const
{
  return pi->programs;
}

QString SMediaInfo::title(void) const
{
  if (pi->title.isEmpty())
    return SStringParser::toCleanName(QFileInfo(pi->filePath).completeBaseName());
  else
    return pi->title;
}

QString SMediaInfo::author(void) const
{
  return pi->author;
}

QString SMediaInfo::copyright(void) const
{
  return pi->copyright;
}

QString SMediaInfo::comment(void) const
{
  return pi->comment;
}

QString SMediaInfo::album(void) const
{
  return pi->album;
}

QString SMediaInfo::genre(void) const
{
  return pi->genre;
}

unsigned SMediaInfo::year(void) const
{
  return pi->year;
}

unsigned SMediaInfo::track(void) const
{
  return pi->track;
}

void SMediaInfo::probe(const QString &filePath)
{
  struct Callback : SInterfaces::BufferReader::ReadCallback
  {
    Callback(const QString &path) : ReadCallback(), file(path)
    {
      file.open(QFile::ReadOnly);
    }

    virtual qint64 read(uchar *buffer, qint64 size)
    {
      if (file.isOpen())
        return file.read((char *)buffer, size);
      else
        return -1;
    }

    virtual qint64 seek(qint64 offset, int whence)
    {
      if (file.isOpen())
      {
        if (whence == SEEK_SET)
          return file.seek(offset) ? 0 : -1;
        else if (whence == SEEK_CUR)
          return file.seek(file.pos() + offset) ? 0 : -1;
        else if (whence == SEEK_END)
          return file.seek(file.size() + offset) ? 0 : -1;
        else if (whence == -1) // get size
          return file.size();
      }

      return -1;
    }

    QFile file;
  };

  const QFileInfo fileInfo(filePath);
  if (fileInfo.exists())
  {
    pi->filePath = filePath;
    pi->size = fileInfo.size();
    pi->lastModified = fileInfo.lastModified();

    Callback callback(filePath);
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      if (!pi->isProbed)
      {
        callback.seek(0, SEEK_SET);
        prober->probeMetadata(*pi, &callback);
      }

      delete prober;
    }

    probeDataStreams();

    pi->isProbed = true;
  }
}

// Subtitles in separate files need to be verified every time
void SMediaInfo::probeDataStreams(void)
{
  if (pi->programs.count() == 1)
  {
    Program * const program = &(pi->programs.first());
    if (!program->videoStreams.isEmpty())
    {
      // Remove subtitles.
      QList<DataStreamInfo> subs;
      for (QList<DataStreamInfo>::Iterator i=program->dataStreams.begin();
           i != program->dataStreams.end(); )
      if (!i->file.isEmpty())
      {
        subs += *i;
        i = program->dataStreams.erase(i);
      }
      else
        i++;

      // Add subtitles with new ID (To ensure IDs match SFileInputNode).
      quint16 nextStreamId = 0xF000;
      foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(pi->filePath))
      {
        bool found = false;
        for (QList<DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
        if (i->file == fileName)
        {
          i->type = DataStreamInfo::Type_Subtitle;
          i->id = nextStreamId++;
          program->dataStreams += *i;

          found = true;
          break;
        }

        if (!found)
        {
          SSubtitleFile file(fileName);
          if (file.open())
          {
            DataStreamInfo stream(
                StreamId(DataStreamInfo::Type_Subtitle, nextStreamId++),
                file.language(),
                QString::null,
                file.codec());

            stream.file = fileName;
            program->dataStreams += stream;
          }
        }
      }
    }
  }
}

} // End of namespace
