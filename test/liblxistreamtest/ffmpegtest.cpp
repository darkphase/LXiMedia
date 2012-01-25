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

#include "ffmpegtest.h"
#include "streamtest.h"
#include <QtTest>
#include <LXiStream>

void FFMpegTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);

  QVERIFY(mediaApp->loadModule("lxistream_ffmpeg"));

//  QTime timer; timer.start();
//  const SMediaInfo mediaInfo();
//  qDebug() << mediaInfo.format() << timer.elapsed();
}

void FFMpegTest::cleanupTestCase(void)
{
  delete mediaApp;
  mediaApp = NULL;
}

/*! Tests deep audio file probe.
 */
void FFMpegTest::MediaFileInfoAudioDeep(void)
{
  const SMediaInfo mediaInfo(QUrl("file::/SoundTest.ogg"));

  QCOMPARE(mediaInfo.fileType(), SMediaInfo::ProbeInfo::FileType_Audio);
  QCOMPARE(mediaInfo.metadata("title").toString(), QString("SoundTest"));
  QCOMPARE(mediaInfo.titles().first().audioStreams.first().codec.codec(), QString("VORBIS"));
  QVERIFY(mediaInfo.titles().first().videoStreams.isEmpty());
  QVERIFY(mediaInfo.titles().first().thumbnail.isNull());

  QCOMPARE(mediaInfo.titles().first().duration.toSec(), 3);
}

/*! Tests encoding and decoding audio samples.
 */
void FFMpegTest::AudioEncodeDecode(void)
{
  static const QSet<QString> skipCodecs = QSet<QString>()
      << "AAC" << "ALAC" << "MP3" << "SONIC" << "VORBIS" << "WMAV1" << "WMAV2"
      << "PCM/S16LE" << "PCM/S16BE" << "PCM/U16LE" << "PCM/U16BE" << "PCM/S8"
      << "PCM/U8" << "PCM/MULAW" << "PCM/ALAW" << "PCM/S32LE" << "PCM/S32BE"
      << "PCM/U32LE" << "PCM/U32BE" << "PCM/S24LE" << "PCM/S24BE" << "PCM/U24LE"
      << "PCM/U24BE" << "PCM/S24DAUD" << "PCM/ZORK" << "PCM/S16LEP" << "PCM/DVD"
      << "PCM/F32BE" << "PCM/F32LE" << "PCM/F64BE" << "PCM/F64LE"
      << "ADPCM/ADX" << "ADPCM/G722" << "ADPCM/G726" << "ADPCM/IMA_WAV"
      << "AMR/WB"
      << "DPCM/ROQ";

  const QSet<QString> decoders = QSet<QString>::fromList(SAudioDecoderNode::codecs());
  const QSet<QString> encoders = QSet<QString>::fromList(SAudioEncoderNode::codecs());

  QList<QString> test = (encoders & decoders).toList();
  qSort(test);
  foreach (const QString &codec, test)
  if (!skipCodecs.contains(codec))
    AudioEncodeDecode(codec.toAscii());
}

void FFMpegTest::AudioEncodeDecode(const char *codecName)
{
//  qDebug() << codecName;

  const SAudioBuffer inBuffer = StreamTest::makeTestBuffer(65536);

  SInterfaces::AudioEncoder *audioEncoder = SInterfaces::AudioEncoder::create(NULL, SAudioCodec(codecName, inBuffer.format().channelSetup(), inBuffer.format().sampleRate()));
  SInterfaces::AudioDecoder *audioDecoder = NULL;

  if (audioEncoder)
  {
    // Now encode it
    SEncodedAudioBufferList encBuffers;
    encBuffers += audioEncoder->encodeBuffer(inBuffer);
    encBuffers += audioEncoder->encodeBuffer(inBuffer);
    encBuffers += audioEncoder->encodeBuffer(SAudioBuffer());

    QVERIFY2(!encBuffers.isEmpty(), codecName);

    // And decode it again
    audioDecoder = SInterfaces::AudioDecoder::create(NULL, encBuffers.first().codec(), NULL);
    QVERIFY(audioDecoder);

    SAudioBufferList decBuffers;
    foreach (const SEncodedAudioBuffer &buffer, encBuffers)
      decBuffers += audioDecoder->decodeBuffer(buffer);

    decBuffers += audioDecoder->decodeBuffer(SEncodedAudioBuffer());

    QVERIFY2(!decBuffers.isEmpty(), codecName);

    // And check the result
    SAudioBuffer outBuffer = decBuffers; // This will merge all decoded buffers into one.
    QVERIFY2(outBuffer.size() >= inBuffer.size(), codecName);
    QVERIFY2(outBuffer.numSamples() >= inBuffer.numSamples(), codecName);
    QCOMPARE(outBuffer.format().numChannels(), inBuffer.format().numChannels());

    if (SAudioEncoderNode::losslessCodecs().contains(codecName) && (outBuffer.size() >= inBuffer.size()))
    for (int i=0, n=inBuffer.numSamples()*inBuffer.format().numChannels(); i<n; i+=4)
    {
      QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i],   reinterpret_cast<const qint16 *>(inBuffer.data())[i]);
      QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+1], reinterpret_cast<const qint16 *>(inBuffer.data())[i+1]);
      QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+2], reinterpret_cast<const qint16 *>(inBuffer.data())[i+2]);
      QCOMPARE(reinterpret_cast<const qint16 *>(outBuffer.data())[i+3], reinterpret_cast<const qint16 *>(inBuffer.data())[i+3]);
    }
  }

  delete audioDecoder;
  delete audioEncoder;
}

/*! Tests encoding and decoding video.
 */
void FFMpegTest::VideoEncodeDecode(void)
{
  static const QSet<QString> skipCodecs = QSet<QString>()
      << "ASV1" << "ASV2" << "FFVHUFF" << "GIF" << "H261" << "H263" << "H264"
      << "THEORA" << "WMV2";

  const QSet<QString> decoders = QSet<QString>::fromList(SVideoDecoderNode::codecs());
  const QSet<QString> encoders = QSet<QString>::fromList(SVideoEncoderNode::codecs());

  QList<QString> test = (encoders & decoders).toList();
  qSort(test);
  foreach (const QString &codec, test)
  if (!skipCodecs.contains(codec))
    VideoEncodeDecode(codec.toAscii());
}

void FFMpegTest::VideoEncodeDecode(const char *codecName)
{
//  qDebug() << codecName;

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

  SInterfaces::VideoEncoder *videoEncoder = SInterfaces::VideoEncoder::create(NULL, SVideoCodec(codecName, SSize(width, height), SInterval::fromFrequency(25)));
  SInterfaces::VideoDecoder *videoDecoder = NULL;

  // Now encode it
  if (videoEncoder)
  {
    QTime timer;
    timer.start();

    SEncodedVideoBufferList encBuffers;
    for (int i=0; i<count; i++)
    {
      inBuffer.setTimeStamp(STime::fromClock(i, 25.0));
      encBuffers += videoEncoder->encodeBuffer(inBuffer);
    }

    encBuffers += videoEncoder->encodeBuffer(SVideoBuffer());
    QCOMPARE(encBuffers.count(), count + 1); // +1 for flush buffer.

    // And decode it again
    videoDecoder = SInterfaces::VideoDecoder::create(NULL, encBuffers.first().codec(), NULL);
    QVERIFY2(videoDecoder, codecName);

    SVideoBufferList decBuffers;
    foreach (const SEncodedVideoBuffer &buffer, encBuffers)
      decBuffers += videoDecoder->decodeBuffer(buffer);

    QVERIFY2(!decBuffers.isEmpty(), codecName);
    QCOMPARE(decBuffers.count(), count + 1); // +1 for flush buffer.

    if (!qApp->arguments().contains("-silent"))
      qDebug() << codecName << QString::number(float(count * 1000) / float(timer.elapsed()), 'f', 2).toAscii().data() << "fps";
  }

  delete videoDecoder;
  delete videoEncoder;
}
