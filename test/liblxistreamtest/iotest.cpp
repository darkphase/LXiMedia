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

#include "iotest.h"
#include <QtGui>
#include <QtTest>
#include "lxistream/liblxistream/common/audioresampler.h"
#include "lxistream/liblxistream/common/psbufferwriter.h"
#include "lxistream/plugins/gui/module.h"


void IOTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  streamApp = new SApplication(SApplication::Initialize_Devices |
                               SApplication::Initialize_LogToConsole);

  QVERIFY(streamApp->loadModule(new GuiBackend::Module()));
}

void IOTest::cleanupTestCase(void)
{
  delete streamApp;
  streamApp = NULL;
}

/*! Tests deep image file probe.
 */
void IOTest::MediaFileInfoImage(void)
{
  const SMediaInfo mediaInfo(":/ImageTest.jpeg");

  QVERIFY(mediaInfo.audioStreams().isEmpty());
  QVERIFY(mediaInfo.videoStreams().isEmpty());
  QVERIFY(!mediaInfo.imageCodec().isNull());
  QCOMPARE(mediaInfo.imageCodec().size().width(), 570);
  QCOMPARE(mediaInfo.imageCodec().size().height(), 717);
  QVERIFY(qFuzzyCompare(mediaInfo.imageCodec().size().aspectRatio(), 1.0f));
  QVERIFY(!mediaInfo.containsAudio());
  QVERIFY(!mediaInfo.containsVideo());
  QVERIFY(mediaInfo.containsImage());
  QCOMPARE(mediaInfo.title(), QString("ImageTest"));
  QVERIFY(!mediaInfo.thumbnails().isEmpty());

  const QImage image = QImage::fromData(mediaInfo.thumbnails().first());
  QVERIFY(image.width() <= 256);
  QVERIFY(image.width() > 128);
  QVERIFY(image.height() <= 256);
  QVERIFY(image.height() > 128);
}

/*! Tests the AudioResampler converting from stereo to mono.
 */
void IOTest::AudioResamplerStereoMono(void)
{
  Common::AudioResampler audioResampler("", NULL);

  // Prepare a two-channel buffer with value 32 on one channel and value 64 on
  // the other
  SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                     SAudioFormat::Channel_Stereo,
                                     8000),
                        64);

  for (unsigned i=0; i<64; i++)
  {
    reinterpret_cast<qint16 *>(inBuffer.data())[i*2]   = 32;
    reinterpret_cast<qint16 *>(inBuffer.data())[i*2+1] = 64;
  }

  // Now resample it to a mono buffer
  audioResampler.setFormat(SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Mono, 8000));
  const SAudioBuffer outBuffer = audioResampler.processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 128 - 8);
  QCOMPARE(outBuffer.numSamples(), 64 - 4);
  QCOMPARE(outBuffer.format().numChannels(), 1);
  for (int i=0; i<outBuffer.size()/2; i++)
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i], qint16(48));
}

/*! Tests the AudioResampler converting from mono to stereo.
 */
void IOTest::AudioResamplerMonoStereo(void)
{
  Common::AudioResampler audioResampler("", NULL);

  // Prepare a one-channel buffer with value 32 in all samples
  SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                     SAudioFormat::Channel_Mono,
                                     8000),
                        64);

  for (unsigned i=0; i<64; i++)
    reinterpret_cast<qint16 *>(inBuffer.data())[i] = 32;

  // Now resample it to a stereo buffer
  audioResampler.setFormat(SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Stereo, 8000));
  const SAudioBuffer outBuffer = audioResampler.processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 256 - 16);
  QCOMPARE(outBuffer.numSamples(), 64 - 4);
  QCOMPARE(outBuffer.format().numChannels(), 2);
  for (int i=0; i<outBuffer.size()/2; i++)
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i], qint16(32));
}

/*! Tests the AudioResampler converting to half the samplerate.
 */
void IOTest::AudioResamplerHalfRate(void)
{
  Common::AudioResampler audioResampler("", NULL);

  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                     SAudioFormat::Channel_Mono,
                                     8000),
                        64);

  for (unsigned i=0; i<64; i+=2)
  {
    reinterpret_cast<qint16 *>(inBuffer.data())[i]   = 32;
    reinterpret_cast<qint16 *>(inBuffer.data())[i+1] = 64;
  }

  // Now resample it to half the samplerate
  audioResampler.setFormat(SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Mono, 4000));
  const SAudioBuffer outBuffer = audioResampler.processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 64 - 4);
  QCOMPARE(outBuffer.numSamples(), 32 - 2);
  QCOMPARE(outBuffer.format().numChannels(), 1);
  for (int i=0; i<outBuffer.size()/2; i++)
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i], qint16(32));
}

/*! Tests the AudioResampler converting to double the samplerate.
 */
void IOTest::AudioResamplerDoubleRate(void)
{
  Common::AudioResampler audioResampler("", NULL);

  // Prepare a one-channel buffer with alternating values 32 and 64
  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                     SAudioFormat::Channel_Mono,
                                     4000),
                        64);

  for (unsigned i=0; i<64; i+=2)
  {
    reinterpret_cast<qint16 *>(inBuffer.data())[i]   = 32;
    reinterpret_cast<qint16 *>(inBuffer.data())[i+1] = 64;
  }

  // Now resample it to half the samplerate
  audioResampler.setFormat(SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Mono, 8000));
  const SAudioBuffer outBuffer = audioResampler.processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 256 - 16);
  QCOMPARE(outBuffer.numSamples(), 128 - 8);
  QCOMPARE(outBuffer.format().numChannels(), 1);
  for (int i=0; i<outBuffer.size()/2; i+=4)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i],   qint16(32));
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+1], qint16(48));
    if (i != 124) QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+2], qint16(64));
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+3], qint16(48));
  }
}

/*! Creates a temporary file and verifies that buffers can written and read.
 */
void IOTest::PsFileLoopback(void)
{
/*  static const unsigned numBuffers = 8;

  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer buffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                   SAudioFormat::Channel_Mono,
                                   8000),
                      2048);

  for (unsigned i=0; i<2048; i+=2)
  {
    reinterpret_cast<qint16 *>(buffer.data())[i]   = 32;
    reinterpret_cast<qint16 *>(buffer.data())[i+1] = 64;
  }

  buffer.setTimeStamp(STime::null);

  QDir temp(QDir::tempPath());
  const QString fileName = temp.absoluteFilePath("IOTest__PsFileLoopback.mpeg");
  temp.remove(fileName);

  SFileOutputNode fileOutput(NULL, fileName);
  QVERIFY(fileOutput.openFormat(Common::PsBufferWriter::formatName));
  QVERIFY(fileOutput.start(NULL));

  for (unsigned i=0; i<numBuffers; i++)
  {
    buffer.setTimeStamp(buffer.duration() * i);
    fileOutput.input(buffer);
  }

  fileOutput.stop();

  SFileInputNode fileInput(NULL, fileName);
  connect(&fileInput, SIGNAL(output(const SAudioBuffer &)), SLOT(receive(const SAudioBuffer &)));
  QVERIFY(fileInput.start());

  while (!fileInput.atEnd())
    fileInput.process();

  fileInput.stop();

  QVERIFY(!audioBufferList.isEmpty());

  // And check the result
  const SAudioBuffer mergedBuffer = audioBufferList; // Merges all audio buffers
  QCOMPARE(mergedBuffer.numBytes(), size_t(4096 * numBuffers));
  QCOMPARE(mergedBuffer.codec().numChannels(), 1u);
  for (unsigned i=0; i<2048*numBuffers; i+=2)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(mergedBuffer.bits())[i],   qint16(32));
    QCOMPARE(reinterpret_cast<const qint16 *>(mergedBuffer.bits())[i+1], qint16(64));
  }

  audioBufferList.clear();
  QVERIFY(temp.remove(fileName));*/
}

void IOTest::receive(const SAudioBuffer &buffer)
{
  audioBufferList += buffer;
}
