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
#include "filenode.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char  * const MediaPlayerSandbox::path = "/mediaplayer/";
const QEvent::Type  MediaPlayerSandbox::responseEventType = QEvent::Type(QEvent::registerEventType());

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
  if (request.isPost())
  {
    if (request.url().hasQueryItem("probeformat"))
    {
      QtConcurrent::run(this, &MediaPlayerSandbox::probeFormat, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("probecontent"))
    {
      QtConcurrent::run(this, &MediaPlayerSandbox::probeContent, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("readimage"))
    {
      QtConcurrent::run(this, &MediaPlayerSandbox::readImage, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("play"))
    {
      const QString path = QString::fromUtf8(request.content());
      if (!path.isEmpty())
      {
        if (path.endsWith("/.all"))
        {
          QStringList files;

          QDir dir(path.left(path.length() - 4));
          foreach (const QString &file, dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase))
            files += dir.absoluteFilePath(file);

          QList< QFuture<SMediaInfo::ProbeInfo::FileType> > futures;
          for (int i=qMax(0, (files.count() / 2) - 4), n=qMin(i+8, files.count()); i<n; i++)
            futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileType, files[i]);

          int audio = 0, video = 0, image = 0;
          for (int i=0; i<futures.count(); i++)
          switch (futures[i].result())
          {
          case SMediaInfo::ProbeInfo::FileType_Disc:
          case SMediaInfo::ProbeInfo::FileType_None:
            break;

          case SMediaInfo::ProbeInfo::FileType_Audio:
            audio++;
            break;

          case SMediaInfo::ProbeInfo::FileType_Video:
            video++;
            break;

          case SMediaInfo::ProbeInfo::FileType_Image:
            image++;
            break;
          }

          SMediaInfo::ProbeInfo::FileType fileType = SMediaInfo::ProbeInfo::FileType_None;
          if ((audio > video) && (audio > image))
          {
            fileType = SMediaInfo::ProbeInfo::FileType_Audio;

            qsrand(QDateTime::currentDateTime().toTime_t());
            for (int i=0; i<files.count(); i++)
              qSwap(files[i], files[qrand() % files.count()]);
          }
          else if ((video > audio) && (video > image))
            fileType = SMediaInfo::ProbeInfo::FileType_Video;
          else if ((image > audio) && (image > video))
            fileType = SMediaInfo::ProbeInfo::FileType_Image;

          if ((fileType == SMediaInfo::ProbeInfo::FileType_Audio) ||
              (fileType == SMediaInfo::ProbeInfo::FileType_Video))
          {
            SandboxPlaylistStream * const stream = new SandboxPlaylistStream(files, fileType);
            if (stream->setup(request, socket))
            if (stream->start())
            {
              streams.append(stream);
              return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
            }

            delete stream;
          }
          else if (fileType == SMediaInfo::ProbeInfo::FileType_Image)
          {
            SandboxSlideShowStream * const stream = new SandboxSlideShowStream(files);
            if (stream->setup(request, socket))
            if (stream->start())
            {
              streams.append(stream);
              return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
            }

            delete stream;
          }
        }
        else
        {
          SandboxFileStream * const stream = new SandboxFileStream(path);
          if (stream->setup(request, socket))
          if (stream->start())
          {
            streams.append(stream);
            return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
          }

          delete stream;
        }
      }

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_InternalServerError);
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void MediaPlayerSandbox::customEvent(QEvent *e)
{
  if (e->type() == responseEventType)
  {
    ResponseEvent * const event = static_cast<ResponseEvent *>(e);

    SSandboxServer::sendHttpResponse(event->request, event->response, event->socket, false);
  }
  else
    BackendSandbox::customEvent(e);
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

SMediaInfo::ProbeInfo::FileType MediaPlayerSandbox::probeFileType(const QString &fileName)
{
  return SMediaInfo(fileName).fileType();
}

void MediaPlayerSandbox::probeFormat(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  QList< QFuture<QByteArray> > futures;
  foreach (const QByteArray &path, request.content().split('\n'))
  if (!path.isEmpty())
    futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileFormat, QString::fromUtf8(path));

  QByteArray content;
  foreach (const QFuture<QByteArray> &future, futures)
    content += future.result() + '\n';

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(
          request,
          SSandboxServer::Status_Ok,
          content,
          SHttpEngine::mimeTextXml),
      socket));
}

QByteArray MediaPlayerSandbox::probeFileFormat(const QString &fileName)
{
  qDebug() << "Probing format:" << fileName;

  FileNode fileNode(fileName);
  if (!fileNode.isNull())
    return fileNode.probeFormat(-1);

  return QByteArray();
}

void MediaPlayerSandbox::probeContent(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  QList< QFuture<QByteArray> > futures;
  foreach (const QByteArray &path, request.content().split('\n'))
  if (!path.isEmpty())
    futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileContent, QString::fromUtf8(path));

  QByteArray content;
  foreach (const QFuture<QByteArray> &future, futures)
    content += future.result() + '\n';

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(
          request,
          SSandboxServer::Status_Ok,
          content,
          SHttpEngine::mimeTextXml),
      socket));
}

QByteArray MediaPlayerSandbox::probeFileContent(const QString &fileName)
{
  qDebug() << "Probing content:" << fileName;

  FileNode fileNode(fileName);
  if (!fileNode.isNull())
    return fileNode.probeContent(-1);

  return QByteArray();
}

void MediaPlayerSandbox::readImage(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  QSize maxsize;
  if (request.url().hasQueryItem("maxsize"))
    maxsize = SSize::fromString(request.url().queryItemValue("maxsize")).size();

  QByteArray format = "png";
  if (request.url().hasQueryItem("format"))
    format = request.url().queryItemValue("format").toAscii();

  QBuffer buffer;
  buffer.open(QAbstractSocket::WriteOnly);
  SImage::fromFile(QString::fromUtf8(request.content()), maxsize).save(&buffer, format, 80);
  buffer.close();

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(
          request,
          SSandboxServer::Status_Ok,
          buffer.data(),
          "image/" + format.toLower()),
      socket));
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


SandboxPlaylistStream::SandboxPlaylistStream(const QStringList &files, SMediaInfo::ProbeInfo::FileType fileType)
  : MediaTranscodeStream(),
    playlistNode(this, files, fileType)
{
}

bool SandboxPlaylistStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (MediaTranscodeStream::setup(
        request, socket,
        &playlistNode,
        STime(),
        true))
  {
    connect(&playlistNode, SIGNAL(finished()), SLOT(stop()));
    connect(&playlistNode, SIGNAL(opened(QString)), SLOT(opened(QString)));
    connect(&playlistNode, SIGNAL(closed(QString)), SLOT(closed(QString)));
    connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

    return true;
  }

  return false;
}

void SandboxPlaylistStream::opened(const QString &filePath)
{
  // Mark as played:
  std::cerr << ("#PLAYED:" + filePath.toUtf8().toHex()).data() << std::endl;

  currentFile = filePath;
}

void SandboxPlaylistStream::closed(const QString &filePath)
{
  if (currentFile == filePath)
    currentFile = QString::null;
}


SandboxSlideShowStream::SandboxSlideShowStream(const QStringList &files)
  : MediaStream(),
    slideShow(this, files)
{
}

bool SandboxSlideShowStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
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

    if (request.url().hasQueryItem("slideduration"))
      slideShow.setSlideDuration(STime::fromMSec(request.url().queryItemValue("slideduration").toInt()));

    return true;
  }
  else
    return false;
}

} } // End of namespaces
