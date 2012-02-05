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

#include "iotest.h"
#include <QtTest>

void IOTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);

  QVERIFY(mediaApp->loadModule("lxistream_ffmpeg"));
  QVERIFY(mediaApp->loadModule("lxistream_gui"));
}

void IOTest::cleanupTestCase(void)
{
  messageList.clear();
  audioBufferList.clear();
  videoBufferList.clear();

  delete mediaApp;
  mediaApp = NULL;
}

/*! Tests deep image file probe.
 */
void IOTest::MediaFileInfoImage(void)
{
  SMediaInfo mediaInfo(QUrl("file::/ImageTest.jpeg"));

  verifyMediaFileInfoImage(mediaInfo);

  const SImage image = mediaInfo.readThumbnail(QSize(128, 128));
  QVERIFY(image.width() <= 128);
  QVERIFY(image.width() > 64);
  QVERIFY(image.height() <= 128);
  QVERIFY(image.height() > 64);
}

/*! Tests deep image file probe.
 */
void IOTest::MediaFileInfoSerialize(void)
{
  SMediaInfo mediaInfo(QUrl("file::/ImageTest.jpeg"));
  mediaInfo.probeContent();

  QByteArray data;
  {
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    mediaInfo.serialize(writer);
  }

  SMediaInfo read;
  {
    QXmlStreamReader reader(data);
    QVERIFY(reader.readNextStartElement());
    read.deserialize(reader);
  }

  verifyMediaFileInfoImage(read);
}

void IOTest::verifyMediaFileInfoImage(const SMediaInfo &mediaInfo)
{
  QVERIFY(mediaInfo.titles().first().audioStreams.isEmpty());
  QVERIFY(mediaInfo.titles().first().videoStreams.isEmpty());
  QVERIFY(!mediaInfo.titles().first().imageCodec.isNull());
  QCOMPARE(mediaInfo.fileType(), SMediaInfo::ProbeInfo::FileType_Image);
  QCOMPARE(mediaInfo.metadata("title").toString(), QString("ImageTest"));
}

/*! Tests the AudioResampler converting to half the samplerate.
 */
void IOTest::AudioResamplerHalfRate(void)
{
  SInterfaces::AudioResampler * const audioResampler =
      SInterfaces::AudioResampler::create(this, "linear");

  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer inBuffer = createAudioBuffer(8000);

  // Now resample it to half the samplerate
  audioResampler->setSampleRate(4000);
  const SAudioBuffer outBuffer = audioResampler->processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 64 - 4);
  QCOMPARE(outBuffer.numSamples(), 32u - 2u);
  QCOMPARE(outBuffer.format().numChannels(), 1u);
  for (int i=0; i<outBuffer.size()/2; i++)
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i], qint16(32));

  delete audioResampler;
}

/*! Tests the AudioResampler converting to double the samplerate.
 */
void IOTest::AudioResamplerDoubleRate(void)
{
  SInterfaces::AudioResampler * const audioResampler =
      SInterfaces::AudioResampler::create(this, "linear");

  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer inBuffer = createAudioBuffer(4000);

  // Now resample it to double the samplerate
  audioResampler->setSampleRate(8000);
  const SAudioBuffer outBuffer = audioResampler->processBuffer(inBuffer);
  QVERIFY(!outBuffer.isNull());

  // And check the result, note that the last 4 input samples are dropped.
  QCOMPARE(outBuffer.size(), 256 - 16);
  QCOMPARE(outBuffer.numSamples(), 128u - 8u);
  QCOMPARE(outBuffer.format().numChannels(), 1u);
  for (int i=0; i<outBuffer.size()/2; i+=4)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i],   qint16(32));
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+1], qint16(48));
    if (i != 124) QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+2], qint16(64));
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+3], qint16(48));
  }

  delete audioResampler;
}

/*! Serializes and deserializes buffers.
 */
void IOTest::BufferSerializerLoopback(void)
{
  static const int numBuffers = 32;

  QBuffer buffer;
  SAudioBuffer audioBuffer = createAudioBuffer(8000);
  SVideoBuffer videoBuffer = createVideoBuffer(SSize(352, 288));

  buffer.open(QBuffer::ReadWrite);
  SBufferSerializerNode bufferSerializer(NULL, &buffer);
  for (int i=0; i<numBuffers; i++)
  {
    bufferSerializer.input("BufferSequence" + QByteArray::number(i));
    bufferSerializer.input(audioBuffer);
    bufferSerializer.input(videoBuffer);
  }

  buffer.seek(0);
  SBufferDeserializerNode bufferDeserializer(NULL, &buffer);
  connect(&bufferDeserializer, SIGNAL(output(QByteArray)), SLOT(receive(QByteArray)));
  connect(&bufferDeserializer, SIGNAL(output(SAudioBuffer)), SLOT(receive(SAudioBuffer)));
  connect(&bufferDeserializer, SIGNAL(output(SVideoBuffer)), SLOT(receive(SVideoBuffer)));

  messageList.clear();
  audioBufferList.clear();
  videoBufferList.clear();
  for (int i=0; i<numBuffers; i++)
  {
    bufferDeserializer.process();
    bufferDeserializer.process();
    bufferDeserializer.process();
  }

  QCOMPARE(messageList.count(), numBuffers);
  for (int i=0; i<messageList.count(); i++)
    QCOMPARE(messageList[i], "BufferSequence" + QByteArray::number(i));

  QCOMPARE(audioBufferList.count(), numBuffers);
  foreach (const SAudioBuffer &buffer, audioBufferList)
  {
    QCOMPARE(buffer.format(), audioBuffer.format());
    QCOMPARE(buffer.timeStamp(), audioBuffer.timeStamp());
    QCOMPARE(buffer.numSamples(), audioBuffer.numSamples());
    QCOMPARE(buffer.duration(), audioBuffer.duration());

    QCOMPARE(buffer.size(), audioBuffer.size());
    QVERIFY(memcmp(buffer.data(), audioBuffer.data(), qMin(buffer.size(), audioBuffer.size())) == 0);
  }

  QCOMPARE(videoBufferList.count(), numBuffers);
  foreach (const SVideoBuffer &buffer, videoBufferList)
  {
    QCOMPARE(buffer.format(), videoBuffer.format());
    QCOMPARE(buffer.timeStamp(), videoBuffer.timeStamp());
    QCOMPARE(buffer.lineSize(0), videoBuffer.lineSize(0));
    QCOMPARE(buffer.offset(0), videoBuffer.offset(0));

    QCOMPARE(buffer.size(), videoBuffer.size());
    QVERIFY(memcmp(buffer.data(), videoBuffer.data(), qMin(buffer.size(), videoBuffer.size())) == 0);
  }
}

void IOTest::receive(const QByteArray &buffer)
{
  messageList += buffer;
}

void IOTest::receive(const SAudioBuffer &buffer)
{
  audioBufferList += buffer;
}

void IOTest::receive(const SVideoBuffer &buffer)
{
  videoBufferList += buffer;
}

SAudioBuffer IOTest::createAudioBuffer(unsigned sampleRate)
{
  SAudioBuffer buffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                   SAudioFormat::Channels_Mono,
                                   sampleRate),
                        64);

  for (unsigned i=0; i<64; i+=2)
  {
    reinterpret_cast<qint16 *>(buffer.data())[i]   = 32;
    reinterpret_cast<qint16 *>(buffer.data())[i+1] = 64;
  }

  return buffer;
}

SVideoBuffer IOTest::createVideoBuffer(const SSize &size)
{
  SVideoBuffer buffer(SVideoFormat(SVideoFormat::Format_GRAY8, size));

  for (int y=0; y<size.height(); y++)
  {
    quint8 * const line = reinterpret_cast<quint8 *>(buffer.scanLine(y, 0));
    for (int x=0; x<size.width(); x++)
      line[x] = x + y;
  }

  return buffer;
}
