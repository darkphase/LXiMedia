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

const int SMediaInfo::tvShowSeason = 100;

SMediaInfo::SMediaInfo(void)
  : pi(NULL)
{
}

SMediaInfo::SMediaInfo(const SMediaInfo &from)
  : pi(from.pi)
{
}

SMediaInfo::SMediaInfo(const QString &filePath)
  : pi(new ProbeInfo())
{
  pi->filePath = filePath;
  pi->isReadable = true;
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
  return QFileInfo(pi->filePath).path();
}

qint64 SMediaInfo::size(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->size;
}

QDateTime SMediaInfo::lastModified(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->lastModified;
}

bool SMediaInfo::isReadable(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->isReadable;
}

QString SMediaInfo::format(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format;
}

SMediaInfo::ProbeInfo::FileType SMediaInfo::fileType(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->fileType;
}

QString SMediaInfo::fileTypeName(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->fileTypeName;
}

STime SMediaInfo::duration(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->duration;
}

const QList<SMediaInfo::Chapter> & SMediaInfo::chapters(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->chapters;
}

const QList<SMediaInfo::AudioStreamInfo> & SMediaInfo::audioStreams(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->audioStreams;
}

const QList<SMediaInfo::VideoStreamInfo> & SMediaInfo::videoStreams(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->videoStreams;
}

const QList<SMediaInfo::DataStreamInfo> & SMediaInfo::dataStreams(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->dataStreams;
}

const SVideoCodec & SMediaInfo::imageCodec(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->imageCodec;
}

const QMap<QString, QVariant> & SMediaInfo::metadata(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->metadata;
}

QVariant SMediaInfo::metadata(const QString &key) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  QMap<QString, QVariant>::ConstIterator i = pi->metadata.find(key);
  if (i != pi->metadata.end())
    return i.value();

  return QVariant();
}

const SVideoBuffer & SMediaInfo::thumbnail(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->thumbnail;
}

void SMediaInfo::readFileInfo(void)
{
  const QFileInfo fileInfo(pi->filePath);
  if (fileInfo.exists())
  {
    pi->size = fileInfo.size();
    pi->lastModified = fileInfo.lastModified();
    pi->isReadable = fileInfo.isReadable();
  }
}

void SMediaInfo::probeFormat(void)
{
  QFile file(pi->filePath);
  if (file.open(QFile::ReadOnly))
  {
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      if (!pi->isFormatProbed)
        prober->probeFormat(*pi, &file);

      delete prober;
    }

    pi->isFormatProbed = true;
  }
}

void SMediaInfo::probeContent(void)
{
  QFile file(pi->filePath);
  if (file.open(QFile::ReadOnly))
  {
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      if (!pi->isContentProbed)
      {
        file.seek(0);
        prober->probeContent(*pi, &file, QSize(128, 128));
      }

      delete prober;
    }

    probeDataStreams();

    pi->isContentProbed = true;
  }
}

// Subtitles in separate files need to be verified every time
void SMediaInfo::probeDataStreams(void)
{
  if (!pi->videoStreams.isEmpty())
  {
    // Remove subtitles.
    QList<DataStreamInfo> subs;
    for (QList<DataStreamInfo>::Iterator i=pi->dataStreams.begin();
         i != pi->dataStreams.end(); )
    if (!i->file.isEmpty())
    {
      subs += *i;
      i = pi->dataStreams.erase(i);
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
        pi->dataStreams += *i;

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
          pi->dataStreams += stream;
        }
      }
    }
  }
}

} // End of namespace
