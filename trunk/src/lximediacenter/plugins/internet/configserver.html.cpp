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
    "   <iframe style=\"width:40em;height:60em;\" src=\"site-tree.html\" frameborder=\"0\">\n"
    "   </iframe><br />\n"
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

SHttpServer::SocketOp ConfigServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, QIODevice *socket, const QString &file)
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

    QStringList selectedAudiences = settings.value("Audiences").toStringList();

    const QString checkon = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkon").toAscii()));
    const QString checkoff = QString::fromUtf8(QByteArray::fromHex(url.queryItemValue("checkoff").toAscii()));
    if (!checkon.isEmpty() || !checkoff.isEmpty())
    {
      if (!checkon.isEmpty())
        selectedAudiences.append(checkon);

      if (!checkoff.isEmpty())
        selectedAudiences.removeAll(checkoff);

      settings.setValue("Audiences", selectedAudiences);
    }

    const QString open = url.queryItemValue("open");
    const QSet<QString> allopen = !open.isEmpty()
                                  ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                  : QSet<QString>();

    htmlParser.setField("FILE", file);
    htmlParser.setField("DIRS", QByteArray(""));
    foreach (const QString &targetAudience, siteDatabase->allTargetAudiences())
    {
      htmlParser.setField("DIR_FULLPATH", targetAudience.toUtf8().toHex());
      htmlParser.setField("DIR_INDENT", QByteArray(""));

      // Expand
      bool addChildren = false;
      QSet<QString> all = allopen;
      if (all.contains(targetAudience))
      {
        all.remove(targetAudience);
        htmlParser.setField("DIR_OPEN", QByteArray("open"));
        addChildren = true;
      }
      else
      {
        all.insert(targetAudience);
        htmlParser.setField("DIR_OPEN", QByteArray("close"));
      }

      htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(all.toList()).join(QString(dirSplit)).toUtf8()).toHex());
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlTreeExpand));

      if (selectedAudiences.contains(targetAudience))
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

      htmlParser.setField("DIR_NAME", targetAudience);

      htmlParser.appendField("DIRS", htmlParser.parse(htmlTreeDir));

      if (addChildren)
      foreach (const QString &identifier, siteDatabase->getSites(targetAudience))
      {
        htmlParser.setField("DIR_FULLPATH", identifier.toUtf8().toHex());
        htmlParser.setField("DIR_INDENT", htmlParser.parse(htmlTreeIndent) + htmlParser.parse(htmlTreeIndent));
        htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", QByteArray(""));

        htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlTreeIndent));
        htmlParser.setField("DIR_NAME", siteDatabase->friendlyName(identifier));

        htmlParser.appendField("DIRS", htmlParser.parse(htmlTreeDir));
      }
    }

    socket->write(response);
    socket->write(htmlParser.parse(htmlTreeIndex));
    return SHttpServer::SocketOp_Close;
  }
  else
  {
    return sendHtmlContent(request, socket, url, response, htmlParser.parse(htmlMain));
  }
}

} } // End of namespaces
