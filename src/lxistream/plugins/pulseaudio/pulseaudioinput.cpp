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

#include "pulseaudioinput.h"

namespace LXiStream {
namespace PulseAudioBackend {


PulseAudioInput::PulseAudioInput(const QString &server, QObject *parent)
  : SInterfaces::AudioInput(parent),
    server(server),
    handle(NULL),
    outFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Stereo, 48000)
{
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
  if (handle)
  {
    pa_simple_free(handle);
    handle = NULL;
  }

  pa_sample_spec sampleSpec;
  sampleSpec.format = PA_SAMPLE_S16LE;
  sampleSpec.rate = outFormat.sampleRate();
  sampleSpec.channels = outFormat.numChannels();

  handle = pa_simple_new(server.isEmpty() ? NULL : server.toAscii().data(),
                         SSystem::name(),
                         PA_STREAM_RECORD,
                         NULL,
                         "PulseAudioInput",
                         &sampleSpec,
                         NULL,
                         NULL,
                         NULL);

  return handle;
}

void PulseAudioInput::stop(void)
{
  if (handle)
  {
    pa_simple_flush(handle, NULL);
    pa_simple_free(handle);
    handle = NULL;
  }
}

void PulseAudioInput::process(void)
{
  const unsigned numSamples = outFormat.sampleRate() * 40 / 1000; // 40 ms of samples
  SAudioBuffer buffer(outFormat, numSamples);

  const int r = pa_simple_read(handle, buffer.data(), buffer.size(), NULL);
  if (r >= 0)
  {
    const STime latency = STime::fromUSec(pa_simple_get_latency(handle, NULL));

    buffer.setTimeStamp(timer.smoothTimeStamp(buffer.duration(), latency));

    emit produce(buffer);
  }
}


} } // End of namespaces
