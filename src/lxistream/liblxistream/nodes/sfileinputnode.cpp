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

#include "nodes/sfileinputnode.h"
#include "smediafilesystem.h"
#include "ssubtitlefile.h"
#include "svideobuffer.h"
#include <LXiCore>

namespace LXiStream {


struct SFileInputNode::Data
{
  QIODevice                   * ioDevice;
  QUrl                          filePath;
  QMap<StreamId, QUrl>          subtitleStreams;
  SSubtitleFile               * subtitleFile;
  SEncodedDataBuffer            nextSubtitle;
};

SFileInputNode::SFileInputNode(SGraph *parent, const QUrl &filePath)
  : SIOInputNode(parent),
    d(new Data())
{
  d->ioDevice = NULL;
  d->subtitleFile = NULL;

  setFilePath(filePath);
}

SFileInputNode::~SFileInputNode()
{
  delete d->ioDevice;
  delete d->subtitleFile;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SFileInputNode::setFilePath(const QUrl &filePath)
{
  delete d->ioDevice;
  d->filePath = filePath;
  d->ioDevice = SMediaFilesystem::open(filePath);

  if (d->ioDevice)
    SIOInputNode::setIODevice(d->ioDevice);
}

QUrl SFileInputNode::filePath(void) const
{
  return d->filePath;
}

bool SFileInputNode::setPosition(STime pos)
{
  if (d->subtitleFile)
    d->subtitleFile->reset();

  return SIOInputNode::setPosition(pos);
}

QList<SFileInputNode::DataStreamInfo> SFileInputNode::dataStreams(int title) const
{
  QList<DataStreamInfo> dataStreams = SIOInputNode::dataStreams(title);

  // Add subtitles.
  quint16 nextStreamId = 0xF000;
  foreach (const QUrl &filePath, SSubtitleFile::findSubtitleFiles(d->filePath))
  {
    SSubtitleFile file(filePath);
    if (file.open())
    {
      DataStreamInfo stream(
          StreamId(StreamId::Type_Subtitle, nextStreamId++),
          file.language(),
          QString::null,
          file.codec());

      stream.file = filePath;
      dataStreams += stream;

      d->subtitleStreams.insert(stream, filePath);
    }
  }

  return dataStreams;
}

void SFileInputNode::selectStreams(int title, const QVector<StreamId> &streamIds)
{
  QVector<StreamId> nextStreamIds;
  foreach (StreamId id, streamIds)
  if (d->subtitleStreams.contains(id))
  {
    d->subtitleFile = new SSubtitleFile(d->subtitleStreams[id]);
    if (d->subtitleFile->open())
    {
      connect(this, SIGNAL(output(const SEncodedVideoBuffer &)), SLOT(parseSubtitle(const SEncodedVideoBuffer &)));
      d->nextSubtitle.clear();
      break; // Limit to one subtitle file.
    }
    else
    {
      delete d->subtitleFile;
      d->subtitleFile = NULL;
    }
  }
  else
    nextStreamIds += id;

  SIOInputNode::selectStreams(title, nextStreamIds);
}

void SFileInputNode::stop(void)
{
  SIOInputNode::stop();

  SIOInputNode::setIODevice(NULL);
  delete d->ioDevice;
  d->ioDevice = NULL;

  if (d->subtitleFile)
  {
    disconnect(this, SIGNAL(output(const SEncodedVideoBuffer &)), this, SLOT(parseSubtitle(const SEncodedVideoBuffer &)));
    delete d->subtitleFile;
    d->subtitleFile = NULL;

    d->nextSubtitle.clear();
  }
}

void SFileInputNode::parseSubtitle(const SEncodedVideoBuffer &videoBuffer)
{
  // parse subtitle (subtitles are sent slightly ahead of video)
  if (d->subtitleFile)
  {
    const STime subtitleTime =
        (videoBuffer.presentationTimeStamp().isValid()
         ? videoBuffer.presentationTimeStamp()
         : videoBuffer.decodingTimeStamp());

    if (d->nextSubtitle.isNull())
    {
      d->nextSubtitle = d->subtitleFile->readSubtitle(subtitleTime);
    }
    else if (d->nextSubtitle.presentationTimeStamp() < subtitleTime)
    {
      emit output(d->nextSubtitle);
      d->nextSubtitle = d->subtitleFile->readSubtitle(subtitleTime);
    }
  }
}


} // End of namespace
