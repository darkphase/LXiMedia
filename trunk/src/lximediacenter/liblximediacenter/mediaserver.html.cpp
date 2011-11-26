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

#include "mediaserver.h"
#include "globalsettings.h"
#include "htmlparser.h"

namespace LXiMediaCenter {

const char MediaServer::m3uPlaylist[] =
    "#EXTM3U\n"
    "\n"
    "{ITEMS}";

const char MediaServer::m3uPlaylistItem[] =
    "#EXTINF:{ITEM_LENGTH},{ITEM_NAME}\n"
    "{ITEM_URL}\n";

const char MediaServer::htmlThumbnailList[] =
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"thumbnaillist\">\n"
    "{ITEMS}"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlThumbnailLoader[] =
    " <div class=\"content\">\n"
    "  <div class=\"thumbnaillist\" id=\"items\">\n"
    "  </div>\n"
    "  <script type=\"text/javascript\">loadListContent(\"items\", 0, {LOAD_ITEM_COUNT});</script>\n"
    " </div>\n";

const char MediaServer::htmlThumbnailItem[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "      <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "     </a>\n"
    "    </div>\n"
    "    <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   </div>\n";

const char MediaServer::htmlThumbnailItemNoTitle[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "      <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "     </a>\n"
    "    </div>\n"
    "   </div>\n";

const char MediaServer::htmlThumbnailItemNoLink[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </div>\n"
    "    <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   </div>\n";

const char MediaServer::htmlThumbnailItemNoLinkNoTitle[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </div>\n"
    "   </div>\n";

const char MediaServer::htmlPlayerLoader[] =
    " <div class=\"content\">\n"
    "  <div class=\"player\" id=\"player\">\n"
    "  </div>\n"
    "  <script type=\"text/javascript\">loadPlayer(\"player\", \"{ITEM_URL}\");</script>\n"
    " </div>\n";

const char MediaServer::htmlPhotoViewer[] =
    "   <img src=\"{ITEM_URL}\" alt=\"...\">\n";

const char MediaServer::htmlVideoPlayer[] =
    "   <video width=\"{ITEM_WIDTH}\" height=\"{ITEM_HEIGHT}\" autoplay=\"autoplay\" controls=\"controls\">\n"
    "    <source src=\"{ITEM_URL}?format=ogg\" type=\"video/ogg\" />\n"
    "    <source src=\"{ITEM_URL}?format=mpeg\" type=\"video/mpeg\" />\n"
    "    <img src=\"{ITEM_URL}?thumbnail=256x256\" alt=\"...\" width=\"256\" height=\"256\" /><br/>\n"
    "    {TR_HTML5_BROWSER}\n"
    "   </video>\n";

const char MediaServer::headList[] =
    " <script type=\"text/javascript\" src=\"/js/contentloader.js\"></script>\n"; // Open and close tag due to IE bug

const char MediaServer::headPlayer[] =
    " <script type=\"text/javascript\" src=\"/js/contentloader.js\"></script>\n"; // Open and close tag due to IE bug

QByteArray MediaServer::buildThumbnailView(const QString &title, const ThumbnailListItemList &items)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);
  htmlParser.setField("ITEMS", buildThumbnailItems(items));
  return htmlParser.parse(htmlThumbnailList);
}

QByteArray MediaServer::buildThumbnailLoader(void)
{
  HtmlParser htmlParser;
  htmlParser.setField("LOAD_ITEM_COUNT", QString::number(qBound(1, QThread::idealThreadCount(), 16) * 8));

  return htmlParser.parse(htmlThumbnailLoader);
}

QByteArray MediaServer::buildThumbnailItems(const ThumbnailListItemList &items)
{
  HtmlParser htmlParser;

  QByteArray result;
  foreach (const ThumbnailListItem &item, items)
  {
    htmlParser.setField("ITEM_TITLE", item.title);
    htmlParser.setField("ITEM_ICONURL", item.iconurl.toEncoded());

    if (!item.url.isEmpty())
    {
      htmlParser.setField("ITEM_URL", item.url.toEncoded());

      result += htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoTitle : htmlThumbnailItem);
    }
    else
      result += htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoLinkNoTitle : htmlThumbnailItemNoLink);
  }

  return result;
}

QByteArray MediaServer::buildPlayerLoader(const QString &file, SUPnPContentDirectory::Item::Type playerType)
{
  HtmlParser htmlParser;

  QUrl url = file;
  url.addQueryItem("player", QString::number(playerType));
  htmlParser.setField("ITEM_URL", url.toEncoded());

  return htmlParser.parse(htmlPlayerLoader);
}

QByteArray MediaServer::buildPhotoViewer(const QString &file, const QSize &size)
{
  HtmlParser htmlParser;

  QUrl url = file;
  url.addQueryItem("format", "jpeg");
  url.addQueryItem("resolution", SSize(size).toString());
  htmlParser.setField("ITEM_URL", url.toEncoded());

  return htmlParser.parse(htmlPhotoViewer);
}

QByteArray MediaServer::buildVideoPlayer(const QString &file, const QSize &size)
{
  HtmlParser htmlParser;

  htmlParser.setField("TR_HTML5_BROWSER",
      tr("Your browser does not support HTML5 video, please upgrade to a "
         "browser that does support it. Please refer to "
         "<a href=\"http://en.wikipedia.org/wiki/HTML5_video\">Wikipedia</a> "
         "for more information."));

  htmlParser.setField("ITEM_WIDTH", QByteArray::number(size.width()));
  htmlParser.setField("ITEM_HEIGHT", QByteArray::number(size.height()));

  QUrl url = file;
  htmlParser.setField("ITEM_URL", url.toEncoded());

  return htmlParser.parse(htmlVideoPlayer);
}

} // End of namespace
