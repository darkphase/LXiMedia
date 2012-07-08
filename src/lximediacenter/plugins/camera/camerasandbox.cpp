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

#include "camerasandbox.h"
#include <iostream>

namespace LXiMediaCenter {
namespace CameraBackend {

const char  * const CameraSandbox::path = "/camera/";
const QEvent::Type  CameraSandbox::probeResponseEventType = QEvent::Type(QEvent::registerEventType());

CameraSandbox::CameraSandbox(const QString &, QObject *parent)
  : BackendSandbox(parent),
    server(NULL)
{
  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

void CameraSandbox::initialize(SSandboxServer *server)
{
  this->server = server;

  BackendSandbox::initialize(server);

  server->registerCallback(path, this);
}

void CameraSandbox::close(void)
{
  BackendSandbox::close();

  server->unregisterCallback(this);
}

SSandboxServer::ResponseMessage CameraSandbox::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const QUrl url(request.path());

  if (request.method() == "GET")
  {
    if (url.hasQueryItem("listcameras"))
    {
      QByteArray content;
      {
        QXmlStreamWriter writer(&content);
        writer.setAutoFormatting(false);
        writer.writeStartElement("cameras");

        foreach (const QString &camera, SAudioVideoInputNode::devices())
        {
          writer.writeStartElement("camera");
          writer.writeCharacters(camera);
          writer.writeEndElement();
        }

        writer.writeEndElement();
      }

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok, content, SHttpEngine::mimeTextXml);
    }
    else if (url.hasQueryItem("opencamera"))
    {
      const QString device = QString::fromUtf8(QByteArray::fromBase64(url.queryItemValue("device").toAscii()));
      if (!device.isEmpty())
      {
        SandboxRecordStream * const recordStream = new SandboxRecordStream(device);
        if (recordStream->setup(request))
        if (recordStream->start())
        {
          SandboxPlaybackStream * const playbackStream = new SandboxPlaybackStream(device);
          if (playbackStream->setup(request, socket))
          if (playbackStream->start())
          {
            streams.append(recordStream);
            streams.append(playbackStream);
            return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
          }

          delete playbackStream;
          recordStream->stop();
        }

        delete recordStream;
      }
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void CameraSandbox::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET,POST");
}

void CameraSandbox::cleanStreams(void)
{
  for (QList<SGraph *>::Iterator i=streams.begin(); i!=streams.end(); )
  if (!(*i)->isRunning())
  {
    delete *i;
    i = streams.erase(i);
  }
  else
    i++;
}


SandboxRecordStream::SandboxRecordStream(const QString &device)
  : SGraph(),
    input(this, device),
    audioEncoder(this),
    videoEncoder(this),
    output(this, QUrl("file:/data/test.mpeg"))
{
}

SandboxRecordStream::~SandboxRecordStream()
{
}

bool SandboxRecordStream::setup(const SHttpServer::RequestMessage &)
{
  connect(&input, SIGNAL(output(SAudioBuffer)), &audioEncoder, SLOT(input(SAudioBuffer)));
  connect(&input, SIGNAL(output(SVideoBuffer)), &videoEncoder, SLOT(input(SVideoBuffer)));

  connect(&audioEncoder, SIGNAL(output(SEncodedAudioBuffer)), &output, SLOT(input(SEncodedAudioBuffer)));
  connect(&videoEncoder, SIGNAL(output(SEncodedVideoBuffer)), &output, SLOT(input(SEncodedVideoBuffer)));

  const SAudioFormat audioFormat(
        SAudioFormat::Format_PCM_S16,
        SAudioFormat::Channels_Mono, 44100);

  const SVideoFormat videoFormat(
        SVideoFormat::Format_Invalid,
        SSize(640, 480), SInterval::fromFrequency(25));

  input.setFormat(audioFormat, videoFormat);

  if (!output.openFormat("vob"))
  {
    qWarning() << "Could not open file format: \"vob\"";
    return false;
  }

  if (!audioEncoder.openCodec(
        SAudioCodec("mp2", audioFormat.channelSetup(), audioFormat.sampleRate()),
        &output, STime(), ::LXiStream::SInterfaces::AudioEncoder::Flag_Fast))
  {
    qWarning() << "Could not open audio codec: \"mp2\"";
    return false;
  }

  if (!videoEncoder.openCodec(
        SVideoCodec("mpeg2video", videoFormat.size(), videoFormat.frameRate()),
        &output, STime(), ::LXiStream::SInterfaces::VideoEncoder::Flag_Fast))
  {
    qWarning() << "Could not open video codec: \"mpeg2video\"";
    return false;
  }

  return true;
}


SandboxPlaybackStream::SandboxPlaybackStream(const QString &)
  : MediaStream(),
    file(this, QUrl("file:/data/test.mpeg")),
    audioDecoder(this),
    videoDecoder(this),
    timeStampResampler(this)
{
  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &timeStampResampler, SLOT(input(SAudioBuffer)));

  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &timeStampResampler, SLOT(input(SVideoBuffer)));

  file.enableTimeShift();
}

SandboxPlaybackStream::~SandboxPlaybackStream()
{
}

bool SandboxPlaybackStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (MediaStream::setup(
        request, socket, STime::null, STime(),
        SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Mono, 44100),
        SVideoFormat(SVideoFormat::Format_Invalid, SSize(640, 480), SInterval::fromFrequency(25))))
  {
    connect(&timeStampResampler, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
    connect(&timeStampResampler, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

    timeStampResampler.setFrameRate(video->encoder.codec().frameRate());

    return true;
  }

  return false;
}

} } // End of namespaces
