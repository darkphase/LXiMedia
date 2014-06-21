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

#include "nodes/sfileinputnode.h"
#include "nodes/ssubtitleinputnode.h"
#include "smediafilesystem.h"
#include "svideobuffer.h"
#include <LXiCore>

namespace LXiStream {


struct SFileInputNode::Data
{
  struct SubtitleFile
  {
    inline                      SubtitleFile(void) : node(NULL) { }
    inline explicit             SubtitleFile(SSubtitleInputNode *node) : node(node) { }

    SSubtitleInputNode        * node;
    QMap<int, QMap<StreamId, DataStreamInfo> > streams;
  };

  QIODevice                   * ioDevice;
  QUrl                          filePath;
  bool                          timeShift;

  QList<SubtitleFile>           subtitleFiles;
};

SFileInputNode::SFileInputNode(SGraph *parent, const QUrl &filePath)
  : SIOInputNode(parent),
    d(new Data())
{
  d->ioDevice = NULL;
  d->timeShift = false;

  setFilePath(filePath);

  connect(this, SIGNAL(output(const SEncodedVideoBuffer &)), SLOT(parseSubtitle(const SEncodedVideoBuffer &)));
}

SFileInputNode::~SFileInputNode()
{
  SIOInputNode::setIODevice(NULL);
  delete d->ioDevice;

  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
    delete file.node;

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SFileInputNode::setFilePath(const QUrl &filePath)
{
  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
    delete file.node;

  SIOInputNode::setIODevice(NULL);
  delete d->ioDevice;

  d->subtitleFiles.clear();

  d->filePath = filePath;
  d->ioDevice = SMediaFilesystem::open(filePath, QIODevice::ReadOnly);

  if (d->ioDevice)
  {
    quint16 nextStreamId = 0xF000;

    foreach (const QUrl &filePath, SSubtitleInputNode::findSubtitleFiles(d->filePath))
    {
      SSubtitleInputNode * const node = new SSubtitleInputNode(NULL, filePath);
      if (node->numTitles() > 0)
      {
        Data::SubtitleFile file(node);
        for (int i=0; i<node->numTitles(); i++)
        {
          foreach (const DataStreamInfo &stream, node->dataStreams(i))
          if ((stream.type & ~StreamId::Type_Flags) == StreamId::Type_Subtitle)
            file.streams[i].insert(StreamId(stream.type, nextStreamId++), stream);
        }

        connect(node, SIGNAL(output(const SEncodedDataBuffer &)), SIGNAL(output(const SEncodedDataBuffer &)));

        d->subtitleFiles += file;
      }
      else
        delete node;
    }

    SIOInputNode::setIODevice(d->ioDevice);
  }
}

QUrl SFileInputNode::filePath(void) const
{
  return d->filePath;
}

void SFileInputNode::enableTimeShift(bool enabled)
{
  d->timeShift = enabled;
}

bool SFileInputNode::setPosition(STime pos)
{
  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
    file.node->setPosition(pos);

  return SIOInputNode::setPosition(pos);
}

QList<SFileInputNode::DataStreamInfo> SFileInputNode::dataStreams(int title) const
{
  QList<DataStreamInfo> dataStreams = SIOInputNode::dataStreams(title);

  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
  {
    QMap<int, QMap<StreamId, DataStreamInfo> >::ConstIterator streams = file.streams.find(title);
    if (streams != file.streams.end())
    for (QMap<StreamId, DataStreamInfo>::ConstIterator i = streams->begin(); i != streams->end(); i++)
    {
      DataStreamInfo s = *i;
      s.id = i.key().id;

      dataStreams += s;
    }
  }

  return dataStreams;
}

void SFileInputNode::selectStreams(int title, const QVector<StreamId> &streamIds)
{
  QVector<StreamId> nextStreamIds;
  foreach (StreamId streamId, streamIds)
  if (streamId.id < 0xF000)
    nextStreamIds += streamId;

  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
  {
    QVector<StreamId> subtitleStreamIds;

    QMap<int, QMap<StreamId, DataStreamInfo> >::ConstIterator streams = file.streams.find(title);
    if (streams != file.streams.end())
    foreach (StreamId streamId, streamIds)
    {
      QMap<StreamId, DataStreamInfo>::ConstIterator stream = streams->find(streamId);
      if (stream != streams->end())
        subtitleStreamIds += *stream;
    }

    file.node->selectStreams(title, subtitleStreamIds);
  }

  SIOInputNode::selectStreams(title, nextStreamIds);
}

bool SFileInputNode::process(void)
{
  if (d->timeShift)
  {
    if (d->ioDevice)
      return SInputNode::process();

    return false;
  }
  else
    return SIOInputNode::process();
}

void SFileInputNode::parseSubtitle(const SEncodedVideoBuffer &videoBuffer)
{
  const STime subtitleTime =
      (videoBuffer.presentationTimeStamp().isValid()
       ? videoBuffer.presentationTimeStamp()
       : videoBuffer.decodingTimeStamp());

  foreach (const Data::SubtitleFile &file, d->subtitleFiles)
    file.node->process(subtitleTime);
}

} // End of namespace
