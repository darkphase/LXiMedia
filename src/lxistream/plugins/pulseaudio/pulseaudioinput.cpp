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

#include "pulseaudioinput.h"
#include "pulseaudiodevices.h"

namespace LXiStreamDevice {
namespace PulseAudioBackend {

PulseAudioInput::PulseAudioInput(const QString &, QObject *parent)
  : SInterfaces::AudioInput(parent),
    handle(NULL),
    outFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 48000)
{
  PulseAudioDevices devices;
  foreach (const pa_source_info &input, devices.inputDevices())
  if (QString(input.name).endsWith(".monitor", Qt::CaseInsensitive))
  {
    handle = pa_simple_new(
          NULL,
          SApplication::name(),
          PA_STREAM_RECORD,
          input.name,
          "PulseAudioInput",
          &(input.sample_spec),
          NULL, NULL, NULL);

    if (handle)
    {
      switch(input.sample_spec.format)
      {
      case PA_SAMPLE_U8:        outFormat.setFormat(SAudioFormat::Format_PCM_U8);     break;
      case PA_SAMPLE_ALAW:      outFormat.setFormat(SAudioFormat::Format_PCM_ALAW);   break;
      case PA_SAMPLE_ULAW:      outFormat.setFormat(SAudioFormat::Format_PCM_MULAW);  break;
      case PA_SAMPLE_S16LE:     outFormat.setFormat(SAudioFormat::Format_PCM_S16LE);  break;
      case PA_SAMPLE_S16BE:     outFormat.setFormat(SAudioFormat::Format_PCM_S16BE);  break;
      case PA_SAMPLE_FLOAT32LE: outFormat.setFormat(SAudioFormat::Format_PCM_F32LE);  break;
      case PA_SAMPLE_FLOAT32BE: outFormat.setFormat(SAudioFormat::Format_PCM_F32BE);  break;
      case PA_SAMPLE_S32LE:     outFormat.setFormat(SAudioFormat::Format_PCM_S32LE);  break;
      case PA_SAMPLE_S32BE:     outFormat.setFormat(SAudioFormat::Format_PCM_S32BE);  break;
      case PA_SAMPLE_S24LE:     outFormat.setFormat(SAudioFormat::Format_PCM_S24LE);  break;
      case PA_SAMPLE_S24BE:     outFormat.setFormat(SAudioFormat::Format_PCM_S24BE);  break;
      case PA_SAMPLE_S24_32LE:  outFormat.setFormat(SAudioFormat::Format_PCM_S32LE);  break;
      case PA_SAMPLE_S24_32BE:  outFormat.setFormat(SAudioFormat::Format_PCM_S32BE);  break;
      default:                  outFormat.setFormat(SAudioFormat::Format_PCM_S16LE);  break;
      }

      outFormat.setSampleRate(input.sample_spec.rate);
      outFormat.setChannelSetup(outFormat.guessChannels(input.sample_spec.channels));

      break;
    }
  }
}

PulseAudioInput::~PulseAudioInput()
{
  if (handle)
    pa_simple_free(handle);
}

void PulseAudioInput::setFormat(const SAudioFormat &f)
{
  outFormat = f;
}

SAudioFormat PulseAudioInput::format(void)
{
  return outFormat;
}

bool PulseAudioInput::start(void)
{
  return handle;
}

void PulseAudioInput::stop(void)
{
}

bool PulseAudioInput::process(void)
{
  const unsigned numSamples = outFormat.sampleRate() * 40 / 1000; // 40 ms of samples
  SAudioBuffer buffer(outFormat, numSamples);

  const int r = pa_simple_read(handle, buffer.data(), buffer.size(), NULL);
  if (r >= 0)
  {
    const STime latency = STime::fromUSec(pa_simple_get_latency(handle, NULL));

    buffer.setTimeStamp(timer.smoothTimeStamp(buffer.duration(), latency));

    emit produce(buffer);
    return true;
  }

  return false;
}

} } // End of namespaces
