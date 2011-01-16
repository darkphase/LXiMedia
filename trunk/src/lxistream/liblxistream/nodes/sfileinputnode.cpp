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
#include "sdebug.h"
#include "sstringparser.h"
#include "ssubtitlefile.h"
#include "svideobuffer.h"

namespace LXiStream {


struct SFileInputNode::Data
{
  inline Data(const QString &fileName) : mediaFile(fileName), subtitleFile(NULL) { }

  QFile                         mediaFile;
  QMap<quint16, QString>        subtitleStreams;
  SSubtitleFile               * subtitleFile;
  SEncodedDataBuffer            nextSubtitle;
};

SFileInputNode::SFileInputNode(SGraph *parent, const QString &fileName)
  : SIOInputNode(parent, NULL),
    d(new Data(fileName))
{
  SIOInputNode::setIODevice(&d->mediaFile);
}

SFileInputNode::~SFileInputNode()
{
  delete d->subtitleFile;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SFileInputNode::open(void)
{
  if (d->mediaFile.open(QIODevice::ReadOnly))
  {
    if (SIOInputNode::open())
      return true;

    d->mediaFile.close();
  }

  return false;
}

void SFileInputNode::stop(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  SIOInputNode::stop();

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
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->subtitleFile)
    d->subtitleFile->reset();

  return SIOInputNode::setPosition(pos);
}

QList<SFileInputNode::DataStreamInfo> SFileInputNode::dataStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  QList<DataStreamInfo> dataStreams = SIOInputNode::dataStreams();

  // Add subtitles.
  quint16 nextStreamId = 0xF000;
  foreach (const QString &fileName, SSubtitleFile::findSubtitleFiles(d->mediaFile.fileName()))
  {
    SSubtitleFile file(fileName);
    if (file.open())
    {
      d->subtitleStreams.insert(nextStreamId, fileName);

      DataStreamInfo stream(nextStreamId++, file.language(), file.codec());
      stream.file = fileName;
      dataStreams += stream;
    }
  }

  return dataStreams;
}

void SFileInputNode::selectStreams(const QList<quint16> &streamIds)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  QList<quint16> nextStreamIds;
  foreach (quint16 id, streamIds)
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
