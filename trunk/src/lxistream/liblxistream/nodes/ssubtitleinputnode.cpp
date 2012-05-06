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

#include "nodes/ssubtitleinputnode.h"
#include <LXiCore>
#include "smediafilesystem.h"
#include "smediainfo.h"

namespace LXiStream {

struct SSubtitleInputNode::Data
{
  QIODevice                   * ioDevice;
  QUrl                          filePath;

  SEncodedDataBuffer            nextSubtitle;
};

SSubtitleInputNode::SSubtitleInputNode(SGraph *parent, const QUrl &filePath)
  : SIOInputNode(parent),
    d(new Data())
{
  d->ioDevice = NULL;

  setFilePath(filePath);
}

SSubtitleInputNode::~SSubtitleInputNode()
{
  delete d->ioDevice;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SSubtitleInputNode::setFilePath(const QUrl &filePath)
{
  delete d->ioDevice;
  d->filePath = filePath;
  d->ioDevice = SMediaFilesystem::open(filePath);

  if (d->ioDevice)
    SIOInputNode::setIODevice(d->ioDevice);
}

QUrl SSubtitleInputNode::filePath(void) const
{
  return d->filePath;
}

bool SSubtitleInputNode::process(STime pos)
{
  if (!d->nextSubtitle.isNull() && (d->nextSubtitle.presentationTimeStamp() < pos))
  {
    emit output(d->nextSubtitle);
    d->nextSubtitle = SEncodedDataBuffer();
  }

  if (d->nextSubtitle.isNull())
    return SIOInputNode::process();
  else
    return true;
}

void SSubtitleInputNode::produce(const SEncodedAudioBuffer &)
{
}

void SSubtitleInputNode::produce(const SEncodedVideoBuffer &)
{
}

void SSubtitleInputNode::produce(const SEncodedDataBuffer &buffer)
{
  d->nextSubtitle = buffer;
}

QList<QUrl> SSubtitleInputNode::findSubtitleFiles(const QUrl &file)
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
