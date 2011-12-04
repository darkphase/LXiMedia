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

#include "internetserver.h"
#include "sitedatabase.h"
#include "module.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char InternetServer::htmlFrontPageContent[] =
    "   <div class=\"thumbnaillist\" id=\"internetitems\">\n"
    "   </div>\n"
    "   <script type=\"text/javascript\">loadListContent(\"internetitems\", \"{SERVER_PATH}\", 0, 0);</script>\n";

const char InternetServer::htmlSettingsMain[] =
    "  <fieldset>\n"
    "   <legend>{TR_INTERNET}</legend>\n"
    "   <form name=\"settings\" action=\"{SERVER_PATH}\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"save_settings\" value=\"settings\" />\n"
    "    <table>\n"
    "     <tr>\n"
    "      <td>\n"
    "       <iframe style=\"width:30em;height:30em;\" src=\"{SERVER_PATH}?site_tree=\" frameborder=\"0\">\n"
    "        <a href=\"{SERVER_PATH}?site_tree=\" target=\"_blank\">frame</a>\n"
    "       </iframe>\n"
    "      </td>\n"
    "      <td class=\"top\">\n"
    "      </td>\n"
    "     </tr>\n"
    "     <tr><td colspan=\"2\">\n"
    "      <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "     </td></tr>\n"
    "    </table>\n"
    "   </form>\n"
    "  </fieldset>\n";

const char InternetServer::htmlSiteTreeIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "</head>\n"
    "<body>\n"
    " <table width=\"100%\" cellspacing=\"0\" cellpadding=\"3\" border=\"0\">\n"
    "{DIRS}\n"
    " </table>\n"
    "</body>\n"
    "</html>\n";

const char InternetServer::htmlSiteTreeDir[] =
    " <tr valign=\"middle\"><td align=\"left\">\n"
    "  <a class=\"hidden\" name=\"{DIR_FULLPATH}\" />\n"
    "{DIR_INDENT}"
    "{DIR_EXPAND}"
    "{DIR_CHECK}"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char InternetServer::htmlSiteTreeIndent[] =
    "  <img src=\"/img/null.png\" width=\"16\" height=\"16\" />\n";

const char InternetServer::htmlSiteTreeExpand[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?site_tree=&amp;open={DIR_ALLOPEN}#{DIR_FULLPATH}\">\n"
    "   <img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />\n"
    "  </a>\n";

const char InternetServer::htmlSiteTreeCheckLink[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?site_tree=&amp;open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "   <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    "  </a>\n";

const char InternetServer::htmlSiteTreeCheckIcon[] =
    "  <img class=\"thumbnail\" src=\"{ITEM_ICON}\" width=\"16\" height=\"16\" />\n";

const char InternetServer::htmlSiteTreeScriptLink[] =
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?edit={DIR_FULLPATH}\" target=\"_blank\">{ITEM_NAME}</a>\n";

const char InternetServer::htmlSiteEditIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{SCRIPT_NAME} - {TR_SCRIPT_EDITOR}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/siteeditor.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/siteeditor.js\"></script>\n" // Open and close tag due to IE bug
    "</head>\n"
    "<body>\n"
    " <div class=\"siteeditor\">\n"
    "  <form name=\"edit\" action=\"{SERVER_PATH}\" method=\"get\">\n"
    "   <input type=\"hidden\" name=\"save\" value=\"{ENCODED_SCRIPT_NAME}\" />\n"
    "   <div class=\"toolbar\">\n"
    "    <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "   </div>\n"
    "   <div class=\"texteditor\">\n"
    "    <textarea class=\"text\" name=\"script\" id=\"script\" wrap=\"off\">{SCRIPT}</textarea><br />\n"
    "    <script type=\"text/javascript\">createScriptArea(\"script\");</script>\n"
    "   </div>\n"
    "  </form>\n"
    " </div>\n"
    "</body>\n"
    "</html>\n";

QByteArray InternetServer::frontPageContent(void)
{
  HtmlParser htmlParser;
  htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());

  return htmlParser.parse(htmlFrontPageContent);
}

QByteArray InternetServer::settingsContent(void)
{
  HtmlParser htmlParser;
  htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());
  htmlParser.setField("TR_INTERNET", tr(Module::pluginName));
  htmlParser.setField("TR_SAVE", tr("Save"));

  return htmlParser.parse(htmlSettingsMain);
}

/*SHttpServer::ResponseMessage ConfigServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, const MediaServer::File &file)
{
  HtmlParser htmlParser;

  htmlParser.setField("TR_HOSTNAME", tr("Host name"));
  htmlParser.setField("TR_TARGET_COUNTRIES", tr("Target countries"));
  htmlParser.setField("TR_CATEGORY", tr("Category"));
  htmlParser.setField("TR_SCRIPT", tr("Script"));
  htmlParser.setField("TR_SITE_SELECTION", tr("Site selection"));
  htmlParser.setField("TR_SITES_EXPLAIN", tr("Select the sites that should be available:"));
  htmlParser.setField("TR_SCRIPT_EDITOR", tr("Script editor"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_REMOVE", tr("Remove"));
  htmlParser.setField("TR_SCRIPT_INFO", tr("Script info"));
  htmlParser.setField("TR_APPEND_IMAGE", tr("Append image"));

  if (file.fileName().endsWith("-tree.html"))
  {
    PluginSettings settings(pluginName());

    QStringList selectedAudiences = settings.value("Audiences").toStringList();

    const QString checkon = QString::fromUtf8(QByteArray::fromHex(file.url().queryItemValue("checkon").toAscii()));
    const QString checkoff = QString::fromUtf8(QByteArray::fromHex(file.url().queryItemValue("checkoff").toAscii()));
    if (!checkon.isEmpty() || !checkoff.isEmpty())
    {
      if (!checkon.isEmpty())
        selectedAudiences.append(checkon);

      if (!checkoff.isEmpty())
        selectedAudiences.removeAll(checkoff);

      settings.setValue("Audiences", selectedAudiences);
    }

    const QString open = file.url().queryItemValue("open");
    const QSet<QString> allopen = !open.isEmpty()
                                  ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                  : QSet<QString>();

    htmlParser.setField("FILE", file.fileName());
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
      htmlParser.setField("DIR_EXPAND", htmlParser.parse(htmlSiteTreeExpand));

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

      htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSiteTreeCheckLink));

      htmlParser.setField("DIR_NAME", targetAudience);

      htmlParser.appendField("DIRS", htmlParser.parse(htmlSiteTreeDir));

      if (addChildren)
      foreach (const QString &identifier, siteDatabase->getSites(targetAudience))
      {
        htmlParser.setField("DIR_FULLPATH", identifier.toUtf8().toHex());
        htmlParser.setField("DIR_INDENT", htmlParser.parse(htmlSiteTreeIndent) + htmlParser.parse(htmlSiteTreeIndent));
        htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
        htmlParser.setField("DIR_EXPAND", QByteArray(""));

        htmlParser.setField("ITEM_ICON", QUrl('/' + pluginName() + "/Sites/" + siteDatabase->reverseDomain(identifier) + "/-thumb.png").toEncoded());
        htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSiteTreeCheckIcon));

        htmlParser.setField("ITEM_NAME", siteDatabase->reverseDomain(identifier));
        htmlParser.setField("ITEM_IDENTIFIER", siteDatabase->fullIdentifier(identifier));
        htmlParser.setField("DIR_NAME", htmlParser.parse(htmlSiteTreeScriptLink));

        htmlParser.appendField("DIRS", htmlParser.parse(htmlSiteTreeDir));
      }
    }

    SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
    response.setField("Cache-Control", "no-cache");
    response.setContentType("text/html;charset=utf-8");
    response.setContent(htmlParser.parse(htmlSiteTreeIndex));

    return response;
  }
  else if (file.fileName() == "edit.html")
  {
    htmlParser.setField("IDENTIFIER", QByteArray(""));
    htmlParser.setField("SCRIPT", QByteArray(""));
    htmlParser.setField("ERROR", QByteArray(""));

    if (file.url().hasQueryItem("save"))
    {
      const QString identifier = QByteArray::fromPercentEncoding(file.url().queryItemValue("identifier").toAscii());
      const QString script = QByteArray::fromPercentEncoding(file.url().queryItemValue("script").toAscii().replace('+', ' '));

      const QScriptSyntaxCheckResult syntaxCheckResult = QScriptEngine::checkSyntax(script);
      if (syntaxCheckResult.state() != QScriptSyntaxCheckResult::Valid)
      {
        htmlParser.setField("IDENTIFIER", identifier);
        htmlParser.setField("SCRIPT", script);
        htmlParser.setField("ERROR",
            QString::number(syntaxCheckResult.errorLineNumber()) + ':' +
            QString::number(syntaxCheckResult.errorColumnNumber()) + ' ' +
            syntaxCheckResult.errorMessage());
      }
      else
      {
        siteDatabase->update(identifier, script);

        SHttpServer::ResponseMessage response(request, SHttpServer::Status_MovedPermanently);
        response.setField("Location", "http://" + request.host() + serverPath());
        return response;
      }
    }
    else if (file.url().hasQueryItem("remove"))
    {
      siteDatabase->remove(QByteArray::fromPercentEncoding(file.url().queryItemValue("identifier").toAscii()));


      SHttpServer::ResponseMessage response(request, SHttpServer::Status_MovedPermanently);
      response.setField("Location", "http://" + request.host() + serverPath());
      return response;
    }
    else
    {
      htmlParser.setField("IDENTIFIER", siteDatabase->fullIdentifier(file.url().queryItemValue("identifier")));
      htmlParser.setField("SCRIPT",  siteDatabase->script(file.url().queryItemValue("identifier")));
    }

    return makeHtmlContent(request, file.url(), htmlParser.parse(htmlEditMain), htmlEditHead);
  }
  else
    return makeHtmlContent(request, file.url(), htmlParser.parse(htmlMain));
}*/

} } // End of namespaces
