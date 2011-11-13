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

  AudioStreamInfo               audioStreamInfo;
  bool                          hasVideo;
  VideoStreamInfo               videoStreamInfo;
};

SPlaylistNode::SPlaylistNode(SGraph *parent, const SMediaInfoList &files)
  : SFileInputNode(parent, QString::null),
    d(new Data())
{
  d->duration = STime::null;
  d->firstFileOffset = STime();
  d->fileId = -1;

  d->hasVideo = true;

  setFiles(files);
}

SPlaylistNode::~SPlaylistNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SPlaylistNode::setFiles(const SMediaInfoList &files)
{
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

        if ((info.codec.size().width() < 1280) && (info.codec.size().height() < 720))
          sizeSD++;
        else if ((info.codec.size().width() < 1920) && (info.codec.size().height() < 1080))
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
    d->audioStreamInfo.codec = SAudioCodec("*", SAudioFormat::Channels_Surround_5_1, 48000);
  else
    d->audioStreamInfo.codec = SAudioCodec("*", SAudioFormat::Channels_Stereo, 48000);

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

STime SPlaylistNode::duration(void) const
{
  return d->duration;
}

bool SPlaylistNode::setPosition(STime pos)
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

void SPlaylistNode::selectStreams(const QVector<StreamId> &)
{
}

bool SPlaylistNode::start(void)
{
  if (!d->fileNames.isEmpty())
  if (SFileInputNode::start())
  {
    d->fileId = -1;

    return openNext();
  }

  return false;
}

void SPlaylistNode::stop(void)
{
  SFileInputNode::stop();

  if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    emit closed(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);

  d->fileId = -1;
}

void SPlaylistNode::endReached(void)
{
  if (!openNext())
    SFileInputNode::endReached();
}

bool SPlaylistNode::openNext(void)
{
  SFileInputNode::stop();

  if ((d->fileId >= 0) && (d->fileId < d->fileNames.count()))
    emit closed(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);

  for (d->fileId++; d->fileId < d->fileNames.count(); d->fileId++)
  {
    setFileName(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);
    if (SFileInputNode::start())
    {
      emit opened(d->fileNames[d->fileId].first, d->fileNames[d->fileId].second);
      return true;
    }
  }

  d->fileId = -1;
  return false;
}

} // End of namespace
