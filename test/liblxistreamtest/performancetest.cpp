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

#include "performancetest.h"
#include "lxistream/algorithms/audioconvert.h"
#include "lxistream/algorithms/data.h"
#include "lxistream/algorithms/deinterlace.h"
#include "lxistream/algorithms/videoconvert.h"
#include <QtTest>

using namespace LXiStream;

void PerformanceTest::initTestCase(void)
{
}

void PerformanceTest::cleanupTestCase(void)
{
}

/*! Tests byte swapping.
 */
void PerformanceTest::DataSwap(void)
{
  static const int count = 16;
  static const quint8 _lxi_align src[count] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  quint8 _lxi_align dst[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::Data::swapBytes(reinterpret_cast<quint16 *>(dst), reinterpret_cast<const quint16 *>(src), count / sizeof(quint16));

  QCOMPARE(dst[0],  quint8(2));  QCOMPARE(dst[1],  quint8(1));
  QCOMPARE(dst[2],  quint8(4));  QCOMPARE(dst[3],  quint8(3));
  QCOMPARE(dst[4],  quint8(6));  QCOMPARE(dst[5],  quint8(5));
  QCOMPARE(dst[6],  quint8(8));  QCOMPARE(dst[7],  quint8(7));
  QCOMPARE(dst[8],  quint8(10)); QCOMPARE(dst[9],  quint8(9));
  QCOMPARE(dst[10], quint8(12)); QCOMPARE(dst[11], quint8(11));
  QCOMPARE(dst[12], quint8(14)); QCOMPARE(dst[13], quint8(13));
  QCOMPARE(dst[14], quint8(16)); QCOMPARE(dst[15], quint8(15));

  Algorithms::Data::swapBytes(reinterpret_cast<qint16 *>(dst), reinterpret_cast<const qint16 *>(src), count / sizeof(qint16));

  QCOMPARE(dst[0],  quint8(2));  QCOMPARE(dst[1],  quint8(1));
  QCOMPARE(dst[2],  quint8(4));  QCOMPARE(dst[3],  quint8(3));
  QCOMPARE(dst[4],  quint8(6));  QCOMPARE(dst[5],  quint8(5));
  QCOMPARE(dst[6],  quint8(8));  QCOMPARE(dst[7],  quint8(7));
  QCOMPARE(dst[8],  quint8(10)); QCOMPARE(dst[9],  quint8(9));
  QCOMPARE(dst[10], quint8(12)); QCOMPARE(dst[11], quint8(11));
  QCOMPARE(dst[12], quint8(14)); QCOMPARE(dst[13], quint8(13));
  QCOMPARE(dst[14], quint8(16)); QCOMPARE(dst[15], quint8(15));

  Algorithms::Data::swapBytes(reinterpret_cast<quint32 *>(dst), reinterpret_cast<const quint32 *>(src), count / sizeof(quint32));

  QCOMPARE(dst[0],  quint8(4));  QCOMPARE(dst[1],  quint8(3));  QCOMPARE(dst[2],  quint8(2));  QCOMPARE(dst[3],  quint8(1));
  QCOMPARE(dst[4],  quint8(8));  QCOMPARE(dst[5],  quint8(7));  QCOMPARE(dst[6],  quint8(6));  QCOMPARE(dst[7],  quint8(5));
  QCOMPARE(dst[8],  quint8(12)); QCOMPARE(dst[9],  quint8(11)); QCOMPARE(dst[10], quint8(10)); QCOMPARE(dst[11], quint8(9));
  QCOMPARE(dst[12], quint8(16)); QCOMPARE(dst[13], quint8(15)); QCOMPARE(dst[14], quint8(14)); QCOMPARE(dst[15], quint8(13));

  Algorithms::Data::swapBytes(reinterpret_cast<qint32 *>(dst), reinterpret_cast<const qint32 *>(src), count / sizeof(qint32));

  QCOMPARE(dst[0],  quint8(4));  QCOMPARE(dst[1],  quint8(3));  QCOMPARE(dst[2],  quint8(2));  QCOMPARE(dst[3],  quint8(1));
  QCOMPARE(dst[4],  quint8(8));  QCOMPARE(dst[5],  quint8(7));  QCOMPARE(dst[6],  quint8(6));  QCOMPARE(dst[7],  quint8(5));
  QCOMPARE(dst[8],  quint8(12)); QCOMPARE(dst[9],  quint8(11)); QCOMPARE(dst[10], quint8(10)); QCOMPARE(dst[11], quint8(9));
  QCOMPARE(dst[12], quint8(16)); QCOMPARE(dst[13], quint8(15)); QCOMPARE(dst[14], quint8(14)); QCOMPARE(dst[15], quint8(13));

  Algorithms::Data::swapBytes(reinterpret_cast<uint64_t *>(dst), reinterpret_cast<const uint64_t *>(src), count / sizeof(uint64_t));

  QCOMPARE(dst[0],  quint8(8));  QCOMPARE(dst[1],  quint8(7));  QCOMPARE(dst[2],  quint8(6));  QCOMPARE(dst[3],  quint8(5));  QCOMPARE(dst[4],  quint8(4));  QCOMPARE(dst[5],  quint8(3));  QCOMPARE(dst[6],  quint8(2));  QCOMPARE(dst[7],  quint8(1));
  QCOMPARE(dst[8],  quint8(16)); QCOMPARE(dst[9],  quint8(15)); QCOMPARE(dst[10], quint8(14)); QCOMPARE(dst[11], quint8(13)); QCOMPARE(dst[12], quint8(12)); QCOMPARE(dst[13], quint8(11)); QCOMPARE(dst[14], quint8(10)); QCOMPARE(dst[15], quint8(9));

  Algorithms::Data::swapBytes(reinterpret_cast<int64_t *>(dst), reinterpret_cast<const int64_t *>(src), count / sizeof(int64_t));

  QCOMPARE(dst[0],  quint8(8));  QCOMPARE(dst[1],  quint8(7));  QCOMPARE(dst[2],  quint8(6));  QCOMPARE(dst[3],  quint8(5));  QCOMPARE(dst[4],  quint8(4));  QCOMPARE(dst[5],  quint8(3));  QCOMPARE(dst[6],  quint8(2));  QCOMPARE(dst[7],  quint8(1));
  QCOMPARE(dst[8],  quint8(16)); QCOMPARE(dst[9],  quint8(15)); QCOMPARE(dst[10], quint8(14)); QCOMPARE(dst[11], quint8(13)); QCOMPARE(dst[12], quint8(12)); QCOMPARE(dst[13], quint8(11)); QCOMPARE(dst[14], quint8(10)); QCOMPARE(dst[15], quint8(9));
}

// Implemented in audioresampler.resample.c
extern "C" unsigned LXiStream_Common_AudioResampler_resampleAudio(
    const qint16 * srcData, unsigned srcSampleRate, unsigned numSamples, unsigned srcNumChannels,
    qint16 * dstData, unsigned dstSampleRate, unsigned maxSamples,
    unsigned *pNextPos, float *pWeightOffset);

void PerformanceTest::AudioResample(void)
{
  const qint16 _lxi_align src[20] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
  qint16 _lxi_align dst[40] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  AudioResampleUp(dst, src, sizeof(dst));

//  for (size_t i=0; i+4<(sizeof(src)/sizeof(src[0])); i++)
//  {
//    qDebug() << dst[i * 2    ] << src[i];
//    qDebug() << dst[i * 2 + 1] << ((int(src[i]) + int(src[i + 1])) / 2);
//  }

  for (size_t i=0; i+4<(sizeof(src)/sizeof(src[0])); i++)
  {
    QCOMPARE(dst[i * 2], src[i]);
    QCOMPARE(int(dst[i * 2 + 1]), (int(src[i]) + int(src[i + 1])) / 2);
  }

  AudioResampleDown(dst, src, sizeof(dst) / 4);

//  for (size_t i=0; i+4<(sizeof(src)/sizeof(src[0])); i++)
//    qDebug() << dst[i / 2] << src[i];

  for (size_t i=0; i+4<(sizeof(src)/sizeof(src[0])); i+=2)
    QCOMPARE(dst[i / 2], src[i]);
}

void * PerformanceTest::AudioResampleUp(void *dst, const void *src, size_t len)
{
  const unsigned numSamples = len / sizeof(qint16);
  unsigned nextPos = 0;
  float weightOffset = 0.0f;

  LXiStream_Common_AudioResampler_resampleAudio(
      reinterpret_cast<const qint16 *>(src), 22050, numSamples / 2, 1,
      reinterpret_cast<qint16 *>(dst),       44100, numSamples,
      &nextPos, &weightOffset);

  return dst;
}

void * PerformanceTest::AudioResampleDown(void *dst, const void *src, size_t len)
{
  const unsigned numSamples = len / sizeof(qint16);
  unsigned nextPos = 0;
  float weightOffset = 0.0f;

  LXiStream_Common_AudioResampler_resampleAudio(
      reinterpret_cast<const qint16 *>(src), 44100, numSamples * 2, 1,
      reinterpret_cast<qint16 *>(dst),       22050, numSamples,
      &nextPos, &weightOffset);

  return dst;
}

//// Implemented in audiomixer.mix.c
//extern "C" void LXiStream_AudioMixer_mixAudio(const qint16 *, unsigned, unsigned, float, qint16 *, unsigned);
//
//void PerformanceTest::AudioMixer(void)
//{
//  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);
//
//  static const unsigned srcNumChannels = 2;
//  static const unsigned dstNumChannels = 6;
//  static const unsigned srcNumSamples = 100 * 1024;
//  static const unsigned dstNumSamples = srcNumSamples * dstNumChannels / srcNumChannels;
//  static const unsigned numLoops = 50;
//
//  qint16 * const srcBuffer = new qint16[srcNumSamples * srcNumChannels];
//  qint16 * const dstBuffer = new qint16[dstNumSamples * dstNumChannels];
//
//  QTime timer;
//  timer.start();
//
//  for (unsigned i=0; i<numLoops; i++)
//  {
//    LXiStream_AudioMixer_mixAudio(srcBuffer, srcNumChannels, srcNumSamples, 1.0f, dstBuffer, dstNumChannels);
//    LXiStream_AudioMixer_mixAudio(srcBuffer, srcNumChannels, srcNumSamples, 0.5f, dstBuffer, dstNumChannels);
//  }
//
//  const int elapsed = timer.elapsed();
//  qDebug() << "LXiStream_AudioMixer_mixAudio = " << (srcNumSamples * srcNumChannels * numLoops / 1000) / elapsed << " Msamples/s";
//
//  delete [] srcBuffer;
//  delete [] dstBuffer;
//}

/*! Tests S8/S16 conversion.
 */
void PerformanceTest::ConvertS8S16(void)
{
  static const int count = 16;
  static const qint8 _lxi_align src[count] = { 1, -2, 3, -4, 5, -6, 7, -8, 9, -10, 11, -12, 13, -14, 15, -16 };
  qint16 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], qint16((qint16(src[i]) << 8)));

  qint8 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests U8/S16 conversion.
 */
void PerformanceTest::ConvertU8S16(void)
{
  static const int count = 16;
  static const quint8 _lxi_align src[count] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  qint16 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], qint16((qint16(src[i]) << 8) - 32768));

  quint8 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests U8/S16 conversion.
 */
void PerformanceTest::ConvertU8U16(void)
{
  static const int count = 16;
  static const quint8 _lxi_align src[count] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  quint16 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], quint16(quint16(src[i]) << 8));

  quint8 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests S16/S32 conversion.
 */
void PerformanceTest::ConvertS16S32(void)
{
  static const int count = 8;
  static const qint16 _lxi_align src[count] = { 1, -2, 3, -4, 5, -6, 7, -8 };
  qint32 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], qint32(qint32(src[i]) << 16));

  qint16 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests U16/U32 conversion.
 */
void PerformanceTest::ConvertU16U32(void)
{
  static const int count = 8;
  static const quint16 _lxi_align src[count] = { 1, -2, 3, -4, 5, -6, 7, -8 };
  quint32 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], quint32(quint32(src[i]) << 16));

  quint16 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests S16/F32 conversion.
 */
void PerformanceTest::ConvertS16F32(void)
{
  static const int count = 8;
  static const qint16 _lxi_align src[count] = { 1, -2, 3, -4, 5, -6, 7, -8 };
  float _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QVERIFY(qFuzzyCompare(dst1[i], float(src[i]) / 32768.0f));

  qint16 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QVERIFY(qAbs(int(dst2[i]) - int(src[i]) <= 1));
}

/*! Tests S32/F32 conversion.
 */
void PerformanceTest::ConvertS32F32(void)
{
  static const int count = 4;
  static const qint32 _lxi_align src[count] = { 1, -2, 3, -4 };
  float _lxi_align dst1[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QVERIFY(qFuzzyCompare(dst1[i], float(src[i]) / 2147483648.0f));

  qint32 _lxi_align dst2[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QVERIFY(qAbs(int(dst2[i]) - int(src[i]) <= 1));
}

/*! Tests S16/F64 conversion.
 */
void PerformanceTest::ConvertS16F64(void)
{
  static const int count = 8;
  static const qint16 _lxi_align src[count] = { 1, -2, 3, -4, 5, -6, 7, -8 };
  double _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QVERIFY(qFuzzyCompare(dst1[i], double(src[i]) / 32768.0));

  qint16 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QVERIFY(qAbs(int(dst2[i]) - int(src[i]) <= 1));
}

/*! Tests S32/F64 conversion.
 */
void PerformanceTest::ConvertS32F64(void)
{
  static const int count = 4;
  static const qint32 _lxi_align src[count] = { 1, -2, 3, -4 };
  double _lxi_align dst1[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QVERIFY(qFuzzyCompare(dst1[i], double(src[i]) / 2147483648.0));

  qint32 _lxi_align dst2[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QVERIFY(qAbs(int(dst2[i]) - int(src[i]) <= 1));
}

/*! Tests U16/S16 conversion.
 */
void PerformanceTest::ConvertU16S16(void)
{
  static const int count = 8;
  static const quint16 _lxi_align src[count] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  qint16 _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], qint16(src[i] + 32768));

  quint16 _lxi_align dst2[count] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests U32/S32 conversion.
 */
void PerformanceTest::ConvertU32S32(void)
{
  static const int count = 4;
  static const quint32 _lxi_align src[count] = { 1, 2, 3, 4 };
  qint32 _lxi_align dst1[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst1, src, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst1[i], qint32(src[i] + 2147483648u));

  quint32 _lxi_align dst2[count] = { 0, 0, 0, 0 };

  Algorithms::AudioConvert::convert(dst2, dst1, count);

  for (int i=0; i<count; i++)
    QCOMPARE(dst2[i], src[i]);
}

/*! Tests deinterlacing.
 */
void PerformanceTest::DeinterlaceBlend(void)
{
  static const int count = 10;
  static const quint8 _lxi_align srca[count] = { 64, 96, 64, 96, 64, 96, 64, 96, 64, 96 };
  static const quint8 _lxi_align srcb[count] = { 96, 64, 96, 64, 96, 64, 96, 64, 96, 64 };

  quint8 _lxi_align dst[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::Deinterlace::blendFields(dst, srca, srcb, count);

  for (int i=0; i<count; i++)
    QCOMPARE(int(dst[i]), 80);
}

/*! Tests deinterlacing.
 */
void PerformanceTest::DeinterlaceSmartBlend(void)
{
  static const int count = 10;
  static const quint8 _lxi_align srca[count] = { 64, 144, 64, 144, 64, 144, 64, 144, 64, 144 };
  static const quint8 _lxi_align srcb[count] = { 96, 192, 96, 192, 96, 192, 96, 192, 96, 192 };
  static const quint8 _lxi_align srcc[count] = { 192, 48, 192, 48, 192, 48, 192, 48, 192, 48 };

  quint8 _lxi_align dst[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::Deinterlace::smartBlendFields(dst, srca, srcb, srcc, count);

  for (int i=0; i<count; i++)
    QCOMPARE(int(dst[i]), 96);
}

/*! Tests YUYV/YUV2 conversion.
 */
void PerformanceTest::ConvertYUYVtoYUV2(void)
{
  static const int count = 40;
  static const quint8 _lxi_align src[count * 2] =
  {
//   Y   U    Y    V   Y    U    Y   V   Y   U    Y    V   Y    U    Y   V   Y    U    Y   V
    96, 64, 160, 192, 96, 192, 160, 64, 96, 64, 160, 192, 96, 192, 160, 64, 96, 192, 160, 64,
    96, 64, 160, 192, 96, 192, 160, 64, 96, 64, 160, 192, 96, 192, 160, 64, 96, 192, 160, 64,
    96, 64, 160, 192, 96, 192, 160, 64, 96, 64, 160, 192, 96, 192, 160, 64, 96, 192, 160, 64,
    96, 64, 160, 192, 96, 192, 160, 64, 96, 64, 160, 192, 96, 192, 160, 64, 96, 192, 160, 64
  };

  quint8 _lxi_align dsty[count    ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dstu[count / 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dstv[count / 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::YUYVtoYUV2(dsty, dstu, dstv, src, count);

//  for (size_t i=0; i<count*2; i+=4)
//  {
//    qDebug() << src[i]     << dsty[i / 2];
//    qDebug() << src[i + 1] << dstu[i / 4];
//    qDebug() << src[i + 2] << dsty[i / 2 + 1];
//    qDebug() << src[i + 3] << dstv[i / 4];
//  }

  for (size_t i=0; i<count*2; i+=4)
  {
    QCOMPARE(dsty[i / 2],     src[i]);
    QCOMPARE(dstu[i / 4],     src[i + 1]);
    QCOMPARE(dsty[i / 2 + 1], src[i + 2]);
    QCOMPARE(dstv[i / 4],     src[i + 3]);
  }
}

/*! Tests UYVY/YUV2 conversion.
 */
void PerformanceTest::ConvertUYVYtoYUV2(void)
{
  static const int count = 40;
  static const quint8 _lxi_align src[count * 2] =
  {
//   U   Y    V    Y    U   Y   V    Y   U   Y    V    Y    U   Y   V    Y   U   Y    V    Y
    64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160,
    64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160,
    64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160,
    64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160,
  };

  quint8 _lxi_align dsty[count    ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dstu[count / 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dstv[count / 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::UYVYtoYUV2(dsty, dstu, dstv, src, count);

//  for (size_t i=0; i<count*2; i+=4)
//  {
//    qDebug() << src[i]     << dstu[i / 4];
//    qDebug() << src[i + 1] << dsty[i / 2];
//    qDebug() << src[i + 2] << dstv[i / 4];
//    qDebug() << src[i + 3] << dsty[i / 2 + 1];
//  }

  for (size_t i=0; i<count*2; i+=4)
  {
    QCOMPARE(dstu[i / 4],     src[i]);
    QCOMPARE(dsty[i / 2],     src[i + 1]);
    QCOMPARE(dstv[i / 4],     src[i + 2]);
    QCOMPARE(dsty[i / 2 + 1], src[i + 3]);
  }
}

/*! Tests merging two UV lines.
 */
void PerformanceTest::MergeUVlines(void)
{
  static const int count = 20;
  static const quint8 _lxi_align srcua[count] = { 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96 };
  static const quint8 _lxi_align srcub[count] = { 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160 };
  static const quint8 _lxi_align srcva[count] = { 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64, 96, 64 };
  static const quint8 _lxi_align srcvb[count] = { 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192, 160, 192 };

  quint8 _lxi_align dstu[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dstv[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::mergeUVlines(dstu, dstv, srcua, srcub, srcva, srcvb, count);

//  for (int i=0; i<count; i++)
//  {
//    qDebug() << srcua[i] << srcub[i] << dstu[i];
//    qDebug() << srcva[i] << srcvb[i] << dstv[i];
//  }

  for (int i=0; i<count; i++)
  {
    QCOMPARE(int(dstu[i]), (int(srcua[i]) + int(srcub[i])) / 2);
    QCOMPARE(int(dstv[i]), (int(srcva[i]) + int(srcvb[i])) / 2);
  }
}

/*! Tests YUYV/RGB conversion.
 */
void PerformanceTest::ConvertYUYVtoRGB(void)
{
  static const int count = 10;
  static const quint8 _lxi_align src[count * 2] =
  {
//   Y   U    Y    V   Y    U    Y   V   Y   U    Y    V   Y    U    Y   V   Y    U    Y   V
    96, 64, 160, 192, 96, 192, 160, 64, 96, 64, 160, 192, 96, 192, 160, 64, 96, 192, 160, 64
  };

  uint32_t _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::YUYVtoRGB(dst1, src, count);

  quint8 _lxi_align dst2[count * 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::RGBtoYUYV(dst2, dst1, count);

//  for (int i=0; i<count*2; i+=4)
//  {
//    const uint8_t * const rgb = reinterpret_cast<const uint8_t *>(dst1 + (i / 2));
//    qDebug() << src[i+0] << rgb[0] << rgb[4] << dst2[i+0];
//    qDebug() << src[i+1] << rgb[1] << rgb[5] << dst2[i+1];
//    qDebug() << src[i+2] << rgb[2] << rgb[6] << dst2[i+2];
//    qDebug() << src[i+3] << rgb[3] << rgb[7] << dst2[i+3];
//  }

  for (int i=0; i<count*2; i++)
    QVERIFY(qAbs(int(src[i]) - int(dst2[i])) < 16);
}

/*! Tests UYVY/RGB conversion.
 */
void PerformanceTest::ConvertUYVYtoRGB(void)
{
  static const int count = 10;
  static const quint8 _lxi_align src[count * 2] =
  {
//   U   Y    V    Y    U   Y   V    Y   U   Y    V    Y    U   Y   V    Y    U   Y   V    Y
    64, 96, 192, 160, 192, 96, 64, 160, 64, 96, 192, 160, 192, 96, 64, 160, 192, 96, 64, 160
  };

  uint32_t _lxi_align dst1[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::UYVYtoRGB(dst1, src, count);

  quint8 _lxi_align dst2[count * 2] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::RGBtoUYVY(dst2, dst1, count);

//  for (int i=0; i<count*2; i+=4)
//  {
//    const uint8_t * const rgb = reinterpret_cast<const uint8_t *>(dst1 + (i / 2));
//    qDebug() << src[i+0] << rgb[0] << rgb[4] << dst2[i+0];
//    qDebug() << src[i+1] << rgb[1] << rgb[5] << dst2[i+1];
//    qDebug() << src[i+2] << rgb[2] << rgb[6] << dst2[i+2];
//    qDebug() << src[i+3] << rgb[3] << rgb[7] << dst2[i+3];
//  }

  for (int i=0; i<count*2; i++)
    QVERIFY(qAbs(int(src[i]) - int(dst2[i])) < 16);
}

/*! Tests BGR/RGB conversion.
 */
void PerformanceTest::ConvertBGRtoRGB(void)
{
  static const int count = 5;
  //                                                R  G  B  A  R  G  B  A  R  G  B  A  R  G  B  A  R  G  B  A
  static const quint8 _lxi_align src[count * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4 };
  quint8 _lxi_align dst1[count * 4] =             { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::BGRtoRGB(
      reinterpret_cast<quint32 *>(dst1),
      reinterpret_cast<const quint32 *>(src),
      count);

  quint8 _lxi_align dst2[count * 4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::BGRtoRGB(
      reinterpret_cast<quint32 *>(dst2),
      reinterpret_cast<quint32 *>(dst1),
      count);

//  for (int i=0; i<count; i++)
//  {
//    qDebug() << dst1[i * 4    ] << src[i * 4 + 2];
//    qDebug() << dst1[i * 4 + 1] << src[i * 4 + 1];
//    qDebug() << dst1[i * 4 + 2] << src[i * 4    ];
//    qDebug() << dst1[i * 4 + 3] << src[i * 4 + 3];
//  }

  for (int i=0; i<count; i++)
  {
    QCOMPARE(dst1[i * 4    ], src[i * 4 + 2]);
    QCOMPARE(dst1[i * 4 + 1], src[i * 4 + 1]);
    QCOMPARE(dst1[i * 4 + 2], src[i * 4    ]);
    QCOMPARE(dst1[i * 4 + 3], src[i * 4 + 3]);
  }
}

/*! Tests YUV1/RGB conversion.
 */
void PerformanceTest::ConvertYUV1toRGB(void)
{
  static const int count = 5;
  static const quint8 _lxi_align srcy[count] = { 96, 160, 96, 160, 96 };
  static const quint8 _lxi_align srcu[count] = { 64, 192, 64, 192, 64 };
  static const quint8 _lxi_align srcv[count] = { 192, 64, 192, 64, 192 };

  quint8 _lxi_align dst1[count * 4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::YUV1toRGB(reinterpret_cast<quint32 *>(dst1), srcy, srcu, srcv, count);

  quint8 _lxi_align dst2y[count] = { 0, 0, 0, 0, 0 };
  quint8 _lxi_align dst2u[count] = { 0, 0, 0, 0, 0 };
  quint8 _lxi_align dst2v[count] = { 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::RGBtoYUV1(dst2y, dst2u, dst2v, reinterpret_cast<const quint32 *>(dst1), count);

//  for (int i=0; i<count; i++)
//  {
//    qDebug() << srcy[i] << dst1[i*4+0] << dst2y[i];
//    qDebug() << srcu[i] << dst1[i*4+1] << dst2u[i];
//    qDebug() << srcv[i] << dst1[i*4+2] << dst2v[i];
//  }

  for (int i=0; i<count; i++)
  {
    QVERIFY(qAbs(int(srcy[i]) - int(dst2y[i])) < 16);
    QVERIFY(qAbs(int(srcu[i]) - int(dst2u[i])) < 16);
    QVERIFY(qAbs(int(srcv[i]) - int(dst2v[i])) < 16);
  }
}

/*! Tests YUV2/RGB conversion.
 */
void PerformanceTest::ConvertYUV2toRGB(void)
{
  static const int count = 10;
  static const quint8 _lxi_align srcy[count] = { 96, 160, 96, 160, 96, 160, 96, 160, 96, 160 };
  static const quint8 _lxi_align srcu[count / 2] = { 64, 192, 64, 192, 64 };
  static const quint8 _lxi_align srcv[count / 2] = { 192, 64, 192, 64, 192 };

  quint8 _lxi_align dst1[count * 4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::YUV2toRGB(reinterpret_cast<quint32 *>(dst1), srcy, srcu, srcv, count);

  quint8 _lxi_align dst2y[count] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  quint8 _lxi_align dst2u[count / 2] = { 0, 0, 0, 0, 0 };
  quint8 _lxi_align dst2v[count / 2] = { 0, 0, 0, 0, 0 };

  Algorithms::VideoConvert::RGBtoYUV2(dst2y, dst2u, dst2v, reinterpret_cast<const quint32 *>(dst1), count);

//  for (int i=0; i<count; i++)
//  {
//    qDebug() << srcy[i] << dst1[i*4+0] << dst2y[i];
//    qDebug() << srcu[i/2] << dst1[i*4+1] << dst2u[i/2];
//    qDebug() << srcv[i/2] << dst1[i*4+2] << dst2v[i/2];
//  }

  for (int i=0; i<count; i++)
  {
    QVERIFY(qAbs(int(srcy[i]) - int(dst2y[i])) < 16);
    QVERIFY(qAbs(int(srcu[i/2]) - int(dst2u[i/2])) < 16);
    QVERIFY(qAbs(int(srcv[i/2]) - int(dst2v[i/2])) < 16);
  }
}

template<typename _rettype, typename _dsttype, typename _srctype, typename _sizetype>
float PerformanceTest::measureSpeed(_rettype (* func)(_dsttype *, const _srctype *, _sizetype), const char *name)
{
  static const _sizetype count = 65536;
  _srctype _lxi_align src[count];
  _dsttype _lxi_align dst[count];

  for (_sizetype i=0; i<count; i++)
  {
    dst[i] = _dsttype(i);
    src[i] = _srctype(i);
  }

  QTime timer;
  timer.start();

  int i;
  for (i = 0; timer.elapsed() < 64; i++)
    func(dst, src, count);

  const float bytes = float(sizeof(src) + sizeof(dst)) * float(i);
  const float result = (bytes / float(timer.elapsed())) / 1073741.824f;

  qDebug() << name << QString::number(result, 'f', 2).toAscii().data() << "GiB/s";

  return result;
}

template<>
float PerformanceTest::measureSpeed(void * (* func)(void *, const void *, size_t), const char *name)
{
  return measureSpeed<void *, char, char, size_t>((void * (*)(char *, const char *, size_t))func, name);
}

template<typename _rettype, typename _dsttype, typename _srctype1, typename _srctype2, typename _sizetype>
float PerformanceTest::measureSpeed(_rettype (* func)(_dsttype *, const _srctype1 *, const _srctype2 *, _sizetype), const char *name)
{
  static const _sizetype count = 65536;
  _srctype1 _lxi_align src1[count];
  _srctype2 _lxi_align src2[count];
  _dsttype _lxi_align dst[count];

  for (_sizetype i=0; i<count; i++)
  {
    dst[i] = _dsttype(i);
    src1[i] = _srctype1(i);
    src2[i] = _srctype2(i);
  }

  QTime timer;
  timer.start();

  int i;
  for (i = 0; timer.elapsed() < 64; i++)
    func(dst, src1, src2, count);

  const float bytes = float(sizeof(src1) + sizeof(src2) + sizeof(dst)) * float(i);
  const float result = (bytes / float(timer.elapsed())) / 1073741.824f;

  qDebug() << name << QString::number(result, 'f', 2).toAscii().data() << "GiB/s";

  return result;
}

template<typename _rettype, typename _dsttype, typename _srctype1, typename _srctype2, typename _srctype3, typename _sizetype>
float PerformanceTest::measureSpeed(_rettype (* func)(_dsttype *, const _srctype1 *, const _srctype2 *, const _srctype3 *, _sizetype), const char *name)
{
  static const _sizetype count = 65536;
  _srctype1 _lxi_align src1[count];
  _srctype2 _lxi_align src2[count];
  _srctype3 _lxi_align src3[count];
  _dsttype _lxi_align dst[count];

  for (_sizetype i=0; i<count; i++)
  {
    dst[i] = _dsttype(i);
    src1[i] = _srctype1(i);
    src2[i] = _srctype2(i);
    src3[i] = _srctype3(i);
  }

  QTime timer;
  timer.start();

  int i;
  for (i = 0; timer.elapsed() < 64; i++)
    func(dst, src1, src2, src3, count);

  const float bytes = float(sizeof(src1) + sizeof(src2) + sizeof(src3) + sizeof(dst)) * float(i);
  const float result = (bytes / float(timer.elapsed())) / 1073741.824f;

  qDebug() << name << QString::number(result, 'f', 2).toAscii().data() << "GiB/s";

  return result;
}

template<typename _rettype, typename _dsttype1, typename _dsttype2, typename _dsttype3, typename _srctype, typename _sizetype>
float PerformanceTest::measureSpeed(_rettype (* func)(_dsttype1 *, _dsttype2 *, _dsttype3 *, const _srctype *, _sizetype), const char *name)
{
  static const _sizetype count = 65536;
  _srctype _lxi_align src[count];
  _dsttype1 _lxi_align dst1[count];
  _dsttype2 _lxi_align dst2[count];
  _dsttype3 _lxi_align dst3[count];

  for (_sizetype i=0; i<count; i++)
  {
    dst1[i] = _dsttype1(i);
    dst2[i] = _dsttype2(i);
    dst3[i] = _dsttype3(i);
    src[i] = _srctype(i);
  }

  QTime timer;
  timer.start();

  int i;
  for (i = 0; timer.elapsed() < 64; i++)
    func(dst1, dst2, dst3, src, count);

  const float bytes = float(sizeof(src) + sizeof(dst1) + sizeof(dst2) + sizeof(dst3)) * float(i);
  const float result = (bytes / float(timer.elapsed())) / 1073741.824f;

  qDebug() << name << QString::number(result, 'f', 2).toAscii().data() << "GiB/s";

  return result;
}

/*! Measures conversion performance.
 */
void PerformanceTest::Performance(void)
{
  if (qApp->arguments().contains("-silent"))
    QSKIP("Performance test skipped when running silent", SkipSingle);

#define measureSpeed(func) measureSpeed(func, #func)

  measureSpeed(memcpy);

  measureSpeed((void(*)(int16_t *, const int16_t *, int))Algorithms::Data::swapBytes);
  measureSpeed((void(*)(int32_t *, const int32_t *, int))Algorithms::Data::swapBytes);
  measureSpeed((void(*)(int64_t *, const int64_t *, int))Algorithms::Data::swapBytes);

  measureSpeed((void(*)(int16_t *, const int8_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int8_t *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int16_t *, const uint8_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint8_t *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint16_t *, const uint8_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint8_t *, const uint16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int32_t *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int16_t *, const int32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint32_t *, const uint16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint16_t *, const uint32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int16_t *, const uint16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint16_t *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int32_t *, const uint32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(uint32_t *, const int32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(float *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int16_t *, const float *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(float *, const int32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int32_t *, const float *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(double *, const int16_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int16_t *, const double *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(double *, const int32_t *, int))Algorithms::AudioConvert::convert);
  measureSpeed((void(*)(int32_t *, const double *, int))Algorithms::AudioConvert::convert);

//  measureSpeed(AudioResampleUp,                                         1, 2);
//  measureSpeed(AudioResampleDown,                                       2, 1);

  measureSpeed(Algorithms::Deinterlace::blendFields);
  measureSpeed(Algorithms::Deinterlace::smartBlendFields);

  measureSpeed((void(*)(uint16_t *, uint8_t *, uint8_t *, const uint16_t *, int))Algorithms::VideoConvert::YUYVtoYUV2);
  measureSpeed((void(*)(uint16_t *, uint8_t *, uint8_t *, const uint16_t *, int))Algorithms::VideoConvert::UYVYtoYUV2);
  measureSpeed((void(*)(uint32_t *, const uint16_t *, int))Algorithms::VideoConvert::YUYVtoRGB);
  measureSpeed((void(*)(uint16_t *, const uint32_t *, int))Algorithms::VideoConvert::RGBtoYUYV);
  measureSpeed((void(*)(uint32_t *, const uint16_t *, int))Algorithms::VideoConvert::UYVYtoRGB);
  measureSpeed((void(*)(uint16_t *, const uint32_t *, int))Algorithms::VideoConvert::RGBtoUYVY);
  measureSpeed(Algorithms::VideoConvert::YUV1toRGB);
  measureSpeed(Algorithms::VideoConvert::RGBtoYUV1);
  measureSpeed(Algorithms::VideoConvert::YUV2toRGB);
  measureSpeed(Algorithms::VideoConvert::RGBtoYUV2);
  measureSpeed(Algorithms::VideoConvert::BGRtoRGB);

#undef measureSpeed
}
