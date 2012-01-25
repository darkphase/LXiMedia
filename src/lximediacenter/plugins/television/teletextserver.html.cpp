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

#include "teletextserver.h"
#include "televisionserver.h"

namespace LXiMediaCenter {

const char * const TeletextServer::cssMain =
    "table.teletext {\n"
    "  padding: 0;\n"
    "  border-spacing: 0em;\n"
    "  border: none;\n"
    "  margin-left: auto;\n"
    "  margin-right: auto;\n"
    "}\n"
    "\n"
    "tr.teletext {\n"
    "  vertical-align: top;\n"
    "}\n"
    "\n"
    "td.teletext {\n"
    "  padding: 0;\n"
    "  text-align: left;\n"
    "}\n"
    "\n"
    "td.teletextleft {\n"
    "  padding: 0;\n"
    "  text-align: left;\n"
    "}\n"
    "\n"
    "td.teletextcenter {\n"
    "  padding: 0;\n"
    "  text-align: center;\n"
    "}\n"
    "\n"
    "td.teletextright {\n"
    "  padding: 0;\n"
    "  text-align: right;\n"
    "}\n"
    "\n"
    "{COLORS}";

const char * const TeletextServer::cssColor =
    "{BASE}.tt{FGCOL}{BGCOL} {\n"
    "  background-color: {BGRGB};\n"
    "  color: {FGRGB};\n"
    "  font-family: monospace;\n"
    "  font-size: 11px;\n"
    "  font-weight: bold;\n"
    "  padding: 0;\n"
    "}\n\n";

const char * const TeletextServer::htmlMain =
    " <table class=\"widgets\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\">\n"
    "    <p class=\"head\">{SELECTED_CHANNEL_NAME}</p>\n"
    "    <table class=\"teletext\">\n"
    "     <tr class=\"teletext\">\n"
    "      <td class=\"teletextleft\" colspan=\"8\">\n"
    "       <form name=\"browse\" action=\"{SELECTED_CHANNEL}.html\" method=\"get\">\n"
    "        <input type=\"text\" size=\"3\" name=\"page\" value=\"{SELECTED_PAGE}\" />\n"
    "        <input type=\"submit\" value=\"{TR_GO}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "      <td class=\"teletextleft\" colspan=\"4\">\n"
    "       <form name=\"browse\" action=\"{SELECTED_CHANNEL}.html\" method=\"get\">\n"
    "        <input type=\"hidden\" name=\"page\" value=\"{HOME_PAGE}\" />\n"
    "        <input type=\"submit\" value=\"{HOME_PAGE}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "      <td class=\"teletextleft\" colspan=\"4\">\n"
    "       <form name=\"browse\" action=\"{SELECTED_CHANNEL}.html\" method=\"get\">\n"
    "        <input type=\"hidden\" name=\"page\" value=\"{PREVIOUS_PAGE}\" />\n"
    "        <input type=\"submit\" value=\"{PREVIOUS_PAGE}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "      <td class=\"teletextleft\" colspan=\"4\">\n"
    "       <form name=\"browse\" action=\"{SELECTED_CHANNEL}.html\" method=\"get\">\n"
    "        <input type=\"hidden\" name=\"page\" value=\"{NEXT_PAGE}\" />\n"
    "        <input type=\"submit\" value=\"{NEXT_PAGE}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "      <td class=\"teletextright\" colspan=\"21\">\n"
    "       <form name=\"bookmark\" action=\"{SELECTED_CHANNEL}.html\" method=\"get\">\n"
    "        <input type=\"hidden\" name=\"page\" value=\"{SELECTED_PAGE}\" />\n"
    "        <select name=\"bookmark\">\n"
    "         <option value=\"-1\" ></option>\n"
    "         <option value=\"0\" {EPG0_SEL}>{TR_EPG_TODAY}</option>\n"
    "         <option value=\"1\" {EPG1_SEL}>{TR_EPG_TOMORROW}</option>\n"
    "         <option value=\"2\" {EPG2_SEL}>{TR_EPG_DAYAFTERTOMORROW}</option>\n"
    "         <option value=\"3\" {EPG3_SEL}>{TR_EPG_MONDAY}</option>\n"
    "         <option value=\"4\" {EPG4_SEL}>{TR_EPG_TUESDAY}</option>\n"
    "         <option value=\"5\" {EPG5_SEL}>{TR_EPG_WEDNESDAY}</option>\n"
    "         <option value=\"6\" {EPG6_SEL}>{TR_EPG_THURSDAY}</option>\n"
    "         <option value=\"7\" {EPG7_SEL}>{TR_EPG_FRIDAY}</option>\n"
    "         <option value=\"8\" {EPG8_SEL}>{TR_EPG_SATURDAY}</option>\n"
    "         <option value=\"9\" {EPG9_SEL}>{TR_EPG_SUNDAY}</option>\n"
    "        </select>\n"
    "        <input type=\"submit\" value=\"{TR_SAVE}\" />\n"
    "       </form>\n"
    "      </td>\n"
    "     </tr>\n"
    "{TELETEXT_CONTENT}"
    "     <tr class=\"teletext\">\n"
    "      <td class=\"teletextright\" colspan=\"41\">\n"
    "{TELETEXT_SUBPAGES}"
    "      </td>\n"
    "     </tr>\n"
    "    </table>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"0%\" rowspan=\"2\">\n"
    "    <table width=\"100%\" cellpadding=\"2\" cellspacing=\"1\" border=\"0\">\n"
    "{TELETEXT_CHANNELS}"
    "    </table>\n"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"nowidget\"></td>\n"
    "  </tr>\n"
    " </table>\n";

const char * const TeletextServer::htmlChannel =
    "     <tr>\n"
    "      <td class=\"left\">\n"
    "       <a href=\"{ITEM_RAWNAME}.html\">{ITEM_PRESET}. {ITEM_NAME}</a>\n"
    "      </td>\n"
    "     </tr>\n";

const char * const TeletextServer::htmlDisabledChannel =
    "     <tr>\n"
    "      <td class=\"left\">\n"
    "       {ITEM_PRESET}. {ITEM_NAME}\n"
    "      </td>\n"
    "     </tr>\n";

const char * const TeletextServer::htmlSubpage =
    "       <a href=\"?page={SELECTED_PAGE}&amp;subpage={ITEM_SUBPAGE}\">{ITEM_SUBPAGE}</a>\n";

bool TeletextServer::handleHtmlRequest(const QUrl &url, const QString &file, QAbstractSocket *socket)
{
  if (file == "teletext.css")
  {
    static const char * const ttcolors[] =
    { "#000000", "#FF0000", "#00FF00", "#FFFF00",
      "#0000FF", "#FF00FF", "#00FFFF", "#FFFFFF" };

    HtmlParser htmlParser;
    htmlParser.setField("COLORS", QByteArray(""));

    for (unsigned fg=0; fg<8; fg++)
    {
      htmlParser.setField("FGCOL", QByteArray::number(fg));
      htmlParser.setField("FGRGB", QByteArray(ttcolors[fg]));

      for (unsigned bg=0; bg<8; bg++)
      {
        htmlParser.setField("BGCOL", QByteArray::number(bg));
        htmlParser.setField("BGRGB", QByteArray(ttcolors[bg]));

        htmlParser.setField("BASE", QByteArray("td"));
        htmlParser.appendField("COLORS", htmlParser.parse(cssColor));

        htmlParser.setField("BASE", QByteArray("a"));
        htmlParser.appendField("COLORS", htmlParser.parse(cssColor));
      }
    }

    return sendReply(socket, htmlParser.parse(cssMain), "text/css", true);
  }
  else
  {
    QHttpResponseHeader response(200);
    response.setContentType("text/html;charset=utf-8");
    response.setValue("Cache-Control", "no-cache");

    PluginSettings settings(plugin);

    HtmlParser htmlParser;
    htmlParser.setField("TR_GO", tr("Go"));
    htmlParser.setField("TR_SAVE", tr("Save"));
    htmlParser.setField("TR_EPG_TODAY", tr("EPG Today"));
    htmlParser.setField("TR_EPG_TOMORROW", tr("EPG Tomorrow"));
    htmlParser.setField("TR_EPG_DAYAFTERTOMORROW", tr("EPG Day after tomorrow"));
    htmlParser.setField("TR_EPG_MONDAY", tr("EPG Monday"));
    htmlParser.setField("TR_EPG_TUESDAY", tr("EPG Tuesday"));
    htmlParser.setField("TR_EPG_WEDNESDAY", tr("EPG Wednesday"));
    htmlParser.setField("TR_EPG_THURSDAY", tr("EPG Thursday"));
    htmlParser.setField("TR_EPG_FRIDAY", tr("EPG Friday"));
    htmlParser.setField("TR_EPG_SATURDAY", tr("EPG Saturday"));
    htmlParser.setField("TR_EPG_SUNDAY", tr("EPG Sunday"));

    QMap<int, QString> allChannels;
    foreach (const QString &group, settings.childGroups())
    if (group.startsWith(typeName(Type_Television) + "Channel_"))
      allChannels[group.mid(8 + typeName(Type_Television).length()).toInt()] = group;

    const QString selectedChannel = SStringParser::toRawName(file.left(file.length() - 5));
    const int selectedPage = url.hasQueryItem("page") ? url.queryItemValue("page").toInt(NULL, 16) : -1;
    const int selectedSubpage = url.hasQueryItem("subpage") ? url.queryItemValue("subpage").toInt() : -1;

    htmlParser.setField("TELETEXT_SUBPAGES", QByteArray(""));
    htmlParser.setField("TELETEXT_CHANNELS", QByteArray(""));
    htmlParser.setField("SELECTED_CHANNEL_NAME", selectedChannel);
    htmlParser.setField("SELECTED_CHANNEL", selectedChannel);
    htmlParser.setField("SELECTED_PAGE", QByteArray("100"));
    htmlParser.setField("HOME_PAGE", QByteArray(""));
    htmlParser.setField("PREVIOUS_PAGE", QByteArray(""));
    htmlParser.setField("NEXT_PAGE", QByteArray(""));
    for (int i=0; i<=9; i++)
      htmlParser.setField("EPG" + QByteArray::number(i) + "_SEL", QByteArray(""));

    unsigned selectedPreset = 1;
    foreach (const QString &group, allChannels)
    {
      settings.beginGroup(group);

      const QString channelName = settings.value("Name").toString();
      const QString rawName = SStringParser::toRawName(channelName);
      const unsigned preset = group.mid(8 + typeName(Type_Television).length()).toUInt();
      if (rawName == selectedChannel)
        selectedPreset = preset;

      htmlParser.setField("ITEM_RAWNAME", rawName.toLower());
      htmlParser.setField("ITEM_PRESET", QByteArray::number(preset));
      htmlParser.setField("ITEM_NAME", channelName);

      if (firstPage(channelName) > 0)
        htmlParser.appendField("TELETEXT_CHANNELS", htmlParser.parse(htmlChannel));
      else
        htmlParser.appendField("TELETEXT_CHANNELS", htmlParser.parse(htmlDisabledChannel));

      settings.endGroup();
    }

    if (selectedPreset > 0)
    {
      settings.beginGroup("TVChannel_" + QString::number(selectedPreset));

      const QString channelName = settings.value("Name").toString();
      const QString channelLink = SStringParser::toRawName(channelName).toLower();
      htmlParser.setField("SELECTED_CHANNEL_NAME", channelName);
      htmlParser.setField("SELECTED_CHANNEL", channelLink);

      QString content;

      const QSet<int> pages = allPages(channelName);

      SDataBuffer::TeletextPage page = readPage(channelName, selectedPage, selectedSubpage);

      htmlParser.setField("SELECTED_PAGE", QByteArray::number(page.pgno, 16));
      foreach (int subpage, allSubPages(channelName, page.pgno))
      {
        htmlParser.setField("ITEM_SUBPAGE", QByteArray::number(subpage));
        htmlParser.appendField("TELETEXT_SUBPAGES", htmlParser.parse(htmlSubpage));
      }

      htmlParser.setField("HOME_PAGE", QByteArray::number(firstPage(channelName), 16).toUpper());
      if (((page.pgno & 0x00FF) != 0) && pages.contains(page.pgno & 0x0F00))
        htmlParser.setField("HOME_PAGE", QByteArray::number(page.pgno & 0x0F00, 16).toUpper());

      htmlParser.setField("PREVIOUS_PAGE", QByteArray::number(page.pgno, 16).toUpper());
      for (int i=page.pgno-1; i>=0x100; i--)
      if (pages.contains(i))
      {
        htmlParser.setField("PREVIOUS_PAGE", QByteArray::number(i, 16).toUpper());
        break;
      }

      htmlParser.setField("NEXT_PAGE", QByteArray::number(page.pgno, 16).toUpper());
      for (int i=page.pgno+1; i<=0x8FF; i++)
      if (pages.contains(i))
      {
        htmlParser.setField("NEXT_PAGE", QByteArray::number(i, 16).toUpper());
        break;
      }

      for (unsigned b=0; b<=9; b++)
      foreach (const QString &p, settings.value("EPGTeletextPage" + QString::number(b)).toString().split(' '))
      if (p.toInt(NULL, 16) == page.pgno)
      {
        htmlParser.setField("EPG" + QByteArray::number(b) + "_SEL", QByteArray("selected=\"selected\""));
        break;
      }

      for (unsigned y=0; y<25; y++)
      {
        int graphics = -1, bgcol = 0, fgcol = 7, height = 15;
        const char * const line = page.line(y);

        // Check if this is a double height line
        if (line)
        for (int x=0; x<40; x++)
        if (line[x] == 13) // Double height
        {
          height *= 2;
          break;
        }

        content += "<tr class=\"teletext\">";

        if (line == NULL)
        {
          content += "<td colspan=\"40\" class=\"tt70\"></td>";
        }
        else for (unsigned x=0; x<40; x++)
        {
          const char c = ((y > 0) || (x >= 8)) ? line[x] : ' ';
          const char l = c & 0x0F;
          const char h = c & 0x70;

          if (h == 0)
          {
            if (l < 8)
              graphics = -qAbs(graphics);

            if ((l >= 1) && (l <= 7))
              fgcol = l;
          }
          else if (h == 0x10)
          {
            if (l != 8)
              graphics = qAbs(graphics);

            if ((l >= 1) && (l <= 7))
              fgcol = l;
            else if (l == 9)
              graphics = 1;
            else if (l == 10)
              graphics = 2;
            else if (l == 12)
              bgcol = 0;
            else if (l == 13)
            {
              bgcol = fgcol;
              fgcol = 0;
            }
            else if (l == 15)
              graphics = -qAbs(graphics);
          }

          const QString color = "tt" + QString::number(fgcol) + QString::number(bgcol);
          content += "<td width=\"12\" class=\"" + color + "\">";

          if (h >= 0x20)
          {
            if (graphics < 0)
            {
              int linkpage = 0;
              if ((y > 0) && (c >= '0') && (c <= '9'))
              {
                const char * const first = line + qMax(0u, x - 3);
                const char * const last = line + qMin(39u, x + 3);

                int page = 0, digits = 0;

                for (const char *i=first; i<=last; i++)
                if ((*i >= '0') && (*i <= '9'))
                {
                  page = (page << 4) + int(*i - '0');
                  digits++;
                }
                else if ((digits == 3) && (page >= 0x100) && (page <= 0x8FF))
                {
                  break;
                }
                else
                {
                  page = 0;
                  digits = 0;
                }

                if ((digits == 3) && (page >= 0x100) && (page <= 0x8FF))
                if (pages.contains(page))
                  linkpage = page;
              }

              if (linkpage > 0)
                content += "<a href=\"" + channelLink +
                           ".html?page=" + QString::number(linkpage, 16) + "\" "
                           "class=\"" + color + "\">";

              content += QChar::fromLatin1(c);

              if (linkpage > 0)
                content += "</a>";
            }
            else
            {
              content += "<img src=\"" +
                         ("0" + QString::number(((h & 0x40) >> 1) | (h & 0x10) | l, 16)).right(2) +
                         QString::number(graphics, 16) +
                         QString::number(fgcol, 16) +
                         QString::number(bgcol, 16) + ".png\" "
                         "width=\"12\" height=\"" + QString::number(height) + "\" "
                         "alt=\" \" />";
            }
          }

          content += "</td>";
        }

        content += "<td width=\"12\" height=\"" + QString::number(height) + "\" class=\"tt70\"></td></tr>\n";

        if (height > 15)
          y++; // Skip next line
      }

      htmlParser.setField("TELETEXT_CONTENT", content);

      settings.endGroup();
    }
    else
      htmlParser.setField("TELETEXT_CONTENT", QByteArray(""));

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlMain), head);
  }
}

} // End of namespace
