/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
#include "setup.h"
#include <iostream>

const int Backend::upnpRepublishTimout = 15000;

Backend::Backend()
  : BackendServer::MasterServer(),
    upnp(this),
    upnpRootDevice(&upnp, serverUuid(), "urn:schemas-upnp-org:device:MediaServer:1", true),
    upnpConnectionManager(&upnpRootDevice),
    upnpContentDirectory(&upnpRootDevice, &upnpConnectionManager),
    upnpMediaReceiverRegistrar(&upnpRootDevice),
    upnpRepublishRequired(false),
    sandboxApplication("\"" + qApp->applicationFilePath() + "\" --sandbox"),
    cssParser(),
    htmlParser(),
    backendServers()
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

  connect(&upnpRootDevice, SIGNAL(handledAction()), SLOT(handledAction()));
  connect(&upnpConnectionManager, SIGNAL(numConnectionsChanged(int)), SLOT(numConnectionsChanged(int)));

  connect(&upnpRepublishTimer, SIGNAL(timeout()), SLOT(republishRootDevice()));
  upnpRepublishTimer.setTimerType(Qt::VeryCoarseTimer);
  upnpRepublishTimer.setSingleShot(true);
}

Backend::~Backend()
{
  qDebug() << "LXiMediaCenter backend stopping.";

  upnpRepublishTimer.stop();
  upnpRepublishRequired = false;

  upnp.close();

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

  upnp.registerHttpCallback("/", this);
  upnp.registerHttpCallback("/css", this);
  upnp.registerHttpCallback("/img", this);
  upnp.registerHttpCallback("/js", this);
  upnp.registerHttpCallback("/help", this);

  upnp.initialize(
        settings.value("HttpPort", defaultPort).toInt(),
        settings.value("BindAllNetworks", false).toBool());

  upnpRepublishRequired = settings.value("RepublishRootDevice", true).toBool();
  if (upnpRepublishRequired)
    upnpRepublishTimer.start(upnpRepublishTimout * 2);

  upnpRootDevice.setDeviceName(settings.value("DeviceName", defaultDeviceName()).toString());
  upnpRootDevice.addIcon("lximedia.png");

  // Setup template parsers
  cssParser.clear();
  htmlParser.clear();
  htmlParser.setStaticField("_PRODUCT", qApp->applicationName());
  htmlParser.setStaticField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());

  backendServers = BackendServer::create(this);
#ifndef Q_OS_MACX
  backendServers += new Setup(settings.value("AllowShutdown", true).toBool(), this);
#endif

  foreach (BackendServer *server, backendServers)
    server->initialize(this);

  // Default codepage
  const QByteArray defaultCodePage =
      settings.value("DefaultCodepage", "System").toByteArray();

  if (defaultCodePage == "System")
    QTextCodec::setCodecForLocale(NULL);
  else
    QTextCodec::setCodecForLocale(QTextCodec::codecForName(defaultCodePage));

  // Figure out the supported DLNA audio protocols.
  MediaServer::mediaProfiles().setCodecs(
      QSet<QString>::fromList(SAudioEncoderNode::codecs()),
      QSet<QString>::fromList(SVideoEncoderNode::codecs()),
      QSet<QString>::fromList(QStringList() << "jpeg" << "png"),
      QSet<QString>::fromList(SIOOutputNode::formats()));

  MediaServer::fileProtocols() = QSet<QString>::fromList(SMediaFilesystem::protocols());

  upnpConnectionManager.setProtocols(
        MediaServer::mediaProfiles().listProtocols(QString::null),
        ConnectionManager::ProtocolList());

  qDebug() << "Finished initialization.";
}

void Backend::reset(void)
{
  // Wait a bit to allow any pending HTTP requests to be handled.
  QTimer::singleShot(500, this, SLOT(resetUpnpRootDevice()));
}

void Backend::resetUpnpRootDevice(void)
{
  QSettings settings;

  upnpRepublishTimer.stop();
  upnpRepublishRequired = false;

  upnp.close();
  upnp.initialize(
        settings.value("HttpPort", defaultPort).toInt(),
        settings.value("BindAllNetworks", false).toBool());

  upnpRepublishRequired = settings.value("RepublishRootDevice", true).toBool();
  if (upnpRepublishRequired)
    upnpRepublishTimer.start(upnpRepublishTimout * 2);

  htmlParser.clear();
  htmlParser.setField("_PRODUCT", qApp->applicationName());
  htmlParser.setField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());
}

#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
void Backend::performExit(void)
{
  qApp->exit(0);
}
#endif

void Backend::republishRootDevice(void)
{
  if (upnpRepublishRequired)
  {
    upnp.removeChild(&upnpRootDevice);
    upnpRootDevice.close();

    upnpRootDevice.initialize();
    upnp.addChild(&upnpRootDevice);

    upnpRepublishTimer.start(upnpRepublishTimout);
  }
}

void Backend::handledAction(void)
{
  if (upnpRepublishRequired && upnpRepublishTimer.isActive())
    upnpRepublishTimer.start(upnpRepublishTimout * 8);
}

void Backend::numConnectionsChanged(int numConnections)
{
  if (upnpRepublishRequired)
  {
    if (numConnections > 0)
      upnpRepublishTimer.stop();
    else if (numConnections == 0)
      upnpRepublishTimer.start(upnpRepublishTimout * 4);
  }
}

RootDevice * Backend::rootDevice(void)
{
  return &upnpRootDevice;
}

ContentDirectory * Backend::contentDirectory(void)
{
  return &upnpContentDirectory;
}

QUuid Backend::serverUuid(void)
{
  const QString uuid = QUuid::createUuid().toString().replace("{", "").replace("}", "");

  QSettings settings;
  if (settings.contains("UUID"))
    return settings.value("UUID", uuid).toString();

  settings.setValue("UUID", uuid);

  return uuid;
}

QString Backend::defaultDeviceName(void)
{
  return UPnP::hostname() + ": " + qApp->applicationName();
}

HttpStatus Backend::sendFile(const QUrl &request, const QString &fileName, QByteArray &contentType, QIODevice *&response)
{
  const QUrlQuery query(request);

  contentType = UPnP::toMimeType(fileName);

  if (fileName.endsWith(".png"))
  {
    QImage image(fileName);
    if (!image.isNull())
    {
      bool render = false;

      if (query.hasQueryItem("scale"))
      {
        render = true;
        image = image.scaled(
            SSize::fromString(query.queryItemValue("scale")).size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
      }

      if (query.hasQueryItem("invert"))
      {
        render = true;
        image.invertPixels();
      }

      if (render)
      {
        QBuffer * const buffer = new QBuffer();
        if (buffer->open(QBuffer::ReadWrite) && image.save(buffer, "PNG"))
        {
          buffer->close();
          response = buffer;
          return HttpStatus_Ok;
        }

        delete buffer;
      }
    }
  }

  response = new QFile(fileName);
  return HttpStatus_Ok;
}
