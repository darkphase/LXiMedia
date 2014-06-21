/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
    output(this),
    proxy()
{
}

MediaStream::~MediaStream()
{
  stop();

  delete audio;
  delete video;

  qDebug() << "Stopped stream";
}

bool MediaStream::setup(const QUrl &request,
                        STime duration,
                        const SAudioFormat &inputAudioFormat,
                        const SVideoFormat &inputVideoFormat,
                        bool enableNormalize,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags,
                        SInterfaces::VideoEncoder::Flags videoEncodeFlags,
                        int videoGopSize)
{
  const QUrlQuery query(request);

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

  decodeSize(request, videoFormat, aspectRatioMode);

  decodeChannels(request, audioFormat);

  if (query.hasQueryItem("subtitlesize"))
    video->subtitleRenderer.setFontSize(query.queryItemValue("subtitlesize").toFloat());

  if (query.queryItemValue("encodemode") == "fast")
  {
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;
    videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_Fast;
    video->resizer.setHighQuality(false);
  }
  else
    video->resizer.setHighQuality(true);

  videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_HardBitrateLimit;

  // Match with DLNA profile
  const QString contentFeatures = QByteArray::fromBase64(query.queryItemValue("contentFeatures").toLatin1());
  const MediaProfiles::VideoProfile videoProfile = MediaServer::mediaProfiles().videoProfileFor(contentFeatures);
  if (videoProfile != 0) // DLNA stream.
  {
    MediaProfiles::correctFormat(videoProfile, audioFormat);
    MediaProfiles::correctFormat(videoProfile, videoFormat);

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

    contentType = MediaProfiles::mimeTypeFor(videoProfile);
  }
  else // Non-DLNA stream.
  {
    SAudioCodec audioCodec;
    SVideoCodec videoCodec;
    QString format = query.queryItemValue("format");

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
      contentType = UPnP::mimeVideoOgg;
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

      contentType = UPnP::mimeVideoFlv;
    }
    else if ((format == "matroska") && SIOOutputNode::formats().contains(format))
    {
      if (SAudioEncoderNode::codecs().contains("mp3"))
        audioCodec = SAudioCodec("mp3", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("mp2", SAudioFormat::Channels_Stereo, 44100);

      videoCodec = SVideoCodec("mpeg4", videoFormat.size(), videoFormat.frameRate());
      videoCodec.setGopSize(videoGopSize);

      contentType = UPnP::mimeVideoMatroska;
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
        contentType = UPnP::mimeVideoMpeg;
      }
      else if (format == "m2ts")
      {
        contentType = UPnP::mimeVideoMpegM2TS;
      }
      else
      {
        format = "vob";
        contentType = UPnP::mimeVideoMpeg;
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

  if (proxy.open(QIODevice::WriteOnly))
  {
    connect(&proxy, SIGNAL(aboutToClose()), SLOT(stop()));

    output.setIODevice(&proxy);
  }

  this->request = request;

  qDebug() << "Started video stream"
      << videoFormat.size().toString()
      << "@" << videoFormat.frameRate().toFrequency() << video->encoder.codec().name()
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().name()
      << contentType;

  return true;
}

bool MediaStream::setup(const QUrl &request,
                        STime duration,
                        const SAudioFormat &inputAudioFormat,
                        bool enableNormalize,
                        SInterfaces::AudioEncoder::Flags audioEncodeFlags)
{
  const QUrlQuery query(request);

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

  decodeChannels(request, audioFormat);

  if (query.queryItemValue("encodemode") == "fast")
    audioEncodeFlags |= SInterfaces::AudioEncoder::Flag_Fast;

  // Match with DLNA profile
  const QString contentFeatures = QByteArray::fromBase64(query.queryItemValue("contentFeatures").toLatin1());
  const MediaProfiles::AudioProfile audioProfile = MediaServer::mediaProfiles().audioProfileFor(contentFeatures);
  if (audioProfile != 0) // DLNA stream.
  {
    MediaProfiles::correctFormat(audioProfile, audioFormat);

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

    contentType = MediaProfiles::mimeTypeFor(audioProfile);
  }
  else // Non-DLNA stream.
  {
    SAudioCodec audioCodec;
    QString format = query.queryItemValue("format");

    if ((format == "adts") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("aac", SAudioFormat::Channels_Stereo, audioFormat.sampleRate());
      contentType = UPnP::mimeAudioAac;
    }
    else if ((format == "ac3") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("ac3", SAudioFormat::Channels_Stereo, audioFormat.sampleRate());
      contentType = UPnP::mimeAudioAc3;
    }
    else if ((format == "mp3") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("mp3", SAudioFormat::Channels_Stereo, 44100);
      contentType = UPnP::mimeAudioMp3;
    }
    else if ((format == "oga") && SIOOutputNode::formats().contains("ogg"))
    {
      if (SAudioEncoderNode::codecs().contains("vorbis"))
        audioCodec = SAudioCodec("vorbis", SAudioFormat::Channels_Stereo, 44100);
      else
        audioCodec = SAudioCodec("flac", SAudioFormat::Channels_Stereo, 44100);

      format = "ogg";
      contentType = UPnP::mimeAudioOgg;
    }
    else if ((format == "s16be") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("pcm_s16be", SAudioFormat::Channels_Stereo, 48000);
      contentType = UPnP::mimeAudioLpcm;
    }
    else if ((format == "wav") && SIOOutputNode::formats().contains(format))
    {
      audioCodec = SAudioCodec("pcm_s16le", SAudioFormat::Channels_Stereo, 44100);
      contentType = UPnP::mimeAudioWave;
    }
    else // Default to mpeg
    {
      audioCodec = SAudioCodec("mp2", SAudioFormat::Channels_Stereo, 44100);
      format = "mp2";
      contentType = UPnP::mimeAudioMpeg;
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

  if (proxy.open(QIODevice::WriteOnly))
  {
    connect(&proxy, SIGNAL(aboutToClose()), SLOT(stop()));

    output.setIODevice(&proxy);
  }

  this->request = request;

  qDebug() << "Started audio stream"
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().name()
      << contentType;

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
  const QUrlQuery query(url);
  if (query.hasQueryItem("resolution"))
  {
	SSize size = videoFormat.size();

    const QStringList formatTxt = query.queryItemValue("resolution").split(',');

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
  const QUrlQuery query(url);
  if (query.hasQueryItem("channels"))
  {
    const SAudioFormat::Channels c =
        SAudioFormat::Channels(query.queryItemValue("channels").toUInt(NULL, 16));

    if ((SAudioFormat::numChannels(c) > 0) &&
        (query.hasQueryItem("music") ||
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

MediaTranscodeStream::~MediaTranscodeStream(void)
{
  stop();
}

bool MediaTranscodeStream::setup(
    const QUrl &request,
    SInputNode *input,
    STime duration,
    bool enableNormalize,
    SInterfaces::AudioEncoder::Flags audioEncodeFlags,
    SInterfaces::VideoEncoder::Flags videoEncodeFlags)
{
  const QUrlQuery query(request);

  int titleId = 0;
  if (query.hasQueryItem("title"))
    titleId = query.queryItemValue("title").toInt();

  if (audioDecoder.open(input) && videoDecoder.open(input) && dataDecoder.open(input))
  {
    // Select streams
    QList<SInputNode::AudioStreamInfo> audioStreams = input->audioStreams(titleId);
    QList<SInputNode::VideoStreamInfo> videoStreams = input->videoStreams(titleId);
    QList<SInputNode::DataStreamInfo>  dataStreams  = input->dataStreams(titleId);

    QVector<SInputNode::StreamId> selectedStreams;
    if (query.hasQueryItem("language"))
    {
      selectedStreams += SInputNode::StreamId::fromString(query.queryItemValue("language"));
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

    if (query.hasQueryItem("subtitles"))
    {
      if (!query.queryItemValue("subtitles").isEmpty())
        selectedStreams += SInputNode::StreamId::fromString(query.queryItemValue("subtitles"));
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

    if (query.hasQueryItem("position"))
    {
      const STime pos = STime::fromSec(query.queryItemValue("position").toInt());
      input->setPosition(pos);

      if (duration > pos)
        duration -= pos;
      else
        duration = STime::null;
    }

    if (query.hasQueryItem("starttime"))
    {
      const STime pos = STime::fromSec(query.queryItemValue("starttime").toInt());
      sync.setStartTime(pos);

      if (duration.isPositive())
        duration += pos;
    }

    bool generateVideo = false;
    if (videoStreams.isEmpty() && query.hasQueryItem("addvideo"))
    {
      SSize size(352, 288);
      if (query.hasQueryItem("resolution"))
      {
        const QStringList formatTxt = query.queryItemValue("resolution").split(',');

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

      if (MediaStream::setup(
            request,
            duration,
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

      if (MediaStream::setup(
            request,
            duration,
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

      if (MediaStream::setup(
            request,
            duration,
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
