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
    masterImdbClient(NULL),
    sandboxApplication("\"" + qApp->applicationFilePath() + "\" --sandbox"),
    cssParser(),
    htmlParser(),
    backendServers()
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

  const QImage icon(":/lximedia.png");
  if (!icon.isNull())
    masterMediaServer.addIcon("/lximedia.png", icon.width(), icon.height(), icon.depth());

  // Supported DLNA protocols
  const QStringList outAudioCodecs = SAudioEncoderNode::codecs();
  const QStringList outVideoCodecs = SVideoEncoderNode::codecs();
  const QStringList outFormats = SIOOutputNode::formats();

  SUPnPBase::ProtocolList audioProtocols;
  {
    if (outFormats.contains("s16be") && outAudioCodecs.contains("PCM/S16BE"))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/L16;rate=48000;channels=2", true, "DLNA.ORG_PN=LPCM", ".lpcm");

    if (outFormats.contains("mp3") && outAudioCodecs.contains("MP3"))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/mpeg", true, "DLNA.ORG_PN=MP3", ".mp3");

    if (outFormats.contains("mp2") && outAudioCodecs.contains("MP2"))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/mpeg", true, QString::null, ".mpa");

    if (outFormats.contains("ac3") && outAudioCodecs.contains("AC3"))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/x-ac3", true, QString::null, ".ac3");

    if (outFormats.contains("ogg") && (outAudioCodecs.contains("VORBIS") || outAudioCodecs.contains("FLAC")))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/ogg", true, QString::null, ".oga");

    if (outFormats.contains("wav") && outAudioCodecs.contains("PCM/S16LE"))
      audioProtocols += SUPnPBase::Protocol("http-get", "audio/wave", true, QString::null, ".wav");

    if (outFormats.contains("flv") && outAudioCodecs.contains("PCM/S16LE"))
      audioProtocols += SUPnPBase::Protocol("http-get", "video/x-flv", true, QString::null, ".flv");
  }

  SUPnPBase::ProtocolList videoProtocols;
  {
    if ((outVideoCodecs.contains("MPEG1") || outVideoCodecs.contains("MPEG2")) &&
        outAudioCodecs.contains("MP2"))
    {
      if (outFormats.contains("vob"))
        videoProtocols += SUPnPBase::Protocol("http-get", "video/mpeg", true, QString::null, ".mpeg");

      if (outFormats.contains("mpegts"))
        videoProtocols += SUPnPBase::Protocol("http-get", "video/MP2T", true, QString::null, ".ts");

      if (outFormats.contains("mpegts"))
      {
        QMap<QString, QString> hdeu;
        hdeu["framerates"] = "24,25";

        QMap<QString, QString> hdna;
        hdna["framerates"] = "24,30";

        videoProtocols += SUPnPBase::Protocol("http-get", "video/MP2T", true, "DLNA.ORG_PN=MPEG_TS_HD_EU_ISO", ".ts", hdeu);
        videoProtocols += SUPnPBase::Protocol("http-get", "video/MP2T", true, "DLNA.ORG_PN=MPEG_TS_HD_NA_ISO", ".ts", hdna);
      }

      if (outFormats.contains("vob"))
      {
        QMap<QString, QString> pal;
        pal["size"]         = "720x576x1.42222,box";
        pal["framerates"]   = "25";
        pal["channels"]     = QString::number(SAudioFormat::Channels_Stereo, 16);

        QMap<QString, QString> ntsc;
        ntsc["size"]        = "704x480x1.21307,box";
        ntsc["framerates"]  = "24,30";
        ntsc["channels"]    = QString::number(SAudioFormat::Channels_Stereo, 16);

        videoProtocols += SUPnPBase::Protocol("http-get", "video/mpeg", true, "DLNA.ORG_PN=MPEG_PS_PAL",  ".mpeg", pal);
        videoProtocols += SUPnPBase::Protocol("http-get", "video/mpeg", true, "DLNA.ORG_PN=MPEG_PS_NTSC", ".mpeg", ntsc);
      }
    }

    if (outFormats.contains("ogg") &&
        outVideoCodecs.contains("THEORA") && (outAudioCodecs.contains("VORBIS") || outAudioCodecs.contains("FLAC")))
    {
      videoProtocols += SUPnPBase::Protocol("http-get", "video/ogg", true, QString::null, ".ogv");
    }

    if (outFormats.contains("flv") &&
        outVideoCodecs.contains("FLV1") && outAudioCodecs.contains("PCM/S16LE"))
    {
      videoProtocols += SUPnPBase::Protocol("http-get", "video/x-flv", true, QString::null, ".flv");
    }
  }

  SUPnPBase::ProtocolList imageProtocols = SUPnPBase::ProtocolList()
      << SUPnPBase::Protocol("http-get", "image/jpeg",  true, "DLNA.ORG_PN=JPEG_LRG", ".jpeg")
      << SUPnPBase::Protocol("http-get", "image/png",   true, "DLNA.ORG_PN=PNG_LRG", ".png")
      << SUPnPBase::Protocol("http-get", "image/png",   true, "DLNA.ORG_PN=PNG_SM", "-thumb.png");

  masterConnectionManager.setSourceProtocols(audioProtocols + videoProtocols + imageProtocols);
  masterContentDirectory.setProtocols(SUPnPContentDirectory::ProtocolType_Audio, audioProtocols);
  masterContentDirectory.setProtocols(SUPnPContentDirectory::ProtocolType_Video, videoProtocols);
  masterContentDirectory.setProtocols(SUPnPContentDirectory::ProtocolType_Image, imageProtocols);

  setContentDirectoryQueryItems();

  /* HTML5 is still very buggy implemented in many browsers, so for now we stay
     with Flash video.

  if (outFormats.contains("ogg") &&
      outVideoCodecs.contains("THEORA") && (outAudioCodecs.contains("VORBIS") || outAudioCodecs.contains("FLAC")))
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

SHttpServer::SocketOp Backend::handleHttpRequest(const SHttpServer::RequestMessage &request, QAbstractSocket *socket)
{
  if ((request.method() == "GET") || (request.method() == "HEAD"))
  {
    const QUrl url(request.path());
    const QString path = url.path();
    const QString file = path.mid(path.lastIndexOf('/') + 1);

    if (path.left(path.lastIndexOf('/') + 1) == "/")
    {
      if (url.hasQueryItem("exit"))
      {
        QCoreApplication::postEvent(this, new QEvent(exitEventType));

        return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NoContent, this);
      }
      else if (url.hasQueryItem("restart"))
      {
        QCoreApplication::postEvent(this, new QEvent(restartEventType));

        return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NoContent, this);
      }
      else if (url.hasQueryItem("shutdown"))
      {
        QCoreApplication::postEvent(this, new QEvent(shutdownEventType));

        return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NoContent, this);
      }
      else if (url.hasQueryItem("dismisserrors"))
      {
        GlobalSettings().setValue("DismissedErrors", sApp->errorLogFiles());

        return handleHtmlRequest(request, socket, file);
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

        SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
        response.setContentType("text/xml;charset=utf-8");
        response.setField("Cache-Control", "no-cache");
        socket->write(response);
        socket->write(doc.toByteArray());
        return SHttpServer::SocketOp_Close;
      }
      else if (url.hasQueryItem("q"))
      {
        return handleHtmlSearch(request, socket, file);
      }
      else if (request.path() == "/")
      {
        return handleHtmlRequest(request, socket, file);
      }
      else if (file.endsWith(".log"))
      {
        return handleHtmlLogFileRequest(request, socket, file);
      }
      else if (file == "settings.html")
      {
        return handleHtmlConfig(request, socket);
      }
      else if (file == "about.html")
      {
        return showAbout(request, socket);
      }
    }

    // Check if the root directory of a plugin was requested.
    QString pluginName = path.mid(1, path.length() - 2);
    foreach (BackendServer *server, backendServers)
    if (pluginName == server->pluginName())
      return handleHtmlRequest(request, socket, pluginName);

    QString sendFile;
    if      (path == "/main.css")                   sendFile = ":/backend/main.css";
    else if (path == "/favicon.ico")                sendFile = ":/lximedia.ico";
    else if (path == "/lximedia.png")               sendFile = ":/lximedia.png";

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
      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType(SHttpServer::toMimeType(sendFile));

      if (url.hasQueryItem("scale") && path.endsWith(".png"))
      {
        QImage image(sendFile);
        if (!image.isNull())
        {
          QSize size = image.size();
          const QStringList sizeTxt = url.queryItemValue("scale").split('x');
          if (sizeTxt.count() >= 2)
            size = QSize(sizeTxt[0].toInt(), sizeTxt[1].toInt());
          else if (sizeTxt.count() >= 1)
            size = QSize(sizeTxt[0].toInt(), sizeTxt[0].toInt());

          image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

          QBuffer b;
          image.save(&b, "PNG");

          response.setContentLength(b.size());
          socket->write(response);
          socket->write(b.data());
          return SHttpServer::SocketOp_Close;
        }
      }
      else
      {
        QFile file(sendFile);
        if (file.open(QFile::ReadOnly))
        {
          response.setContentLength(file.size());
          socket->write(response);
          socket->write(file.readAll());
          return SHttpServer::SocketOp_Close;
        }
      }
    }
  }

  return SHttpServer::sendResponse(request, socket, SHttpServer::Status_NotFound, this);
}

void Backend::handleHttpOptions(SHttpServer::ResponseHeader &response)
{
  response.setField("Allow", response.field("Allow") + ",GET");
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
  QMap<SSandboxClient::Priority, QList<SSandboxClient *> >::Iterator i = sandboxClients.find(priority);
  if ((i == sandboxClients.end()) || i->isEmpty())
  {
#ifndef DEBUG_USE_LOCAL_SANDBOX
    return new SSandboxClient(sandboxApplication, priority);
#else
    Sandbox * sandbox = new Sandbox();
    sandbox->start("local");
    return new SSandboxClient(sandbox->server(), priority);
#endif
  }
  else
    return i->takeLast();
}

void Backend::recycleSandbox(SSandboxClient *sandboxClient)
{
  sandboxClients[sandboxClient->priority()].append(sandboxClient);
}

void Backend::setContentDirectoryQueryItems(void)
{
  GlobalSettings settings;
  settings.beginGroup("DLNA");

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

  foreach (const QString &group, settings.childGroups() << QString::null)
  if (group.isEmpty() || group.startsWith("Client_"))
  {
    settings.beginGroup(group);

    QMap<QString, QString> queryItems;

    const QString transcodeSize = settings.value("TranscodeSize", genericTranscodeSize).toString();
    const QString transcodeCrop = settings.value("TranscodeCrop", genericTranscodeCrop).toString();
    foreach (const GlobalSettings::TranscodeSize &size, GlobalSettings::allTranscodeSizes())
    if (size.name == transcodeSize)
    {
      QString sizeStr =
          QString::number(size.size.width()) + 'x' +
          QString::number(size.size.height()) + 'x' +
          QString::number(size.size.aspectRatio(), 'f', 3);

      if (!transcodeCrop.isEmpty())
        sizeStr += ',' + transcodeCrop.toLower();

      queryItems["size"] = sizeStr;
      break;
    }

    QString channels = QString::number(SAudioFormat::Channels_Stereo, 16);
    const QString transcodeChannels = settings.value("TranscodeChannels", genericTranscodeChannels).toString();
    foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
    if (channel.name == transcodeChannels)
    {
      channels = QString::number(channel.channels, 16);
      break;
    }

    const QString transcodeMusicChannels = settings.value("TranscodeMusicChannels", genericTranscodeMusicChannels).toString();
    foreach (const GlobalSettings::TranscodeChannel &channel, GlobalSettings::allTranscodeChannels())
    if (channel.name == transcodeMusicChannels)
    {
      channels += "," + QString::number(channel.channels, 16);
      break;
    }

    queryItems["channels"] = channels;

    queryItems["priority"] = "high";

    const QString encodeMode = settings.value("EncodeMode", genericEncodeMode).toString();
    if (!encodeMode.isEmpty())
      queryItems["encode"] = encodeMode.toLower();
    else
      queryItems["encode"] = "fast";

    masterContentDirectory.setQueryItems(QString::null, queryItems);

    settings.endGroup();
  }
}
