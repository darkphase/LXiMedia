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
#include "setup.h"
#include <QtXml>
#include <iostream>

#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
const QEvent::Type  Backend::exitEventType = QEvent::Type(QEvent::registerEventType());
#endif

Backend::Backend()
  : BackendServer::MasterServer(),
    upnpRootDevice(serverUuid(), "urn:schemas-upnp-org:device:MediaServer:1", this),
    upnpConnectionManager(&upnpRootDevice),
    upnpContentDirectory(&upnpRootDevice, &upnpConnectionManager),
    upnpMediaReceiverRegistrar(&upnpRootDevice),
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
}

Backend::~Backend()
{
  qDebug() << "LXiMediaCenter backend stopping.";

  upnpRootDevice.close();

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

  upnpRootDevice.registerHttpCallback("/", this);
  upnpRootDevice.registerHttpCallback("/css", this);
  upnpRootDevice.registerHttpCallback("/img", this);
  upnpRootDevice.registerHttpCallback("/js", this);
  upnpRootDevice.registerHttpCallback("/help", this);

  upnpRootDevice.setDeviceName(settings.value("DeviceName", defaultDeviceName()).toString());
  upnpRootDevice.addIcon("/lximedia.png");

  upnpRootDevice.initialize(
        settings.value("HttpPort", defaultPort).toInt(),
        settings.value("BindAllNetworks", false).toBool());

  // Setup template parsers
  cssParser.clear();
  htmlParser.clear();
  htmlParser.setStaticField("_PRODUCT", qApp->applicationName());
  htmlParser.setStaticField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());

  backendServers = BackendServer::create(this);
  backendServers += new Setup(settings.value("AllowShutdown", true).toBool(), this);
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
  QSettings settings;

  upnpRootDevice.close();
  upnpRootDevice.initialize(
        settings.value("HttpPort", defaultPort).toInt(),
        settings.value("BindAllNetworks", false).toBool());

  htmlParser.clear();
  htmlParser.setField("_PRODUCT", qApp->applicationName());
  htmlParser.setField("_HOSTNAME", (settings.value("DeviceName", defaultDeviceName())).toString());
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
  return UPnP::hostname() + ": " + qApp->applicationName();
}

HttpStatus Backend::sendFile(const QUrl &request, const QString &fileName, QByteArray &contentType, QIODevice *&response)
{
  const QUrlQuery query(request);

  contentType = RootDevice::toMimeType(fileName);

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
