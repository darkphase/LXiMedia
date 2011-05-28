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

    PulseAudioInput::registerClass<PulseAudioInput>(1);
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
      "<h3>PulseAudio</h3>\n"
      "Website: <a href=\"http://www.pulseaudio.org/\">www.pulseaudio.org</a><br />\n"
      "<br />\n"
      "Used under the terms of the GNU Lesser General Public License version 2.1\n"
      "as published by the Free Software Foundation.\n";

  return text;
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lxistreamdevice_pulseaudio, LXiStreamDevice::PulseAudioBackend::Module);
