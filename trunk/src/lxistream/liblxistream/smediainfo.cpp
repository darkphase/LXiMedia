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

bool SMediaInfo::isReadable(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->isReadable;
}

qint64 SMediaInfo::size(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->fileInfo.size;
}

QDateTime SMediaInfo::lastModified(void) const
{
  if (pi->isReadable && !pi->isFileInfoRead)
    const_cast<SMediaInfo *>(this)->readFileInfo();

  return pi->fileInfo.lastModified;
}

QString SMediaInfo::format(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.format;
}

SMediaInfo::ProbeInfo::FileType SMediaInfo::fileType(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.fileType;
}

QString SMediaInfo::fileTypeName(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.fileTypeName;
}

bool SMediaInfo::isComplexFile(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.isComplexFile;
}

QByteArray SMediaInfo::quickHash(void) const
{
  if (pi->isReadable && !pi->isFormatProbed)
    const_cast<SMediaInfo *>(this)->probeFormat();

  return pi->format.quickHash;
}

QVariant SMediaInfo::metadata(const QString &key) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  QMap<QString, QVariant>::ConstIterator i = pi->format.metadata.find(key);
  if (i != pi->format.metadata.end())
    return i.value();

  return QVariant();
}

const QList<SMediaInfo::ProbeInfo::Title> & SMediaInfo::titles(void) const
{
  if (pi->isReadable && !pi->isContentProbed)
    const_cast<SMediaInfo *>(this)->probeContent();

  return pi->content.titles;
}

void SMediaInfo::readFileInfo(void)
{
  const QFileInfo fileInfo(pi->filePath);
  if (fileInfo.exists())
  {
    pi->fileInfo.isDir = fileInfo.isDir();
    pi->fileInfo.size = fileInfo.size();
    pi->fileInfo.lastModified = fileInfo.lastModified();
    pi->isReadable = fileInfo.isReadable();

    pi->isFileInfoRead = true;
  }
}

void SMediaInfo::probeFormat(void)
{
  if (pi->isReadable && !pi->isFileInfoRead)
    readFileInfo();

  if (!pi->fileInfo.isDir)
  {
    QFile file(pi->filePath);
    if (file.open(QFile::ReadOnly))
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (file.seek(0))
        if (!pi->isFormatProbed)
          prober->probeFormat(*pi, &file);

        delete prober;
      }

      pi->isFormatProbed = true;
    }
  }
  else
  {
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      if (!pi->isFormatProbed)
        prober->probeFormat(*pi, NULL);

      delete prober;
    }

    pi->isFormatProbed = true;
  }
}

void SMediaInfo::probeContent(void)
{
  if (pi->isReadable && !pi->isFileInfoRead)
    readFileInfo();

  if (!pi->fileInfo.isDir)
  {
    QFile file(pi->filePath);
    if (file.open(QFile::ReadOnly))
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (file.seek(0))
        {
          if (!pi->isFormatProbed)
            prober->probeFormat(*pi, &file);

          if (pi->isFormatProbed && !pi->isContentProbed)
            prober->probeContent(*pi, &file, QSize(128, 128));
        }

        delete prober;
      }

      probeDataStreams();

      pi->isContentProbed = true;
    }
  }
  else
  {
    foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
    {
      if (!pi->isFormatProbed)
        prober->probeFormat(*pi, NULL);

      if (pi->isFormatProbed && !pi->isContentProbed)
        prober->probeContent(*pi, NULL, QSize(128, 128));

      delete prober;
    }

    pi->isContentProbed = true;
  }
}

void SMediaInfo::probeDataStreams(void)
{
  if (pi->content.titles.count() == 1)
  {
    ProbeInfo::Title &title = pi->content.titles.first();

    if (!title.videoStreams.isEmpty())
    {
      // Remove subtitles.
      QList<DataStreamInfo> subs;
      for (QList<DataStreamInfo>::Iterator i=title.dataStreams.begin();
           i != title.dataStreams.end(); )
      if (!i->file.isEmpty())
      {
        subs += *i;
        i = title.dataStreams.erase(i);
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
          title.dataStreams += *i;

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
            title.dataStreams += stream;
          }
        }
      }
    }
  }
}

} // End of namespace
