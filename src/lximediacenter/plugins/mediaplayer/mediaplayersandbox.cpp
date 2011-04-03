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
#include <iostream>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char  * const MediaPlayerSandbox::path = "/mediaplayer/";

MediaPlayerSandbox::MediaPlayerSandbox(const QString &, QObject *parent)
  : BackendSandbox(parent),
    server(NULL),
    mutex(QMutex::Recursive)
{
  connect(&cleanStreamsTimer, SIGNAL(timeout()), SLOT(cleanStreams()));
  cleanStreamsTimer.start(5000);
}

void MediaPlayerSandbox::initialize(SSandboxServer *server)
{
  this->server = server;

  BackendSandbox::initialize(server);

  server->registerCallback("/mediaplayer/", this);
}

void MediaPlayerSandbox::close(void)
{
  BackendSandbox::close();

  server->unregisterCallback(this);
}

SSandboxServer::SocketOp MediaPlayerSandbox::handleHttpRequest(const SSandboxServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());

  if (url.hasQueryItem("probe"))
  {
    const QString file = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("probe").toAscii()));

    SMediaInfo mediaInfo(file);
    if (!mediaInfo.isNull())
      return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_Ok, mediaInfo.toByteArray(-1), this);
  }
  else if (url.hasQueryItem("playfile"))
  {
    SMediaInfo file;
    file.fromByteArray(SHttpServer::readContent(request, socket));

    SandboxFileStream * const stream = new SandboxFileStream(file.filePath());
    if (stream->file.open(url.queryItemValue("pid").toUShort()))
    if (stream->setup(request, socket))
    if (stream->start())
    {
      QMutexLocker l(&mutex);

      streams.append(stream);
      return SSandboxServer::SocketOp_LeaveOpen;
    }

    delete stream;
    return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_InternalServerError, this);
  }
  else if (url.hasQueryItem("playlist"))
  {
    SMediaInfoList files;
    foreach (const QByteArray &node, SHttpServer::readContent(request, socket).split('\n'))
    {
      SMediaInfo file;
      file.fromByteArray(node);

      files.append(file);
    }

    SandboxPlaylistStream * const stream = new SandboxPlaylistStream(files);
    if (stream->setup(request, socket))
    if (stream->start())
    {
      QMutexLocker l(&mutex);

      streams.append(stream);
      return SSandboxServer::SocketOp_LeaveOpen;
    }

    delete stream;
    return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_InternalServerError, this);
  }
  else if (url.hasQueryItem("playslideshow"))
  {
    SMediaInfoList files;
    foreach (const QByteArray &node, SHttpServer::readContent(request, socket).split('\n'))
    {
      SMediaInfo file;
      file.fromByteArray(node);

      files.append(file);
    }

    SandboxSlideShowStream * const stream = new SandboxSlideShowStream(files);
    if (stream->setup(request, socket))
    if (stream->start())
    {
      QMutexLocker l(&mutex);

      streams.append(stream);
      return SSandboxServer::SocketOp_LeaveOpen;
    }

    delete stream;
    return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_InternalServerError, this);
  }

  return SSandboxServer::sendResponse(request, socket, SSandboxServer::Status_NotFound, this);
}

void MediaPlayerSandbox::cleanStreams(void)
{
  if (mutex.tryLock(0))
  {
    for (QList<MediaStream *>::Iterator i=streams.begin(); i!=streams.end(); )
    if (!(*i)->isRunning())
    {
      delete *i;
      i = streams.erase(i);
    }
    else
      i++;

    mutex.unlock();
  }
}


SandboxFileStream::SandboxFileStream(const QString &fileName)
  : MediaTranscodeStream(),
    fileName(fileName),
    startTime(QDateTime::currentDateTime()),
    file(this, fileName)
{
  connect(&file, SIGNAL(finished()), SLOT(stop()));

  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
}

SandboxFileStream::~SandboxFileStream()
{
  // Mark as played:
  if (startTime.secsTo(QDateTime::currentDateTime()) >= 120)
    std::cerr << ('%' + fileName.toUtf8().toHex()).data() << std::endl;
}

bool SandboxFileStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  return MediaTranscodeStream::setup(request, socket, &file);
}


SandboxPlaylistStream::SandboxPlaylistStream(const SMediaInfoList &files)
  : MediaTranscodeStream(),
    playlistNode(this, files)
{
  connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
  connect(&playlistNode, SIGNAL(opened(QString, quint16)), SLOT(opened(QString, quint16)));
  connect(&playlistNode, SIGNAL(closed(QString, quint16)), SLOT(closed(QString, quint16)));
  connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));
}

bool SandboxPlaylistStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  return MediaTranscodeStream::setup(request, socket, &playlistNode);
}

void SandboxPlaylistStream::opened(const QString &filePath, quint16 programId)
{
  currentFile = filePath;
  startTime = QDateTime::currentDateTime();
}

void SandboxPlaylistStream::closed(const QString &filePath, quint16 programId)
{
  // Mark as played:
  if (startTime.secsTo(QDateTime::currentDateTime()) >= 120)
    std::cerr << ('%' + filePath.toUtf8().toHex()).data() << std::endl;

  if (currentFile == filePath)
    currentFile = QString::null;
}


SandboxSlideShowStream::SandboxSlideShowStream(const SMediaInfoList &files)
  : MediaStream(),
    slideShow(this, files)
{
  connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
  connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&slideShow, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
}

bool SandboxSlideShowStream::setup(const SHttpServer::RequestHeader &request, QIODevice *socket)
{
  if (MediaStream::setup(
          request, socket,
          slideShow.duration(),
          SInterval::fromFrequency(slideShow.frameRate), slideShow.size(),
          SAudioFormat::Channel_Stereo))
  {
    slideShow.setSize(videoResizer.size());

    return true;
  }
  else
    return false;
}

} } // End of namespaces
