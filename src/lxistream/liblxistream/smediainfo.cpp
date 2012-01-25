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

#include "smediainfo.h"
#include "sinterfaces.h"
#include "smediafilesystem.h"
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

SMediaInfo::SMediaInfo(const QUrl &filePath)
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

QUrl SMediaInfo::filePath(void) const
{
  return pi->filePath;
}

QString SMediaInfo::fileName(void) const
{
  const QString path = pi->filePath.path();
  
  return path.mid(path.lastIndexOf('/') + 1);
}

QString SMediaInfo::baseName(void) const
{
  const QString name = fileName();
  const int lastdot = name.lastIndexOf('.');

  return lastdot >= 0 ? name.left(lastdot) : name;
}

QUrl SMediaInfo::path(void) const
{
  QUrl result = pi->filePath;

  QString path = result.path();
  path = path.left(path.lastIndexOf('/') + 1);
  result.setPath(path);
  
  return result;
}

SMediaFilesystem SMediaInfo::directory(void) const
{
  return SMediaFilesystem(path());
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
  const SMediaFilesystem mediaDir(path());

  const SMediaFilesystem::Info info = mediaDir.readInfo(fileName());
  pi->fileInfo.isDir = info.isDir;
  pi->fileInfo.size = info.size;
  pi->fileInfo.lastModified = info.lastModified;
  pi->isReadable = info.isReadable;

  pi->isFileInfoRead = true;
}

void SMediaInfo::probeFormat(void)
{
  if (pi->isReadable && !pi->isFileInfoRead)
    readFileInfo();

  if (!pi->fileInfo.isDir)
  {
    const SMediaFilesystem mediaDir(path());

    QIODevice * const ioDevice = mediaDir.openFile(fileName());
    if (ioDevice)
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (ioDevice->seek(0))
        if (!pi->isFormatProbed)
          prober->probeFormat(*pi, ioDevice);

        delete prober;
      }

      pi->isFormatProbed = true;
      delete ioDevice;
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
    const SMediaFilesystem mediaDir(path());

    QIODevice * const ioDevice = mediaDir.openFile(fileName());
    if (ioDevice)
    {
      foreach (SInterfaces::FormatProber *prober, SInterfaces::FormatProber::create(NULL))
      {
        if (ioDevice->seek(0))
        {
          if (!pi->isFormatProbed)
            prober->probeFormat(*pi, ioDevice);

          if (pi->isFormatProbed && !pi->isContentProbed)
            prober->probeContent(*pi, ioDevice, QSize(128, 128));
        }

        delete prober;
      }

      probeDataStreams();

      pi->isContentProbed = true;
      delete ioDevice;
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
      foreach (const QUrl &filePath, SSubtitleFile::findSubtitleFiles(pi->filePath))
      {
        bool found = false;
        for (QList<DataStreamInfo>::Iterator i=subs.begin(); i!=subs.end(); i++)
        if (i->file == filePath)
        {
          i->type = DataStreamInfo::Type_Subtitle;
          i->id = nextStreamId++;
          title.dataStreams += *i;

          found = true;
          break;
        }

        if (!found)
        {
          SSubtitleFile file(filePath);
          if (file.open())
          {
            DataStreamInfo stream(
                StreamId(DataStreamInfo::Type_Subtitle, nextStreamId++),
                file.language(),
                QString::null,
                file.codec());

            stream.file = filePath;
            title.dataStreams += stream;
          }
        }
      }
    }
  }
}

} // End of namespace
