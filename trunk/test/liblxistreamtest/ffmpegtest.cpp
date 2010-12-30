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

#include "ffmpegtest.h"
#include <QtTest>
#include <LXiStream>
#include "lxistream/plugins/ffmpeg/audiodecoder.h"
#include "lxistream/plugins/ffmpeg/audioencoder.h"
#include "lxistream/plugins/ffmpeg/bufferreader.h"
#include "lxistream/plugins/ffmpeg/bufferwriter.h"
#include "lxistream/plugins/ffmpeg/ffmpegcommon.h"
#include "lxistream/plugins/ffmpeg/module.h"
#include "lxistream/plugins/ffmpeg/videodecoder.h"
#include "lxistream/plugins/ffmpeg/videoencoder.h"


void FFMpegTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  QVERIFY(SSystem::initialize(SSystem::Initialize_Devices |
                              SSystem::Initialize_LogToConsole, 0));

  QVERIFY(SSystem::loadModule(new FFMpegBackend::Module()));

//  const SMediaInfo mediaInfo("");
//  qDebug() << mediaInfo.videoStreams().first().codec.frameRate();
//
//  QFile file("/tmp/test.jpeg");
//  if (file.open(QFile::WriteOnly))
//    file.write(mediaInfo.thumbnails().first());
}

void FFMpegTest::cleanupTestCase(void)
{
  SSystem::shutdown();
}

/*! Tests deep audio file probe.
 */
void FFMpegTest::MediaFileInfoAudioDeep(void)
{
  const SMediaInfo mediaInfo(":/SoundTest.ogg");

  QVERIFY(mediaInfo.containsAudio());
  QVERIFY(!mediaInfo.containsVideo());
  QVERIFY(!mediaInfo.containsImage());
  QCOMPARE(mediaInfo.title(), QString("SoundTest"));
  QVERIFY(mediaInfo.thumbnails().isEmpty());
  QCOMPARE(mediaInfo.audioStreams().first().codec.codec(), QString("VORBIS"));
  QVERIFY(mediaInfo.videoStreams().isEmpty());

  QCOMPARE(mediaInfo.duration().toSec(), 3);
}

/*! Tests encoding and decoding audio samples.
 */
void FFMpegTest::AudioEncodeDecode(void)
{
  FFMpegBackend::AudioEncoder audioEncoder("", NULL);
  FFMpegBackend::AudioDecoder audioDecoder("", NULL);

  // Prepare a one-channel buffer with alternating values 32 and 64
  SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                     SAudioFormat::Channel_Mono,
                                     8000),
                        2048);

  for (unsigned i=0; i<2048; i+=2)
  {
    reinterpret_cast<qint16 *>(inBuffer.data())[i]   = 32;
    reinterpret_cast<qint16 *>(inBuffer.data())[i+1] = 64;
  }

  // Now encode it with FLAC (Free Lossless Audio Codec) if available.
  if (audioEncoder.openCodec(SAudioCodec("FLAC", SAudioFormat::Channel_Mono, 8000)))
  {
    SEncodedAudioBufferList encBuffers;
    for (unsigned i=0; i<8; i++)
      encBuffers += audioEncoder.encodeBuffer(inBuffer);

    QVERIFY(!encBuffers.isEmpty());

    // And decode it again
    QVERIFY(audioDecoder.openCodec(encBuffers.first().codec()));
    SAudioBufferList decBuffers;
    foreach (const SEncodedAudioBuffer &buffer, encBuffers)
      decBuffers += audioDecoder.decodeBuffer(buffer);

    QVERIFY(!decBuffers.isEmpty());

    // And check the result (As FLAC is lossless, we can do a bitwise compare)
    SAudioBuffer outBuffer = decBuffers; // This will merge all decoded buffers into one.
    QVERIFY(outBuffer.size() >= inBuffer.size());
    QVERIFY(outBuffer.numSamples() >= inBuffer.numSamples());
    QCOMPARE(outBuffer.format().numChannels(), 1);
    for (unsigned i=0; i<2048; i+=2)
    {
      QCOMPARE(reinterpret_cast<qint16 *>(outBuffer.data())[i],   qint16(32));
      QCOMPARE(reinterpret_cast<qint16 *>(outBuffer.data())[i+1], qint16(64));
    }
  }
}

/*! Tests encoding and decoding video.
 */
void FFMpegTest::VideoEncodeDecode(void)
{
  QSet<QByteArray> encoders, decoders;
  for (::AVCodec *codec=::av_codec_next(NULL); codec; codec=::av_codec_next(codec))
  if (codec->type == CODEC_TYPE_VIDEO)
  {
    if (codec->encode)
      encoders += FFMpegBackend::FFMpegCommon::fromFFMpegCodecID(codec->id);

    if (codec->decode)
      decoders += FFMpegBackend::FFMpegCommon::fromFFMpegCodecID(codec->id);
  }

  const QSet<QByteArray> codecs = QSet<QByteArray>()
      << "FLV1" << "H263" << "FFVHUFF" << "MJPEG" << "MPEG1" << "MPEG2"
      << "MPEG4" << "PBM";// Buggy: << "THEORA";

  QList<QByteArray> test = (codecs & encoders & decoders).toList();
  qSort(test);
  foreach (const QByteArray &codec, test)
    VideoEncodeDecode(codec);
}

void FFMpegTest::VideoEncodeDecode(const char *codecName)
{
  FFMpegBackend::VideoEncoder videoEncoder("", NULL);
  FFMpegBackend::VideoDecoder videoDecoder("", NULL);

  // Prepare a buffer
  static const unsigned width = 352;
  static const unsigned height = 288;
  static const int count = 32;

  SVideoBuffer inBuffer(SVideoFormat(SVideoFormat::Format_YUYV422,
                                     SSize(width, height),
                                     SInterval::fromFrequency(25),
                                     SVideoFormat::FieldMode_Progressive));

  for (unsigned y=0; y<height; y++)
  {
    quint16 * const line = reinterpret_cast<quint16 *>(inBuffer.scanLine(y, 0));
    for (unsigned x=0; x<width; x++)
      line[x] = (x * y) * 10;
  }

  // Now encode it
  QVERIFY(videoEncoder.openCodec(SVideoCodec(codecName, SSize(width, height), SInterval::fromFrequency(25))));
  SEncodedVideoBufferList encBuffers;
  for (int i=0; i<count; i++)
  {
    inBuffer.setTimeStamp(STime::fromClock(i, 25.0));
    encBuffers += videoEncoder.encodeBuffer(inBuffer);
  }

  encBuffers += videoEncoder.encodeBuffer(SVideoBuffer());
  QCOMPARE(encBuffers.count(), count);

  // And decode it again
  QVERIFY(videoDecoder.openCodec(encBuffers.first().codec()));
  SVideoBufferList decBuffers;
  foreach (const SEncodedVideoBuffer &buffer, encBuffers)
    decBuffers += videoDecoder.decodeBuffer(buffer);

  decBuffers += videoDecoder.decodeBuffer(SEncodedVideoBuffer());
  QCOMPARE(decBuffers.count(), count);
}
