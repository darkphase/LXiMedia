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

#include "nodes/stimestampresamplernode.h"
#include "saudiobuffer.h"
#include "sgraph.h"
#include "svideobuffer.h"

namespace LXiStream {

struct STimeStampResamplerNode::Data
{
  SInterval                     frameRate;
  double                        ratio;
  bool                          resample;
};

STimeStampResamplerNode::STimeStampResamplerNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->ratio = 1.0;
  d->resample = false;
}

STimeStampResamplerNode::~STimeStampResamplerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void STimeStampResamplerNode::setFrameRate(SInterval frameRate)
{
  d->frameRate = frameRate;
  d->ratio = 1.0;
  d->resample = false;
}

void STimeStampResamplerNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && d->resample)
  {
    SAudioBuffer buffer = audioBuffer;

    SAudioFormat f = buffer.format();
    const unsigned oldSampleRate = f.sampleRate();
    const unsigned sampleRate = unsigned(oldSampleRate * d->ratio) & ~1u;
    f.setSampleRate(sampleRate);
    buffer.setFormat(f);

    const STime timeStamp = buffer.timeStamp();
    if (timeStamp.isValid())
      buffer.setTimeStamp(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval()));

    emit output(buffer);
  }
  else
    emit output(audioBuffer);
}

void STimeStampResamplerNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->frameRate.isValid())
  {
    const SInterval srcInterval = videoBuffer.format().frameRate();
    const double srcRate = srcInterval.toFrequency();
    const double dstRate = d->frameRate.toFrequency();

    d->ratio = dstRate / srcRate;

    // Only resample if needed and below 8%.
    d->resample = !qFuzzyCompare(d->ratio, 1.0) && (qAbs(d->ratio - 1.0) < 0.08);
    if (d->resample)
    {
      SVideoBuffer buffer = videoBuffer;

      SVideoFormat f = buffer.format();
      f.setFrameRate(d->frameRate);
      buffer.setFormat(f);

      const STime timeStamp = buffer.timeStamp();
      if (timeStamp.isValid())
      {
        qint64 clock = timeStamp.toClock(srcInterval);
        buffer.setTimeStamp(STime::fromClock(clock, d->frameRate));
      }

      emit output(buffer);
      return;
    }
  }

  emit output(videoBuffer);
}

void STimeStampResamplerNode::input(const SSubpictureBuffer &subpictureBuffer)
{
  if (!subpictureBuffer.isNull() && d->resample)
  {
    SSubpictureBuffer buffer = subpictureBuffer;

    const STime timeStamp = buffer.timeStamp();
    if (timeStamp.isValid())
      buffer.setTimeStamp(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval()));

    emit output(buffer);
  }
  else
    emit output(subpictureBuffer);
}

void STimeStampResamplerNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  if (!subtitleBuffer.isNull() && d->resample)
  {
    SSubtitleBuffer buffer = subtitleBuffer;

    const STime timeStamp = buffer.timeStamp();
    if (timeStamp.isValid())
      buffer.setTimeStamp(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval()));

    emit output(buffer);
  }
  else
    emit output(subtitleBuffer);
}


} // End of namespace
