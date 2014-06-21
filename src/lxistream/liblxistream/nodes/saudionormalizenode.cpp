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

#include "nodes/saudionormalizenode.h"
#include "../../algorithms/audioprocess.h"

namespace LXiStream {

struct SAudioNormalizeNode::Data
{
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
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

bool SAudioNormalizeNode::start(void)
{
  return true;
}

void SAudioNormalizeNode::stop(void)
{
  d->history.clear();
  d->delay.clear();
}

void SAudioNormalizeNode::input(const SAudioBuffer &audioBuffer)
{
  if (audioBuffer.isNull() || (audioBuffer.format() == SAudioFormat::Format_PCM_S16))
  {
    if (!audioBuffer.isNull())
    {
      const STime currentTime = audioBuffer.timeStamp();

      d->history.insert(currentTime, Algorithms::AudioProcess::avg(
          reinterpret_cast<const qint16 *>(audioBuffer.data()),
          audioBuffer.numSamples() * audioBuffer.format().numChannels()));

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

        SAudioBuffer destBuffer(srcBuffer.format(), srcBuffer.numSamples());
        destBuffer.setTimeStamp(srcBuffer.timeStamp());

        Algorithms::AudioProcess::gain(
            reinterpret_cast<qint16 *>(destBuffer.data()),
            reinterpret_cast<const qint16 *>(srcBuffer.data()),
            srcBuffer.numSamples() * srcBuffer.format().numChannels(),
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

        SAudioBuffer destBuffer(srcBuffer.format(), srcBuffer.numSamples());
        destBuffer.setTimeStamp(srcBuffer.timeStamp());

        Algorithms::AudioProcess::gain(
            reinterpret_cast<qint16 *>(destBuffer.data()),
            reinterpret_cast<const qint16 *>(srcBuffer.data()),
            srcBuffer.numSamples() * srcBuffer.format().numChannels(),
            4096.0f / float(avg));

        emit output(destBuffer);
      }

      emit output(audioBuffer);
    }
  }
  else
    emit output(audioBuffer);
}

} // End of namespace
