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

#include "nodes/splaylistnode.h"
#include "nodes/sfileinputnode.h"
#include "sdebug.h"

namespace LXiStream {


struct SPlaylistNode::Data
{
  SScheduler::Dependency      * loadDependency;

  QStringList                   fileNames;
  QList<STime>                  fileOffsets;
  STime                         duration;
  STime                         firstFileOffset;
  int                           fileId;
  int                           nextFileId;
  SFileInputNode              * file;
  SFileInputNode              * nextFile;
  QSemaphore                    nextFileReady;

  AudioStreamInfo               audioStreamInfo;
  bool                          hasVideo;
  VideoStreamInfo               videoStreamInfo;
};

SPlaylistNode::SPlaylistNode(SGraph *parent, const SMediaInfoList &files)
  : QObject(parent),
    SGraph::SourceNode(parent),
    d(new Data())
{
  d->loadDependency = parent ? new SScheduler::Dependency(parent) : NULL;

  d->duration = STime::null;
  d->firstFileOffset = STime();

  d->fileId = -1;
  d->nextFileId = -1;
  d->file = NULL;
  d->nextFile = NULL;

  d->audioStreamInfo.codec = SAudioCodec("*", SAudioFormat::Channel_Stereo, 48000);
  d->hasVideo = true;
  d->videoStreamInfo.codec = SVideoCodec("*");

  int fps15 = 0, fps25 = 0, fps30 = 0;
  SSize size;
  foreach (const SMediaInfo &file, files)
  {
    d->hasVideo &= file.containsVideo();

    if (d->hasVideo)
    foreach (const VideoStreamInfo &info, file.videoStreams())
    {
      const double frameRate = info.codec.frameRate().toFrequency();
      if (frameRate < 20.0)
        fps15++;
      else if ((frameRate < 27.5) || ((frameRate >= 40.0) && (frameRate < 55.0)))
        fps25++;
      else if ((frameRate < 40.0) || ((frameRate >= 55.0) && (frameRate < 65.0)))
        fps30++;

      if (size.isNull() || (info.codec.size() > size))
        size = info.codec.size();
    }

    d->fileNames += file.filePath();
    d->fileOffsets += d->duration;
    d->duration += file.duration();
  }

  if (d->hasVideo)
  {
    if ((fps15 > fps25) && (fps15 > fps30))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(15));
    else if ((fps30 > fps25) && (fps30 > fps15))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(30));
    else
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(25));

    d->videoStreamInfo.codec.setSize(size);
  }
}

SPlaylistNode::~SPlaylistNode()
{
  if (d->file)
  {
    d->file->stop();
    delete d->file;
  }

  if (d->nextFile)
  {
    d->nextFile->stop();
    delete d->nextFile;
  }

  delete d->loadDependency;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SPlaylistNode::open(void)
{
  return !d->fileNames.isEmpty();
}

bool SPlaylistNode::start(void)
{
  d->fileId = -1;
  d->nextFileId = -1;

  return true;
}

void SPlaylistNode::stop(void)
{
  if (d->file)
  {
    d->file->stop();
    delete d->file;
    d->file = NULL;
  }

  if (d->nextFile)
  {
    d->nextFile->stop();
    delete d->nextFile;
    d->nextFile = NULL;
  }

  d->fileId = -1;
  d->nextFileId = -1;
}

void SPlaylistNode::process(void)
{
  if (d->nextFileId == -1)
  {
    openNext();

    if (d->nextFile && d->firstFileOffset.isValid())
      d->nextFile->setPosition(d->firstFileOffset);
  }

  if (d->file == NULL)
  {
    if (d->nextFileId == -2)
    {
      emit finished();
      return;
    }

    d->nextFileReady.acquire();
    d->fileId = d->nextFileId;
    d->file = d->nextFile;

    // Start loading next
    if ((d->nextFileId >= 0) && (d->nextFileId < d->fileNames.count()))
      schedule(&SPlaylistNode::openNext, d->loadDependency, SScheduler::Priority_Low);
    else
      d->nextFileId = -2;

    if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    {
      qDebug() << "SPlaylistNode playing next file:" << d->fileNames[d->fileId];
      emit opened(d->fileNames[d->fileId]);
    }
  }

  if (d->file)
    d->file->process();
}

STime SPlaylistNode::duration(void) const
{
  return d->duration;
}

bool SPlaylistNode::setPosition(STime pos)
{
  if (d->nextFileId == -1)
  {
    STime curPos = pos;
    while (!d->fileNames.isEmpty() && !d->fileOffsets.isEmpty())
    {
      if (d->fileOffsets.count() > 1)
      if (curPos > d->fileOffsets[1])
      {
        d->fileNames.takeFirst();
        d->fileOffsets.takeFirst();
        curPos = pos - d->fileOffsets.first();
        continue;
      }

      break;
    }

    if (!d->fileNames.isEmpty() && !d->fileOffsets.isEmpty())
      d->firstFileOffset = curPos;

    return true;
  }

  return false;
}

STime SPlaylistNode::position(void) const
{
  return STime();
}

QList<SPlaylistNode::Chapter> SPlaylistNode::chapters(void) const
{
  return QList<SPlaylistNode::Chapter>();
}

QList<SPlaylistNode::AudioStreamInfo> SPlaylistNode::audioStreams(void) const
{
  return QList<AudioStreamInfo>() << d->audioStreamInfo;
}

QList<SPlaylistNode::VideoStreamInfo> SPlaylistNode::videoStreams(void) const
{
  if (d->hasVideo)
    return QList<VideoStreamInfo>() << d->videoStreamInfo;
  else
    return QList<VideoStreamInfo>();
}

QList<SPlaylistNode::DataStreamInfo>  SPlaylistNode::dataStreams(void) const
{
  return QList<DataStreamInfo>();
}

void SPlaylistNode::selectStreams(const QList<StreamId> &)
{
}

SFileInputNode * SPlaylistNode::openFile(const QString &fileName)
{
  SFileInputNode * const file = new SFileInputNode(NULL, fileName);
  if (file->open() && file->start())
  {
    connect(file, SIGNAL(output(SEncodedAudioBuffer)), SIGNAL(output(SEncodedAudioBuffer)));
    connect(file, SIGNAL(output(SEncodedVideoBuffer)), SIGNAL(output(SEncodedVideoBuffer)));
    connect(file, SIGNAL(output(SEncodedDataBuffer)), SIGNAL(output(SEncodedDataBuffer)));
    connect(file, SIGNAL(finished()), SLOT(closeFile()), Qt::QueuedConnection);

    return file;
  }

  delete file;
  return NULL;
}

void SPlaylistNode::openNext(void)
{
  for (d->nextFileId++; d->nextFileId < d->fileNames.count(); d->nextFileId++)
  {
    SFileInputNode * const file = openFile(d->fileNames[d->nextFileId]);
    if (file)
    {
      d->nextFile = file;
      d->nextFileReady.release();

      return;
    }
  }

  d->nextFile = NULL;
  d->nextFileId = -2;
  d->nextFileReady.release();
}

void SPlaylistNode::closeFile(void)
{
  if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    emit closed(d->fileNames[d->fileId]);

  if (d->file)
  {
    d->file->stop();
    delete d->file;
    d->file = NULL;
  }
}

} // End of namespace
