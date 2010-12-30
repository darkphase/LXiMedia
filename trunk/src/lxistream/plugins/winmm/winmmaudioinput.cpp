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

#include "winmmaudioinput.h"

namespace LXiStream {
namespace WinMMBackend {


WinMMAudioInput::WinMMAudioInput(int dev, QObject *parent)
  : SNodes::Audio::Source(Behavior_Blocking, 0, SCodecList(), parent),
    running(false),
    dev(dev),
    waveIn(NULL),
    format(SCodec::Format_Invalid),
    outSampleSize(0),
    outSampleRate(48000),
    outNumChannels(2)
{
}

bool WinMMAudioInput::prepare(const SCodecList &)
{
  bool retval = false;

  /*snd_pcm_hw_params_t *hw_params = NULL;

  if (snd_pcm_open(&pcm, dev.toAscii().data(), SND_PCM_STREAM_CAPTURE, 0) == 0)
  if (snd_pcm_hw_params_malloc(&hw_params) == 0)
  if (snd_pcm_hw_params_any(pcm, hw_params) == 0)
  if (snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) == 0)
  if (snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE) == 0)
  if (snd_pcm_hw_params_set_rate_near(pcm, hw_params, &outSampleRate, 0) == 0)
  if (snd_pcm_hw_params_set_channels_near(pcm, hw_params, &outNumChannels) == 0)
  if (snd_pcm_hw_params(pcm, hw_params) == 0)
  if (snd_pcm_prepare(pcm) == 0)
  {
    format = SCodec(SAudioCodec::Format_PCM_S16LE, SCodec::guessChannels(outNumChannels), outSampleRate);
    outSampleSize = format.sampleSize();

    retval = true;
  }

  if (hw_params)
    snd_pcm_hw_params_free(hw_params);
*/
  return retval;
}

bool WinMMAudioInput::unprepare(void)
{
  //if (pcm)
  //  snd_pcm_close(pcm);

  return true;
}

SNode::Result WinMMAudioInput::processBuffer(const SBuffer &, SBufferList &output)
{
  /*const unsigned numSamples = outSampleRate * 40 / 1000; // 40 ms of samples
  SAudioBuffer buffer(numSamples * outNumChannels * outSampleSize);

  for (int n=0; n<3; n++)
  {
    int err = snd_pcm_readi(pcm, buffer.bits(), numSamples);
    if (err > 0)
    {
      // Determine latency
      STime delay = STime::fromMSec(0);
      snd_pcm_sframes_t framesDelay = 0;
      if (snd_pcm_delay(pcm, &framesDelay) == 0)
        delay = STime::fromClock(framesDelay, outSampleRate);

      const STime duration = STime::fromClock(err, outSampleRate);
      buffer.setTimeStamp(timer.smoothTimeStamp(duration, delay));
      buffer.setDecodedTimeStamp(buffer.timeStamp());
      buffer.setCodec(format);
      buffer.setNumSamples(err);

      output << buffer;
      return Result_Active;
    }
    else if (err < 0)
    {
      qWarning() << "AlsaInput: Recovering";
      snd_pcm_recover(pcm, err, 1);
    }
  }*/

  return Result_Blocked;
}


} } // End of namespaces
