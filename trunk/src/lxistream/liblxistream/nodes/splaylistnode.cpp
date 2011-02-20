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
#include "sgraph.h"

namespace LXiStream {


struct SPlaylistNode::Data
{
  SDependency                 * dependency;

  QStringList                   fileNames;
  int                           currentFile;
  SFileInputNode              * file;

  AudioStreamInfo               audioStreamInfo;
  bool                          hasVideo;
  VideoStreamInfo               videoStreamInfo;
};

SPlaylistNode::SPlaylistNode(SGraph *parent, const SMediaInfoList &files)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;

  d->currentFile = 0;
  d->file = NULL;

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

  delete d->dependency;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SPlaylistNode::open(void)
{
  for (d->currentFile = 0; d->currentFile < d->fileNames.count(); d->currentFile++)
  if (openFile(d->fileNames[d->currentFile]))
    return true;

  return false;
}

bool SPlaylistNode::start(void)
{
  if (d->file == NULL)
    return open();

  return true;
}

void SPlaylistNode::stop(void)
{
  delete d->file;
  d->file = NULL;

  d->currentFile = 0;
}

void SPlaylistNode::process(void)
{
  if (graph)
    graph->queue(this, &SPlaylistNode::processTask, d->dependency);
  else
    processTask();
}

STime SPlaylistNode::duration(void) const
{
  return STime();
}

bool SPlaylistNode::setPosition(STime)
{
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

void SPlaylistNode::processTask(void)
{
  if (d->file == NULL)
    openNext();

  if (d->file)
    d->file->process();
}

bool SPlaylistNode::openFile(const QString &fileName)
{
  if (d->file)
  {
    d->file->stop();
    delete d->file;
  }

  d->file = new SFileInputNode(NULL, fileName);
  if (d->file->open() && d->file->start())
  {
    qDebug() << "Playing next file:" << fileName;

    connect(d->file, SIGNAL(output(SEncodedAudioBuffer)), SIGNAL(output(SEncodedAudioBuffer)));
    connect(d->file, SIGNAL(output(SEncodedVideoBuffer)), SIGNAL(output(SEncodedVideoBuffer)));
    connect(d->file, SIGNAL(output(SEncodedDataBuffer)), SIGNAL(output(SEncodedDataBuffer)));
    connect(d->file, SIGNAL(finished()), SLOT(openNext()), Qt::QueuedConnection);

    return true;
  }
  else
  {
    delete d->file;
    d->file = NULL;

    return false;
  }
}

void SPlaylistNode::openNext(void)
{
  if (d->currentFile < d->fileNames.count())
    emit closed(d->fileNames[d->currentFile]);

  for (d->currentFile++; d->currentFile < d->fileNames.count(); d->currentFile++)
  if (openFile(d->fileNames[d->currentFile]))
  {
    emit opened(d->fileNames[d->currentFile]);
    return;
  }

  emit finished();
}

} // End of namespace
