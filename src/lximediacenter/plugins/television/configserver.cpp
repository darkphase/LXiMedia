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

#include "configserver.h"
#include "scan.h"
#include "televisionbackend.h"
#include "televisionserver.h"

namespace LXiMediaCenter {

const QEvent::Type ConfigServer::startScanEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type ConfigServer::stopScanEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type ConfigServer::setFreqEventType = QEvent::Type(QEvent::registerEventType());

ConfigServer::ConfigServer(TelevisionServer *televisionServer, MasterServer *server, TelevisionBackend *plugin)
            :BackendServer(QT_TR_NOOP("Television Setup"), server),
             plugin(plugin),
             televisionServer(televisionServer),
             lock(QReadWriteLock::Recursive),
             graph(SGraph::MediaTask_None),
             runningScan(NULL)
{
}

bool ConfigServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (url.hasQueryItem("add"))
    return addChannels(request, socket, url);
  else if (url.hasQueryItem("update"))
    return updateChannels(request, socket, url);
  else if (file == "snapshot.jpeg")
    return sendScanSnapshot(socket,
                            url.queryItemValue("channel").toULongLong(),
                            url.queryItemValue("width").toInt(),
                            url.queryItemValue("height").toInt());
  else if (file == "signalplot.png")
    return sendSignalPlot(socket);
  else if (file.isEmpty() || file.endsWith(".html"))
    return handleHtmlRequest(url, file, socket);

  return BackendServer::handleConnection(request, socket);
}

void ConfigServer::customEvent(QEvent *e)
{
  if (e->type() == startScanEventType)
  {
    SDebug::Trace t("ConfigServer::customEvent(startScanEventType)");
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    delete runningScan;
    runningScan = new Scan(televisionServer->tuners[static_cast<StartScanEvent *>(e)->device]->terminal,
                           static_cast<StartScanEvent *>(e)->device,
                           static_cast<StartScanEvent *>(e)->freq,
                           this);
  }
  else if (e->type() == stopScanEventType)
  {
    SDebug::Trace t("ConfigServer::customEvent(stopScanEventType)");
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    delete runningScan;
    runningScan = NULL;
  }
  else if ((e->type() == setFreqEventType) && runningScan)
  {
    SDebug::Trace t("ConfigServer::customEvent(setFreqEventType)");
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    runningScan->setFrequency(static_cast<SetFreqEvent *>(e)->freq);
  }
}

bool ConfigServer::addChannels(const QHttpRequestHeader &request, QAbstractSocket *socket, const QUrl &url)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  PluginSettings settings(plugin);
  QMap<unsigned, unsigned> presets;

  foreach (const QString &id, url.allQueryItemValues("add"))
  {
    const QString device =
        QByteArray::fromPercentEncoding(
            url.queryItemValue("device_" + id).replace('+', ' ').toAscii());

    const QString name =
        QByteArray::fromPercentEncoding(
            url.queryItemValue("name_" + id).replace('+', ' ').toAscii());

    const QString type =
        typeName(Type(url.queryItemValue("type_" + id).toInt()));

    addChannel(settings, type,
               url.queryItemValue("transponder_" + id).toULongLong(),
               url.queryItemValue("serviceid_" + id).toULongLong(),
               device, name);
  }

  QCoreApplication::postEvent(this, new QEvent(stopScanEventType));
  for (int i=0; (i<20) && (runningScan!=NULL); i++) T::msleep(250);

  QHttpResponseHeader response(301);
  response.setValue("Location", "http://" + request.value("Host") + httpPath());
  socket->write(response.toString().toUtf8());
  return false;
}

void ConfigServer::addChannel(PluginSettings &settings, const QString &type, quint64 transponder, quint16 serviceID, const QString &device, const QString &name)
{
  // Find existing channel
  int preset = 0;
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(type + "Channel_"))
  {
    settings.beginGroup(group);

    if (SStringParser::toRawName(settings.value("name").toString()) == SStringParser::toRawName(name))
    {
      preset = group.mid(8 + type.length()).toInt();
      settings.endGroup();
      break;
    }

    settings.endGroup();
  }

  // Or find an empty preset
  if (preset == 0)
  {
    preset = 1;
    for (bool busy = true; busy; )
    {
      busy = false;
      foreach (const QString &group, settings.childGroups())
      if (group.startsWith(type + "Channel_"))
      if (group.mid(8 + type.length()).toInt() == preset)
      {
        preset++;
        busy = true;
      }
    }
  }

  settings.beginGroup(type + "Channel_" + QString::number(preset));
    settings.setValue("Name", name);
    settings.beginGroup("Device_" + SStringParser::toRawName(device));
      settings.setValue("Name", device);
      settings.setValue("Transponder", transponder);
      if (serviceID > 0) settings.setValue("ServiceID", serviceID);
      settings.setValue("Use", true);
    settings.endGroup();
  settings.endGroup();

}

bool ConfigServer::updateChannels(const QHttpRequestHeader &request, QAbstractSocket *socket, const QUrl &url)
{
  typedef QPair<QString, QString> StringPair;

  PluginSettings settings(plugin);
  QMap<unsigned, unsigned> presets;

  const QString type = typeName(Type(url.queryItemValue("type").toInt()));

  foreach (const StringPair &item, url.queryItems())
  if (item.first.startsWith("newpreset_"))
  {
    const QString presetStr = item.first.mid(10);
    unsigned preset = presetStr.toUInt();
    unsigned newPreset = item.second.toUInt();

    if (presets.contains(preset))
      preset = presets[preset];

    if (preset != newPreset)
    {
      presets[preset] = newPreset;
      presets[newPreset] = preset;
    }

    const QString name =
        QByteArray::fromPercentEncoding(
            url.queryItemValue("newname_" + presetStr).replace('+', ' ').toAscii());

    if (!url.hasQueryItem("delete_" + presetStr))
      updateChannel(settings, type, preset, name, newPreset, url.allQueryItemValues("use_" + presetStr));
    else
      settings.remove(type + "Channel_" + QString::number(preset));
  }

  QHttpResponseHeader response(301);
  response.setValue("Location", "http://" + request.value("Host") + httpPath());
  socket->write(response.toString().toUtf8());
  return false;
}

void ConfigServer::updateChannel(PluginSettings &settings, const QString &type, int preset, const QString &newname, int newpreset, const QStringList &devices)
{
  if (newpreset != preset)
  {
    copyPresets(settings, type, newpreset, -1);
    copyPresets(settings, type, preset, newpreset);
    copyPresets(settings, type, -1, preset);
  }

  settings.beginGroup(type + "Channel_" + QString::number(newpreset));
  settings.setValue("Name", newname);

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith("Device_"))
  {
    settings.beginGroup(group);
    settings.setValue("Use", bool(devices.contains(settings.value("Name").toByteArray().toPercentEncoding())));
    settings.endGroup();
  }

  settings.endGroup();
}

void ConfigServer::copyPresets(PluginSettings &settings, const QString &type, int preset, int newpreset)
{
  // Get data from old group
  QList< QPair<QString, QVariant> > keys;
  QList< QPair<QString, QString> > groups;

  groups += QPair<QString, QString>(type + "Channel_" + QString::number(preset), "");

  while (!groups.isEmpty())
  {
    QPair<QString, QString> group = groups.takeFirst();
    settings.beginGroup(group.first);

    foreach (const QString &childKey, settings.childKeys())
      keys += QPair<QString, QVariant>(group.second + childKey, settings.value(childKey));

    foreach (const QString &childGroup, settings.childGroups())
      groups += QPair<QString, QString>(group.first + "/" + childGroup, group.second + childGroup + "/");

    settings.endGroup();
  }

  // Delete old group
  settings.remove(type + "Channel_" + QString::number(preset));

  // Create new group
  settings.beginGroup(type + "Channel_" + QString::number(newpreset));
    for (QList< QPair<QString, QVariant> >::Iterator i=keys.begin(); i!=keys.end(); i++)
      settings.setValue(i->first, i->second);
  settings.endGroup();
}

bool ConfigServer::sendScanSnapshot(QAbstractSocket *socket, quint64 channel, int maxH, int maxW) const
{
  QImage image;
  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  if (runningScan)
  {
    if (channel == 0)
    {
      image = runningScan->lastSnapshot();
    }
    else foreach (const Scan::Channel &c, runningScan->channels())
    if (c.frequency == channel)
    {
      image = c.thumbnail;
      break;
    }
  }

  l.unlock();

  if (image.isNull())
  {
    image = QImage(maxW, maxH, QImage::Format_RGB32);
    memset(image.bits(), 0, image.numBytes());
  }
  else if ((maxW > 0) && (maxH > 0) && ((image.width() > maxW) || (image.height() > maxH)))
  {
    image = image.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::FastTransformation);
  }

  QByteArray jpgData;
  QBuffer buffer(&jpgData);
  buffer.open(QIODevice::WriteOnly);
  image.save(&buffer, "JPEG", 80);
  buffer.close();

  return sendReply(socket, buffer.data(), "image/jpeg");
}

bool ConfigServer::sendSignalPlot(QAbstractSocket *socket) const
{
  QImage image;

  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  if (runningScan)
    image = runningScan->signalPlot();

  l.unlock();

  if (!image.isNull())
  {
    QByteArray jpgData;
    QBuffer buffer(&jpgData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();

    return sendReply(socket, buffer.data(), "image/png");
  }
  else
  {
    socket->write(QHttpResponseHeader(404).toString().toUtf8());
    return false;
  }
}

bool ConfigServer::handleHtmlRequest(const QUrl &url, const QString &, QAbstractSocket *socket)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  QHttpResponseHeader response(200);
  response.setContentType("text/html;charset=utf-8");
  response.setValue("Cache-Control", "no-cache");

  PluginSettings settings(plugin);

  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  const QString scan = url.queryItemValue("scan");
  if ((scan == "start") && (runningScan == NULL))
  {
    const QString device = QByteArray::fromPercentEncoding(url.queryItemValue("device").toAscii());
    if (televisionServer->tuners.contains(device))
    {
      QCoreApplication::postEvent(this,
        new StartScanEvent(device, 0));

      l.unlock();
        for (int i=0; (i<20) && (runningScan==NULL); i++) T::msleep(250);
      l.relock(__FILE__, __LINE__);
    }
  }
  else if ((scan == "add") && (runningScan == NULL))
  {
    const QString device = QByteArray::fromPercentEncoding(url.queryItemValue("device").toAscii());
    if (televisionServer->tuners.contains(device))
    {
      QCoreApplication::postEvent(this,
        new StartScanEvent(device, url.queryItemValue("freq").toULongLong() * Q_UINT64_C(1000)));

      l.unlock();
        for (int i=0; (i<20) && (runningScan==NULL); i++) T::msleep(250);
      l.relock(__FILE__, __LINE__);
    }
  }
  else if ((scan == "stop") && (runningScan != NULL))
  {
    runningScan->stop();
  }
  else if ((scan == "close") && (runningScan != NULL))
  {
    QCoreApplication::postEvent(this, new QEvent(stopScanEventType));
    l.unlock();
      for (int i=0; (i<20) && (runningScan!=NULL); i++) T::msleep(250);
    l.relock(__FILE__, __LINE__);
  }

  HtmlParser htmlParser;
  htmlParser.setField("TR_ADD", tr("Add"));
  htmlParser.setField("TR_ADD_CHANNEL", tr("Add channel"));
  htmlParser.setField("TR_TUNERS", tr("Tuners"));
  htmlParser.setField("TR_CHANNEL_NAME", tr("Channel name"));
  htmlParser.setField("TR_CLOSE_SCAN", tr("Close scan"));
  htmlParser.setField("TR_DELETE", tr("Delete"));
  htmlParser.setField("TR_FOUND_CHANNELS", tr("Found channels"));
  htmlParser.setField("TR_FREQUENCY", tr("Frequency"));
  htmlParser.setField("TR_LAST_CHANNEL", tr("Last channel"));
  htmlParser.setField("TR_LAST_PROGRAMME", tr("Last programme"));
  htmlParser.setField("TR_PROGRESS", tr("Progress"));
  htmlParser.setField("TR_PROVIDER", tr("Provider"));
  htmlParser.setField("TR_RADIO_STATIONS", tr("Radio stations"));
  htmlParser.setField("TR_SAVE", tr("Save"));
  htmlParser.setField("TR_SET", tr("Set"));
  htmlParser.setField("TR_START_SCAN", tr("Scan for channels"));
  htmlParser.setField("TR_STOP_SCAN", tr("Stop scan"));
  htmlParser.setField("TR_TV_CHANNELS", tr("TV channels"));

  htmlParser.setField("FOUND_CHANNEL_LIST", QByteArray(""));

  if (runningScan == NULL)
  {
    htmlParser.setField("COMPLETED", QByteArray(""));
    htmlParser.setField("FREQ", QByteArray(""));
    htmlParser.setField("LAST_CHANNEL", QByteArray(""));
    htmlParser.setField("LAST_PROGRAMME", QByteArray(""));
    htmlParser.setField("SCAN_ACTION", QByteArray("start"));

    htmlParser.setField("TUNER_LIST", QByteArray(""));
    for (QMap<QString, TelevisionServer::Tuner *>::Iterator i=televisionServer->tuners.begin();
         i!=televisionServer->tuners.end();
         i++)
    {
      htmlParser.setField("DEVICE", i.key());
      htmlParser.setField("NAME", (*i)->terminal->friendlyName());
      htmlParser.appendField("TUNER_LIST", htmlParser.parse(htmlTunerItem));
    }

    htmlParser.setField("TYPE_TV", QString::number(Type_Television));
    htmlParser.setField("TYPE_RADIO", QString::number(Type_Radio));

    for (int typeId = Type_Television; typeId<=Type_Radio; typeId++)
    {
      const QString type = typeName(Type(typeId));

      QMap<int, QString> channels;
      foreach (const QString &group, settings.childGroups())
      if (group.startsWith(type + "Channel_"))
        channels[group.mid(8 + type.length()).toInt()] = group;

      htmlParser.setField((type.toUpper() + "_CHANNEL_LIST").toAscii(), QByteArray(""));
      for (QMap<int, QString>::ConstIterator group=channels.begin(); group!=channels.end(); group++)
      {
        settings.beginGroup(*group);

        htmlParser.setField("PRESETS", QByteArray(""));
        for (QMap<int, QString>::Iterator i=channels.begin(); i!=channels.end(); i++)
        {
          htmlParser.setField("VALUE", QByteArray::number(i.key()));
          htmlParser.setField("TEXT", QByteArray::number(i.key()));
          htmlParser.setField("SELECTED", group.key() == i.key() ? QByteArray("selected=\"selected\"") : QByteArray(""));
          htmlParser.appendField("PRESETS", htmlParser.parse(htmlOption));
        }

        htmlParser.setField("PRESET", QString::number(group.key()));
        htmlParser.setField("NAME", settings.value("Name").toString());

        QStringList devices;
        foreach (const QString &group, settings.childGroups())
        if (group.startsWith("Device_"))
          devices += group;

        htmlParser.setField("ROWS", QByteArray::number(devices.count() + 1));
        htmlParser.appendField((type.toUpper() + "_CHANNEL_LIST").toAscii(), htmlParser.parse(htmlChannelItem));

        foreach (const QString &group, devices)
        {
          settings.beginGroup(group);

          const QString name = settings.value("Name").toString();
          if (televisionServer->tuners.contains(name))
          {
            htmlParser.setField("DEVICE", name);
            htmlParser.setField("NAME", televisionServer->tuners[name]->terminal->friendlyName());
            htmlParser.setField("CHECKED", QByteArray(settings.value("Use", true).toBool() ? "checked" : ""));
            htmlParser.appendField((type.toUpper() + "_CHANNEL_LIST").toAscii(), htmlParser.parse(htmlDeviceItem));
          }

          settings.endGroup();
        }

        settings.endGroup();
      }
    }

    l.unlock();

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlMain));
  }
  else if (runningScan->isDvb() == false)
  {
    if (runningScan->isBusy() && (scan != "stop"))
      response.setValue("Refresh", "1;URL=");

    htmlParser.setField("TUNER", runningScan->device()->friendlyName());

    htmlParser.setField("COMPLETED", QByteArray::number(int(runningScan->completed() * 100.0)) + " %");
    htmlParser.setField("FREQ", QLocale().toString(runningScan->scanFrequency() / Q_UINT64_C(1000)) + " kHz");
    htmlParser.setField("LAST_CHANNEL", runningScan->lastChannel());
    htmlParser.setField("LAST_PROGRAMME", runningScan->lastProgramme());

    htmlParser.setField("DEVICE", runningScan->deviceName());
    htmlParser.setField("PID", QString::number(0));

    htmlParser.setField("CHANNEL_LIST", QByteArray(""));
    unsigned id = 0;
    foreach (const Scan::Channel &channel, runningScan->channels())
    {
      htmlParser.setField("ID", QByteArray::number(id++));
      htmlParser.setField("TYPE", QString::number(channel.type));
      htmlParser.setField("NAME", channel.name);
      htmlParser.setField("TRANSPONDER", QString::number(channel.frequency));
      htmlParser.setField("SERVICEID", QString::number(channel.serviceID));
      htmlParser.setField("INFO", channel.name + ", " + channel.programme + ", " + QLocale().toString(channel.frequency / Q_UINT64_C(1000)) + " kHz");
      htmlParser.setField("THUMBNAIL", httpPath() + "?action=scan_snapshot&amp;channel=" + QString::number(channel.frequency) + "&amp;width=88&amp;height=72");
      htmlParser.setField("IMAGE", httpPath() + "?action=scan_snapshot&amp;channel=" + QString::number(channel.frequency));
      htmlParser.appendField("CHANNEL_LIST", htmlParser.parse(htmlFoundChannelItem));
    }

    l.unlock();

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlAnalogScan));
  }
  else
  {
    if (runningScan->isBusy() && (scan != "stop"))
      response.setValue("Refresh", "1;URL=");

    htmlParser.setField("TUNER", runningScan->device()->friendlyName());

    htmlParser.setField("COMPLETED", QByteArray::number(int(runningScan->completed() * 100.0)) + " %");
    htmlParser.setField("FREQ", QLocale().toString(runningScan->scanFrequency() / Q_UINT64_C(1000)) + " kHz");
    htmlParser.setField("PROVIDER", runningScan->lastProvider());

    htmlParser.setField("DEVICE", runningScan->deviceName());
    htmlParser.setField("PID", QString::number(0));

    htmlParser.setField("CHANNEL_LIST", QByteArray(""));
    unsigned id = 0;
    foreach (const Scan::Channel &channel, runningScan->channels())
    {
      htmlParser.setField("ID", QByteArray::number(id++));
      htmlParser.setField("TYPE", QString::number(channel.type));
      htmlParser.setField("NAME", channel.name);
      htmlParser.setField("TRANSPONDER", QString::number(channel.frequency));
      htmlParser.setField("SERVICEID", QString::number(channel.serviceID));
      htmlParser.setField("INFO", channel.name + ", " + channel.programme);
      htmlParser.setField("IMAGE", httpPath() + "?action=scan_snapshot&amp;channel=" + QString::number(channel.frequency));
      htmlParser.appendField("CHANNEL_LIST", htmlParser.parse(htmlFoundChannelItem));
    }

    l.unlock();

    return sendHtmlContent(socket, url, response, htmlParser.parse(htmlDigitalScan));
  }
}

} // End of namespace
