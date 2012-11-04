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

#include "module.h"
#include "pulseaudiodevices.h"
#include "pulseaudioinput.h"
#include "pulseaudiooutput.h"
#include <QtGui/QApplication>

namespace LXiStreamDevice {
namespace PulseAudioBackend {

bool Module::registerClasses(void)
{
  int result = false;

  if (qobject_cast<QApplication *>(QCoreApplication::instance()) == NULL)
  {
    qWarning() <<
        "Not loading PulseAudio because this is a non-gui application (i.e. "
        "a QCoreApplication is used instead of a QApplication)";

    return false; // Non-gui application
  }

  pa_sample_spec sampleSpec;
  sampleSpec.format = PA_SAMPLE_S16LE;
  sampleSpec.rate = 48000;
  sampleSpec.channels = 2;

  pa_simple * const handle = pa_simple_new(
      NULL,
      SApplication::name(),
      PA_STREAM_PLAYBACK,
      NULL,
      "PulseAudioDevice::listDevices",
      &sampleSpec,
      NULL,
      NULL,
      NULL);

  if (handle)
  {
    pa_simple_free(handle);

    PulseAudioOutput::registerClass<PulseAudioOutput>(1);

    PulseAudioDevices devices;
    foreach (const pa_source_info &input, devices.inputDevices())
    if (QString(input.name).endsWith(".monitor", Qt::CaseInsensitive))
    {
      PulseAudioInput::registerClass<PulseAudioInput>(SFactory::Scheme(1, "Desktop"));
      break;
    }

    result = true;
  }

  return result;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "PulseAudio plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      " <h3>PulseAudio</h3>\n"
      " <p>Website: <a href=\"http://www.pulseaudio.org/\">www.pulseaudio.org</a></p>\n"
      " <p>Used under the terms of the GNU Lesser General Public License version 2.1\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

pa_sample_format_t toPulseAudio(SAudioFormat::Format format)
{
  switch(format)
  {
  case SAudioFormat::Format_PCM_U8:     return PA_SAMPLE_U8;
  case SAudioFormat::Format_PCM_ALAW:   return PA_SAMPLE_ALAW;
  case SAudioFormat::Format_PCM_MULAW:  return PA_SAMPLE_ULAW;
  case SAudioFormat::Format_PCM_S16LE:  return PA_SAMPLE_S16LE;
  case SAudioFormat::Format_PCM_S16BE:  return PA_SAMPLE_S16BE;
  case SAudioFormat::Format_PCM_F32LE:  return PA_SAMPLE_FLOAT32LE;
  case SAudioFormat::Format_PCM_F32BE:  return PA_SAMPLE_FLOAT32BE;
  case SAudioFormat::Format_PCM_S32LE:  return PA_SAMPLE_S32LE;
  case SAudioFormat::Format_PCM_S32BE:  return PA_SAMPLE_S32BE;
  case SAudioFormat::Format_PCM_S24LE:  return PA_SAMPLE_S24LE;
  case SAudioFormat::Format_PCM_S24BE:  return PA_SAMPLE_S24BE;
  default:                              return PA_SAMPLE_INVALID;
  }
}

SAudioFormat::Format fromPulseAudio(pa_sample_format_t format)
{
  switch(format)
  {
  case PA_SAMPLE_U8:        return SAudioFormat::Format_PCM_U8;
  case PA_SAMPLE_ALAW:      return SAudioFormat::Format_PCM_ALAW;
  case PA_SAMPLE_ULAW:      return SAudioFormat::Format_PCM_MULAW;
  case PA_SAMPLE_S16LE:     return SAudioFormat::Format_PCM_S16LE;
  case PA_SAMPLE_S16BE:     return SAudioFormat::Format_PCM_S16BE;
  case PA_SAMPLE_FLOAT32LE: return SAudioFormat::Format_PCM_F32LE;
  case PA_SAMPLE_FLOAT32BE: return SAudioFormat::Format_PCM_F32BE;
  case PA_SAMPLE_S32LE:     return SAudioFormat::Format_PCM_S32LE;
  case PA_SAMPLE_S32BE:     return SAudioFormat::Format_PCM_S32BE;
  case PA_SAMPLE_S24LE:     return SAudioFormat::Format_PCM_S24LE;
  case PA_SAMPLE_S24BE:     return SAudioFormat::Format_PCM_S24BE;
  case PA_SAMPLE_S24_32LE:  return SAudioFormat::Format_PCM_S32LE;
  case PA_SAMPLE_S24_32BE:  return SAudioFormat::Format_PCM_S32BE;
  default:                  return SAudioFormat::Format_Invalid;
  }
}

} } // End of namespaces
