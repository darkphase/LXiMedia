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

#include "nodes/snetworkinputnode.h"
#include <LXiCore>

namespace LXiStream {

class SNetworkInputNode::BufferThread : public QThread
{
public:
                                BufferThread(SNetworkInputNode *parent);
  virtual                       ~BufferThread();

protected:
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);

public:
  static const QEvent::Type     bufferEventType;
  static const QEvent::Type     stopThreadEventType;

private:
  SNetworkInputNode     * const parent;
};

struct SNetworkInputNode::Data
{
  QUrl                          url;
  quint16                       programId;
  QMutex                        bufferMutex;
  STime                         bufferDuration;
  float                         bufferSize;
  bool                          bufferReady;

  BufferThread                * bufferThread;
};

SNetworkInputNode::SNetworkInputNode(SGraph *parent, const QUrl &url, quint16 programId)
  : SInputNode(parent),
    d(new Data())
{
  d->programId = 0;
  d->bufferDuration = STime::fromSec(5);
  d->bufferSize = 0.0f;
  d->bufferReady = false;
  d->bufferThread = NULL;

  setUrl(url, programId);
}

SNetworkInputNode::~SNetworkInputNode()
{
  delete d->bufferThread;
  delete bufferReader();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SNetworkInputNode::setUrl(const QUrl &url, quint16 programId)
{
  if (bufferReader())
  {
    delete bufferReader();
    setBufferReader(NULL);
  }

  d->bufferSize = 0.0f;
  d->bufferReady = false;

  delete d->bufferThread;
  d->bufferThread = NULL;

  if (url.isValid() && !url.isEmpty())
  {
    SInterfaces::NetworkBufferReader * const bufferReader = SInterfaces::NetworkBufferReader::create(this, url.scheme(), false);
    if (bufferReader)
    {
      d->url = url;
      d->programId = programId;

      d->bufferThread = new BufferThread(this);

      setBufferReader(bufferReader);
    }
  }
}

QUrl SNetworkInputNode::url(void) const
{
  return d->url;
}

void SNetworkInputNode::setBufferDuration(const STime &bufferDuration)
{
  d->bufferDuration = bufferDuration;
}

STime SNetworkInputNode::bufferDuration(void) const
{
  return d->bufferDuration;
}

bool SNetworkInputNode::fillBuffer(void)
{
  qApp->postEvent(d->bufferThread, new QEvent(BufferThread::bufferEventType));

  return d->bufferReady;
}

bool SNetworkInputNode::bufferReady(void) const
{
  return d->bufferReady;
}

float SNetworkInputNode::bufferProgress(void) const
{
  return d->bufferSize;
}

bool SNetworkInputNode::start(void)
{
  SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(SInputNode::bufferReader());
  if (bufferReader && d->url.isValid() && !d->url.isEmpty())
  if (bufferReader->start(d->url, this, d->programId))
  if (SInputNode::start())
  {
    if (!d->bufferThread->isRunning())
      d->bufferThread->start();

    return true;
  }

  return false;
}

void SNetworkInputNode::stop(void)
{
  SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(SInputNode::bufferReader());
  if (bufferReader)
    bufferReader->stop();

  SInputNode::stop();
}

bool SNetworkInputNode::process(void)
{
  bool result = false;
  if (d->bufferMutex.tryLock(0))
  {
    result = SInputNode::process();

    d->bufferMutex.unlock();

    if (result)
      qApp->postEvent(d->bufferThread, new QEvent(BufferThread::bufferEventType));
  }

  return result;
}

const QEvent::Type  SNetworkInputNode::BufferThread::bufferEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SNetworkInputNode::BufferThread::stopThreadEventType = QEvent::Type(QEvent::registerEventType());

SNetworkInputNode::BufferThread::BufferThread(SNetworkInputNode *parent)
  : QThread(parent),
    parent(parent)
{
}

SNetworkInputNode::BufferThread::~BufferThread()
{
  if (isRunning())
  {
    qApp->postEvent(this, new QEvent(stopThreadEventType));
    if (!wait(5000))
      terminate();
  }
}

void SNetworkInputNode::BufferThread::run(void)
{
  setTerminationEnabled();

  exec();
}

void SNetworkInputNode::BufferThread::customEvent(QEvent *e)
{
  if (e->type() == bufferEventType)
  {
    QMutexLocker l(&parent->d->bufferMutex);

    SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(parent->bufferReader());
    if (bufferReader)
    {
      QTime timer; timer.start();
      while (qAbs(timer.elapsed()) < 10)
      if (bufferReader->bufferDuration() < parent->d->bufferDuration)
        bufferReader->buffer();
      else
        break;

      const STime duration = bufferReader->bufferDuration();
      if (duration < parent->d->bufferDuration)
      {
        if (parent->d->bufferReady && (duration.toMSec() < 250))
          parent->d->bufferReady = false;
      }
      else
        parent->d->bufferReady = true;

      parent->d->bufferSize = float(duration.toMSec()) / float(parent->d->bufferDuration.toMSec());
      qDebug() << parent->d->bufferSize;
    }
  }
  else if (e->type() == stopThreadEventType)
    exit();
  else
    QThread::customEvent(e);
}

} // End of namespace
