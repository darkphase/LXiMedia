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

#include "backend.h"

const char * const Backend::htmlIndex =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">\n"
    "<head>\n"
    " <title>{_PRODUCT} @ {_HOSTNAME}</title>\n"
    " <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />\n"
    " <link rel=\"stylesheet\" href=\"/main.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    " <link rel=\"stylesheet\" href=\"/menu.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n"
    "{HEAD}"
    "</head>\n"
    "<body>\n"
    " <table class=\"head\">\n"
    "  <tr class=\"head\">\n"
    "   <td class=\"headlogo\">{TR_LOGO}</td>\n"
    "{HEAD_MENUITEMS}"
    "  </tr>\n"
    " </table>\n"
    " <table class=\"submenu\">\n"
    "  <tr class=\"submenu\">\n"
    "   <td></td>\n"
    "{HEAD_SUBMENUITEMS}"
    "   <td></td>\n"
    "  </tr>\n"
    " </table>\n"
    "{CONTENT}"
    "</body>\n"
    "</html>\n";

const char * const Backend::htmlMenuItem =
    "   <td class=\"headmenuitem\"><a class=\"headmenuitem\" href=\"{LINK}\">{TEXT}</a></td>\n";

const char * const Backend::htmlMenuItemSel =
    "   <td class=\"headmenuitemsel\">{TEXT}</td>\n";

const char * const Backend::htmlSubMenuItem =
    "   <td class=\"submenuitem\"><a class=\"submenuitem\" href=\"{LINK}\">{TEXT}</a></td>\n";

const char * const Backend::htmlSubMenuItemSel =
    "   <td class=\"submenuitemsel\">{TEXT}</td>\n";

//const char * const Backend::htmlMenuSideShutdownItem =
//    "     <tr><td class=\"menu\" align=\"center\">\n"
//    "      <script language=\"JavaScript\" type=\"text/javascript\">\n"
//    "       <!--\n"
//    "       function confirm{ACTION}()\n"
//    "       {\n"
//    "         if (confirm(\"{CONFIRM_TEXT}\"))\n"
//    "           window.location = \"{LINK}\";\n"
//    "       }\n"
//    "       document.write(\"<p class=\\\"menuitem\\\"><a class=\\\"menuitem\\\" href=\\\"javascript:confirm{ACTION}();\\\">{TEXT}</a></p>\");\n"
//    "       //-->\n"
//    "      </script>\n"
//    "      <noscript>\n"
//    "       <p class=\"menuitem\"><a class=\"menuitem\" href=\"{LINK}\">{TEXT}</a></p>\n"
//    "      </noscript>\n"
//    "     </td></tr>\n";

const char * const Backend::htmlMain =
    " <table class=\"widgetsfull\">\n"
    "{WIDGET_ROWS}"
    " </table>\n";

const char * const Backend::htmlWidgetRow =
    "  <tr class=\"widgets\">\n"
    "{WIDGETS}"
    "  </tr>\n";

const char * const Backend::htmlWidgetButton =
    "      <td class=\"widgetbutton\">\n"
    "       <div class=\"widgetbuttonsmall\">\n"
    "        <a class=\"widgetbutton\" href=\"{LINK}\">\n"
    "         <img src=\"{ICON}\" alt=\"{TEXT}\" />\n"
    "        </a>\n"
    "       </div>\n"
    "       <div class=\"widgetbuttonsmalltitle\">\n"
    "        <a class=\"widgetbuttontitle\" href=\"{LINK}\">{TEXT}</a>\n"
    "       </div>\n"
    "      </td>\n";

const char * const Backend::htmlSearchWidget =
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <span class=\"logoc\">{TR_MEDIA}</span><span class=\"logoa\">{TR_SEARCH}</span>\n"
    "    <form name=\"search\" action=\"\" method=\"get\">\n"
    "     <input type=\"text\" size=\"60\" name=\"q\" value=\"\" /><br /><br />\n"
    "     <input type=\"submit\" value=\"{TR_SEARCH}\" />\n"
    "    </form>\n"
    "   </td>\n";

const char * const Backend::htmlToolboxWidget =
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <table class=\"widgetbuttons\">\n"
    "     <tr class=\"widgetbuttons\">\n"
    "{BUTTONS}"
    "     </tr>\n"
    "    </table>\n"
    "   </td>\n";

const char * const Backend::htmlSearchResults =
    "<table class=\"searchresultlistbase\">\n"
    " <tr>\n"
    "  <td class=\"searchresultlistsearchbar\">\n"
    "   <form name=\"search\" action=\"\" method=\"get\">\n"
    "    {TR_SEARCH}:\n"
    "    <input type=\"text\" size=\"40\" name=\"q\" value=\"{QUERY}\" />\n"
    "    <input type=\"submit\" value=\"{TR_SEARCH}\" />\n"
    "   </form><br />\n"
    "   <p class=\"light\">{TR_RESULTS} {FROM} - {TO} {TR_OF} {OF} ({TIME} {TR_SECONDS})</p>\n"
    "  </td>\n"
    " </tr>\n"
    " <tr><td>\n"
    "  <table class=\"searchresultlist\">\n"
    "{SEARCHRESULTS}"
    "  </table>\n"
    " </td></tr>\n"
    " <tr><td>\n"
    "  <div class=\"pageselector\">\n"
    "   {TR_PAGE}:{PAGES}<br />\n"
    "  </div>\n"
    " </td></tr>\n"
    "</table>\n";

const char * const Backend::htmlSearchResultsPage =
    " <a class=\"pageselector\" href=\"{ITEM_LINK}\">{ITEM_NUMBER}</a>";

const char * const Backend::htmlSearchResultsItem =
    " <tr class=\"searchresultlist\"><td class=\"searchresultlistitemhead\" colspan=\"2\">\n"
    "  <a class=\"searchresultlistitem\" href=\"{ITEM_LINK}\">{ITEM_HEADLINE}</a>\n"
    " </td></tr>\n"
    " <tr class=\"searchresultlist\">\n"
    "  <td class=\"searchresultlistitem\" width=\"68\"></td>\n"
    "  <td class=\"searchresultlistitem\">\n"
    "   <p class=\"light\">{ITEM_TEXT}</p>\n"
    "   <p class=\"light\"><i>{TR_RELEVANCE} {ITEM_RELEVANCE}</i></p>\n"
    "  </td>\n"
    " </tr>\n";

const char * const Backend::htmlSearchResultsItemThumb =
    " <tr class=\"searchresultlist\"><td class=\"searchresultlistitemhead\" colspan=\"2\">\n"
    "  <a class=\"searchresultlistitem\" href=\"{ITEM_LINK}\">{ITEM_HEADLINE}</a>\n"
    " </td></tr>\n"
    " <tr class=\"searchresultlist\">\n"
    "  <td class=\"searchresultlistitemthumb\" width=\"68\">\n"
    "   <a href=\"{ITEM_LINK}\"><img src=\"{ITEM_ICON}?size=64\" alt=\"Thumb\" /></a>\n"
    "  </td>\n"
    "  <td class=\"searchresultlistitem\">\n"
    "   <p class=\"light\">{ITEM_TEXT}</p>\n"
    "   <p class=\"light\"><i>{TR_RELEVANCE} {ITEM_RELEVANCE}</i></p>\n"
    "  </td>\n"
    " </tr>\n";

const char * const Backend::htmlErrorLogWidget =
    "   <td class=\"widget\">\n"
    "    <p class=\"head\">{TR_ERRORS}</p>\n"
    "    <table>\n"
    "     <tr><td colspan=\"2\">{TR_HELP_SUBMIT}<br /><br /></td></tr>\n"
    "{ERROR_LOG_FILES}"
    "     <tr><td class=\"right\" colspan=\"2\">\n"
    "      <br />\n"
    "      <a href=\"?dismisserrors\">{TR_DISMISS}</a>\n"
    "     </td></tr>\n"
    "    </table>\n"
    "   </td>\n";

const char * const Backend::htmlErrorLogWidgetFile =
    "     <tr>\n"
    "      <td>{ITEM_DATE}</td>\n"
    "      <td><a href=\"{ITEM_LINK}\">{ITEM_NAME}</a></td>\n"
    "     </tr>\n";

const char * const Backend::htmlLogFile =
    "<table class=\"log\">\n"
    " <tr class=\"log\"><th class=\"log\">{TR_DATE}</th><th class=\"log\">{TR_TYPE}</th><th class=\"log\">PID:TID</th><th class=\"log\">{TR_MESSAGE}</th></tr>\n"
    "{LOG_MESSAGES}"
    "</table>\n"
    "<a name=\"bottom\"></a>\n";

const char * const Backend::htmlLogFileHeadline =
    " <tr class=\"log\">\n"
    "  <td width=\"140\" rowspan=\"{ITEM_ROWS}\" class=\"{ITEM_CLASS}\">\n"
    "   {ITEM_DATE}\n"
    "  </td>\n"
    "  <td width=\"40\" rowspan=\"{ITEM_ROWS}\" class=\"{ITEM_CLASS}\">\n"
    "   {ITEM_TYPE}\n"
    "  </td>\n"
    "  <td width=\"40\" rowspan=\"{ITEM_ROWS}\" class=\"{ITEM_CLASS}\">\n"
    "   {ITEM_PID}:{ITEM_TID}\n"
    "  </td>\n"
    "  <td class=\"{ITEM_CLASS}\">\n"
    "   {ITEM_HEADLINE}\n"
    "  </td>\n"
    " </tr>\n";

const char * const Backend::htmlLogFileMessage =
    " <tr class=\"log\">\n"
    "  <td class=\"{ITEM_CLASS}\">\n"
    "{ITEM_MESSAGE}\n"
    "  </td>\n"
    " </tr>\n";

const char * const Backend::htmlAbout =
    " <h1>{_PRODUCT}</h1>\n"
    " Version: {VERSION}<br />\n"
    " Website: <a href=\"http://lximedia.sourceforge.net/\">lximedia.sourceforge.net</a><br />\n"
    " <br />\n"
    " <b>Copyright &copy; 2009-2010 A.J. Admiraal. All rights reserved.</b><br />\n"
    " <br />\n"
    " This program is free software; you can redistribute it and/or modify it\n"
    " under the terms of the GNU General Public License version 2 as published\n"
    " by the Free Software Foundation.<br />\n"
    " <br />\n"
    " This program is distributed in the hope that it will be useful, but WITHOUT\n"
    " ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
    " FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br />\n"
    " <br />\n"
    "{ABOUT_LXISTREAM}"
    " <h1>Qt</h1>\n"
    " Version: {QT_VERSION}<br />\n"
    " Website: <a href=\"http://qt.nokia.com/\">qt.nokia.com</a><br />\n"
    " <br />\n"
    " <b>Copyright &copy; 2010 Nokia Corporation and/or its subsidiary(-ies).</b><br />\n"
    " <br />\n"
    " Used under the terms of the GNU Lesser General Public License version 2.1\n"
    " as published by the Free Software Foundation.<br />\n";

const char * const Backend::htmlConfigMain =
    " <table class=\"widgetsfull\">\n"
    "  <tr class=\"widgets\">\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_HTTPSERVER_SETTINGS}</p>\n"
    "    {TR_HTTPSERVER_EXPLAIN}<br />\n"
    "    <br />\n"
    "    <form name=\"httpsettings\" action=\"settings.html\" method=\"get\">\n"
    "     <input type=\"hidden\" name=\"httpsettings\" value=\"httpsettings\" />\n"
    "     {TR_HTTP_PORT_NUMBER}:\n"
    "     <input type=\"text\" size=\"6\" name=\"httpport\" value=\"{HTTPPORT}\" /><br />\n"
    "     <br />\n"
    "     <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "    </form>\n"
    "   </td>\n"
    "   <td class=\"widget\" width=\"50%\">\n"
    "    <p class=\"head\">{TR_IMDB_SETTINGS}</p>\n"
    "    {TR_DOWNLOAD_IMDB_EXPLAIN}<br />\n"
    "    <a class=\"light\" href=\"http://www.imdb.com/interfaces\">www.imdb.com/interfaces</a><br />\n"
    "    <br />\n"
    "{IMDB_ACTION}"
    "   </td>\n"
    "  </tr>\n"
    "  <tr class=\"widgets\"><td class=\"widget\" colspan=\"2\">\n"
    "   <form name=\"dlnasettings\" action=\"settings.html\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"dlnasettings\" value=\"dlnasettings\" />\n"
    "    <table class=\"main\">\n"
    "     <tr class=\"main\"><td class=\"center\" colspan=\"4\">\n"
    "      <p class=\"head\">{TR_DLNA_SETTINGS}</p>\n"
    "     </td></tr>\n"
    "     <tr class=\"main\">\n"
    "      <td class=\"left\"></td>\n"
    "      <td class=\"left\">\n"
    "       <p class=\"head2\">{TR_MEDIA_TRANSCODE_SETTINGS}</p>"
    "       <p class=\"light\">{TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN}</p>"
    "      </td>\n"
    "      <td class=\"left\">\n"
    "       <p class=\"head2\">{TR_MUSIC_TRANSCODE_SETTINGS}</p>"
    "       <p class=\"light\">{TR_MUSIC_TRANSCODE_SETTINGS_EXPLAIN}</p>"
    "      </td>\n"
    "      <td class=\"right\"></td>\n"
    "     </tr>\n"
    "{CLIENT_ROWS}"
    "     <tr class=\"main\"><td class=\"center\" colspan=\"4\">\n"
    "      <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "     </td></tr>\n"
    "    </table>\n"
    "   </form>\n"
    "  </td></tr>\n"
    " </table>\n";

const char * const Backend::htmlConfigDlnaDefaultRow =
    "   <tr class=\"main\">\n"
    "    <td class=\"left\">\n"
    "     <p class=\"head2\">{NAME}</p>"
    "    </td>\n"
    "    <td class=\"left\">\n"
    "     <select name=\"transcodesize\">\n"
    "{FORMATS}"
    "     </select>\n"
    "     <select name=\"cropmode\">\n"
    "      <option value=\"Box\" {SELECTED_BOX}>{TR_LETTERBOX}</option>\n"
    "      <option value=\"Zoom\" {SELECTED_ZOOM}>{TR_FULLSCREEN}</option>\n"
    "     </select>\n"
    "     <select name=\"channels\">\n"
    "{CHANNELS}"
    "     </select>\n"
    "     <select name=\"encodemode\">\n"
    "      <option value=\"Fast\" {SELECTED_FAST}>{TR_FAST}</option>\n"
    "      <option value=\"Slow\" {SELECTED_SLOW}>{TR_HIGH_QUALITY}</option>\n"
    "     </select>\n"
    "    </td>\n"
    "    <td class=\"left\">\n"
    "     <select name=\"musicchannels\">\n"
    "{MUSICCHANNELS}"
    "     </select>\n"
    "    </td>\n"
    "    <td class=\"right\">\n"
    "     <input type=\"submit\" name=\"defaults\" value=\"{TR_DEFAULTS}\" />\n"
    "    </td>\n"
    "   </tr>\n";

const char * const Backend::htmlConfigDlnaClientRow =
    "   <tr class=\"main\">\n"
    "    <td class=\"left\">\n"
    "     <p class=\"head2\" title=\"{USERAGENT}\">{NAME}</p>\n"
    "     <p class=\"light\">{LAST_SEEN}</p>"
    "    </td>\n"
    "    <td class=\"left\">\n"
    "     <select name=\"transcodesize-{NAME}\">\n"
    "{FORMATS}"
    "     </select>\n"
    "     <select name=\"cropmode-{NAME}\">\n"
    "      <option value=\"Box\" {SELECTED_BOX}>{TR_LETTERBOX}</option>\n"
    "      <option value=\"Zoom\" {SELECTED_ZOOM}>{TR_FULLSCREEN}</option>\n"
    "     </select>\n"
    "     <select name=\"channels-{NAME}\">\n"
    "{CHANNELS}"
    "     </select>\n"
    "     <select name=\"encodemode-{NAME}\">\n"
    "      <option value=\"Fast\" {SELECTED_FAST}>{TR_FAST}</option>\n"
    "      <option value=\"Slow\" {SELECTED_SLOW}>{TR_HIGH_QUALITY}</option>\n"
    "     </select>\n"
    "    </td>\n"
    "    <td class=\"left\">\n"
    "     <select name=\"musicchannels-{NAME}\">\n"
    "{MUSICCHANNELS}"
    "     </select>\n"
    "    </td>\n"
    "    <td class=\"right\">\n"
    "     <input type=\"submit\" name=\"erase-{NAME}\" value=\"{TR_ERASE}\" />\n"
    "    </td>\n"
    "   </tr>\n";

const char * const Backend::htmlConfigOption =
    "      <option value=\"{VALUE}\" {SELECTED}>{TEXT}</option>\n";

const char * const Backend::htmlConfigImdbDownload =
    "   <form name=\"imdbsettings\" action=\"settings.html\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"imdbsettings\" value=\"imdbsettings\" />\n"
    "    <input type=\"submit\" name=\"download\" value=\"{TR_DOWNLOAD_IMDB}\" />\n"
    "   </form>\n";

const char * const Backend::headSearchResults =
    " <link rel=\"stylesheet\" href=\"/list.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n";


HttpServer::SocketOp Backend::handleHtmlRequest(const QUrl &url, const QString &, QAbstractSocket *socket)
{
  HtmlParser htmlParser(this->htmlParser);
  htmlParser.setField("TR_SEARCH", tr("Search"));

  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  htmlParser.setField("WIDGET_ROWS", QByteArray(""));
  htmlParser.setField("WIDGETS", QByteArray(""));

  // Search
  htmlParser.appendField("WIDGETS", htmlParser.parse(htmlSearchWidget));

  // Toolbox
  htmlParser.setField("BUTTONS", QByteArray(""));

#ifndef QT_NO_DEBUG
  htmlParser.setField("TEXT", tr("Shutdown"));
  htmlParser.setField("ICON", QByteArray("/img/shutdown.png"));
  htmlParser.setField("LINK", QByteArray("/?shutdown"));
  htmlParser.appendField("BUTTONS", htmlParser.parse(htmlWidgetButton));

  htmlParser.setField("TEXT", tr("Restart"));
  htmlParser.setField("ICON", QByteArray("/img/restart.png"));
  htmlParser.setField("LINK", QByteArray("/?restart"));
  htmlParser.appendField("BUTTONS", htmlParser.parse(htmlWidgetButton));
#endif

  htmlParser.appendField("WIDGETS", htmlParser.parse(htmlToolboxWidget));

  htmlParser.appendField("WIDGET_ROWS", htmlParser.parse(htmlWidgetRow));
  htmlParser.setField("WIDGETS", QByteArray(""));

  // Custom widgets
  QList<QByteArray> widgets;
  foreach (const BackendServer *server, backendServers)
  {
    const QByteArray widget = server->frontPageWidget();
    if (!widget.isEmpty())
      widgets += widget;
  }

  // Submit error log
  const QSet<QString> dismissedFiles =
      QSet<QString>::fromList(GlobalSettings().value("DismissedErrors").toStringList());

  QStringList errorLogFiles;
  foreach (const QString &file, SDebug::LogFile::errorLogFiles())
  if (!dismissedFiles.contains(file))
    errorLogFiles += file;

  if (!errorLogFiles.isEmpty())
  {
    htmlParser.setField("TR_ERRORS",tr("Program errors"));
    htmlParser.setField("TR_DISMISS",tr("Dismiss errors"));
    htmlParser.setField("TR_HELP_SUBMIT",
        tr("Unexpected errors were found in the log files. Please take some time "
           "to to submit a bug report on "
           "<a href=\"http://sourceforge.net/projects/lximedia/develop\">"
           "sourceforge.net/projects/lximedia/develop</a> if you think these "
           "errors are due to bugs in the software."));

    htmlParser.setField("ERROR_LOG_FILES", QByteArray(""));
    QString lastDate;
    for (int i=0; i<errorLogFiles.count(); i++)
    {
      const QFileInfo info(errorLogFiles[i]);

      htmlParser.setField("ITEM", QByteArray::number(i));
      htmlParser.setField("ITEM_NAME", info.fileName());
      htmlParser.setField("ITEM_LINK", "/" + info.fileName());

      const QString date = info.created().toString("yyyy-MM-dd");
      htmlParser.setField("ITEM_DATE", (date != lastDate) ? date : QString::null);
      lastDate = date;

      htmlParser.appendField("ERROR_LOG_FILES", htmlParser.parse(htmlErrorLogWidgetFile));
    }

    widgets += htmlParser.parse(htmlErrorLogWidget);
  }

  int count = 0;
  foreach (const QByteArray &widget, widgets)
  {
    htmlParser.appendField("WIDGETS", widget);
    if (++count >= 2)
    {
      htmlParser.appendField("WIDGET_ROWS", htmlParser.parse(htmlWidgetRow));
      htmlParser.setField("WIDGETS", QByteArray(""));
      count = 0;
    }
  }

  if (count)
  {
    htmlParser.appendField("WIDGET_ROWS", htmlParser.parse(htmlWidgetRow));
    htmlParser.setField("WIDGETS", QByteArray(""));
  }

  socket->write(response);
  socket->write(parseHtmlContent(url, htmlParser.parse(htmlMain), ""));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp Backend::handleHtmlSearch(const QUrl &url, const QString &, QAbstractSocket *socket)
{
  static const int resultsPerPage = 30;

  HtmlParser htmlParser(this->htmlParser);
  htmlParser.setField("TR_OF", tr("of"));
  htmlParser.setField("TR_PAGE", tr("Page"));
  htmlParser.setField("TR_RELEVANCE", tr("Relevance"));
  htmlParser.setField("TR_RESULTS", tr("Results"));
  htmlParser.setField("TR_SEARCH", tr("Search"));
  htmlParser.setField("TR_SECONDS", tr("seconds"));

  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  const QString queryValue = url.queryItemValue("q");
  const QString queryString = QByteArray::fromPercentEncoding(queryValue.toAscii().replace('+', ' '));
  htmlParser.setField("QUERY", queryString);

  const SearchCacheEntry entry = search(queryString);
  htmlParser.setField("SEARCHRESULTS", QByteArray(""));

  const int first = url.queryItemValue("first").toUInt();
  const int last = qMin(first + resultsPerPage, entry.results.count()) - 1;
  int count = 0;

  htmlParser.setField("TIME", QByteArray::number(qreal(entry.duration) / 1000.0, 'f', 2));
  htmlParser.setField("FROM", QByteArray::number(first));
  htmlParser.setField("TO", QByteArray::number(last));
  htmlParser.setField("OF", QByteArray::number(entry.results.count()));

  for (QMultiMap<qreal, BackendServer::SearchResult>::ConstIterator i = entry.results.begin();
       i != entry.results.end();
       i++)
  {
    if ((count >= first) && (count <= last))
    {
      htmlParser.setField("ITEM_HEADLINE", i->headline);
      htmlParser.setField("ITEM_LINK", i->location);
      htmlParser.setField("ITEM_TEXT", i->text);
      htmlParser.setField("ITEM_RELEVANCE", QString::number(qBound(0, int(i->relevance * 100.0), 100)) + "%");

      if (!i->thumbLocation.isEmpty())
      {
        htmlParser.setField("ITEM_ICON", i->thumbLocation);
        htmlParser.appendField("SEARCHRESULTS", htmlParser.parse(htmlSearchResultsItemThumb));
      }
      else
        htmlParser.appendField("SEARCHRESULTS", htmlParser.parse(htmlSearchResultsItem));
    }
    else if (count > last)
      break;

    count++;
  }

  htmlParser.setField("PAGES", QByteArray(""));
  for (int i=0, n=(entry.results.count() + (resultsPerPage - 1)) / resultsPerPage; i<n; i++)
  {
    if ((i * resultsPerPage) != first)
    {
      htmlParser.setField("ITEM_LINK", "/?q=" + queryValue + "&amp;first=" + QString::number(i * resultsPerPage));
      htmlParser.setField("ITEM_NUMBER", QByteArray::number(i + 1));
      htmlParser.appendField("PAGES", htmlParser.parse(htmlSearchResultsPage));
    }
    else
      htmlParser.appendField("PAGES", QByteArray::number(i + 1));
  }

  if (last < entry.results.count() - 1)
  {
    htmlParser.setField("ITEM_LINK", "/?q=" + queryValue + "&amp;first=" + QString::number(last + 1));
    htmlParser.setField("ITEM_NUMBER", tr("Next"));
    htmlParser.appendField("PAGES", htmlParser.parse(htmlSearchResultsPage));
  }

  socket->write(response);
  socket->write(parseHtmlContent(url, htmlParser.parse(htmlSearchResults), headSearchResults));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp Backend::showAbout(const QUrl &url, QAbstractSocket *socket)
{
  HtmlParser htmlParser(this->htmlParser);

  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  htmlParser.setField("VERSION", QByteArray(GlobalSettings::version()));
  htmlParser.setField("ABOUT_LXISTREAM", SSystem::about());
  htmlParser.setField("QT_VERSION", QByteArray(qVersion()));

  socket->write(response);
  socket->write(parseHtmlContent(url, htmlParser.parse(htmlAbout),""));
  return HttpServer::SocketOp_Close;
}

HttpServer::SocketOp Backend::handleHtmlConfig(const QUrl &url, QAbstractSocket *socket)
{
  HttpServer::ResponseHeader response(HttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  GlobalSettings settings;

  // HTTP server
  if (url.hasQueryItem("httpsettings") && url.hasQueryItem("httpport"))
  {
    const int portValue = url.queryItemValue("httpport").toInt();
    if ((portValue > 0) && (portValue < 65536))
      settings.setValue("HttpPort", portValue);
  }

  htmlParser.setField("TR_HTTPSERVER_SETTINGS", tr("HTTP server settings"));
  htmlParser.setField("TR_HTTP_PORT_NUMBER", tr("Preferred HTTP port number"));
  htmlParser.setField("TR_HTTPSERVER_EXPLAIN",
    tr("This form configures the internal HTTP server. Note that the daemon "
       "needs to be restarted to apply these settings."));

  htmlParser.setField("HTTPPORT", settings.value("HttpPort", settings.defaultBackendHttpPort()).toString());

  // IMDB
  if (url.hasQueryItem("imdbsettings") && url.hasQueryItem("download"))
  {
    ImdbClient::obtainIMDBFiles();
  }

  htmlParser.setField("TR_IMDB_SETTINGS", tr("IMDb settings"));
  htmlParser.setField("TR_DOWNLOAD_IMDB_EXPLAIN",
    tr("This form will allow the imdb.com data files to be downloaded to "
       "provide additional information on the media files that are available. "
       "Downloading and parsing the files will take several minutes."));

  if (ImdbClient::isAvailable() && !ImdbClient::needUpdate())
  {
    htmlParser.setField("IMDB_ACTION", "    <b>" + tr("IMDb files are available") + "</b>\n");
  }
  else if (ImdbClient::isDownloading() || !ImdbClient::needUpdate())
  {
    htmlParser.setField("IMDB_ACTION", "    <b>" + tr("Downloading and parsing IMBb files") + " ...</b>\n");
  }
  else
  {
    htmlParser.setField("TR_DOWNLOAD_IMDB", ImdbClient::needUpdate() ? tr("Update IMDb files") : tr("Download IMDb files"));
    htmlParser.setField("IMDB_ACTION", htmlParser.parse(htmlConfigImdbDownload));
  }

  // DLNA
  settings.beginGroup("DLNA");

  if (url.hasQueryItem("dlnasettings") && url.hasQueryItem("save"))
  {
    const QString sizeName =
        QByteArray::fromPercentEncoding(url.queryItemValue("transcodesize").toAscii().replace('+', ' '));
    if (!sizeName.isEmpty())
      settings.setValue("TranscodeSize", sizeName);
    else
      settings.remove("TranscodeSize");

    const QString cropName =
        QByteArray::fromPercentEncoding(url.queryItemValue("cropmode").toAscii().replace('+', ' '));
    if (!cropName.isEmpty())
      settings.setValue("TranscodeCrop", cropName);
    else
      settings.remove("TranscodeCrop");

    const QString encodeModeName =
        QByteArray::fromPercentEncoding(url.queryItemValue("encodemode").toAscii().replace('+', ' '));
    if (!encodeModeName.isEmpty())
      settings.setValue("EncodeMode", encodeModeName);
    else
      settings.remove("EncodeMode");

    const QString channelsName =
        QByteArray::fromPercentEncoding(url.queryItemValue("channels").toAscii().replace('+', ' '));
    if (!channelsName.isEmpty())
      settings.setValue("TranscodeChannels", channelsName);
    else
      settings.remove("TranscodeChannels");

    const QString musicChannelsName =
        QByteArray::fromPercentEncoding(url.queryItemValue("musicchannels").toAscii().replace('+', ' '));
    if (!musicChannelsName.isEmpty())
      settings.setValue("TranscodeMusicChannels", musicChannelsName);
    else
      settings.remove("TranscodeMusicChannels");

    foreach (const QString &group, settings.childGroups())
    if (group.startsWith("Client_"))
    {
      settings.beginGroup(group);
      const QString name = group.mid(7);

      const QString clientSizeName =
          QByteArray::fromPercentEncoding(url.queryItemValue("transcodesize-" + name).toAscii().replace('+', ' '));
      if (!clientSizeName.isEmpty() && (clientSizeName != sizeName))
        settings.setValue("TranscodeSize", clientSizeName);
      else
        settings.remove("TranscodeSize");

      const QString clientCropName =
          QByteArray::fromPercentEncoding(url.queryItemValue("cropmode-" + name).toAscii().replace('+', ' '));
      if (!clientCropName.isEmpty() && (clientCropName != cropName))
        settings.setValue("TranscodeCrop", clientCropName);
      else
        settings.remove("TranscodeCrop");

      const QString clientEncodeModeName =
          QByteArray::fromPercentEncoding(url.queryItemValue("encodemode-" + name).toAscii().replace('+', ' '));
      if (!clientEncodeModeName.isEmpty() && (clientEncodeModeName != encodeModeName))
        settings.setValue("EncodeMode", clientEncodeModeName);
      else
        settings.remove("EncodeMode");

      const QString clientChannelsName =
          QByteArray::fromPercentEncoding(url.queryItemValue("channels-" + name).toAscii().replace('+', ' '));
      if (!clientChannelsName.isEmpty() && (clientChannelsName != channelsName))
        settings.setValue("TranscodeChannels", clientChannelsName);
      else
        settings.remove("TranscodeChannels");

      const QString clientMusicChannelsName =
          QByteArray::fromPercentEncoding(url.queryItemValue("musicchannels-" + name).toAscii().replace('+', ' '));
      if (!clientMusicChannelsName.isEmpty() && (clientMusicChannelsName != musicChannelsName))
        settings.setValue("TranscodeMusicChannels", clientMusicChannelsName);
      else
        settings.remove("TranscodeMusicChannels");

      settings.endGroup();
    }
  }
  else if (url.hasQueryItem("dlnasettings") && url.hasQueryItem("defaults"))
  {
    settings.remove("TranscodeSize");
    settings.remove("TranscodeCrop");
    settings.remove("EncodeMode");
    settings.remove("TranscodeChannels");
    settings.remove("TranscodeMusicChannels");
  }
  else if (url.hasQueryItem("dlnasettings"))
  {
    foreach (const QString &group, settings.childGroups())
    if (group.startsWith("Client_") && url.hasQueryItem("erase-" + group.mid(7)))
      settings.remove(group);
  }

  const QString genericTranscodeSize =
      settings.value("TranscodeSize", settings.defaultTranscodeSizeName()).toString();
  const QString genericTranscodeCrop =
      settings.value("TranscodeCrop", settings.defaultTranscodeCropName()).toString();
  const QString genericEncodeMode =
      settings.value("EncodeMode", settings.defaultEncodeModeName()).toString();
  const QString genericTranscodeChannels =
      settings.value("TranscodeChannels", settings.defaultTranscodeChannelName()).toString();
  const QString genericTranscodeMusicChannels =
      settings.value("TranscodeMusicChannels", settings.defaultTranscodeMusicChannelName()).toString();

  HtmlParser htmlParser;
  htmlParser.setField("TR_DLNA_SETTINGS", tr("DLNA transcode settings"));
  htmlParser.setField("TR_MEDIA_TRANSCODE_SETTINGS", tr("Media transcode settings"));
  htmlParser.setField("TR_MUSIC_TRANSCODE_SETTINGS", tr("Music transcode settings"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_ERASE", tr("Erase"));
  htmlParser.setField("TR_DEFAULTS", tr("Defaults"));
  htmlParser.setField("TR_LETTERBOX", tr("Letterbox"));
  htmlParser.setField("TR_FULLSCREEN", tr("Fullscreen"));
  htmlParser.setField("TR_FAST", tr("Fast"));
  htmlParser.setField("TR_HIGH_QUALITY", tr("High quality"));

  htmlParser.setField("TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN",
    tr("The transcode settings for the DLNA clients. Note that higher settings "
       "require more CPU power and more network bandwidth. The \"Letterbox\" "
       "setting will add black bars to the image if the aspect ratio does not "
       "match, the \"Fullscreen\" setting will zoom in and cut off part of the "
       "image. The \"High quality\" setting will use more CPU power but gives "
       "higher quality images than the \"Fast\" setting, which only encodes "
       "intra-frames."));

  htmlParser.setField("TR_MUSIC_TRANSCODE_SETTINGS_EXPLAIN",
    tr("The transcode settings specifically for music."));

  htmlParser.setField("CLIENT_ROWS", QByteArray(""));

  struct T
  {
    static void addFormat(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeSize, const GlobalSettings::TranscodeSize &size)
    {
      if (settings.value("TranscodeSize", genericTranscodeSize).toString() == size.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", size.name);
      htmlParser.setField("TEXT", size.name +
                          " (" + QString::number(size.size.width()) +
                          "x" + QString::number(size.size.height()) + ")");
      htmlParser.appendField("FORMATS", htmlParser.parse(htmlConfigOption));
    }

    static void addChannel(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeChannels, const GlobalSettings::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeChannels", genericTranscodeChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("CHANNELS", htmlParser.parse(htmlConfigOption));
    }

    static void addMusicChannel(HtmlParser &htmlParser, GlobalSettings &settings, const QString &genericTranscodeMusicChannels, const GlobalSettings::TranscodeChannel &channel)
    {
      if (settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString() == channel.name)
        htmlParser.setField("SELECTED", QByteArray("selected=\"selected\""));
      else
        htmlParser.setField("SELECTED", QByteArray(""));

      htmlParser.setField("VALUE", channel.name);
      htmlParser.setField("TEXT", channel.name);
      htmlParser.appendField("MUSICCHANNELS", htmlParser.parse(htmlConfigOption));
    }
  };

  // Default settings
  htmlParser.setField("NAME", tr("Default settings"));
  htmlParser.setField("FORMATS", QByteArray(""));
  htmlParser.setField("CHANNELS", QByteArray(""));
  htmlParser.setField("MUSICCHANNELS", QByteArray(""));

  foreach (const GlobalSettings::TranscodeSize &size, settings.allTranscodeSizes())
    T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

  foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
    T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

  foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
    T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

  if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
  {
    htmlParser.setField("SELECTED_BOX", QByteArray("selected=\"selected\""));
    htmlParser.setField("SELECTED_ZOOM", QByteArray(""));
  }
  else
  {
    htmlParser.setField("SELECTED_BOX", QByteArray(""));
    htmlParser.setField("SELECTED_ZOOM", QByteArray("selected=\"selected\""));
  }

  if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
  {
    htmlParser.setField("SELECTED_FAST", QByteArray("selected=\"selected\""));
    htmlParser.setField("SELECTED_SLOW", QByteArray(""));
  }
  else
  {
    htmlParser.setField("SELECTED_FAST", QByteArray(""));
    htmlParser.setField("SELECTED_SLOW", QByteArray("selected=\"selected\""));
  }

  htmlParser.appendField("CLIENT_ROWS", htmlParser.parse(htmlConfigDlnaDefaultRow));

  // DLNA clients
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith("Client_"))
  {
    settings.beginGroup(group);

    htmlParser.setField("NAME", group.mid(7));
    htmlParser.setField("USERAGENT", settings.value("UserAgent", tr("Unknown")).toString());
    htmlParser.setField("LAST_SEEN", settings.value("LastSeen").toDateTime().toString(BackendServer::searchDateTimeFormat));
    htmlParser.setField("FORMATS", QByteArray(""));
    htmlParser.setField("CHANNELS", QByteArray(""));
    htmlParser.setField("MUSICCHANNELS", QByteArray(""));

    foreach (const GlobalSettings::TranscodeSize &size, settings.allTranscodeSizes())
      T::addFormat(htmlParser, settings, genericTranscodeSize,  size);

    foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
      T::addChannel(htmlParser, settings, genericTranscodeChannels, channel);

    foreach (const GlobalSettings::TranscodeChannel &channel, settings.allTranscodeChannels())
      T::addMusicChannel(htmlParser, settings, genericTranscodeMusicChannels, channel);

    if (settings.value("TranscodeCrop", genericTranscodeCrop).toString() == "Box")
    {
      htmlParser.setField("SELECTED_BOX", QByteArray("selected=\"selected\""));
      htmlParser.setField("SELECTED_ZOOM", QByteArray(""));
    }
    else
    {
      htmlParser.setField("SELECTED_BOX", QByteArray(""));
      htmlParser.setField("SELECTED_ZOOM", QByteArray("selected=\"selected\""));
    }

    if (settings.value("EncodeMode", genericEncodeMode).toString() == "Fast")
    {
      htmlParser.setField("SELECTED_FAST", QByteArray("selected=\"selected\""));
      htmlParser.setField("SELECTED_SLOW", QByteArray(""));
    }
    else
    {
      htmlParser.setField("SELECTED_FAST", QByteArray(""));
      htmlParser.setField("SELECTED_SLOW", QByteArray("selected=\"selected\""));
    }

    settings.endGroup();
    htmlParser.appendField("CLIENT_ROWS", htmlParser.parse(htmlConfigDlnaClientRow));
  }

  socket->write(response);
  socket->write(parseHtmlContent(url, htmlParser.parse(htmlConfigMain), ""));
  return HttpServer::SocketOp_Close;
}
