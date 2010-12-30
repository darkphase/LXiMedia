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

#include "test.h"

// Implemented in audioresampler.resample.c
extern "C" unsigned LXiStream_AudioResampler_resampleAudio(
    const qint16 * srcData, unsigned srcSampleRate,
    unsigned numSamples, unsigned srcNumChannels,
    qint16 * dstData, unsigned dstSampleRate,
    unsigned maxSamples, unsigned dstNumChannels,
    unsigned *pNextPos, float *pWeightOffset);

void LXiStreamTest::Performance_AudioResamplerResample(void)
{
  Performance_AudioResamplerResample(44100, 48000, "up");
  Performance_AudioResamplerResample(44100, 8000, "down");
}

void LXiStreamTest::Performance_AudioResamplerResample(unsigned srcSampleRate, unsigned dstSampleRate, const char *name)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned srcNumChannels = 2;
  static const unsigned dstNumChannels = 2;
  static const unsigned srcNumSamples = 8192;
  static const unsigned dstNumSamples = srcNumSamples * dstSampleRate / srcSampleRate;
  static const unsigned numLoops = 600;

  qint16 * const srcBuffer = new qint16[(srcNumSamples + 8) * srcNumChannels + 16];
  qint16 * const dstBuffer = new qint16[dstNumSamples * dstNumChannels + 16];

  QTime timer;
  timer.start();

  for (unsigned i=0; i<numLoops; i++)
  {
    unsigned nextPos = 0;
    float weightOffset = 0.0f;

    LXiStream_AudioResampler_resampleAudio(srcBuffer, srcSampleRate,
                                          srcNumSamples, srcNumChannels,
                                          dstBuffer, dstSampleRate,
                                          dstNumSamples, dstNumChannels,
                                          &nextPos, &weightOffset);
  }

  const int elapsed = timer.elapsed();
  qDebug() << "LXiStream_AudioResampler_resampleAudio(" << name << ") = " << (srcNumSamples * srcNumChannels * numLoops / 1000) / elapsed << " Msamples/s";

  delete [] srcBuffer;
  delete [] dstBuffer;
}

// Implemented in audiomixer.mix.c
extern "C" void LXiStream_AudioMixer_mixAudio(const qint16 *, unsigned, unsigned, float, qint16 *, unsigned);

void LXiStreamTest::Performance_AudioMixerMix(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned srcNumChannels = 2;
  static const unsigned dstNumChannels = 6;
  static const unsigned srcNumSamples = 100 * 1024;
  static const unsigned dstNumSamples = srcNumSamples * dstNumChannels / srcNumChannels;
  static const unsigned numLoops = 50;

  qint16 * const srcBuffer = new qint16[srcNumSamples * srcNumChannels];
  qint16 * const dstBuffer = new qint16[dstNumSamples * dstNumChannels];

  QTime timer;
  timer.start();

  for (unsigned i=0; i<numLoops; i++)
  {
    LXiStream_AudioMixer_mixAudio(srcBuffer, srcNumChannels, srcNumSamples, 1.0f, dstBuffer, dstNumChannels);
    LXiStream_AudioMixer_mixAudio(srcBuffer, srcNumChannels, srcNumSamples, 0.5f, dstBuffer, dstNumChannels);
  }

  const int elapsed = timer.elapsed();
  qDebug() << "LXiStream_AudioMixer_mixAudio = " << (srcNumSamples * srcNumChannels * numLoops / 1000) / elapsed << " Msamples/s";

  delete [] srcBuffer;
  delete [] dstBuffer;
}

// Implemented in deinterlace.mix.c
extern "C" void LXiStream_Common_Deinterlace_mixFields(const quint8 *, quint8 *, const quint8 *, unsigned);

void LXiStreamTest::Performance_DeinterlaceMix(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned width = 768;
  static const unsigned height = 576;
  static const unsigned numFrames = 500;

  quint8 * const _srcBuffer  = new quint8[size_t(width * height) + SMemoryBuffer::dataAlignVal];

  // Aligned buffers
  quint8 * const srcBuffer  = SMemoryBuffer::align(_srcBuffer);

  QTime timer;
  timer.start();

  for (unsigned f=0; f<numFrames; f++)
  {
    for (unsigned i=1, n=height-1; i<n; i+=2)
    {
      const uchar * const line0 = srcBuffer + ((i - 1) * width);
      uchar * const line1 = srcBuffer + (i * width);
      const uchar * const line2 = srcBuffer + ((i + 1) * width);

      LXiStream_Common_Deinterlace_mixFields(line0, line1, line2, width);
    }
  }

  const int elapsed = timer.elapsed();
  qDebug() << "LXiStream_Common_Deinterlace_mixFields = " << numFrames * 1000 / elapsed << " f/s";

  delete [] _srcBuffer;
}

#ifdef ENABLE_FFMPEG
// Implemented in videoencoder.convert.c
extern "C" void LXiStream_VideoEncoder_convertYUYVtoYUV422P(const void *, unsigned, size_t, quint8 *, quint8 *, quint8 *);
extern "C" void LXiStream_VideoEncoder_convertUYVYtoYUV422P(const void *, unsigned, size_t, quint8 *, quint8 *, quint8 *);
extern "C" void LXiStream_VideoEncoder_convertYUYVtoYUV420P(const void *, unsigned, size_t, quint8 *, quint8 *, quint8 *);
extern "C" void LXiStream_VideoEncoder_convertUYVYtoYUV420P(const void *, unsigned, size_t, quint8 *, quint8 *, quint8 *);

void LXiStreamTest::Performance_VideoEncoderConvert(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned width = 768;
  static const unsigned height = 576;
  static const unsigned numFrames = 500;

  qint16 * const _srcBuffer  = new qint16[size_t(width * height) + SMemoryBuffer::dataAlignVal];
  quint8 * const _dstBufferY = new quint8[size_t(width * height) + SMemoryBuffer::dataAlignVal];
  quint8 * const _dstBufferU = new quint8[size_t(width * height / 2) + SMemoryBuffer::dataAlignVal];
  quint8 * const _dstBufferV = new quint8[size_t(width * height / 2) + SMemoryBuffer::dataAlignVal];

  // Aligned buffers
  qint16 * const srcBuffer  = SMemoryBuffer::align(_srcBuffer);
  quint8 * const dstBufferY = SMemoryBuffer::align(_dstBufferY);
  quint8 * const dstBufferU = SMemoryBuffer::align(_dstBufferU);
  quint8 * const dstBufferV = SMemoryBuffer::align(_dstBufferV);

  QTime timer;

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_VideoEncoder_convertYUYVtoYUV422P(srcBuffer, height, width * sizeof(*srcBuffer), dstBufferY, dstBufferU, dstBufferV);
  int elapsed = timer.elapsed();
  qDebug() << "LXiStream_VideoEncoder_convertYUYVtoYUV422P = " << numFrames * 1000 / elapsed << " f/s";

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_VideoEncoder_convertUYVYtoYUV422P(srcBuffer, height, width * sizeof(*srcBuffer), dstBufferY, dstBufferU, dstBufferV);
  elapsed = timer.elapsed();
  qDebug() << "LXiStream_VideoEncoder_convertUYVYtoYUV422P = " << numFrames * 1000 / elapsed << " f/s";

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_VideoEncoder_convertYUYVtoYUV420P(srcBuffer, height, width * sizeof(*srcBuffer), dstBufferY, dstBufferU, dstBufferV);
  elapsed = timer.elapsed();
  qDebug() << "LXiStream_VideoEncoder_convertYUYVtoYUV420P = " << numFrames * 1000 / elapsed << " f/s";

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_VideoEncoder_convertUYVYtoYUV420P(srcBuffer, height, width * sizeof(*srcBuffer), dstBufferY, dstBufferU, dstBufferV);
  elapsed = timer.elapsed();
  qDebug() << "LXiStream_VideoEncoder_convertUYVYtoYUV420P = " << numFrames * 1000 / elapsed << " f/s";

  delete [] _srcBuffer;
  delete [] _dstBufferY;
  delete [] _dstBufferU;
  delete [] _dstBufferV;
}
#endif

// Implemented in simagebuffer.convert.c
extern "C" void LXiStream_SImageBuffer_convertBGRtoRGB(quint32 *rgb, const quint32 *bgr, unsigned numPixels);

void LXiStreamTest::Performance_BGRtoRGB(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned width = 768;
  static const unsigned height = 576;
  static const unsigned numFrames = 500;

  quint32 * const _srcBuffer = new quint32[size_t(width * height) + SMemoryBuffer::dataAlignVal];
  quint32 * const _dstBuffer = new quint32[size_t(width * height) + SMemoryBuffer::dataAlignVal];

  // Aligned buffers
  quint32 * const srcBuffer = SMemoryBuffer::align(_srcBuffer);
  quint32 * const dstBuffer = SMemoryBuffer::align(_dstBuffer);

  QTime timer;

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_SImageBuffer_convertBGRtoRGB(dstBuffer, srcBuffer, width * height);
  int elapsed = timer.elapsed();
  qDebug() << "LXiStream_SImageBuffer_convertBGRtoRGB = " << numFrames * 1000 / elapsed << " f/s";

  delete [] _srcBuffer;
  delete [] _dstBuffer;
}

// Implemented in simagebuffer.convert.c
extern "C" void LXiStream_SImageBuffer_convertYUYVtoRGB(quint32 *rgb, const quint16 *yuv, unsigned numPixels);
extern "C" void LXiStream_SImageBuffer_convertUYVYtoRGB(quint32 *rgb, const quint16 *yuv, unsigned numPixels);

void LXiStreamTest::Performance_YUVtoRGB(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  static const unsigned width = 768;
  static const unsigned height = 576;
  static const unsigned numFrames = 50;

  quint16 * const _srcBuffer = new quint16[size_t(width * height) + SMemoryBuffer::dataAlignVal];
  quint32 * const _dstBuffer = new quint32[size_t(width * height) + SMemoryBuffer::dataAlignVal];

  // Aligned buffers
  quint16 * const srcBuffer = SMemoryBuffer::align(_srcBuffer);
  quint32 * const dstBuffer = SMemoryBuffer::align(_dstBuffer);

  QTime timer;

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_SImageBuffer_convertYUYVtoRGB(dstBuffer, srcBuffer, width * height);
  int elapsed = timer.elapsed();
  qDebug() << "LXiStream_SImageBuffer_convertYUYVtoRGB = " << numFrames * 1000 / elapsed << " f/s";

  timer.start();
  for (unsigned i=0; i<numFrames; i++)
    LXiStream_SImageBuffer_convertUYVYtoRGB(dstBuffer, srcBuffer, width * height);
  elapsed = timer.elapsed();
  qDebug() << "LXiStream_SImageBuffer_convertUYVYtoRGB = " << numFrames * 1000 / elapsed << " f/s";

  delete [] _srcBuffer;
  delete [] _dstBuffer;
}
