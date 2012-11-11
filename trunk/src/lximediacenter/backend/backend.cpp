/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "backend.h"
#ifdef DEBUG_USE_LOCAL_SANDBOX
# include "sandbox.h"
#endif
#include <QtXml>
#include <iostream>

#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
const QEvent::Type  Backend::exitEventType = QEvent::Type(QEvent::registerEventType());
#endif

Backend::Backend()
  : BackendServer::MasterServer(),
    masterHttpServer(SUPnPBase::protocol(), serverUuid()),
    masterSsdpServer(&masterHttpServer),
    masterMediaServer("/upnp/"),
    masterConnectionManager("/upnp/"),
    masterContentDirectory("/upnp/"),
    masterMediaReceiverRegistrar("/upnp/"),
    sandboxApplication("\"" + qApp->applicationFilePath() + "\" --sandbox"),
    cssParser(),
    htmlParser(),
    backendServers(),
    initSandbox(NULL)
{
  // Seed the random number generator.
  qsrand(uint(QDateTime::currentDateTime().toTime_t() + qApp->applicationPid()));

  // Remove previous temporary files.
  const QString baseName = sApp->tempFileBase();
  const QString searchName = baseName.left(baseName.lastIndexOf('-') + 1) + '*';
  QDir tmpDir = QDir::temp();

  foreach (const QString &fileName, tmpDir.entryList(QStringList(searchName), QDir::Files | QDir::System))
  if (!fileName.startsWith(baseName))
    tmpDir.remove(fileName);

  // Open device configuration
  MediaServer::mediaProfiles().openDeviceConfig(":/devices.ini");
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

  qDebug() << "LXiMediaCenter backend stopped.";
}

void Backend::start(void)
{
  QSettings settings;

  masterHttpServer.initialize(
      settings.value("BindAllNetworks", false).toBool()
          ? QNetworkInterface::allAddresses()
          : SSsdpClient::localAddresses(),
      settings.value("HttpPort", defaultPort).toInt());

  // Setup HTTP server
  masterHttpServer.registerCallback("/", this);

  // Setup template parsers
  cssParser.clear();
  htmlParser.clear();
  htmlParser.setStaticField("_PRODUCT", qApp->applicationName());
  htmlParser.setStaticField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());

  backendServers = BackendServer::create(this);
  foreach (BackendServer *server, backendServers)
    server->initialize(this);

  // Setup SSDP server
  masterSsdpServer.initialize(SSsdpClient::localInterfaces());

  // Setup DLNA server
  masterMediaServer.initialize(&masterHttpServer, &masterSsdpServer);
  masterConnectionManager.initialize(&masterHttpServer, &masterMediaServer);
  masterContentDirectory.initialize(&masterHttpServer, &masterMediaServer);
  masterMediaReceiverRegistrar.initialize(&masterHttpServer, &masterMediaServer);

  masterMediaServer.setDeviceName(settings.value("DeviceName", defaultDeviceName()).toString());

  // Default codepage
  const QByteArray defaultCodePage =
      settings.value("DefaultCodepage", "System").toByteArray();

  if (defaultCodePage == "System")
    QTextCodec::setCodecForLocale(NULL);
  else
    QTextCodec::setCodecForLocale(QTextCodec::codecForName(defaultCodePage));

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
  QStringList fileProtocols;

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
    fileProtocols = T::readElement(doc, "fileprotocols", "protocol");
  }

  // Figure out the supported DLNA audio protocols.
  MediaServer::mediaProfiles().setCodecs(
      QSet<QString>::fromList(outAudioCodecs),
      QSet<QString>::fromList(outVideoCodecs),
      QSet<QString>::fromList(QStringList() << "JPEG" << "PNG"),
      QSet<QString>::fromList(outFormats));

  MediaServer::fileProtocols() = QSet<QString>::fromList(fileProtocols);

  masterConnectionManager.setSourceProtocols(MediaServer::mediaProfiles().listProtocols(QString::null));
  masterConnectionManager.setSinkProtocols(SUPnPBase::ProtocolList());

  qDebug() << "Finished initialization.";
}

void Backend::reset(void)
{
  QSettings settings;

  masterHttpServer.reset(
      settings.value("BindAllNetworks", false).toBool()
          ? QNetworkInterface::allAddresses()
          : SSsdpClient::localAddresses(),
      settings.value("HttpPort", defaultPort).toInt());

  masterSsdpServer.reset();
  masterMediaServer.reset();
  masterConnectionManager.reset();
  masterContentDirectory.reset();
  masterMediaReceiverRegistrar.reset();

  htmlParser.clear();
  htmlParser.setField("_PRODUCT", qApp->applicationName());
  htmlParser.setField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());
}

void Backend::addModules(const SHttpEngine::ResponseMessage &modules)
{
  delete initSandbox;
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
#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
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

QUuid Backend::serverUuid(void)
{
  QString uuid = "00000000-0000-0000-0000-000000000000";

  QSettings settings;

  if (settings.contains("UUID"))
    return settings.value("UUID", uuid).toString();

  uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");
  settings.setValue("UUID", uuid);

  return uuid;
}

QString Backend::defaultDeviceName(void)
{
  return QHostInfo::localHostName() + ": " + qApp->applicationName();
}

SHttpServer::ResponseMessage Backend::sendFile(const SHttpServer::RequestMessage &request, const QString &fileName)
{
  SHttpServer::ResponseMessage response(request, SHttpServer::Status_Ok);
  response.setContentType(SHttpServer::toMimeType(fileName));

  if (fileName.endsWith(".png"))
  {
    QImage image(fileName);
    if (!image.isNull())
    {
      bool render = false;

      if (request.url().hasQueryItem("scale"))
      {
        render = true;
        image = image.scaled(
            SSize::fromString(request.url().queryItemValue("scale")).size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
      }

      if (request.url().hasQueryItem("invert"))
      {
        render = true;
        image.invertPixels();
      }

      if (render)
      {
        QBuffer buffer;
        buffer.open(QBuffer::WriteOnly);
        if (image.save(&buffer, "PNG"))
        {
          buffer.close();

          response.setContent(buffer.data());
          return response;
        }
      }
    }
  }

  QFile file(fileName);
  if (file.open(QFile::ReadOnly))
  {
    response.setContent(file.readAll());
    return response;
  }

  return SHttpServer::ResponseMessage(request, SHttpServer::Status_NotFound);
}
