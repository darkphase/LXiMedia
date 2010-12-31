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

CameraServer::CameraServer(TelevisionBackend *plugin, MasterServer *server, const QStringList &cameras)
  : VideoServer(QT_TR_NOOP("Cameras"), plugin, server),
    plugin(plugin),
    cameras(cameras)
{
  enableDlna();

  foreach (const QString &camera, cameras)
  {
    DlnaServer::File file(dlnaDir.server());
    file.mimeType = "video/mpeg";
    file.url = httpPath() + camera.toUtf8().toHex() + ".mpeg";

    dlnaDir.addFile(camera, file);
  }
}

CameraServer::~CameraServer()
{
}

bool CameraServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (file.isEmpty() || file.endsWith(".html"))
    return handleHtmlRequest(url, file, socket);

  return VideoServer::handleConnection(request, socket);
}

bool CameraServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());

  QString camera;
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');
  if (file.count() >= 2)
    camera = QString::fromUtf8(QByteArray::fromHex(file.first().toAscii()));

  // Create a new stream
  CameraStream *stream = new CameraStream(this, socket->peerAddress(), request.path(), camera);
  stream->setup(false);
  if (stream->start())
    return true; // The graph owns the socket now.

  delete stream;

  socket->write(QHttpResponseHeader(404).toString().toUtf8());

  qWarning() << "Failed to start stream" << request.path();
  return false;
}

bool CameraServer::buildPlaylist(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QStringList file = url.path().mid(url.path().lastIndexOf('/') + 1).split('.');

  HtmlParser htmlParser;
  htmlParser.setField("ITEMS", QByteArray(""));

  const QString server = "http://" + request.value("Host") + httpPath();

  if (file.count() >= 2)
  {
    htmlParser.setField("ITEM_LENGTH", QByteArray(""));
    htmlParser.setField("ITEM_NAME", file.first());

    htmlParser.setField("ITEM_URL", server.toAscii() + file.first() + ".mpeg");
    htmlParser.appendField("ITEMS", htmlParser.parse(m3uPlaylistItem));
  }

  QHttpResponseHeader response(200);
  response.setContentType("audio/x-mpegurl");
  response.setValue("Cache-Control", "no-cache");
  socket->write(response.toString().toUtf8());
  socket->write(htmlParser.parse(m3uPlaylist));

  return false;
}


CameraServer::CameraStream::CameraStream(CameraServer *parent, const QHostAddress &peer, const QString &url, const QString &camera)
  : Stream(parent, peer, url),
    input(this, camera)
{
  connect(&input, SIGNAL(output(SAudioBuffer)), &sync, SLOT(input(SAudioBuffer)));
  connect(&input, SIGNAL(output(SVideoBuffer)), &subtitleRenderer, SLOT(input(SVideoBuffer)));
}

} // End of namespace
