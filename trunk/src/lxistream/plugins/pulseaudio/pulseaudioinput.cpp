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
#include "module.h"

namespace LXiStreamDevice {
namespace PulseAudioBackend {

PulseAudioInput::PulseAudioInput(const QString &, QObject *parent)
  : SInterfaces::AudioInput(parent),
    handle(NULL),
    outFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 48000),
    bufferRate(SInterval::fromFrequency(25))
{
  PulseAudioDevices devices;
  foreach (const pa_source_info &input, devices.inputDevices())
  if (QString(input.name).endsWith(".monitor", Qt::CaseInsensitive))
  {
    inputName = input.name;

    outFormat.setFormat(fromPulseAudio(input.sample_spec.format));
    outFormat.setSampleRate(input.sample_spec.rate);
    outFormat.setChannelSetup(outFormat.guessChannels(input.sample_spec.channels));

    break;
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
  openHandle();

  // Create a silent audio buffer
  silentBuffer.setFormat(outFormat);
  silentBuffer.setNumSamples(outFormat.sampleRate() / bufferRate.toFrequency());
  memset(silentBuffer.data(), 0, silentBuffer.size());

  lastTryOpen = lastBuffer = timer.timeStamp();

  return true;
}

void PulseAudioInput::stop(void)
{
  if (handle)
  {
    pa_simple_free(handle);
    handle = NULL;
  }
}

bool PulseAudioInput::process(void)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  if (handle)
  {
    SAudioBuffer buffer(outFormat, outFormat.sampleRate() / bufferRate.toFrequency());

    const int r = pa_simple_read(handle, buffer.data(), buffer.size(), NULL);
    if (r >= 0)
    {
      lastBuffer = timer.smoothTimeStamp(buffer.duration());
      buffer.setTimeStamp(lastBuffer);

      emit produce(buffer);
      return true;
    }
    else
    {
      pa_simple_free(handle);
      handle = NULL;
    }
  }

  if (handle == NULL)
  {
    const STime now = timer.timeStamp();
    const STime next = lastBuffer + bufferRate;
    const qint64 wait = (next - now).toMSec();

    if (qAbs(wait) >= 1000)
      lastBuffer = now;
    else if (wait > 0)
      T::msleep(wait);

    lastBuffer += bufferRate;

    silentBuffer.setTimeStamp(lastBuffer);
    emit produce(silentBuffer);

    if (qAbs((now - lastTryOpen).toMSec()) >= 1000)
    {
      openHandle();
      lastTryOpen = now;
    }

    return true;
  }

  return false;
}

bool PulseAudioInput::openHandle()
{
  pa_sample_spec sampleSpec;
  sampleSpec.format = toPulseAudio(outFormat.format());
  sampleSpec.rate = outFormat.sampleRate();
  sampleSpec.channels = outFormat.numChannels();

  handle = pa_simple_new(
        NULL,
        SApplication::name(),
        PA_STREAM_RECORD,
        inputName,
        "PulseAudioInput",
        &sampleSpec,
        NULL, NULL, NULL);

  return handle;
}

} } // End of namespaces
