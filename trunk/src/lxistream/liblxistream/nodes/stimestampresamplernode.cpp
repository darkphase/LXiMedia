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
};

STimeStampResamplerNode::STimeStampResamplerNode(SGraph *parent)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->ratio = 1.0;
}

STimeStampResamplerNode::~STimeStampResamplerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void STimeStampResamplerNode::setFrameRate(SInterval frameRate)
{
  d->frameRate = frameRate;
}

void STimeStampResamplerNode::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && !qFuzzyCompare(d->ratio, 1.0))
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
    SVideoBuffer buffer = videoBuffer;

    SVideoFormat f = buffer.format();
    const SInterval oldFrameRate = f.frameRate();
    f.setFrameRate(d->frameRate);
    buffer.setFormat(f);

    const STime timeStamp = buffer.timeStamp();
    if (timeStamp.isValid())
      buffer.setTimeStamp(STime::fromClock(timeStamp.toClock(oldFrameRate), d->frameRate));

    d->ratio = d->frameRate.toFrequency() / oldFrameRate.toFrequency();

    emit output(buffer);
  }
  else
    emit output(videoBuffer);
}

void STimeStampResamplerNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  if (!subtitleBuffer.isNull() && !qFuzzyCompare(d->ratio, 1.0))
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
