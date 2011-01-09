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

#include <liblximediacenter/videoserver.h>
#include <liblximediacenter/htmlparser.h>

//#define ENABLE_HTML5_PLAYER

namespace LXiMediaCenter {


const unsigned      VideoServer::itemsPerThumbnailPage = 60;
const char  * const VideoServer::audioTimeFormat = "m:ss";
const char  * const VideoServer::videoTimeFormat = "h:mm:ss";


const char * const VideoServer::m3uPlaylist =
    "#EXTM3U\n"
    "\n"
    "{ITEMS}";

const char * const VideoServer::m3uPlaylistItem =
    "#EXTINF:{ITEM_LENGTH},{ITEM_NAME}\n"
    "{ITEM_URL}\n";

const char * const VideoServer::htmlPages =
    "<div class=\"pageselector\">\n"
    " {TR_PAGE}:{PAGES}\n"
    "</div>\n";

const char * const VideoServer::htmlPageItem =
    " <a class=\"pageselector\" href=\"{ITEM_LINK}\">{ITEM_NAME}</a>";

const char * const VideoServer::htmlPageCurrentItem =
    " {ITEM_NAME}";

const char * const VideoServer::htmlThumbnails =
    " {PAGES}\n"
    " <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "  <!--\n"
    "  var winW = window.innerWidth - 32;\n"
    "  if (winW >= 1440)\n"
    "    document.write(\"<table class=\\\"thumbnaillist\\\">{LIST2_ESC}</table>\");\n"
    "  else if (winW >= 1152)\n"
    "    document.write(\"<table class=\\\"thumbnaillist\\\">{LIST1_ESC}</table>\");\n"
    "  else\n"
    "    document.write(\"<table class=\\\"thumbnaillist\\\">{LIST0_ESC}</table>\");\n"
    "  //-->\n"
    " </script>\n"
    " <noscript>\n"
    "  <table class=\"thumbnaillist\">\n"
    "{LIST0}"
    "  </table>\n"
    " </noscript>\n";

const char * const VideoServer::htmlThumbnailItemRow =
    "   <tr class=\"thumbnaillist\">\n"
    "{ROW_ITEMS}"
    "   </tr>\n";

const char * const VideoServer::htmlThumbnailItem =
    "    <td class=\"thumbnaillistitem\">\n"
    "     <center>\n"
    "      <div class=\"{ITEM_CLASS}\">\n"
    "       <a class=\"thumbnaillistitem\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "        <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "       </a>\n"
    "      </div>\n"
    "      <div class=\"thumbnaillistitemtitle\">\n"
    "       <a class=\"thumbnaillistitemtitle\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "        {ITEM_TITLE}\n"
    "       </a>\n"
    "      </div>\n"
    "      <div class=\"thumbnaillistitemnote\">\n"
    "       {ITEM_SUBTITLE}\n"
    "      </div>\n"
    "     </center>\n"
    "    </td>\n";

const char * const VideoServer::htmlThumbnailItemNoTitle =
    "    <td class=\"thumbnaillistitem\">\n"
    "     <center>\n"
    "      <div class=\"thumbnaillistitem\">\n"
    "       <a class=\"thumbnaillistitem\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "        <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "       </a>\n"
    "      </div>\n"
    "     </center>\n"
    "    </td>\n";

const char * const VideoServer::htmlPlayer =
    " <table class=\"main\">\n"
    "  <tr class=\"main\">\n"
    "   <td class=\"maincenter\" colspan=\"2\">\n"
    "{PLAYER}\n"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"main\">\n"
    "   <td class=\"mainleft\" width=\"40%\">\n"
    "{PLAYER_INFOITEMS}"
    "   </td>\n"
    "   <td class=\"mainleft\" width=\"60%\">\n"
    "    <b>{PLAYER_DESCRIPTION_NAME}:</b><br />\n"
    "{PLAYER_DESCRIPTION}"
    "   </td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const VideoServer::htmlPlayerAudioItem =
#ifdef ENABLE_HTML5_PLAYER
    "    <audio src=\"{PLAYER_ITEM}.wav\" autoplay=\"autoplay\" controls=\"controls\">\n"
#endif
    "     <center>\n"
    "      <div id=\"player\" style=\"display:block;height:{HEIGHT}px;\" href=\"{PLAYER_ITEM}.flv\"></div>\n"
    "      <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "       <!--\n"
    "       flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "         clip: { autoPlay: true }\n"
    "        } );\n"
    "       //-->\n"
    "      </script>\n"
    "     </center>\n"
#ifdef ENABLE_HTML5_PLAYER
    "    </audio>\n"
#endif
    ;

const char * const VideoServer::htmlPlayerVideoItem =
#ifdef ENABLE_HTML5_PLAYER
    "    <video src=\"{PLAYER_ITEM}.ogv{QUERY}\" width=\"{WIDTH}\" height=\"{HEIGHT}\" autoplay=\"autoplay\" controls=\"controls\">\n"
#endif
    "     <center>\n"
    "      <div id=\"player\" style=\"display:block;width:{WIDTH}px;height:{HEIGHT}px;\" href=\"{PLAYER_ITEM}.flv{QUERYX}\"></div>\n"
    "      <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "       <!--\n"
    "       flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "         clip: { scaling: 'fit', accelerated: true, autoPlay: true }\n"
    "        } );\n"
    "       //-->\n"
    "      </script>\n"
    "     </center>\n"
#ifdef ENABLE_HTML5_PLAYER
    "    </video>\n"
#endif
    ;

const char * const VideoServer::htmlPlayerThumbItem =
    "    <center>\n"
    "     <table style=\"padding:0;border-spacing:0;width:{WIDTH}px;height:{HEIGHT}px;background-image:url('{PLAYER_ITEM}-thumb.jpeg?size={WIDTH}x{HEIGHT}');background-position:center;background-repeat:no-repeat;background-color:#000000;\">\n"
    "      <tr style=\"vertical-align:middle;height:{HEIGHT2}px;\"><td>\n"
    "       <center>\n"
    "        <div style=\"padding:1em;background-color:rgba(255,255,255,0.7);width:{WIDTH23}px;\">\n"
    "         {TR_DOWNLOAD_OPTIONS_EXPLAIN}\n"
    "         <br /><br />\n"
    "         <form name=\"play\" action=\"{PLAYER_ITEM}.html\" method=\"get\">\n"
    "          <input type=\"hidden\" name=\"play\" value=\"play\" />\n"
    "          {TR_LANGUAGE}:\n"
    "          <select name=\"language\">\n"
    "{LANGUAGES}"
    "          </select>\n"
    "          {TR_SUBTITLES}:\n"
    "          <select name=\"subtitles\">\n"
    "{SUBTITLES}"
    "          </select>\n"
    "          <br /><br />\n"
    "          {TR_START_FROM}:\n"
    "          <select name=\"position\">\n"
    "{CHAPTERS}"
    "          </select>\n"
    "          <br /><br />\n"
    "          <input type=\"submit\" value=\"{TR_PLAY_HERE}\" />\n"
    "         </form>\n"
    "        </div>\n"
    "       </center>\n"
    "      </td></tr>\n"
    "      <tr style=\"vertical-align:middle;height:{HEIGHT2}px;\"><td>\n"
    "       <center>\n"
    "        <div style=\"padding:1em;background-color:rgba(255,255,255,0.7);width:{WIDTH23}px;\">\n"
    "         {TR_TRANSCODE_OPTIONS_EXPLAIN}\n"
    "         <br /><br />\n"
    "         <form name=\"dlnasettings\" action=\"{TITLE}.mpeg\" method=\"get\">\n"
    "          <input type=\"hidden\" name=\"item\" value=\"{PLAYER_ITEM}\" />\n"
    "          <input type=\"hidden\" name=\"encode\" value=\"slow\" />\n"
    "          <input type=\"hidden\" name=\"priority\" value=\"lowest\" />\n"
    "          {TR_LANGUAGE}:\n"
    "          <select name=\"language\">\n"
    "{LANGUAGES}"
    "          </select>\n"
    "          {TR_SUBTITLES}:\n"
    "          <select name=\"subtitles\">\n"
    "{SUBTITLES}"
    "          </select>\n"
    "          <br /><br />\n"
    "          {TR_TRANSCODE_TO}:\n"
    "          <select name=\"size\">\n"
    "{FORMATS}"
    "          </select>\n"
    "          <select name=\"requestchannels\">\n"
    "{CHANNELS}"
    "          </select>\n"
    "          <br /><br />\n"
    "          <input type=\"submit\" value=\"{TR_DOWNLOAD}\" />\n"
    "         </form>\n"
    "        </div>\n"
    "       </center>\n"
    "      </td></tr>\n"
    "     </table>\n"
    "    </center>\n";

const char * const VideoServer::htmlPlayerThumbItemOption =
    "           <option value=\"{VALUE}\" {SELECTED}>{TEXT}</option>\n";

const char * const VideoServer::htmlPlayerInfoItem =
    "    <b>{ITEM_NAME}:</b><br />\n"
    "    {ITEM_VALUE}<br /><br />\n";

const char * const VideoServer::htmlPlayerInfoActionHead =
    "    <b>{ITEM_NAME}:</b><br />\n";

const char * const VideoServer::htmlPlayerInfoAction =
    "    <a href=\"{ITEM_LINK}\">{ITEM_NAME}</a><br />\n";


const char * const VideoServer::headList =
    " <link rel=\"stylesheet\" href=\"/list.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n";

const char * const VideoServer::headPlayer =
    " <script type=\"text/javascript\" src=\"/swf/flowplayer.js\" />\n";

QByteArray VideoServer::buildThumbnailView(const QString &title, const ThumbnailListItemMap &items, const QUrl &url)
{
  HtmlParser htmlParser;
  htmlParser.setField("TR_PAGE", tr("Page"));

  QStringList pageNames;
  {
    QSet<QString> names;
    foreach (const ThumbnailListItem &item, items)
      names.insert(item.page);

    pageNames = names.toList();
    qSort(pageNames);
  }

  if (pageNames.isEmpty() || pageNames.first().isEmpty())
  {
    // Build the page selector
    const unsigned numPages = (items.count() + (itemsPerThumbnailPage - 1)) / itemsPerThumbnailPage;
    unsigned selectedPage = url.queryItemValue("page").toUInt();
    if (selectedPage == 0)
      selectedPage = 1;

    htmlParser.setField("PAGES", QByteArray(""));
    for (unsigned i=0; i<numPages; i++)
    {
      htmlParser.setField("ITEM_LINK", "?page=" + QString::number(i+1));
      htmlParser.setField("ITEM_NAME", QString::number(i+1));

      if (i != (selectedPage - 1))
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageItem));
      else
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageCurrentItem));
    }

    if (selectedPage < numPages)
    {
      htmlParser.setField("ITEM_LINK", "?page=" + QString::number(selectedPage+1));
      htmlParser.setField("ITEM_NAME", tr("Next"));
      htmlParser.appendField("PAGES", htmlParser.parse(htmlPageItem));
    }

    if (numPages > 1)
      htmlParser.setField("PAGES", htmlParser.parse(htmlPages));
    else
      htmlParser.setField("PAGES", QByteArray(""));

    // Build the content for the selected page
    const unsigned pageBegin = (selectedPage - 1) * itemsPerThumbnailPage;
    const unsigned pageEnd = pageBegin + itemsPerThumbnailPage;

    htmlParser.setField("LIST0", QByteArray(""));
    htmlParser.setField("LIST1", QByteArray(""));
    htmlParser.setField("LIST2", QByteArray(""));
    for (unsigned cls=0; cls<3; cls++)
    {
      htmlParser.setField("ROW_ITEMS", QByteArray(""));
      unsigned col = 0, i = -1;
      foreach (const ThumbnailListItem &item, items)
      if ((++i >= pageBegin) && (i < pageEnd))
      {
        htmlParser.setField("ITEM_CLASS", QByteArray(item.played ? "thumbnaillistitemplayed" : "thumbnaillistitem"));
        htmlParser.setField("ITEM_TITLE", item.title);
        htmlParser.setField("ITEM_RAW_TITLE", SStringParser::toRawName(item.title));
        htmlParser.setField("ITEM_SUBTITLE", item.subtitle);
        htmlParser.setField("ITEM_ICONURL", item.iconurl);
        htmlParser.setField("ITEM_URL", item.url);
        htmlParser.appendField("ROW_ITEMS", htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoTitle : htmlThumbnailItem));

        if (++col == 3 + cls)
        {
          htmlParser.appendField("LIST" + QByteArray::number(cls), htmlParser.parse(htmlThumbnailItemRow));
          htmlParser.setField("ROW_ITEMS", QByteArray(""));
          col = 0;
        }
      }

      if (col > 0)
        htmlParser.appendField("LIST" + QByteArray::number(cls), htmlParser.parse(htmlThumbnailItemRow));

      QByteArray esc = htmlParser.field("LIST" + QByteArray::number(cls));
      htmlParser.setField("LIST" + QByteArray::number(cls) + "_ESC", esc.replace('\\', "\\\\").replace('\"', "\\\"").simplified());
    }

    htmlParser.setField("TITLE", title);
  }
  else
  {
    // Build the page selector
    QString selectedPage = url.queryItemValue("page").toUpper();
    if (selectedPage.isEmpty())
      selectedPage = SStringParser::toRawName(pageNames.first());

    QString selectedPageName = selectedPage;
    htmlParser.setField("PAGES", QByteArray(""));
    foreach (const QString &pageName, pageNames)
    {
      const QString rawPageName = SStringParser::toRawName(pageName);
      htmlParser.setField("ITEM_LINK", "?page=" + rawPageName.toLower());
      htmlParser.setField("ITEM_NAME", pageName);

      if (rawPageName == selectedPage)
      {
        selectedPageName = pageName;
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageCurrentItem));
      }
      else
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageItem));
    }

    if (pageNames.count() > 1)
      htmlParser.setField("PAGES", htmlParser.parse(htmlPages));
    else
      htmlParser.setField("PAGES", QByteArray(""));

    // Build the content for the selected page
    htmlParser.setField("LIST0", QByteArray(""));
    htmlParser.setField("LIST1", QByteArray(""));
    htmlParser.setField("LIST2", QByteArray(""));
    for (unsigned cls=0; cls<3; cls++)
    {
      htmlParser.setField("ROW_TITLES", QByteArray(""));
      htmlParser.setField("ROW_ITEMS", QByteArray(""));
      unsigned col = 0;
      foreach (const ThumbnailListItem &item, items)
      if (SStringParser::toRawName(item.page) == selectedPage)
      {
        htmlParser.setField("ITEM_CLASS", QByteArray(item.played ? "thumbnaillistitemplayed" : "thumbnaillistitem"));
        htmlParser.setField("ITEM_TITLE", item.title);
        htmlParser.setField("ITEM_RAW_TITLE", SStringParser::toRawName(item.title));
        htmlParser.setField("ITEM_SUBTITLE", item.subtitle);
        htmlParser.setField("ITEM_ICONURL", item.iconurl);
        htmlParser.setField("ITEM_URL", item.url);
        htmlParser.appendField("ROW_ITEMS", htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoTitle : htmlThumbnailItem));

        if (++col == 3 + cls)
        {
          htmlParser.appendField("LIST" + QByteArray::number(cls), htmlParser.parse(htmlThumbnailItemRow));
          htmlParser.setField("ROW_ITEMS", QByteArray(""));
          col = 0;
        }
      }

      if (col > 0)
        htmlParser.appendField("LIST" + QByteArray::number(cls), htmlParser.parse(htmlThumbnailItemRow));

      QByteArray esc = htmlParser.field("LIST" + QByteArray::number(cls));
      htmlParser.setField("LIST" + QByteArray::number(cls) + "_ESC", esc.replace('\\', "\\\\").replace('\"', "\\\"").simplified());
    }

    htmlParser.setField("TITLE", title + " - " + selectedPageName);
  }

  return htmlParser.parse(htmlThumbnails);
}

QByteArray VideoServer::buildVideoPlayer(const QByteArray &item, const SMediaInfo &mediaInfo, const QUrl &url, const QSize &size)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", mediaInfo.title());
  htmlParser.setField("PLAYER_ITEM", item);
  htmlParser.setField("TR_PLAY_HERE", tr("Play now"));
  htmlParser.setField("TR_PLAY_EXTERNAL", tr("Play in external player"));
  htmlParser.setField("TR_DOWNLOAD", tr("Download file"));
  htmlParser.setField("TR_LANGUAGE", tr("Language"));
  htmlParser.setField("TR_SUBTITLES", tr("Subtitles"));
  htmlParser.setField("TR_START_FROM", tr("Start from"));
  htmlParser.setField("TR_TRANSCODE_TO", tr("Transcode to"));

  htmlParser.setField("TR_DOWNLOAD_OPTIONS_EXPLAIN",
    tr("The file can be played in your web browser."));

  htmlParser.setField("TR_TRANSCODE_OPTIONS_EXPLAIN",
    tr("The file can be transcoded to standard MPEG and placed on a USB stick "
       "or network storage to be played on a TV or media player."));

  htmlParser.setField("WIDTH", QByteArray::number(size.width()));
  htmlParser.setField("WIDTH23", QByteArray::number(size.width() * 2 / 3));
  htmlParser.setField("HEIGHT", QByteArray::number(size.height()));
  htmlParser.setField("HEIGHT2", QByteArray::number(size.height() / 2));
  htmlParser.setField("LANGUAGE", url.queryItemValue("language"));

  QByteArray query =
      "?size=" + QByteArray::number(size.width()) + "x" + QByteArray::number(size.height()) +
      "&language=" + url.queryItemValue("language").toAscii() +
      "&subtitles=" + url.queryItemValue("subtitles").toAscii() +
      "&position=" + url.queryItemValue("position").toAscii();
  htmlParser.setField("QUERYX", "?query=" + query.toHex().toUpper());
  htmlParser.setField("QUERY", query.replace("&", "&amp;"));

  int count = 1;
  htmlParser.setField("LANGUAGES", QByteArray(""));
  htmlParser.setField("SELECTED", QByteArray(""));
  foreach (const SInterfaces::FormatProber::AudioStreamInfo &stream, mediaInfo.audioStreams())
  {
    htmlParser.setField("VALUE", QByteArray::number(stream.streamId, 16));
    if (stream.language[0] != 0)
      htmlParser.setField("TEXT", QString::number(count++) + ". " + SStringParser::iso639Language(stream.language));
    else
      htmlParser.setField("TEXT", QString::number(count++) + ". " + tr("Unknown"));

    htmlParser.appendField("LANGUAGES", htmlParser.parse(htmlPlayerThumbItemOption));
  }

  count = 1;
  htmlParser.setField("SUBTITLES", QByteArray(""));
  htmlParser.setField("SELECTED", QByteArray(""));
  foreach (const SInterfaces::FormatProber::DataStreamInfo &stream, mediaInfo.dataStreams())
  {
    htmlParser.setField("VALUE", QByteArray::number(stream.streamId, 16));
    if (stream.language[0] != 0)
      htmlParser.setField("TEXT", QString::number(count++) + ". " + SStringParser::iso639Language(stream.language));
    else
      htmlParser.setField("TEXT", QString::number(count++) + ". " + tr("Unknown"));

    htmlParser.appendField("SUBTITLES", htmlParser.parse(htmlPlayerThumbItemOption));
  }

  htmlParser.setField("VALUE", QByteArray(""));
  htmlParser.setField("TEXT", tr("None"));
  htmlParser.appendField("SUBTITLES", htmlParser.parse(htmlPlayerThumbItemOption));

  GlobalSettings settings;
  settings.beginGroup("DLNA");

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();

  struct T
  {
    static void addFormat(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeSize, const GlobalSettings::TranscodeSize &size)
    {
      if (settings.value("TranscodeSize", genericTranscodeSize).toString() == size.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", QString::number(size.size.width()) +
                                   "x" + QString::number(size.size.height()) +
                                   "x" + QString::number(size.size.aspectRatio()));
      htmlParser.setField("TEXT", size.name +
                          " (" + QString::number(size.size.width()) +
                          "x" + QString::number(size.size.height()) + ")");
      htmlParser.appendField("FORMATS", htmlParser.parse(htmlPlayerThumbItemOption));
    }

    static void addChannel(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeChannels, const GlobalSettings::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeChannels", genericTranscodeChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", QString::number(channel.channels, 16));
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("CHANNELS", htmlParser.parse(htmlPlayerThumbItemOption));
    }
  };

  htmlParser.setField("FORMATS", QByteArray(""));
  htmlParser.setField("CHANNELS", QByteArray(""));

  foreach (const GlobalSettings::TranscodeSize &size, settings.allTranscodeSizes())
    T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

  foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
    T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

  htmlParser.setField("CHAPTERS", QByteArray(""));
  htmlParser.setField("SELECTED", QByteArray(""));
  if (!mediaInfo.chapters().isEmpty())
  {
    int chapterNum = 1;
    foreach (const SMediaInfo::Chapter &chapter, mediaInfo.chapters())
    {
      htmlParser.setField("VALUE", QByteArray::number(chapter.begin.toSec()));
      htmlParser.setField("TEXT", tr("Chapter") + " " + QString::number(chapterNum++));
      if (!chapter.title.isEmpty())
        htmlParser.appendField("TEXT", ", " + chapter.title);

      htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
    }
  }
  else if (mediaInfo.duration().isValid())
  {
    for (int i=0, n=mediaInfo.duration().toSec(); i<n; i+=120)
    {
      htmlParser.setField("VALUE", QByteArray::number(i));
      htmlParser.setField("TEXT", QTime().addSecs(i).toString(videoTimeFormat));
      htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
    }
  }
  else
  {
    htmlParser.setField("VALUE", QByteArray("0"));
    htmlParser.setField("TEXT", QTime().toString(videoTimeFormat));
    htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
  }

  if (url.hasQueryItem("play"))
    return htmlParser.parse(htmlPlayerVideoItem);
  else
    return htmlParser.parse(htmlPlayerThumbItem);
}


} // End of namespace
