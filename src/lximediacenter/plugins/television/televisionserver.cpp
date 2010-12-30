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

#include "televisionserver.h"
#if defined(Q_OS_UNIX)
  #include <sys/vfs.h>
#elif defined (Q_OS_WIN32)
  #include <windows.h>
#endif
#include <LXiStreamGui>
#include "televisionbackend.h"

namespace LXiMediaCenter {

TelevisionServer::TelevisionServer(EpgDatabase *epgDatabase, TeletextServer *teletextServer, MasterServer *server, TelevisionBackend *plugin)
                 :VideoServer(QT_TR_NOOP("Television"), server),
                  plugin(plugin),
                  epgDatabase(epgDatabase),
                  teletextServer(teletextServer),
                  timeshiftDir(GlobalSettings::applicationDataDir() + "/timeshift"),
                  timeshiftEnabled(true),
                  buildingPlan(false)
{
  enableDlna();

  PluginSettings settings(plugin);

  head = " <link rel=\"stylesheet\" href=\"" + httpPath().toUtf8() + "epg.css\" "
         "type=\"text/css\" media=\"screen, handheld, projection\" />";

  if (timeshiftEnabled)
  if (!timeshiftDir.exists())
  if (!timeshiftDir.mkpath(timeshiftDir.absolutePath()))
  {
    timeshiftEnabled = false;
    qWarning() << "Failed to create timeshift directory:" << timeshiftDir.absolutePath();
  }

  foreach (const SSystem::DeviceEntry &captureDevice, SSystem::availableVideoCaptureDevices())
  {
    STerminals::AudioVideoDevice * const dev = SSystem::createTerminal<STerminals::AudioVideoDevice>(this, captureDevice.url, false);
    if (dev)
    {
      bool found = false;
      foreach (const QString &input, dev->inputs())
      if (input.trimmed().toUpper().startsWith("TELEVISION"))
      if (dev->selectInput(input))
      {
        found = true;
        break;
      }

      if (found)
      if (dev->tuner() != NULL)
      {
        tuners[captureDevice.url] = new Tuner(dev);

        if (qobject_cast<SDigitalTuner *>(dev->tuner()))
          tunerPrio.insert(-1, captureDevice.url);
        else
          tunerPrio.insert(0, captureDevice.url);

        continue;
      }

      delete dev;
    }
  }

  if (hasTuners())
  {
    dlnaDir.setSubdirLimit(999);
    dlnaDir.sortOrder -= 10;

    foreach (const QString &group, settings.childGroups())
    if (group.startsWith(typeName(Type_Television) + "Channel_"))
    {
      settings.beginGroup(group);

      const QString name = settings.value("Name").toString();
      const unsigned preset = group.mid(8 + typeName(Type_Television).length()).toUInt();
      const QString title = ("00" + QString::number(preset)).right(3) + " " + name;
      dlnaDir.addDir(title, new ChannelDir(dlnaDir.server(), this, name));

      settings.endGroup();
    }

    connect(&checkScheduleTimer, SIGNAL(timeout()), SLOT(checkSchedule()));
    connect(&checkTeletextCapturesTimer, SIGNAL(timeout()), SLOT(checkTeletextCaptures()));
    connect(&checkDiskspaceTimer, SIGNAL(timeout()), SLOT(checkDiskspace()));

    checkTeletextCapturesTimer.start(180000);
    QTimer::singleShot(4000, this, SLOT(checkTeletextCaptures()));

    if (timeshiftEnabled)
    {
      checkScheduleTimer.start(60000);
      checkDiskspaceTimer.start(600000);
      checkDiskspace();
      updateRecordPlan();
    }
  }
}

TelevisionServer::~TelevisionServer()
{
  QThreadPool::globalInstance()->waitForDone();

  removeAllStreams();

  for (QMap<QString, CaptureStream *>::Iterator i = teletextCaptureStreams.begin();
       i != teletextCaptureStreams.end();
       i = teletextCaptureStreams.begin())
  {
    stopTeletextCapture(*i);
  }
  
  for (QMap<QString, CaptureStream *>::Iterator i = captureStreams.begin();
       i != captureStreams.end();
       i = captureStreams.begin())
  {
    stopCapture(*i);
  }
  
  foreach (Tuner *tuner, tuners)
    delete tuner;
}

QMap<QString, QSet<QString> > TelevisionServer::tunersByChannel(void) const
{
  PluginSettings settings(plugin);

  QMap<QString, QSet<QString> > result;

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);
    const QString rawChannel = SStringParser::toRawName(settings.value("Name").toString());

    foreach (const QString &group, settings.childGroups())
    if (group.startsWith("Device_"))
    {
      settings.beginGroup(group);

      const QString tunerName = settings.value("Name").toString();
      if (tuners.contains(tunerName))
        result[tunerName] += rawChannel;

      settings.endGroup();
    }

    settings.endGroup();
  }

  return result;
}

bool TelevisionServer::areSharingTransponder(const QString &ch1, const QString &ch2, const QString &tuner)
{
  PluginSettings settings(plugin);

  quint64 t1 = 0, t2 = 0;

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    quint64 *t = NULL;
    const QString channel = SStringParser::toRawName(settings.value("Name").toString());
    if (channel == ch1)
      t = &t1;
    else if (channel == ch2)
      t = &t2;

    if (t)
    foreach (const QString &group, settings.childGroups())
    if (group.startsWith("Device_"))
    {
      settings.beginGroup(group);

      if (settings.value("Name").toString() == tuner)
        *t = settings.value("Transponder").toULongLong();

      settings.endGroup();
    }

    settings.endGroup();
  }

  return (t1 == t2) && (t1 != 0);
}

bool TelevisionServer::willRecord(const QString &channelName, const QDateTime &begin, const QDateTime &end) const
{
  const QString rawChannel = SStringParser::toRawName(channelName);

  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  foreach (const Record &record, records)
  if ((record.rawChannel == rawChannel) && (record.begin <= begin) && (record.end >= end))
    return true;

  return false;
}

bool TelevisionServer::handleConnection(const QHttpRequestHeader &request, QAbstractSocket *socket)
{
  const QUrl url(request.path());
  const QString file = url.path().mid(url.path().lastIndexOf('/') + 1);

  if (url.hasQueryItem("record"))
  {
    foreach (const QString &item, url.queryItemValue("record").split(','))
    {
      QString rawChannel;
      QDateTime date;
      if (fromID(item, rawChannel, date))
      {
        const EpgDatabase::Programme p = epgDatabase->getProgramme(rawChannel, date);
        if (!p.utcDate.isNull())
          epgDatabase->recordProgramme(rawChannel, date, p.recordPriority ? 0 : 1);
      }
    }

    updateRecordPlan();
  }
  
  if (file.endsWith("-thumb.jpeg"))
  {
    const QImage image = getThumbnail(file.left(file.length() - 11));
    if (!image.isNull())
    {
      QByteArray jpgData;
      QBuffer buffer(&jpgData);
      buffer.open(QIODevice::WriteOnly);

      image.save(&buffer, "JPEG", 80);
      buffer.close();

      return sendReply(socket, buffer.data(), "image/jpeg", true);
    }

    QHttpResponseHeader response(301);
    response.setValue("Location", "http://" + request.value("Host") + "/img/null.png");
    socket->write(response.toString().toUtf8());
    return false;
  }
  else if (file.isEmpty() || file.endsWith(".html") || file.endsWith(".css"))
    return handleHtmlRequest(url, file, socket);

  return VideoServer::handleConnection(request, socket);
}

TelevisionServer::CaptureStream * TelevisionServer::startCapture(const QString &channelName, const QString &tunerName)
{
  const QString rawChannel = SStringParser::toRawName(channelName);

  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  // Stop any teletext captures on this channel.
  QMap<QString, CaptureStream *>::Iterator cap = teletextCaptureStreams.find(rawChannel);
  if (cap != teletextCaptureStreams.end())
    stopTeletextCapture(*cap);

  cap = captureStreams.find(rawChannel);
  if (cap == captureStreams.end())
  {
    CaptureStream * const stream = new CaptureStream();
    stream->refCount = 0;
    stream->graph = new SGraph(SGraph::MediaTask_Capture);
    stream->fileName =
        QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "@" +
        rawChannel.toLower() + ".lxm.mpeg";
    stream->rawChannel = rawChannel;
    stream->start = QDateTime::currentDateTime().toUTC();

    stream->sinkTerminal = stream->graph->createTerminal<STerminals::FileStream>(timeshiftDir.absoluteFilePath(stream->fileName));
    if (stream->sinkTerminal)
    {
      QFile::setPermissions(timeshiftDir.absoluteFilePath(stream->fileName),
                            QFile::ReadOwner | QFile::WriteOwner |
                            QFile::ReadGroup | QFile::WriteGroup |
                            QFile::ReadOther | QFile::WriteOther);

      const QPair<Tuner *, quint64> tuner = findAvailableTuner(rawChannel, true, tunerName);

      stream->sinkNode = stream->graph->openStream(stream->sinkTerminal, stream->sinkTerminal->outputStream(0));
      stream->tuner = tuner.first;
      if (stream->tuner)
      {
        const STerminal::Stream inputStream = stream->tuner->terminal->inputStream(tuner.second);
        if ((inputStream.serviceID == tuner.second) || !inputStream.name.isEmpty())
        {
          // The encoders will do nothing if the input stream is already encoded.
          stream->tuner->terminal->setAudioEnabled(true);
          stream->tuner->terminal->setVideoEnabled(true);
          stream->captureNode = stream->graph->openStream(stream->tuner->terminal, inputStream);

          stream->graph->registerNode(stream->audioEncoder = new SAudioEncoderNode(SBufferEncoder::Flag_Fast));
          stream->wideScreenDetect = stream->graph->createNode<SNodes::Video::WideScreenDetect>();
          stream->analogVideoFilter = stream->graph->createNode<SNodes::Video::AnalogVideoFilter>();
          stream->graph->registerNode(stream->videoEncoder = new SVideoEncoderNode(SBufferEncoder::Flag_Fast));

          stream->teletextNode = teletextServer->createTeletextNode(stream->rawChannel, stream->graph);
          stream->graph->registerNode(stream->teletextNode);

          stream->graph->connectNodes(stream->captureNode, stream->audioEncoder);
          stream->graph->connectNodes(stream->captureNode, stream->wideScreenDetect);
          stream->graph->connectNodes(stream->captureNode, stream->teletextNode);
          stream->graph->connectNodes(stream->wideScreenDetect, stream->analogVideoFilter);
          stream->graph->connectNodes(stream->analogVideoFilter, stream->videoEncoder);
          stream->graph->connectNodes(stream->audioEncoder, stream->sinkNode);
          stream->graph->connectNodes(stream->videoEncoder, stream->sinkNode);

          if (stream->graph->prepare())
          {
            stream->graph->start();

            qDebug() << "Started capture stream" << stream->fileName;

            stream->tuner->refCount++;
            cap = captureStreams.insert(rawChannel, stream);

            epgDatabase->storeRecord(stream->rawChannel,
                                     stream->fileName,
                                     stream->start,
                                     stream->start.addSecs(120));

            return stream;
          }
        }
      }
    }

    delete stream->graph; // Will also delete any objects created by the graph.
    delete stream;
  }
  else
    return *cap;

  return NULL;
}

void TelevisionServer::stopCapture(CaptureStream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  stream->graph->stop();
  stream->graph->unprepare();
  delete stream->graph; // Will also delete any objects created by the graph.

  qDebug() << "Stopped capture stream" << stream->fileName;

  for (QMap<QString, CaptureStream *>::Iterator i = captureStreams.begin();
       i != captureStreams.end(); )
  if (*i == stream)
    i = captureStreams.erase(i);
  else
    i++;

  const QDateTime now = QDateTime::currentDateTime().toUTC();
  if (stream->start.secsTo(now) >= (5 * 60))
  {
    epgDatabase->storeRecord(stream->rawChannel,
                             stream->fileName,
                             stream->start,
                             now);
  }
  else // Record is too short; delete the file.
  {
    timeshiftDir.remove(stream->fileName);
  }

  if (--stream->tuner->refCount <= 0)
    stream->tuner->transponder = 0;

  delete stream;
}


TelevisionServer::CaptureStream * TelevisionServer::startTeletextCapture(const QString &channelName)
{
  const QString rawChannel = SStringParser::toRawName(channelName);

  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  QMap<QString, CaptureStream *>::Iterator cap = captureStreams.find(rawChannel);
  if (cap == captureStreams.end())
    cap = teletextCaptureStreams.find(rawChannel);

  if (cap == teletextCaptureStreams.end())
  {
    CaptureStream * const stream = new CaptureStream();
    stream->refCount = 0;
    stream->graph = new SGraph(SGraph::MediaTask_None);
    stream->fileName = QString::null;
    stream->rawChannel = rawChannel;
    stream->start = QDateTime::currentDateTime().toUTC();

    const QPair<Tuner *, quint64> tuner = findAvailableTuner(rawChannel, false);

    stream->sinkTerminal = NULL;
    stream->sinkNode = NULL;
    stream->tuner = tuner.first;
    if (stream->tuner)
    {
      const STerminal::Stream inputStream = stream->tuner->terminal->inputStream(tuner.second);
      if ((inputStream.serviceID == tuner.second) || !inputStream.name.isEmpty())
      {
        stream->tuner->terminal->setAudioEnabled(false);
        stream->tuner->terminal->setVideoEnabled(true);
        stream->captureNode = stream->graph->openStream(stream->tuner->terminal, inputStream);
        stream->audioEncoder = NULL;
        stream->wideScreenDetect = NULL;
        stream->analogVideoFilter = NULL;
        stream->videoEncoder = NULL;
        stream->teletextNode = teletextServer->createTeletextNode(stream->rawChannel, stream->graph);
        stream->graph->registerNode(stream->teletextNode);
        stream->graph->connectNodes(stream->captureNode, stream->teletextNode);

        if (stream->graph->prepare())
        {
          stream->graph->start();

          qDebug() << "Started teletext capture stream for" << stream->rawChannel;

          stream->tuner->refCount++;
          cap = teletextCaptureStreams.insert(rawChannel, stream);
          return stream;
        }
      }
    }

    delete stream->graph; // Will also delete any objects created by the graph.
    delete stream;
  }
  else
    return *cap;

  return NULL;
}

void TelevisionServer::stopTeletextCapture(CaptureStream *stream)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  stream->graph->stop();
  stream->graph->unprepare();
  delete stream->graph; // Will also delete any objects created by the graph.

  qDebug() << "Stopped teletext capture stream for" << stream->rawChannel;

  for (QMap<QString, CaptureStream *>::Iterator i = teletextCaptureStreams.begin();
       i != teletextCaptureStreams.end(); )
  if (*i == stream)
    i = teletextCaptureStreams.erase(i);
  else
    i++;

  if (--stream->tuner->refCount <= 0)
    stream->tuner->transponder = 0;

  delete stream;
}

QPair<TelevisionServer::Tuner *, quint64> TelevisionServer::findAvailableTuner(const QString &rawChannel, bool stoptt, const QString &tunerName)
{
  PluginSettings settings(plugin);

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    if (SStringParser::toRawName(settings.value("Name").toString()) == rawChannel)
    {
      foreach (const QString &device, tunerPrio)
      {
        const QString group = "Device_" + SStringParser::toRawName(device);

        if (settings.childGroups().contains(group))
        if (group.startsWith("Device_"))
        {
          settings.beginGroup(group);

          if (settings.value("Use", true).toBool())
          {
            const QString device = settings.value("Name").toString();
            const quint64 transponder = settings.value("Transponder").toULongLong();
            const quint64 serviceID = settings.value("ServiceID").toULongLong();

            if ((tunerName == QString::null) || (device == tunerName))
            if (tuners.contains(device))
            {
              TelevisionServer::Tuner * const tuner = tuners[device];

              // Stop any teletext captures on this tuner if they are blocking the
              // requested channel
              if (stoptt)
              {
                if (tuner->transponder != transponder)
                for (QMap<QString, CaptureStream *>::Iterator i = teletextCaptureStreams.begin();
                     i != teletextCaptureStreams.end(); )
                if ((*i)->tuner == tuner)
                {
                  stopTeletextCapture(*i);
                  i = teletextCaptureStreams.begin();
                }
                else
                  i++;
              }

              // Return the tuner
              if (tuner->transponder == 0)
              {
                STuner * const stuner = tuner->terminal->tuner();
                if (stuner)
                  stuner->setFrequency(transponder);

                tuner->transponder = transponder;

                return QPair<Tuner *, quint64>(tuner, serviceID);
              }
              else if (tuner->transponder == transponder)
                return QPair<Tuner *, quint64>(tuner, serviceID);
            }
          }

          settings.endGroup();
        }
      }

      return QPair<Tuner *, quint64>(NULL, 0);
    }

    settings.endGroup();
  }

  return QPair<Tuner *, quint64>(NULL, 0);
}

qint64 TelevisionServer::getFreeDiskSpace(void)
{
  if (timeshiftDir.exists())
  {
    #if defined(Q_OS_UNIX)
      struct statfs fsinfo;
      if (statfs(timeshiftDir.absolutePath().toUtf8().data(), &fsinfo) == 0)
        return qint64(fsinfo.f_bavail) * qint64(fsinfo.f_bsize);
    #elif defined(Q_OS_WIN32)
      ULARGE_INTEGER freeBytesAvailable;
      ULARGE_INTEGER totalNumberOfBytes;
      ULARGE_INTEGER totalNumberOfFreeBytes;

      if (::GetDiskFreeSpaceEx((const WCHAR *)timeshiftDir.absolutePath().utf16(),
                               &freeBytesAvailable,
                               &totalNumberOfBytes,
                               &totalNumberOfFreeBytes) == TRUE)
      {
        return qint64(freeBytesAvailable.QuadPart);
      }
    #endif
  }

  return -1;
}

void TelevisionServer::updateRecordPlanTask(void)
{
  SDebug::Trace t("TelevisionServer::updateRecordPlanTask");

  const QDateTime begin = QDateTime::currentDateTime().toUTC().addSecs(-15 * 60);
  const QDateTime end = begin.addDays(1);

  // Get the programmes that were manually scheduled to record.
  QMultiMap<qreal, EpgDatabase::Programme> recordProgrammes;
  foreach (const EpgDatabase::Programme &programme, epgDatabase->getProgrammesToRecord(begin, end))
    recordProgrammes.insert(-programme.recordPriority * manualPriorityBoost, programme);

  /* Add automatically selected programmes
  QMap<QString, int> popularProgrammes;
  foreach (const EpgDatabase::Programme &programme, epgDatabase->getViewedProgrammes(begin.addDays(-EpgDatabase::daysInHistory), begin))
  {
    const QString rawName = SStringParser::toRawName(programme.name);
    QMap<QString, int>::Iterator i = popularProgrammes.find(rawName);
    if (i == popularProgrammes.end())
      i = popularProgrammes.insert(rawName, 0);

    (*i) += 1;
  }

  for (QMap<QString, int>::ConstIterator i=popularProgrammes.begin(); i!=popularProgrammes.end(); i++)
  foreach (const EpgDatabase::Programme &programme, epgDatabase->getProgrammesByName(i.key(), begin, end))
    recordProgrammes.insert(-(*i), programme);*/

  // Build an ordered list of records
  QMultiMap<qreal, Record> allRecords;
  for (QMultiMap<qreal, EpgDatabase::Programme>::ConstIterator programme = recordProgrammes.begin();
       programme != recordProgrammes.end();
       programme++)
  {
    const EpgDatabase::Programme nextProgramme = epgDatabase->getNextProgramme(programme->channelName, programme->utcDate);
    if (nextProgramme.utcDate > programme->utcDate)
    {
      Record record;
      record.programmeName = programme->name;
      record.rawChannel = SStringParser::toRawName(programme->channelName);
      record.begin = programme->utcDate.addSecs(-startEarlierSecs);
      record.end = nextProgramme.utcDate.addSecs(stopLaterSecs);
      record.priority = programme.key();
      record.isScheduled = true;

      allRecords.insert(record.priority, record);
    }
  }

  // Copy already running records
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  foreach (Record record, records)
  if (record.captureStream)
  {
    record.priority = manualPriorityBoost * manualPriorityBoost;
    allRecords.insert(record.priority, record);
  }

  // Build a record plan for each tuner
  const QMap<QString, QSet<QString> > tuners = tunersByChannel();
  QMap<QString, QMultiMap<qreal, Record> > recordsByTuner;
  foreach (const QString &tuner, tuners.keys())
  {
    QMap<QString, QMultiMap<qreal, Record> >::Iterator records = recordsByTuner.find(tuner);
    if (records == recordsByTuner.end())
      records = recordsByTuner.insert(tuner, QMultiMap<qreal, Record>());

    // Add all records that can be executed on this tuner
    for (QMultiMap<qreal, Record>::Iterator i=allRecords.begin(); i!=allRecords.end(); )
    if ((i->tunerName == tuner) ||
        (i->tunerName.isEmpty() && tuners[tuner].contains(i->rawChannel)))
    {
      i->tunerName = tuner;
      records->insert(i->priority, *i);
      i = allRecords.erase(i);
    }
    else
      i++;

    // Remove overlapping Programmes with a lower priority
    for (QMultiMap<qreal, Record>::Iterator i=records->begin(); i!=records->end(); i++)
    if (i->isScheduled)
    for (QMultiMap<qreal, Record>::Iterator j=i+1; j!=records->end(); j++)
    if (j->isScheduled)
    if ((i->begin < j->end) && (i->end > j->begin))
    if (i->rawChannel != j->rawChannel) // Programmes on the same channel can't overlap
    if (!areSharingTransponder(i->rawChannel, j->rawChannel, tuner))
    {
//       std::cout << "Not recording: \"" << j->ProgrammeName.toAscii().data()
//                 << "\" (" << j->channelName.toAscii().data()
//                 << " @ " << j->start.toString(Qt::ISODate).toAscii().data()
//                 << ") because it overlaps with \"" << i->ProgrammeName.toAscii().data()
//                 << "\" (" << i->channelName.toAscii().data()
//                 << " @ " << i->start.toString(Qt::ISODate).toAscii().data()
//                 << ")"
//                 << std::endl;

      j->isScheduled = false;
    }

    // Move unscheduled records to allRecords
    for (QMultiMap<qreal, Record>::Iterator i=records->begin(); i!=records->end(); )
    if (!i->isScheduled)
    {
      allRecords.insert(i->priority, *i);
      i = records->erase(i);
    }
    else
      i++;
  }

  // Merge overlapping Programmes on the same channel
  records.clear();
  for (QMap<QString, QMultiMap<qreal, Record> >::Iterator i=recordsByTuner.begin(); i!=recordsByTuner.end(); i++)
  for (QMultiMap<qreal, Record>::Iterator j=i->begin(); j!=i->end(); j++)
  if (j->isScheduled)
  {
    for (QMultiMap<qreal, Record>::Iterator k=j+1; k!=i->end(); k++)
    if (k->isScheduled)
    if ((j->begin < k->end) && (j->end > k->begin))
    if (j->rawChannel == k->rawChannel) // Merge records
    {
      // Merge with j
      if (k->begin < j->begin)
        j->begin = k->begin;

      if (k->end > j->end)
        j->end = k->end;

//       std::cout << "Merging: \"" << k->ProgrammeName.toAscii().data()
//                 << "\" (" << k->channelName.toAscii().data()
//                 << " @ " << k->start.toString(Qt::ISODate).toAscii().data()
//                 << ") with \"" << j->ProgrammeName.toAscii().data()
//                 << "\""
//                 << std::endl;

      k->isScheduled = false; // Merged.
    }

    // Add the record
    if (j->isScheduled)
      records.insert(j->begin, *j);
  }

  foreach (const Record &record, records)
  {
    qDebug() << "Scheduled record" << record.programmeName
             << "at" << record.rawChannel
             << record.begin.toString(Qt::ISODate)
             << " - " << record.end.toString(Qt::ISODate);
  }

  buildingPlan = false;
}

QDateTime TelevisionServer::roundTime(const QDateTime &date)
{
  QDateTime rounded = date;
  QTime time = rounded.time();
  time.setHMS(time.hour(), (time.minute() / 30) * 30, 0, 0);
  rounded.setTime(time);

  return rounded;
}

QString TelevisionServer::toID(const QString &channelName, const QDateTime &utcDate, const QTime &time)
{
  if (utcDate.isValid())
    return SStringParser::toRawName(channelName) +
           time.toString("hhmm") +
           utcDate.toString("yyyyMMddhhmmss");
  else
    return SStringParser::toRawName(channelName) +
           time.toString("hhmm") +
           "00000000000000";
}

bool TelevisionServer::fromID(const QString &idr, QString &rawChannel, QDateTime &utcDate)
{
  const QString id = idr.trimmed();

  if (id.length() > 18)
  {
    rawChannel = id.left(id.length() - 18);
    utcDate = QDateTime::fromString(id.right(14), "yyyyMMddhhmmss");
    utcDate.setTimeSpec(Qt::UTC);

    return true;
  }

  return false;
}

void TelevisionServer::updateRecordPlan(void)
{
  SDebug::Trace t("TelevisionServer::updateRecordPlan");
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  if (!buildingPlan)
  {
    buildingPlan = true;
    QtConcurrent::run(this, &TelevisionServer::updateRecordPlanTask);
  }
}

void TelevisionServer::checkSchedule(void)
{
  SDebug::Trace t("TelevisionServer::checkSchedule");
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  const QDateTime now = QDateTime::currentDateTime().toUTC();

  // Stop finished records
  for (QMultiMap<QDateTime, Record>::Iterator i=records.begin(); i!=records.end(); )
  if (i->end < now)
  {
    if (i->captureStream)
    {
      qDebug() << "Stopped record of" << i->programmeName << "at" << i->rawChannel;

      if (--(i->captureStream->refCount) == 0)
        stopCapture(i->captureStream);
    }

    i = records.erase(i);
  }
  else
    i++;

  // Start new records
  for (QMultiMap<QDateTime, Record>::Iterator i=records.begin(); i!=records.end(); i++)
  if ((i->begin <= now) && (i->captureStream == NULL))
  {
    i->captureStream = startCapture(i->rawChannel, i->tunerName);

    if (i->captureStream)
    {
      i->captureStream->refCount++;

      qDebug() << "Started record of" << i->programmeName << "at" << i->rawChannel;
    }
    else
      qDebug() << "Failed starting record of" << i->programmeName << "at" << i->rawChannel;
  }
}

void TelevisionServer::checkTeletextCaptures(void)
{
  SDebug::Trace t("TelevisionServer::checkTeletextCaptures");
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  // Stop all running captures
  QSet<QString> stopped;
  for (QMap<QString, CaptureStream *>::Iterator i = teletextCaptureStreams.begin();
       i != teletextCaptureStreams.end();
       i = teletextCaptureStreams.begin())
  {
    // Stored because the teletext data is committed in the background and
    // needsUpdate() may still return true for a few seconds.
    stopped.insert(i.key());

    stopTeletextCapture(*i);
  }

  // First start EPG captures.
  bool started = false;
  foreach (const QString &channelName, epgDatabase->getChannels())
  if (!stopped.contains(SStringParser::toRawName(channelName)) &&
      teletextServer->needsUpdate(channelName))
  {
    startTeletextCapture(channelName);
    started = true;
  }

  if (!started && !stopped.isEmpty())
    updateRecordPlan();
}

void TelevisionServer::checkDiskspace(void)
{
  SDebug::Trace t("TelevisionServer::checkDiskspace");
  const QDateTime now = QDateTime::currentDateTime();

  // First remove all recordings over a week old.
  foreach (const QFileInfo &info, timeshiftDir.entryInfoList(QDir::Files, QDir::Name))
  if (info.created().daysTo(now) > 7)
    timeshiftDir.remove(info.fileName());

  // If there is still not enough disk space; delete some more.
  qint64 availableSpace = getFreeDiskSpace();

  if ((availableSpace >= 0) && (availableSpace < reservedSpace))
  {
    SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

    foreach (const QFileInfo &info, timeshiftDir.entryInfoList(QDir::Files, QDir::Name))
    {
      bool found = false;
      foreach (CaptureStream *stream, captureStreams)
      if (stream->fileName == info.fileName())
      {
        found = true;
        break;
      }

      if (!found)
      {
        timeshiftDir.remove(info.fileName());
        availableSpace += info.size();
        if (availableSpace >= reservedSpace)
          break;
      }
    }
  }
}

bool TelevisionServer::streamVideo(const QHttpRequestHeader &request, QAbstractSocket *socket, const StreamRequest &r)
{
  QString rawChannel;
  QDateTime date;
  if (fromID(r.item, rawChannel, date) == false)
  {
    socket->write(QHttpResponseHeader(404).toString().toUtf8());
    return false;
  }

  StreamRequest streamRequest = r;
  streamRequest.item = "television:" + r.item;
  streamRequest.position = "0";
  streamRequest.channelSetup = SAudioCodec::Channel_Stereo;
  streamRequest.frameTime = STime::fromMSec(40);

  if (joinExistingStream(socket, streamRequest))
    return true; // The graph owns the socket now.

  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  const unsigned streamId = registerStreamId();
  if (streamId == 0)
  {
    socket->write(QHttpResponseHeader(503).toString().toUtf8());
    return false;
  }

  const bool ownClient = request.value("User-Agent").contains("LXiStream");
  const QUrl url = request.path();
  const bool fastTranscode = url.queryItemValue("transcode") != "slow";
  const QString fileName = QDir(url.path()).dirName();

  if (!date.isValid()) // Live capture
  {
    Stream * const stream = new Stream();
    stream->rawChannel = rawChannel;
    stream->start = QDateTime::currentDateTime().toUTC();
    stream->startPos = 0;
    stream->contentType = HttpServer::toMimeType(fileName);
    stream->request = streamRequest;
    stream->startPos = 0;
    stream->startTime = QDateTime::currentDateTime();

    stream->graph = new SGraph(SGraph::MediaTask_Playback);
    stream->httpTerminal = stream->graph->createTerminal<STerminals::HttpStream>("http:///" + fileName, false);

    if (stream->httpTerminal)
    {
      stream->httpNode = stream->graph->openStream(stream->httpTerminal, stream->httpTerminal->outputStream(0));

      QHttpResponseHeader header(200);
      header.setContentType(stream->contentType);
      header.setValue("Cache-Control", "no-cache");
      header.setValue("X-Identifier", QString::number(streamId));
      stream->httpNode->setProperty("httpHeader", header.toString().toUtf8());

      if (!ownClient)
      {
        stream->httpNode->setProperty("enablePrivateData", false);
        buildIntro(stream);
      }
      else
        stream->httpNode->setProperty("enablePrivateData", true);

      stream->httpNode->invokeMethod("addSocket", Q_ARG(QAbstractSocket *, socket));

      stream->captureStream = startCapture(rawChannel);
      if (stream->captureStream)
      {
        stream->captureStream->refCount++;
        stream->inputTerminal = stream->captureStream->sinkTerminal;
        stream->inputNode = stream->graph->openStream(stream->inputTerminal, stream->inputTerminal->inputStream(0));
        stream->inputNode->setProperty("position", stream->inputNode->property("duration").toInt() - 10); // Seek to end
        stream->startPos = stream->inputNode->property("position").toInt();

        if (!ownClient)
          buildTranscodeGraph(stream, fastTranscode);
        else
          stream->graph->connectNodes(stream->inputNode, stream->httpNode);

        if (stream->graph->prepare())
        {
          stream->graph->start();

          addStream(stream, streamId);

          qDebug() << "Started video stream" << streamId << stream->captureStream->fileName;
          return true; // The graph owns the socket now.
        }
      }

      qWarning() << "Failed to start video stream for channel" << stream->rawChannel;

      delete stream->graph; // Will also delete any objects created by the graph.
      if (stream->captureStream)
      if (--stream->captureStream->refCount == 0)
        stopCapture(stream->captureStream);

      delete stream;

      return true; // The socket was already assigned to the graph
    }

    delete stream;
  }
  else // Playback
  {
    const EpgDatabase::Record record = epgDatabase->getRecord(rawChannel, date);
    if (timeshiftDir.exists(record.fileName))
    {
      Stream * const stream = new Stream();
      stream->rawChannel = rawChannel;
      stream->start = date;
      stream->captureStream = NULL;
      stream->contentType = HttpServer::toMimeType(fileName);
      stream->request = streamRequest;
      stream->startPos = stream->request.position.toInt();
      stream->startTime = QDateTime::currentDateTime();
      stream->graph = new SGraph(SGraph::MediaTask_None);
      stream->httpTerminal = stream->graph->createTerminal<STerminals::HttpStream>("http:///" + fileName, false);
      stream->inputTerminal = stream->graph->createTerminal<STerminals::FileStream>(timeshiftDir.absoluteFilePath(record.fileName), false);

      if (stream->httpTerminal && stream->inputTerminal)
      {
        stream->httpNode = stream->graph->openStream(stream->httpTerminal, stream->httpTerminal->outputStream(0));

        QHttpResponseHeader header(200);
        header.setContentType("video/mpeg");
        header.setValue("Cache-Control", "no-cache");
        header.setValue("X-Identifier", QString::number(streamId));
        stream->httpNode->setProperty("httpHeader", header.toString().toUtf8());

        if (!ownClient)
        {
          stream->httpNode->setProperty("enablePrivateData", false);
          buildIntro(stream);
        }
        else
          stream->httpNode->setProperty("enablePrivateData", true);

        stream->httpNode->invokeMethod("addSocket", Q_ARG(QAbstractSocket *, socket));

        stream->inputNode = stream->graph->openStream(stream->inputTerminal, stream->inputTerminal->inputStream(0));
        stream->inputNode->setProperty("position", (record.begin.secsTo(date) - 15) + stream->startPos);
        stream->startPos = stream->inputNode->property("position").toInt();

        if (!ownClient)
          buildTranscodeGraph(stream, fastTranscode);
        else
          stream->graph->connectNodes(stream->inputNode, stream->httpNode);

        if (stream->graph->prepare())
        {
          stream->graph->start();

          addStream(stream, streamId);

          qDebug() << "Started video stream" << streamId << record.fileName;
          return true; // The graph owns the socket now.
        }
      }

      qWarning() << "Failed to start video stream" << stream->captureStream->fileName;

      delete stream->graph; // Will also delete any objects created by the graph.
      delete stream;

      socket->write(QHttpResponseHeader(500).toString().toUtf8());
      return false;
    }
  }

  socket->write(QHttpResponseHeader(404).toString().toUtf8());
  return false;
}

void TelevisionServer::removeStream(VideoServer::Stream *s, unsigned streamId)
{
  SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

  Stream * const stream = static_cast<Stream *>(s);

  epgDatabase->storeView(stream->rawChannel,
                         stream->start,
                         stream->start.addSecs(qMax(0,
                             stream->inputNode->property("position").toInt() -
                             stream->startPos)));

  CaptureStream * const captureStream = static_cast<Stream *>(stream)->captureStream;

  VideoServer::removeStream(stream, streamId);

  if (captureStream)
  if (--captureStream->refCount == 0)
    stopCapture(captureStream);
}

BackendServer::SearchResultList TelevisionServer::search(const QStringList &query) const
{
  SearchResultList results;

  const QDateTime now = QDateTime::currentDateTime().toUTC();
  const QDateTime today = QDateTime(now.toLocalTime().addSecs(-4 * 60 * 60).date(), QTime(4, 0), Qt::LocalTime).toUTC();
  const QDateTime lastWeek = today.addDays(-7);
  const QDateTime nexWeek = today.addDays(7);

  foreach (const EpgDatabase::Programme &programme, epgDatabase->queryProgrammes(query, lastWeek, nexWeek))
  {
    const qreal match =
        qMin(SStringParser::computeMatch(SStringParser::toRawName(programme.name), query) +
             SStringParser::computeMatch(SStringParser::toRawName(programme.category), query) +
             SStringParser::computeMatch(SStringParser::toRawName(programme.description), query), 1.0) /
        sqrt(qreal(qMax(1, qAbs(programme.utcDate.daysTo(now)))));

    if (match >= minSearchRelevance)
    {
      SearchResult result;
      result.relevance = match;
      result.headline = programme.name + " [" + programme.channelName + " - " + programme.utcDate.toLocalTime().toString(searchDateTimeFormat) + "] (" + tr("TV programme") + ")";
      result.location = toID(programme.channelName, programme.utcDate, programme.stationDate.time()).toAscii() + ".html";
      result.text = programme.description;

      results += result;
    }
  }

  return results;
}

QImage TelevisionServer::getThumbnail(const QString &id)
{
  QImage image;
  QString rawChannel;
  QDateTime date;
  if (fromID(id, rawChannel, date))
  if (date.isValid())
  {
    const QDateTime now = QDateTime::currentDateTime().toUTC();
    const EpgDatabase::Programme programme = epgDatabase->getProgramme(rawChannel, date);
    const EpgDatabase::Programme nextProgramme = epgDatabase->getNextProgramme(rawChannel, programme.utcDate);

    if (programme.utcDate < now)
    {
      const QList<EpgDatabase::Record> records = epgDatabase->getRecords(rawChannel, programme.utcDate, nextProgramme.utcDate);
      foreach (const EpgDatabase::Record &record, records)
      if (timeshiftDir.exists(record.fileName))
      {
        STerminals::FileStream * const file = SSystem::createTerminal<STerminals::FileStream>(NULL, timeshiftDir.absoluteFilePath(record.fileName));
        SNode * const inputNode = file->openStream(file->inputStream(0));
        inputNode->setProperty("position", record.begin.secsTo(date) + 120);

        if (inputNode->prepare(SCodecList()))
        {
          SVideoDecoderNode decoder;
          if (decoder.prepare())
          {
            int maxIter = 256;
            SBufferList buffers;
            while ((inputNode->processBuffer(SBuffer(), buffers) == SNode::Result_Active) && (--maxIter > 0) && image.isNull())
            {
              foreach (const SBuffer &buffer, buffers)
              if ((buffer.typeId() & SBuffer::dataTypeIdMask) == (SVideoBuffer::baseTypeId & SBuffer::dataTypeIdMask))
              {
                SBufferList decoded;
                if (decoder.processBuffer(buffer, decoded) == SNode::Result_Active)
                foreach (const LXiStreamGui::SImageBuffer &imageBuffer, decoded)
                if (!imageBuffer.isNull())
                {
                  image = imageBuffer.toImage();
                  break;
                }

                if (!image.isNull())
                  break;
              }
            }

            decoder.unprepare();
          }

          inputNode->unprepare();
        }

        delete file;
        if (!image.isNull())
          break;
      }
    }
  }

  return image;
}


TelevisionServer::ChannelDir::ChannelDir(DlnaServer *server, TelevisionServer *parent, const QString &name)
    : DlnaServerDir(server),
      parent(parent),
      name(name)
{
}

/*! Lists all recorded programmes of the last week.
 */
const FileServerDir::DirMap & TelevisionServer::ChannelDir::listDirs(void)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  if (lastDirsUpdate.isNull() || (qAbs(lastDirsUpdate.elapsed()) >= 60000))
  {
    const QDateTime now = QDateTime::currentDateTime().toUTC();
    const QDateTime start = QDateTime(now.toLocalTime().addSecs(-4 * 60 * 60).addDays(-6).date(), QTime(4, 0), Qt::LocalTime).toUTC();
    const QDateTime stop = start.addDays(7);

    int sortOrder = 1;
    QList<EpgDatabase::Programme> programmes = parent->epgDatabase->getProgrammes(name, start, stop);
    while (!programmes.isEmpty())
    {
      const EpgDatabase::Programme programme = programmes.takeFirst();
      const QString title = programme.utcDate.toLocalTime().toString("ddd hh:mm") + " " + programme.name;

      DlnaServerDir * programmeDir = static_cast<DlnaServerDir *>(findDir(title));
      if (programmeDir == NULL)
      {
        if (!programmes.isEmpty())
        {
          const QList<EpgDatabase::Record> records = parent->epgDatabase->getRecords(name, programme.utcDate, programmes.first().utcDate);
          if (!records.isEmpty())
          {
            bool hasAllFiles = true;
            QDateTime begin = records.first().begin, end = records.first().end;
            foreach (const EpgDatabase::Record &record, records)
            {
              if (record.begin < begin)
                begin = record.begin;

              if (record.end > end)
                end = record.end;

              hasAllFiles &= parent->timeshiftDir.exists(record.fileName);
            }

            if (begin < programme.utcDate)
              begin = programme.utcDate;

            if ((end >= programmes.first().utcDate) && hasAllFiles) // Fully recorded
            {
              addDir(title, programmeDir = new DlnaServerDir(server()));

              static const int seekBy = 120;
              const STime duration = STime::fromSec(begin.secsTo(programmes.first().utcDate));
              const QString url = parent->httpPath() + toID(rawChannel, begin, programme.stationDate.time()) + ".mpeg";

              for (int i=0, n=duration.toSec(); i<n; i+=seekBy)
              {
                DlnaServer::File file(server());
                file.mimeType = "video/mpeg";
                file.description = programme.description;
                file.duration = duration - STime::fromSec(i);
                file.url = (i != 0) ? (url + "&position=" + QString::number(i)) : url;
                programmeDir->addFile(tr("Play from") + " " + begin.addSecs(i).toLocalTime().toString("hh:mm"), file);
              }
            }
          }
        }
      }

      if (programmeDir)
        programmeDir->sortOrder = quint32(DlnaServer::defaultSortOrder + sortOrder++);
    }

    lastDirsUpdate.start();
  }

  return DlnaServerDir::listDirs();
}

/*! Lists now running and next two programmes.
 */
const DlnaServerDir::FileMap & TelevisionServer::ChannelDir::listFiles(void)
{
  SDebug::MutexLocker l(&server()->mutex, __FILE__, __LINE__);

  if (lastFilesUpdate.isNull() || (qAbs(lastFilesUpdate.elapsed()) >= 60000))
  {
    const QDateTime now = QDateTime::currentDateTime().toUTC();
    const QDateTime start = QDateTime(now.toLocalTime().addSecs(-4 * 60 * 60).date(), QTime(4, 0), Qt::LocalTime).toUTC();
    const QDateTime stop = start.addDays(1);

    clear();

    bool hasOnAir = false;
    int next = 3;
    int sortOrder = -1000;
    QList<EpgDatabase::Programme> programmes = parent->epgDatabase->getProgrammes(name, start, stop);
    while (!programmes.isEmpty())
    {
      const EpgDatabase::Programme programme = programmes.takeFirst();

      const STime duration = programmes.isEmpty()
                             ? STime::fromSec(2 * 60 * 60)
                             : STime::fromSec(programme.utcDate.secsTo(programmes.first().utcDate));


      if ((programme.utcDate <= now) && (programme.utcDate.addSecs(duration.toSec()) > now))
      {
        if (!parent->epgDatabase->getActiveRecord(name, programme.utcDate).fileName.isEmpty())
        {
          DlnaServer::File file(server());
          file.sortOrder = quint32(DlnaServer::defaultSortOrder + sortOrder++);
          file.mimeType = "video/mpeg";
          file.description = programme.description;
          file.duration = duration;
          file.url = parent->httpPath() + toID(rawChannel, programme.utcDate, programme.stationDate.time()) + ".mpeg";
          addFile(tr("Now") + " " + programme.utcDate.toLocalTime().toString("hh:mm") + " " + programme.name, file);
        }

        DlnaServer::File file(server());
        file.sortOrder = quint32(DlnaServer::defaultSortOrder + sortOrder++);
        file.mimeType = "video/mpeg";
        file.description = programme.description;
        file.duration = duration;
        file.url = parent->httpPath() + toID(rawChannel, QDateTime(), programme.stationDate.time()) + ".mpeg";
        addFile(tr("Live") + " " + now.toLocalTime().toString("hh:mm") + " " + programme.name, file);

        hasOnAir = true;
      }
      else if ((programme.utcDate > now) && (next-- > 0))
      {
        DlnaServer::File file(server());
        file.sortOrder = quint32(DlnaServer::defaultSortOrder + sortOrder++);
        file.mimeType = "video/mpeg";
        file.description = programme.description;
        file.duration = duration;
        file.url = parent->httpPath() + toID(rawChannel, QDateTime(), programme.stationDate.time()) + ".mpeg";
        addFile(tr("Next") + " " + programme.utcDate.toLocalTime().toString("hh:mm") + " " + programme.name, file);
      }
    }

    if (!hasOnAir)
    { // No current programme has been found (EPG may be incomplete)
      const QDateTime ptime = QDateTime::currentDateTime();
      DlnaServer::File file(server());
      file.mimeType = "video/mpeg";
      file.duration = STime::fromSec(2 * 60 * 60);
      file.url = parent->httpPath() + toID(rawChannel, QDateTime(), ptime.time()) + ".mpeg";
      addFile(tr("Now") + ptime.toString("hh:mm") + " " + tr("Unknown"), file);
    }

    lastFilesUpdate.start();
  }

  return DlnaServerDir::listFiles();
}


QString typeName(Type type)
{
  switch (type)
  {
  default:
  case Type_Unknown:
    return QString::null;

  case Type_Television:
    return "TV";

  case Type_Radio:
    return "Radio";
  }
}

} // End of namespace
