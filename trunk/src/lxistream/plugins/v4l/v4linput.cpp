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

#include "v4linput.h"

#include <lxstream/backend/mediatimer.h>
#include <lxstream/backend/videoconsumer.h>
#include <lxstream/backend/videobuffer.h>

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#include "vbiinput.h"

namespace LXiStream {
namespace V4lBackend {


V4lInput::Producer::Producer(V4lInput *parent)
                   :VideoProducer(JobScheduler::NO_JOB),
                    numBuffers(0),
                    captureBuffers(NULL),
                    frameCounter(0),
                    mediaTimer(),
                    availableBuffers(),
                    producedBuffers(),
                    parent(parent),
                    running(false),
                    lastTime(0),
                    sourceTime(0),
                    avgFrameDeltaT(40),
                    vbiInput(NULL)
{
}

V4lInput::Producer::~Producer()
{
  stopStream();

  ScopeLock l(parent);

  if (!parent->isV4L1)
  {
    for(int i=0; i<numBuffers; i++)
    {
      munmap((void *)captureBuffers[i].data,
            captureBuffers[i].vidbuf.length);
    }

    delete[] captureBuffers;
    captureBuffers = NULL;
  }
}

Object::Ptr<DataProducer> V4lInput::Producer::getDataProducer(void) const
{
  return static_cast<DataProducer *>(vbiInput);
}

Object::Ptr<AudioProducer> V4lInput::Producer::getAudioProducer(void) const
{
  return NULL;
}

Object::Ptr<VideoProducer> V4lInput::Producer::getVideoProducer(void) const
{
  return const_cast<Producer *>(this);
}

void V4lInput::Producer::setProgram(const Stream::Program &)
{
}

bool V4lInput::Producer::startStream(void)
{
  ScopeLock l(parent);

  lastTime = 0;
  sourceTime = 0;
  avgFrameDeltaT = 40;

  if (!parent->isV4L1)
  {
    if (ioctl(parent->devDesc, VIDIOC_STREAMON, &captureBuffers[0].vidbuf.type ) >= 0)
    {
      l.unlock();
        queueAllBuffers();
      l.relock();

      frameCounter = 0;
      running = true;
      QThread::start(QThread::TimeCriticalPriority);

      parent->tuner.setVolume(1.0);

      return VideoProducer::startStream();
    }
  }
  else // V4L1 compatibility
  {
    frameCounter = 0;
    running = true;
    QThread::start(QThread::TimeCriticalPriority);

    return VideoProducer::startStream();
  }

  return false;
}

bool V4lInput::Producer::stopStream(void)
{
  running = false;
  QThread::wait();

  bool retval = VideoProducer::stopStream();

  ScopeLock l(parent);

  if (!parent->isV4L1)
  {
    ioctl(parent->devDesc, VIDIOC_STREAMOFF, &captureBuffers[0].vidbuf.type);

    // Clear all buffers still queued
    producedBuffers.getAllBuffers();
  }

  return retval;
}

void V4lInput::Producer::generateBuffer(BufferType, bool async)
{
  Buffer::Ptr<VideoBuffer> buffer = producedBuffers.getBuffer(0);

  if (buffer != NULL)
    produceBuffer(buffer, async);
}

void V4lInput::Producer::objectReleased(Object *object)
{
  BufferPtr * const buffer = dynamic_cast<BufferPtr *>(object);

  if (buffer)
  {
    if (buffer->index >= 0)
      queueBuffer(buffer->index);

    ScopeLock l(parent);

    buffer->index = -1;
    availableBuffers.enqueue(buffer);
  }
}

double V4lInput::Producer::getDataQuality(void) const
{
  return vbiInput ? vbiInput->getDataQuality() : 1.0;
}

bool V4lInput::Producer::nextImage(const char *&pixelData, size_t &dataSize, int &index)
{
  if (numBuffers > 0)
  {
    // Wait for next buffer
    timeval timeout;
    fd_set rdset;
    int n;

    FD_ZERO(&rdset);
    FD_SET(parent->devDesc, &rdset);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    n = select(parent->devDesc + 1, &rdset, 0, 0, &timeout);
    if( n == -1 )
      return false;

    ScopeLock l(parent);

    v4l2_buffer cur_buf;
    cur_buf.type = captureBuffers[0].vidbuf.type;
    if(ioctl(parent->devDesc, VIDIOC_DQBUF, &cur_buf) < 0)
      return false; // A restart may recover

    // Got a buffer
    frameCounter++;
    pixelData = captureBuffers[cur_buf.index].data;
    dataSize = captureBuffers[cur_buf.index].vidbuf.length;
    index = cur_buf.index;

    return true;
  }

  return false;
}

bool V4lInput::Producer::readImage(void *pixelData, size_t &dataSize)
{
  if (pixelData)
  {
    const int bytesRead = read(parent->devDesc, reinterpret_cast<char *>(pixelData), dataSize);

    if (bytesRead > 0)
    {
      dataSize = bytesRead;
      return true;
    }
  }

  return false;
}

bool V4lInput::Producer::queueBuffer(int buffer)
{
  ScopeLock l(parent);

  if (ioctl(parent->devDesc, VIDIOC_QBUF, &captureBuffers[buffer].vidbuf) >= 0 )
    return true;

  return false;
}

bool V4lInput::Producer::queueAllBuffers()
{
  bool result = true;

  for(int i=0; i<numBuffers; i++)
    result &= queueBuffer(i);

  return result;
}

void V4lInput::Producer::run(void)
{
  if (!parent->isV4L1)
  {
    while (running)
    if (availableBuffers.count() > 0)
    {
      const quint64 currentTime = mediaTimer.getTimeStamp();

      if (qAbs(qint64(currentTime) - qint64(sourceTime)) >= qint64(80 * (MediaTimer::timeStampPrecision / 1000)))
        sourceTime = currentTime; // We lost some time somewhere, resync
      else if (qAbs(qint64(currentTime) - qint64(sourceTime)) >= qint64(10 * (MediaTimer::timeStampPrecision / 1000)))
        sourceTime += (qint64(currentTime) - qint64(sourceTime)) >> 3;

      if (qAbs(qint64(currentTime) - qint64(lastTime)) < qint64(500 * (MediaTimer::timeStampPrecision / 1000)))
        avgFrameDeltaT = ((avgFrameDeltaT << 2) + avgFrameDeltaT + avgFrameDeltaT + avgFrameDeltaT + (qint64(currentTime) - qint64(lastTime))) >> 3;

      const char *pixelData = NULL;
      size_t size = 0;
      int index = -1;
      if (nextImage(pixelData, size, index))
      {
        ScopeLock l(parent);
          Buffer::Ptr<BufferPtr> buffer = availableBuffers.dequeue();
        l.unlock();

        buffer->index = index;
        buffer->setBuffer(pixelData, size);
        buffer->setTimeStamp(sourceTime);
        buffer->setCodec(parent->codec);
        buffer->setFieldMode(parent->fieldMode);
        buffer->setWidth(parent->width);
        buffer->setHeight(parent->height);
        buffer->setWidthAspect(parent->getWidthAspect());
        buffer->setHeightAspect(parent->getHeightAspect());
        buffer->setLineSize(0, parent->width * PixelSize(parent->codec));
        buffer->setOffset(0, 0);

        producedBuffers.putBuffer(buffer, 250);
        sourceBuffer(true);

        if (producedBuffers.numBuffersQueued() > 3)
          sourceBuffer(true);

        lastTime = currentTime;
        sourceTime += avgFrameDeltaT;
      }
      else // Try read()
      {
        size_t numBytes = parent->width * parent->height * PixelSize(parent->codec);
        Buffer::Ptr<VideoBufferAligned> buffer = VideoBufferAligned::getManager()->getBuffer(numBytes);

        if (buffer && readImage(buffer->getBuffer(), numBytes))
        {
          buffer->setTimeStamp(currentTime);
          buffer->setCodec(parent->codec);
          buffer->setFieldMode(parent->fieldMode);
          buffer->setWidth(parent->width);
          buffer->setHeight(parent->height);
          buffer->setWidthAspect(parent->getWidthAspect());
          buffer->setHeightAspect(parent->getHeightAspect());
          buffer->setLineSize(0, parent->width * PixelSize(parent->codec));
          buffer->setOffset(0, 0);

          producedBuffers.putBuffer(buffer, 250);
          sourceBuffer(true);

          if (producedBuffers.numBuffersQueued() > 3)
            sourceBuffer(true);

          lastTime = currentTime;
          sourceTime += avgFrameDeltaT;
        }
        else // Problem receiving frames, try a restart.
        {
          msleep(40);

          ScopeLock l(parent);

          // Stop the stream
          ioctl(parent->devDesc, VIDIOC_STREAMOFF, &captureBuffers[0].vidbuf.type);
          producedBuffers.getAllBuffers();

          // Start it again
          if (ioctl(parent->devDesc, VIDIOC_STREAMON, &captureBuffers[0].vidbuf.type ) >= 0)
          {
            l.unlock();
            queueAllBuffers();
          }
        }
      }
    }
    else
    {
      msleep(100);
      sourceBuffer(true);
    }
  }
  else // V4L1 compatibility
  {
    while (running)
    {
      const unsigned numBytes = parent->width * parent->height * PixelSize(parent->codec);
      Buffer::Ptr<VideoBufferAligned> buffer = VideoBufferAligned::getManager()->getBuffer(numBytes);

      if (buffer)
      if (read(parent->devDesc, buffer->getBuffer(), numBytes) > 0)
      {
        const quint64 currentTime = mediaTimer.getTimeStamp();

        buffer->setTimeStamp(currentTime);
        buffer->setCodec(parent->codec);
        buffer->setFieldMode(parent->fieldMode);
        buffer->setWidth(parent->width);
        buffer->setHeight(parent->height);
        buffer->setWidthAspect(parent->getWidthAspect());
        buffer->setHeightAspect(parent->getHeightAspect());
        buffer->setLineSize(0, parent->width * PixelSize(parent->codec));
        buffer->setOffset(0, 0);

        producedBuffers.putBuffer(buffer, 250);
        sourceBuffer(true);

        if (producedBuffers.numBuffersQueued() > 3)
          sourceBuffer(true);

        continue;
      }

      msleep(250);
    }
  }
}


} } // End of namespaces
