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
#include "htmlparser.h"

namespace LXiMediaCenter {

const char MediaServer::m3uPlaylist[] =
    "#EXTM3U\n"
    "\n"
    "{ITEMS}";

const char MediaServer::m3uPlaylistItem[] =
    "#EXTINF:{ITEM_LENGTH},{ITEM_NAME}\n"
    "{ITEM_URL}\n";

const char MediaServer::htmlListHead[] =
    " <link rel=\"stylesheet\" href=\"/css/list.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/list.js\"></script>\n"; // Open and close tag due to IE bug

const char MediaServer::htmlListLoader[] =
    " <div class=\"list_{LIST_TYPE}\" id=\"items\">\n"
    " </div>\n"
    " <script type=\"text/javascript\">loadListContent(\"items\", \"{PATH}\", 0, {LOAD_ITEM_COUNT});</script>\n";

const char MediaServer::htmlListItemLink[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "   <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   <div class=\"text\">\n"
    "{ITEM_TEXT}\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemLinkNoTitle[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemFunc[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\" onclick=\"{ITEM_FUNC}('{ITEM_FILE}')\">\n"
    "    <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "   </div>\n"
    "   <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   <div class=\"text\">\n"
    "{ITEM_TEXT}\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemFuncNoTitle[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\" onclick=\"{ITEM_FUNC}('{ITEM_FILE}')\">\n"
    "    <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemNoLink[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "   </div>\n"
    "   <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   <div class=\"text\">\n"
    "{ITEM_TEXT}\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemNoLinkNoTitle[] =
    "  <div class=\"listitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlListItemTextLine[] =
    "    <p class=\"text\">{TEXT_LINE}</p>\n";

const char MediaServer::htmlAudioPlayer[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/player.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/player.js\"></script>\n" // Open and close tag due to IE bug
    "</head>\n"
    "<body id=\"body\" onresize=\"resizeWindow()\">\n"
    " <iframe class=\"browser\" id=\"browser\" src=\"{PATH}\" frameborder=\"0\">\n"
    " </iframe>\n"
    " <div class=\"audioplayer\" id=\"audioplayer\">\n"
    "  <span><img src=\"/img/audio-file.png?invert=\" alt=\"&gt;\" /></span>\n"
    "  <span class=\"player\" id=\"player\"></span>\n"
    "  <span><img class=\"button\" src=\"/img/close.png?invert=\" alt=\"[X]\" onclick=\"closePlayer()\" /></span>\n"
    " </div>\n"
    " <script type=\"text/javascript\">loadAudio(\"{ITEM_URL}\", \"{ITEM_TITLE}\");</script>\n"
    "</body>\n"
    "</html>\n";

const char MediaServer::htmlPlayer[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/player.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/player.js\"></script>\n" // Open and close tag due to IE bug
    "</head>\n"
    "<body id=\"body\" onresize=\"resizeWindow()\">\n"
    " <div class=\"player\" id=\"player\" onmousemove=\"showControls()\">\n"
    " </div>\n"
    " <script type=\"text/javascript\">load{ITEM_TYPE}(\"{ITEM_URL}\");</script>\n"
    " <div class=\"playercontrols\" id=\"controls\" onmousemove=\"showControls()\">\n"
    "  <img src=\"/img/close.png?invert=\" alt=\"[X]\" onclick=\"history.back()\" />\n"
    "  <div class=\"thumbnailbar\" id=\"items\" onmouseover=\"lockControls()\" onmouseout=\"unlockControls()\">\n"
    "  </div>\n"
    "  <script type=\"text/javascript\">loadThumbnailBar(\"{PATH}\", 0, {LOAD_ITEM_COUNT});</script>\n"
    " </div>\n"
    "</body>\n"
    "</html>\n";

QByteArray MediaServer::buildListLoader(const QString &path, ListType listType)
{
  HtmlParser htmlParser;

  switch(listType)
  {
  case ListType_Thumbnails:
    htmlParser.setField("LIST_TYPE", QByteArray("thumbnails"));
    break;

  case ListType_Details:
    htmlParser.setField("LIST_TYPE", QByteArray("details"));
    break;
  }

  htmlParser.setField("PATH", QUrl(path).toEncoded());
  htmlParser.setField("LOAD_ITEM_COUNT", QString::number(loadItemCount()));

  return htmlParser.parse(htmlListLoader);
}

QByteArray MediaServer::buildListItems(const ThumbnailListItemList &items)
{
  HtmlParser htmlParser;

  QByteArray result;
  foreach (const ThumbnailListItem &item, items)
  {
    htmlParser.setField("ITEM_TITLE", item.title);

    htmlParser.setField("ITEM_TEXT", QByteArray(""));
    foreach (const QString &text, item.text)
    {
      htmlParser.setField("TEXT_LINE", text);
      htmlParser.appendField("ITEM_TEXT", htmlParser.parse(htmlListItemTextLine));
    }

    htmlParser.setField("ITEM_ICONURL", item.iconurl.toEncoded());

    if (!item.url.isEmpty())
    {
      if (item.func.isEmpty())
      {
        htmlParser.setField("ITEM_URL", item.url.toEncoded());
        result += htmlParser.parse(item.title.isEmpty() ? htmlListItemLinkNoTitle : htmlListItemLink);
      }
      else
      {
        htmlParser.setField("ITEM_FUNC", item.func);
        htmlParser.setField("ITEM_FILE", item.url.toEncoded(QUrl::RemoveQuery));
        result += htmlParser.parse(item.title.isEmpty() ? htmlListItemFuncNoTitle : htmlListItemFunc);
      }
    }
    else
      result += htmlParser.parse(item.title.isEmpty() ? htmlListItemNoLinkNoTitle : htmlListItemNoLink);
  }

  return result;
}

SHttpServer::ResponseMessage MediaServer::buildAudioPlayer(const SHttpServer::RequestMessage &request)
{
  HtmlParser htmlParser;

  QString path = request.file();
  path = path.left(path.lastIndexOf('/') + 1);
  htmlParser.setField("PATH", QUrl(path).toEncoded());
  htmlParser.setField("ITEM_URL", QUrl(request.file()).toEncoded());

  const Item item = getItem(request.file());
  htmlParser.setField("ITEM_TITLE", item.title);
  if (!item.artist.isEmpty())
    htmlParser.appendField("ITEM_TITLE", " [" + item.artist + "]");
  if (item.duration > 0)
    htmlParser.appendField("ITEM_TITLE", " (" + QTime(0, 0, 0).addSecs(item.duration).toString("m:ss") + ")");

  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setField("Cache-Control", "no-cache");
  response.setContentType(SHttpEngine::mimeTextHtml);
  response.setContent(htmlParser.parse(htmlAudioPlayer));
  return response;
}

SHttpServer::ResponseMessage MediaServer::buildPlayer(const SHttpServer::RequestMessage &request)
{
  HtmlParser htmlParser;

  QString path = request.file();
  path = path.left(path.lastIndexOf('/') + 1);
  htmlParser.setField("PATH", QUrl(path).toEncoded());
  htmlParser.setField("LOAD_ITEM_COUNT", QString::number(loadItemCount()));
  htmlParser.setField("ITEM_URL", QUrl(request.file()).toEncoded());

  const Item item = getItem(request.file());
  if ((int(item.type) / 10) == 2)
    htmlParser.setField("ITEM_TYPE", QByteArray("Video"));
  else if ((int(item.type) / 10) == 3)
    htmlParser.setField("ITEM_TYPE", QByteArray("Image"));

  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setField("Cache-Control", "no-cache");
  response.setContentType(SHttpEngine::mimeTextHtml);
  response.setContent(htmlParser.parse(htmlPlayer));
  return response;
}

} // End of namespace
