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

bool MediaStream::setup(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, STime duration, SInterval frameRate, SSize size, SAudioFormat::Channels channels)
{
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));
  connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&sync, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  connect(&audio->encoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  video = new Video(this);
  connect(&video->deinterlacer, SIGNAL(output(SVideoBuffer)), &video->subpictureRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subpictureRenderer, SIGNAL(output(SVideoBuffer)), &video->letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&video->letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &video->resizer, SLOT(input(SVideoBuffer)));
  connect(&video->resizer, SIGNAL(output(SVideoBuffer)), &video->box, SLOT(input(SVideoBuffer)));
  connect(&video->box, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&video->subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &video->encoder, SLOT(input(SVideoBuffer)));
  connect(&video->encoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpeg"; // Default to mpeg file

  // Set stream properties
  sync.setFrameRate(frameRate);

  Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
  if (url.hasQueryItem("size"))
  {
    const QStringList formatTxt = url.queryItemValue("size").split(',');

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

  if (url.hasQueryItem("channels"))
  {
    if (url.queryItemValue("music") == "true")
    {
      channels = SAudioFormat::Channels(url.queryItemValue("channels").toUInt(NULL, 16));
    }
    else
    {
      const SAudioFormat::Channels c =
          SAudioFormat::Channels(url.queryItemValue("channels").toUInt(NULL, 16));

      if ((SAudioFormat::numChannels(c) > 0) &&
          (SAudioFormat::numChannels(channels) > SAudioFormat::numChannels(c)))
      {
        channels = c;
      }
    }
  }

  SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
  SInterfaces::VideoEncoder::Flags videoEncodeFlags = SInterfaces::VideoEncoder::Flag_Fast;
  if (url.queryItemValue("encode") == "slow")
  {
    audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;
    videoEncodeFlags = SInterfaces::VideoEncoder::Flag_None;
    video->resizer.setHighQuality(true);
  }
  else
    video->resizer.setHighQuality(false);

  videoEncodeFlags |= SInterfaces::VideoEncoder::Flag_HardBitrateLimit;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg") || (file.last().toLower() == "ts"))
  {
    if (SAudioFormat::numChannels(channels) <= 2)
    {
      const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channels_Stereo, 48000);
      audio->matrix.setChannels(audioOutCodec.channelSetup());
      audio->resampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else if (SAudioFormat::numChannels(channels) == 4)
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channels_Quadraphonic, 48000);
      audio->matrix.setChannels(audioOutCodec.channelSetup());
      audio->resampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channels_Surround_5_1, 48000);
      audio->matrix.setChannels(audioOutCodec.channelSetup());
      audio->resampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    if (!video->encoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!video->encoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    if (file.last().toLower() != "ts")
    { // Program stream
      output.openFormat("vob", audio->encoder.codec(), video->encoder.codec(), duration);
      header.setContentType("video/MP2P");
    }
    else
    { // Transport stream
      output.openFormat("mpegts", audio->encoder.codec(), video->encoder.codec(), duration);
      header.setContentType("video/MP2T");
    }
  }
  else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
  {
    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channels_Stereo, 44100);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audio->encoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("THEORA", size, frameRate);
    if (!video->encoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audio->encoder.codec(), video->encoder.codec(), duration);
    header.setContentType("video/ogg");
  }
  else if (file.last().toLower() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
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
    qDebug() << "Incorrect video format:" << file.last();
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

bool MediaStream::setup(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, STime duration, SAudioFormat::Channels channels)
{
  audio = new Audio(this);
  connect(&audio->matrix, SIGNAL(output(SAudioBuffer)), &audio->resampler, SLOT(input(SAudioBuffer)));
  connect(&audio->resampler, SIGNAL(output(SAudioBuffer)), &audio->encoder, SLOT(input(SAudioBuffer)));
  connect(&audio->encoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpa"; // Default to mpeg file

  // Set stream properties
  if (url.hasQueryItem("requestchannels"))
  {
    const SAudioFormat::Channels c =
        SAudioFormat::Channels(url.queryItemValue("requestchannels").toUInt(NULL, 16));

    if ((SAudioFormat::numChannels(c) > 0) &&
        (SAudioFormat::numChannels(channels) > SAudioFormat::numChannels(c)))
    {
      channels = c;
    }
  }
  else if (url.hasQueryItem("forcechannels"))
    channels = SAudioFormat::Channels(url.queryItemValue("forcechannels").toUInt(NULL, 16));

  SInterfaces::AudioEncoder::Flags audioEncodeFlags = SInterfaces::AudioEncoder::Flag_Fast;
  if (url.queryItemValue("encode") == "slow")
    audioEncodeFlags = SInterfaces::AudioEncoder::Flag_None;

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if (file.last().toLower() == "mpa")
  {
    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channels_Stereo, 48000);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.openFormat("mp2", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/mpeg");
  }
  else if (file.last().toLower() == "mp3")
  {
    const SAudioCodec audioOutCodec("MP3", SAudioFormat::Channels_Stereo, 48000);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.openFormat("mp3", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/mp3");
  }
  else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "oga"))
  {
    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channels_Stereo, 44100);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/ogg");
  }
  else if (file.last().toLower() == "lpcm")
  {
    const SAudioCodec audioOutCodec("PCM/S16BE", SAudioFormat::Channels_Stereo, 48000);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("s16be", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/L16;rate=48000;channels=2");
  }
  else if (file.last().toLower() == "wav")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("wav", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/wave");
  }
  else if (file.last().toLower() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channels_Stereo, 44100);
    audio->matrix.setChannels(audioOutCodec.channelSetup());
    audio->resampler.setSampleRate(audioOutCodec.sampleRate());
    audio->encoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audio->encoder.codec(), SVideoCodec(), duration);
    header.setContentType("video/x-flv");
  }
  else
  {
    qDebug() << "Incorrect audio format:" << file.last();
    return false;
  }

  connect(socket, SIGNAL(disconnected()), SLOT(stop()));
  socket->write(header);
  socket->flush();
  output.setIODevice(socket, true);

  qDebug() << "Started audio stream"
      << SAudioFormat::channelSetupName(audio->encoder.codec().channelSetup())
      << audio->encoder.codec().sampleRate() << audio->encoder.codec().codec()
      << header.contentType();

  return true;
}


MediaTranscodeStream::MediaTranscodeStream(void)
  : MediaStream(),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this),
    timeStampResampler(this)
{
}

bool MediaTranscodeStream::setup(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, SInterfaces::BufferReaderNode *input, STime duration)
{
  const QUrl url(request.path());

  // Select streams
  const QList<SIOInputNode::AudioStreamInfo> audioStreams = input->audioStreams();
  const QList<SIOInputNode::VideoStreamInfo> videoStreams = input->videoStreams();
  const QList<SIOInputNode::DataStreamInfo>  dataStreams  = input->dataStreams();

  QList<SIOInputNode::StreamId> selectedStreams;
  if (url.hasQueryItem("language"))
    selectedStreams += url.queryItemValue("language").toUInt(NULL, 16);
  else if (!audioStreams.isEmpty())
    selectedStreams += audioStreams.first();

  if (!videoStreams.isEmpty())
    selectedStreams += videoStreams.first();

  if (url.hasQueryItem("subtitles"))
  {
    if (!url.queryItemValue("subtitles").isEmpty())
      selectedStreams += url.queryItemValue("subtitles").toUInt(NULL, 16);
  }
  else if (!dataStreams.isEmpty())
    selectedStreams += dataStreams.first();

  input->selectStreams(selectedStreams);

  // Seek to start
  if (!duration.isValid())
    duration = input->duration();

  if (url.hasQueryItem("position"))
  {
    const STime pos = STime::fromSec(url.queryItemValue("position").toInt());
    input->setPosition(pos);

    if (duration > pos)
      duration -= pos;
    else
      duration = STime::null;
  }

  if (url.hasQueryItem("starttime"))
  {
    const STime pos = STime::fromSec(url.queryItemValue("starttime").toInt());
    sync.setStartTime(pos);

    if (duration.isPositive())
      duration += pos;
  }

  // Set stream properties
  if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;
    const SVideoCodec videoInCodec = videoStreams.first().codec;

    const SInterval roundedFrameRate = STimeStampResamplerNode::roundFrameRate(videoInCodec.frameRate());
    const STime roundedDuration = duration * (roundedFrameRate.toFrequency() / videoInCodec.frameRate().toFrequency());

    timeStampResampler.setFrameRate(roundedFrameRate);

    if (MediaStream::setup(request, socket, roundedDuration,
                           roundedFrameRate, videoInCodec.size(),
                           audioInCodec.channelSetup()))
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

      if (audio->matrix.channels() == SAudioFormat::Channels_Stereo)
      {
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
        audio->matrix.setDefaultMatrix(SAudioMatrixNode::Matrix_SurroundToStereo);
      }

      return true;
    }
  }

  if (!audioStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    if (MediaStream::setup(request, socket, duration,
                           audioInCodec.channelSetup()))
    {
      connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

      if (audio->matrix.channels() == SAudioFormat::Channels_Stereo)
      {
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);
        audio->matrix.setDefaultMatrix(SAudioMatrixNode::Matrix_SurroundToStereo);
      }

      return true;
    }
  }

  return false;
}

} // End of namespace
