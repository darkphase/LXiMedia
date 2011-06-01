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

#include "configserver.h"
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char * const ConfigServer::htmlMain =
    " <div class=\"content\">\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_RADIO_STATIONS}</legend>\n"
    "   {TR_RADIO_STATIONS_EXPLAIN}<br />\n"
    "   <iframe style=\"width:40em;height:60em;\" src=\"site-edit.html\" frameborder=\"0\">\n"
    "   </iframe>\n"
    "  </fieldset>\n"
    " </div>\n";

const char * const ConfigServer::htmlTreeIndex =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "{DIRS}\n"
    " </table>\n"
    "</body>\n"
    "</html>\n";

const char * const ConfigServer::htmlTreeDir =
    " <tr align=\"middle\"><td align=\"left\">\n"
    "  <a class=\"hidden\" name=\"{DIR_FULLPATH}\" />\n"
    "  {DIR_INDENT}\n"
    "  {DIR_EXPAND}\n"
    "  {DIR_CHECK}\n"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char * const ConfigServer::htmlTreeIndent =
    " <img src=\"/img/null.png\" width=\"16\" height=\"16\" />\n";

const char * const ConfigServer::htmlTreeExpand =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char * const ConfigServer::htmlTreeCheck =
    " <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />";

const char * const ConfigServer::htmlTreeCheckLink =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char * const ConfigServer::htmlEditIndex =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <iframe style=\"width:40em;height:30em;\" src=\"site-tree.html\" frameborder=\"0\">\n"
    " </iframe><br />\n"
    " <form name=\"addsite\" action=\"\" method=\"get\">\n"
    "  <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "   <tr>\n"
    "    <td>{TR_NAME}:</td>\n"
    "    <td><input type=\"text\" size=\"30\" name=\"sitename\" value=\"{NAME}\" /></td>\n"
    "   </tr><tr>\n"
    "    <td>{TR_COUNTRIES}:</td>\n"
    "    <td><input type=\"text\" size=\"30\" name=\"sitecountries\" value=\"{COUNTRIES}\" /></td>\n"
    "   </tr><tr>\n"
    "    <td>{TR_CATEGORY}:</td>\n"
    "    <td>\n"
    "     <select name=\"sitecategory\">\n"
    "      <option value=\"{VAL_RADIO_STATIONS}\">{TR_RADIO_STATIONS}</option>\n"
    "     </select>\n"
    "    </td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">{TR_SCRIPT}:</td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">\n"
    "     <textarea name=\"sitescript\" rows=\"10\" cols=\"50\">{SCRIPT}</textarea>\n"
    "    </td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">\n"
    "     <input type=\"submit\" name=\"add\" value=\"Add\" />\n"
    "     <input type=\"submit\" name=\"update\" value=\"Update\" />\n"
    "    </td>\n"
    "   </tr>\n"
    "  </table>\n"
    " </form>\n"
    "</body>\n"
    "</html>\n";

SHttpServer::SocketOp ConfigServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, const QString &file)
{
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  const QUrl url(request.path());
  HtmlParser htmlParser;

  htmlParser.setField("TR_NAME", tr("Name"));
  htmlParser.setField("TR_COUNTRIES", tr("Countries"));
  htmlParser.setField("TR_CATEGORY", tr("Category"));
  htmlParser.setField("TR_SCRIPT", tr("Script"));
  htmlParser.setField("TR_RADIO_STATIONS", tr("Radio stations"));
  htmlParser.setField("TR_RADIO_STATIONS_EXPLAIN", tr("Select the web radio stations that should be available:"));

  htmlParser.setField("VAL_RADIO_STATIONS", QByteArray::number(SiteDatabase::Category_Radio));

  if (file.endsWith("-tree.html"))
  {
    socket->write(response);
    socket->write(htmlParser.parse(htmlTreeIndex));
    return SHttpServer::SocketOp_Close;
  }
  if (file.endsWith("-edit.html"))
  {
    htmlParser.setField("NAME", url.queryItemValue("sitename").replace('+', ' '));
    htmlParser.setField("COUNTRIES", url.queryItemValue("sitecountries").replace('+', ' '));

    QByteArray script = QByteArray::fromPercentEncoding(url.queryItemValue("sitescript").replace('+', ' ').toAscii());
    if (script.isEmpty())
    {
      script =
          "function listItems()\n"
          "{\n"
          "  return \"http://streams.fresh.fm:8100/\";\n"
          "}\n";
    }

    htmlParser.setField("SCRIPT", script);

    socket->write(response);
    socket->write(htmlParser.parse(htmlEditIndex));
    return SHttpServer::SocketOp_Close;
  }
  else
  {
    htmlParser.setField("TR_RADIO_STATIONS", tr("Radio stations"));
    htmlParser.setField("TR_RADIO_STATIONS_EXPLAIN", tr("Select the web radio stations that should be available:"));

    return sendHtmlContent(request, socket, url, response, htmlParser.parse(htmlMain));
  }
}

} } // End of namespaces
