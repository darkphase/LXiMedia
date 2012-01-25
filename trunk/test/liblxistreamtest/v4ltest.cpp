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

#include "test.h"
#include "lxistream/plugins/v4l/module.h"
#include "lxistream/plugins/v4l/v4l1device.h"
#include "lxistream/plugins/v4l/v4l2device.h"

/*! Loads the V4lBackend plugin.

    \node This is required before any of the other tests depending on the
          V4lBackend can run.
 */
void LXiStreamTest::V4lBackend_Load(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  QVERIFY(SSystem::loadModule(new V4lBackend::Module()));
}

/*! Tests the V4l1 input.
 */
void LXiStreamTest::V4lBackend_V4l1Input(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  SGraph graph(SGraph::MediaTask_None);

  // Find V4L1 devices
  QStringList devices;
  foreach (const SSystem::DeviceEntry &device, graph.availableVideoCaptureDevices())
  if (device.url.startsWith("lx-v4l1:"))
    devices += device.url;

  if (devices.count() == 0) QSKIP("Test skipped beucause no V4L1 devices present.", SkipSingle);

  V4lBackend_input(SSystem::createTerminal<V4lBackend::V4l1Device>(this, devices.first()));
}

/*! Tests the V4l2 input.
 */
void LXiStreamTest::V4lBackend_V4l2Input(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  SGraph graph(SGraph::MediaTask_None);

  // Find V4L1 devices
  QStringList devices;
  foreach (const SSystem::DeviceEntry &device, graph.availableVideoCaptureDevices())
  if (device.url.startsWith("lx-v4l2:"))
    devices += device.url;

  if (devices.count() == 0) QSKIP("Test skipped beucause no V4L2 devices present.", SkipSingle);

  V4lBackend_input(SSystem::createTerminal<V4lBackend::V4l2Device>(this, devices.first()));
}

void LXiStreamTest::V4lBackend_input(LXiStream::STerminal *video)
{
  //qDebug() << device;

  // Create objects.
  SAnalogTuner * const tuner = qobject_cast<SAnalogTuner *>(video->tuner());
  if (tuner)
  {
    QVERIFY(tuner->setAudioStandard(SAnalogTuner::AudioStandard_Mono));
    QVERIFY(tuner->setVideoStandard(SAnalogTuner::VideoStandard_PAL_BG));
    QVERIFY(tuner->setFrequency(215875000));
    QCOMPARE(tuner->frequency(), quint64(215875000));
  }

  SNode * const source = video->openStream(video->inputStream(0));

  // Start sampling 1 second of video and get all buffers
  QVERIFY(source->prepare(SImageBuffer::toImageCodecs()));
  QTime timer;
  timer.start();
  SBufferQueue imageBuffers;
  while (qAbs(timer.elapsed()) < 1000)
  {
    SBufferList out;
    if (source->processBuffer(SBuffer(), out) == SNode::Result_Active)
      imageBuffers.enqueue(out);
  }
  QVERIFY(source->unprepare());

  QVERIFY(!imageBuffers.isEmpty());

  // Write the acquired buffers to tmp
  //int n=0;
  //for (SBuffer buffer = imageBuffers.dequeue(); !buffer.isNull(); buffer = imageBuffers.dequeue())
  //if (SImageBuffer::isMyType(buffer))
  //  SImageBuffer(buffer).toImage().save("/tmp/buffer" + QString::number(n++) + ".png");

  delete video;
}
