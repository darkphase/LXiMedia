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

#include "mediaplayersandbox.h"
#include <QtConcurrentRun>
#include <iostream>
#include "filenode.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char  * const MediaPlayerSandbox::path = "/mediaplayer/";
const QEvent::Type  MediaPlayerSandbox::probeResponseEventType = QEvent::Type(QEvent::registerEventType());

MediaPlayerSandbox::MediaPlayerSandbox(const QString &, QObject *parent)
  : BackendSandbox(parent),
    server(NULL)
{
  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

void MediaPlayerSandbox::initialize(SSandboxServer *server)
{
  this->server = server;

  BackendSandbox::initialize(server);

  server->registerCallback(path, this);
}

void MediaPlayerSandbox::close(void)
{
  BackendSandbox::close();

  server->unregisterCallback(this);

  cleanStreams();
}

SSandboxServer::ResponseMessage MediaPlayerSandbox::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if (request.isGet())
  {
    if (file.url().hasQueryItem("probe"))
    {
      QtConcurrent::run(this, &MediaPlayerSandbox::probe, request, socket, QString::fromUtf8(QByteArray::fromHex(file.url().queryItemValue("probe").toAscii())));

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
  }
  else if (request.isPost())
  {
    if (file.url().hasQueryItem("playfile"))
    {
      const SMediaInfo info = FileNode::fromByteArray(request.content());
      if (!info.isNull())
      {
        SandboxFileStream * const stream = new SandboxFileStream(info.filePath());
        if (stream->file.open(file.url().queryItemValue("pid").toUShort()))
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
    else if (file.url().hasQueryItem("playlist"))
    {
      SMediaInfoList files;
      foreach (const QByteArray &node, request.content().split('\n'))
      if (!node.isEmpty())
        files.append(FileNode::fromByteArray(node));

      SandboxPlaylistStream * const stream = new SandboxPlaylistStream(files);
      if (stream->setup(request, socket))
      if (stream->start())
      {
        streams.append(stream);
        return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
      }

      delete stream;
      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_InternalServerError);
    }
    else if (file.url().hasQueryItem("playslideshow"))
    {
      SMediaInfoList files;
      foreach (const QByteArray &node, request.content().split('\n'))
        files.append(FileNode::fromByteArray(node));

      SandboxSlideShowStream * const stream = new SandboxSlideShowStream(files);
      if (stream->setup(request, socket))
      if (stream->start())
      {
        streams.append(stream);
        return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
      }

      delete stream;
      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_InternalServerError);
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void MediaPlayerSandbox::customEvent(QEvent *e)
{
  if (e->type() == probeResponseEventType)
  {
    const ProbeResponseEvent * const event = static_cast<ProbeResponseEvent *>(e);

    SHttpServer::ResponseMessage response =
        event->data.isEmpty()
        ? SHttpServer::ResponseMessage(event->request, SSandboxServer::Status_NotFound)
        : SHttpServer::ResponseMessage(event->request, SSandboxServer::Status_Ok, event->data, SHttpEngine::mimeTextXml);

    SSandboxServer::sendHttpResponse(event->request, response, event->socket, false);
  }
  else
    BackendSandbox::customEvent(e);
}

void MediaPlayerSandbox::probe(const SSandboxServer::RequestMessage &request, QIODevice *socket, const QString &file)
{
  qDebug() << "Probing:" << file;

  FileNode fileNode(file);
  if (!fileNode.isNull())
    qApp->postEvent(this, new ProbeResponseEvent(request, socket, fileNode.toByteArray(-1)));
  else
    qApp->postEvent(this, new ProbeResponseEvent(request, socket, QByteArray()));
}

void MediaPlayerSandbox::cleanStreams(void)
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


SandboxFileStream::SandboxFileStream(const QString &fileName)
  : MediaTranscodeStream(),
    file(this, fileName)
{
  connect(&file, SIGNAL(finished()), SLOT(stop()));

  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

  // Mark as played:
  std::cerr << ("#PLAYED:" + fileName.toUtf8().toHex()).data() << std::endl;
}

SandboxFileStream::~SandboxFileStream()
{
}

bool SandboxFileStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  return MediaTranscodeStream::setup(request, socket, &file);
}


SandboxPlaylistStream::SandboxPlaylistStream(const SMediaInfoList &files)
  : MediaTranscodeStream(),
    playlistNode(this, files)
{
}

bool SandboxPlaylistStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if (MediaTranscodeStream::setup(
      request, socket,
      &playlistNode,
      STime(),
      file.url().queryItemValue("music") == "true"))
  {
    connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
    connect(&playlistNode, SIGNAL(opened(QString, quint16)), SLOT(opened(QString, quint16)));
    connect(&playlistNode, SIGNAL(closed(QString, quint16)), SLOT(closed(QString, quint16)));
    connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

    return true;
  }

  return false;
}

void SandboxPlaylistStream::opened(const QString &filePath, quint16 /*programId*/)
{
  // Mark as played:
  std::cerr << ("#PLAYED:" + filePath.toUtf8().toHex()).data() << std::endl;

  currentFile = filePath;
}

void SandboxPlaylistStream::closed(const QString &filePath, quint16 /*programId*/)
{
  if (currentFile == filePath)
    currentFile = QString::null;
}


SandboxSlideShowStream::SandboxSlideShowStream(const SMediaInfoList &files)
  : MediaStream(),
    slideShow(this, files)
{
}

bool SandboxSlideShowStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  const MediaServer::File file(request);

  if (MediaStream::setup(
          request, socket,
          STime::null, slideShow.duration(),
          SAudioFormat(SAudioFormat::Format_Invalid, SAudioFormat::Channels_Stereo, 48000),
          SVideoFormat(SVideoFormat::Format_Invalid, slideShow.size(), SInterval::fromFrequency(slideShow.frameRate)),
          false,
          SInterfaces::AudioEncoder::Flag_None,
          SInterfaces::VideoEncoder::Flag_Slideshow))
  {
    connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
    connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&slideShow, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));

    slideShow.setSize(video->resizer.size());

    if (file.url().hasQueryItem("slideduration"))
      slideShow.setSlideDuration(STime::fromMSec(file.url().queryItemValue("slideduration").toInt()));

    return true;
  }
  else
    return false;
}

} } // End of namespaces
