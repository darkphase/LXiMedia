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

#include "nodes/stimestampsyncnode.h"
#include "saudiobuffer.h"
#include "svideobuffer.h"

namespace LXiStream {

template <class _buffer>
struct STimeStampSyncNode::Queue
{
  inline                        Queue(STime time) : time(time) { }

  QMultiMap<STime, _buffer>     buffers;
  STime                         time;
};

struct STimeStampSyncNode::Data
{
  inline Data(void)
    : maxAudioDelay(STime::fromMSec(24) * maxAudioBufferCount),
      maxVideoDelay(STime::fromMSec(40) * maxVideoBufferCount),
      running(false), inTimeStamp(STime::null)
  {
  }

  static const int              maxAudioBufferCount = 256;
  static const int              maxVideoBufferCount = 96;

  QMutex                        mutex;

  QMap<quint16, Queue<SAudioBuffer> > audioQueue;
  QMap<quint16, Queue<SVideoBuffer> > videoQueue;

  const STime                   maxAudioDelay;
  const STime                   maxVideoDelay;
  bool                          running;
  STime                         firstTimeStamp;
  STime                         inTimeStamp;
  SInterval                     frameRate;
  STime                         startTime;
};


STimeStampSyncNode::STimeStampSyncNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->frameRate = SInterval::fromFrequency(25);
  d->startTime = STime::null;
}

STimeStampSyncNode::~STimeStampSyncNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

SInterval STimeStampSyncNode::frameRate(void) const
{
  return d->frameRate;
}

void STimeStampSyncNode::setFrameRate(SInterval r)
{
  d->frameRate = r;
}

STime STimeStampSyncNode::startTime(void) const
{
  return d->startTime;
}

void STimeStampSyncNode::setStartTime(STime startTime)
{
  d->startTime = startTime;
}

bool STimeStampSyncNode::start(void)
{
  return true;
}

void STimeStampSyncNode::stop(void)
{
}

void STimeStampSyncNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);
  QMutexLocker l(&d->mutex);

  if (!audioBuffer.isNull())
  {
    const STime timeStamp = audioBuffer.timeStamp();
//    qDebug() << "AI" << timeStamp.toMSec();
    if (!d->firstTimeStamp.isValid() || (timeStamp >= d->firstTimeStamp))
    {
      QMap<quint16, Queue<SAudioBuffer> >::Iterator i = d->audioQueue.find(0);
      if (i == d->audioQueue.end())
        i = d->audioQueue.insert(0, Queue<SAudioBuffer>(d->startTime));

      // Dump out-of-range buffers
      for (QMultiMap<STime, SAudioBuffer>::Iterator j = i->buffers.lowerBound(timeStamp + d->maxAudioDelay);
           j != i->buffers.end(); )
      {
        if (d->videoQueue.isEmpty())
        { // No video stream, output all audio packets (probably next song is starting).
          SAudioBuffer ab = *j;

          ab.setTimeStamp(i->time);
//          qDebug() << "AOP" << ab.timeStamp().toMSec();
          emit output(ab);

          i->time += ab.duration();
        }

        j = i->buffers.erase(j);
      }

      if (i->buffers.count() < d->maxAudioBufferCount)
        i->buffers.insert(timeStamp, audioBuffer);

      if (!d->videoQueue.isEmpty())
      {
        output();
      }
      else for (QMultiMap<STime, SAudioBuffer>::Iterator j=i->buffers.begin(); j!=i->buffers.end(); )
      if ((qAbs(audioBuffer.timeStamp() - j->timeStamp()) > d->maxAudioDelay) ||
          (i->buffers.count() > d->maxAudioBufferCount))
      {
        SAudioBuffer ab = *j;

        ab.setTimeStamp(i->time);
//        qDebug() << "AOF" << ab.timeStamp().toMSec();
        emit output(ab);

        i->time += ab.duration();
        j = i->buffers.erase(j);
      }
      else
        break;
    }
  }
}

void STimeStampSyncNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_MiscProcessing);
  QMutexLocker l(&d->mutex);

  if (!videoBuffer.isNull())
  {
    const STime timeStamp = videoBuffer.timeStamp();
//    qDebug() << "VI" << timeStamp.toMSec();
    if (!d->firstTimeStamp.isValid() || (timeStamp >= d->firstTimeStamp))
    {
      QMap<quint16, Queue<SVideoBuffer> >::Iterator i = d->videoQueue.find(0);
      if (i == d->videoQueue.end())
      {
        i = d->videoQueue.insert(0, Queue<SVideoBuffer>(d->startTime));
        if (!d->frameRate.isValid())
          d->frameRate = videoBuffer.format().frameRate();
      }

      // Dump out-of-range buffers
      for (QMultiMap<STime, SVideoBuffer>::Iterator j = i->buffers.lowerBound(timeStamp + d->maxVideoDelay);
           j != i->buffers.end(); )
      {
        j = i->buffers.erase(j);
      }

      if (i->buffers.count() < d->maxVideoBufferCount)
        i->buffers.insert(timeStamp, videoBuffer);

      if (!d->audioQueue.isEmpty())
      {
        output();
      }
      else for (QMultiMap<STime, SVideoBuffer>::Iterator j=i->buffers.begin(); j!=i->buffers.end(); )
      if ((qAbs(videoBuffer.timeStamp() - j->timeStamp()) > d->maxVideoDelay) ||
          (i->buffers.count() > d->maxVideoBufferCount))
      {
        SVideoBuffer vb = *j;

        vb.setTimeStamp(i->time);
//        qDebug() << "VOF" << vb.timeStamp().toMSec();
        emit output(vb);

        i->time += STime(1, d->frameRate);
        j = i->buffers.erase(j);
      }
      else
        break;
    }
  }
}

void STimeStampSyncNode::output(void)
{
  const int minBufferSize = d->firstTimeStamp.isValid() ? 1 : (d->maxVideoBufferCount / 4);
  bool hasAudio = false, hasVideo = false, audioReady = true, videoReady = true;
  for (QMap<quint16, Queue<SAudioBuffer> >::Iterator i=d->audioQueue.begin(); i!=d->audioQueue.end(); i++)
  {
    hasAudio = true;
    audioReady &= (i->buffers.count() >= minBufferSize);
  }

  for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
  {
    hasVideo = true;
    videoReady &= (i->buffers.count() >= minBufferSize);
  }

  audioReady &= hasAudio;
  videoReady &= hasVideo;

  if (!d->running && audioReady && videoReady)
  {
    if (!d->firstTimeStamp.isValid())
    {
      d->firstTimeStamp = STime::null;

      for (QMap<quint16, Queue<SAudioBuffer> >::Iterator i=d->audioQueue.begin(); i!=d->audioQueue.end(); i++)
      if (!i->buffers.isEmpty())
        d->firstTimeStamp = qMax(d->firstTimeStamp, i->buffers.begin().key());

      for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
      if (!i->buffers.isEmpty())
        d->firstTimeStamp = qMax(d->firstTimeStamp, i->buffers.begin().key());

//      qDebug() << "firstTimeStamp" << d->firstTimeStamp.toMSec();
    }

    for (QMap<quint16, Queue<SAudioBuffer> >::Iterator i=d->audioQueue.begin(); i!=d->audioQueue.end(); i++)
    while (!i->buffers.isEmpty() && (i->buffers.begin().key() < d->firstTimeStamp))
      i->buffers.erase(i->buffers.begin());

    for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
    while (!i->buffers.isEmpty() && (i->buffers.begin().key() < d->firstTimeStamp))
      i->buffers.erase(i->buffers.begin());

    d->running = true;
  }
  else if (audioReady && videoReady)
  {
    // Send audio buffers
    STime lvkey;
    for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
    if (!i->buffers.isEmpty())
      lvkey = lvkey.isValid() ? qMin(lvkey, i->buffers.begin().key()) : i->buffers.begin().key();

    STime audioTimeMin, audioTimeMax;
    STime inTime;

    if (lvkey.isValid())
    for (QMap<quint16, Queue<SAudioBuffer> >::Iterator i=d->audioQueue.begin(); i!=d->audioQueue.end(); i++)
    {
      STime at = i->time, it = d->inTimeStamp;
      audioTimeMin = audioTimeMin.isValid() ? qMin(audioTimeMin, at) : at;

      for (QMultiMap<STime, SAudioBuffer>::Iterator j = i->buffers.begin();
           (j != i->buffers.end()) && (lvkey >= it);
           j = i->buffers.erase(j))
      {
        it = j.key();

        SAudioBuffer ab = *j;

//        qDebug() << "AO1" << ab.timeStamp().toMSec() <<  i->time.toMSec() << lvkey.toMSec() << it.toMSec();
        ab.setTimeStamp(i->time);
        emit output(ab);

        at = i->time;
        i->time += ab.duration();
      }

      audioTimeMax = audioTimeMax.isValid() ? qMin(audioTimeMax, at) : at;
      inTime = inTime.isValid() ? qMin(inTime, it) : it;
    }

    if (inTime.isValid())
      d->inTimeStamp = inTime;

    const STime frameTime = d->frameRate.isValid() ? STime(1, d->frameRate) : STime::fromMSec(15);
    const STime maxDelta = frameTime * -1;

    // Dump old video buffers
    for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
    for (QMultiMap<STime, SVideoBuffer>::Iterator j = i->buffers.begin(); j != i->buffers.end(); )
    {
      if ((j.key() - d->inTimeStamp) <= maxDelta)
      {
//        qDebug() << "DUMPV" << j.key().toMSec() << d->inTimeStamp.toMSec() << frameTime.toMSec();
        j = i->buffers.erase(j);
      }
      else
        break;
    }

    // Output video buffers
    if (audioTimeMin.isValid() && audioTimeMax.isValid())
    for (QMap<quint16, Queue<SVideoBuffer> >::Iterator i=d->videoQueue.begin(); i!=d->videoQueue.end(); i++)
    for (QMultiMap<STime, SVideoBuffer>::Iterator j = i->buffers.begin(); j != i->buffers.end(); )
    {
      const STime delta = j.key() - d->inTimeStamp;
      if (!delta.isPositive())
      {
        const STime nextVideoTimeMin = audioTimeMin + delta;

        SVideoBuffer vb = *j;
        j = i->buffers.erase(j);

        if (i->time <= (audioTimeMax + frameTime))
        do
        {
//          qDebug() << "VO1" << vb.timeStamp().toMSec() << i->time.toMSec() << nextVideoTimeMin.toMSec() << delta.toMSec();
          vb.setTimeStamp(i->time);
          emit output(vb);

          i->time += frameTime;
//          if (i->time <= nextVideoTimeMin) qDebug() << "DUPV" << i->time.toMSec() << nextVideoTimeMin.toMSec();
        } while (i->time <= nextVideoTimeMin);
      }
      else
        break;
    }
  }
}


} // End of namespace
