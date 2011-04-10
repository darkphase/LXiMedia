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
    timeStampResampler(this),
    audioResampler(this, "linear"),
    deinterlacer(this),
    subpictureRenderer(this),
    letterboxDetectNode(this),
    videoResizer(this),
    videoBox(this),
    subtitleRenderer(this),
    sync(this),
    audioEncoder(this),
    videoEncoder(this),
    output(this)
{
  connect(&output, SIGNAL(disconnected()), SLOT(stop()));

  // Audio
  connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audioResampler, SLOT(input(SAudioBuffer)));
  connect(&audioResampler, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&sync, SIGNAL(output(SAudioBuffer)), &audioEncoder, SLOT(input(SAudioBuffer)));
  connect(&audioEncoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));

  // Video
  connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &deinterlacer, SLOT(input(SVideoBuffer)));
  connect(&deinterlacer, SIGNAL(output(SVideoBuffer)), &subpictureRenderer, SLOT(input(SVideoBuffer)));
  connect(&subpictureRenderer, SIGNAL(output(SVideoBuffer)), &letterboxDetectNode, SLOT(input(SVideoBuffer)));
  connect(&letterboxDetectNode, SIGNAL(output(SVideoBuffer)), &videoResizer, SLOT(input(SVideoBuffer)));
  connect(&videoResizer, SIGNAL(output(SVideoBuffer)), &videoBox, SLOT(input(SVideoBuffer)));
  connect(&videoBox, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
  connect(&subtitleRenderer, SIGNAL(output(SVideoBuffer)), &sync, SLOT(input(SVideoBuffer)));
  connect(&sync, SIGNAL(output(SVideoBuffer)), &videoEncoder, SLOT(input(SVideoBuffer)));
  connect(&videoEncoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  // Data
  connect(&timeStampResampler, SIGNAL(output(SSubpictureBuffer)), &subpictureRenderer, SLOT(input(SSubpictureBuffer)));
  connect(&timeStampResampler, SIGNAL(output(SSubtitleBuffer)), &subtitleRenderer, SLOT(input(SSubtitleBuffer)));
}

MediaStream::~MediaStream()
{
  qDebug() << "Stopped stream";
}

bool MediaStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket, STime duration, SInterval frameRate, SSize size, SAudioFormat::Channels channels)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpeg"; // Default to mpeg file

  // Stream priority
  if (url.queryItemValue("priority") == "low")
    setPriority(Priority_Low);
  else if (url.queryItemValue("priority") == "high")
    setPriority(Priority_High);

  // Set stream properties
  const double freq = frameRate.toFrequency();
  const double d15 = freq - 15.0;
  const double d24 = freq - 24.0;
  const double d25 = freq - 25.0;
  const double d30 = freq - 30.0;
  const double d50 = freq - 50.0;
  const double d60 = freq - 60.0;

  if ((d15 > -3.0) && (d15 < 3.0))
  {
    duration *= (15.0 / freq);
    frameRate = SInterval::fromFrequency(15);
  }
  else if ((d24 > -2.0) && (d24 < 0.6))
  {
    duration *= (24.0 / freq);
    frameRate = SInterval::fromFrequency(24);
  }
  else if ((d25 > -2.0) && (d25 < 2.1))
  {
    duration *= (25.0 / freq);
    frameRate = SInterval::fromFrequency(25);
  }
  else if ((d30 > -3.0) && (d30 < 4.0))
  {
    duration *= (30.0 / freq);
    frameRate = SInterval::fromFrequency(30);
  }
  else if ((d50 > -5.0) && (d50 < 5.0))
  {
    duration *= (50.0 / freq);
    frameRate = SInterval::fromFrequency(50);
  }
  else if ((d60 > -5.0) && (d60 < 5.0))
  {
    duration *= (60.0 / freq);
    frameRate = SInterval::fromFrequency(60);
  }
  else
    frameRate = SInterval::fromFrequency(25);

  timeStampResampler.setFrameRate(frameRate);
  sync.setFrameRate(frameRate);

  Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio;
  if (url.hasQueryItem("size"))
  {
    const QStringList formatTxt = url.queryItemValue("size").split('/');

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
    videoResizer.setHighQuality(true);
  }
  else
    videoResizer.setHighQuality(false);

  SHttpServer::ResponseHeader header(request, SHttpServer::Status_Ok);
  header.setField("Cache-Control", "no-cache");

  if ((file.last().toLower() == "mpeg") || (file.last().toLower() == "mpg") || (file.last().toLower() == "ts"))
  {
    if (SAudioFormat::numChannels(channels) <= 2)
    {
      const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else if (SAudioFormat::numChannels(channels) == 4)
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Quadraphonic, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }
    else
    {
      const SAudioCodec audioOutCodec("AC3", SAudioFormat::Channel_Surround_5_1, 48000);
      audioResampler.setChannels(audioOutCodec.channelSetup());
      audioResampler.setSampleRate(audioOutCodec.sampleRate());
      if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
        return false;
    }

    if (!videoEncoder.openCodec(SVideoCodec("MPEG2", size, frameRate), videoEncodeFlags))
    if (!videoEncoder.openCodec(SVideoCodec("MPEG1", size, frameRate), videoEncodeFlags))
      return false;

    if (file.last().toLower() != "ts")
    { // Program stream
      output.openFormat("vob", audioEncoder.codec(), videoEncoder.codec(), duration);
      header.setContentType("video/MP2P");
    }
    else
    { // Transport stream
      output.openFormat("mpegts", audioEncoder.codec(), videoEncoder.codec(), duration);
      header.setContentType("video/MP2T");
    }
  }
  else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "ogv"))
  {
    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("THEORA", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audioEncoder.codec(), videoEncoder.codec(), duration);
    header.setContentType("video/ogg");
  }
  else if (file.last().toLower() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    if (!audioEncoder.openCodec(audioOutCodec, audioEncodeFlags))
      return false;

    const SVideoCodec videoOutCodec("FLV1", size, frameRate);
    if (!videoEncoder.openCodec(videoOutCodec, videoEncodeFlags))
      return false;

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audioEncoder.codec(), videoEncoder.codec(), duration);
    header.setContentType("video/x-flv");
  }
  else
  {
    qDebug() << "Incorrect video format:" << file.last();
    return false;
  }

  videoResizer.setSize(size);
  videoResizer.setAspectRatioMode(aspectRatioMode);
  videoBox.setSize(size);

  socket->write(header);
  output.setIODevice(socket, true);

  //enableTrace("/tmp/test.svg");

  qDebug() << "Started video stream"
      << size.width() << "x" << size.height()
      << "@" << frameRate.toFrequency() << videoEncoder.codec().codec()
      << SAudioFormat::channelSetupName(audioEncoder.codec().channelSetup())
      << audioEncoder.codec().sampleRate() << audioEncoder.codec().codec()
      << header.contentType();

  return true;
}

bool MediaStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket, STime duration, SAudioFormat::Channels channels)
{
  QUrl url(request.path());
  if (url.hasQueryItem("query"))
    url = url.toEncoded(QUrl::RemoveQuery) + QByteArray::fromHex(url.queryItemValue("query").toAscii());

  QStringList file = request.file().split('.');
  if (file.count() <= 1)
    file += "mpa"; // Default to mpeg file

  // Stream priority
  if (url.queryItemValue("priority") == "low")
    setPriority(Priority_Low);
  else if (url.queryItemValue("priority") == "high")
    setPriority(Priority_High);

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
    const SAudioCodec audioOutCodec("MP2", SAudioFormat::Channel_Stereo, 48000);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.openFormat("mp2", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/mpeg");
  }
  else if (file.last().toLower() == "mp3")
  {
    const SAudioCodec audioOutCodec("MP3", SAudioFormat::Channel_Stereo, 48000);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.openFormat("mp3", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/mp3");
  }
  else if ((file.last().toLower() == "ogg") || (file.last().toLower() == "oga"))
  {
    const SAudioCodec audioOutCodec("FLAC", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("ogg", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/ogg");
  }
  else if (file.last().toLower() == "lpcm")
  {
    const SAudioCodec audioOutCodec("PCM/S16BE", SAudioFormat::Channel_Stereo, 48000);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("s16be", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/L16;rate=48000;channels=2");
  }
  else if (file.last().toLower() == "wav")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("wav", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("audio/wave");
  }
  else if (file.last().toLower() == "flv")
  {
    const SAudioCodec audioOutCodec("PCM/S16LE", SAudioFormat::Channel_Stereo, 44100);
    audioResampler.setChannels(audioOutCodec.channelSetup());
    audioResampler.setSampleRate(audioOutCodec.sampleRate());
    audioEncoder.openCodec(audioOutCodec, audioEncodeFlags);

    output.enablePseudoStreaming(1.1f);
    output.openFormat("flv", audioEncoder.codec(), SVideoCodec(), duration);
    header.setContentType("video/x-flv");
  }
  else
  {
    qDebug() << "Incorrect audio format:" << file.last();
    return false;
  }

  socket->write(header);
  output.setIODevice(socket, true);

  //enableTrace("/tmp/test.svg");

  qDebug() << "Started audio stream"
      << SAudioFormat::channelSetupName(audioEncoder.codec().channelSetup())
      << audioEncoder.codec().sampleRate() << audioEncoder.codec().codec()
      << header.contentType();

  return true;
}


MediaTranscodeStream::MediaTranscodeStream(void)
  : MediaStream(),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this)
{
  // Audio
  connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));

  // Video
  connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));

  // Data
  connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &timeStampResampler, SLOT(input(SSubpictureBuffer)));
  connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &timeStampResampler, SLOT(input(SSubtitleBuffer)));
}

bool MediaTranscodeStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket, SInterfaces::BufferReaderNode *input, STime duration)
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

  // Set stream properties
  if (!audioStreams.isEmpty() && !videoStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;
    const SVideoCodec videoInCodec = videoStreams.first().codec;

    if (MediaStream::setup(request, socket,
                      duration,
                      videoInCodec.frameRate(), videoInCodec.size(),
                      audioInCodec.channelSetup()))
    {
      if (audioResampler.channels() == SAudioFormat::Channel_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }

  if (!audioStreams.isEmpty())
  {
    const SAudioCodec audioInCodec = audioStreams.first().codec;

    if (MediaStream::setup(request, socket,
                      duration,
                      audioInCodec.channelSetup()))
    {
      if (audioResampler.channels() == SAudioFormat::Channel_Stereo)
        audioDecoder.setFlags(SInterfaces::AudioDecoder::Flag_DownsampleToStereo);

      return true;
    }
  }

  return false;
}

} // End of namespace
