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

#include "alsainput.h"
#include "module.h"

namespace LXiStreamDevice {
namespace AlsaBackend {

AlsaInput::AlsaInput(const QString &dev, QObject *parent)
  : SInterfaces::AudioInput(parent),
    dev(deviceName(dev)),
    channel(channelName(dev)),
    mixer(dev),
    pcm(NULL),
    outFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 48000)
{
}

AlsaInput::~AlsaInput()
{
  if (pcm)
    snd_pcm_close(pcm);
}

void AlsaInput::setFormat(const SAudioFormat &c)
{
  outFormat = c;
}

SAudioFormat AlsaInput::format(void)
{
  return outFormat;
}

bool AlsaInput::start(void)
{
  bool retval = false;

  snd_pcm_hw_params_t *hw_params = NULL;

  unsigned outSampleRate = outFormat.sampleRate();
  unsigned outNumChannels = outFormat.numChannels();

  if (!channel.isEmpty())
  if (!mixer.open() || !mixer.activateInputChannel(channel))
    return false;

  if (snd_pcm_open(&pcm, dev.toLatin1().data(), SND_PCM_STREAM_CAPTURE, 0) == 0)
  if (snd_pcm_hw_params_malloc(&hw_params) == 0)
  if (snd_pcm_hw_params_any(pcm, hw_params) == 0)
  if (snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) == 0)
  if (snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE) == 0)
  if (snd_pcm_hw_params_set_rate_near(pcm, hw_params, &outSampleRate, 0) == 0)
  if (snd_pcm_hw_params_set_channels_near(pcm, hw_params, &outNumChannels) == 0)
  if (snd_pcm_hw_params(pcm, hw_params) == 0)
  if (snd_pcm_prepare(pcm) == 0)
  {
    outFormat.setSampleRate(outSampleRate);
    outFormat.setChannelSetup(SAudioFormat::guessChannels(outNumChannels));

    retval = true;
  }

  if (hw_params)
    snd_pcm_hw_params_free(hw_params);

  return retval;
}

void AlsaInput::stop(void)
{
  if (pcm)
  {
    snd_pcm_close(pcm);
    pcm = NULL;
  }

  mixer.close();
}

bool AlsaInput::process(void)
{
  const unsigned numSamples = outFormat.sampleRate() * 40 / 1000; // 40 ms of samples
  SAudioBuffer buffer(outFormat, numSamples);

  for (int n=0; n<3; n++)
  {
    int err = snd_pcm_readi(pcm, buffer.data(), numSamples);
    if (err > 0)
    {
      // Determine latency
      STime delay = STime::null;
      snd_pcm_sframes_t framesDelay = 0;
      if (snd_pcm_delay(pcm, &framesDelay) == 0)
        delay = STime::fromClock(framesDelay, outFormat.sampleRate());

      const STime duration = STime::fromClock(err, outFormat.sampleRate());
      buffer.setTimeStamp(timer.smoothTimeStamp(duration, delay));

      emit produce(buffer);
      return true;
    }
    else if (err < 0)
    {
      qDebug() << "AlsaInput: Recovering";
      snd_pcm_recover(pcm, err, 1);
    }
  }

  return false;
}

} } // End of namespaces
