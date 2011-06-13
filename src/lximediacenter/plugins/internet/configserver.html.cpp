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
    "   <legend>{TR_SITES}</legend>\n"
    "   {TR_SITES_EXPLAIN}<br />\n"
    "   <iframe style=\"width:40em;height:60em;\" src=\"site-edit.html\" frameborder=\"0\" name=\"edit\">\n"
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

const char * const ConfigServer::htmlTreeCheckLink =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char * const ConfigServer::htmlTreeEditItemLink =
    " <a href=\"site-edit.html?item={DIR_ITEM}&amp;open={DIR_ALLOPEN}&amp;pos={DIR_FULLPATH}\" target=\"edit\">{DIR_ITEMNAME}</a>\n";

const char * const ConfigServer::htmlEditIndex =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <iframe style=\"width:40em;height:30em;\" src=\"site-tree.html?open={DIR_ALLOPEN}#{DIR_FULLPATH}\" frameborder=\"0\">\n"
    " </iframe><br />\n"
    " <form name=\"addsite\" action=\"site-edit.html\" method=\"get\">\n"
    "  <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "   <tr>\n"
    "    <td>{TR_HOSTNAME}:</td>\n"
    "    <td><input type=\"text\" size=\"30\" name=\"hostname\" value=\"{HOSTNAME}\" /></td>\n"
    "   </tr><tr>\n"
    "    <td>{TR_TARGET_COUNTRIES}:</td>\n"
    "    <td><input type=\"text\" size=\"30\" name=\"countries\" value=\"{COUNTRIES}\" /></td>\n"
    "   </tr><tr>\n"
    "    <td>{TR_CATEGORY}:</td>\n"
    "    <td>\n"
    "     <select name=\"category\">\n"
    "      <option value=\"1\" {SEL_CATEGORY_1}>{TR_RADIO_STATIONS}</option>\n"
    "      <option value=\"2\" {SEL_CATEGORY_2}>{TR_TELEVISION_STATIONS}</option>\n"
    "     </select>\n"
    "    </td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">{TR_SCRIPT}:</td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">\n"
    "     <textarea name=\"script\" rows=\"10\" cols=\"50\">{SCRIPT}</textarea>\n"
    "    </td>\n"
    "   </tr><tr>\n"
    "    <td colspan=\"2\">\n"
    "     <input type=\"submit\" value=\"Add/Update\" />\n"
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

  htmlParser.setField("TR_HOSTNAME", tr("Host name"));
  htmlParser.setField("TR_TARGET_COUNTRIES", tr("Target countries"));
  htmlParser.setField("TR_CATEGORY", tr("Category"));
  htmlParser.setField("TR_SCRIPT", tr("Script"));
  htmlParser.setField("TR_SITES", tr("Sites"));
  htmlParser.setField("TR_SITES_EXPLAIN", tr("Select the sites that should be available:"));
  htmlParser.setField("TR_RADIO_STATIONS", tr("Radio stations"));
  htmlParser.setField("TR_TELEVISION_STATIONS", tr("Television stations"));

  if (file.endsWith("-tree.html"))
  {
    PluginSettings settings(pluginName());

    QStringList selectedSites = settings.value("Sites").toStringList();

    const QString checkon = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkon").toAscii()));
    const QString checkoff = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkoff").toAscii()));
    if (!checkon.isEmpty() || !checkoff.isEmpty())
    {
      if (!checkon.isEmpty())
        selectedSites.append(checkon);

      if (!checkoff.isEmpty())
        selectedSites.removeAll(checkoff);

      settings.setValue("Sites", selectedSites);
    }

    const QString open = url.queryItemValue("open");
    const QSet<QString> allopen = !open.isEmpty()
                                  ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                  : QSet<QString>();

    htmlParser.setField("FILE", file);
    htmlParser.setField("DIRS", QByteArray(""));
    foreach (const QString &country, siteDatabase->allCountries())
    {
      htmlParser.setField("DIR_FULLPATH", country.toUtf8().toHex());
      htmlParser.setField("DIR_INDENT", QByteArray(""));

      // Expand
      bool addChildren = false;
      QSet<QString> all = allopen;
      if (all.contains(country))
      {
        all.remove(country);
        htmlParser.setField("DIR_OPEN", QByteArray("open"));
        addChildren = true;
      }
      else
      {
        all.insert(country);
        htmlParser.setField("DIR_OPEN", QByteArray("close"));
      }

      htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(all.toList()).join(QString(dirSplit)).toUtf8()).toHex());
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlTreeExpand));

      htmlParser.setField("DIR_CHECK", QByteArray(""));

      htmlParser.setField("DIR_NAME", country);

      htmlParser.appendField("DIRS", htmlParser.parse(htmlTreeDir));

      if (addChildren)
      foreach (const QString &hostname, siteDatabase->getSites(country))
      {
        htmlParser.setField("DIR_FULLPATH", hostname.toUtf8().toHex());
        htmlParser.setField("DIR_INDENT", htmlParser.parse(htmlTreeIndent));
        htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", QByteArray(""));

        if (selectedSites.contains(hostname))
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("full"));
          htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkoff"));
        }
        else
        {
          htmlParser.setField("DIR_CHECKED", QByteArray("none"));
          htmlParser.setField("DIR_CHECKTYPE", QByteArray("checkon"));
        }

        htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlTreeCheckLink));
        htmlParser.setField("DIR_ITEM", hostname.toUtf8().toHex());
        htmlParser.setField("DIR_ITEMNAME", hostname);
        htmlParser.setField("DIR_NAME", htmlParser.parse(htmlTreeEditItemLink));

        htmlParser.appendField("DIRS", htmlParser.parse(htmlTreeDir));
      }
    }

    socket->write(response);
    socket->write(htmlParser.parse(htmlTreeIndex));
    return SHttpServer::SocketOp_Close;
  }
  else if (file.endsWith("-edit.html"))
  {
    htmlParser.setField("DIR_FULLPATH", url.queryItemValue("pos"));
    htmlParser.setField("DIR_ALLOPEN", url.queryItemValue("open"));

    if (url.hasQueryItem("hostname") && url.hasQueryItem("countries") &&
        url.hasQueryItem("category") && url.hasQueryItem("script"))
    {
      const QString hostname = url.queryItemValue("hostname").replace('+', ' ');
      const QString countries = url.queryItemValue("countries").replace('+', ' ');
      const SiteDatabase::Category category = SiteDatabase::Category(url.queryItemValue("category").toUInt());
      const QString script = QByteArray::fromPercentEncoding(url.queryItemValue("script").replace('+', ' ').toAscii());

      htmlParser.setField("HOSTNAME", hostname);
      htmlParser.setField("COUNTRIES", countries);

      for (int i=SiteDatabase::Category_None; i<SiteDatabase::Category_Sentinel; i++)
        htmlParser.setField("SEL_CATEGORY_" + QByteArray::number(i), QByteArray(""));

      htmlParser.setField("SEL_CATEGORY_" + QByteArray::number(category), QByteArray("selected=\"selected\""));
      htmlParser.setField("SCRIPT", script);

      if (!hostname.isEmpty() && !countries.isEmpty() && !script.isEmpty())
        siteDatabase->setSite(hostname, countries, category, script);
    }
    else
    {
      const QString hostname = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("item").toAscii()));
      QString countries, script;
      SiteDatabase::Category category = SiteDatabase::Category_None;

      siteDatabase->getSite(hostname, countries, category, script);

      htmlParser.setField("HOSTNAME", hostname);
      htmlParser.setField("COUNTRIES", countries);
      htmlParser.setField("SCRIPT", script);

      for (int i=SiteDatabase::Category_None; i<SiteDatabase::Category_Sentinel; i++)
        htmlParser.setField("SEL_CATEGORY_" + QByteArray::number(i), QByteArray(category == i ? "selected=\"selected\"" : ""));

      /*htmlParser.setField("SCRIPT", QByteArray(
          "function streamLocation()\n"
          "{\n"
          "  return \"http://streams.fresh.fm:8100/\";\n"
          "}\n"));*/
    }

    socket->write(response);
    socket->write(htmlParser.parse(htmlEditIndex));
    return SHttpServer::SocketOp_Close;
  }
  else
  {
    return sendHtmlContent(request, socket, url, response, htmlParser.parse(htmlMain));
  }
}

} } // End of namespaces
