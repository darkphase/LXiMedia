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

#include "audioresampler.h"

namespace LXiStream {
namespace FFMpegBackend {


AudioResampler::AudioResampler(const QString &, QObject *parent)
  : SInterfaces::AudioResampler(parent),
    outSampleRate(48000),
    inFormat(SAudioFormat::Format_PCM_S16, 0, 0),
    reopen(false)
{
}

AudioResampler::~AudioResampler()
{
  foreach (const Channel &channel, channels)
  if (channel.context)
    ::av_resample_close(channel.context);
}

void AudioResampler::setSampleRate(unsigned rate)
{
  outSampleRate = rate;
}

unsigned AudioResampler::sampleRate(void)
{
  return outSampleRate;
}

SAudioBuffer AudioResampler::processBuffer(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull() && (outSampleRate > 0))
  {
    Q_ASSERT(audioBuffer.format() == SAudioFormat::Format_PCM_S16);

    if (audioBuffer.format() == SAudioFormat::Format_PCM_S16)
    if (audioBuffer.format().sampleRate() != outSampleRate)
    {
      if (inFormat != audioBuffer.format())
      {
        foreach (const Channel &channel, channels)
        if (channel.context)
          ::av_resample_close(channel.context);

        channels.clear();

        inFormat = audioBuffer.format();

        const int channelSetup = inFormat.channelSetup();
        for (int i=0; i<32; i++)
        if ((channelSetup & (quint32(1) << i)) != 0)
        {
          channels.append(Channel(
              SAudioFormat::Channel(quint32(1) << i),
              ::av_resample_init(outSampleRate, inFormat.sampleRate(), 16, 10, 1, 0.95)));
        }
      }

      QList< QFuture<SAudioBuffer> > futures;
      for (int i=0; i<channels.count(); i++)
        futures.append(QtConcurrent::run(this, &AudioResampler::resampleChannel, &channels[i], audioBuffer));

      SAudioFormat outFormat = inFormat;
      outFormat.setSampleRate(outSampleRate);

      unsigned numSamples = UINT_MAX;
      for (int i=0; i<futures.count(); i++)
      {
        futures[i].waitForFinished();
        numSamples = qMin(numSamples, futures[i].result().numSamples());
      }

      if ((numSamples > 0) && (numSamples < UINT_MAX))
      {
        SAudioBuffer destBuffer(outFormat, numSamples);
        for (int i=0; i<futures.count(); i++)
          destBuffer.setChannels(futures[i].result());

        destBuffer.setTimeStamp(audioBuffer.timeStamp());

        return destBuffer;
      }
    }
  }

  return audioBuffer;
}

SAudioBuffer AudioResampler::resampleChannel(Channel *channel, const SAudioBuffer &audioBuffer) const
{
  const SAudioBuffer channelBuffer = audioBuffer.getChannel(channel->channel);
  const SAudioBuffer srcBuffer = SAudioBufferList() << channel->channelBuffer << channelBuffer;

  // Estimate buffer size
  const unsigned estNumSamples = STime::fromClock(audioBuffer.numSamples(), inFormat.sampleRate()).toClock(outSampleRate) + 32;
  SAudioFormat outFormat = inFormat;
  outFormat.setChannelSetup(channel->channel);
  outFormat.setSampleRate(outSampleRate);
  SAudioBuffer dstBuffer(outFormat, estNumSamples);

  int consumed = 0;
  int produced = ::av_resample(
      channel->context,
      (short *)dstBuffer.data(),
      ((short *)srcBuffer.data()) + channel->bufferOffset,
      &consumed,
      srcBuffer.numSamples() - channel->bufferOffset,
      dstBuffer.numSamples(),
      1);

  dstBuffer.setNumSamples(produced);

  channel->bufferOffset += consumed;
  if (!channel->channelBuffer.isNull() && (channel->bufferOffset >= channel->channelBuffer.numSamples()))
  {
    channel->bufferOffset -= channel->channelBuffer.numSamples();
    channel->channelBuffer = channelBuffer;
  }
  else
    channel->channelBuffer = srcBuffer;

  return dstBuffer;
}

} } // End of namespaces