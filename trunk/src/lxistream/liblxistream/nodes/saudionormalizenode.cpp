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

#include "nodes/saudionormalizenode.h"

// Implemented in saudionormalizenode.mul.c
extern "C" qint16 LXiStream_SAudioNormalizeNode_measure
 (const qint16 * srcData, unsigned numSamples, unsigned srcNumChannels);
extern "C" void LXiStream_SAudioNormalizeNode_gain
 (const qint16 * srcData, unsigned numSamples, unsigned srcNumChannels,
  qint16 * dstData, float factor);

namespace LXiStream {

struct SAudioNormalizeNode::Data
{
  QFuture<void>                 future;
  QMap<STime, qint16>           history;
  QQueue<SAudioBuffer>          delay;
};

SAudioNormalizeNode::SAudioNormalizeNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
}

SAudioNormalizeNode::~SAudioNormalizeNode()
{
  d->future.waitForFinished();
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SAudioNormalizeNode::start(void)
{
  return true;
}

void SAudioNormalizeNode::stop(void)
{
  d->future.waitForFinished();
  d->history.clear();
  d->delay.clear();
}

void SAudioNormalizeNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_WAIT(d->future.waitForFinished());
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  if (audioBuffer.isNull() || (audioBuffer.format() == SAudioFormat::Format_PCM_S16))
    d->future = QtConcurrent::run(this, &SAudioNormalizeNode::processTask, audioBuffer);
  else
    emit output(audioBuffer);
}

void SAudioNormalizeNode::processTask(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION(TaskType_AudioProcessing);

  if (!audioBuffer.isNull())
  {
    const STime currentTime = audioBuffer.timeStamp();

    d->history.insert(currentTime, LXiStream_SAudioNormalizeNode_measure
        (reinterpret_cast<const qint16 *>(audioBuffer.data()),
         audioBuffer.numSamples(),
         audioBuffer.format().numChannels()));

    qint64 avg = 0;
    for (QMap<STime, qint16>::Iterator i=d->history.begin(); i!=d->history.end(); )
    if ((i.value() >= 512) && (qAbs(i.key() - currentTime).toSec() < 30))
    {
      avg += i.value();
      i++;
    }
    else
      i = d->history.erase(i);

    if (d->history.count() > 0)
      avg /= d->history.count();
    else
      avg = 4096;

//    qDebug() << "factor" << avg << (4096.0f / float(avg));

    d->delay.enqueue(audioBuffer);
    while ((d->delay.count() >= (d->history.count() / 2)) && !d->delay.isEmpty())
    {
      const SAudioBuffer srcBuffer = d->delay.dequeue();

      SAudioBuffer destBuffer(audioBuffer.format(), srcBuffer.numSamples());
      destBuffer.setTimeStamp(srcBuffer.timeStamp());

      LXiStream_SAudioNormalizeNode_gain
          (reinterpret_cast<const qint16 *>(srcBuffer.data()),
           srcBuffer.numSamples(),
           srcBuffer.format().numChannels(),
           reinterpret_cast<qint16 *>(destBuffer.data()),
           4096.0f / float(avg));

      emit output(destBuffer);
    }
  }
  else // Flush queue
  {
    qint64 avg = 0;
    for (QMap<STime, qint16>::Iterator i=d->history.begin(); i!=d->history.end(); )
    {
      if (i.value() >= 512)
        avg += i.value();

      i = d->history.erase(i);
    }

    if (d->history.count() > 0)
      avg /= d->history.count();
    else
      avg = 4096;

    while (!d->delay.isEmpty())
    {
      const SAudioBuffer srcBuffer = d->delay.dequeue();

      SAudioBuffer destBuffer(audioBuffer.format(), srcBuffer.numSamples());
      destBuffer.setTimeStamp(srcBuffer.timeStamp());

      LXiStream_SAudioNormalizeNode_gain
          (reinterpret_cast<const qint16 *>(srcBuffer.data()),
           srcBuffer.numSamples(),
           srcBuffer.format().numChannels(),
           reinterpret_cast<qint16 *>(destBuffer.data()),
           4096.0f / float(avg));

      emit output(destBuffer);
    }

    emit output(audioBuffer);
  }
}

} // End of namespace
