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
#include "svideobuffer.h"

namespace LXiStream {

struct STimeStampResamplerNode::Data
{
  QMutex                        mutex;

  SInterval                     frameRate;
  double                        maxRatio;
  double                        ratio;
  bool                          resample;

  STime                         highestTimeStamp;
  QList<STime>                  outputOffset;
};

STimeStampResamplerNode::STimeStampResamplerNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->maxRatio = 0.0;
  d->ratio = 1.0;
  d->resample = false;

  d->highestTimeStamp = STime::null;
  d->outputOffset.append(STime::null);
}

STimeStampResamplerNode::~STimeStampResamplerNode()
{
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void STimeStampResamplerNode::setFrameRate(SInterval frameRate, double maxRatio)
{
  d->frameRate = frameRate;
  d->maxRatio = maxRatio;
  d->ratio = 1.0;
  d->resample = false;
}

const QVector<double> & STimeStampResamplerNode::standardFrameRates(void)
{
  static QVector<double> rates;
  if (rates.isEmpty())
    rates << 24.0 << 25.0 << 30.0 << 50.0 << 60.0;

  return rates;
}

SInterval STimeStampResamplerNode::roundFrameRate(SInterval frameRate, const QVector<double> &rates)
{
  const double freq = frameRate.toFrequency();

  for (int i=0; i<rates.count(); i++)
  {
    const double min = (i == 0) ? 0.0 : ((rates[i-1] + rates[i]) / 2.0);
    const double max = (i == (rates.count() - 1)) ? INT_MAX : ((rates[i] + rates[i+1]) / 2.0);

    if ((freq > min) && (freq < max))
      return SInterval::fromFrequency(rates[i]);
  }

  return frameRate;
}

void STimeStampResamplerNode::input(const SAudioBuffer &audioBuffer)
{
  LXI_PROFILE_FUNCTION;
  QMutexLocker l(&d->mutex);

  if (!audioBuffer.isNull())
  {
    SAudioBuffer buffer = audioBuffer;

    if (d->resample)
    {
      SAudioFormat f = buffer.format();
      const unsigned oldSampleRate = f.sampleRate();
      const unsigned sampleRate = unsigned(oldSampleRate * d->ratio) & ~1u;
      f.setSampleRate(sampleRate);
      buffer.setFormat(f);

      const STime timeStamp = buffer.timeStamp();
      if (timeStamp.isValid())
        buffer.setTimeStamp(correct(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval())));
    }
    else
      buffer.setTimeStamp(correct(buffer.timeStamp()));

    emit output(buffer);
  }
  else
    emit output(audioBuffer);
}

bool STimeStampResamplerNode::start(void)
{
  return true;
}

void STimeStampResamplerNode::stop(void)
{
}

void STimeStampResamplerNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;
  QMutexLocker l(&d->mutex);

  if (!videoBuffer.isNull() && d->frameRate.isValid())
  {
    const SInterval srcInterval = videoBuffer.format().frameRate();
    const double srcRate = srcInterval.toFrequency();
    const double dstRate = d->frameRate.toFrequency();

    d->ratio = dstRate / srcRate;

    SVideoBuffer buffer = videoBuffer;

    // Only resample if needed and below maxRatio.
    d->resample = !qFuzzyCompare(d->ratio, 1.0) && (qAbs(d->ratio - 1.0) < d->maxRatio);
    if (d->resample)
    {
      SVideoFormat f = buffer.format();
      f.setFrameRate(d->frameRate);
      buffer.setFormat(f);

      const STime timeStamp = buffer.timeStamp();
      if (timeStamp.isValid())
      {
        qint64 clock = timeStamp.toClock(srcInterval);
        buffer.setTimeStamp(correct(STime::fromClock(clock, d->frameRate)));
      }
    }
    else
      buffer.setTimeStamp(correct(buffer.timeStamp()));

    emit output(buffer);
  }
  else
    emit output(videoBuffer);
}

void STimeStampResamplerNode::input(const SSubpictureBuffer &subpictureBuffer)
{
  LXI_PROFILE_FUNCTION;
  QMutexLocker l(&d->mutex);

  if (!subpictureBuffer.isNull())
  {
    SSubpictureBuffer buffer = subpictureBuffer;

    if (d->resample)
    {
      const STime timeStamp = buffer.timeStamp();
      if (timeStamp.isValid())
        buffer.setTimeStamp(correct(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval())));
    }
    else
      buffer.setTimeStamp(correct(buffer.timeStamp()));

    emit output(buffer);
  }
  else
    emit output(subpictureBuffer);
}

void STimeStampResamplerNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  LXI_PROFILE_FUNCTION;
  QMutexLocker l(&d->mutex);

  if (!subtitleBuffer.isNull())
  {
    SSubtitleBuffer buffer = subtitleBuffer;

    if (d->resample)
    {
      const STime timeStamp = buffer.timeStamp();
      if (timeStamp.isValid())
        buffer.setTimeStamp(correct(STime(qint64(timeStamp.count() / d->ratio), timeStamp.interval())));
    }
    else
      buffer.setTimeStamp(correct(buffer.timeStamp()));

    emit output(buffer);
  }
  else
    emit output(subtitleBuffer);
}

STime STimeStampResamplerNode::correct(const STime &timeStamp)
{
  if (timeStamp.isValid())
  {
    QMap<STime, STime> corrected;
    foreach (const STime &offset, d->outputOffset)
    {
      const STime result = timeStamp + offset;
      corrected.insert(d->highestTimeStamp - result, result);
    }

    STime result = corrected.begin().value();
    if (qAbs(corrected.begin().key()) > STime::fromSec(10)) // No suitable offset found.
    {
      result += corrected.begin().key();

      const STime offset = result - timeStamp;
      for (QList<STime>::Iterator i=d->outputOffset.begin(); i!=d->outputOffset.end();)
      if (qAbs(*i - offset) < STime::fromSec(5))
        i++;
      else
        i = d->outputOffset.erase(i);

      d->outputOffset.prepend(offset);
      if (d->outputOffset.count() > numChannels)
        d->outputOffset.takeLast();
    }

    d->highestTimeStamp = qMax(d->highestTimeStamp, result);

    return result;
  }

  return timeStamp;
}

} // End of namespace
