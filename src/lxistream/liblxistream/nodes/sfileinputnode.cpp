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

#include "nodes/sfileinputnode.h"
#include "ssubtitlefile.h"
#include "svideobuffer.h"
#include <LXiCore>

namespace LXiStream {


struct SFileInputNode::Data
{
  inline Data(const QString &path) : mediaFile(path), subtitleFile(NULL) { }

  QFile                         mediaFile;
  QMap<StreamId, QString>       subtitleStreams;
  SSubtitleFile               * subtitleFile;
  SEncodedDataBuffer            nextSubtitle;
};

SFileInputNode::SFileInputNode(SGraph *parent, const QString &fileName)
  : SIOInputNode(parent, NULL, fileName),
    d(new Data(path))
{
}

SFileInputNode::~SFileInputNode()
{
  delete d->subtitleFile;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SFileInputNode::setFileName(const QString &fileName)
{
  d->mediaFile.close();
  d->mediaFile.setFileName(fileName);
}

bool SFileInputNode::open(quint16 programId)
{
  if (d->mediaFile.open(QFile::ReadOnly))
  {
    SIOInputNode::setIODevice(&d->mediaFile);

    if (SIOInputNode::open(programId))
      return true;

    d->mediaFile.close();
  }
  else if (SIOInputNode::open(programId))
    return true;

  return false;
}

void SFileInputNode::stop(void)
{
  SIOInputNode::stop();

  SIOInputNode::setIODevice(NULL);
  d->mediaFile.close();

  if (d->subtitleFile)
  {
    disconnect(this, SIGNAL(output(const SEncodedVideoBuffer &)), this, SLOT(parseSubtitle(const SEncodedVideoBuffer &)));
    delete d->subtitleFile;
    d->subtitleFile = NULL;

    d->nextSubtitle.clear();
  }
}

bool SFileInputNode::setPosition(STime pos)
{
  if (d->subtitleFile)
    d->subtitleFile->reset();

  return SIOInputNode::setPosition(pos);
}

QList<SFileInputNode::DataStreamInfo> SFileInputNode::dataStreams(void) const
{
  QList<DataStreamInfo> dataStreams = SIOInputNode::dataStreams();

  // Add subtitles.
  quint16 nextStreamId = 0xF000;
  foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(d->mediaFile.fileName()))
  {
    SSubtitleFile file(fileName);
    if (file.open())
    {
      DataStreamInfo stream(
          StreamId(StreamId::Type_Subtitle, nextStreamId++),
          file.language(),
          QString::null,
          file.codec());

      stream.file = fileName;
      dataStreams += stream;

      d->subtitleStreams.insert(stream, fileName);
    }
  }

  return dataStreams;
}

void SFileInputNode::selectStreams(const QVector<StreamId> &streamIds)
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

  SIOInputNode::selectStreams(nextStreamIds);
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
