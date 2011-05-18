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

bool                MediaServer::html5Enabled = false;
const unsigned      MediaServer::itemsPerThumbnailPage = 60;
const char          MediaServer::audioTimeFormat[] = "m:ss";
const char          MediaServer::videoTimeFormat[] = "h:mm:ss";

const char MediaServer::m3uPlaylist[] =
    "#EXTM3U\n"
    "\n"
    "{ITEMS}";

const char MediaServer::m3uPlaylistItem[] =
    "#EXTINF:{ITEM_LENGTH},{ITEM_NAME}\n"
    "{ITEM_URL}\n";

const char MediaServer::htmlPageItem[] =
    "    <li><a href=\"{ITEM_LINK}\">{ITEM_NAME}</a></li>\n";

const char MediaServer::htmlPageSeparator[] =
    "    <li>{ITEM_NAME}</li>\n";

const char MediaServer::htmlThumbnailList[] =
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    "  <div class=\"thumbnaillist\">\n"
    "{ITEMS}"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlThumbnailItem[] =
    "  <div class=\"thumbnaillistitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "   {ITEM_TITLE}<br />\n"
    "   <small>{ITEM_SUBTITLE}</small>\n"
    "  </div>\n";

const char MediaServer::htmlThumbnailItemNoTitle[] =
    "  <div class=\"thumbnaillistitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "  </div>\n";

const char MediaServer::htmlDetailedList[] =
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    " <table class=\"list\">\n"
    "{ITEMS}"
    " </table>\n"
    " </div>\n";

const char MediaServer::htmlDetailedListRow[] =
    "  <tr class=\"{ROW_CLASS}\">\n"
    "{COLUMNS}"
    "  </tr>\n";

const char MediaServer::htmlDetailedListHead[] =
    "   <th class=\"{ROW_CLASS}\">\n"
    "    {ITEM_TITLE}\n"
    "   </th>\n";

const char MediaServer::htmlDetailedListIcon[] =
    "   <td class=\"{ROW_CLASS}\" width=\"0%\">\n"
    "    <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "   </td>\n";

const char MediaServer::htmlDetailedListColumn[] =
    "   <td class=\"{ROW_CLASS}\">\n"
    "    {ITEM_TITLE}\n"
    "   </td>\n";

const char MediaServer::htmlDetailedListColumnLink[] =
    "   <td class=\"{ROW_CLASS}\">\n"
    "    <a class=\"listitem\" title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     {ITEM_TITLE}\n"
    "    </a>\n"
    "   </td>\n";

const char MediaServer::htmlPlayerAudioItem[] =
    "    <div id=\"player\" style=\"display:block;height:{HEIGHT}px;\" href=\"{PLAYER_ITEM}.flv\"></div>\n"
    "    <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "     <!--\n"
    "     flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "       clip: { autoPlay: true }\n"
    "      } );\n"
    "     //-->\n"
    "    </script>\n";

const char MediaServer::htmlPlayerAudioItemHtml5[] =
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    "  <div class=\"player\">\n"
    "   <audio src=\"{PLAYER_ITEM}.wav\" autoplay=\"autoplay\" controls=\"controls\">\n"
    "{FLV_PLAYER}"
    "   </audio>\n"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlPlayerVideoItem[] =
    "    <div id=\"player\" style=\"display:block;width:{WIDTH}px;height:{HEIGHT}px;\" href=\"{PLAYER_ITEM}.flv{QUERYX}\"></div>\n"
    "    <script language=\"JavaScript\" type=\"text/javascript\">\n"
    "     <!--\n"
    "     flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "       clip: { scaling: 'fit', accelerated: true, autoPlay: true }\n"
    "      } );\n"
    "     //-->\n"
    "    </script>\n";

const char MediaServer::htmlPlayerVideoItemHtml5[] =
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    "  <div class=\"player\">\n"
    "   <video src=\"{PLAYER_ITEM}.ogg{QUERY}\" width=\"{WIDTH}\" height=\"{HEIGHT}\" autoplay=\"autoplay\" controls=\"controls\">\n"
    "{FLV_PLAYER}"
    "   </video>\n"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlPlayerThumbItem[] =
    " <div class=\"content\">\n"
    "  <div class=\"pageselector\"><ul>\n"
    "{PAGES}"
    "  </ul></div>\n"
    "  <div class=\"player\">\n"
    "   <div style=\"padding:0;margin:0;display:table-cell;"
                    "width:{WIDTH}px;height:{HEIGHT}px;vertical-align:middle;"
                    "background-image:url('{PLAYER_ITEM}-thumb.png?size={WIDTH}x{HEIGHT}');"
                    "background-position:center;background-repeat:no-repeat;background-color:#000000;\">\n"
    "    <div style=\"padding:1em;margin:1em auto;width:{WIDTH23}px;background-color:rgb(176,176,176);background-color:rgba(255,255,255,0.7);\">\n"
    "     {TR_DOWNLOAD_OPTIONS_EXPLAIN}\n"
    "     <br /><br />\n"
    "     <form name=\"play\" action=\"{PLAYER_ITEM}.html\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"play\" value=\"play\" />\n"
    "      {TR_LANGUAGE}:\n"
    "      <select name=\"language\">\n"
    "{LANGUAGES}"
    "      </select>\n"
    "      {TR_SUBTITLES}:\n"
    "      <select name=\"subtitles\">\n"
    "{SUBTITLES}"
    "      </select>\n"
    "      <br /><br />\n"
    "      {TR_START_FROM}:\n"
    "      <select name=\"position\">\n"
    "{CHAPTERS}"
    "      </select>\n"
    "      <br /><br />\n"
    "      <input type=\"submit\" value=\"{TR_PLAY_HERE}\" />\n"
    "     </form>\n"
    "    </div>\n"
    "    <div style=\"padding:1em;margin:1em auto;width:{WIDTH23}px;background-color:rgb(176,176,176);background-color:rgba(255,255,255,0.7);\">\n"
    "     {TR_TRANSCODE_OPTIONS_EXPLAIN}\n"
    "     <br /><br />\n"
    "     <form name=\"dlnasettings\" action=\"{CLEAN_TITLE}.mpeg\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"item\" value=\"{PLAYER_ITEM}\" />\n"
    "      <input type=\"hidden\" name=\"encode\" value=\"slow\" />\n"
    "      <input type=\"hidden\" name=\"priority\" value=\"low\" />\n"
    "      {TR_LANGUAGE}:\n"
    "      <select name=\"language\">\n"
    "{LANGUAGES}"
    "      </select>\n"
    "      {TR_SUBTITLES}:\n"
    "      <select name=\"subtitles\">\n"
    "{SUBTITLES}"
    "      </select>\n"
    "      <br /><br />\n"
    "      {TR_TRANSCODE_TO}:\n"
    "      <select name=\"size\">\n"
    "{FORMATS}"
    "      </select>\n"
    "      <select name=\"requestchannels\">\n"
    "{CHANNELS}"
    "      </select>\n"
    "      <br /><br />\n"
    "      <input type=\"submit\" value=\"{TR_DOWNLOAD}\" />\n"
    "     </form>\n"
    "    </div>\n"
    "   </div>\n"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlPlayerThumbItemOption[] =
    "           <option value=\"{VALUE}\" {SELECTED}>{TEXT}</option>\n";

const char MediaServer::htmlPlayerInfoItem[] =
    "    <b>{ITEM_NAME}:</b><br />\n"
    "    {ITEM_VALUE}<br /><br />\n";

const char MediaServer::htmlPlayerInfoActionHead[] =
    "    <b>{ITEM_NAME}:</b><br />\n";

const char MediaServer::htmlPlayerInfoAction[] =
    "    <a href=\"{ITEM_LINK}\">{ITEM_NAME}</a><br />\n";


const char MediaServer::headPlayer[] =
    " <script type=\"text/javascript\" src=\"/swf/flowplayer.js\"><!-- IE bug --></script>\n";

void MediaServer::enableHtml5(bool enabled)
{
  html5Enabled = enabled;
}

QByteArray MediaServer::buildPages(const QString &path)
{
  QByteArray result;

  QByteArray fullPath = serverPath();
  HtmlParser htmlParser;
  htmlParser.setField("ITEM_LINK", fullPath);
  htmlParser.setField("ITEM_NAME", serverName());
  result += htmlParser.parse(htmlPageItem);

  const int lastSlash = path.lastIndexOf('/');
  if (lastSlash > 0)
  {
    const QStringList dirs = path.left(lastSlash).split('/', QString::SkipEmptyParts);

    for (int i=0; i<dirs.count(); i++)
    {
      htmlParser.setField("ITEM_NAME", QByteArray(">"));
      result += htmlParser.parse(htmlPageSeparator);

      fullPath += dirs[i] + "/";
      htmlParser.setField("ITEM_LINK", fullPath);
      htmlParser.setField("ITEM_NAME", dirs[i]);
      result += htmlParser.parse(htmlPageItem);
    }
  }

  return result;
}

QByteArray MediaServer::buildThumbnailView(const QString &dirPath, const ThumbnailListItemList &items, int start, int total)
{
  HtmlParser htmlParser;

  // Build the page selector
  htmlParser.setField("PAGES", buildPages(dirPath));

  const unsigned numPages = (total + (itemsPerThumbnailPage - 1)) / itemsPerThumbnailPage;
  const unsigned selectedPage = start / itemsPerThumbnailPage;

  if (numPages > 1)
  {
    htmlParser.setField("ITEM_NAME", QByteArray("("));
    htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));

    for (unsigned i=0; i<numPages; i++)
    {
      htmlParser.setField("ITEM_LINK", "?start=" + QString::number(i * itemsPerThumbnailPage));
      htmlParser.setField("ITEM_NAME", QString::number(i+1));

      if (i != selectedPage)
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageItem));
      else
        htmlParser.appendField("PAGES", htmlParser.parse(htmlPageItem));
    }

    htmlParser.setField("ITEM_NAME", QByteArray(")"));
    htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));
  }

  // Build the content
  htmlParser.setField("ITEMS", QByteArray(""));
  foreach (const ThumbnailListItem &item, items)
  {
    htmlParser.setField("ITEM_TITLE", item.title);
    htmlParser.setField("ITEM_SUBTITLE", item.subtitle);
    htmlParser.setField("ITEM_ICONURL", item.iconurl.toString());
    htmlParser.setField("ITEM_URL", item.url.toString());
    htmlParser.appendField("ITEMS", htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoTitle : htmlThumbnailItem));
  }

  return htmlParser.parse(htmlThumbnailList);
}

QByteArray MediaServer::buildDetailedView(const QString &dirPath, const QStringList &columns, const DetailedListItemList &items)
{
  HtmlParser htmlParser;

  // Build the page selector
  htmlParser.setField("PAGES", buildPages(dirPath));

  // Build the table head
  htmlParser.setField("ITEMS", QByteArray(""));
  htmlParser.setField("COLUMNS", QByteArray(""));
  htmlParser.setField("ROW_CLASS", QByteArray("listitem"));

  htmlParser.setField("ITEM_TITLE", QByteArray(""));
  htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListHead)); // Icon column

  foreach (const QString &column, columns)
  {
    htmlParser.setField("ITEM_TITLE", column);
    htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListHead));
  }

  htmlParser.appendField("ITEMS", htmlParser.parse(htmlDetailedListRow));

  // Build the content
  int itemNum = 0;
  foreach (const DetailedListItem &item, items)
  {
    htmlParser.setField("COLUMNS", QByteArray(""));
    htmlParser.setField("ROW_CLASS", QByteArray((itemNum++ & 1) ? "listaltitem" : "listitem"));

    for (int i=0; i<item.columns.count(); i++)
    {
      htmlParser.setField("ITEM_TITLE", item.columns[i]);
      if (i == 0)
      {
        htmlParser.setField("ITEM_ICONURL", item.iconurl.toString());
        htmlParser.setField("ITEM_URL", item.url.toString());
        htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListIcon));
        htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumnLink));
      }
      else
        htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumn));
    }

    htmlParser.appendField("ITEMS", htmlParser.parse(htmlDetailedListRow));
  }

  return htmlParser.parse(htmlDetailedList);
}

QByteArray MediaServer::buildVideoPlayer(const QString &dirPath, const QByteArray &item, const QString &title, const SMediaInfo::Program &program, const QUrl &url, const QSize &size, SAudioFormat::Channels channels)
{
  HtmlParser htmlParser;

  // Build the page selector
  htmlParser.setField("PAGES", buildPages(dirPath));
  htmlParser.setField("ITEM_NAME", QByteArray(">"));
  htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));
  htmlParser.setField("ITEM_NAME", title);
  htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));

  // Build the player
  htmlParser.setField("CLEAN_TITLE", SStringParser::toCleanName(title));
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
  htmlParser.setField("LANGUAGE", url.queryItemValue("language"));

  QByteArray query =
      "?size=" + QByteArray::number(size.width()) + "x" + QByteArray::number(size.height()) +
      "&channels=" + QByteArray::number(channels, 16) +
      "&language=" + url.queryItemValue("language").toAscii() +
      "&subtitles=" + url.queryItemValue("subtitles").toAscii() +
      "&position=" + url.queryItemValue("position").toAscii();
  htmlParser.setField("QUERYX", "?query=" + query.toHex().toUpper());
  htmlParser.setField("QUERY", query.replace("&", "&amp;"));

  int count = 1;
  htmlParser.setField("LANGUAGES", QByteArray(""));
  htmlParser.setField("SELECTED", QByteArray(""));
  foreach (const SMediaInfo::AudioStreamInfo &stream, program.audioStreams)
  {
    QString title = stream.title + " ";
    if (stream.language[0] != 0)
      title += " " + SStringParser::iso639Language(stream.language);
    else
      title += " " + tr("Unknown");

    htmlParser.setField("VALUE", QByteArray::number(stream, 16));
    htmlParser.setField("TEXT", QString::number(count++) + ". " + title.simplified());
    htmlParser.appendField("LANGUAGES", htmlParser.parse(htmlPlayerThumbItemOption));
  }

  count = 1;
  htmlParser.setField("SUBTITLES", QByteArray(""));
  htmlParser.setField("SELECTED", QByteArray(""));
  foreach (const SMediaInfo::DataStreamInfo &stream, program.dataStreams)
  {
    QString title = stream.title + " ";
    if (stream.language[0] != 0)
      title += " " + SStringParser::iso639Language(stream.language);
    else
      title += " " + tr("Unknown");

    htmlParser.setField("VALUE", QByteArray::number(stream, 16));
    htmlParser.setField("TEXT", QString::number(count++) + ". " + title.simplified());
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
  if (!program.chapters.isEmpty())
  {
    htmlParser.setField("VALUE", QByteArray::number(0));
    htmlParser.setField("TEXT", tr("Begin") + ", " + QTime().addSecs(0).toString(videoTimeFormat));
    htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));

    int chapterNum = 1;
    foreach (const SMediaInfo::Chapter &chapter, program.chapters)
    {
      htmlParser.setField("VALUE", QByteArray::number(chapter.begin.toSec()));
      htmlParser.setField("TEXT", tr("Chapter") + " " + QString::number(chapterNum++));
      if (!chapter.title.isEmpty())
        htmlParser.appendField("TEXT", ", " + chapter.title);

      htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
    }
  }
  else if (program.duration.isValid())
  {
    for (int i=0, n=program.duration.toSec(); i<n; i+=seekBySecs)
    {
      htmlParser.setField("VALUE", QByteArray::number(i));
      htmlParser.setField("TEXT", QTime().addSecs(i).toString(videoTimeFormat));
      htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
    }
  }
  else
  {
    htmlParser.setField("VALUE", QByteArray("0"));
    htmlParser.setField("TEXT", QTime().addSecs(0).toString(videoTimeFormat));
    htmlParser.appendField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));
  }

  if (url.hasQueryItem("play"))
  {
    if (html5Enabled)
    {
      htmlParser.setField("FLV_PLAYER", htmlParser.parse(htmlPlayerVideoItem));
      return htmlParser.parse(htmlPlayerVideoItemHtml5);
    }
    else
      return htmlParser.parse(htmlPlayerVideoItem);
  }
  else
    return htmlParser.parse(htmlPlayerThumbItem);
}

QByteArray MediaServer::buildVideoPlayer(const QString &dirPath, const QByteArray &item, const QString &title, const QUrl &url, const QSize &size, SAudioFormat::Channels channels)
{
  HtmlParser htmlParser;

  // Build the page selector
  htmlParser.setField("PAGES", buildPages(dirPath));
  htmlParser.setField("ITEM_NAME", QByteArray(">"));
  htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));
  htmlParser.setField("ITEM_NAME", title);
  htmlParser.appendField("PAGES", htmlParser.parse(htmlPageSeparator));

  // Build the player
  htmlParser.setField("CLEAN_TITLE", SStringParser::toCleanName(title));
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
  htmlParser.setField("LANGUAGE", url.queryItemValue("language"));

  QByteArray query =
      "?size=" + QByteArray::number(size.width()) + "x" + QByteArray::number(size.height()) +
      "&channels=" + QByteArray::number(channels, 16);
  htmlParser.setField("QUERYX", "?query=" + query.toHex().toUpper());
  htmlParser.setField("QUERY", query.replace("&", "&amp;"));

  htmlParser.setField("SELECTED", QByteArray(""));

  htmlParser.setField("VALUE", QByteArray(""));
  htmlParser.setField("TEXT", "1. " + tr("Unknown"));
  htmlParser.setField("LANGUAGES", htmlParser.parse(htmlPlayerThumbItemOption));

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

  htmlParser.setField("VALUE", QByteArray("0"));
  htmlParser.setField("TEXT", QTime().addSecs(0).toString(videoTimeFormat));
  htmlParser.setField("CHAPTERS", htmlParser.parse(htmlPlayerThumbItemOption));

  if (url.hasQueryItem("play"))
  {
    if (html5Enabled)
    {
      htmlParser.setField("FLV_PLAYER", htmlParser.parse(htmlPlayerVideoItem));
      return htmlParser.parse(htmlPlayerVideoItemHtml5);
    }
    else
      return htmlParser.parse(htmlPlayerVideoItem);
  }
  else
    return htmlParser.parse(htmlPlayerThumbItem);
}


} // End of namespace
