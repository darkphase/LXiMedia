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

#include "coretest.h"
#include <QtGui>
#include <QtTest>
#include <LXiStream>

void CoreTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  QVERIFY(SSystem::initialize(SSystem::Initialize_Devices |
                              SSystem::Initialize_LogToConsole, 0));
}

void CoreTest::cleanupTestCase(void)
{
  SSystem::shutdown();
}

void CoreTest::SDebug_Log(void)
{
  qDebug() << "Test debug message";
#ifdef QT_NO_DEBUG
  qWarning() << "Test warning message";
  qCritical() << "Test critical message";
#endif
}

void CoreTest::SDebug_MutexLocker(void)
{
  QMutex mutex;

  QVERIFY(mutex.tryLock(0));
  mutex.unlock();

  SDebug::MutexLocker m(&mutex, __FILE__, __LINE__);

  QVERIFY(!mutex.tryLock(0));

  m.unlock();

  QVERIFY(mutex.tryLock(0));
  mutex.unlock();

  SDebug::MutexLocker n(&mutex, __FILE__, __LINE__);

  QVERIFY(!mutex.tryLock(0));
}

/*! Validates that the SStringParser::toCleanName method functions correctly.
 */
void CoreTest::StringParser_CleanName(void)
{
  QCOMPARE(SStringParser::toCleanName("**Clean-*!name?"), QString("Clean name"));
}

/*! Validates that the SStringParser::toRawName method functions correctly.
 */
void CoreTest::StringParser_RawName(void)
{
  QCOMPARE(SStringParser::toRawName("R&~a%%%W-**n   A#$m@@@__+@e?!!1/{  }2"), QString("RAWNAME12"));
}

/*! Validates that the SStringParser::findMatch method functions correctly.
 */
void CoreTest::StringParser_FindMatch(void)
{
  QCOMPARE(SStringParser::findMatch("long sentence", "weird sentinel"), QString(" sent"));
  QCOMPARE(SStringParser::findMatch("something new", "something old"), QString("something "));
  QCOMPARE(SStringParser::findMatch("the weirdest text", "the most cleanest text"), QString("est text"));

  QCOMPARE(SStringParser::findMatch("brown", "fax").length(), 0);
  QCOMPARE(SStringParser::findMatch("green", "bear").length(), 1);
}

/*! Validates that the SStringParser::computeMatch method functions correctly.
 */
void CoreTest::StringParser_ComputeMatch(void)
{
  QVERIFY(qFuzzyCompare(SStringParser::computeMatch("some text", "some text"), 1.0));
  QVERIFY(qFuzzyCompare(SStringParser::computeMatch("some text with additional useless information", "some text"), 1.0));
  QVERIFY(SStringParser::computeMatch("some text with additional useless information", "some text with useful information") > 0.1f);
  QVERIFY(SStringParser::computeMatch("some text with additional useless information", "some text") >
          SStringParser::computeMatch("some text with additional useless information", "some text with useful information"));
}

/*! Tests the buffer data.
 */
void CoreTest::BufferData(void)
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
void CoreTest::BufferClone(void)
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
void CoreTest::BufferEnlarge(void)
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
void CoreTest::BufferExternal(void)
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
                                   SAudioFormat::Channel_Stereo,
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
void CoreTest::fillBuffer(char *data, int size)
{
  for (int i=0; i<size; i++)
    data[i] = char(i * 3);
}

/*! Tests the SAudioFormat class
 */
void CoreTest::AudioFormat(void)
{
  SAudioFormat format = SAudioFormat::Format_PCM_S16;
  QVERIFY(format == SAudioFormat(SAudioFormat::Format_PCM_S16));
  QVERIFY(format != SAudioFormat(SAudioFormat::Format_PCM_F32));
  QVERIFY(format == SAudioFormat::Format_PCM_S16);
  QVERIFY(format != SAudioFormat::Format_PCM_F32);
  QCOMPARE(format.format(), SAudioFormat::Format_PCM_S16);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.numChannels(), 0);
  QCOMPARE(format.sampleRate(), 0);

  format.setFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Surround_5_1, 48000);
  QVERIFY(format == SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channel_Surround_5_1, 48000));
  QVERIFY(format != SAudioFormat(SAudioFormat::Format_PCM_S16));
  QVERIFY(format == SAudioFormat::Format_PCM_S16);
  QVERIFY(format != SAudioFormat::Format_PCM_F32);
  QCOMPARE(format.isNull(), false);
  QCOMPARE(format.numChannels(), 6);
  QCOMPARE(format.sampleRate(), 48000);

  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Mono), 1);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Stereo), 2);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Quadraphonic), 4);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Surround_3_0), 3);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Surround_4_0), 4);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Surround_5_0), 5);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Surround_5_1), 6);
  QCOMPARE(SAudioFormat::numChannels(SAudioFormat::Channel_Surround_7_1), 8);
  QCOMPARE(SAudioFormat::guessChannels(1), SAudioFormat::Channel_Mono);
  QCOMPARE(SAudioFormat::guessChannels(2), SAudioFormat::Channel_Stereo);
  QCOMPARE(SAudioFormat::guessChannels(4), SAudioFormat::Channel_Quadraphonic);
  QCOMPARE(SAudioFormat::guessChannels(6), SAudioFormat::Channel_Surround_5_1);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_S16), 2);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_S8), 1);
  QCOMPARE(SAudioFormat::sampleSize(SAudioFormat::Format_PCM_F32), 4);
}

/*! Tests the SAudioCodec class
 */
void CoreTest::AudioCodec(void)
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

  codec.setCodec("VORBIS", SAudioFormat::Channel_Surround_5_1, 48000);
  QVERIFY(codec == SAudioCodec("VORBIS", SAudioFormat::Channel_Surround_5_1, 48000));
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
void CoreTest::VideoFormat(void)
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
  QCOMPARE(SVideoFormat::nullPixelValue(SVideoFormat::Format_RGB32), quint32(0));
}

/*! Tests the SVideoCodec class
 */
void CoreTest::VideoCodec(void)
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
void CoreTest::Time(void)
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
