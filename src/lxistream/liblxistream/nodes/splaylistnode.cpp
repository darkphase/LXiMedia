/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "nodes/splaylistnode.h"
#include "nodes/sfileinputnode.h"

namespace LXiStream {


struct SPlaylistNode::Data
{
  QList<QUrl>                   files;
  int                           fileId;

  QList<AudioStreamInfo>        audioStreams;
  QList<VideoStreamInfo>        videoStreams;
};

SPlaylistNode::SPlaylistNode(SGraph *parent, const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType fileType)
  : SFileInputNode(parent, QUrl()),
    d(new Data())
{
  setFiles(files, fileType);
}

SPlaylistNode::~SPlaylistNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SPlaylistNode::setFiles(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType fileType)
{
  d->files = files;
  d->fileId = -1;

  d->audioStreams.clear();
  d->videoStreams.clear();

  if ((fileType == SMediaInfo::ProbeInfo::FileType_Audio) ||
      (fileType == SMediaInfo::ProbeInfo::FileType_Video))
  {
    AudioStreamInfo streamInfo;
    streamInfo.codec.setChannelSetup(SAudioFormat::Channels_Stereo);
    streamInfo.codec.setSampleRate(48000);
    d->audioStreams += streamInfo;
  }

  if (fileType == SMediaInfo::ProbeInfo::FileType_Video)
  {
    VideoStreamInfo streamInfo;
    streamInfo.codec.setFrameRate(SInterval::fromFrequency(25));
    streamInfo.codec.setSize(SSize(1280, 720));
    d->videoStreams += streamInfo;
  }
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

QList<SPlaylistNode::AudioStreamInfo> SPlaylistNode::audioStreams(int) const
{
  return d->audioStreams;
}

QList<SPlaylistNode::VideoStreamInfo> SPlaylistNode::videoStreams(int) const
{
  return d->videoStreams;
}

QList<SPlaylistNode::DataStreamInfo>  SPlaylistNode::dataStreams(int) const
{
  return QList<DataStreamInfo>();
}

void SPlaylistNode::selectStreams(int, const QVector<StreamId> &)
{
}

bool SPlaylistNode::start(void)
{
  d->fileId = -1;

  return openNext();
}

void SPlaylistNode::stop(void)
{
  SFileInputNode::stop();

  if ((d->fileId >= 0) && (d->fileId < d->files.count()))
    emit closed(d->files[d->fileId]);

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

  if ((d->fileId >= 0) && (d->fileId < d->files.count()))
    emit closed(d->files[d->fileId]);

  for (d->fileId++; d->fileId < d->files.count(); d->fileId++)
  {
    setFilePath(d->files[d->fileId]);

    if (d->audioStreams.isEmpty() || !SFileInputNode::audioStreams(0).isEmpty())
    if (d->videoStreams.isEmpty() || !SFileInputNode::videoStreams(0).isEmpty())
    if (SFileInputNode::start())
    {
      emit opened(d->files[d->fileId]);
      return true;
    }
  }

  d->fileId = -1;
  return false;
}

} // End of namespace
