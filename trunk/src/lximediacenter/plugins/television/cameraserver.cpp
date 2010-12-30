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

#include "cameraserver.h"
#include "televisionbackend.h"

namespace LXiMediaCenter {

CameraServer::CameraServer(MasterServer *server, TelevisionBackend *plugin)
             :VideoServer(QT_TR_NOOP("Cameras"), server),
              plugin(plugin)
{
  enableDlna();

  quint32 id = 0x80000000;

  foreach (const SSystem::DeviceEntry &captureDevice, SSystem::availableVideoCaptureDevices())
  {
    STerminals::AudioVideoDevice * const dev = SSystem::createTerminal<STerminals::AudioVideoDevice>(this, captureDevice.url, false);
    if (dev)
    {
      if (dev->tuner() == NULL)
      {
        cameras[captureDevice.url] = new Camera(id++, dev);

        DlnaServer::File file(dlnaDir.server());
        file.mimeType = "video/mpeg";
        file.url = httpPath() + captureDevice.url.toUtf8().toHex() + ".mpeg";

        QString title = dev->friendlyName();
        for (int i=1; (i<1000) && (dlnaDir.findFile(title).id > 0); i++)
        if (i == 1)
          title = title + ("00" + QString::number(i)).right(3);
        else
          title = title.left(title.length() - 3) + ("00" + QString::number(i)).right(3);

        dlnaDir.addFile(title, file);
      }
      else
        delete dev;
    }
  }

  dlnaDir.sortOrder += 10;
}

CameraServer::~CameraServer()
{
  removeAllStreams();
  
  foreach (Camera *camera, cameras)
    delete camera;
}

bool CameraServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (file.isEmpty() || file.endsWith(".html"))
    return handleHtmlRequest(url, file, socket);

  return VideoServer::handleConnection(request, socket);
}

bool CameraServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket, const StreamRequest &r)
{
  const QString camera = QString::fromUtf8(QByteArray::fromHex(r.item.toAscii()));
  StreamRequest streamRequest = r;
  streamRequest.item = "camera:" + camera;
  streamRequest.position = "0";
  streamRequest.frameTime = STime::fromMSec(40);
  streamRequest.channelSetup = SAudioCodec::Channel_Stereo;

  if (joinExistingStream(socket, streamRequest))
    return true; // The graph owns the socket now.

  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  const unsigned streamId = registerStreamId();
  if (streamId == 0)
  {
    socket->write(QHttpResponseHeader(503).toString().toUtf8());
    return false;
  }

  const bool ownClient = request.value("User-Agent").contains("LXiStream");
  const QUrl url = request.path();
  const bool fastTranscode = url.queryItemValue("transcode") != "slow";
  const QString fileName = QDir(url.path()).dirName();

  Stream * const stream = new Stream();
  stream->request = streamRequest;
  stream->startPos = 0;
  stream->contentType = HttpServer::toMimeType(fileName);
  stream->startTime = QDateTime::currentDateTime();

  stream->graph = new SGraph(SGraph::MediaTask_Playback);
  stream->httpTerminal = stream->graph->createTerminal<STerminals::HttpStream>("http:///" + fileName, false);

  if (stream->httpTerminal)
  {
    stream->httpNode = stream->graph->openStream(stream->httpTerminal, stream->httpTerminal->outputStream(0));

    QHttpResponseHeader header(200);
    header.setContentType(stream->contentType);
    header.setValue("Cache-Control", "no-cache");
    header.setValue("X-Identifier", QString::number(streamId));
    stream->httpNode->setProperty("httpHeader", header.toString().toUtf8());

    if (!ownClient)
    {
      stream->httpNode->setProperty("enablePrivateData", false);
      buildIntro(stream);
    }
    else
      stream->httpNode->setProperty("enablePrivateData", true);

    stream->httpNode->invokeMethod("addSocket", Q_ARG(QAbstractSocket *, socket));

    stream->inputTerminal = cameras[camera]->terminal;
    stream->inputNode = stream->graph->openStream(stream->inputTerminal, stream->inputTerminal->inputStream(0));

    if (!ownClient)
      buildTranscodeGraph(stream, fastTranscode);
    else
      stream->graph->connectNodes(stream->inputNode, stream->httpNode);

    if (stream->graph->prepare())
    {
      stream->graph->start();

      addStream(stream, streamId);

      qDebug() << "Started video stream for camera " << camera;
      return true; // The graph owns the socket now.
    }

    qWarning() << "Failed to start video stream for camera" << camera;

    delete stream->graph; // Will also delete any objects created by the graph.
    delete stream;
    return true; // The graph owns the socket now.
  }

  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}

} // End of namespace
