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
const char          MediaServer::audioTimeFormat[] = "m:ss";
const char          MediaServer::videoTimeFormat[] = "h:mm:ss";

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
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"thumbnaillist\" id=\"items\">\n"
    "  </div>\n"
    "  <script type=\"text/javascript\">loadListContent(\"items\", 0, 128);</script>\n"
    " </div>\n";

const char MediaServer::htmlThumbnailItem[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "      <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "     </a>\n"
    "    </div>\n"
    "    <div class=\"title\">{ITEM_TITLE}</div>\n"
    "    <div class=\"subtitle\">{ITEM_SUBTITLE}</div>\n"
    "   </div>\n";

const char MediaServer::htmlThumbnailItemNoTitle[] =
    "   <div class=\"thumbnaillistitem\">\n"
    "    <div class=\"thumbnail\">\n"
    "     <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "      <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "     </a>\n"
    "    </div>\n"
    "   </div>\n";

const char MediaServer::htmlDetailedList[] =
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <table class=\"detailedlist\">\n"
    "{ITEMS}"
    "  </table>\n"
    " </div>\n";

const char MediaServer::htmlDetailedLoader[] =
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <table class=\"detailedlist\" id=\"items\">\n"
    "{HEADROW}"
    "  </table>\n"
    "  <script type=\"text/javascript\">loadListContent(\"items\", 0, 128);</script>\n"
    " </div>\n";

const char MediaServer::htmlDetailedListRow[] =
    "   <tr>\n"
    "{COLUMNS}"
    "   </tr>\n";

const char MediaServer::htmlDetailedListHead[] =
    "    <th class=\"{ITEM_CLASS}\">{ITEM_TITLE}</th>\n";

const char MediaServer::htmlDetailedListColumn[] =
    "    <td class=\"{ITEM_CLASS}\">{ITEM_TITLE}</td>\n";

const char MediaServer::htmlDetailedListColumnIcon[] =
    "    <td class=\"{ITEM_CLASS}\"><img src=\"{ITEM_ICONURL}\" alt=\"..\" />{ITEM_TITLE}</td>\n";

const char MediaServer::htmlDetailedListColumnLink[] =
    "    <td class=\"{ITEM_CLASS}\"><a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">{ITEM_TITLE}</a></td>\n";

const char MediaServer::htmlDetailedListColumnLinkIcon[] =
    "    <td class=\"{ITEM_CLASS}\"><a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\"><img src=\"{ITEM_ICONURL}\" alt=\"..\" />{ITEM_TITLE}</a></td>\n";

const char MediaServer::htmlPlayerAudioItem[] =
    " <div class=\"menu\">\n"
    "  <ul>\n"
    "   <li>{TR_DETAILS}</li>\n"
    "{DETAILS}"
    "  </ul>\n"
    " </div>\n"
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"player\">\n"
    "{PLAYER}"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlPlayerAudioItemHtml5[] =
    "   <audio autoplay=\"autoplay\" controls=\"controls\">\n"
    "{SOURCES}"
    "{FLV_PLAYER}"
    "   </audio>\n";

const char MediaServer::htmlPlayerAudioSourceItemHtml5[] =
    "    <source src='{SOURCE_URL}' type='{SOURCE_CODEC}' />\n";

const char MediaServer::htmlPlayerAudioItemFlv[] =
    "    <div id=\"player\" style=\"display:block;height:{HEIGHT}px;\" href=\"{PLAYER_URL}\"></div>\n"
    "    <script type=\"text/javascript\"><!--\n"
    "     flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "       clip: { autoPlay: true }\n"
    "      } );\n"
    "     //-->\n"
    "    </script>\n";

const char MediaServer::htmlPlayerVideoItem[] =
    " <div class=\"menu\">\n"
    "  <ul>\n"
    "   <li>{TR_DETAILS}</li>\n"
    "{DETAILS}"
    "  </ul>\n"
    " </div>\n"
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"player\">\n"
    "{PLAYER}"
    "  </div>\n"
    " </div>\n";

const char MediaServer::htmlPlayerVideoItemHtml5[] =
    "   <video width=\"{WIDTH}\" height=\"{HEIGHT}\" autoplay=\"autoplay\" controls=\"controls\" poster=\"{PLAYER_ITEM}-thumb.png?resolution={WIDTH}x{HEIGHT}\">\n"
    "{SOURCES}"
    "{FLV_PLAYER}"
    "   </video>\n";

const char MediaServer::htmlPlayerVideoSourceItemHtml5[] =
    "    <source src='{SOURCE_URL}' type='{SOURCE_CODEC}' />\n";

const char MediaServer::htmlPlayerVideoItemFlv[] =
    "    <div id=\"player\" style=\"display:block;width:{WIDTH}px;height:{HEIGHT}px;\" href=\"{PLAYER_URL}\"></div>\n"
    "    <script type=\"text/javascript\">\n"
    "     flowplayer(\"player\", \"/swf/flowplayer.swf\", {\n"
    "       clip: { scaling: 'fit', accelerated: true, autoPlay: true }\n"
    "      } );\n"
    "    </script>\n";

const char MediaServer::htmlPlayerThumbItem[] =
    " <div class=\"menu\">\n"
    "  <ul>\n"
    "   <li>{TR_DETAILS}</li>\n"
    "{DETAILS}"
    "  </ul>\n"
    " </div>\n"
    " <div class=\"content\">\n"
    "  <h1>{TITLE}</h1>\n"
    "  <div class=\"player\">\n"
    "   <div style=\"padding:0;margin:0;display:table-cell;"
                    "width:{WIDTH}px;height:{HEIGHT}px;vertical-align:middle;"
                    "background-image:url('{PLAYER_ITEM}-thumb.png?resolution={WIDTH}x{HEIGHT}');"
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
    "     <form name=\"dlnasettings\" action=\"{ENCODED_TITLE}.mpeg\" method=\"get\">\n"
    "      <input type=\"hidden\" name=\"item\" value=\"{PLAYER_ITEM}\" />\n"
    "      <input type=\"hidden\" name=\"encodemode\" value=\"slow\" />\n"
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
    "      <select name=\"resolution\">\n"
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

const char MediaServer::htmlDetail[] =
    "   <li>{ITEM_NAME}: {ITEM_VALUE}</li>\n";


const char MediaServer::headList[] =
    " <script type=\"text/javascript\" src=\"/js/dynamiclist.js\"></script>\n"; // Open and close tag due to IE bug

const char MediaServer::headPlayer[] =
    " <script type=\"text/javascript\" src=\"/swf/flowplayer.js\"></script>\n"; // Open and close tag due to IE bug

void MediaServer::enableHtml5(bool enabled)
{
  html5Enabled = enabled;
}

QString MediaServer::audioFormatString(const SMediaInfo::Program &program)
{
  QString result;
  foreach (const SMediaInfo::AudioStreamInfo &stream, program.audioStreams)
  {
    const QString setupName = SAudioFormat::channelSetupName(stream.codec.channelSetup());
    if (!setupName.isEmpty())
      result += ", " + setupName;
  }

  return result.isEmpty() ? tr("Unknown") : result.mid(2);
}

QString MediaServer::videoFormatString(const SMediaInfo::Program &program)
{
  QString result;
  foreach (const SMediaInfo::VideoStreamInfo &stream, program.videoStreams)
  {
    if (!stream.codec.size().isNull())
    {
      result += ", " + QString::number(stream.codec.size().width()) +
                " x " + QString::number(stream.codec.size().height());

      if (stream.codec.frameRate().isValid())
        result +=  " @ " + QString::number(stream.codec.frameRate().toFrequency(), 'f', 2) + " fps";
    }
  }

  return result.isEmpty() ? tr("Unknown") : result.mid(2);
}

QByteArray MediaServer::buildThumbnailView(const QString &title, const ThumbnailListItemList &items)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);
  htmlParser.setField("ITEMS", buildThumbnailItems(items));
  return htmlParser.parse(htmlThumbnailList);
}

QByteArray MediaServer::buildThumbnailLoader(const QString &title)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);
  return htmlParser.parse(htmlThumbnailLoader);
}

QByteArray MediaServer::buildThumbnailItems(const ThumbnailListItemList &items)
{
  HtmlParser htmlParser;

  QByteArray result;
  foreach (const ThumbnailListItem &item, items)
  {
    htmlParser.setField("ITEM_TITLE", item.title);
    htmlParser.setField("ITEM_SUBTITLE", item.subtitle);
    htmlParser.setField("ITEM_ICONURL", item.iconurl.toEncoded());
    htmlParser.setField("ITEM_URL", item.url.toEncoded());
    result += htmlParser.parse(item.title.isEmpty() ? htmlThumbnailItemNoTitle : htmlThumbnailItem);
  }

  return result;
}

QByteArray MediaServer::buildDetailedView(const QString &title, const QList< QPair<QString, bool> > &columns, const DetailedListItemList &items)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);

  // Build the table head
  htmlParser.setField("COLUMNS", QByteArray(""));
  for (int i=0; i<columns.count(); i++)
  {
    htmlParser.setField("ITEM_TITLE", columns[i].first);
    htmlParser.setField("ITEM_CLASS", QByteArray(columns[i].second ? "stretch" : "nostretch"));
    htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListHead));
  }

  htmlParser.setField("ITEMS", htmlParser.parse(htmlDetailedListRow) + buildDetailedItems(items));

  return htmlParser.parse(htmlDetailedList);
}

QByteArray MediaServer::buildDetailedLoader(const QString &title, const QList< QPair<QString, bool> > &columns)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);

  // Build the table head
  htmlParser.setField("COLUMNS", QByteArray(""));
  for (int i=0; i<columns.count(); i++)
  {
    htmlParser.setField("ITEM_TITLE", columns[i].first);
    htmlParser.setField("ITEM_CLASS", QByteArray(columns[i].second ? "stretch" : "nostretch"));
    htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListHead));
  }

  htmlParser.setField("HEADROW", htmlParser.parse(htmlDetailedListRow));

  return htmlParser.parse(htmlDetailedLoader);
}

QByteArray MediaServer::buildDetailedItems(const DetailedListItemList &items)
{
  HtmlParser htmlParser;

  QByteArray result;
  for (int i=0; i<items.count(); i++)
  {
    htmlParser.setField("COLUMNS", QByteArray(""));
    for (int j=0; j<items[i].columns.count(); j++)
    {
      htmlParser.setField("ITEM_TITLE", items[i].columns[j].title);
      htmlParser.setField("ITEM_CLASS", QByteArray(i & 1 ? "nostretch_b" : "nostretch_a"));
      htmlParser.setField("ITEM_ICONURL", items[i].columns[j].iconurl.toEncoded());
      htmlParser.setField("ITEM_URL", items[i].columns[j].url.toEncoded());

      if (!items[i].columns[j].url.isEmpty())
      {
        if (!items[i].columns[j].iconurl.isEmpty())
          htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumnLinkIcon));
        else
          htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumnLink));
      }
      else
      {
        if (!items[i].columns[j].iconurl.isEmpty())
          htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumnIcon));
        else
          htmlParser.appendField("COLUMNS", htmlParser.parse(htmlDetailedListColumn));
      }
    }

    result += htmlParser.parse(htmlDetailedListRow);
  }

  return result;
}

QByteArray MediaServer::buildVideoPlayer(const QByteArray &item, const QString &title, const SMediaInfo::Program &program, const QUrl &url, const QSize &size, SAudioFormat::Channels channels)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);
  htmlParser.setField("TR_DETAILS", tr("Details"));

  htmlParser.setField("DETAILS", QByteArray(""));
  htmlParser.setField("ITEM_NAME", tr("Duration"));
  htmlParser.setField("ITEM_VALUE", QTime().addSecs(program.duration.toSec()).toString("h:mm:ss"));
  htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));
  htmlParser.setField("ITEM_NAME", tr("Audio"));
  htmlParser.setField("ITEM_VALUE", audioFormatString(program));
  htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));
  htmlParser.setField("ITEM_NAME", tr("Video"));
  htmlParser.setField("ITEM_VALUE", videoFormatString(program));
  htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));

  htmlParser.setField("ENCODED_TITLE", SStringParser::toCleanName(title).toUtf8().toPercentEncoding());
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
    const QByteArray query =
        "resolution=" + QByteArray::number(size.width()) + "x" + QByteArray::number(size.height()) +
        "&channels=" + QByteArray::number(channels, 16) +
        "&language=" + url.queryItemValue("language").toAscii() +
        "&subtitles=" + url.queryItemValue("subtitles").toAscii() +
        "&position=" + url.queryItemValue("position").toAscii();

    if (html5Enabled)
    {
      htmlParser.setField("PLAYER_URL", SUPnPContentDirectory::toQueryPath(item, query, ".flv"));
      htmlParser.setField("FLV_PLAYER", htmlParser.parse(htmlPlayerVideoItemFlv));

      htmlParser.setField("SOURCE_URL", SUPnPContentDirectory::toQueryPath(item, query, ".ogg"));
      htmlParser.setField("SOURCE_CODEC", QByteArray("video/ogg; codecs=\"theora, flac\""));
      htmlParser.setField("SOURCES", htmlParser.parse(htmlPlayerVideoSourceItemHtml5));
      htmlParser.setField("PLAYER", htmlParser.parse(htmlPlayerVideoItemHtml5));
    }
    else
    {
      htmlParser.setField("PLAYER_URL", SUPnPContentDirectory::toQueryPath(item, query, ".flv"));
      htmlParser.setField("PLAYER", htmlParser.parse(htmlPlayerVideoItemFlv));
    }

    return htmlParser.parse(htmlPlayerVideoItem);
  }
  else
    return htmlParser.parse(htmlPlayerThumbItem);
}

QByteArray MediaServer::buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &url, const QSize &size, SAudioFormat::Channels channels)
{
  HtmlParser htmlParser;
  htmlParser.setField("TITLE", title);
  htmlParser.setField("TR_DETAILS", tr("Details"));

  htmlParser.setField("DETAILS", QByteArray(""));
  htmlParser.setField("ITEM_NAME", tr("Audio"));
  htmlParser.setField("ITEM_VALUE", QString(SAudioFormat::channelSetupName(channels)));
  htmlParser.appendField("DETAILS", htmlParser.parse(htmlDetail));

  // Build the player
  htmlParser.setField("CLEAN_TITLE", SStringParser::toCleanName(title).toUtf8().toPercentEncoding());
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
    const QByteArray query =
        "resolution=" + QByteArray::number(size.width()) + "x" + QByteArray::number(size.height()) +
        "&channels=" + QByteArray::number(channels, 16);

    if (html5Enabled)
    {
      htmlParser.setField("PLAYER_URL", SUPnPContentDirectory::toQueryPath(item, query, ".flv"));
      htmlParser.setField("FLV_PLAYER", htmlParser.parse(htmlPlayerVideoItemFlv));

      htmlParser.setField("SOURCE_URL", SUPnPContentDirectory::toQueryPath(item, query, ".ogg"));
      htmlParser.setField("SOURCE_CODEC", QByteArray("video/ogg; codecs=\"theora, flac\""));
      htmlParser.setField("SOURCES", htmlParser.parse(htmlPlayerVideoSourceItemHtml5));
      htmlParser.setField("PLAYER", htmlParser.parse(htmlPlayerVideoItemHtml5));
    }
    else
    {
      htmlParser.setField("PLAYER_URL", SUPnPContentDirectory::toQueryPath(item, query, ".flv"));
      htmlParser.setField("PLAYER", htmlParser.parse(htmlPlayerVideoItemFlv));
    }

    return htmlParser.parse(htmlPlayerVideoItem);
  }
  else
    return htmlParser.parse(htmlPlayerThumbItem);
}


} // End of namespace
