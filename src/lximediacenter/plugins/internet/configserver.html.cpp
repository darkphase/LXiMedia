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
#include <QtScript>
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

const char ConfigServer::htmlMain[] =
    " <div class=\"content\">\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{TR_SITE_SELECTION}</legend>\n"
    "   {TR_SITES_EXPLAIN}<br />\n"
    "   <iframe style=\"width:40em;height:40em;\" src=\"site-tree.html\" frameborder=\"0\" name=\"tree\">\n"
    "   </iframe><br />\n"
    "  </fieldset>\n"
    " </div>\n";

const char ConfigServer::htmlTreeIndex[] =
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

const char ConfigServer::htmlTreeDir[] =
    " <tr valign=\"middle\"><td align=\"left\">\n"
    "  <a class=\"hidden\" name=\"{DIR_FULLPATH}\" />\n"
    "  {DIR_INDENT}\n"
    "  {DIR_EXPAND}\n"
    "  {DIR_CHECK}\n"
    "  {DIR_NAME}\n"
    " </td></tr>\n";

const char ConfigServer::htmlTreeIndent[] =
    " <img src=\"/img/null.png\" width=\"16\" height=\"16\" />\n";

const char ConfigServer::htmlTreeExpand[] =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/tree{DIR_OPEN}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char ConfigServer::htmlTreeCheckLink[] =
    " <a class=\"hidden\" href=\"{FILE}?open={DIR_ALLOPEN}&amp;{DIR_CHECKTYPE}={DIR_FULLPATH}#{DIR_FULLPATH}\">\n"
    "  <img src=\"/img/check{DIR_CHECKED}.png\" width=\"16\" height=\"16\" />\n"
    " </a>\n";

const char ConfigServer::htmlTreeCheckIcon[] =
    " <img class=\"thumbnail\" src=\"{ITEM_ICON}?resolution=16x16\" width=\"16\" height=\"16\" />\n";

const char ConfigServer::htmlTreeScriptLink[] =
    " <a class=\"hidden\" href=\"edit.html?identifier={ITEM_IDENTIFIER}\" target=\"_parent\">{ITEM_NAME}</a>\n";

const char ConfigServer::htmlEditMain[] =
    " <div class=\"content\">\n"
    "  <fieldset style=\"float:left;\">\n"
    "   <legend>{IDENTIFIER}.js - {TR_SCRIPT_EDITOR}</legend>\n"
    "   <form name=\"edit\" action=\"edit.html\" method=\"get\">\n"
    "    <table class=\"editmenu\"><tr>\n"
    "     <td>\n"
    "      <input type=\"text\" size=\"40\" name=\"identifier\" value=\"{IDENTIFIER}\" />\n"
    "      <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "      <input type=\"submit\" name=\"remove\" value=\"{TR_REMOVE}\" />\n"
    "     </td><td>\n"
    "      <a href=\"README.txt\" target=\"_blank\">{TR_SCRIPT_INFO}</a>\n"
    "     </td>\n"
    "    </tr></table>\n"
    "    <div class=\"error\">{ERROR}</div>\n"
    "    <textarea name=\"script\" id=\"script\" wrap=\"off\">{SCRIPT}</textarea><br />\n"
    "    <script type=\"text/javascript\">createScriptArea('script');</script>\n"
    "   </form>\n"
    "  </fieldset>\n"
    " </div>\n";

const char ConfigServer::htmlEditHead[] =
    " <style>\n"
    "  table.editmenu {\n"
    "   padding: 0;\n"
    "   margin: 0;\n"
    "   width: 100%;\n"
    "   border: none;\n"
    "  }\n"
    "  .editmenu td {\n"
    "   text-align: right;\n"
    "  }\n"
    "  .editmenu td:first-child {\n"
    "   text-align: left;\n"
    "  }\n"
    "  textarea {\n"
    "   font-size: 1em;\n"
    "   width: 80em;\n"
    "   height: 50em;\n"
    "  }\n"
    "  #script {\n"
    "   margin-left: 30px;\n"
    "   border: 1px solid black;\n"
    "  }\n"
    " .scriptarea {\n"
    "   display: block;\n"
    "   margin: 0;\n"
    "   border: 1px solid black;\n"
    "   border-right: none;\n"
    "   background: #F0EFEF;\n"
    "  } \n"
    "  div.error {\n"
    "   margin-top: 0.25em;\n"
    "   margin-bottom: 0.25em;\n"
    "   color: #800000;\n"
    "   font-weight: bold;\n"
    "  }\n"
    " </style>\n"
    " <script type=\"text/javascript\"><!--\n"
    "  function createScriptArea(id)\n"
    "  {\n"
    "   var el = document.createElement('TEXTAREA');\n"
    "   var ta = document.getElementById(id);\n"
    "   var string = '';\n"
    "   for (var no=1;no<300;no++) {\n"
    "    if (string.length>0) string += '\\n';\n"
    "    string += no;\n"
    "   }\n"
    "   el.className      = 'scriptarea';\n"
    "   el.style.height   = (ta.offsetHeight-4) + \"px\";\n"
    "   el.style.width    = \"25px\";\n"
    "   el.style.position = \"absolute\";\n"
    "   el.style.overflow = 'hidden';\n"
    "   el.style.textAlign = 'right';\n"
    "   el.style.paddingRight = '0.2em';\n"
    "   el.innerHTML      = string;\n"
    "   el.innerText      = string;\n"
    "   el.style.zIndex   = 0;\n"
    "   ta.style.zIndex   = 1;\n"
    "   ta.style.position = \"relative\";\n"
    "   ta.parentNode.insertBefore(el, ta.nextSibling);\n"
    "   setLine();\n"
    "   ta.focus();\n"
    "\n"
    "   ta.onkeydown    = function() { setLine(); }\n"
    "   ta.onmousedown  = function() { setLine(); }\n"
    "   ta.onscroll     = function() { setLine(); }\n"
    "   ta.onblur       = function() { setLine(); }\n"
    "   ta.onfocus      = function() { setLine(); }\n"
    "   ta.onmousewheel = function() { setLine(); }\n"
    "   ta.onmouseover  = function() { setLine(); }\n"
    "   ta.onmouseup    = function() { setLine(); }\n"
    "\n"
    "   function setLine() {\n"
    "    el.scrollTop   = ta.scrollTop;\n"
    "    el.style.top   = (ta.offsetTop) + \"px\";\n"
    "    el.style.left  = (ta.offsetLeft - 27) + \"px\";\n"
    "   }\n"
    "  }//-->\n"
    " </script>\n";

SHttpServer::ResponseMessage ConfigServer::handleHtmlRequest(const SHttpServer::RequestMessage &request, const MediaServer::File &file)
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

  if (file.fullName().endsWith("-tree.html"))
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

    htmlParser.setField("FILE", file.fullName());
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

        htmlParser.setField("ITEM_ICON", '/' + pluginName() + '/' + siteDatabase->category(identifier) + '/' + siteDatabase->reverseDomain(identifier) + "/-thumb.png");
        htmlParser.setField("DIR_CHECK", htmlParser.parse(htmlTreeCheckIcon));

        htmlParser.setField("ITEM_NAME", siteDatabase->reverseDomain(identifier));
        htmlParser.setField("ITEM_IDENTIFIER", siteDatabase->fullIdentifier(identifier));
        htmlParser.setField("DIR_NAME", htmlParser.parse(htmlTreeScriptLink));

        htmlParser.appendField("DIRS", htmlParser.parse(htmlTreeDir));
      }
    }

    SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
    response.setField("Cache-Control", "no-cache");
    response.setContentType("text/html;charset=utf-8");
    response.setContent(htmlParser.parse(htmlTreeIndex));

    return response;
  }
  else if (file.fullName() == "edit.html")
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
}

} } // End of namespaces
