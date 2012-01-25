/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "televisionserver.h"
#include "televisionbackend.h"

namespace LXiMediaCenter {

const int   TelevisionServer::epgNumCols = 36;
const int   TelevisionServer::epgSecsPerCol = 300;
const float TelevisionServer::epgColWidth = 1.0f;
const float TelevisionServer::epgRowHeight = 2.5f;
const float TelevisionServer::epgChannelNameWidth = 10.0f;
const float TelevisionServer::epgCellPadding = 0.1f;
const float TelevisionServer::epgCellSpacing = 0.1f;

const char * const TelevisionServer::cssEpg =
    "table.epg {\n"
    "  width: {EPG_CHANNELS_WIDTH}em;\n"
    "  border-spacing: {EPG_CHANNELS_SPACING}em;\n"
    "  margin-left: auto;\n"
    "  margin-right: auto;\n"
    "  border: none;\n"
    "}\n"
    "\n"
    "tr.epg {\n"
    "  vertical-align: top;\n"
    "}\n"
    "\n"
    "td.epg {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: left;\n"
    "}\n"
    "\n"
    "td.epghead {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: center;\n"
    "  color: {_PALETTE_TEXT};\n"
    "  background-color: {_PALETTE_ALTBASE};\n"
    "}\n"
    "\n"
    "td.epgchannel {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: left;\n"
    "  color: {_PALETTE_TEXT};\n"
    "  background-color: {_PALETTE_ALTBASE};\n"
    "}\n"
    "\n"
    "td.epgnoprogramme {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: center;\n"
    "  color: {_PALETTE_BUTTONTEXT_LIGHT};\n"
    "  background-color: {_PALETTE_BUTTON};\n"
    "}\n"
    "\n"
    "td.epgprogramme {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: center;\n"
    "  color: {_PALETTE_BUTTONTEXT};\n"
    "  background-color: {_PALETTE_BUTTON};\n"
    "}\n"
    "\n"
    "td.epgprogrammerec {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: center;\n"
    "  color: {_PALETTE_BUTTONTEXT};\n"
    "  background-color: {_PALETTE_BUTTON_RED};\n"
    "}\n"
    "\n"
    "td.epgprogrammenow {\n"
    "  padding: {EPG_CHANNELS_PADDING}em;\n"
    "  text-align: center;\n"
    "  color: {_PALETTE_BUTTONTEXT};\n"
    "  background-color: {_PALETTE_BUTTON_GREEN};\n"
    "}\n"
    "\n"
    "a.epgprogramme {\n"
    "  color: {_PALETTE_BUTTONTEXT};\n"
    "  text-decoration: none;\n"
    "}\n";

const char * const TelevisionServer::htmlEpgMain =
    " <table class=\"widgets\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"50%\" rowspan=\"2\">\n"
    "    <p class=\"head\">{TR_EPG}</p>\n"
    "    <table class=\"epg\">\n"
    "     <tr class=\"epg\"><td class=\"epghead\" colspan=\"{EPG_ALLCOLS}\">\n"
    "{EPG_DAYS}"
    "     </td></tr>\n"
    "     <tr class=\"epg\"><td class=\"epghead\" colspan=\"{EPG_ALLCOLS}\">\n"
    "{EPG_TIMES}"
    "     </td></tr>\n"
    "     <tr class=\"epg\">\n"
    "      <td class=\"epg\" width=\"{EPG_CHANNEL_NAME_WIDTH}\"></td>\n"
    "{EPG_TIME_SPACERS}"
    "     </tr>\n"
    "{EPG_CHANNELS}"
    "    </table>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_RECORDED}</p>\n"
    "{EPG_RECORDED}"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"nowidget\" width=\"50%\"></td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const TelevisionServer::htmlEpgTime =
    "      <a class=\"widget\" href=\"{ITEM_LINK}\">{ITEM_TEXT}</a>\n";

const char * const TelevisionServer::htmlEpgTimeSpacer =
    "      <td class=\"epg\" style=\"width: {ITEM_WIDTH}em\">\n"
    "      </td>\n";

const char * const TelevisionServer::htmlEpgHeadRow =
    "     <tr class=\"epg\">\n"
    "      <td class=\"epg\">{HEAD_DAY}</td>\n"
    "{HEAD_COLS}"
    "     </tr>\n";

const char * const TelevisionServer::htmlEpgHeadCol =
    "      <td class=\"epg\" colspan=\"{ITEM_NUMCOLS}\" style=\"width: {ITEM_WIDTH}em\">{ITEM_TEXT}</td>\n";

const char * const TelevisionServer::htmlEpgChannel =
    "     <tr class=\"epg\">\n"
    "      <td class=\"epgchannel\">\n"
    "       <div style=\"width: {EPG_CHANNEL_NAME_WIDTH_EP}em; height: {EPG_CHANNEL_ROW_HEIGHT}em; overflow: hidden\">\n"
    "        {CHANNEL_PRESET}. {CHANNEL_NAME}\n"
    "       </div>\n"
    "      </td>\n"
    "{PROGRAMMES}"
    "     </tr>\n";

const char * const TelevisionServer::htmlEpgProgramme =
    "      <td class=\"{ITEM_CLASS}\" colspan=\"{ITEM_NUMCOLS}\">\n"
    "       <div style=\"width: {ITEM_WIDTH_EP}em; height: {EPG_CHANNEL_ROW_HEIGHT}em; overflow: hidden\">\n"
    "        <a class=\"epgprogramme\" title=\"{ITEM_TIME} {ITEM_TEXT}\" href=\"{ITEM_LINK}\">{ITEM_TEXT}</a>\n"
    "       </div>\n"
    "      </td>\n";

const char * const TelevisionServer::htmlEpgNoProgramme =
    "      <td class=\"epgnoprogramme\" colspan=\"{ITEM_NUMCOLS}\">\n"
    "       <div style=\"width: {ITEM_WIDTH_EP}em; height: {EPG_CHANNEL_ROW_HEIGHT}em; overflow: hidden\">\n"
    "        {ITEM_TEXT}\n"
    "       </div>\n"
    "      </td>\n";

const char * const TelevisionServer::htmlProgrammeItem =
    "       <tr valign=\"top\">\n"
    "        <td {PROGRAMME_CLASS} align=\"left\" width=\"10%\">\{PROGRAMME_TIME}</td>\n"
    "        <td align=\"left\" width=\"65%\">\n"
    "         <a href=\"{PROGRAMME_ID}.html\">\n"
    "          <div title=\"{PROGRAMME_DESCRIPTION}\">{PROGRAMME_NAME}</div>\n"
    "         </a>\n"
    "        </td>\n"
    "        <td align=\"right\" width=\"25%\">{PROGRAMME_STATUS}</td>\n"
    "       </tr>\n";

const char * const TelevisionServer::htmlProgramme =
    "<table width=\"100%\" cellpadding=\"2\" cellspacing=\"1\" border=\"0\">\n"
    " <tr valign=\"top\">\n"
    "  <th width=\"100%\" colspan=\"3\">{PROGRAMME_TITLE}</th>\n"
    " </tr>\n"
    " <tr valign=\"top\">\n"
    "  <td class=\"accent\" width=\"20%\">\n"
    "   <b>{TR_CHANNEL}:</b><br />\n"
    "   {PROGRAMME_CHANNEL}<br /><br />\n"
    "   <b>{TR_DATE}:</b><br />\n"
    "   {PROGRAMME_DATE}<br /><br />\n"
    "   <b>{TR_TIME}:</b><br />\n"
    "   {PROGRAMME_TIME}<br /><br />\n"
    "   <b>{TR_CATEGORY}:</b><br />\n"
    "   {PROGRAMME_CATEGORY}<br /><br />\n"
    "   <b>{TR_POPULARITY}:</b><br />\n"
    "   {PROGRAMME_POPULARITY}<br /><br />\n"
    "   <b>{TR_THIS_PROGRAMME}:</b><br />\n"
    "{PROGRAMME_ACTIONS}"
    "   <br />\n"
    "  </td>\n"
    "  <td class=\"transparent\" width=\"40%\">\n"
    "   {PROGRAMME_DESCRIPTION}\n"
    "  </td>\n"
    "  <td class=\"transparent\" width=\"40%\" align=\"right\">\n"
    "   <img src=\"{PROGRAMME_ID}-thumb.jpeg\" alt=\"{PROGRAMME_TITLE}\" width = \"100%\" />\n"
    "  </td>\n"
    " </tr>\n"
    "</table>\n";

const char * const TelevisionServer::htmlProgrammeAction =
    "   <a href=\"{PROGRAMME_ACTION_LINK}\">{PROGRAMME_ACTION_NAME}</a><br />\n";

const char * const TelevisionServer::htmlDays =
    " <tr valign=\"top\">\n"
    "  <td width=\"100%\">{DAYS}</th>\n"
    " </tr>\n";

const char * const TelevisionServer::htmlDaysItem =
    " <a href=\"{ITEM_LINK}\">{ITEM_NAME}</a>";

const char * const TelevisionServer::htmlDaysCurrentItem =
    " <b>{ITEM_NAME}</b>";


bool TelevisionServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  PluginSettings settings(plugin);

  QReadLocker l(&lock);

  HtmlParser htmlParser;
  htmlParser.setField("TR_EPG", tr("Electronic Programme Guide"));
  htmlParser.setField("TR_RECORDED", tr("Recorded programmes"));

  htmlParser.setField("EPG_CHANNELS_WIDTH", QByteArray::number((epgChannelNameWidth - epgCellSpacing) + (epgNumCols * epgColWidth), 'f', 1));
  htmlParser.setField("EPG_CHANNELS_PADDING", QByteArray::number(epgCellPadding, 'f', 1));
  htmlParser.setField("EPG_CHANNELS_SPACING", QByteArray::number(epgCellSpacing, 'f', 1));
  htmlParser.setField("EPG_CHANNEL_ROW_HEIGHT", QByteArray::number(epgRowHeight, 'f', 1));
  htmlParser.setField("EPG_CHANNEL_NAME_WIDTH", QByteArray::number(epgChannelNameWidth - epgCellSpacing, 'f', 1));
  htmlParser.setField("EPG_CHANNEL_NAME_WIDTH_EP", QByteArray::number((epgChannelNameWidth - epgCellSpacing) - (epgCellPadding * 2), 'f', 1));
  htmlParser.setField("EPG_ALLCOLS", QByteArray::number(epgNumCols + 1));

  QMap<int, QString> channels;
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
    channels[group.mid(8 + typeName(Type_Television).length()).toInt()] = group;

  const QDateTime now = QDateTime::currentDateTime().toUTC();

  if (file == "epg.css")
  {
    QHttpResponseHeader response(200);
    response.setContentType("text/css;charset=utf-8");
    response.setValue("Cache-Control", "max-age=1800");

    socket->write(response.toString().toUtf8());
    socket->write(htmlParser.parse(cssEpg));
    return false;
  }
  else if (file.endsWith(".html"))
  {
    QString rawChannel;
    QDateTime date;
    if (fromID(file.left(file.length() - 5), rawChannel, date) == false)
    {
      socket->write(QHttpResponseHeader(404).toString().toUtf8());
      return false;
    }

    if (date.isValid())
    {
      const EpgDatabase::Programme programme = epgDatabase->getProgramme(rawChannel, date);
      const EpgDatabase::Programme nextProgramme = epgDatabase->getNextProgramme(rawChannel, programme.utcDate);

      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Programme"));
      htmlParser.setField("ITEM_VALUE", programme.name);
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Channel"));
      htmlParser.setField("ITEM_VALUE", epgDatabase->getChannelName(rawChannel));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Time"));
      htmlParser.setField("ITEM_VALUE", programme.utcDate.toLocalTime().toString(searchDateFormat) + " " +
                                        programme.utcDate.toLocalTime().toString(searchTimeFormat) + " - " +
                                        nextProgramme.utcDate.toLocalTime().toString(searchTimeFormat));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Category"));
      htmlParser.setField("ITEM_VALUE", programme.category);
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Popularity"));
//      htmlParser.setField("ITEM_VALUE", QByteArray::number(epgAnalyzer ? epgAnalyzer->popularity(rawChannel, programme) * 100.0 : 0.0, 'f', 1) + " %");
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));

      htmlParser.setField("PLAYER_DESCRIPTION_NAME", tr("Description"));
      htmlParser.setField("PLAYER_DESCRIPTION", programme.description);

      htmlParser.setField("ITEM_NAME", tr("This programme"));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoActionHead));

      if (programme.utcDate > now)
      {
        htmlParser.setField("ITEM_LINK", "?record=" + toID(rawChannel, programme.utcDate, programme.stationDate.time()));
        htmlParser.setField("ITEM_NAME", programme.recordPriority ? tr("Cancel record") : tr("Schedule record"));
        htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoAction));
      }

      bool hasVideoItem = false;
      if (programme.utcDate < now)
      {
        const QList<EpgDatabase::Record> records = epgDatabase->getRecords(rawChannel, programme.utcDate, nextProgramme.utcDate);
        if (!records.isEmpty())
        {
          bool hasAllFiles = true;
          foreach (const EpgDatabase::Record &record, records)
            hasAllFiles &= timeshiftDir.exists(record.fileName);

          if (hasAllFiles)
          {
            htmlParser.setField("PLAYER_ITEM", toID(rawChannel, programme.utcDate, programme.stationDate.time()));
            htmlParser.setField("PLAYER_ITEM_TEXT", tr("Play in external player"));
            htmlParser.setField("DOWNLOAD_ITEM_TEXT", tr("Download as MPEG file"));
            htmlParser.setField("PLAYER_VIDEOITEMS", htmlParser.parse(htmlPlayerVideoItem));
            hasVideoItem = true;
          }
        }
      }

      if ((programme.utcDate.addSecs(-2 * 60) <= now) && (nextProgramme.utcDate > now))
      {
        if (hasVideoItem)
        {
          htmlParser.setField("ITEM_LINK", toID(rawChannel, QDateTime(), programme.stationDate.time()) + ".html");
          htmlParser.setField("ITEM_NAME", tr("Watch live"));
          htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoAction));
        }
        else
        {
          htmlParser.setField("PLAYER_ITEM", toID(rawChannel, QDateTime(), programme.stationDate.time()));
          htmlParser.setField("PLAYER_ITEM_TEXT", tr("Play in external player"));
          htmlParser.setField("DOWNLOAD_ITEM_TEXT", tr("Download as MPEG file"));
          htmlParser.setField("PLAYER_VIDEOITEMS", htmlParser.parse(htmlPlayerVideoItem));
          hasVideoItem = true;
        }
      }
    }
    else
    {
      htmlParser.setField("PLAYER_INFOITEMS", QByteArray(""));
      htmlParser.setField("ITEM_NAME", tr("Channel"));
      htmlParser.setField("ITEM_VALUE", epgDatabase->getChannelName(rawChannel));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));
      htmlParser.setField("ITEM_NAME", tr("Time"));
      htmlParser.setField("ITEM_VALUE", now.toLocalTime().toString(searchDateFormat) + " " +
                                        now.toLocalTime().toString(searchTimeFormat));
      htmlParser.appendField("PLAYER_INFOITEMS", htmlParser.parse(htmlPlayerInfoItem));

      htmlParser.setField("PLAYER_DESCRIPTION_NAME", tr("Description"));
      htmlParser.setField("PLAYER_DESCRIPTION", QByteArray(""));

      htmlParser.setField("PLAYER_ITEM", toID(rawChannel, QDateTime(), now.toLocalTime().time()));
      htmlParser.setField("PLAYER_ITEM_TEXT", tr("Play in external player"));
      htmlParser.setField("DOWNLOAD_ITEM_TEXT", tr("Download as MPEG file"));
      htmlParser.setField("PLAYER_VIDEOITEMS", htmlParser.parse(htmlPlayerVideoItem));
    }

    l.unlock();

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlPlayer), headPlayer);
  }
  else
  {
    htmlParser.setField("EPG_CHANNELS", QByteArray(""));
    htmlParser.setField("EPG_RECORDED", QByteArray(""));

    // Get the correct time
    QDateTime left = roundTime(now);
    if (url.hasQueryItem("time"))
    {
      left = QDateTime::fromString(url.queryItemValue("time"), Qt::ISODate);
      left.setTimeSpec(Qt::UTC);
    }

    const QDateTime right = left.addSecs(epgNumCols * epgSecsPerCol);
    const int nowColumn = left.secsTo(now) / epgSecsPerCol;


    htmlParser.setField("EPG_DAYS", QByteArray(""));
    for (int i=-2; i<=2; i++)
    if (i != 0)
    {
      const QDateTime date = left.toLocalTime().addDays(i);
      htmlParser.setField("ITEM_LINK", "?time=" + date.toUTC().toString(Qt::ISODate));
      htmlParser.setField("ITEM_TEXT", date.toString("dddd"));
      htmlParser.appendField("EPG_DAYS", htmlParser.parse(htmlEpgTime));
    }

    htmlParser.setField("EPG_TIMES", QByteArray(""));
    for (int i=0; i<24; i+=2)
    {
      const QDateTime date(left.toLocalTime().date(), QTime(i, 0, 0), Qt::LocalTime);
      htmlParser.setField("ITEM_LINK", "?time=" + date.toUTC().toString(Qt::ISODate));
      htmlParser.setField("ITEM_TEXT", date.toString("hh:mm"));
      htmlParser.appendField("EPG_TIMES", htmlParser.parse(htmlEpgTime));
    }

    // Build spacers
    htmlParser.setField("EPG_TIME_SPACERS", QByteArray(""));
    for (int i=0; i<epgNumCols; i++)
    {
      htmlParser.setField("ITEM_WIDTH", QByteArray::number(epgColWidth - epgCellSpacing, 'f', 1));
      htmlParser.appendField("EPG_TIME_SPACERS", htmlParser.parse(htmlEpgTimeSpacer));
    }

    // Build head
    static const int colsPerHeadSection = 1800 / epgSecsPerCol;
    htmlParser.setField("HEAD_DAY", left.toLocalTime().toString("dddd"));
    htmlParser.setField("HEAD_COLS", QByteArray(""));
    for (int i=0; i<(epgNumCols/colsPerHeadSection); i++)
    {
      htmlParser.setField("ITEM_NUMCOLS", QByteArray::number(colsPerHeadSection));
      htmlParser.setField("ITEM_WIDTH", QByteArray::number((epgColWidth * colsPerHeadSection) - epgCellSpacing, 'f', 1));
      htmlParser.setField("ITEM_TEXT", left.toLocalTime().addSecs(i * 1800).toString("hh:mm"));
      htmlParser.appendField("HEAD_COLS", htmlParser.parse(htmlEpgHeadCol));
    }

    const QByteArray headRow = htmlParser.parse(htmlEpgHeadRow);
    static const unsigned rowsPerHead = 4;
    unsigned rowIndex = 0;

    foreach (const QString &group, channels)
    {
      settings.beginGroup(group);
      const QString channelName = settings.value("Name").toString();
      const unsigned preset = group.mid(8 + typeName(Type_Television).length()).toUInt();

      if ((rowIndex++ % rowsPerHead) == 0)
        htmlParser.appendField("EPG_CHANNELS", headRow);

      htmlParser.setField("CHANNEL_PRESET", QByteArray::number(preset));
      htmlParser.setField("CHANNEL_NAME", channelName);
      htmlParser.setField("PROGRAMMES", QByteArray(""));

      int lastColumn = 0;
      QList<EpgDatabase::Programme> programmes = epgDatabase->getProgrammes(channelName, left.addSecs(-4 * 60 * 60), right);
      while (!programmes.isEmpty() && (lastColumn < epgNumCols))
      {
        const EpgDatabase::Programme programme = programmes.takeFirst();
        const int leftColumn = left.secsTo(programme.utcDate) / epgSecsPerCol;
        const int rightColumn = programmes.isEmpty() ? epgNumCols : (left.secsTo(programmes.first().utcDate) / epgSecsPerCol);

        if ((leftColumn < rightColumn) && (rightColumn > lastColumn))
        {
          if (leftColumn > lastColumn)
          {
            const int cols = leftColumn - lastColumn;
            htmlParser.setField("ITEM_NUMCOLS", QByteArray::number(cols));
            htmlParser.setField("ITEM_WIDTH_EP", QByteArray::number(((cols * epgColWidth) - epgCellSpacing) - (epgCellPadding * 2), 'f', 1));
            htmlParser.setField("ITEM_TEXT", tr("Not available"));
            htmlParser.appendField("PROGRAMMES", htmlParser.parse(htmlEpgNoProgramme));
          }

          const int cols = qMin(rightColumn, epgNumCols) - qMax(lastColumn, leftColumn);
          htmlParser.setField("ITEM_NUMCOLS", QByteArray::number(cols));
          htmlParser.setField("ITEM_WIDTH_EP", QByteArray::number(((cols * epgColWidth) - epgCellSpacing) - (epgCellPadding * 2), 'f', 1));
          htmlParser.setField("ITEM_TEXT", programme.name);
          htmlParser.setField("ITEM_LINK", toID(channelName, programme.utcDate, programme.stationDate.time()) + ".html");
          htmlParser.setField("ITEM_TIME", programme.utcDate.toLocalTime().toString("hh:mm"));

          if (!programmes.isEmpty())
            htmlParser.appendField("ITEM_TIME", " - " + programmes.first().utcDate.toLocalTime().toString("hh:mm"));

          if (programme.recordPriority)
            htmlParser.setField("ITEM_CLASS", QByteArray("epgprogrammerec"));
          else if ((leftColumn <= nowColumn) && (rightColumn > nowColumn))
            htmlParser.setField("ITEM_CLASS", QByteArray("epgprogrammenow"));
          else
            htmlParser.setField("ITEM_CLASS", QByteArray("epgprogramme"));

          htmlParser.appendField("PROGRAMMES", htmlParser.parse(htmlEpgProgramme));

          lastColumn = rightColumn;
        }
      }

      if (lastColumn < epgNumCols)
      {
        const int cols = epgNumCols - lastColumn;
        htmlParser.setField("ITEM_NUMCOLS", QByteArray::number(cols));
        htmlParser.setField("ITEM_WIDTH_EP", QByteArray::number(((cols * epgColWidth) - epgCellSpacing) - (epgCellPadding * 2), 'f', 1));
        htmlParser.setField("ITEM_TEXT", tr("Not available"));
        htmlParser.appendField("PROGRAMMES", htmlParser.parse(htmlEpgNoProgramme));
      }

      settings.endGroup();

      htmlParser.appendField("EPG_CHANNELS", htmlParser.parse(htmlEpgChannel));
    }

    l.unlock();

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlEpgMain), head);
  }

  l.unlock();
  response.setStatusLine(404);
  socket->write(response.toString().toUtf8());
  return false;
}

} // End of namespace
