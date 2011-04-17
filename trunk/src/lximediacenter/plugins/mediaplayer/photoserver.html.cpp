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

const char * const PhotoServer::htmlView =
    "<table class=\"full\"><tr><td><table width=\"100%\">\n"
    " <tr>\n"
    "  <td class=\"left\" width=\"40%\">\n"
    "   <p class=\"light\">{PHOTO_INFO0}</p>\n"
    "  </td>\n"
    "  <td class=\"center\">\n"
    "   <a href=\"{PHOTO}-similar.html?album={ITEM_ALBUM}\">{PHOTO_FINGERPRINT}</a>\n"
    "  </td>\n"
    "  <td class=\"right\" width=\"40%\">\n"
    "   <p class=\"light\">{PHOTO_INFO1}</p>\n"
    "  </td>\n"
    " </tr>\n"
    " <tr>\n"
    "  <td class=\"center\" colspan=\"3\">\n"
    "   <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "    <!--\n"
    "    var winW = window.innerWidth - 32;\n"
    "    var winH = window.innerHeight - 175;\n"
    "    document.write(\"<a href=\\\"{PHOTO}.jpeg\\\"><img src=\\\"{PHOTO}.jpeg?size=\" + winW + \"x\" + winH + \"\\\" alt=\\\"Photo\\\" /></a>\");\n"
    "    //-->\n"
    "   </script>\n"
    "   <noscript>\n"
    "    <a href=\"{PHOTO}.jpeg\">\n"
    "     <img src=\"{PHOTO}.jpeg\" alt=\"Photo\" width=\"100%\" />\n"
    "    </a>\n"
    "   </noscript>\n"
    "  </td>\n"
    " </tr>\n"
    " <tr>\n"
    "  <td class=\"left\" width=\"40%\">\n"
    "   <p class=\"light\">{PHOTO_INFO2}</p>\n"
    "  </td>\n"
    "  <td class=\"center\">\n"
    "   <a href=\"{PREVIOUS_UID}.html?album={ITEM_ALBUM}\">&lt;&lt;</a>\n"
    "   <a href=\"{NEXT_UID}.html?album={ITEM_ALBUM}\">&gt;&gt;</a>\n"
    "  </td>\n"
    "  <td class=\"right\" width=\"40%\">\n"
    "   <p class=\"light\">{PHOTO_INFO3}</p>\n"
    "  </td>\n"
    " </tr>\n"
    "</table></td></tr></table>\n";

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
    htmlParser.setField("PHOTO_INFO0", node.fileName());
    htmlParser.setField("PHOTO_INFO1", node.lastModified().toString(searchDateTimeFormat));
    htmlParser.setField("PHOTO_INFO2", node.fileTypeName());
    htmlParser.setField("PHOTO_INFO3", videoFormatString(program));
    htmlParser.setField("PHOTO_FINGERPRINT", /*node.fingerPrint.isNull() ?*/ QString("") /*: tr("Find similar")*/);

    htmlParser.setField("PHOTO", MediaDatabase::toUidString(uid));
    htmlParser.setField("PREVIOUS_UID", MediaDatabase::toUidString(uid));
    htmlParser.setField("NEXT_UID", MediaDatabase::toUidString(uid));
    htmlParser.setField("ITEM_ALBUM", album.toLower());

    /*const QList<MediaDatabase::UniqueID> files = mediaDatabase->allPhotoFiles(album);
    if (!files.isEmpty())
    {
      QList<MediaDatabase::UniqueID>::ConstIterator i;
      unsigned photoNumber = 0;
      for (i=files.begin(); i!=files.end(); i++, photoNumber++)
      if (*i == uid)
        break;

      if (i != files.end())
      {
        const QList<MediaDatabase::UniqueID>::ConstIterator previous = i - 1;
        const QList<MediaDatabase::UniqueID>::ConstIterator next = i + 1;

        if ((i != files.begin()) && (previous != files.end()))
          htmlParser.setField("PREVIOUS_UID", MediaDatabase::toUidString(*previous));

        if (next != files.end())
          htmlParser.setField("NEXT_UID", MediaDatabase::toUidString(*next));
      }
    }*/

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlView));
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

} } // End of namespaces
