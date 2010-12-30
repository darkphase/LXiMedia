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

#include <liblximediacenter/httpoutputnode.h>

namespace LXiMediaCenter {

struct HttpOutputNode::Data
{
  struct Socket
  {
    QAbstractSocket           * socket;
    bool                        sendCache;
  };

  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  inline Data(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  QVector<Socket>               sockets;
  QByteArray                    header;

  float                         streamingSpeed;
  STime                         streamingPreload;
  STimer                        streamTimer;

  SInterfaces::BufferWriter   * bufferWriter;

  bool                          caching;
  QByteArray                    cache;
  QTime                         cacheTimer;
};

HttpOutputNode::HttpOutputNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::SinkNode(parent),
    d(new Data())
{
  d->streamingSpeed = 0.0f;
  d->bufferWriter = NULL;
  d->caching = true;

  // Default header.
  QHttpResponseHeader header(200);
  header.setValue("Cache-Control", "no-cache");
  d->header = header.toString().toUtf8();
}

HttpOutputNode::~HttpOutputNode()
{
  delete d->bufferWriter;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void HttpOutputNode::setHeader(const QByteArray &header)
{
  d->header = header;
}

void HttpOutputNode::enablePseudoStreaming(float speed, STime preload)
{
  d->streamingSpeed = speed;
  d->streamingPreload = preload;
}

bool HttpOutputNode::addSocket(QAbstractSocket *socket)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  if (d->caching || d->sockets.isEmpty())
  {
    Data::Socket s;
    s.socket = socket;
    s.sendCache = true;

    s.socket->moveToThread(thread());
    s.socket->setParent(this);

    d->sockets += s;

    return true;
  }

  return false;
}

bool HttpOutputNode::openFormat(const QString &format, const SAudioCodec &audioCodec, STime duration)
{
  return openFormat(format,
                    QList<SAudioCodec>() << audioCodec,
                    QList<SVideoCodec>(),
                    duration);
}

bool HttpOutputNode::openFormat(const QString &format, const SAudioCodec &audioCodec, const SVideoCodec &videoCodec, STime duration)
{
  return openFormat(format,
                    QList<SAudioCodec>() << audioCodec,
                    QList<SVideoCodec>() << videoCodec,
                    duration);
}

bool HttpOutputNode::openFormat(const QString &format, const QList<SAudioCodec> &audioCodecs, const QList<SVideoCodec> &videoCodecs, STime duration)
{
  delete d->bufferWriter;
  d->bufferWriter = SInterfaces::BufferWriter::create(this, format, false);

  if (d->bufferWriter)
    return d->bufferWriter->createStreams(audioCodecs, videoCodecs, duration);

  return false;
}

bool HttpOutputNode::start(STimer *)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  if (d->bufferWriter)
  if (d->bufferWriter->start(this))
  {
    d->caching = true;
    d->cache.reserve(SIOOutputNode::outBufferSize * 8);
    d->cache.append(d->header);
    d->cacheTimer.start();

    return true;
  }

  return false;
}

void HttpOutputNode::stop(void)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  d->caching = false;
  d->cache.clear();

  if (d->bufferWriter)
    d->bufferWriter->stop();

  foreach (const Data::Socket &s, d->sockets)
  if (s.socket->state() != QAbstractSocket::UnconnectedState)
  {
    while (s.socket->bytesToWrite() > 0)
    if (!s.socket->waitForBytesWritten(SIOOutputNode::outBufferDelay))
      break;

    s.socket->disconnectFromHost();
    if (s.socket->state() != QAbstractSocket::UnconnectedState)
      s.socket->waitForDisconnected(SIOOutputNode::outBufferDelay);
  }

  d->sockets.clear();
}

bool HttpOutputNode::isConnected(void) const
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  if (d->caching)
    return true;

  foreach (const Data::Socket &s, d->sockets)
  if (s.socket->state() == QAbstractSocket::ConnectedState)
    return true;

  return false;
}

void HttpOutputNode::input(const SEncodedAudioBuffer &buffer)
{
  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void HttpOutputNode::input(const SEncodedVideoBuffer &buffer)
{
  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void HttpOutputNode::input(const SEncodedDataBuffer &buffer)
{
  if (!qFuzzyCompare(d->streamingSpeed, 0.0f))
    blockUntil(buffer.decodingTimeStamp().isValid() ? buffer.decodingTimeStamp() : buffer.presentationTimeStamp());

  if (d->bufferWriter)
    d->bufferWriter->process(buffer);
}

void HttpOutputNode::write(const uchar *buffer, qint64 size)
{
  SDebug::MutexLocker l(&d->mutex, __FILE__, __LINE__);

  if (d->caching)
  {
    for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); s++)
    if (s->sendCache && (s->socket->state() == QAbstractSocket::ConnectedState))
    {
      //qDebug() << "Reuse" << d->cache.size();
      s->socket->write(d->cache);
      s->sendCache = false;
    }

    if ((qAbs(d->cacheTimer.elapsed()) < 5000) &&
        ((d->cache.size() + size) < (SIOOutputNode::outBufferSize * 8)))
    {
      d->cache.append(QByteArray(reinterpret_cast<const char *>(buffer), size));
    }
    else
    {
      d->cache.clear();
      d->caching = false;
    }
  }

  for (QVector<Data::Socket>::Iterator s=d->sockets.begin(); s!=d->sockets.end(); )
  if (s->socket->state() == QAbstractSocket::ConnectedState)
  {
    while (s->socket->bytesToWrite() >= SIOOutputNode::outBufferSize)
    if (!s->socket->waitForBytesWritten(-1))
      break; // Error.

    if (!s->sendCache && (s->socket->state() == QAbstractSocket::ConnectedState))
    for (qint64 i=0; i<size; )
    {
      const qint64 r = s->socket->write((char *)buffer + i, size - i);
      if (r > 0)
        i += r;
      else
        break;
    }

    s++;
  }
  else
    s = d->sockets.erase(s);

  if (!isConnected())
  {
    emit disconnected();
  }
  else if (d->sockets.isEmpty()) // No connections, but still caching.
  {
    l.unlock();
    Data::T::msleep(SIOOutputNode::outBufferDelay / 20);
  }
}

void HttpOutputNode::blockUntil(STime timeStamp)
{
  static const int maxDelay = 250;

  if (timeStamp >= d->streamingPreload)
  {
    const STime correctedTime = STime::fromMSec(qint64(float(timeStamp.toMSec()) / d->streamingSpeed));

    // This blocks the thread until it is time to process the buffer. The
    // timestamp is divided by 2 to allow processing 2 times realtime.
    const STime duration = d->streamTimer.correctOffset(correctedTime, STime::fromMSec(maxDelay));
    if (duration.isPositive())
      Data::T::msleep(qBound(0, int(duration.toMSec()), maxDelay));
  }
}

} // End of namespace
