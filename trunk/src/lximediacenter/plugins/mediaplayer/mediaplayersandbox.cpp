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

#include "mediaplayersandbox.h"
#include <iostream>

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
  server = NULL;

  cleanStreams();
}

SSandboxServer::ResponseMessage MediaPlayerSandbox::httpRequest(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isPost())
  {
    if (request.url().hasQueryItem("probeformat"))
    {
      startTask(&MediaPlayerSandbox::probeFormat, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("probecontent"))
    {
      startTask(&MediaPlayerSandbox::probeContent, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("readthumbnail"))
    {
      startTask(&MediaPlayerSandbox::readThumbnail, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("readimage"))
    {
      startTask(&MediaPlayerSandbox::readImage, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("listfiles"))
    {
      startTask(&MediaPlayerSandbox::listFiles, request, socket);

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_None);
    }
    else if (request.url().hasQueryItem("play"))
    {
      return play(request, socket);
    }
  }

  return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_NotFound);
}

void MediaPlayerSandbox::customEvent(QEvent *e)
{
  if (e->type() == responseEventType)
  {
    ResponseEvent * const event = static_cast<ResponseEvent *>(e);

    if (server)
      server->sendHttpResponse(event->request, event->response, event->socket, true);
    else
      event->socket->deleteLater();
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

void MediaPlayerSandbox::startTask(TaskFunc taskFunc, const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  class Task : public QRunnable
  {
  public:
    Task(MediaPlayerSandbox * parent, TaskFunc taskFunc, const SSandboxServer::RequestMessage &request, QIODevice *socket)
      : parent(parent), taskFunc(taskFunc), request(request), socket(socket)
    {
    }

    virtual void run(void)
    {
      (parent->*taskFunc)(request, socket);
    }

  private:
    MediaPlayerSandbox * const parent;
    const TaskFunc taskFunc;

    const SSandboxServer::RequestMessage request;
    QIODevice * const socket;
  };

  int priority = 0;
  if (request.url().hasQueryItem("priority"))
    priority = request.url().queryItemValue("priority").toInt();

  QThreadPool::globalInstance()->start(new Task(this, taskFunc, request, socket), priority);
}

void MediaPlayerSandbox::listFiles(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const int start = request.url().queryItemValue("start").toInt();
  const int count = request.url().queryItemValue("count").toInt();
  const bool returnAll = count == 0;

  const QUrl path = QString::fromUtf8(request.content());
  SMediaFilesystem filesystem(path);

  QStringList items;
  {
    QMutexLocker l(&itemCacheMutex);

    QMap<QUrl, QPair<QStringList, QTime> >::Iterator i = itemCache.find(path);
    if ((i == itemCache.end()) ||
        ((start == 0) && (count >= 0) && (qAbs(i->second.elapsed()) > 15000)))
    {
      l.unlock();

      items = filesystem.entryList(
          QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files,
          QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

      l.relock();

      i = itemCache.insert(path, qMakePair(items, QTime()));
      i->second.start();
    }
    else
      items = i->first;
  }

  QByteArray content;
  {
    struct T
    {
      static const char * trueFalse(bool b) { return b ? "true" : "false"; }

      static void writeFile(QXmlStreamWriter &writer, const SMediaFilesystem::Info &info, const QUrl &path)
      {
        writer.writeStartElement("file");

        writer.writeAttribute("isdir", trueFalse(info.isDir));
        writer.writeAttribute("isreadable", trueFalse(info.isReadable));
        writer.writeAttribute("size", QString::number(info.size));
        writer.writeAttribute("lastmodified", info.lastModified.toString(Qt::ISODate));

        writer.writeCharacters(path.toString());

        writer.writeEndElement();
      }
    };

    QXmlStreamWriter writer(&content);
    writer.setAutoFormatting(false);
    writer.writeStartElement("files");

    writer.writeAttribute("total", QString::number(items.count()));

    if (count >= 0)
    {
      for (int i=start, n=0; (i<items.count()) && (returnAll || (n<count)); i++, n++)
        T::writeFile(writer, filesystem.readInfo(items[i]), filesystem.filePath(items[i]));
    }
    else
    {
      for (int n = items.count(), ni = qMax(1, (n + (-count - 1)) / -count), i = ni / 2; i < n; i += ni)
        T::writeFile(writer, filesystem.readInfo(items[i]), filesystem.filePath(items[i]));
    }

    writer.writeEndElement();
  }

  SHttpServer::ResponseMessage response(
      request,
      SSandboxServer::Status_Ok,
      content,
      QString(SHttpEngine::mimeTextXml) + ";files");

  foreach (const QString &field, request.fieldNames())
  if (field.startsWith("X-"))
    response.setField(field, request.field(field));

  qApp->postEvent(this, new ResponseEvent(request, response, socket));
}

void MediaPlayerSandbox::probeFormat(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  SMediaInfoList nodes;

  const QList<QByteArray> paths = request.content().split('\n');
  if (paths.count() > 1)
  {
    QList< QFuture<SMediaInfo> > futures;
    foreach (const QByteArray &path, paths)
    if (!path.isEmpty())
      futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileFormat, QString::fromUtf8(path));

    foreach (const QFuture<SMediaInfo> &future, futures)
      nodes += future.result();
  }
  else if (!paths.isEmpty())
    nodes += probeFileFormat(QString::fromUtf8(paths.first()));

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(
          request,
          SSandboxServer::Status_Ok,
          serializeNodes(nodes),
          QString(SHttpEngine::mimeTextXml) + ";mediainfo"),
      socket));
}

SMediaInfo MediaPlayerSandbox::probeFileFormat(const QUrl &filePath)
{
  SMediaInfo fileNode(filePath);
  if (!fileNode.isNull())
    fileNode.probeFormat();

  return fileNode;
}

void MediaPlayerSandbox::probeContent(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  SMediaInfoList nodes;

  const QList<QByteArray> paths = request.content().split('\n');
  if (paths.count() > 1)
  {
    QList< QFuture<SMediaInfo> > futures;
    foreach (const QByteArray &path, paths)
    if (!path.isEmpty())
      futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileContent, QString::fromUtf8(path));

    foreach (const QFuture<SMediaInfo> &future, futures)
      nodes += future.result();
  }
  else if (!paths.isEmpty())
    nodes += probeFileContent(QString::fromUtf8(paths.first()));

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(
          request,
          SSandboxServer::Status_Ok,
          serializeNodes(nodes),
          QString(SHttpEngine::mimeTextXml) + ";mediainfo"),
      socket));
}

SMediaInfo MediaPlayerSandbox::probeFileContent(const QUrl &filePath)
{
  qDebug() << "Probing file:" << filePath.toString(QUrl::RemovePassword);

  SMediaInfo fileNode(filePath);
  if (!fileNode.isNull())
    fileNode.probeContent();

  return fileNode;
}

void MediaPlayerSandbox::readThumbnail(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  QSize maxSize(128, 128);
  if (request.url().hasQueryItem("maxsize"))
    maxSize = SSize::fromString(request.url().queryItemValue("maxsize")).size();

  QColor backgroundColor = Qt::black;
  if (request.url().hasQueryItem("bgcolor"))
    backgroundColor.setNamedColor('#' + request.url().queryItemValue("bgcolor"));

  QByteArray format = "png";
  if (request.url().hasQueryItem("format"))
    format = request.url().queryItemValue("format").toAscii();

  SMediaInfo fileNode(QString::fromUtf8(request.content()));
  if (!fileNode.isNull())
  {
    const SVideoBuffer thumbnail = fileNode.readThumbnail(maxSize);
    if (!thumbnail.isNull())
    {
      QBuffer buffer;
      buffer.open(QBuffer::WriteOnly);
      if (SImage(thumbnail, true).save(&buffer, format, 80))
      {
        buffer.close();

        qApp->postEvent(this, new ResponseEvent(
            request,
            SHttpServer::ResponseMessage(
                request,
                SSandboxServer::Status_Ok,
                buffer.data(),
                "image/" + format.toLower()),
            socket));

        return;
      }
    }
  }

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(request, SSandboxServer::Status_NotFound),
      socket));
}

void MediaPlayerSandbox::readImage(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  QSize maxsize;
  if (request.url().hasQueryItem("maxsize"))
    maxsize = SSize::fromString(request.url().queryItemValue("maxsize")).size();

  QColor backgroundColor = Qt::black;
  if (request.url().hasQueryItem("bgcolor"))
    backgroundColor.setNamedColor('#' + request.url().queryItemValue("bgcolor"));

  QByteArray format = "png";
  if (request.url().hasQueryItem("format"))
    format = request.url().queryItemValue("format").toAscii();

  QImage image = SImage::fromFile(QString::fromUtf8(request.content()), maxsize);
  if (!image.isNull())
  {
    if (maxsize.isNull())
      maxsize = image.size();

    if (maxsize != image.size())
    {
      QImage result(maxsize, QImage::Format_ARGB32);
      QPainter p;
      p.begin(&result);
        p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
        p.fillRect(result.rect(), backgroundColor);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
        p.drawImage(
            (result.width() / 2) - (image.width() / 2),
            (result.height() / 2) - (image.height() / 2),
            image);
      p.end();
      image = result;
    }

    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    if (image.save(&buffer, format, 80))
    {
      buffer.close();

      qApp->postEvent(this, new ResponseEvent(
          request,
          SHttpServer::ResponseMessage(
              request,
              SSandboxServer::Status_Ok,
              buffer.data(),
              "image/" + format.toLower()),
          socket));

      return;
    }
  }

  qApp->postEvent(this, new ResponseEvent(
      request,
      SHttpServer::ResponseMessage(request, SSandboxServer::Status_NotFound),
      socket));
}

SSandboxServer::ResponseMessage MediaPlayerSandbox::play(const SSandboxServer::RequestMessage &request, QIODevice *socket)
{
  const QUrl path(QString::fromUtf8(request.content()));
  if (!path.isEmpty())
  {
    if (request.url().queryItemValue("play") == "all")
    {
      QList<QUrl> files;
      for (QList<QUrl> dirs = QList<QUrl>() << path; !dirs.isEmpty(); )
      {
        const SMediaFilesystem dir(dirs.takeFirst());
        foreach (const QString &file, dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase))
          files += dir.filePath(file);

        foreach (const QString &subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase))
          dirs += dir.filePath(subDir);
      }

      QList< QFuture<SMediaInfo::ProbeInfo::FileType> > futures;
      for (int n = files.count(), ni = qMax(1, n / 16), i = ni / 2; i < n; i += ni)
        futures += QtConcurrent::run(&MediaPlayerSandbox::probeFileType, files[i]);

      int audio = 0, video = 0, image = 0;
      for (int i=0; i<futures.count(); i++)
      switch (futures[i].result())
      {
      case SMediaInfo::ProbeInfo::FileType_Audio:   audio++; break;
      case SMediaInfo::ProbeInfo::FileType_Video:   video++; break;
      case SMediaInfo::ProbeInfo::FileType_Image:   image++; break;

      case SMediaInfo::ProbeInfo::FileType_None:
      case SMediaInfo::ProbeInfo::FileType_Directory:
      case SMediaInfo::ProbeInfo::FileType_Drive:
      case SMediaInfo::ProbeInfo::FileType_Disc:
        break;
      }

      SMediaInfo::ProbeInfo::FileType fileType = SMediaInfo::ProbeInfo::FileType_None;
      if ((audio > video) && (audio > image))
      {
        fileType = SMediaInfo::ProbeInfo::FileType_Audio;

        // Randomize files.
        for (int i=0; i<files.count(); i++)
          qSwap(files[i], files[qrand() % files.count()]);
      }
      else if ((image > audio) && (image > video))
      {
        fileType = SMediaInfo::ProbeInfo::FileType_Image;

        // Only play files in this directory.
        files.clear();
        const SMediaFilesystem dir(path);
        foreach (const QString &file, dir.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase))
          files += dir.filePath(file);
      }
      else if ((video > audio) && (video > image))
        fileType = SMediaInfo::ProbeInfo::FileType_Video;

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

SMediaInfo::ProbeInfo::FileType MediaPlayerSandbox::probeFileType(const QUrl &filePath)
{
  return SMediaInfo(filePath).fileType();
}

QByteArray MediaPlayerSandbox::serializeNodes(const SMediaInfoList &nodes)
{
  QByteArray result;
  {
    QXmlStreamWriter writer(&result);
    writer.setAutoFormatting(false);
    writer.writeStartElement("nodes");

    foreach (const SMediaInfo &node, nodes)
      node.serialize(writer);

    writer.writeEndElement();
  }

  return result;
}


SandboxFileStream::SandboxFileStream(const QUrl &filePath)
  : MediaTranscodeStream(),
    file(this, filePath)
{
  connect(&file, SIGNAL(finished()), SLOT(stop()));

  connect(&file, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
  connect(&file, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
  connect(&file, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

  // Mark as played:
  std::cerr << ("#PLAYED:" + filePath.toEncoded().toHex()).data() << std::endl;
}

SandboxFileStream::~SandboxFileStream()
{
}

bool SandboxFileStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  return MediaTranscodeStream::setup(request, socket, &file);
}


SandboxPlaylistStream::SandboxPlaylistStream(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType fileType)
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
    connect(&playlistNode, SIGNAL(opened(QUrl)), SLOT(opened(QUrl)));
    connect(&playlistNode, SIGNAL(closed(QUrl)), SLOT(closed(QUrl)));
    connect(&playlistNode, SIGNAL(output(SEncodedAudioBuffer)), &audioDecoder, SLOT(input(SEncodedAudioBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedVideoBuffer)), &videoDecoder, SLOT(input(SEncodedVideoBuffer)));
    connect(&playlistNode, SIGNAL(output(SEncodedDataBuffer)), &dataDecoder, SLOT(input(SEncodedDataBuffer)));

    return true;
  }

  return false;
}

void SandboxPlaylistStream::opened(const QUrl &filePath)
{
  // Mark as played:
  std::cerr << ("#PLAYED:" + filePath.toEncoded().toHex()).data() << std::endl;

  currentFile = filePath;
}

void SandboxPlaylistStream::closed(const QUrl &filePath)
{
  if (currentFile == filePath)
    currentFile = QString::null;
}


SandboxSlideShowStream::SandboxSlideShowStream(const QList<QUrl> &files)
  : MediaStream(),
    slideShow(this, files)
{
}

bool SandboxSlideShowStream::setup(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.url().hasQueryItem("slideduration"))
    slideShow.setSlideDuration(STime::fromMSec(request.url().queryItemValue("slideduration").toInt()));

  if (MediaStream::setup(
          request, socket,
          STime::null, slideShow.duration(),
          SAudioFormat(SAudioFormat::Format_Invalid, SAudioFormat::Channels_Stereo, 48000),
          SVideoFormat(SVideoFormat::Format_Invalid, slideShow.size(), SInterval::fromFrequency(slideShow.frameRate)),
          false,
          SInterfaces::AudioEncoder::Flag_None,
          SInterfaces::VideoEncoder::Flag_Slideshow,
          slideShow.framesPerSlide()))
  {
    connect(&slideShow, SIGNAL(finished()), SLOT(stop()));
    connect(&slideShow, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
    connect(&slideShow, SIGNAL(output(SVideoBuffer)), &video->subtitleRenderer, SLOT(input(SVideoBuffer)));

    slideShow.setSize(video->resizer.size());

    return true;
  }
  else
    return false;
}

} } // End of namespaces
