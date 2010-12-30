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
    const QString camera = file.left(file.length() - 5);
    htmlParser.setField("PLAYER_ITEM", camera);
    htmlParser.setField("PLAYER_ITEM_TEXT", tr("Play in external player"));
    htmlParser.setField("DOWNLOAD_ITEM_TEXT", tr("Download as MPEG file"));
    htmlParser.setField("PLAYER_VIDEOITEMS", htmlParser.parse(htmlPlayerVideoItem));

    htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));

    htmlParser.setField("PLAYER_DESCRIPTION_NAME", tr("Description"));
    htmlParser.setField("PLAYER_DESCRIPTION", QByteArray(""));

    l.unlock();
    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
  }
  else
  {
    ThumbnailListItemMap cameras;
    for (QMap<QString, Camera *>::Iterator i=this->cameras.begin();
         i!=this->cameras.end();
         i++)
    {
      ThumbnailListItem item;
      item.title = (*i)->terminal->friendlyName();
      item.iconurl = i.key().toUtf8().toHex() + "-thumb.jpeg";
      item.url = i.key().toUtf8().toHex() + ".html";
      cameras.insert(SStringParser::toRawName(item.title), item);
    }

    l.unlock();
    return sendHtmlContent(socket, url, response, buildThumbnailView(tr("Cameras"), cameras, url), headList);
  }
}

} // End of namespace
