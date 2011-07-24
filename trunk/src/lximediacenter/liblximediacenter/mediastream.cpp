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

#include "mediastream.h"
#include <LXiStreamGui>
#include "mediaserver.h"

namespace LXiMediaCenter {

MediaStream::MediaStream(void)
  : SGraph(),
    audio(NULL),
    video(NULL),
    sync(this),
    output(this)
{
}

MediaStream::~MediaStream()
{
  delete audio;
  delete video;

  qDebug() << "Stopped stream";
}

bool MediaStream::setup(const SHttpServer::RequestMessage &request,
                        QIODevice *socket,
                        STime duration,
                        SInterval frameRate,
                        SSize size,
                        SAudioFormat::Channels inChannels,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags,
                        SInterfaces::VideoEncoder::Flags videoEncodeFlags)
{
  const MediaServer::File file(request);

  delete audio;
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));
  connect(&sync, SIGNAL(compensateAudio(float)), &audio->resampler, SLOT(compensate(float)));

  if (file.url().queryItemValue("music") == "true")
  {
    connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &audio->gapRemover, SLOT(input(SAudioBuffer)));
    connect(&audio->gapRemover, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&sync, SIGNAL(output(SAudioBuffer)), &audio->audioNormalizer, SLOT(input(SAudioBuffer)));
    connect(&audio->audioNormalizer, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  }
  else
  {
    connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&sync, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  }

  connect(&audio->encoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  delete video;
  video = new Video(this);
  connect(&video->deinterlacer, SIGNAL(output(SVideoBuffer)), &video->subpictureRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subpictureRenderer, SIGNAL(output(SVideoBuffer)), &video->letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&video->letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &video->resizer, SLOT(input(SVideoBuffer)));
  connect(&video->resizer, SIGNAL(output(SVideoBuffer)), &video->box, SLOT(input(SVideoBuffer)));
  connect(&video->box, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &video->encoder, SLOT(input(SVideoBuffer)));
  connect(&video->encoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  // Set stream properties
  sync.setFrameRate(frameRate);

  Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
  if (file.url().hasQueryItem("resolution"))
  {
    const QStringList formatTxt = file.url().queryItemValue("resolution").split(',');

    const QStringList sizeTxt = formatTxt.first().split('x');
    if (sizeTxt.count() >= 2)
    {
      size.setWidth(sizeTxt[0].toInt());
      size.setHeight(sizeTxt[1].toInt());
      if (sizeTxt.count() >= 3)
        size.setAspectRatio(sizeTxt[2].toFloat());
      else
        size.setAspectRatio(1.0f);
    }

    if (formatTxt.count() >= 2)
    {
      if (formatTxt[1] == "box")
        aspectRatioMode = Qt::KeepAspectRatio;
      else if (formatTxt[1] == "zoom")
        aspectRatioMode = Qt::KeepAspectRatioByExpanding;
    }
  }

  SAudioFormat::Channels outChannels = inChannels;
  if (file.url().hasQueryItem("channels"))
  {
    const QStringList cl = file.url().queryItemValue("channels").split(',');

    if ((file.url().queryItemValue("music") == "true") && (cl.count() >= 2))
    {
      outChannels = SAudioFormat::Channels(cl[1].toUInt(NULL, 16));
    }
    else if (!cl.isEmpty())
    {
      const SAudioFormat::Channels c = SAudioFormat::Channels(cl[0].toUInt(NULL, 16));
      if ((SAudioFormat::numChannels(c) > 0) &&
          (SAudioFormat::numChannels(inChannels) > SAudioFormat::numChannels(c)))
      {
        outChannels = c;
      }
    }
  }

  if (file.url().queryItemValue("encodemode") == "fast")
  {
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;
    videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Fast;
    video->resizer.setHighQuality(false);
  }
  else
    video->resizer.setHighQuality(true);

  videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_HardBitrateLimit;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");
  header.setField("transferMode.dlna.org", "Streaming");
  if (file.url().hasQueryItem("contentFeatures"))
    header.setField("contentFeatures.dlna.org", QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii()));

  if ((file.suffix() == "mpeg") || (file.suffix() == "mpg") || (file.suffix() == "ts"))
  {
    if ((outChannels == SAudioFormat::Channels_Mono) || (outChannels == SAudioFormat::Channels_Stereo))
    {
      const SAudioCodec audioOutCodec("MP2", outChannels, 48000);
      audio->outChannels = audioOutCodec.channelSetup();
      audio->matrix.guessMatrices(audioOutCodec.channelSetup());
      audio->resampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else
    {
      const SAudioCodec audioOutCodec("AC3", outChannels, 48000);
      audio->outChannels = audioOutCodec.channelSetup();
      audio->matrix.guessMatrices(audioOutCodec.channelSetup());
      audio->resampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    if (!video->encoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!video->encoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    if (file.suffix() != "ts")
    { // Program stream
      output.openFormat("vob", audio->encoder.codec(), video->encoder.codec(), duration);
      header.setContentType("video/mpeg");
    }
    else
    { // Transport stream
      output.openFormat("mpegts", audio->encoder.codec(), video->encoder.codec(), duration);
      header.setContentType("video/mpeg");
    }
  }
  else if ((file.suffix() == "ogg") || (file.suffix() == "ogv"))
  {
    SAudioCodec audioOutCodec("VORBIS", outChannels, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
    {
      audioOutCodec.setCodec("FLAC", audioOutCodec.channelSetup(), audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    const SVideoCodec videoOutCodec("THEORA", size, frameRate);
    if (!video->encoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audio->encoder.codec(), video->encoder.codec(), duration);
    header.setContentType("video/ogg");
  }
  else if (file.suffix() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("FLV1", size, frameRate);
    if (!video->encoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audio->encoder.codec(), video->encoder.codec(), duration);
    header.setContentType("video/x-flv");
  }
  else
  {
    qDebug() << "Incorrect video format:" << file.suffix();
    return false;
  }

  video->resizer.setSize(size);
  video->resizer.setAspectRatioMode(aspectRatioMode);
  video->box.setSize(size);

  connect(socket, SIGNAL(disconnected()), SLOT(stop()));
  socket->write(header);
  output.setIODevice(socket, true);

  qDebug() << "Started video stream"
      << size.width() << "x" << size.height()
      << "@" << frameRate.toFrequency() << video->encoder.codec().codec()
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().codec()
      << header.contentType();

  return true;
}

bool MediaStream::setup(const SHttpServer::RequestMessage &request,
                        QIODevice *socket,
                        STime duration,
                        SAudioFormat::Channels inChannels,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags)
{
  const MediaServer::File file(request);

  delete audio;
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));

  if (file.url().queryItemValue("music") == "true")
  {
    connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &audio->gapRemover, SLOT(input(SAudioBuffer)));
    connect(&audio->gapRemover, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&sync, SIGNAL(output(SAudioBuffer)), &audio->audioNormalizer, SLOT(input(SAudioBuffer)));
    connect(&audio->audioNormalizer, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  }
  else
  {
    connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&sync, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  }

  connect(&audio->encoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  SAudioFormat::Channels outChannels = inChannels;
  if (file.url().hasQueryItem("channels"))
  {
    const QStringList cl = file.url().queryItemValue("channels").split(',');

    if ((file.url().queryItemValue("music") == "true") && (cl.count() >= 2))
      outChannels = SAudioFormat::Channels(cl[1].toUInt(NULL, 16));
    else if (!cl.isEmpty())
      outChannels = SAudioFormat::Channels(cl[0].toUInt(NULL, 16));
  }

  if (file.url().queryItemValue("encodemode") == "fast")
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");
  header.setField("transferMode.dlna.org", "Streaming");
  if (file.url().hasQueryItem("contentFeatures"))
    header.setField("contentFeatures.dlna.org", QByteArray::fromHex(file.url().queryItemValue("contentFeatures").toAscii()));

  if ((file.suffix() == "mpeg") || (file.suffix() == "mpa") || (file.suffix() == "ts"))
  {
    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channels_Stereo, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    if (file.suffix() != "ts")
    {
      output.openFormat("mp2", audio->encoder.codec(), SVideoCodec(), duration);
      header.setContentType("audio/mpeg");
    }
    else
    { // Transport stream
      output.openFormat("mpegts", audio->encoder.codec(), SVideoCodec(), duration);
      header.setContentType("video/mpeg");
    }
  }
  else if (file.suffix() == "mp3")
  {
    const SAudioCodec audioOutCodec("MP3", SAudioFormat::Channels_Stereo, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.openFormat("mp3", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/mp3");
  }
  else if (file.suffix() == "ac3")
  {
    const SAudioCodec audioOutCodec("AC3", outChannels, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ac3", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/x-ac3");
  }
  else if ((file.suffix() == "ogg") || (file.suffix() == "oga"))
  {
    SAudioCodec audioOutCodec("VORBIS", outChannels, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
    {
      audioOutCodec.setCodec("FLAC", audioOutCodec.channelSetup(), audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/ogg");
  }
  else if (file.suffix() == "lpcm")
  {
    const SAudioCodec audioOutCodec("PCM/S16BE", SAudioFormat::Channels_Stereo, 48000);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("s16be", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/L16;rate=48000;channels=2");
  }
  else if (file.suffix() == "wav")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("wav", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/wave");
  }
  else if (file.suffix() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->outChannels = audioOutCodec.channelSetup();
    audio->matrix.guessMatrices(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("video/x-flv");
  }
  else
  {
    qDebug() << "Incorrect audio format:" << file.suffix();
    return false;
  }

  connect(socket, SIGNAL(disconnected()), SLOT(stop()));
  socket->write(header);
  output.setIODevice(socket, true);

  qDebug() << "Started audio stream"
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().codec()
      << header.contentType();

  return true;
}


MediaStream::Audio::Audio(SGraph *parent)
  : outChannels(SAudioFormat::Channel_None),
    matrix(parent),
    resampler(parent),
    gapRemover(parent),
    audioNormalizer(parent),
    encoder(parent)
{
}

MediaStream::Audio::~Audio()
{
}


MediaStream::Video::Video(SGraph *parent)
  : deinterlacer(parent),
    subpictureRenderer(parent),
    letterboxDetectNode(parent),
    resizer(parent),
    box(parent),
    subtitleRenderer(parent),
    encoder(parent)
{
}

MediaStream::Video::~Video()
{
}


MediaTranscodeStream::MediaTranscodeStream(void)
  : MediaStream(),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this),
    videoGenerator(this),
    timeStampResampler(this)
{
}

bool MediaTranscodeStream::setup(const SHttpServer::RequestMessage &request,
                                 QIODevice *socket,
                                 SInterfaces::BufferReaderNode *input,
                                 STime duration,
                                 SInterfaces::AudioEncoder::Flags audioEncodeFlags,
                                 SInterfaces::VideoEncoder::Flags videoEncodeFlags)
{
  const MediaServer::File file(request);

  // Select streams
  QList<SIOInputNode::AudioStreamInfo> audioStreams = input->audioStreams();
  QList<SIOInputNode::VideoStreamInfo> videoStreams = input->videoStreams();
  QList<SIOInputNode::DataStreamInfo>  dataStreams  = input->dataStreams();

  QList<SIOInputNode::StreamId> selectedStreams;
  if (file.url().hasQueryItem("language"))
    selectedStreams += file.url().queryItemValue("language").toUInt(NULL, 16);
  else if (!audioStreams.isEmpty())
    selectedStreams += audioStreams.first();

  if (!videoStreams.isEmpty())
    selectedStreams += videoStreams.first();

  if (file.url().hasQueryItem("subtitles"))
  {
    if (!file.url().queryItemValue("subtitles").isEmpty())
      selectedStreams += file.url().queryItemValue("subtitles").toUInt(NULL, 16);
  }
  else if (!dataStreams.isEmpty())
    selectedStreams += dataStreams.first();

  input->selectStreams(selectedStreams);

  // Seek to start
  if (!duration.isValid())
    duration = input->duration();

  if (file.url().hasQueryItem("position"))
  {
    const STime pos = STime::fromSec(file.url().queryItemValue("position").toInt());
    input->setPosition(pos);

    if (duration > pos)
      duration -= pos;
    else
      duration = STime::null;
  }

  if (file.url().hasQueryItem("starttime"))
  {
    const STime pos = STime::fromSec(file.url().queryItemValue("starttime").toInt());
    sync.setStartTime(pos);

    if (duration.isPositive())
      duration += pos;
  }

  QVector<double> frameRates = STimeStampResamplerNode::standardFrameRates();
  if (file.url().hasQueryItem("framerates"))
  {
    QVector<double> rates;
    foreach (const QString &rate, file.url().queryItemValue("framerates").split(','))
    {
      bool ok = false;
      const double val = rate.toDouble(&ok);
      if (ok)
        rates += val;
    }

    if (!rates.isEmpty())
      frameRates = rates;
  }

  bool generateVideo = false;
  if (file.url().queryItemValue("music") == "true")
  {
    const QString musicMode = file.url().queryItemValue("musicmode");
    if (musicMode.startsWith("addvideo"))
    {
      SSize size(352, 288);
      if (file.url().hasQueryItem("resolution"))
      {
        const QStringList formatTxt = file.url().queryItemValue("resolution").split(',');

        const QStringList sizeTxt = formatTxt.first().split('x');
        if (sizeTxt.count() >= 2)
        {
          size.setWidth(sizeTxt[0].toInt());
          size.setHeight(sizeTxt[1].toInt());
          if (sizeTxt.count() >= 3)
            size.setAspectRatio(sizeTxt[2].toFloat());
          else
            size.setAspectRatio(1.0f);
        }
      }

      SImage blackImage(size, SImage::Format_RGB32);
      blackImage.fill(0);
      videoGenerator.setImage(blackImage);
      videoGenerator.setFrameRate(SInterval::fromFrequency(24));
      generateVideo = true;
    }
    else if (musicMode == "removevideo")
      videoStreams.clear();
  }

  // Set stream properties
  if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
  { // Decode audio and video
    const SAudioCodec audioInCodec = audioStreams.first().codec;
    const SVideoCodec videoInCodec = videoStreams.first().codec;

    const SInterval roundedFrameRate = STimeStampResamplerNode::roundFrameRate(videoInCodec.frameRate(), frameRates);
    const STime roundedDuration = duration * (roundedFrameRate.toFrequency() / videoInCodec.frameRate().toFrequency());

    timeStampResampler.setFrameRate(roundedFrameRate);

    if (MediaStream::setup(request, socket, roundedDuration,
                           roundedFrameRate, videoInCodec.size(),
                           audioInCodec.channelSetup(),
                           audioEncodeFlags, videoEncodeFlags))
    {
      // Audio
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

      // Video
      connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

      // Data
      connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &timeStampResampler, SLOT(input(SSubpictureBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SSubpictureBuffer)), &video->subpictureRenderer, SLOT(input(SSubpictureBuffer)));
      connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &video->subtitleRenderer, SLOT(input(SSubtitleBuffer)));

      if (audio->outChannels == SAudioFormat::Channels_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      // To improve performance of multithreaded decoding.
      videoDecoder.setFlags(SInterfaces::VideoDecoder::Flag_Fast);

      return true;
    }
  }
  else if (!audioStreams.isEmpty() && generateVideo)
  { // Decode audio, generate video
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    const SInterval frameRate = videoGenerator.frameRate();
    timeStampResampler.setFrameRate(frameRate);

    videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Fast;

    if (MediaStream::setup(request, socket, duration,
                           frameRate, videoGenerator.image().size(),
                           audioInCodec.channelSetup(),
                           audioEncodeFlags, videoEncodeFlags))
    {
      // Audio
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

      // Video
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &videoGenerator, SLOT(input(SAudioBuffer)));
      connect(&videoGenerator, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

      // Data
      connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &timeStampResampler, SLOT(input(SSubpictureBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SSubpictureBuffer)), &video->subpictureRenderer, SLOT(input(SSubpictureBuffer)));
      connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
      connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &video->subtitleRenderer, SLOT(input(SSubtitleBuffer)));

      if (audio->outChannels == SAudioFormat::Channels_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }
  else if (!audioStreams.isEmpty())
  { // Decode audio
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    if (MediaStream::setup(request, socket, duration,
                           audioInCodec.channelSetup()))
    {
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

      if (audio->outChannels == SAudioFormat::Channels_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }

  return false;
}

} // End of namespace
