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
    "{HEAD}"
    "</head>\n"
    "<body>\n"
    " <table class=\"location\">\n"
    "  <tr><td>\n"
    "   <div>\n"
    "    <img src=\"/appicon.png\" alt=\"..\" />\n"
    "    <ul>\n"
    "     <li><a href=\"/\">{_HOSTNAME}</a></li>\n"
    "{PATH}"
    "    </ul>\n"
    "   </div>\n"
    "  </td><td>\n"
    "   <div>\n"
    "    <form name=\"search\" action=\"/\" method=\"get\">\n"
    "     <input type=\"text\" size=\"40\" name=\"q\" value=\"{SEARCH_QUERY}\" />\n"
    "     <input type=\"image\" src=\"/img/database-query.png\" alt=\"{TR_SEARCH}\"/>\n"
    "    </form>\n"
    "   </div>\n"
    "  </td></tr>\n"
    " </table>\n"
    "{MAIN_MENUGROUPS}"
    "{CONTENT}"
    "</body>\n"
    "</html>\n";

const char * const Backend::htmlLocationItem =
    "     <li><a href=\"{ITEM_LINK}\">{ITEM_NAME}</a></li>\n";

const char * const Backend::htmlMenuGroup =
    " <div class=\"menu\">\n"
    "  <ul>\n"
    "   <li>{TEXT}</li>\n"
    "{ITEMS}"
    "  </ul>\n"
    " </div>\n";

const char * const Backend::htmlMenuItem =
    "   <li><a href=\"{ITEM_URL}\"><img src=\"{ITEM_ICONURL}\" alt=\"..\">{ITEM_TITLE}</a></li>\n";

const char * const Backend::htmlMain =
    "{LOG_ERRORS}"
    " <div class=\"content\">\n"
    "{GROUPS}"
    " </div>\n";

const char * const Backend::htmlMainGroupItem =
    "  <h1>{ITEM_TITLE}</h1>\n"
    "  <div class=\"thumbnaillist\">\n"
    "{ITEMS}"
    "  </div>\n";

const char * const Backend::htmlMainServerItem =
    "  <div class=\"thumbnaillistitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "   {ITEM_TITLE}\n"
    "  </div>\n";

const char * const Backend::htmlSearchResults =
    " <div class=\"content\">\n"
    "  <h1>{ITEM_TITLE}</h1>\n"
    "  <div class=\"thumbnaillist\">\n"
    "{SEARCHRESULTS}"
    "  </div>\n"
    " </div>\n";

const char * const Backend::htmlSearchResultsItem =
    "  <div class=\"thumbnaillistitem\">\n"
    "   <div class=\"thumbnail\">\n"
    "    <a title=\"{ITEM_TITLE}\" href=\"{ITEM_URL}\">\n"
    "     <img src=\"{ITEM_ICONURL}\" alt=\"{ITEM_TITLE}\" />\n"
    "    </a>\n"
    "   </div>\n"
    "   <div class=\"title\">{ITEM_TITLE}</div>\n"
    "   <div class=\"subtitle\">{TR_RELEVANCE}: {ITEM_RELEVANCE}</div>\n"
    "  </div>\n";

const char * const Backend::htmlLogFile =
    "{LOG_ERRORS}"
    " <div class=\"content\">\n"
    "  <table class=\"detailedlist\">\n"
    "   <tr>\n"
    "    <th class=\"nostretch\">{TR_DATE}</th>\n"
    "    <th class=\"nostretch\">{TR_TYPE}</th>\n"
    "    <th class=\"nostretch\">PID:TID</th>\n"
    "    <th class=\"stretch\">{TR_MESSAGE}</th>\n"
    "   </tr>\n"
    "{LOG_MESSAGES}"
    "  </table>\n"
    "  <a name=\"bottom\"></a>\n"
    " </div>\n";

const char * const Backend::htmlLogFileHeadline =
    "   <tr>\n"
    "    <td class=\"nostretch_a\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_DATE}\n"
    "    </td>\n"
    "    <td class=\"nostretch_a\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_TYPE}\n"
    "    </td>\n"
    "    <td class=\"nostretch_a\" rowspan=\"{ITEM_ROWS}\">\n"
    "     {ITEM_PID}:{ITEM_TID}\n"
    "    </td>\n"
    "    <td class=\"stretch_a\">\n"
    "     {ITEM_HEADLINE}\n"
    "    </td>\n"
    "   </tr>\n";

const char * const Backend::htmlLogFileMessage =
    "   <tr>\n"
    "    <td>\n"
    "{ITEM_MESSAGE}\n"
    "    </td>\n"
    "   </tr>\n";

const char * const Backend::htmlAbout =
    " <div class=\"content\">\n"
    "{ABOUT_LXIMEDIA}"
    " </div>\n";

const char * const Backend::htmlConfigMain =
    " <div class=\"content\">\n"
    "  <fieldset>\n"
    "   <legend>{TR_HTTPSERVER_SETTINGS}</legend>\n"
    "   {TR_HTTPSERVER_EXPLAIN}<br />\n"
    "   <br />\n"
    "   <form name=\"httpsettings\" action=\"settings.html\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"httpsettings\" value=\"httpsettings\" />\n"
    "    {TR_HTTP_PORT_NUMBER}:\n"
    "    <input type=\"text\" size=\"6\" name=\"httpport\" value=\"{HTTPPORT}\" /><br />\n"
    "    <br />\n"
    "    <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "   </form>\n"
    "  </fieldset>\n"
    "  <fieldset>\n"
    "   <legend>{TR_IMDB_SETTINGS}</legend>\n"
    "   {TR_DOWNLOAD_IMDB_EXPLAIN}<br />\n"
    "   <a href=\"http://www.imdb.com/interfaces\">www.imdb.com/interfaces</a><br />\n"
    "   <br />\n"
    "{IMDB_ACTION}"
    "  </fieldset>\n"
    "  <fieldset>\n"
    "   <legend>{TR_DLNA_SETTINGS}</legend>\n"
    "   <form name=\"dlnasettings\" action=\"settings.html\" method=\"get\">\n"
    "    <input type=\"hidden\" name=\"dlnasettings\" value=\"dlnasettings\" />\n"
    "    <table>\n"
    "     <tr>\n"
    "      <td></td>\n"
    "      <td>\n"
    "       {TR_MEDIA_TRANSCODE_SETTINGS}<br />\n"
    "       {TR_MEDIA_TRANSCODE_SETTINGS_EXPLAIN}\n"
    "      </td>\n"
    "      <td>\n"
    "       {TR_MUSIC_TRANSCODE_SETTINGS}<br />\n"
    "       {TR_MUSIC_TRANSCODE_SETTINGS_EXPLAIN}"
    "      </td>\n"
    "      <td></td>\n"
    "     </tr>\n"
    "{CLIENT_ROWS}"
    "     <tr><td colspan=\"4\">\n"
    "     <input type=\"submit\" name=\"save\" value=\"{TR_SAVE}\" />\n"
    "     </td></tr>\n"
    "    </table>\n"
    "   </form>\n"
    "  </fieldset>\n"
    " </div>\n";

const char * const Backend::htmlConfigDlnaDefaultRow =
    "   <tr>\n"
    "    <td>\n"
    "     <p>{NAME}</p>"
    "    </td>\n"
    "    <td>\n"
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
    "    <td>\n"
    "     <select name=\"musicchannels\">\n"
    "{MUSICCHANNELS}"
    "     </select>\n"
    "    </td>\n"
    "    <td>\n"
    "     <input type=\"submit\" name=\"defaults\" value=\"{TR_DEFAULTS}\" />\n"
    "    </td>\n"
    "   </tr>\n";

const char * const Backend::htmlConfigDlnaClientRow =
    "   <tr>\n"
    "    <td>\n"
    "     <p title=\"{USERAGENT}\">{NAME}</p>\n"
    "     {LAST_SEEN}"
    "    </td>\n"
    "    <td>\n"
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
    "    <td>\n"
    "     <select name=\"musicchannels-{NAME}\">\n"
    "{MUSICCHANNELS}"
    "     </select>\n"
    "    </td>\n"
    "    <td>\n"
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


QByteArray Backend::parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const
{
  HtmlParser localParser(htmlParser);
  localParser.setField("TR_SEARCH", tr("Search"));
  localParser.setField("SEARCH_QUERY", QByteArray(""));
  localParser.setField("PATH", QByteArray(""));

  const QString path = url.path();
  const int lastSlash = path.lastIndexOf('/');
  if (lastSlash > 0)
  {
    QString fullPath = "/";
    foreach (const QString &dir, path.left(lastSlash).split('/', QString::SkipEmptyParts))
    {
      fullPath += dir + "/";
      localParser.setField("ITEM_LINK", fullPath);
      localParser.setField("ITEM_NAME", dir);
      localParser.appendField("PATH", localParser.parse(htmlLocationItem));
    }
  }

  if (url.hasQueryItem("q"))
  {
    const QString queryValue = url.queryItemValue("q");
    const QString queryString = QByteArray::fromPercentEncoding(queryValue.toAscii().replace('+', ' '));
    localParser.setField("TR_SEARCH", tr("Search"));
    localParser.setField("SEARCH_QUERY", queryString);

    localParser.setField("ITEM_LINK", "/?q=" + queryValue);
    localParser.setField("ITEM_NAME", tr("Search") + ": " + queryString);
    localParser.appendField("PATH", localParser.parse(htmlLocationItem));
  }

  localParser.setField("HEAD", head);
  localParser.setField("CONTENT", content);
  return localParser.parse(htmlIndex);
}

QByteArray Backend::parseHtmlLogErrors(void) const
{
  HtmlParser htmlParser(this->htmlParser);

  // Submit error log
  const QSet<QString> dismissedFiles =
      QSet<QString>::fromList(GlobalSettings().value("DismissedErrors").toStringList());

  QStringList errorLogFiles;
  foreach (const QString &file, sApp->errorLogFiles())
  if (!dismissedFiles.contains(file))
    errorLogFiles += file;

  if (!errorLogFiles.isEmpty())
  {
    htmlParser.setField("TR_ERRORS",tr("Program errors"));
    htmlParser.setField("TR_DISMISS",tr("Dismiss all errors"));

    htmlParser.setField("ITEM_ICONURL", QByteArray("/img/journal.png?scale=32"));

    htmlParser.setField("ERROR_LOG_FILES", QByteArray(""));
    QString lastDate;
    for (int i=0; i<errorLogFiles.count(); i++)
    {
      const QFileInfo info(errorLogFiles[i]);

      htmlParser.setField("ITEM_TITLE", info.created().toString("yyyy-MM-dd hh:mm"));
      htmlParser.setField("ITEM_URL", "/" + info.fileName());
      htmlParser.appendField("ITEMS", htmlParser.parse(htmlMenuItem));
    }

    htmlParser.setField("ITEM_TITLE", tr("Dismiss all errors"));
    htmlParser.setField("ITEM_URL", QByteArray("/?dismisserrors"));
    htmlParser.appendField("ITEMS", htmlParser.parse(htmlMenuItem));

    htmlParser.setField("TEXT", tr("Program errors"));
    return htmlParser.parse(htmlMenuGroup);
  }

  return QByteArray();
}

SHttpServer::SocketOp Backend::handleHtmlRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, const QString &file)
{
  HtmlParser htmlParser(this->htmlParser);

  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  htmlParser.setField("LOG_ERRORS", parseHtmlLogErrors());

  htmlParser.setField("GROUPS", QByteArray(""));
  for (QMap<QString, QList<MenuItem> >::ConstIterator i=submenuItems.begin();
       i!=submenuItems.end();
       i++)
  if (file.isEmpty() || (file == i.key()))
  {
    htmlParser.setField("ITEMS", QByteArray(""));
    foreach (const MenuItem &item, *i)
    {
      htmlParser.setField("ITEM_TITLE", item.title);
      htmlParser.setField("ITEM_URL", item.url);
      htmlParser.setField("ITEM_ICONURL", item.iconurl);
      htmlParser.appendField("ITEMS", htmlParser.parse(htmlMainServerItem));
    }

    htmlParser.setField("ITEM_TITLE", i.key());
    htmlParser.appendField("GROUPS", htmlParser.parse(htmlMainGroupItem));
  }

  socket->write(response);
  socket->write(parseHtmlContent(QUrl(request.path()), htmlParser.parse(htmlMain), ""));
  return SHttpServer::SocketOp_Close;
}

SHttpServer::SocketOp Backend::handleHtmlSearch(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, const QString &)
{
  HtmlParser htmlParser(this->htmlParser);
  htmlParser.setField("TR_OF", tr("of"));
  htmlParser.setField("TR_RELEVANCE", tr("Relevance"));
  htmlParser.setField("TR_RESULTS", tr("Results"));

  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  const QUrl url(request.path());
  const QString queryValue = url.queryItemValue("q");
  const QString queryString = QByteArray::fromPercentEncoding(queryValue.toAscii().replace('+', ' '));
  const SearchCacheEntry entry = search(queryString);
  htmlParser.setField("SEARCHRESULTS", QByteArray(""));

  foreach (const BackendServer::SearchResult &result, entry.results)
  {
    htmlParser.setField("ITEM_TITLE", result.headline);
    htmlParser.setField("ITEM_RELEVANCE", QString::number(qBound(0, int(result.relevance * 100.0), 100)) + "%");
    htmlParser.setField("ITEM_URL", result.location);
    htmlParser.setField("ITEM_ICONURL", result.thumbLocation);

    htmlParser.appendField("SEARCHRESULTS", htmlParser.parse(htmlSearchResultsItem));
  }

  htmlParser.setField("ITEM_TITLE", tr("Search") + ": " + queryString);

  socket->write(response);
  socket->write(parseHtmlContent(url, htmlParser.parse(htmlSearchResults), ""));
  return SHttpServer::SocketOp_Close;
}

SHttpServer::SocketOp Backend::handleHtmlLogFileRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket, const QString &file)
{
  QString logFileName;
  if (file == "main.log")
  {
    logFileName = sApp->activeLogFile();
  }
  else foreach (const QString &f, sApp->allLogFiles())
  if (f.endsWith("/" + file))
  {
    logFileName = f;
    break;
  }

  SApplication::LogFile logFile(logFileName);
  if (logFile.open(SApplication::LogFile::ReadOnly))
  {
    const QUrl url(request.path());

    HtmlParser htmlParser(this->htmlParser);
    htmlParser.setField("TR_DATE", tr("Date"));
    htmlParser.setField("TR_TYPE", tr("Type"));
    htmlParser.setField("TR_MESSAGE", tr("Message"));

    htmlParser.setField("LOG_ERRORS", parseHtmlLogErrors());

    htmlParser.setField("LOG_MESSAGES", QByteArray(""));

    for (SApplication::LogFile::Message msg=logFile.readMessage();
         msg.date.isValid();
         msg=logFile.readMessage())
    {
      const bool mr = !msg.message.isEmpty();

      htmlParser.setField("ITEM_ROWS", QByteArray::number(mr ? 2 : 1));

      htmlParser.setField("ITEM_ROWS", QByteArray::number(mr ? 2 : 1));
      htmlParser.setField("ITEM_DATE", msg.date.toString("yyyy-MM-dd/hh:mm:ss"));
      htmlParser.setField("ITEM_TYPE", msg.type);
      htmlParser.setField("ITEM_PID", QByteArray::number(msg.pid));
      htmlParser.setField("ITEM_TID", QByteArray::number(msg.tid));
      htmlParser.setField("ITEM_TYPE", msg.type);
      htmlParser.setField("ITEM_HEADLINE", msg.headline);
      htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileHeadline));

      if (mr)
      {
        htmlParser.setField("ITEM_MESSAGE", msg.message.replace('\n', "<br />\n"));
        htmlParser.appendField("LOG_MESSAGES", htmlParser.parse(htmlLogFileMessage));
      }
    }

    SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
    response.setContentType("text/html;charset=utf-8");
    response.setField("Cache-Control", "no-cache");
    if (logFileName == sApp->activeLogFile())
      response.setField("Refresh", "10;URL=#bottom");

    socket->write(response);
    socket->write(parseHtmlContent(url, htmlParser.parse(htmlLogFile), ""));
    return SHttpServer::SocketOp_Close;
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

SHttpServer::SocketOp Backend::showAbout(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  HtmlParser htmlParser(this->htmlParser);

  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  htmlParser.setField("ABOUT_LXIMEDIA", sApp->about());

  socket->write(response);
  socket->write(parseHtmlContent(QUrl(request.path()), htmlParser.parse(htmlAbout), ""));
  return SHttpServer::SocketOp_Close;
}

SHttpServer::SocketOp Backend::handleHtmlConfig(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
  response.setContentType("text/html;charset=utf-8");
  response.setField("Cache-Control", "no-cache");

  GlobalSettings settings;

  // HTTP server
  const QUrl url(request.path());
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
  if (masterImdbClient && url.hasQueryItem("imdbsettings") && url.hasQueryItem("download"))
  {
    masterImdbClient->obtainIMDBFiles();
  }

  htmlParser.setField("TR_IMDB_SETTINGS", tr("IMDb settings"));
  htmlParser.setField("TR_DOWNLOAD_IMDB_EXPLAIN",
    tr("This form will allow the imdb.com data files to be downloaded to "
       "provide additional information on the media files that are available. "
       "Downloading and parsing the files will take several minutes."));

  if (masterImdbClient && masterImdbClient->isAvailable() && !masterImdbClient->needUpdate())
  {
    htmlParser.setField("IMDB_ACTION", "    <b>" + tr("IMDb files are available") + "</b>\n");
  }
  else if (masterImdbClient && (masterImdbClient->isDownloading() || !masterImdbClient->needUpdate()))
  {
    htmlParser.setField("IMDB_ACTION", "    <b>" + tr("Downloading and parsing IMBb files") + " ...</b>\n");
  }
  else if (masterImdbClient)
  {
    htmlParser.setField("TR_DOWNLOAD_IMDB", masterImdbClient->needUpdate() ? tr("Update IMDb files") : tr("Download IMDb files"));
    htmlParser.setField("IMDB_ACTION", htmlParser.parse(htmlConfigImdbDownload));
  }
  else
    htmlParser.setField("IMDB_ACTION", QByteArray(""));

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

    setContentDirectoryQueryItems();
  }
  else if (url.hasQueryItem("dlnasettings") && url.hasQueryItem("defaults"))
  {
    settings.remove("TranscodeSize");
    settings.remove("TranscodeCrop");
    settings.remove("EncodeMode");
    settings.remove("TranscodeChannels");
    settings.remove("TranscodeMusicChannels");

    setContentDirectoryQueryItems();
  }
  else if (url.hasQueryItem("dlnasettings"))
  {
    foreach (const QString &group, settings.childGroups())
    if (group.startsWith("Client_") && url.hasQueryItem("erase-" + group.mid(7)))
      settings.remove(group);

    setContentDirectoryQueryItems();
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
  const QMap<QString, QString> activeClients = masterContentDirectory.activeClients();
  for (QMap<QString, QString>::ConstIterator i=activeClients.begin(); i!=activeClients.end(); i++)
  {
    settings.beginGroup("Client_" + i.key());
    settings.setValue("UserAgent", i.value());
    settings.endGroup();
  }

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
  return SHttpServer::SocketOp_Close;
}
