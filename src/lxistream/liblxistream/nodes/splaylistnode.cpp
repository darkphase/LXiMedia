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

namespace LXiStream {


struct SPlaylistNode::Data
{
  QList< QPair<QString, quint16> > fileNames;
  QList<STime>                  fileOffsets;
  STime                         duration;
  STime                         firstFileOffset;
  int                           fileId;
  int                           nextFileId;
  SFileInputNode              * file;
  SFileInputNode              * nextFile;

  AudioStreamInfo               audioStreamInfo;
  bool                          hasVideo;
  VideoStreamInfo               videoStreamInfo;

  QFuture<void>                 loadFuture;
};

SPlaylistNode::SPlaylistNode(SGraph *parent, const SMediaInfoList &files)
  : QObject(parent),
    SGraph::SourceNode(parent),
    d(new Data())
{
  d->duration = STime::null;
  d->firstFileOffset = STime();

  d->fileId = -1;
  d->nextFileId = -1;
  d->file = NULL;
  d->nextFile = NULL;

  d->hasVideo = true;

  bool surround = true;
  int fps15 = 0, fps24 = 0, fps25 = 0, fps30 = 0;
  int sizeSD = 0, size720 = 0, size1080 = 0;
  SSize maxSize;
  foreach (const SMediaInfo &file, files)
  {
    quint16 programId = 0;
    foreach (const SMediaInfo::Program &program, file.programs())
    {
      foreach (const AudioStreamInfo &info, program.audioStreams)
        surround &= info.codec.numChannels() >= 5;

      d->hasVideo &= !program.videoStreams.isEmpty();
      if (d->hasVideo)
      foreach (const VideoStreamInfo &info, program.videoStreams)
      {
        const double frameRate = info.codec.frameRate().toFrequency();
        if (frameRate < 23.0)
          fps15++;
        else if ((frameRate < 24.5) || ((frameRate >= 44.0) && (frameRate < 49.0)))
          fps24++;
        else if ((frameRate < 27.5) || ((frameRate >= 44.0) && (frameRate < 55.0)))
          fps25++;
        else if ((frameRate < 32.5) || ((frameRate >= 55.0) && (frameRate < 65.0)))
          fps30++;

        if (info.codec.size().width() < 1280)
          sizeSD++;
        else if (info.codec.size().width() < 1920)
          size720++;
        else
          size1080++;

        if (maxSize.isNull() || (info.codec.size() > maxSize))
          maxSize = info.codec.size();
      }

      d->fileNames += qMakePair(file.filePath(), programId++);
      d->fileOffsets += d->duration;
      d->duration += program.duration;
    }
  }

  if (surround)
    d->audioStreamInfo.codec = SAudioCodec("*", SAudioFormat::Channel_Surround_5_1, 48000);
  else
    d->audioStreamInfo.codec = SAudioCodec("*", SAudioFormat::Channel_Stereo, 48000);

  if (d->hasVideo)
  {
    d->videoStreamInfo.codec = SVideoCodec("*");

    if ((fps15 > fps24) && (fps15 > fps25) && (fps15 > fps30))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(15));
    else if ((fps24 > fps15) && (fps24 > fps25) && (fps24 > fps30))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(24));
    else if ((fps25 > fps15) && (fps25 > fps24) && (fps25 > fps30))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(25));
    else if ((fps30 > fps15) && (fps30 > fps24) && (fps30 > fps25))
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(30));
    else // Default
      d->videoStreamInfo.codec.setFrameRate(SInterval::fromFrequency(25));

    if ((size720 == 0) && (size1080 == 0))
      d->videoStreamInfo.codec.setSize(maxSize);
    else if (size1080 > size720)
      d->videoStreamInfo.codec.setSize(SSize(1920, 1080));
    else
      d->videoStreamInfo.codec.setSize(SSize(1280, 720));
  }
}

SPlaylistNode::~SPlaylistNode()
{
  d->loadFuture.waitForFinished();

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

  d->loadFuture = QtConcurrent::run(this, &SPlaylistNode::openNext);

  return true;
}

void SPlaylistNode::stop(void)
{
  d->loadFuture.waitForFinished();

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
  LXI_PROFILE_FUNCTION;

  if (d->file == NULL)
  {
    d->loadFuture.waitForFinished();

    if (d->nextFileId == -2)
    {
      emit finished();
      return;
    }

    d->fileId = d->nextFileId;
    d->file = d->nextFile;

    // Start loading next
    if ((d->nextFileId >= 0) && (d->nextFileId < d->fileNames.count()))
      d->loadFuture = QtConcurrent::run(this, &SPlaylistNode::openNext);
    else
      d->nextFileId = -2;

    if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    {
      qDebug() << "SPlaylistNode playing next file:" << d->fileNames[d->fileId].first << d->fileNames[d->fileId].second;
      emit opened(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);
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

SFileInputNode * SPlaylistNode::openFile(const QString &fileName, quint16 programId)
{
  LXI_PROFILE_FUNCTION;

  SFileInputNode * const file = new SFileInputNode(NULL, fileName);
  if (file->open(programId) && file->start())
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
    SFileInputNode * const file = openFile(d->fileNames[d->nextFileId].first, d->fileNames[d->nextFileId].second);
    if (file)
    {
      d->nextFile = file;

      if ((d->nextFileId == 0) && d->firstFileOffset.isValid())
        d->nextFile->setPosition(d->firstFileOffset);

      return;
    }
  }

  d->nextFile = NULL;
  d->nextFileId = -2;
}

void SPlaylistNode::closeFile(void)
{
  if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    emit closed(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);

  if (d->file)
  {
    d->file->stop();
    delete d->file;
    d->file = NULL;
  }
}

} // End of namespace
