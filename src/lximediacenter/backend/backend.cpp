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
    formatSandbox(NULL)
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
      localParser.setField("ITEM_URL", item.url);
      localParser.setField("ITEM_ICONURL", item.iconurl + "?scale=32");
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
  formatSandbox = createSandbox(SSandboxClient::Priority_Low);
  connect(formatSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(start(SHttpEngine::ResponseMessage)));

  SSandboxClient::RequestMessage request(formatSandbox);
  request.setRequest("GET", "/?formats");
  formatSandbox->sendRequest(request);
}

void Backend::start(const SHttpEngine::ResponseMessage &formats)
{
  recycleSandbox(formatSandbox);
  formatSandbox = NULL;

  // Decode the message
  QStringList outAudioCodecs;
  QStringList outVideoCodecs;
  QStringList outFormats;
  foreach (const QString &line, formats.content().split('\n'))
  if (line.startsWith("AudioCodecs:"))
    outAudioCodecs = line.mid(12).trimmed().split('\t');
  else if (line.startsWith("VideoCodecs:"))
    outVideoCodecs = line.mid(12).trimmed().split('\t');
  else if (line.startsWith("Formats:"))
    outFormats = line.mid(8).trimmed().split('\t');

  // Supported DLNA audio protocols
  if (outFormats.contains("ac3") && outAudioCodecs.contains("AC3"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::AC3, -2); // Prefer

  if (outFormats.contains("s16be") && outAudioCodecs.contains("PCM/S16BE"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::LPCM, -2); // Prefer

  if (outFormats.contains("mp2") && outAudioCodecs.contains("MP2"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::MP2);

  if (outFormats.contains("mp3") && outAudioCodecs.contains("MP3"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::MP3, -1); // Prefer

  if (outFormats.contains("ogg") && outAudioCodecs.contains("VORBIS"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::VORBIS, 1);

  // Supported DLNA video protocols
  if (outFormats.contains("vob") && outVideoCodecs.contains("MPEG1") && outAudioCodecs.contains("MP2"))
    MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG1, 16);

  if (outFormats.contains("vob") && outVideoCodecs.contains("MPEG2"))
  {
    if (outAudioCodecs.contains("MP2"))
    {
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_PAL);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_NTSC);
    }

    if (outAudioCodecs.contains("AC3"))
    {
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_PAL_XAC3);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_NTSC_XAC3);
    }

    if (outAudioCodecs.contains("MP2") || outAudioCodecs.contains("AC3"))
    {
      // Prefer EU over NA because it supports 25 Hz and MP2.
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_SD_EU, 4);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_HD_EU, -9);
    }

    if (outAudioCodecs.contains("AC3"))
    {
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_SD_NA, 7);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_PS_HD_NA, -6);
    }
  }

  if (outFormats.contains("mpegts") && outVideoCodecs.contains("MPEG2"))
  {
    if (outAudioCodecs.contains("MP2") || outAudioCodecs.contains("AC3"))
    {
      // Prefer EU over NA because it supports 25 Hz and MP2.
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_SD_EU_ISO, 5);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_HD_EU_ISO, -8);
    }

    if (outAudioCodecs.contains("AC3"))
    {
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_SD_NA_ISO, 8);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_HD_NA_ISO, -5);
    }
  }

  if (outFormats.contains("m2ts") && outVideoCodecs.contains("MPEG2"))
  {
    if (outAudioCodecs.contains("MP2") || outAudioCodecs.contains("AC3"))
    {
      // Prefer EU over NA because it supports 25 Hz and MP2.
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_SD_EU, 6);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_HD_EU, -7);
    }

    if (outAudioCodecs.contains("AC3"))
    {
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_SD_NA, 9);
      MediaServer::mediaProfiles().addProfile(MediaProfiles::MPEG_TS_HD_NA, -4);
    }
  }

  if (outFormats.contains("ogg") &&
      outVideoCodecs.contains("THEORA") && outAudioCodecs.contains("VORBIS"))
  {
    MediaServer::mediaProfiles().addProfile(MediaProfiles::VORBIS, 9);
  }

  // Supported DLNA image protocols
  MediaServer::mediaProfiles().addProfile(MediaProfiles::JPEG_LRG, -3); // Prefer
  MediaServer::mediaProfiles().addProfile(MediaProfiles::JPEG_MED);
  MediaServer::mediaProfiles().addProfile(MediaProfiles::JPEG_SM);
  MediaServer::mediaProfiles().addProfile(MediaProfiles::JPEG_TN);
  MediaServer::mediaProfiles().addProfile(MediaProfiles::PNG_LRG, -2); // Prefer
  MediaServer::mediaProfiles().addProfile(MediaProfiles::PNG_TN);

  masterConnectionManager.setSourceProtocols(MediaServer::mediaProfiles().listProtocols());
  masterConnectionManager.setSinkProtocols(SUPnPBase::ProtocolList());

  /* HTML5 is still very buggy implemented in many browsers, so for now we stay
     with Flash video.

  if (outFormats.contains("ogg") &&
      outVideoCodecs.contains("THEORA") && outAudioCodecs.contains("FLAC"))
  {
    MediaServer::enableHtml5();
  }*/

  qDebug() << "Finished initialization.";
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

        // DLNA clients
        settings.beginGroup("DLNA");

        foreach (const QString &group, settings.childGroups())
        if (group.startsWith("Client_"))
        {
          QDomElement dlnaClient = doc.createElement("dlnaclient");
          root.appendChild(dlnaClient);
          dlnaClient.setAttribute("name", group.mid(7));
          dlnaClient.setAttribute("useragent", settings.value("UserAgent", tr("Unknown")).toString());
        }

        settings.endGroup();

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

        SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
        response.setField("Cache-Control", "no-cache");
        response.setContentType("text/xml;charset=utf-8");
        response.setContent(doc.toByteArray());

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
