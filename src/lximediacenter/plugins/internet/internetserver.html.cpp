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
#include "scriptengine.h"
#include "module.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char InternetServer::htmlFrontPageContent[] =
    "   <div class=\"list_thumbnails\" id=\"internetitems\">\n"
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
    "       {TR_SELECT_SITES}:<br />\n"
    "       <iframe style=\"width:30em;height:30em;\" src=\"{SERVER_PATH}?site_tree=\" frameborder=\"0\">\n"
    "        <a href=\"{SERVER_PATH}?site_tree=\" target=\"_blank\">frame</a>\n"
    "       </iframe><br />\n"
    "       <a href=\"{SERVER_PATH}?edit=\" target=\"_blank\">{TR_CREATE_NEW_SITE}</a>"
    "      </td>\n"
    "      <td class=\"top\">\n"
    "       {TR_SITES_EXPLAIN}<br /><br />\n"
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
    "{DIR_NAME}"
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
    "  <a class=\"hidden\" href=\"{SERVER_PATH}?edit={ITEM_HOST}\" target=\"_blank\">{ITEM_NAME}</a>\n";

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
    "  <form name=\"edit\" action=\"{SERVER_PATH}\" method=\"post\" enctype=\"multipart/form-data\">\n"
    "   <table class=\"toolbar\">\n"
    "    <tr><td>\n"
    "     {TR_DOMAIN}:\n"
    "     <input type=\"text\" size=\"20\" name=\"host\" value=\"{SCRIPT_HOST}\" />\n"
    "     <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "{BUTTONS}"
    "     {TR_ADD_ICON}:\n"
    "     <input type=\"file\" size=\"3\" name=\"image\" onchange=\"javascript: document.forms['edit'].submit();\" />\n"
    "     <span class=\"message\" id=\"message\">{MESSAGE}</span>\n"
    "    </td><td>\n"
    "     <input type=\"submit\" name=\"close\" value=\"{TR_CLOSE}\" />\n"
    "    </td></tr>\n"
    "   </table>\n"
    "   <div class=\"texteditor\">\n"
    "    <textarea class=\"script\" name=\"script\" id=\"script\" wrap=\"off\">{SCRIPT}</textarea><br />\n"
    "   </div>\n"
    "   <script type=\"text/javascript\">createScriptArea(\"script\", \"message\", {SERVER_PATH});</script>\n"
    "  </form>\n"
    " </div>\n"
    "</body>\n"
    "</html>\n";

const char InternetServer::htmlSiteEditButton[] =
    "     <input type=\"submit\" name=\"{BUTTON_NAME}\" value=\"{BUTTON_TEXT}\" />\n";

const char InternetServer::htmlSiteEditCloseIndex[] =
    "<!DOCTYPE html>\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{TR_SCRIPT_EDITOR}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <link rel=\"stylesheet\" href=\"/css/siteeditor.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <script type=\"text/javascript\" src=\"/js/siteeditor.js\"></script>\n" // Open and close tag due to IE bug
    "</head>\n"
    "<body>\n"
    " <div class=\"siteeditor\">{TR_CLOSE_WINDOW}</div>\n"
    " <script type=\"text/javascript\">closeWindow();</script>\n"
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
  htmlParser.setField("TR_SELECT_SITES", tr("Select sites"));
  htmlParser.setField("TR_CREATE_NEW_SITE", tr("Create new site"));

  htmlParser.setField("TR_SITES_EXPLAIN", tr(
      "Sites are categorized by audience, an audience is a group that has a "
      "common geographical location, language, or interest. The sites for each "
      "selected audience will be available in the user interface."
      ));

  return htmlParser.parse(htmlSettingsMain);
}

SHttpServer::ResponseMessage InternetServer::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if (request.isGet())
  {
    if (request.url().hasQueryItem("site_tree"))
    {
      QSettings settings;
      settings.beginGroup(Module::pluginName);

      QStringList selectedAudiences = settings.value("Audiences").toStringList();

      const QString checkon = QString::fromUtf8(QByteArray::fromHex(request.url().queryItemValue("checkon").toAscii()));
      const QString checkoff = QString::fromUtf8(QByteArray::fromHex(request.url().queryItemValue("checkoff").toAscii()));
      if (!checkon.isEmpty() || !checkoff.isEmpty())
      {
        if (!checkon.isEmpty())
          selectedAudiences.append(checkon);

        if (!checkoff.isEmpty())
          selectedAudiences.removeAll(checkoff);

        settings.setValue("Audiences", selectedAudiences);
      }

      const QString open = request.url().queryItemValue("open");
      const QSet<QString> allopen = !open.isEmpty()
                                    ? QSet<QString>::fromList(QString::fromUtf8(qUncompress(QByteArray::fromHex(open.toAscii()))).split(dirSplit))
                                    : QSet<QString>();

      HtmlParser htmlParser;
      htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());
      htmlParser.setField("DIRS", QByteArray(""));
      foreach (const QString &targetAudience, siteDatabase->allAudiences())
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
        foreach (const QString &host, siteDatabase->getSites(targetAudience))
        {
          const QString name = siteDatabase->getName(host);

          htmlParser.setField("DIR_FULLPATH", host.toUtf8().toHex());
          htmlParser.setField("DIR_INDENT", htmlParser.parse(htmlSiteTreeIndent) + htmlParser.parse(htmlSiteTreeIndent));
          htmlParser.setField("DIR_ALLOPEN", qCompress(QStringList(allopen.toList()).join(QString(dirSplit)).toUtf8()).toHex());
          htmlParser.setField("DIR_EXPAND", QByteArray(""));

          htmlParser.setField("ITEM_ICON", QUrl(serverPath() + name + "/?thumbnail=16").toEncoded());
          htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlSiteTreeCheckIcon));

          htmlParser.setField("ITEM_HOST", host);
          htmlParser.setField("ITEM_NAME", name);
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
    else if (request.url().hasQueryItem("thumbnail"))
    {
      const QSize size = SSize::fromString(request.url().queryItemValue("thumbnail")).size();
      QString defaultIcon = ":/img/null.png";
      QByteArray content;

      QString name = sitePath(request.file());
      name = name.left(name.indexOf('/'));

      ScriptEngine * const engine = getScriptEngine(siteDatabase->getHost(name));
      if (engine)
        content = makeThumbnail(size, engine->icon(request.fileName()), request.url().queryItemValue("overlay"));

      if (content.isEmpty())
        content = makeThumbnail(size, QImage(defaultIcon));

      return SSandboxServer::ResponseMessage(request, SSandboxServer::Status_Ok, content, SHttpEngine::mimeImagePng);
    }
    else if (request.url().hasQueryItem("edit"))
    {
      const QString host = request.url().queryItemValue("edit");
      if (host.isEmpty())
      {
        QFile file(":/internet/example.js");
        if (file.open(QFile::ReadOnly))
          return editRequest(request, "example.org", QString::fromUtf8(file.readAll()));
      }
      else
        return editRequest(request, host, siteDatabase->getScript(host));
    }
  }
  else if (request.isPost())
  {
    if (request.url().hasQueryItem("parse"))
    {
      const QScriptSyntaxCheckResult result =
          QScriptEngine::checkSyntax(request.content());

      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);

      if (result.state() != QScriptSyntaxCheckResult::Valid)
      {
        response.setContent(
            "Line " + QByteArray::number(result.errorLineNumber()) + ": " +
            result.errorMessage().toUtf8());
      }
      else
        response.setContent(QByteArray());

      return response;
    }
    else
    {
      const SHttpEngine::MimePartMap parts = SHttpEngine::splitMultipartMime(request.content());
      QString host, script, message;

      if (parts.contains("save") && parts.contains("host") &&
          parts.contains("script"))
      {
        host = QString::fromUtf8(parts["host"].content);
        script = QString::fromUtf8(parts["script"].content);

        if (!host.isEmpty() && !script.isEmpty())
        {
          if (siteDatabase->updateScript(host, script))
            deleteScriptEngine(host);
          else
            message = tr("Could not save file.");
        }
      }
      else if (parts.contains("delete") && parts.contains("host"))
      {
        host = QString::fromUtf8(parts["host"].content);

        if (siteDatabase->deleteLocalScript(host))
          deleteScriptEngine(host);

        script = siteDatabase->getScript(host);
      }
      else if (parts.contains("image") && !parts["image"].content.isEmpty() &&
               parts.contains("host") && parts.contains("script"))
      {
        host = QString::fromUtf8(parts["host"].content);
        script = QString::fromUtf8(parts["script"].content);

        QImage image = QImage::fromData(parts["image"].content);
        if (!image.isNull())
        {
          image = image.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          QImage result(128, 128, QImage::Format_ARGB32);
          QPainter p;
          p.begin(&result);
            p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha
            p.fillRect(result.rect(), Qt::transparent);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver); // Process alpha
            p.drawImage(
                (result.width() / 2) - (image.width() / 2),
                (result.height() / 2) - (image.height() / 2),
                image);
          p.end();

          QBuffer b;
          result.convertToFormat(QImage::Format_Indexed8).save(&b, "PNG");

          QString fileName = "image";
          if (parts["image"].fields.contains("filename"))
            fileName = SStringParser::toCleanName(QFileInfo(parts["image"].fields["filename"]).completeBaseName()).replace(' ', '_');

          script = script.trimmed();
          if (script.endsWith('}'))
            script += '\n';

          script += "\nvar " + fileName + "_png = \"" +  b.data().toBase64() + "\";\n";
        }
      }

      return editRequest(request, host, script, message);
    }
  }

  return MediaServer::httpRequest(request, socket);
}

SHttpServer::ResponseMessage InternetServer::editRequest(const SHttpServer::RequestMessage &request, const QString &host, const QString &script, const QString &message)
{
  HtmlParser htmlParser;
  htmlParser.setField("TR_ADD_ICON", tr("Add icon"));
  htmlParser.setField("TR_ADD", tr("Add"));
  htmlParser.setField("TR_CLOSE", tr("Close"));
  htmlParser.setField("TR_CLOSE_WINDOW", tr("This window can be closed."));
  htmlParser.setField("TR_DOMAIN", tr("Domain"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_SCRIPT_EDITOR", tr("Script editor"));
  htmlParser.setField("MESSAGE", message);

  if (!host.isEmpty() && !script.isEmpty())
  {
    htmlParser.setField("BUTTONS", QByteArray(""));
    if (siteDatabase->isLocal(host))
    {
      htmlParser.setField("BUTTON_NAME", QByteArray("delete"));

      if (siteDatabase->isGlobal(host))
        htmlParser.setField("BUTTON_TEXT", tr("Revert"));
      else
        htmlParser.setField("BUTTON_TEXT", tr("Delete"));

      htmlParser.appendField("BUTTONS", htmlParser.parse(htmlSiteEditButton));
    }

    htmlParser.setField("SERVER_PATH", QUrl(serverPath()).toEncoded());
    htmlParser.setField("SCRIPT_HOST", host);
    htmlParser.setField("SCRIPT_NAME", siteDatabase->getName(host));
    htmlParser.setField("SCRIPT", script);

    SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
    response.setField("Cache-Control", "no-cache");
    response.setContentType("text/html;charset=utf-8");
    response.setContent(htmlParser.parse(htmlSiteEditIndex));
    return response;
  }
  else
  {
    SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
    response.setField("Cache-Control", "no-cache");
    response.setContentType("text/html;charset=utf-8");
    response.setContent(htmlParser.parse(htmlSiteEditCloseIndex));
    return response;
  }
}

} } // End of namespaces
