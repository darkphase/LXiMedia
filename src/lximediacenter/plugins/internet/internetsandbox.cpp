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

#include "internetsandbox.h"
#include <QtConcurrentRun>
#include <iostream>

namespace LXiMediaCenter {
namespace InternetBackend {

const char  * const InternetSandbox::path = "/internet/";

InternetSandbox::InternetSandbox(const QString &, QObject *parent)
  : BackendSandbox(parent),
    server(NULL)
{
  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

void InternetSandbox::initialize(SSandboxServer *server)
{
  this->server = server;

  BackendSandbox::initialize(server);

  server->registerCallback(path, this);
}

void InternetSandbox::close(void)
{
  BackendSandbox::close();

  server->unregisterCallback(this);

  cleanStreams();
}

SSandboxServer::ResponseMessage InternetSandbox::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if (request.method() == "POST")
  {
    if (file.url().hasQueryItem("playstream"))
    {
      const QUrl url = QString::fromUtf8(request.content());
      if (url.isValid())
      {
        SandboxNetworkStream * const stream = new SandboxNetworkStream(url);
        if (stream->setup(request, socket))
        if (stream->start())
        {
          streams.append(stream);
          return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
        }

        delete stream;
      }

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_InternalServerError);
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void InternetSandbox::cleanStreams(void)
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


SandboxNetworkStream::SandboxNetworkStream(const QUrl &url)
  : MediaStream(),
    source(this, url)
{
  connect(&source, SIGNAL(finished()), SLOT(stop()));
}

SandboxNetworkStream::~SandboxNetworkStream()
{
}

bool SandboxNetworkStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if (file.url().hasQueryItem("resolution"))
  {
    const QStringList formatTxt = file.url().queryItemValue("resolution").split(',');

    const QStringList sizeTxt = formatTxt.first().split('x');
    if (sizeTxt.count() >= 2)
    {
      SSize size = source.size();
      size.setWidth(sizeTxt[0].toInt());
      size.setHeight(sizeTxt[1].toInt());
      if (sizeTxt.count() >= 3)
        size.setAspectRatio(sizeTxt[2].toFloat());
      else
        size.setAspectRatio(1.0f);

      source.setSize(size);
    }
  }

  if (file.url().hasQueryItem("channels"))
  {
    const QStringList cl = file.url().queryItemValue("channels").split(',');

    if ((file.url().queryItemValue("music") == "true") && (cl.count() >= 2))
      source.setChannelSetup(SAudioFormat::Channels(cl[1].toUInt(NULL, 16)));
    else if (!cl.isEmpty())
      source.setChannelSetup(SAudioFormat::Channels(cl[0].toUInt(NULL, 16)));
  }

  const bool hasVideo = file.url().queryItemValue("music") != "true";
  const bool generateVideo = file.url().queryItemValue("musicmode").startsWith("addvideo");

  if (source.open(hasVideo, generateVideo))
  {
    if (hasVideo || generateVideo)
    {
      if (MediaStream::setup(
            request, socket,
            STime::null,
            SAudioFormat(SAudioFormat::Format_Invalid, source.channelSetup()),
            SVideoFormat(SVideoFormat::Format_Invalid, source.size(), source.frameRate()),
            false))
      {
        connect(&source, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
        connect(&source, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));
        connect(&source, SIGNAL(output(SSubpictureBuffer)), &video->subpictureRenderer, SLOT(input(SSubpictureBuffer)));
        connect(&source, SIGNAL(output(SSubtitleBuffer)), &video->subtitleRenderer, SLOT(input(SSubtitleBuffer)));

        return true;
      }
    }
    else
    {
      if (MediaStream::setup(
            request, socket,
            STime::null,
            SAudioFormat(SAudioFormat::Format_Invalid, source.channelSetup()),
            false))
      {
        connect(&source, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

        return true;
      }
    }
  }

  return false;
}

} } // End of namespaces
