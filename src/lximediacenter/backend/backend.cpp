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

#ifndef QT_NO_DEBUG
const QEvent::Type  Backend::exitEventType = QEvent::Type(QEvent::registerEventType());
#endif

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

  backendServers = BackendServer::create(this);
  foreach (BackendServer *server, backendServers)
    server->initialize(this);

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

void Backend::customEvent(QEvent *e)
{
#ifndef QT_NO_DEBUG
  if (e->type() == exitEventType)
    qApp->exit(0);
  else
#endif
    QObject::customEvent(e);
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
  struct SandboxThread : QThread
  {
    SandboxThread()
      : startSem(0), sandbox(NULL)
    {
    }

    virtual void run(void)
    {
      sandbox = new Sandbox();
      sandbox->start("local");

      startSem.release(1);

      exec();

      delete sandbox;
    }

    QSemaphore startSem;
    Sandbox * volatile sandbox;
  };

  SandboxThread * const thread = new SandboxThread();
  thread->start();
  thread->startSem.acquire(1);

  return new SSandboxClient(thread->sandbox->server(), priority);
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

SHttpServer::ResponseMessage Backend::sendFile(const SHttpServer::RequestMessage &request, const QString &fileName)
{
  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setContentType(SHttpServer::toMimeType(fileName));

  if (request.url().hasQueryItem("scale") && fileName.endsWith(".png"))
  {
    QImage image(fileName);
    if (!image.isNull())
    {
      image = image.scaled(
          SSize::fromString(request.url().queryItemValue("scale")).size(),
          Qt::KeepAspectRatio,
          Qt::SmoothTransformation);

      QBuffer b;
      image.save(&b, "PNG");

      response.setContent(b.data());
      return response;
    }
  }
  else
  {
    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
      response.setContent(file.readAll());
      return response;
    }
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}
