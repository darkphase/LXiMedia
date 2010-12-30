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

bool PhotoServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  if (file.endsWith("-similar.html"))
  {
    /*const MediaDatabase::Node base = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));
    if (!base.mediaInfo.fingerPrint().isNull())
    {
      const QString baseName = SStringParser::toRawName(base.mediaProbe.title);
      const QString album = SStringParser::toRawName(url.queryItemValue("album"));
      const QString albumLower = album.toLower();
      const QList<MediaDatabase::UniqueID> files = mediaDatabase->allPhotoFiles(album);

      QMultiMap<qreal, MediaDatabase::Node> similar;
      foreach (const MediaDatabase::UniqueID &item, files)
      {
        const MediaDatabase::Node node = mediaDatabase->readNode(item);
        if (!node.fingerPrint.isNull())
        {
          const QString nodeName = SStringParser::toRawName(node.mediaProbe.title);

          qreal delta = base.fingerPrint.delta(node.fingerPrint);
          if ((baseName.length() >= 8) && (nodeName.length() >= 8))
            delta = qMin(delta, 1.0 - SStringParser::computeBidirMatch(baseName, nodeName));

          similar.insert(delta, node);
        }
      }

      ThumbnailListItemMap photos;
      foreach (const MediaDatabase::Node &node, similar)
      {
        const QString uidString = MediaDatabase::toUidString(node.uid);

        ThumbnailListItem item;
        item.iconurl = uidString + "-thumb.jpeg";
        item.url = uidString + ".html?album=" + albumLower;
        photos.insert(uidString, item);

        if (photos.count() >= int(itemsPerThumbnailPage))
          break;
      }

      return sendHtmlContent(socket, url, response, buildThumbnailView(tr("Similar photos"), photos, url), headList);
    }*/
  }
  else if (file.endsWith("-album.html"))
  {
    const QString album = SStringParser::toRawName(file.left(file.length() - 11));
    const QString albumLower = album.toLower();
    const QList<MediaDatabase::UniqueID> files = mediaDatabase->allPhotoFiles(album);

    QString title;
    ThumbnailListItemMap photos;
    foreach (MediaDatabase::UniqueID uid, files)
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        const QString uidString = MediaDatabase::toUidString(uid);

        ThumbnailListItem item;
        item.iconurl = uidString + "-thumb.jpeg";
        item.url = uidString + ".html?album=" + albumLower;

        photos.insert(node.fileName(), item);

        if (title.isEmpty())
        {
          QDir parentDir(node.path);
          parentDir.cdUp();

          title = parentDir.dirName();
        }
      }
    }

    if (title.isEmpty())
      title = albumLower;

    return sendHtmlContent(socket, url, response, buildThumbnailView(title, photos, url), headList);
  }
  else if (file.endsWith(".html")) // View photo
  {
    const QString album = SStringParser::toRawName(url.queryItemValue("album"));
    const MediaDatabase::Node node = mediaDatabase->readNode(MediaDatabase::fromUidString(file.left(16)));

    HtmlParser htmlParser;
    htmlParser.setField("PHOTO_INFO0", node.fileName());
    htmlParser.setField("PHOTO_INFO1", node.lastModified.toString(searchDateTimeFormat));
    htmlParser.setField("PHOTO_INFO2", node.mediaInfo.fileTypeName());
    htmlParser.setField("PHOTO_INFO3", videoFormatString(node.mediaInfo));
    htmlParser.setField("PHOTO_FINGERPRINT", /*node.fingerPrint.isNull() ?*/ QString("") /*: tr("Find similar")*/);

    htmlParser.setField("PHOTO", MediaDatabase::toUidString(node.uid));
    htmlParser.setField("PREVIOUS_UID", MediaDatabase::toUidString(node.uid));
    htmlParser.setField("NEXT_UID", MediaDatabase::toUidString(node.uid));
    htmlParser.setField("ITEM_ALBUM", album.toLower());

    const QList<MediaDatabase::UniqueID> files = mediaDatabase->allPhotoFiles(album);
    if (!files.isEmpty())
    {
      QList<MediaDatabase::UniqueID>::ConstIterator i;
      unsigned photoNumber = 0;
      for (i=files.begin(); i!=files.end(); i++, photoNumber++)
      if (*i == node.uid)
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
    }

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlView));
  }
  else // View album overview
  {
    ThumbnailListItemMap photoAlbums;
    foreach (const QString &photoAlbum, mediaDatabase->allPhotoAlbums())
    {
      const QList<MediaDatabase::UniqueID> files = mediaDatabase->allPhotoFiles(photoAlbum);
      if (files.count() >= minPhotosInAlbum)
      {
        ThumbnailListItem item;

        const MediaDatabase::UniqueID uid = files[qrand() % files.count()];
        const MediaDatabase::Node node = mediaDatabase->readNode(uid);
        if (!node.isNull())
        {
          QDir parentDir(node.path);
          parentDir.cdUp();

         item.title = parentDir.dirName();
        }
        else
          item.title = photoAlbum;

        item.subtitle = QString::number(files.count()) + " " + tr("photos");
        item.iconurl = MediaDatabase::toUidString(uid) + "-thumb.jpeg";
        item.url = photoAlbum.toLower() + "-album.html";

        photoAlbums.insert(photoAlbum, item);
      }
    }

    return sendHtmlContent(socket, url, response, buildThumbnailView(tr("Photo albums"), photoAlbums, url), headList);
  }

  response.setStatusLine(404);
  socket->write(response.toString().toUtf8());
  return false;
}

} // End of namespace
