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

#include "movieserver.h"

namespace LXiMediaCenter {

const char * const MovieServer::htmlMovieWidget =
    "    <table class=\"widgetbuttons\">\n"
    "     <tr class=\"widgetbuttons\">\n"
    "{LATEST_MOVIES}"
    "     </tr>\n"
    "    </table>\n";

const char * const MovieServer::htmlMovieWidgetItem =
    "      <td class=\"widgetbutton\">\n"
    "       <div class=\"widgetbutton\">\n"
    "        <a class=\"widgetbutton\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "         <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "        </a>\n"
    "       </div>\n"
    "       <div class=\"widgetbuttontitle\">\n"
    "        <a class=\"widgetbuttontitle\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "         {ITEM_TITLE}\n"
    "        </a>\n"
    "       </div>\n"
    "      </td>\n";

const char * const MovieServer::htmlRatingStar =
    "   <img src=\"{ITEM_RATING_IMAGE}\" alt=\"{ITEM_RATING_ALT}\" width=\"16\" height=\"16\" />\n";

QByteArray MovieServer::frontPageWidget(void) const
{
  HtmlParser htmlParser;

  QMultiMap<qint64, PlayItem> movies;
  foreach (const QString &movie, mediaDatabase->allMovies())
  {
    const QList<MediaDatabase::UniqueID> files = mediaDatabase->allMovieFiles(movie);
    if (!files.isEmpty())
    {
      const SMediaInfo node = mediaDatabase->readNode(files.first());
      if (!node.isNull())
        movies.insert(-qint64(node.lastModified().toTime_t()), PlayItem(files.first(), node));
    }
  }

  htmlParser.setField("LATEST_MOVIES", QByteArray(""));
  int count = 0;
  foreach (const PlayItem &item, movies)
  {
    const QString uidString = MediaDatabase::toUidString(item.uid);

    QString title = QFileInfo(item.mediaInfo.filePath()).completeBaseName();
    const ImdbClient::Entry imdbEntry = mediaDatabase->getMovieImdbEntry(SStringParser::toRawName(title));
    if (!imdbEntry.isNull())
    {
      title = imdbEntry.title;
      if (imdbEntry.rating > 0.0f)
        title += " [" + QByteArray::number(imdbEntry.rating, 'f', 1) +"]";
    }

    htmlParser.setField("ITEM_TITLE", title);
    htmlParser.setField("ITEM_URL", httpPath() + uidString + ".html");
    htmlParser.setField("ITEM_ICONURL", httpPath() + uidString + "-thumb.jpeg?size=128");
    htmlParser.appendField("LATEST_MOVIES", htmlParser.parse(htmlMovieWidgetItem));

    if (++count >= 3)
      break;
  }

  if (count > 0)
  {
    htmlParser.setField("WIDGET_TITLE", tr("Latest movies"));
    htmlParser.setField("WIDGET_CONTENT", htmlParser.parse(htmlMovieWidget));
    return htmlParser.parse(htmlFrontPageWidget);
  }
  else
    return MediaPlayerServer::frontPageWidget();
}

} // End of namespace
