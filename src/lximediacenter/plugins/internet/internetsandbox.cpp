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
  if (request.method() == "POST")
  {
    if (request.url().hasQueryItem("playstream"))
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
    url(url),
    streamInput(this),
    audioDecoder(this),
    videoDecoder(this),
    dataDecoder(this),
    videoGenerator(this)
{
}

SandboxNetworkStream::~SandboxNetworkStream()
{
}

bool SandboxNetworkStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (audioDecoder.open(&streamInput) && videoDecoder.open(&streamInput) && dataDecoder.open(&streamInput))
  {
    if (request.url().hasQueryItem("resolution"))
    {
      const QStringList formatTxt = request.url().queryItemValue("resolution").split(',');

      const QStringList sizeTxt = formatTxt.first().split('x');
      if (sizeTxt.count() >= 2)
      {
        SSize size = streamInput.size();
        size.setWidth(sizeTxt[0].toInt());
        size.setHeight(sizeTxt[1].toInt());
        if (sizeTxt.count() >= 3)
          size.setAspectRatio(sizeTxt[2].toFloat());
        else
          size.setAspectRatio(1.0f);

        streamInput.setSize(size);
      }
    }

    if (url.hasQueryItem("channels"))
    {
      const SAudioFormat::Channels c =
          SAudioFormat::Channels(url.queryItemValue("channels").toUInt(NULL, 16));

      if (SAudioFormat::numChannels(c) > 0)
        streamInput.setChannelSetup(c);
    }

    const bool hasVideo = request.url().queryItemValue("music") != "true";
    const bool generateVideo = request.url().queryItemValue("musicmode").startsWith("addvideo");

    streamInput.setUrl(url, hasVideo || generateVideo);
    connect(&streamInput, SIGNAL(finished()), SLOT(stop()));

    if (hasVideo)
    {
      if (MediaStream::setup(
            request, socket,
            STime::null, STime::null,
            SAudioFormat(SAudioFormat::Format_Invalid, streamInput.channelSetup(), streamInput.sampleRate()),
            SVideoFormat(SVideoFormat::Format_Invalid, streamInput.size(), streamInput.frameRate()),
            false))
      {
        connect(&streamInput, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
        connect(&streamInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
        connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

        connect(&streamInput, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));
        connect(&streamInput, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
        connect(&videoDecoder, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

        connect(&streamInput, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
        connect(&dataDecoder, SIGNAL(output(SSubpictureBuffer)), &video->subpictureRenderer, SLOT(input(SSubpictureBuffer)));
        connect(&dataDecoder, SIGNAL(output(SSubtitleBuffer)), &video->subtitleRenderer, SLOT(input(SSubtitleBuffer)));

        return true;
      }
    }
    else if (generateVideo)
    {
      if (MediaStream::setup(
            request, socket,
            STime::null, STime::null,
            SAudioFormat(SAudioFormat::Format_Invalid, streamInput.channelSetup()),
            false))
      {
        connect(&streamInput, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
        connect(&streamInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
        connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

        connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &videoGenerator, SLOT(input(SAudioBuffer)));
        connect(&videoGenerator, SIGNAL(output(SVideoBuffer)), &video->deinterlacer, SLOT(input(SVideoBuffer)));

        return true;
      }
    }
    else
    {
      if (MediaStream::setup(
            request, socket,
            STime::null, STime::null,
            SAudioFormat(SAudioFormat::Format_Invalid, streamInput.channelSetup()),
            false))
      {
        connect(&streamInput, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));
        connect(&streamInput, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
        connect(&audioDecoder, SIGNAL(output(SAudioBuffer)), &audio->matrix, SLOT(input(SAudioBuffer)));

        return true;
      }
    }
  }

  return false;
}

} } // End of namespaces
