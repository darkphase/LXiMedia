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
                        STime position, STime duration,
                        const SAudioFormat &inputAudioFormat,
                        const SVideoFormat &inputVideoFormat,
                        bool enableNormalize,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags,
                        SInterfaces::VideoEncoder::Flags videoEncodeFlags,
                        int videoGopSize)
{
  connect(&output, SIGNAL(closed(QIODevice *)), SLOT(stop()));

  delete audio;
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));
  connect(&sync, SIGNAL(compensateAudio(float)), &audio->resampler, SLOT(compensate(float)));

  if (enableNormalize)
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
  connect(&video->deinterlacer, SIGNAL(output(SVideoBuffer)), &video->letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&video->letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &video->subpictureRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subpictureRenderer, SIGNAL(output(SVideoBuffer)), &video->resizer, SLOT(input(SVideoBuffer)));
  connect(&video->resizer, SIGNAL(output(SVideoBuffer)), &video->box, SLOT(input(SVideoBuffer)));
  connect(&video->box, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &video->encoder, SLOT(input(SVideoBuffer)));
  connect(&video->encoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  // Set output stream properties
  Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
  SAudioFormat audioFormat = inputAudioFormat;
  SVideoFormat videoFormat = inputVideoFormat;

  decodeSize(request.url(), videoFormat, aspectRatioMode);

  decodeChannels(request.url(), audioFormat);

  if (request.url().hasQueryItem("subtitlesize"))
    video->subtitleRenderer.setFontSize(request.url().queryItemValue("subtitlesize").toFloat());

  if (request.url().queryItemValue("encodemode") == "fast")
  {
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;
    videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Fast;
    video->resizer.setHighQuality(false);
  }
  else
    video->resizer.setHighQuality(true);

  videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_HardBitrateLimit;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setCacheControl(-1);

  // Match with DLNA profile
  const QString contentFeatures = QByteArray::fromBase64(request.url().queryItemValue("contentFeatures").toAscii());
  const MediaProfiles::VideoProfile videoProfile = MediaServer::mediaProfiles().videoProfileFor(contentFeatures);
  if (videoProfile != 0) // DLNA stream.
  {
    MediaProfiles::correctFormat(videoProfile, audioFormat);
    MediaProfiles::correctFormat(videoProfile, videoFormat);

    header.setField("transferMode.dlna.org", "Streaming");
    header.setField("contentFeatures.dlna.org", contentFeatures);
    if (position.isValid() && duration.isValid() && !duration.isNull())
    {
      header.setField("availableSeekRange.dlna.org",
                      "1 npt=" + QString::number(float(position.toMSec()) / 1000.0f, 'f', 2) +
                      "-" + QString::number(float((position + duration).toMSec()) / 1000.0f, 'f', 2));
    }

    if (!output.openFormat(MediaProfiles::formatFor(videoProfile)))
    {
      qWarning() << "Could not open file format:" << MediaProfiles::formatFor(videoProfile);
      return false;
    }

    const SAudioCodec audioCodec(MediaProfiles::audioCodecFor(videoProfile, audioFormat.channelSetup()), audioFormat.channelSetup(), audioFormat.sampleRate());
    audio->outChannels = audioCodec.channelSetup();
    audio->matrix.guessMatrices(audioCodec.channelSetup());
    audio->resampler.setSampleRate(audioCodec.sampleRate());
    if (!audio->encoder.openCodec(audioCodec, &output, duration, audioEncodeFlags))
    {
      qWarning() << "Could not open audio codec:" << audioCodec.name();
      return false;
    }

    SVideoCodec videoCodec(MediaProfiles::videoCodecFor(videoProfile), videoFormat.size(), videoFormat.frameRate());
    videoCodec.setGopSize(videoGopSize);

    video->resizer.setSize(videoCodec.size());
    video->resizer.setAspectRatioMode(aspectRatioMode);
    video->box.setSize(videoCodec.size());

    if (!video->encoder.openCodec(videoCodec, &output, duration, videoEncodeFlags))
    {
      qWarning() << "Could not open video codec:" << videoCodec.name();
      return false;
    }

    sync.setFrameRate(videoFormat.frameRate());

    header.setContentType(MediaProfiles::mimeTypeFor(videoProfile));
  }
  else // Non-DLNA stream.
  {
    SAudioCodec audioCodec;
    SVideoCodec videoCodec;
    QString format = request.url().queryItemValue("format");

    if ((format == "ogv") && SIOOutputNode::formats().contains("ogg"))
    {
      QSize size = videoFormat.size().absoluteSize();
      if ((size.width() > 768) || (size.height() > 576))
      {
        size.scale(768, 576, Qt::KeepAspectRatio);
        videoFormat.setSize(size);
      }

      if (SAudioEncoderNode::codecs().contains("vorbis"))
        audioCodec = SAudioCodec("vorbis", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("flac", SAudioFormat::Channels_Stereo, 44100);

      videoCodec = SVideoCodec("theora", videoFormat.size(), videoFormat.frameRate());
      videoCodec.setGopSize(videoGopSize);

      format = "ogg";
      header.setContentType(SHttpEngine::mimeVideoOgg);
    }
    else if ((format == "flv") && SIOOutputNode::formats().contains(format))
    {
      QSize size = videoFormat.size().absoluteSize();
      if ((size.width() > 768) || (size.height() > 576))
      {
        size.scale(768, 576, Qt::KeepAspectRatio);
        videoFormat.setSize(size);
      }

      if (SAudioEncoderNode::codecs().contains("mp3"))
        audioCodec = SAudioCodec("mp3", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("pcm_s16le", SAudioFormat::Channels_Stereo, 44100);

      videoCodec = SVideoCodec("flv1", videoFormat.size(), videoFormat.frameRate());
      videoCodec.setGopSize(videoGopSize);

      header.setContentType(SHttpEngine::mimeVideoFlv);
    }
    else if ((format == "matroska") && SIOOutputNode::formats().contains(format))
    {
      if (SAudioEncoderNode::codecs().contains("mp3"))
        audioCodec = SAudioCodec("mp3", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("mp2", SAudioFormat::Channels_Stereo, 44100);

      videoCodec = SVideoCodec("mpeg4", videoFormat.size(), videoFormat.frameRate());
      videoCodec.setGopSize(videoGopSize);

      header.setContentType(SHttpEngine::mimeVideoMatroska);
    }
    else // Default to mpeg
    {
      const SInterval roundedFrameRate =
          STimeStampResamplerNode::roundFrameRate(
              videoFormat.frameRate(),
              STimeStampResamplerNode::standardFrameRates());

      if ((audioFormat.channelSetup() == SAudioFormat::Channels_Mono) ||
          (audioFormat.channelSetup() == SAudioFormat::Channels_Stereo))
      {
        audioCodec = SAudioCodec("mp2", audioFormat.channelSetup(), audioFormat.sampleRate());
      }
      else
        audioCodec = SAudioCodec("ac3", audioFormat.channelSetup(), audioFormat.sampleRate());

      videoCodec = SVideoCodec("mpeg2video", videoFormat.size(), roundedFrameRate);
      videoCodec.setGopSize(videoGopSize);

      if (format == "mpegts")
      {
        header.setContentType(SHttpEngine::mimeVideoMpeg);
      }
      else if (format == "m2ts")
      {
        header.setContentType(SHttpEngine::mimeVideoMpegM2TS);
      }
      else
      {
        format = "vob";
        header.setContentType(SHttpEngine::mimeVideoMpeg);
      }
    }

    if (!output.openFormat(format))
    {
      qWarning() << "Could not open file format:" << format;
      return false;
    }

    audio->outChannels = audioCodec.channelSetup();
    audio->matrix.guessMatrices(audioCodec.channelSetup());
    audio->resampler.setSampleRate(audioCodec.sampleRate());
    if (!audio->encoder.openCodec(audioCodec, &output, duration, audioEncodeFlags))
    {
      qWarning() << "Could not open audio codec:" << audioCodec.name();
      return false;
    }

    video->resizer.setSize(videoCodec.size());
    video->resizer.setAspectRatioMode(aspectRatioMode);
    video->box.setSize(videoCodec.size());

    if (!video->encoder.openCodec(videoCodec, &output, duration, videoEncodeFlags))
    {
      qWarning() << "Could not open video codec:" << videoCodec.name();
      return false;
    }

    sync.setFrameRate(videoFormat.frameRate());
  }

  connect(socket, SIGNAL(disconnected()), SLOT(stop()));
  socket->write(header);

  connect(&output, SIGNAL(closed(QIODevice *)), request.httpEngine, SLOT(closeSocket(QIODevice *)));
  output.setIODevice(socket);

  qDebug() << "Started video stream"
      << videoFormat.size().toString()
      << "@" << videoFormat.frameRate().toFrequency() << video->encoder.codec().name()
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().name()
      << header.contentType();

  return true;
}

bool MediaStream::setup(const SHttpServer::RequestMessage &request,
                        QIODevice *socket,
                        STime position, STime duration,
                        const SAudioFormat &inputAudioFormat,
                        bool enableNormalize,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags)
{
  delete audio;
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));

  if (enableNormalize)
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

  // Set output stream properties
  SAudioFormat audioFormat = inputAudioFormat;

  decodeChannels(request.url(), audioFormat);

  if (request.url().queryItemValue("encodemode") == "fast")
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setCacheControl(-1);

  // Match with DLNA profile
  const QString contentFeatures = QByteArray::fromBase64(request.url().queryItemValue("contentFeatures").toAscii());
  const MediaProfiles::AudioProfile audioProfile = MediaServer::mediaProfiles().audioProfileFor(contentFeatures);
  if (audioProfile != 0) // DLNA stream.
  {
    MediaProfiles::correctFormat(audioProfile, audioFormat);

    header.setField("transferMode.dlna.org", "Streaming");
    header.setField("contentFeatures.dlna.org", contentFeatures);
    if (position.isValid() && duration.isValid() && !duration.isNull())
    {
      header.setField("X-AvailableSeekRange",
                      "1 npt=" + QString::number(float(position.toMSec()) / 1000.0f, 'f', 2) +
                      "-" + QString::number(float((position + duration).toMSec()) / 1000.0f, 'f', 2));
    }

    if (!output.openFormat(MediaProfiles::formatFor(audioProfile)))
    {
      qWarning() << "Could not open file format:" << MediaProfiles::formatFor(audioProfile);
      return false;
    }

    const SAudioCodec audioCodec(MediaProfiles::audioCodecFor(audioProfile, audioFormat.channelSetup()), audioFormat.channelSetup(), audioFormat.sampleRate());
    audio->outChannels = audioCodec.channelSetup();
    audio->matrix.guessMatrices(audioCodec.channelSetup());
    audio->resampler.setSampleRate(audioCodec.sampleRate());
    if (!audio->encoder.openCodec(audioCodec, &output, duration, audioEncodeFlags))
    {
      qWarning() << "Could not open audio codec:" << audioCodec.name();
      return false;
    }

    header.setContentType(MediaProfiles::mimeTypeFor(audioProfile));
  }
  else // Non-DLNA stream.
  {
    SAudioCodec audioCodec;
    QString format = request.url().queryItemValue("format");

    if ((format == "adts") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("aac", SAudioFormat::Channels_Stereo, audioFormat.sampleRate());
      header.setContentType(SHttpEngine::mimeAudioAac);
    }
    else if ((format == "ac3") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("ac3", SAudioFormat::Channels_Stereo, audioFormat.sampleRate());
      header.setContentType(SHttpEngine::mimeAudioAc3);
    }
    else if ((format == "mp3") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("mp3", SAudioFormat::Channels_Stereo, 44100);
      header.setContentType(SHttpEngine::mimeAudioMp3);
    }
    else if ((format == "oga") && SIOOutputNode::formats().contains("ogg"))
    {
      if (SAudioEncoderNode::codecs().contains("vorbis"))
        audioCodec = SAudioCodec("vorbis", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("flac", SAudioFormat::Channels_Stereo, 44100);

      format = "ogg";
      header.setContentType(SHttpEngine::mimeAudioOgg);
    }
    else if ((format == "s16be") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("pcm_s16be", SAudioFormat::Channels_Stereo, 48000);
      header.setContentType(SHttpEngine::mimeAudioLpcm);
    }
    else if ((format == "wav") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("pcm_s16le", SAudioFormat::Channels_Stereo, 44100);
      header.setContentType(SHttpEngine::mimeAudioWave);
    }
    else // Default to mpeg
    {
      audioCodec = SAudioCodec("mp2", SAudioFormat::Channels_Stereo, 44100);
      format = "mp2";
      header.setContentType(SHttpEngine::mimeAudioMpeg);
    }

    if (!output.openFormat(format))
    {
      qWarning() << "Could not open file format:" << format;
      return false;
    }

    audio->outChannels = audioCodec.channelSetup();
    audio->matrix.guessMatrices(audioCodec.channelSetup());
    audio->resampler.setSampleRate(audioCodec.sampleRate());
    if (!audio->encoder.openCodec(audioCodec, &output, duration, audioEncodeFlags))
    {
      qWarning() << "Could not open audio codec:" << audioCodec.name();
      return false;
    }
  }

  connect(socket, SIGNAL(disconnected()), SLOT(stop()));
  socket->write(header);

  connect(&output, SIGNAL(closed(QIODevice*)), request.httpEngine, SLOT(closeSocket(QIODevice *)));
  output.setIODevice(socket);

  qDebug() << "Started audio stream"
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().name()
      << header.contentType();

  return true;
}

SSize MediaStream::decodeSize(const QUrl &url)
{
  SVideoFormat format(SVideoFormat::Format_Invalid, SSize(720, 576, 1.42222));
  Qt::AspectRatioMode aspectRatioMode;
  decodeSize(url, format, aspectRatioMode);

  return format.size();
}

void MediaStream::decodeSize(const QUrl &url, SVideoFormat &videoFormat, Qt::AspectRatioMode &aspectRatioMode)
{
  if (url.hasQueryItem("resolution"))
  {
	SSize size = videoFormat.size();

    const QStringList formatTxt = url.queryItemValue("resolution").split(',');

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

    videoFormat.setSize(size);
  }
}

SAudioFormat::Channels MediaStream::decodeChannels(const QUrl &url)
{
  SAudioFormat format(SAudioFormat::Format_Invalid, SAudioFormat::Channels_Stereo);
  decodeChannels(url, format);

  return format.channelSetup();
}

void MediaStream::decodeChannels(const QUrl &url, SAudioFormat &audioFormat)
{
  if (url.hasQueryItem("channels"))
  {
    const SAudioFormat::Channels c =
        SAudioFormat::Channels(url.queryItemValue("channels").toUInt(NULL, 16));

    if ((SAudioFormat::numChannels(c) > 0) &&
        (url.hasQueryItem("music") ||
         (audioFormat.numChannels() > SAudioFormat::numChannels(c))))
    {
      audioFormat.setChannelSetup(c);
    }
  }
}

SSize MediaStream::toStandardVideoSize(const SSize &size)
{
  if ((size.width() > 1280) || (size.height() > 720))
    return SSize(1920, 1080);
  else if ((size.width() > 768) || (size.height() > 576))
    return SSize(1280, 720);
  else if ((size.width() > 640) || (size.height() > 480))
    return SSize(768, 576);
  else if ((size.width() > 384) || (size.height() > 288))
    return SSize(640, 480);
  else
    return SSize(384, 288);
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
    letterboxDetectNode(parent),
    subpictureRenderer(parent),
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

bool MediaTranscodeStream::setup(
    const SHttpServer::RequestMessage &request,
    QIODevice *socket,
    SInputNode *input,
    STime duration,
    bool enableNormalize,
    SInterfaces::AudioEncoder::Flags audioEncodeFlags,
    SInterfaces::VideoEncoder::Flags videoEncodeFlags)
{
  int titleId = 0;
  if (request.url().hasQueryItem("title"))
    titleId = request.url().queryItemValue("title").toInt();

  if (audioDecoder.open(input) && videoDecoder.open(input) && dataDecoder.open(input))
  {
    // Select streams
    QList<SInputNode::AudioStreamInfo> audioStreams = input->audioStreams(titleId);
    QList<SInputNode::VideoStreamInfo> videoStreams = input->videoStreams(titleId);
    QList<SInputNode::DataStreamInfo>  dataStreams  = input->dataStreams(titleId);

    QVector<SInputNode::StreamId> selectedStreams;
    if (request.url().hasQueryItem("language"))
    {
      selectedStreams += SInputNode::StreamId::fromString(request.url().queryItemValue("language"));
    }
    else foreach (const SInputNode::AudioStreamInfo &stream, audioStreams)
    if (!stream.codec.isNull())
    {
      selectedStreams += stream;
      break;
    }

    foreach (const SInputNode::VideoStreamInfo &stream, videoStreams)
    if (!stream.codec.isNull())
    {
      selectedStreams += stream;
      break;
    }

    if (request.url().hasQueryItem("subtitles"))
    {
      if (!request.url().queryItemValue("subtitles").isEmpty())
        selectedStreams += SInputNode::StreamId::fromString(request.url().queryItemValue("subtitles"));
    }
    else foreach (const SInputNode::DataStreamInfo &stream, dataStreams)
    if (!stream.codec.isNull())
    {
      selectedStreams += stream;
      break;
    }

    input->selectStreams(titleId, selectedStreams);

    // Seek to start
    if (!duration.isValid())
      duration = input->duration();

    if (request.url().hasQueryItem("position"))
    {
      const STime pos = STime::fromSec(request.url().queryItemValue("position").toInt());
      input->setPosition(pos);

      if (duration > pos)
        duration -= pos;
      else
        duration = STime::null;
    }

    if (request.url().hasQueryItem("starttime"))
    {
      const STime pos = STime::fromSec(request.url().queryItemValue("starttime").toInt());
      sync.setStartTime(pos);

      if (duration.isPositive())
        duration += pos;
    }

    bool generateVideo = false;
    if (videoStreams.isEmpty() && request.url().hasQueryItem("addvideo"))
    {
      SSize size(352, 288);
      if (request.url().hasQueryItem("resolution"))
      {
        const QStringList formatTxt = request.url().queryItemValue("resolution").split(',');

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

    // Set stream properties
    if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
    { // Decode audio and video
      const SAudioCodec audioInCodec = audioStreams.first().codec;
      const SVideoCodec videoInCodec = videoStreams.first().codec;

      if (MediaStream::setup(request, socket,
              input->position(), duration,
              SAudioFormat(SAudioFormat::Format_Invalid, audioInCodec.channelSetup(), audioInCodec.sampleRate()),
              SVideoFormat(SVideoFormat::Format_Invalid, toStandardVideoSize(videoInCodec.size()), videoInCodec.frameRate()),
              false,
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

        timeStampResampler.setFrameRate(video->encoder.codec().frameRate());

        if (audio->outChannels == SAudioFormat::Channels_Stereo)
          audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

        return true;
      }
    }
    else if (!audioStreams.isEmpty() && generateVideo)
    { // Decode audio, generate video
      const SAudioCodec audioInCodec = audioStreams.first().codec;

      const SInterval frameRate = videoGenerator.frameRate();
      timeStampResampler.setFrameRate(frameRate);

      videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Fast;
      videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Slideshow;

      if (MediaStream::setup(request, socket,
              input->position(), duration,
              SAudioFormat(SAudioFormat::Format_Invalid, audioInCodec.channelSetup(), audioInCodec.sampleRate()),
              SVideoFormat(SVideoFormat::Format_Invalid, videoGenerator.image().size(), frameRate),
              enableNormalize,
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

      if (MediaStream::setup(request, socket,
              input->position(), duration,
              SAudioFormat(SAudioFormat::Format_Invalid, audioInCodec.channelSetup(), audioInCodec.sampleRate()),
              enableNormalize))
      {
        connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

        if (audio->outChannels == SAudioFormat::Channels_Stereo)
          audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

        return true;
      }
    }
  }

  return false;
}

} // End of namespace
