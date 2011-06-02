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

#include "streamtest.h"
#include <QtTest>

SAudioBuffer StreamTest::makeTestBuffer(unsigned numSamples)
{
  SAudioBuffer buffer(
      SAudioFormat(
          SAudioFormat::Format_PCM_S16,
          SAudioFormat::Channels_Stereo,
          44100),
      numSamples);

  for (int i=0, n=buffer.numSamples()*buffer.format().numChannels(); i<n; i+=4)
  {
    reinterpret_cast<qint16 *>(buffer.data())[i]   = -64;
    reinterpret_cast<qint16 *>(buffer.data())[i+1] = -32;
    reinterpret_cast<qint16 *>(buffer.data())[i+2] =  64;
    reinterpret_cast<qint16 *>(buffer.data())[i+3] =  32;
  }

  return buffer;
}


void StreamTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);
}

void StreamTest::cleanupTestCase(void)
{
  delete mediaApp;
  mediaApp = NULL;
}

/*! Tests the buffer data.
 */
void StreamTest::BufferData(void)
{
  SBuffer buffer(4096);
  QVERIFY(buffer.capacity() >= 4096);

  for (unsigned i=0; i<4096; i++)
    buffer.data()[i] = char(i & 0xFFu);

  buffer.resize(4096);

  for (int i=0; i<buffer.size(); i++)
    QCOMPARE(buffer.data()[i], char(i & 0xFFu));
}

/*! Tests the implicit cloning of buffer data.
 */
void StreamTest::BufferClone(void)
{
  // Original buffer
  SBuffer buffer(4096);
  const char * bufferAddr = buffer.data();

  fillBuffer(buffer.data(), 4096);
  buffer.resize(4096);

  // First reference the buffer
  SBuffer bufferRef = buffer;

  QCOMPARE(bufferRef.capacity(), buffer.capacity());
  QCOMPARE(bufferRef.size(), buffer.size());
  QVERIFY(memcmp(bufferRef.constData(), bufferAddr, bufferRef.size()) == 0);

  QCOMPARE(buffer.constData(), bufferAddr);
  QCOMPARE(bufferRef.constData(), bufferAddr);

  // And now enforce a clone of the second buffer
  const char * bufferRefAddr = bufferRef.data();
  QVERIFY(bufferRefAddr != bufferAddr);
  QCOMPARE(buffer.constData(), bufferAddr);
  QCOMPARE(bufferRef.constData(), bufferRefAddr);

  QCOMPARE(bufferRef.capacity(), buffer.capacity());
  QCOMPARE(bufferRef.size(), buffer.size());
  QVERIFY(memcmp(bufferRef.data(), bufferAddr, bufferRef.size()) == 0);

  QCOMPARE(buffer.constData(), bufferAddr);
  QCOMPARE(bufferRef.constData(), bufferRefAddr);
}

/*! Tests the enlarging of buffers.
 */
void StreamTest::BufferEnlarge(void)
{
  // Original buffer
  SBuffer buffer(4096);
  fillBuffer(buffer.data(), 4096);
  buffer.resize(4096);

  // Clone a buffer
  SBuffer enlargedBuffer = buffer;
  const char * bufferAddr = enlargedBuffer.data(); // Implicitly makes a copy
  QVERIFY(bufferAddr != buffer.data());

  // Expand the buffer to an equal size (should do nothing)
  enlargedBuffer.resize(4096);
  QCOMPARE(enlargedBuffer.data(), bufferAddr);
  QVERIFY(enlargedBuffer.capacity() >= 4096);

  // Expand the buffer to a larger size
  enlargedBuffer.resize(8192);
  QVERIFY(enlargedBuffer.data() != bufferAddr);
  QVERIFY(enlargedBuffer.capacity() >= 8192);
  QVERIFY(memcmp(enlargedBuffer.data(), buffer.data(), buffer.size()) == 0);
}

/*! Tests external buffers.
 */
void StreamTest::BufferExternal(void)
{
  class Memory : public SBuffer::Memory
  {
  public:
    Memory(int capacity, char *data, int size, int *release)
      : SBuffer::Memory(capacity, data, size), release(release)
    {
    }

    virtual ~Memory()
    {
      (*release)++;
    }

  private:
    int * const release;
  };

  char extBuffer[4096];
  int release = 0;
  fillBuffer(extBuffer, sizeof(extBuffer));

  SAudioBuffer buffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                   SAudioFormat::Channels_Stereo,
                                   8000),
                      SBuffer::MemoryPtr(new Memory(sizeof(extBuffer),
                                         extBuffer,
                                         sizeof(extBuffer),
                                         &release)));

  // bits() should not clone this buffer, numBytes should be set to.
  QCOMPARE(buffer.data(), extBuffer);
  QCOMPARE(buffer.size(), int(sizeof(extBuffer)));

  // Reference the buffer.
  SAudioBuffer bufferRef = buffer;
  QCOMPARE(bufferRef.constData(), extBuffer);
  QCOMPARE(bufferRef.size(), int(sizeof(extBuffer)));

  // bits() should clone this buffer.
  QVERIFY(bufferRef.data() != extBuffer);
  QVERIFY(bufferRef.capacity() >= int(sizeof(extBuffer)));
  QVERIFY(memcmp(bufferRef.data(), extBuffer, sizeof(extBuffer)) == 0);

  // Now check if the buffer is released properly
  QCOMPARE(release, 0);
  buffer.clear();
  QCOMPARE(release, 1);
}

/*! This helper function fills a buffer with a deterministic test pattern.
 */
void StreamTest::fillBuffer(char *data, int size)
{
  for (int i=0; i<size; i++)
    data[i] = char(i * 3);
}

/*! Tests the SAudioBuffer class
 */
void StreamTest::AudioBuffer(void)
{
  const SAudioBuffer inBuffer = StreamTest::makeTestBuffer(65536);

  const SAudioBuffer leftBuffer = inBuffer.getChannel(SAudioFormat::Channel_LeftFront);
  const SAudioBuffer centerBuffer = inBuffer.getChannel(SAudioFormat::Channel_CenterFront);
  const SAudioBuffer rightBuffer = inBuffer.getChannel(SAudioFormat::Channel_RightFront);

  QVERIFY(!leftBuffer.isEmpty());
  QVERIFY(centerBuffer.isEmpty());
  QVERIFY(!rightBuffer.isEmpty());

  for (int i=0, n=leftBuffer.numSamples()*leftBuffer.format().numChannels(); i<n; i+=2)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(leftBuffer.data())[i],   qint16(-64));
    QCOMPARE(reinterpret_cast<const qint16 *>(leftBuffer.data())[i+1], qint16( 64));
  }

  for (int i=0, n=rightBuffer.numSamples()*rightBuffer.format().numChannels(); i<n; i+=2)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(rightBuffer.data())[i],   qint16(-32));
    QCOMPARE(reinterpret_cast<const qint16 *>(rightBuffer.data())[i+1], qint16( 32));
  }

  SAudioBuffer outBuffer(inBuffer.format(), inBuffer.numSamples());
  outBuffer.setChannels(leftBuffer);
  outBuffer.setChannels(centerBuffer);
  outBuffer.setChannels(rightBuffer);

  QVERIFY(outBuffer.size() >= inBuffer.size());

  if (outBuffer.size() >= inBuffer.size())
  for (int i=0, n=inBuffer.numSamples()*inBuffer.format().numChannels(); i<n; i+=4)
  {
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i],   reinterpret_cast<const qint16 *>(inBuffer.data())[i]);
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+1], reinterpret_cast<const qint16 *>(inBuffer.data())[i+1]);
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+2], reinterpret_cast<const qint16 *>(inBuffer.data())[i+2]);
    QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+3], reinterpret_cast<const qint16 *>(inBuffer.data())[i+3]);
  }
}

/*! Tests the SAudioFormat class
 */
void StreamTest::AudioFormat(void)
{
  SAudioFormat format = SAudioFormat::Format_PCM_S16;
  QVERIFY(format == SAudioFormat(SAudioFormat::Format_PCM_S16));
  QVERIFY(format != SAudioFormat(SAudioFormat::Format_PCM_F32));
  QVERIFY(format == SAudioFormat::Format_PCM_S16);
  QVERIFY(format != SAudioFormat::Format_PCM_F32);
  QCOMPARE(format.format(), SAudioFormat::Format_PCM_S16);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.numChannels(), 0u);
  QCOMPARE(format.sampleRate(), 0u);

  format.setFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Surround_5_1, 48000);
  QVERIFY(format == SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Surround_5_1, 48000));
  QVERIFY(format != SAudioFormat(SAudioFormat::Format_PCM_S16));
  QVERIFY(format == SAudioFormat::Format_PCM_S16);
  QVERIFY(format != SAudioFormat::Format_PCM_F32);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.numChannels(), 6u);
  QCOMPARE(format.sampleRate(), 48000u);

  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Mono), 1u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Stereo), 2u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Quadraphonic), 4u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Surround_3_0), 3u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Surround_4_0), 4u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Surround_5_0), 5u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Surround_5_1), 6u);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channels_Surround_7_1), 8u);
  QCOMPARE(SAudioFormat::guessChannels(1), SAudioFormat::Channels_Mono);
  QCOMPARE(SAudioFormat::guessChannels(2), SAudioFormat::Channels_Stereo);
  QCOMPARE(SAudioFormat::guessChannels(4), SAudioFormat::Channels_Quadraphonic);
  QCOMPARE(SAudioFormat::guessChannels(6), SAudioFormat::Channels_Surround_5_1);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_S16), 2u);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_S8), 1u);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_F32), 4u);
}

/*! Tests the SAudioCodec class
 */
void StreamTest::AudioCodec(void)
{
  SAudioCodec codec = "AC3";
  QVERIFY(codec == SAudioCodec("AC3"));
  QVERIFY(codec != SAudioCodec("VORBIS"));
  QVERIFY(codec == "AC3");
  QVERIFY(codec != "VORBIS");
  QCOMPARE(codec.codec(), QString("AC3"));
  QCOMPARE(codec.isNull(), false);
  QCOMPARE(codec.numChannels(), 0);
  QCOMPARE(codec.sampleRate(), 0);

  codec.setCodec("VORBIS", SAudioFormat::Channels_Surround_5_1, 48000);
  QVERIFY(codec == SAudioCodec("VORBIS", SAudioFormat::Channels_Surround_5_1, 48000));
  QVERIFY(codec != SAudioCodec("VORBIS"));
  QVERIFY(codec == "VORBIS");
  QVERIFY(codec != "AC3");
  QCOMPARE(codec.codec(), QString("VORBIS"));
  QCOMPARE(codec.isNull(), false);
  QCOMPARE(codec.numChannels(), 6);
  QCOMPARE(codec.sampleRate(), 48000);
}

/*! Tests the SVideoFormat class
 */
void StreamTest::VideoFormat(void)
{
  SVideoFormat format = SVideoFormat::Format_YUYV422;
  QVERIFY(format == SVideoFormat(SVideoFormat::Format_YUYV422));
  QVERIFY(format != SVideoFormat(SVideoFormat::Format_RGB32));
  QVERIFY(format == SVideoFormat::Format_YUYV422);
  QVERIFY(format != SVideoFormat::Format_RGB32);
  QCOMPARE(format.format(), SVideoFormat::Format_YUYV422);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.size().width(), 0);
  QCOMPARE(format.size().height(), 0);
  QVERIFY(qFuzzyCompare(format.size().aspectRatio(), 1.0f));
  QVERIFY(!format.frameRate().isValid());
  QCOMPARE(format.fieldMode(), SVideoFormat::FieldMode_Invalid);

  format.setFormat(SVideoFormat::Format_YUYV422, SSize(384, 288, 0.7f), SInterval::fromFrequency(25), SVideoFormat::FieldMode_Progressive);
  QVERIFY(format == SVideoFormat(SVideoFormat::Format_YUYV422, SSize(384, 288, 0.7f), SInterval::fromFrequency(25), SVideoFormat::FieldMode_Progressive));
  QVERIFY(format != SVideoFormat(SVideoFormat::Format_YUYV422));
  QVERIFY(format == SVideoFormat::Format_YUYV422);
  QVERIFY(format != SVideoFormat::Format_RGB32);
  QCOMPARE(format.format(), SVideoFormat::Format_YUYV422);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.size().width(), 384);
  QCOMPARE(format.size().height(), 288);
  QVERIFY(qFuzzyCompare(format.size().aspectRatio(), 0.7f));
  QVERIFY(qFuzzyCompare(format.frameRate().toFrequency(), 25.0));
  QCOMPARE(format.fieldMode(), SVideoFormat::FieldMode_Progressive);

  QCOMPARE(SVideoFormat::sampleSize(SVideoFormat::Format_RGB32), 4);
  QCOMPARE(SVideoFormat::sampleSize(SVideoFormat::Format_RGB555), 2);
  QCOMPARE(SVideoFormat::sampleSize(SVideoFormat::Format_YUYV422), 2);
  QCOMPARE(SVideoFormat::numPlanes(SVideoFormat::Format_RGB32), 1);
  QCOMPARE(SVideoFormat::numPlanes(SVideoFormat::Format_YUYV422), 1);
  QCOMPARE(SVideoFormat::numPlanes(SVideoFormat::Format_YUV420P), 3);
  QCOMPARE(SVideoFormat::nullPixelValue(SVideoFormat::Format_RGB32), quint32(0xFF000000));
}

/*! Tests the SVideoCodec class
 */
void StreamTest::VideoCodec(void)
{
  SVideoCodec codec = "MPEG1";
  QVERIFY(codec == SVideoCodec("MPEG1"));
  QVERIFY(codec != SVideoCodec("THEORA"));
  QVERIFY(codec == "MPEG1");
  QVERIFY(codec != "THEORA");
  QCOMPARE(codec.codec(), QString("MPEG1"));
  QCOMPARE(codec.isNull(), false);
  QCOMPARE(codec.size().width(), 0);
  QCOMPARE(codec.size().height(), 0);
  QVERIFY(qFuzzyCompare(codec.size().aspectRatio(), 1.0f));
  QVERIFY(!codec.frameRate().isValid());

  codec.setCodec("THEORA", SSize(384, 288, 0.7f), SInterval::fromFrequency(25));
  QVERIFY(codec == SVideoCodec("THEORA", SSize(384, 288, 0.7f), SInterval::fromFrequency(25)));
  QVERIFY(codec != SVideoCodec("THEORA"));
  QVERIFY(codec == "THEORA");
  QVERIFY(codec != "MPEG1");
  QCOMPARE(codec.codec(), QString("THEORA"));
  QCOMPARE(codec.isNull(), false);
  QCOMPARE(codec.size().width(), 384);
  QCOMPARE(codec.size().height(), 288);
  QVERIFY(qFuzzyCompare(codec.size().aspectRatio(), 0.7f));
  QVERIFY(qFuzzyCompare(codec.frameRate().toFrequency(), 25.0));
}

/*! Tests the time computations are ok
 */
void StreamTest::Time(void)
{
  QVERIFY(!STime().isValid());
  QVERIFY(!STime().isNull());
  QVERIFY(STime::fromHour(4).isPositive());
  QVERIFY(!STime::fromHour(0).isPositive());
  QVERIFY(!STime::fromHour(-4).isPositive());
  QVERIFY(!STime::fromHour(4).isNegative());
  QVERIFY(!STime::fromHour(0).isNegative());
  QVERIFY(STime::fromHour(-4).isNegative());

  QCOMPARE((STime::fromHour(2) + STime::fromHour(2)).toHour(), 4);
  QCOMPARE((STime::fromHour(4) - STime::fromHour(1)).toHour(), 3);

  QVERIFY(STime::fromHour(4) == STime::fromMin(240));
  QVERIFY(STime::fromHour(4) != STime::fromMin(239));
  QVERIFY(STime::fromHour(4) >  STime::fromMin(239));
  QVERIFY(STime::fromHour(4) >= STime::fromMin(240));
  QVERIFY(STime::fromHour(4) <  STime::fromMin(241));
  QVERIFY(STime::fromHour(4) <= STime::fromMin(240));
  QVERIFY(STime::fromClock(3, 2, 3) == STime::fromMSec(2000l));
  QVERIFY(STime::fromClock(4, 2, 3) > STime::fromMSec(2000l));
  QVERIFY(STime::fromClock(2, 2, 3) < STime::fromMSec(2000l));

  QCOMPARE(STime::fromHour(4).toHour(), 4);
  QCOMPARE(STime::fromMin(240).toHour(), 4);
  QCOMPARE(STime::fromSec(14400).toHour(), 4);
  QCOMPARE(STime::fromMSec(14400000l).toHour(), 4);
  QCOMPARE(STime::fromUSec(Q_INT64_C(14400000000)).toHour(), 4);

  QCOMPARE(STime::fromMin(240).toMin(), 240);
  QCOMPARE(STime::fromSec(14400).toSec(), 14400);
  QCOMPARE(STime::fromMSec(14400000l).toMSec(), Q_INT64_C(14400000));
  QCOMPARE(STime::fromUSec(Q_INT64_C(14400000000)).toUSec(), Q_INT64_C(14400000000));

  QCOMPARE(STime::fromClock(Q_INT64_C(324000000), 90000).toHour(), 1);
  QCOMPARE(STime::fromHour(1).toClock(90000), Q_INT64_C(324000000));
  QCOMPARE(STime::fromClock(Q_INT64_C(1296000000), 90000).toHour(), 4);
  QCOMPARE(STime::fromHour(4).toClock(90000), Q_INT64_C(1296000000));
  QCOMPARE(STime::fromClock(Q_INT64_C(1296000000), 90000.0).toHour(), 4);
  QCOMPARE(STime::fromHour(4).toClock(90000.0), Q_INT64_C(1296000000));
  QCOMPARE(STime::fromClock(Q_INT64_C(90000), 1, 25).toHour(), 1);
  QCOMPARE(STime::fromHour(1).toClock(1, 25), Q_INT64_C(90000));
  QCOMPARE(STime::fromClock(3, 2, 3).toMSec(), Q_INT64_C(2000));
  QCOMPARE(STime::fromMSec(2000l).toClock(2, 3), Q_INT64_C(3));

  QCOMPARE((STime::fromSec(4) + STime::fromMSec(500l)).toMSec(), Q_INT64_C(4500));
  QCOMPARE((STime::fromClock(3, 2, 3) + STime::fromMSec(500l)).toMSec(), Q_INT64_C(2500));

  QCOMPARE(STime(1, SInterval::fromFrequency(25)).toMSec(), Q_INT64_C(40));
  QCOMPARE(STime(1, SInterval::fromFrequency(25.0)).toMSec(), Q_INT64_C(40));
  QCOMPARE(STime(1, SInterval(1, 25)).toMSec(), Q_INT64_C(40));

  QCOMPARE(qAbs(STime::fromHour(4)).toMin(), 240);
  QCOMPARE(qAbs(STime::fromHour(-4)).toMin(), 240);
}
