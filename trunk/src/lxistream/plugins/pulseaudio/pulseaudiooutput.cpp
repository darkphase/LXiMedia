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

#include "pulseaudiooutput.h"

namespace LXiStream {
namespace PulseAudioBackend {


PulseAudioOutput::PulseAudioOutput(const QString &server, QObject *parent)
  : SInterfaces::AudioOutput(parent),
    server(server),
    handle(NULL),
    outLatency(STime::null),
    inFormat()
{
}

PulseAudioOutput::~PulseAudioOutput()
{
  if (handle)
    pa_simple_free(handle);
}

bool PulseAudioOutput::start(void)
{
  return true;
}

void PulseAudioOutput::stop(void)
{
  if (handle)
  {
    pa_simple_drain(handle, NULL);
    pa_simple_free(handle);
    handle = NULL;
  }
}

STime PulseAudioOutput::latency(void) const
{
  return outLatency;
}

void PulseAudioOutput::consume(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  {
    if ((inFormat != audioBuffer.format()) || (handle == NULL))
      openCodec(audioBuffer.format());

    if (handle)
    {
      const qint16 * const data = reinterpret_cast<const qint16 *>(audioBuffer.data());
      const size_t dataSize = audioBuffer.size();

      if (dataSize > 0)
        pa_simple_write(handle, data, dataSize, NULL);

      outLatency = STime::fromUSec(handle ? pa_simple_get_latency(handle, NULL) : 0);
    }
  }
}

void PulseAudioOutput::openCodec(const SAudioFormat &reqFormat)
{
  if (handle)
  {
    pa_simple_free(handle);
    handle = NULL;
  }

  if (!reqFormat.isNull())
  {
    pa_sample_spec sampleSpec;
    sampleSpec.format = PA_SAMPLE_S16LE;
    sampleSpec.rate = reqFormat.sampleRate();
    sampleSpec.channels = reqFormat.numChannels();

    handle = pa_simple_new(server.isEmpty() ? NULL : server.toAscii().data(),
                           SSystem::name(),
                           PA_STREAM_PLAYBACK,
                           NULL,
                           "PulseAudioOutput",
                           &sampleSpec,
                           NULL,
                           NULL,
                           NULL);

    if (!handle)
      qWarning() << "PulseAudioOutput: Can't create output stream.";

    inFormat = reqFormat;
  }
}


} } // End of namespaces
