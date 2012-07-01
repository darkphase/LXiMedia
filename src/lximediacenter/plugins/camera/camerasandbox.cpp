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
        SandboxInputStream * const stream = new SandboxInputStream(device);
        if (stream->setup(request, socket))
        if (stream->start())
        {
          streams.append(stream);
          return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
        }

        delete stream;
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
  for (QList<MediaStream *>::Iterator i=streams.begin(); i!=streams.end(); )
  if (!(*i)->isRunning())
  {
    delete *i;
    i = streams.erase(i);
  }
  else
    i++;
}


SandboxInputStream::SandboxInputStream(const QString &device)
  : MediaStream(),
    input(this, device)
{
}

SandboxInputStream::~SandboxInputStream()
{
}

bool SandboxInputStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  const bool result = MediaStream::setup(
        request, socket,
        STime(), STime(),
        SAudioFormat(SAudioFormat::Format_PCM_S16, SAudioFormat::Channels_Stereo, 44100),
        SVideoFormat(SVideoFormat::Format_Invalid, SSize(640, 480), SInterval::fromFrequency(25)));

  input.setFormat(
        SAudioFormat(
          SAudioFormat::Format_PCM_S16,
          audio->encoder.codec().channelSetup(),
          audio->encoder.codec().sampleRate()),
        SVideoFormat(
          SVideoFormat::Format_Invalid,
          video->encoder.codec().size(),
          video->encoder.codec().frameRate()));

  connect(&input, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
  connect(&input, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

  return result;
}

} } // End of namespaces
