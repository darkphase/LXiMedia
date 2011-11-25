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

  void                          start(void);

protected:
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);

public:
  static const QEvent::Type     bufferEventType;
  static const QEvent::Type     stopThreadEventType;

private:
  SNetworkInputNode     * const parent;
  bool                          running;
};

struct SNetworkInputNode::Data
{
  static const int              timeslot = 250;

  QUrl                          url;
  QMutex                        bufferMutex;
  STime                         bufferDuration;
  float                         bufferSize;
  bool                          bufferReady;

  BufferThread                * bufferThread;
};

SNetworkInputNode::SNetworkInputNode(SGraph *parent, const QUrl &url)
  : SInputNode(parent),
    d(new Data())
{
  d->bufferDuration = STime::fromSec(10);
  d->bufferSize = 0.0f;
  d->bufferReady = false;
  d->bufferThread = NULL;

  setUrl(url);
}

SNetworkInputNode::~SNetworkInputNode()
{
  d->bufferMutex.lock();

  delete d->bufferThread;
  delete bufferReader();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void SNetworkInputNode::setUrl(const QUrl &url)
{
  QMutexLocker l(&d->bufferMutex);

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
  QMutexLocker l(&d->bufferMutex);

  SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(SInputNode::bufferReader());
  if (bufferReader && d->url.isValid() && !d->url.isEmpty())
  if (bufferReader->start(d->url, this))
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
  QMutexLocker l(&d->bufferMutex);

  SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(SInputNode::bufferReader());
  if (bufferReader)
    bufferReader->stop();

  SInputNode::stop();

  delete d->bufferThread;
  d->bufferThread = new BufferThread(this);
}

bool SNetworkInputNode::process(void)
{
  QMutexLocker l(&d->bufferMutex);

  if (SInputNode::process())
  {
    qApp->postEvent(d->bufferThread, new QEvent(BufferThread::bufferEventType));

    return true;
  }

  return false;
}

const QEvent::Type  SNetworkInputNode::BufferThread::bufferEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  SNetworkInputNode::BufferThread::stopThreadEventType = QEvent::Type(QEvent::registerEventType());

SNetworkInputNode::BufferThread::BufferThread(SNetworkInputNode *parent)
  : QThread(),
    parent(parent),
    running(false)
{
  QThread::moveToThread(this);
}

SNetworkInputNode::BufferThread::~BufferThread()
{
  if (running)
    qApp->postEvent(this, new QEvent(stopThreadEventType), Qt::HighEventPriority);

  wait();
}

void SNetworkInputNode::BufferThread::start(void)
{
  running = true;

  QThread::start();
}

void SNetworkInputNode::BufferThread::run(void)
{
  exec();
}

void SNetworkInputNode::BufferThread::customEvent(QEvent *e)
{
  if (e->type() == bufferEventType)
  {
    if (running)
    if (parent->d->bufferMutex.tryLock(parent->d->timeslot / 2))
    {
      QTime timer; timer.start();

      SInterfaces::NetworkBufferReader * const bufferReader = static_cast<SInterfaces::NetworkBufferReader *>(parent->bufferReader());
      if (bufferReader)
      {
        STime duration;
        while (((duration = bufferReader->bufferDuration()) < parent->d->bufferDuration) &&
               (qAbs(timer.elapsed()) < (parent->d->timeslot / 4)))
        {
          bufferReader->buffer();
        }

        if (parent->d->bufferReady && (duration.toMSec() < parent->d->timeslot * 2))
        {
          parent->d->bufferReady = false;
          parent->d->bufferDuration *= 2;
        }
        else if (duration >= parent->d->bufferDuration)
          parent->d->bufferReady = true;

        parent->d->bufferSize = float(duration.toMSec()) / float(parent->d->bufferDuration.toMSec());
      }

      parent->d->bufferMutex.unlock();

      // Give the processor some time to process received buffers.
      msleep(parent->d->timeslot / 2);

      if (!parent->d->bufferReady)
        qApp->postEvent(this, new QEvent(BufferThread::bufferEventType));
    }
  }
  else if (e->type() == stopThreadEventType)
  {
    running = false;

    exit();
  }
  else
    QThread::customEvent(e);
}

} // End of namespace
