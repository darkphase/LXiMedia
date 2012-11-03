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

#ifndef PULSEAUDIODEVICES_H
#define PULSEAUDIODEVICES_H

#include <QtCore>
#include <pulse/pulseaudio.h>

namespace LXiStreamDevice {
namespace PulseAudioBackend {

class PulseAudioDevices : public QObject
{
Q_OBJECT
public:
                                PulseAudioDevices(QObject * = NULL);
  virtual                       ~PulseAudioDevices();

  inline const QMap<QString, pa_source_info> & inputDevices() const { return inputs; }
  inline const QMap<QString, pa_sink_info> & outputDevices() const { return outputs; }

private:
  static void                   stateCallback(pa_context *, void *);
  static void                   sinklistCallback(pa_context *, const pa_sink_info *, int, void *);
  static void                   sourcelistCallback(pa_context *, const pa_source_info *, int, void *);
  void                          buildDevicelist();

private:
  QMap<QString, pa_source_info> inputs;
  QMap<QString, pa_sink_info>   outputs;
};

} } // End of namespaces

#endif
