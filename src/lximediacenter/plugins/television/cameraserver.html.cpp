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

namespace LXiMediaCenter {

bool CameraServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  HtmlParser htmlParser;

  if (file.endsWith(".html")) // Show player
  {
    htmlParser.setField("PLAYER", buildVideoPlayer(file.left(file.length() - 5).toAscii(), SMediaInfo(), url));

    htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
    htmlParser.setField("PLAYER_DESCRIPTION_NAME", QByteArray(""));
    htmlParser.setField("PLAYER_DESCRIPTION", QByteArray(""));

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
  }
  else
  {
    ThumbnailListItemMap items;
    foreach (const QString &camera, cameras)
    {
      ThumbnailListItem item;
      item.title = camera;
      item.iconurl = camera.toUtf8().toHex() + "-thumb.jpeg";
      item.url = camera.toUtf8().toHex() + ".html";
      items.insert(SStringParser::toRawName(item.title), item);
    }

    l.unlock();
    return sendHtmlContent(socket, url, response, buildThumbnailView(tr("Cameras"), items, url), headList);
  }
}

} // End of namespace
