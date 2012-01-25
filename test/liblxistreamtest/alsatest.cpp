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

#include "alsatest.h"
#include "lxistream/plugins/alsa/alsainput.h"
#include "lxistream/plugins/alsa/alsaoutput.h"
#include "lxistream/plugins/alsa/module.h"


void AlsaTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  QVERIFY(SSystem::initialize(SSystem::Initialize_Devices |
                              SSystem::Initialize_LogToConsole, 0));

  QVERIFY(SSystem::loadModule(new AlsaBackend::Module()));
}

void AlsaTest::cleanupTestCase(void)
{
  SSystem::shutdown();
}

/*! Tests the ALSA input and output.
 */
void AlsaTest::AlsaInputOutput(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  // Find ALSA devices
  QStringList devices;
  foreach (const SSystem::DeviceEntry &device, SSystem::availableAudioDevices())
  if (device.url.startsWith("lx-alsa:"))
    devices += device.url;

  if (devices.count() == 0) QSKIP("Test skipped beucause no ALSA devices present.", SkipSingle);

  // Create objects.
  STerminals::AudioDevice * const audio = SSystem::createTerminal<AlsaBackend::AlsaDevice>(this, devices.first());
  SNode * const source = audio->openStream(audio->inputStream(0));
  SNode * const sink = audio->openStream(audio->outputStream(0));

  // Start sampling 1 second of audio and get all buffers
  QVERIFY(source->prepare());
  QTime timer;
  timer.start();
  SBufferQueue audioBuffers;
  while (qAbs(timer.elapsed()) < 1000)
  {
    SBufferList out;
    if (source->processBuffer(SBuffer(), out) == SNode::Result_Active)
      audioBuffers.enqueue(out);
  }
  QVERIFY(source->unprepare());

  QVERIFY(!audioBuffers.isEmpty());
  QVERIFY(audioBuffers.totalLatency() > STime::fromMSec(900));

  // Playback the acquired buffers
  QVERIFY(sink->prepare());
  SBufferList dummy;
  for (SBuffer buffer = audioBuffers.dequeue(); !buffer.isNull(); buffer = audioBuffers.dequeue())
    sink->processBuffer(buffer, dummy);
  QVERIFY(sink->unprepare());

  delete audio;
}
