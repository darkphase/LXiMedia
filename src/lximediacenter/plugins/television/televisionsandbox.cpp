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

#include "televisionsandbox.h"
#include <iostream>

namespace LXiMediaCenter {
namespace TelevisionBackend {

const char  * const TelevisionSandbox::path = "/television/";
const QEvent::Type  TelevisionSandbox::probeResponseEventType = QEvent::Type(QEvent::registerEventType());

TelevisionSandbox::TelevisionSandbox(const QString &, QObject *parent)
  : BackendSandbox(parent),
    server(NULL)
{
  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

void TelevisionSandbox::initialize(SSandboxServer *server)
{
  this->server = server;

  BackendSandbox::initialize(server);

  server->registerCallback(path, this);
}

void TelevisionSandbox::close(void)
{
  BackendSandbox::close();

  server->unregisterCallback(this);
}

SSandboxServer::SocketOp TelevisionSandbox::handleHttpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const QUrl url(request.path());

  if (request.method() == "GET")
  {
    if (url.hasQueryItem("opencamera"))
    {
      const QString device = QString::fromUtf8(QByteArray::fromBase64(url.queryItemValue("device").toAscii()));
      if (!device.isEmpty())
      {
        SandboxInputStream * const stream = new SandboxInputStream(device);
        if (stream->setup(request, socket))
        if (stream->start())
        {
          streams.append(stream);
          return SSandboxServer::SocketOp_LeaveOpen;
        }

        delete stream;
      }

      return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_InternalServerError, this);
    }
  }

  return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_NotFound, this);
}

void TelevisionSandbox::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET,POST");
}

void TelevisionSandbox::cleanStreams(void)
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
  const bool result = MediaStream::setup(request, socket, STime(), SInterval::fromFrequency(15), SSize(352, 288), SAudioFormat::Channels_Stereo);

  connect(&input, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
  connect(&input, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

  return result;
}

} } // End of namespaces
