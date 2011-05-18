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

#include "photoserver.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

const char PhotoServer::htmlView[] =
    " <ul class=\"menu\">\n"
    "  <li>{TR_DETAILS}</li>\n"
    "{DETAILS}"
    " </ul>\n"
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    "  <div class=\"player\">\n"
    "   <a href=\"{PHOTO}.jpeg\">\n"
    "    <img src=\"{PHOTO}.jpeg\" alt=\"Photo\" width=\"100%\" />\n"
    "   </a>\n"
    "  </div>\n"
    " </div>\n";

const char PhotoServer::htmlDetail[] =
    "  <li>{ITEM_NAME}: {ITEM_VALUE}</li>\n";

SHttpServer::SocketOp PhotoServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, const QString &file)
{
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  const QUrl url(request.path());
  const QString album = SStringParser::toRawName(url.queryItemValue("album"));
  const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file);
  const FileNode node = mediaDatabase->readNode(uid);
  if (!node.isNull())
  if (uid.pid < node.programs().count())
  {
    const SMediaInfo::Program program = node.programs().at(uid.pid);

    HtmlParser htmlParser;
    htmlParser.setField("TR_DETAILS", tr("Details"));

    htmlParser.setField("DETAILS", QByteArray(""));
    htmlParser.setField("ITEM_NAME", tr("File"));
    htmlParser.setField("ITEM_VALUE", node.fileName());
    htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));
    htmlParser.setField("ITEM_NAME", tr("Date"));
    htmlParser.setField("ITEM_VALUE", node.lastModified().toString(searchDateTimeFormat));
    htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));
    htmlParser.setField("ITEM_NAME", tr("Type"));
    htmlParser.setField("ITEM_VALUE", node.fileTypeName());
    htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));
    htmlParser.setField("ITEM_NAME", tr("Format"));
    htmlParser.setField("ITEM_VALUE", videoFormatString(program));
    htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));

    QString basePath = url.path().mid(serverPath().length());
    basePath = basePath.startsWith('/') ? basePath : ('/' + basePath);

    // Build the page selector
    htmlParser.setField("PAGES", buildPages(basePath));
    htmlParser.setField("ITEM_NAME", QByteArray(">"));
    htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));
    htmlParser.setField("ITEM_NAME", node.title());
    htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));

    // Build the viewer
    htmlParser.setField("PHOTO", MediaDatabase::toUidString(uid));
    return sendHtmlContent(request, socket, url, response, htmlParser.parse(htmlView));
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

} } // End of namespaces
