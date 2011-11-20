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
    " <div class=\"menu\">\n"
    "  <ul>\n"
    "   <li>{TR_DETAILS}</li>\n"
    "{DETAILS}"
    "  </ul>\n"
    " </div>\n"
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"player\">\n"
    "   <a href=\"{PHOTO}.jpeg\">\n"
    "    <img src=\"{PHOTO}.jpeg\" alt=\"Photo\" width=\"100%\" />\n"
    "   </a>\n"
    "  </div>\n"
    " </div>\n";

SHttpServer::ResponseMessage PhotoServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, const MediaServer::File &file)
{
  const MediaDatabase::UniqueID uid = MediaDatabase::fromUidString(file.fileName());
  const FileNode node = mediaDatabase->readNode(uid);
  if (!node.isNull())
  foreach (const SMediaInfo::Program &program, node.programs())
  {
    HtmlParser htmlParser;
    htmlParser.setField("TITLE", node.title());
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

    htmlParser.setField("PHOTO", MediaDatabase::toUidString(uid));
    return makeHtmlContent(request, file.url(), htmlParser.parse(htmlView));
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

} } // End of namespaces
