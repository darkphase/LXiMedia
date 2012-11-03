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

    PulseAudioInput::registerClass<PulseAudioInput>(SFactory::Scheme(1, "Desktop"));
    PulseAudioOutput::registerClass<PulseAudioOutput>(1);

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

} } // End of namespaces
