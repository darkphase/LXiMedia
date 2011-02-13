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
#include <iostream>
#ifdef Q_OS_UNIX
#include <unistd.h>
#include "unixdaemon.h"
#endif

const quint8        Backend::haltExitCode = 0x33;
const QEvent::Type  Backend::exitEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  Backend::restartEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  Backend::shutdownEventType = QEvent::Type(QEvent::registerEventType());
const QUrl          Backend::submitErrorUrl("http://www.admiraal.dds.nl/submitlog.php");

Backend::Backend()
        :BackendServer::MasterServer(),
         masterHttpServer(),
         masterSsdpServer(),
         masterDlnaServer(&masterHttpServer),
         cssParser(),
         htmlParser(),
         backendPlugins(),
         backendServers()
{
  // Initialize LXiStream
  QDir logDir(GlobalSettings::applicationDataDir() + "/log");
  if (!logDir.exists())
    logDir.mkpath(logDir.absolutePath());

  SSystem::initialize(SSystem::Initialize_Default, logDir.absolutePath());

  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  qDebug() << "Using data directory" << GlobalSettings::applicationDataDir();

  // Check and backup settings file
  const QString settingsFile = GlobalSettings::settingsFile();
  if (QFile::exists(settingsFile + ".bak"))
  {
    if (!QFile::exists(settingsFile) || (QFileInfo(settingsFile).size() == 0))
    {
      qDebug() << "Using backup settings " << settingsFile + ".bak";

      // Backup exists, use it.
      QFile::remove(settingsFile);
      QFile::rename(settingsFile + ".bak", settingsFile);
    }
    else
      QFile::remove(settingsFile + ".bak");
  }

  QFile::copy(settingsFile, settingsFile + ".bak");

  // Open database
  Database::initialize();
}

Backend::~Backend()
{
  qDebug() << "LXiMediaCenter backend stopping.";

  ImdbClient::shutdown();

  masterDlnaServer.close();
  masterSsdpServer.close();
  masterHttpServer.close();

  foreach (BackendServer *server, backendServers)
    delete server;

  foreach (BackendPlugin *plugin, backendPlugins)
    delete plugin;

  QThreadPool::globalInstance()->waitForDone();

  // Remove backup settings
  const QString settingsFile = GlobalSettings::settingsFile();
  if (QFile::exists(settingsFile))
    QFile::remove(settingsFile + ".bak");

  qDebug() << "LXiMediaCenter backend stopped.";

  // Shutdown LXiStream
  SSystem::shutdown();

  // Close database
  Database::shutdown();
}

void Backend::start(void)
{
  GlobalSettings settings;

  masterHttpServer.initialize(settings.defaultBackendInterfaces(),
                              settings.value("HttpPort", settings.defaultBackendHttpPort()).toInt());

  // Write a port file so the server can be found
  QFile portFile(QDir(GlobalSettings::applicationDataDir()).absoluteFilePath("server.port"));
  if (portFile.open(QFile::WriteOnly))
    portFile.write(QByteArray::number(masterHttpServer.serverPort(QHostAddress::LocalHost)));

  // Setup HTTP server
  masterHttpServer.registerCallback("/", this);

  // Setup default palette.
  HtmlParser::Palette palette;
//  palette.window      = HtmlParser::Palette::Rgb(240, 240, 255);
//  palette.windowText  = HtmlParser::Palette::Rgb(  0,   0,   0);
//  palette.base        = HtmlParser::Palette::Rgb(160, 160, 192);
//  palette.altBase     = HtmlParser::Palette::Rgb(128, 128, 160);
//  palette.text        = HtmlParser::Palette::Rgb( 32,  32,  48);
//  palette.button      = HtmlParser::Palette::Rgb( 64,  64,  80);
//  palette.buttonText  = HtmlParser::Palette::Rgb(255, 255, 255);
  palette.window      = HtmlParser::Palette::Rgb(160, 160, 192);
  palette.windowText  = HtmlParser::Palette::Rgb( 32,  32,  48);
  palette.base        = HtmlParser::Palette::Rgb(240, 240, 255);
  palette.altBase     = HtmlParser::Palette::Rgb(224, 224, 240);
  palette.text        = HtmlParser::Palette::Rgb(  0,   0,   0);
  palette.button      = HtmlParser::Palette::Rgb( 64,  64,  80);
  palette.buttonText  = HtmlParser::Palette::Rgb(255, 255, 255);

  HtmlParser::setPalette(palette);
  cssParser.clear();
  htmlParser.clear();

  htmlParser.setField("TR_MEDIA",               tr("Media"));
  htmlParser.setField("TR_CENTER",              tr("Center"));
  htmlParser.setField("TR_LOGO",                tr("<span class=\"logoa\">LX</span><span class=\"logob\">i</span><span class=\"logoc\">Media</span><span class=\"logoa\">Center</span>"));

  SDebug::WriteLocker wl(&lock, __FILE__, __LINE__);

  // A minimal general menu
  QList<QPair<QString, QString> > generalMenu;
  generalMenu += QPair<QString, QString>(tr("Main"), "/");
  generalMenu += QPair<QString, QString>(tr("Log"),  "/main.log#bottom");
  generalMenu += QPair<QString, QString>(tr("About"),  "/about.html");
  submenuItems[tr("General")] = generalMenu;

  wl.unlock();

  // This call may take a while if the database needs to be updated ...
  ImdbClient::initialize(&threadPool);

  wl.relock(__FILE__, __LINE__);

  // The full general menu
  generalMenu.clear();
  generalMenu += QPair<QString, QString>(tr("Main"), "/");
  generalMenu += QPair<QString, QString>(tr("Log"),  "/main.log#bottom");
  generalMenu += QPair<QString, QString>(tr("Settings"),  "/settings.html");
  generalMenu += QPair<QString, QString>(tr("About"),  "/about.html");
  submenuItems[tr("General")] = generalMenu;

  // Load plugins
  backendPlugins = BackendPlugin::loadPlugins();
  foreach (BackendPlugin *backendPlugin, backendPlugins)
  if (backendPlugin)
  {
    wl.unlock();

    qDebug() << "Loading backend:" << backendPlugin->pluginName()
             << "by" << backendPlugin->authorName()
             << "version" << backendPlugin->pluginVersion();

    const QList<BackendServer *> servers = backendPlugin->createServers(this);

    wl.relock(__FILE__, __LINE__);

    if (!servers.isEmpty())
    {
      QList<QPair<QString, QString> > menu;
      foreach (BackendServer *server, servers)
      {
        backendServers += server;

        menu += QPair<QString, QString>(server->name(), server->httpPath());
      }

      submenuItems[backendPlugin->pluginName()] = menu;
    }
  }

  wl.unlock();

  // Setup SSDP server
  masterSsdpServer.initialize(settings.defaultBackendInterfaces());
  masterSsdpServer.publish(GlobalSettings::productAbbr() + QString(":server"), &masterHttpServer, "/");

  // Setup DLNA server
  masterDlnaServer.initialize(&masterHttpServer, &masterSsdpServer);

//  DlnaServer::File shutdown(dlnaServiceDir.server());
//  shutdown.url = "/?shutdown=shutdown";
//  shutdown.mimeType = "video/mpeg";
//  dlnaServiceDir.addFile(tr("Shutdown server"), shutdown);
//  dlnaServiceDir.sortOrder = 0xFFFFFFFF; // Last item
//
//  masterDlnaServer.addDir("/" + tr("Service"), &dlnaServiceDir);

  qDebug() << "Finished initialization.";
}

Backend::SearchCacheEntry Backend::search(const QString &query) const
{
  GlobalSettings settings;

  const QString queryText = query.simplified();
  const QStringList queryRaw = SStringParser::toRawName(queryText.split(' '));

  SDebug::ReadLocker rl(&lock, __FILE__, __LINE__);

  // Look for a cache entry
  QMap<QString, SearchCacheEntry>::ConstIterator i = searchCache.find(queryText);
  if (i != searchCache.end())
  if (i->update.elapsed() < 60000)
    return *i;

  rl.unlock();

  QTime timer;
  timer.start();

  // Start parallel searches
  class Query : public QRunnable
  {
  public:
    inline Query(const BackendServer *backendServer, const QStringList &query)
      : backendServer(backendServer),
        query(query)
    {
      setAutoDelete(false);
    }

    virtual void run(void)
    {
      result = backendServer->search(query);
      finished.release(1);
    }

  public:
    const BackendServer * const backendServer;
    const QStringList query;

    BackendServer::SearchResultList result;
    QSemaphore finished;
  };

  QList<QRunnable *> tasks;
  foreach (const BackendServer *backendServer, backendServers)
  {
    QRunnable * const q = new Query(backendServer, queryRaw);
    threadPool.start(q, 1); // High priority since these need to be responsive.
    tasks += q;
  }

  // Gather all results, this will block until the tasks are ready.
  SearchCacheEntry entry;
  foreach (QRunnable *r, tasks)
  {
    Query * const q = static_cast<Query *>(r);
    q->finished.acquire(1);

    foreach (BackendServer::SearchResult result, q->result)
    {
      const QByteArray baseUrl = q->backendServer->httpPath().toUtf8();

      if (!result.location.isEmpty())
        result.location = baseUrl + result.location;

      if (!result.thumbLocation.isEmpty())
        result.thumbLocation = baseUrl + result.thumbLocation;

      entry.results.insert(1.0 - result.relevance, result);
    }

    delete q;
  }

  entry.duration = timer.elapsed();

  SDebug::WriteLocker wl(&lock, __FILE__, __LINE__);

  while (searchCache.count() > 64)
    searchCache.erase(searchCache.begin());

  entry.update.start();
  searchCache.insert(queryText, entry);

  // Remove old searches
  for (QMap<QString, SearchCacheEntry>::Iterator i=searchCache.begin(); i!=searchCache.end(); )
  if (i->update.elapsed() >= 60000)
    i = searchCache.erase(i);
  else
    i++;

  return entry;
}

void Backend::customEvent(QEvent *e)
{
  if (e->type() == exitEventType)
  {
    qApp->exit(0);
  }
  else if (e->type() == restartEventType)
  {
    // This exitcode instructs main.cpp to restart.
    qApp->exit(-1);
  }
  else if (e->type() == shutdownEventType)
  {
    // This exitcode instructs the UnixDaemon to initiate a system halt.
    qApp->exit(haltExitCode);
  }
}

HttpServer::SocketOp Backend::handleHttpRequest(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString path = url.path();
  const QString file = path.mid(path.lastIndexOf('/') + 1);

  if (path.left(path.lastIndexOf('/') + 1) == "/")
  {
    if (url.hasQueryItem("exit"))
    {
      QCoreApplication::postEvent(this, new QEvent(exitEventType));

      socket->write(HttpServer::ResponseHeader(HttpServer::Status_NoContent));
      return HttpServer::SocketOp_Close;
    }
    else if (url.hasQueryItem("restart"))
    {
      QCoreApplication::postEvent(this, new QEvent(restartEventType));

      socket->write(HttpServer::ResponseHeader(HttpServer::Status_NoContent));
      return HttpServer::SocketOp_Close;
    }
    else if (url.hasQueryItem("shutdown"))
    {
      QCoreApplication::postEvent(this, new QEvent(shutdownEventType));

      socket->write(HttpServer::ResponseHeader(HttpServer::Status_NoContent));
      return HttpServer::SocketOp_Close;
    }
    else if (url.hasQueryItem("dismisserrors"))
    {
      GlobalSettings().setValue("DismissedErrors", SDebug::LogFile::errorLogFiles());

      return handleHtmlRequest(url, file, socket);
    }
    else if (file == "traystatus.xml")
    {
      QDomDocument doc("");
      QDomElement root = doc.createElement("traystatus");
      doc.appendChild(root);

      // Hostinfo
      QDomElement hostInfo = doc.createElement("hostinfo");
      root.appendChild(hostInfo);
      hostInfo.setAttribute("hostname", QHostInfo::localHostName());

      // DLNA clients
      GlobalSettings settings;
      settings.beginGroup("DLNA");

      foreach (const QString &group, settings.childGroups())
      if (group.startsWith("Client_"))
      {
        QDomElement dlnaClient = doc.createElement("dlnaclient");
        root.appendChild(dlnaClient);
        dlnaClient.setAttribute("name", group.mid(7));
        dlnaClient.setAttribute("useragent", settings.value("UserAgent", tr("Unknown")).toString());
        dlnaClient.setAttribute("lastseen", settings.value("LastSeen").toDateTime().toString(Qt::ISODate));
      }

      settings.endGroup();

      // Active log file
      QDomElement activeLogFile = doc.createElement("activelogfile");
      root.appendChild(activeLogFile);
      activeLogFile.setAttribute("name", QFileInfo(SDebug::LogFile::activeLogFile()).fileName());

      // Error logs
      const QSet<QString> dismissedFiles =
          QSet<QString>::fromList(settings.value("DismissedErrors").toStringList());

      QStringList errorLogFiles;
      foreach (const QString &file, SDebug::LogFile::errorLogFiles())
      if (!dismissedFiles.contains(file))
        errorLogFiles += file;

      foreach (const QString &file, errorLogFiles)
      {
        QDomElement errorLogFile = doc.createElement("errorlogfile");
        root.appendChild(errorLogFile);
        errorLogFile.setAttribute("name", QFileInfo(file).fileName());
      }

      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentType("text/xml;charset=utf-8");
      response.setField("Cache-Control", "no-cache");
      socket->write(response);
      socket->write(doc.toByteArray());
      return HttpServer::SocketOp_Close;
    }
    else if (file.endsWith(".css"))
    {
      return handleCssRequest(url, file, socket);
    }
    else if (url.hasQueryItem("q"))
    {
      return handleHtmlSearch(url, file, socket);
    }
    else if (request.path() == "/")
    {
      return handleHtmlRequest(url, file, socket);
    }
    else if (file.endsWith(".log"))
    {
      static const char * const logHead = " <link rel=\"stylesheet\" href=\"/log.css\" type=\"text/css\" media=\"screen, handheld, projection\" />\n";

      QString logFileName;
      if (file == "main.log")
      {
        logFileName = SDebug::LogFile::activeLogFile();
      }
      else foreach (const QString &f, SDebug::LogFile::allLogFiles())
      if (f.endsWith("/" + file))
      {
        logFileName = f;
        break;
      }

      SDebug::LogFile logFile(logFileName);
      if (logFile.open(SDebug::LogFile::ReadOnly))
      {
        HtmlParser htmlParser(this->htmlParser);
        htmlParser.setField("TR_DATE", tr("Date"));
        htmlParser.setField("TR_TYPE", tr("Type"));
        htmlParser.setField("TR_MESSAGE", tr("Message"));

        htmlParser.setField("LOG_MESSAGES", QByteArray(""));

        for (SDebug::LogFile::Message msg=logFile.readMessage();
             msg.date.isValid();
             msg=logFile.readMessage())
        {
          const bool mr = !msg.message.isEmpty();

          htmlParser.setField("ITEM_ROWS", QByteArray::number(mr ? 2 : 1));

          if (msg.type == "INF")
            htmlParser.setField("ITEM_CLASS", QByteArray("loginf"));
          else if (msg.type == "WRN")
            htmlParser.setField("ITEM_CLASS", QByteArray("logwrn"));
          else if ((msg.type == "CRT") || (msg.type == "EXC"))
            htmlParser.setField("ITEM_CLASS", QByteArray("logerr"));
          else
            htmlParser.setField("ITEM_CLASS", QByteArray("logdbg"));

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

        HttpServer::ResponseHeader response(HttpServer::Status_Ok);
        response.setContentType("text/html;charset=utf-8");
        response.setField("Cache-Control", "no-cache");
        if (logFileName == SDebug::LogFile::activeLogFile())
          response.setField("Refresh", "10;URL=#bottom");

        socket->write(response);
        socket->write(parseHtmlContent(url, htmlParser.parse(htmlLogFile), logHead));
        return HttpServer::SocketOp_Close;
      }
    }
    else if (file == "settings.html")
    {
      return handleHtmlConfig(url, socket);
    }
    else if (file == "about.html")
    {
      return showAbout(url, socket);
    }
  }
  else
  {
    QString file;
    if      (path == "/favicon.ico")                file = ":/lximediacenter/appicon.ico";
    else if (path == "/appicon.png")                file = ":/lximediacenter/appicon.png";
    else if (path == "/logo.png")                   file = ":/lximediacenter/logo.png";

    else if (path == "/img/null.png")               file = ":/backend/null.png";
    else if (path == "/img/checknone.png")          file = ":/backend/checknone.png";
    else if (path == "/img/checkfull.png")          file = ":/backend/checkfull.png";
    else if (path == "/img/checksome.png")          file = ":/backend/checksome.png";
    else if (path == "/img/checknonedisabled.png")  file = ":/backend/checknonedisabled.png";
    else if (path == "/img/checkfulldisabled.png")  file = ":/backend/checkfulldisabled.png";
    else if (path == "/img/checksomedisabled.png")  file = ":/backend/checksomedisabled.png";
    else if (path == "/img/treeopen.png")           file = ":/backend/treeopen.png";
    else if (path == "/img/treeclose.png")          file = ":/backend/treeclose.png";
    else if (path == "/img/starenabled.png")        file = ":/backend/starenabled.png";
    else if (path == "/img/stardisabled.png")       file = ":/backend/stardisabled.png";
    else if (path == "/img/directory.png")          file = ":/backend/directory.png";
    else if (path == "/img/audio-file.png")         file = ":/backend/audio-file.png";
    else if (path == "/img/video-file.png")         file = ":/backend/video-file.png";
    else if (path == "/img/image-file.png")         file = ":/backend/image-file.png";
    else if (path == "/img/restart.png")            file = ":/backend/restart.png";
    else if (path == "/img/shutdown.png")           file = ":/backend/shutdown.png";

    else if (path == "/swf/flowplayer.swf")         file = ":/flowplayer/flowplayer-3.2.5.swf";
    else if (path == "/swf/flowplayer.controls.swf")file = ":/flowplayer/flowplayer.controls-3.2.3.swf";
    else if (path == "/swf/flowplayer.js")          file = ":/flowplayer/flowplayer-3.2.4.min.js";

    QFile f(file);
    if (f.open(QFile::ReadOnly))
    {
      HttpServer::ResponseHeader response(HttpServer::Status_Ok);
      response.setContentLength(f.size());
      response.setContentType(HttpServer::toMimeType(file));
      socket->write(response);
      socket->write(f.readAll());
      return HttpServer::SocketOp_Close;
    }
  }

  socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
  return HttpServer::SocketOp_Close;
}

QByteArray Backend::parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const
{
  HtmlParser localParser(htmlParser);
  localParser.setField("HEAD", head);

  // Build menus
  SDebug::ReadLocker rl(&lock, __FILE__, __LINE__);

  QString pluginPath = url.path(), pagePath;
  const int s2 = pluginPath.indexOf('/', 1);
  if (s2 > 1)
  {
    pagePath = pluginPath.mid(s2);
    pluginPath = pluginPath.left(s2 + 1);

    const int s3 = pagePath.indexOf('/', 1);
    if (s3 > 1)
      pagePath = pagePath.left(s3 + 1);
  }
  else
  {
    pagePath = pluginPath;
    pluginPath = "/";
  }

  localParser.setField("HEAD_MENUITEMS", QByteArray(""));
  localParser.setField("HEAD_SUBMENUITEMS", QByteArray(""));
  for (QMap<QString, QList<QPair<QString, QString> > >::ConstIterator i=submenuItems.begin();
       i!=submenuItems.end();
       i++)
  {
    bool selected = false;
    for (QList<QPair<QString, QString> >::ConstIterator j=i->begin(); j!=i->end(); j++)
    {
      QString p = j->second;
      const int s2 = p.indexOf('/', 1);
      if (s2 > 1)
        p = p.left(s2 + 1);
      else
        p = "/";

      if (p == pluginPath)
      {
        selected = true;

        localParser.setField("TEXT", j->first);
        localParser.setField("LINK", j->second);

        const char * const html = j->second.split('#').first().endsWith(pagePath) ? htmlSubMenuItemSel : htmlSubMenuItem;
        localParser.appendField("HEAD_SUBMENUITEMS", localParser.parse(html));
      }
    }

    localParser.setField("TEXT", i.key());
    localParser.setField("LINK", i.value().first().second);
    localParser.appendField("HEAD_MENUITEMS", localParser.parse(selected ? htmlMenuItemSel : htmlMenuItem));
  }

  rl.unlock();

  localParser.setField("CONTENT", content);

  return localParser.parse(htmlIndex);
}

HttpServer * Backend::httpServer(void)
{
  return &masterHttpServer;
}

SsdpServer * Backend::ssdpServer(void)
{
  return &masterSsdpServer;
}

DlnaServer * Backend::dlnaServer(void)
{
  return &masterDlnaServer;
}

QThreadPool * Backend::ioThreadPool(void)
{
  return &threadPool;
}
