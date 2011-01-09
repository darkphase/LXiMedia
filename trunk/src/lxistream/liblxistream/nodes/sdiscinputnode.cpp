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

#include "nodes/sdiscinputnode.h"
#include "sdebug.h"
#include "sgraph.h"

namespace LXiStream {

struct SDiscInputNode::Data
{
  bool                          opened;
  SInterfaces::DiscReader     * discReader;
  SInterfaces::BufferReader   * bufferReader;
  SInterfaces::BufferReader::ReadCallback * readCallback;
};

SDiscInputNode::SDiscInputNode(SGraph *parent, const QString &path)
  : QObject(parent),
    SInterfaces::SourceNode(parent),
    d(new Data())
{
  d->opened = false;
  d->discReader = NULL;
  d->bufferReader = NULL;
  d->readCallback = NULL;

  // Detect disc format.
  QMultiMap<int, QString> formats;
  foreach (SInterfaces::DiscFormatProber *prober, SInterfaces::DiscFormatProber::create(this))
  {
    foreach (const SInterfaces::DiscFormatProber::Format &format, prober->probeFormat(path))
      formats.insert(-format.confidence, format.name);

    delete prober;
  }

  // Now try to open a parser.
  foreach (const QString &format, formats)
  {
    d->discReader = SInterfaces::DiscReader::create(this, format, path, false);
    if (d->discReader)
      break;
  }
}

SDiscInputNode::~SDiscInputNode()
{
  if (d->readCallback && d->discReader)
    d->discReader->closeTitle(d->readCallback);

  delete d->bufferReader;
  delete d->discReader;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

unsigned SDiscInputNode::numTitles(void) const
{
  if (d->discReader)
    return d->discReader->numTitles();

  return 0;
}

bool SDiscInputNode::openTitle(unsigned title)
{
  if (d->discReader)
  {
    d->readCallback = d->discReader->openTitle(title);
    if (d->readCallback)
    {
      // Detect format.
      QByteArray buffer(SInterfaces::FileFormatProber::defaultProbeSize, 0);
      const qint64 size = d->readCallback->read(reinterpret_cast<uchar *>(buffer.data()), buffer.size());
      if (size > 0)
      {
        d->readCallback->seek(0, SEEK_SET);
        buffer.resize(size);

        QMultiMap<int, QString> formats;
        foreach (SInterfaces::FileFormatProber *prober, SInterfaces::FileFormatProber::create(this))
        {
          foreach (const SInterfaces::FileFormatProber::Format &format, prober->probeFormat(buffer))
            formats.insert(-format.confidence, format.name);

          delete prober;
        }

        // Now try to open a parser.
        foreach (const QString &format, formats)
        {
          d->bufferReader = SInterfaces::BufferReader::create(this, format, false);
          if (d->bufferReader)
          {
            if (d->bufferReader->start(d->readCallback, this, false))
              return d->opened = true;

            delete d->bufferReader;
            d->bufferReader = NULL;
          }
        }
      }
    }
  }

  return false;
}

STime SDiscInputNode::duration(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->duration();

  return STime();
}

bool SDiscInputNode::setPosition(STime pos)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->setPosition(pos);

  return false;
}

STime SDiscInputNode::position(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->position();

  return STime();
}

QList<SDiscInputNode::AudioStreamInfo> SDiscInputNode::audioStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->audioStreams();

  return QList<AudioStreamInfo>();
}

QList<SDiscInputNode::VideoStreamInfo> SDiscInputNode::videoStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->videoStreams();

  return QList<VideoStreamInfo>();
}

QList<SDiscInputNode::DataStreamInfo> SDiscInputNode::dataStreams(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    return d->bufferReader->dataStreams();

  return QList<DataStreamInfo>();
}

void SDiscInputNode::selectStreams(const QList<quint16> &streamIds)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
    d->bufferReader->selectStreams(streamIds);
}

bool SDiscInputNode::start(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!d->opened)
    openTitle(0);

  return d->opened;
}

void SDiscInputNode::stop(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
  {
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;
  }
}

void SDiscInputNode::process(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (d->bufferReader)
  {
    if (d->bufferReader->process())
      return;

    // Finished; close input.
    d->bufferReader->stop();

    delete d->bufferReader;
    d->bufferReader = NULL;

    emit output(SEncodedAudioBuffer());
    emit output(SEncodedVideoBuffer());
    emit output(SEncodedDataBuffer());
    emit finished();
  }
}

void SDiscInputNode::produce(const SEncodedAudioBuffer &buffer)
{
  emit output(buffer);
}

void SDiscInputNode::produce(const SEncodedVideoBuffer &buffer)
{
  emit output(buffer);
}

void SDiscInputNode::produce(const SEncodedDataBuffer &buffer)
{
  emit output(buffer);
}


} // End of namespace
