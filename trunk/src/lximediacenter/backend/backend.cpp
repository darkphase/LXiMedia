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
#ifdef DEBUG_USE_LOCAL_SANDBOX
# include "sandbox.h"
#endif
#include <iostream>


const QEvent::Type  Backend::exitEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  Backend::restartEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  Backend::shutdownEventType = QEvent::Type(QEvent::registerEventType());
const QUrl          Backend::submitErrorUrl("http://www.admiraal.dds.nl/submitlog.php");

QString Backend::createLogDir(void)
{
  QDir logDir(GlobalSettings::applicationDataDir() + "/log");
  if (!logDir.exists())
    logDir.mkpath(logDir.absolutePath());

  return logDir.absolutePath();
}

Backend::Backend()
  : BackendServer::MasterServer(),
    masterHttpServer(SUPnPBase::protocol(), GlobalSettings::serverUuid()),
    masterSsdpServer(&masterHttpServer),
    masterMediaServer("/upnp/"),
    masterConnectionManager("/upnp/"),
    masterContentDirectory("/upnp/"),
    masterMediaReceiverRegistrar("/upnp/"),
    masterImdbClient(NULL),
    sandboxApplication("\"" + qApp->applicationFilePath() + "\" --sandbox"),
    cssParser(),
    htmlParser(),
    backendServers(),
    initSandbox(NULL)
{
  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));

  qDebug() << "Using data directory" << GlobalSettings::applicationDataDir();

  // Check and backup settings file
  const QString settingsFile = GlobalSettings::settingsFile();
  const QString bakSettingsFile = settingsFile + ".bak";
  if (QFile::exists(bakSettingsFile))
  {
    if (!QFile::exists(settingsFile) || (QFileInfo(settingsFile).size() == 0))
    {
      qDebug() << "Using backup settings " << bakSettingsFile;

      // Backup exists, use it.
      QFile::remove(settingsFile);
      QFile::rename(bakSettingsFile, settingsFile);
    }
    else
      QFile::remove(bakSettingsFile);
  }

  QFile::copy(settingsFile, bakSettingsFile);

  // Open device configuration
  MediaServer::mediaProfiles().openDeviceConfig(":/devices.ini");

  // Open database
  Database::initialize();
}

Backend::~Backend()
{
  qDebug() << "LXiMediaCenter backend stopping.";

  masterContentDirectory.close();
  masterConnectionManager.close();
  masterMediaReceiverRegistrar.close();
  masterMediaServer.close();
  masterSsdpServer.close();
  masterHttpServer.close();

  foreach (BackendServer *server, backendServers)
  {
    server->close();
    delete server;
  }

  QThreadPool::globalInstance()->waitForDone();

  delete masterImdbClient;
  masterImdbClient = NULL;

  // Remove backup settings
  const QString settingsFile = GlobalSettings::settingsFile();
  if (QFile::exists(settingsFile))
    QFile::remove(settingsFile + ".bak");

  qDebug() << "LXiMediaCenter backend stopped.";

  // Close database
  Database::shutdown();
}

void Backend::start(void)
{
  GlobalSettings settings;

  masterHttpServer.initialize(
      settings.defaultBackendInterfaces(),
      settings.value("HttpPort", settings.defaultBackendHttpPort()).toInt());

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

  // This call may take a while if the database needs to be updated ...
  masterImdbClient = new ImdbClient(this);

  // Build the menu
  QList<MenuItem> generalMenu;
  generalMenu += MenuItem(tr("Main"),      "/",                 "/lximedia.png");
  generalMenu += MenuItem(tr("Log"),       "/main.log#bottom",  "/img/journal.png");
#ifndef QT_NO_DEBUG
  generalMenu += MenuItem(tr("Exit"),      "/?exit",            "/img/control.png");
  generalMenu += MenuItem(tr("Restart"),   "/?restart",         "/img/control.png");
#endif
  generalMenu += MenuItem(tr("Settings"),  "/settings.html",    "/img/control.png");
  generalMenu += MenuItem(tr("About"),     "/about.html",       "/img/glossary.png");
  submenuItems[tr("General")] = generalMenu;

  backendServers = BackendServer::create(this);
  foreach (BackendServer *server, backendServers)
  {
    server->initialize(this);

    submenuItems[server->pluginName()] +=
        MenuItem(server->serverName(), server->serverPath(), server->serverIconPath());
  }

  htmlParser.setField("MAIN_MENUGROUPS", QByteArray(""));
  for (QMap<QString, QList<MenuItem> >::ConstIterator i=submenuItems.begin();
       i!=submenuItems.end();
       i++)
  {
    HtmlParser localParser(htmlParser);
    localParser.setField("ITEMS", QByteArray(""));
    foreach (const MenuItem &item, *i)
    {
      localParser.setField("ITEM_TITLE", item.title);
      localParser.setField("ITEM_URL", QUrl(item.url).toEncoded());
      localParser.setField("ITEM_ICONURL", QUrl(item.iconurl + "?scale=32").toEncoded());
      localParser.appendField("ITEMS", localParser.parse(htmlMenuItem));
    }

    localParser.setField("TEXT", i.key());
    htmlParser.appendField("MAIN_MENUGROUPS", localParser.parse(htmlMenuGroup));
  }

  // Setup SSDP server
  masterSsdpServer.initialize(settings.defaultBackendInterfaces());
  masterSsdpServer.publish(qApp->applicationName() + QString(":server"), "/", 1);

  // Setup DLNA server
  masterMediaServer.initialize(&masterHttpServer, &masterSsdpServer);
  masterConnectionManager.initialize(&masterHttpServer, &masterMediaServer);
  masterContentDirectory.initialize(&masterHttpServer, &masterMediaServer);
  masterMediaReceiverRegistrar.initialize(&masterHttpServer, &masterMediaServer);

  masterMediaServer.setDeviceName(settings.value("DeviceName", settings.defaultDeviceName()).toString());

  const QImage icon(":/lximedia.png");
  if (!icon.isNull())
    masterMediaServer.addIcon("/lximedia.png", icon.width(), icon.height(), icon.depth());

  // Request all supported formats
  initSandbox = createSandbox(SSandboxClient::Priority_Low);
  connect(initSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(start(SHttpEngine::ResponseMessage)));

  SSandboxClient::RequestMessage request(initSandbox);
  request.setRequest("GET", "/?formats");
  initSandbox->sendRequest(request);
}

void Backend::start(const SHttpEngine::ResponseMessage &formats)
{
  disconnect(initSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), this, SLOT(start(SHttpEngine::ResponseMessage)));
  connect(initSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(addModules(SHttpEngine::ResponseMessage)));

  SSandboxClient::RequestMessage request(initSandbox);
  request.setRequest("GET", "/?modules");
  initSandbox->sendRequest(request);

  // Decode the message
  QStringList outAudioCodecs;
  QStringList outVideoCodecs;
  QStringList outFormats;

  QDomDocument doc("");
  if (doc.setContent(formats.content()))
  {
    struct T
    {
      static QStringList readElement(QDomDocument &doc, const QString &name, const QString &type)
      {
        QStringList result;

        QDomElement codecsElm = doc.documentElement().firstChildElement(name);
        for (QDomElement codecElm = codecsElm.firstChildElement(type);
             !codecElm.isNull();
             codecElm = codecElm.nextSiblingElement(type))
        {
          result += codecElm.text();
        }

        return result;
      }
    };

    outAudioCodecs = T::readElement(doc, "audiocodecs", "codec");
    outVideoCodecs = T::readElement(doc, "videocodecs", "codec");
    outFormats = T::readElement(doc, "formats", "format");
  }

  // Figure out the supported DLNA audio protocols.
  MediaServer::mediaProfiles().setCodecs(
      QSet<QString>::fromList(outAudioCodecs),
      QSet<QString>::fromList(outVideoCodecs),
      QSet<QString>::fromList(QStringList() << "JPEG" << "PNG"),
      QSet<QString>::fromList(outFormats));

  masterConnectionManager.setSourceProtocols(MediaServer::mediaProfiles().listProtocols(QString::null));
  masterConnectionManager.setSinkProtocols(SUPnPBase::ProtocolList());

  if (outFormats.contains("ogg") &&
      outVideoCodecs.contains("THEORA") && outAudioCodecs.contains("VORBIS"))
  {
    MediaServer::enableHtml5();
  }

  qDebug() << "Finished initialization.";
}

void Backend::reset(void)
{
  GlobalSettings settings;

  const quint16 newPort = settings.value("HttpPort", settings.defaultBackendHttpPort()).toInt();
  if (newPort != masterHttpServer.defaultPort())
    masterHttpServer.reset(settings.defaultBackendInterfaces(), newPort);

  masterSsdpServer.reset();
  masterMediaServer.reset();
  masterConnectionManager.reset();
  masterContentDirectory.reset();
  masterMediaReceiverRegistrar.reset();
}

void Backend::addModules(const SHttpEngine::ResponseMessage &modules)
{
  recycleSandbox(initSandbox);
  initSandbox = NULL;

  QDomDocument doc("");
  if (doc.setContent(modules.content()))
  {
    struct VirtualModule : public SModule
    {
      virtual bool registerClasses(void) { return true; }
      virtual void unload(void) { }
      virtual QByteArray about(void) { return aboutText; }
      virtual QByteArray licenses(void) { return licensesText; }

      QByteArray aboutText;
      QByteArray licensesText;
    };

    for (QDomElement moduleElm = doc.documentElement().firstChildElement("module");
         !moduleElm.isNull();
         moduleElm = moduleElm.nextSiblingElement("module"))
    {
      VirtualModule * const module = new VirtualModule();
      module->aboutText = moduleElm.firstChildElement("about").text().toUtf8();
      module->licensesText = moduleElm.firstChildElement("licenses").text().toUtf8();
      sApp->loadModule(module);
    }
  }
}

Backend::SearchCacheEntry Backend::search(const QString &query) const
{
  GlobalSettings settings;

  const QString queryText = query.simplified();
  const QStringList queryRaw = SStringParser::toRawName(queryText.split(' '));

  // Look for a cache entry
  QMap<QString, SearchCacheEntry>::ConstIterator i = searchCache.find(queryText);
  if (i != searchCache.end())
  if (i->update.elapsed() < 60000)
    return *i;

  QTime timer;
  timer.start();
  SearchCacheEntry entry;

  foreach (const BackendServer *backendServer, backendServers)
  {
    const QString baseUrl = backendServer->serverPath();

    foreach (BackendServer::SearchResult result, backendServer->search(queryRaw))
    {
      if (!result.location.isEmpty())
        result.location = baseUrl + result.location;

      if (!result.thumbLocation.isEmpty())
        result.thumbLocation = baseUrl + result.thumbLocation;

      entry.results.insert(1.0 - result.relevance, result);
    }
  }

  entry.duration = timer.elapsed();

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
    qApp->exit(0);
  }
}

SHttpServer::ResponseMessage Backend::httpRequest(const SHttpServer::RequestMessage &request, QIODevice *)
{
  if (request.isGet())
  {
    const MediaServer::File file(request);
    const QString path = file.url().path();

    if (path.left(path.lastIndexOf('/') + 1) == "/")
    {
      if (file.url().hasQueryItem("exit"))
      {
        QCoreApplication::postEvent(this, new QEvent(exitEventType));

        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NoContent);
      }
      else if (file.url().hasQueryItem("restart"))
      {
        QCoreApplication::postEvent(this, new QEvent(restartEventType));

        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NoContent);
      }
      else if (file.url().hasQueryItem("shutdown"))
      {
        QCoreApplication::postEvent(this, new QEvent(shutdownEventType));

        return SHttpServer::ResponseMessage(request, SHttpServer::Status_NoContent);
      }
      else if (file.url().hasQueryItem("dismisserrors"))
      {
        GlobalSettings().setValue("DismissedErrors", sApp->errorLogFiles());

        return handleHtmlRequest(request, file);
      }
      else if (file.fileName() == "traystatus.xml")
      {
        GlobalSettings settings;

        QDomDocument doc("");
        QDomElement root = doc.createElement("traystatus");
        doc.appendChild(root);

        // Hostinfo
        QDomElement hostInfo = doc.createElement("hostinfo");
        root.appendChild(hostInfo);
        hostInfo.setAttribute("hostname", settings.value("DeviceName", settings.defaultDeviceName()).toString());

        // Error logs
        const QSet<QString> dismissedFiles =
            QSet<QString>::fromList(settings.value("DismissedErrors").toStringList());

        QStringList errorLogFiles;
        foreach (const QString &file, sApp->errorLogFiles())
        if (!dismissedFiles.contains(file))
          errorLogFiles += file;

        foreach (const QString &file, errorLogFiles)
        {
          QDomElement errorLogFile = doc.createElement("errorlogfile");
          root.appendChild(errorLogFile);
          errorLogFile.setAttribute("name", QFileInfo(file).fileName());
        }

        SHttpServer::ResponseMessage response(
            request, SHttpServer::Status_Ok,
            doc.toByteArray(-1), SHttpEngine::mimeTextXml);

        response.setField("Cache-Control", "no-cache");
        return response;
      }
      else if (file.url().hasQueryItem("q"))
      {
        return handleHtmlSearch(request, file);
      }
      else if (request.path() == "/")
      {
        return handleHtmlRequest(request, file);
      }
      else if (file.fileName().endsWith(".log", Qt::CaseInsensitive))
      {
        return handleHtmlLogFileRequest(request, file);
      }
      else if (file.fileName() == "settings.html")
      {
        return handleHtmlConfig(request);
      }
      else if (file.fileName() == "about.html")
      {
        return showAbout(request);
      }
    }

    // Check if the root directory of a plugin was requested.
    QString pluginName = path.mid(1, path.length() - 2);
    foreach (BackendServer *server, backendServers)
    if (pluginName == server->pluginName())
      return handleHtmlRequest(request, pluginName);

    QString sendFile;
    if      (path == "/favicon.ico")                sendFile = ":/lximedia.ico";
    else if (path == "/lximedia.png")               sendFile = ":/lximedia.png";

    else if (path == "/css/main.css")               sendFile = ":/css/main.css";
    else if (path == "/css/phone.css")              sendFile = ":/css/phone.css";

    else if (path == "/js/dynamiclist.js")          sendFile = ":/js/dynamiclist.js";

    else if (path == "/swf/flowplayer.swf")         sendFile = ":/flowplayer/flowplayer-3.2.5.swf";
    else if (path == "/swf/flowplayer.controls.swf")sendFile = ":/flowplayer/flowplayer.controls-3.2.3.swf";
    else if (path == "/swf/flowplayer.js")          sendFile = ":/flowplayer/flowplayer-3.2.4.min.js";

    else if (path.startsWith("/img/"))
    {
      static const QDir imgDir(":/lximediacenter/images/");
      if (imgDir.exists(path.mid(5)))
        sendFile = imgDir.absoluteFilePath(path.mid(5));
    }

    if (!sendFile.isEmpty())
    {
      SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
      response.setContentType(SHttpServer::toMimeType(sendFile));

      if (file.url().hasQueryItem("scale") && path.endsWith(".png"))
      {
        QImage image(sendFile);
        if (!image.isNull())
        {
          QSize size = image.size();
          const QStringList sizeTxt = file.url().queryItemValue("scale").split('x');
          if (sizeTxt.count() >= 2)
            size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
          else if (sizeTxt.count() >= 1)
            size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());

          image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          QBuffer b;
          image.save(&b, "PNG");

          response.setContent(b.data());
          return response;
        }
      }
      else
      {
        QFile file(sendFile);
        if (file.open(QFile::ReadOnly))
        {
          response.setContent(file.readAll());
          return response;
        }
      }
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}

SHttpServer * Backend::httpServer(void)
{
  return &masterHttpServer;
}

SSsdpServer * Backend::ssdpServer(void)
{
  return &masterSsdpServer;
}

SUPnPContentDirectory * Backend::contentDirectory(void)
{
  return &masterContentDirectory;
}

ImdbClient * Backend::imdbClient(void)
{
  return masterImdbClient;
}

SSandboxClient * Backend::createSandbox(SSandboxClient::Priority priority)
{
#ifndef DEBUG_USE_LOCAL_SANDBOX
  return new SSandboxClient(sandboxApplication, priority);
#else
  Sandbox * sandbox = new Sandbox();
  sandbox->start("local");
  return new SSandboxClient(sandbox->server(), priority);
#endif
}

void Backend::recycleSandbox(SSandboxClient *sandboxClient)
{
  if (sandboxClient)
  {
    SHttpEngine::RequestMessage message(sandboxClient);
    message.setRequest("GET", "/?exit");

    sandboxClient->openRequest(message, NULL, NULL);
    QTimer::singleShot(30000, sandboxClient, SLOT(deleteLater()));
  }
}
