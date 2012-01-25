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

#include "epgdatabase.h"
#include "televisionbackend.h"
#include "televisionserver.h"

namespace LXiMediaCenter {

EpgDatabase::EpgDatabase(TelevisionBackend *plugin)
            :QObject(plugin),
             plugin(plugin),
             extEpgTimer()
{
  // Create tables that don't exist
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlQuery query(Database::database());

  query.exec("CREATE TABLE IF NOT EXISTS TelevisionProgrammes ("
             "rawChannel     TEXT NOT NULL,"
             "utcDate        DATE NOT NULL,"
             "rawName        TEXT NOT NULL,"
             "name           TEXT NOT NULL,"
             "category       TEXT,"
             "description    TEXT,"
             "stationDate    DATE NOT NULL,"
             "imdbLink       TEXT,"
             "recordPriority INTEGER NOT NULL,"
             "PRIMARY KEY(rawChannel, utcDate),"
             "FOREIGN KEY(imdbLink) REFERENCES ImdbEntries(rawName) ON DELETE SET NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionProgrammes_rawChannel "
             "ON TelevisionProgrammes(rawChannel)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionProgrammes_utcDate "
             "ON TelevisionProgrammes(utcDate)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionProgrammes_rawName "
             "ON TelevisionProgrammes(rawName)");

  query.exec("CREATE TABLE IF NOT EXISTS TelevisionRecords ("
             "rawChannel     TEXT NOT NULL,"
             "file           TEXT NOT NULL,"
             "begin          DATE NOT NULL,"
             "end            DATE NOT NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionRecords_rawChannel "
             "ON TelevisionRecords(rawChannel)");

  query.exec("CREATE TABLE IF NOT EXISTS TelevisionViews ("
             "rawChannel     TEXT NOT NULL,"
             "begin          DATE NOT NULL,"
             "end            DATE NOT NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS TelevisionViews_rawChannel "
             "ON TelevisionViews(rawChannel)");

  connect(&extEpgTimer, SIGNAL(timeout()), SLOT(importExternalEpg()));
  connect(&cleanTimer, SIGNAL(timeout()), SLOT(cleanDatabase()));

  extEpgTimer.start(300000);
  QTimer::singleShot(3000, this, SLOT(importExternalEpg()));
  cleanTimer.start(3600000);
  cleanDatabase();
}

EpgDatabase::~EpgDatabase()
{
  QThreadPool::globalInstance()->waitForDone();
}

void EpgDatabase::importFile(const QString &fileName, bool remove)
{
  SDebug::Trace t("EpgDatabase::loadEpgFile");

  QFile file(fileName);
  if (file.open(QIODevice::ReadOnly))
  {
    importXml(file.readAll());

    if (remove)
      file.remove();
  }
}

void EpgDatabase::importXml(const QByteArray &data)
{
  QDomDocument doc("tv");
  doc.setContent(data);
  QDomElement root = doc.documentElement();

  const QStringList channels = getChannels();

  // This map contains the name translations from XMLTV names to local names.
  typedef struct QPair<QString, float> IdLink;
  QMap<QString, IdLink> channelLinks;

  for (QDomElement channelElement = root.firstChildElement("channel");
       !channelElement.isNull();
       channelElement = channelElement.nextSiblingElement("channel"))
  {
    const QString channelID = channelElement.attribute("id");
    QString bestName = QString::null;
    float bestMatch = 0.0f;

    // Find the best matching channel based on the display-name.
    for (QDomElement nameElement = channelElement.firstChildElement("display-name");
         !nameElement.isNull();
         nameElement = nameElement.nextSiblingElement("display-name"))
    {
      const QString displayName = nameElement.text();

      foreach (const QString &channel, channels)
      {
        const float match =
            SStringParser::computeBidirMatch(SStringParser::toRawName(channel),
                                             SStringParser::toRawName(displayName));

        if (match > bestMatch)
        {
          bestName = channel;
          bestMatch = match;
        }
      }
    }

    // Prevent matching two different channels to the same channel name.
    if ((bestName != QString::null) && (bestMatch > 0.0f))
    for (QMap<QString, IdLink>::iterator i=channelLinks.begin(); i!=channelLinks.end(); )
    if (i->first == bestName)
    {
      if (i->second > bestMatch) // Other match is better
      {
        bestMatch = 0.0f;
        break;
      }
      else
        i = channelLinks.erase(i);
    }
    else
      i++;

    // Assign the new link
    if ((bestName != QString::null) && (bestMatch > 0.0f))
    {
      channelLinks[channelID].first = bestName;
      channelLinks[channelID].second = bestMatch;

      QString offset = QString::null;
      QDomElement utcOffsetTag = channelElement.firstChildElement(QString(GlobalSettings::productAbbr()) + ":utc-offset");
      if (!utcOffsetTag.isNull())
        offset = utcOffsetTag.text();
    }
  }

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  Database::database().transaction();

  for (QDomElement programmeElement = root.firstChildElement("programme");
       !programmeElement.isNull();
       programmeElement = programmeElement.nextSiblingElement("programme"))
  {
    const QMap<QString, IdLink>::const_iterator channelLink =
        channelLinks.find(programmeElement.attribute("channel"));

    if (channelLink != channelLinks.end())
    {
      XmlTvProgramme programme;
      programme.fromXml(programmeElement);
      programme.channelName = channelLink->first;

      if (programme.utcDate.isValid())
        addProgramme(programme);
    }
  }

  Database::database().commit();
}

QStringList EpgDatabase::getChannels(void) const
{
  PluginSettings settings(plugin);

  QMap<unsigned, QString> result;

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    result.insert(group.mid(8 + typeName(Type_Television).length()).toUInt(),
                  settings.value("Name").toString());

    settings.endGroup();
  }

  return result.values();
}

QString EpgDatabase::getChannelName(const QString &rawName, QMap<QString, QString> *cache) const
{
  if (cache)
  {
    if (cache->isEmpty())
    foreach (const QString &name, getChannels())
      cache->insert(SStringParser::toRawName(name), name);

    QMap<QString, QString>::ConstIterator i = cache->find(rawName);
    if (i != cache->end())
      return *i;
  }
  else
  {
    foreach (const QString &name, getChannels())
    if (SStringParser::toRawName(name) == rawName)
      return name;
  }

  return QString::null;
}

QDateTime EpgDatabase::getLocalTime(const QString &channelName, const QDateTime &time) const
{
  PluginSettings settings(plugin);

  const QString rawChannel = SStringParser::toRawName(channelName);

  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    if (SStringParser::toRawName(settings.value("Name").toString()) == rawChannel)
    if (settings.contains("UTCOffset"))
      return time.addSecs(settings.value("UTCOffset").toInt());

    settings.endGroup();
  }

  return time;
}

void EpgDatabase::setLocalTime(const QString &channelName, const QTime &localTime)
{
  if (localTime.isValid())
  {
    const QDateTime now = QDateTime::currentDateTime().toUTC();
    const QString rawChannel = SStringParser::toRawName(channelName);

    int utcOffset = now.time().secsTo(localTime);
    if (utcOffset < -43800) // Deal with midnight wrap-around
      utcOffset += 86400;

    // This prevents jitter
    utcOffset = ((utcOffset + 30) / 60) * 60;

    PluginSettings settings(plugin);

    foreach (const QString &group, settings.childGroups())
    if (group.startsWith(typeName(Type_Television) + "Channel_"))
    {
      settings.beginGroup(group);

      if (SStringParser::toRawName(settings.value("Name").toString()) == rawChannel)
      {
        if (settings.contains("UTCOffset"))
        {
          if (qAbs(settings.value("UTCOffset").toInt() - utcOffset) > 60)
            settings.setValue("UTCOffset", utcOffset);
        }
        else
          settings.setValue("UTCOffset", utcOffset);
      }

      settings.endGroup();
    }
  }
}

EpgDatabase::Programme EpgDatabase::getProgramme(const QString &channelName, const QDateTime &time) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT name, category, description, utcDate, stationDate, recordPriority "
                "FROM TelevisionProgrammes WHERE rawChannel = :rawChannel "
                "AND utcDate <= :time AND utcDate > :tooOld "
                "ORDER BY utcDate DESC LIMIT 1");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, time.toUTC());
  query.bindValue(2, time.toUTC().addSecs(-(8 * 60 * 60)));
  query.exec();
  if (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = channelName;
    programme.utcDate = query.value(3).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(4).toDateTime();
    programme.recordPriority = query.value(5).toInt();

    return programme;
  }

  return Programme();
}

EpgDatabase::Programme EpgDatabase::getNextProgramme(const QString &channelName, const QDateTime &time) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT name, category, description, utcDate, stationDate, recordPriority "
                "FROM TelevisionProgrammes WHERE rawChannel = :rawChannel "
                "AND utcDate > :time ORDER BY utcDate ASC LIMIT 1");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, time.toUTC());
  query.exec();
  if (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = channelName;
    programme.utcDate = query.value(3).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(4).toDateTime();
    programme.recordPriority = query.value(5).toInt();

    return programme;
  }

  return Programme();
}

EpgDatabase::Programme EpgDatabase::getCurrentProgramme(const QString &channelName) const
{
  return getProgramme(channelName, QDateTime::currentDateTime().toUTC());
}

QList<EpgDatabase::Programme> EpgDatabase::getProgrammes(const QString &channelName, const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Programme> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT name, category, description, utcDate, stationDate, recordPriority "
                "FROM TelevisionProgrammes WHERE rawChannel = :rawChannel "
                "AND utcDate >= :begin AND utcDate < :end ORDER BY utcDate ASC");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, begin.toUTC());
  query.bindValue(2, end.toUTC());
  query.exec();
  while (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = channelName;
    programme.utcDate = query.value(3).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(4).toDateTime();
    programme.recordPriority = query.value(5).toInt();

    result << programme;
  }

  return result;
}

QList<EpgDatabase::Programme> EpgDatabase::queryProgrammes(const QStringList &q, const QDateTime &begin, const QDateTime &end) const
{
  QList<EpgDatabase::Programme> result;
  QMap<QString, QString> channelNames;

  QString qs1, qs2, qs3;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND name LIKE '%" + rawItem + "%'";
    qs2 += " AND category LIKE '%" + rawItem + "%'";
    qs3 += " AND description LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty() && !qs3.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.prepare("SELECT name, category, description, rawName, utcDate, stationDate, recordPriority "
                  "FROM TelevisionProgrammes WHERE utcDate >= :begin AND utcDate < :end AND ("
                  "(" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ") OR (" + qs3.mid(5) + ")"
                  ") ORDER BY utcDate ASC");
    query.bindValue(0, begin.toUTC());
    query.bindValue(1, end.toUTC());
    query.exec();
    while (query.next())
    {
      Programme programme;
      programme.name = query.value(0).toString();
      programme.category = query.value(1).toString();
      programme.description = query.value(2).toString();
      programme.channelName = getChannelName(query.value(3).toString(), &channelNames);
      programme.utcDate = query.value(4).toDateTime();
      programme.utcDate.setTimeSpec(Qt::UTC);
      programme.stationDate = query.value(5).toDateTime();
      programme.recordPriority = query.value(6).toInt();

      result << programme;
    }
  }

  return result;
}

/*! Gathers the EPG information from the specified teletext page.

    \param day  Value indicating the day this page represents. 0 means today, 1
                tomorrow, 2 day after tomorrow, etc. -1 means monday, -2
                tuesday, -3 wednesday, etc.
  */
void EpgDatabase::gatherEPG(const QString &channelName, const SDataBuffer::TeletextPage &page, int day)
{
  const QDateTime now = QDateTime::currentDateTime();
  const QTime localTime = page.localTime();
  if (localTime.isValid())
    setLocalTime(channelName, localTime);

  const QStringList lines = page.decodedLines();

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  Database::database().transaction();

  foreach (const QString &line, lines)
  {
    QStringList times = line.left(8).simplified().split(' ', QString::SkipEmptyParts);
    QStringList words = line.simplified().split(' ', QString::SkipEmptyParts);
    QString name = "";
    QTime time;

    if ((times.count() >= 1) && (words.count() >= 2))
    if ((time = isTime(times[0])).isValid())
    for (int i=1; i<words.count(); i++)
    {
      if ((!isPage(words[i])) && (!isTime(words[i]).isValid()))
        name += " " + words[i];
    }

    if (time.isValid() && (name.length() > 0))
    {
      QDate date = getLocalTime(channelName, now.toUTC()).date();
      if (day < 0)
      {
        const int today = date.dayOfWeek(); // Monday = 1, Tuesday = 2, etc.
        const int delta = (-day) - today;
        if (delta >= 0)
          date.addDays(delta);
        else
          date.addDays(delta + 7);
      }
      else
        date = date.addDays(day);

      if (time.hour() < 4)
        date = date.addDays(1);

      Programme programme;
      programme.name = filterName(name);
      programme.channelName = channelName;
      programme.stationDate = QDateTime(date, time);

      addProgramme(programme);
    }
  }

  Database::database().commit();
}

void EpgDatabase::addProgramme(Programme programme)
{
  PluginSettings settings(plugin);

  const QString rawChannel = SStringParser::toRawName(programme.channelName);

  // Get the UTC offset
  int utcOffset = 100000;
  foreach (const QString &group, settings.childGroups())
  if (group.startsWith(typeName(Type_Television) + "Channel_"))
  {
    settings.beginGroup(group);

    if (SStringParser::toRawName(settings.value("Name").toString()) == rawChannel)
    if (settings.contains("UTCOffset"))
      utcOffset = settings.value("UTCOffset").toInt();

    settings.endGroup();
  }

  if (programme.stationDate.isValid() && (programme.name.length() > 0))
  {
    if (!programme.utcDate.isValid())
    {
      if (utcOffset != 100000)
      {
        programme.utcDate = programme.stationDate.addSecs(-utcOffset);
        programme.utcDate.setTimeSpec(Qt::UTC);
      }
    }
    else if (utcOffset == 100000)
    {
      QDateTime ud = programme.utcDate; ud.setTimeSpec(Qt::UTC);
      QDateTime sd = programme.stationDate; sd.setTimeSpec(Qt::UTC);

      utcOffset = ud.secsTo(sd);
      setLocalTime(programme.channelName, QTime::currentTime().addSecs(utcOffset));
    }

    if (programme.utcDate.isValid())
    {
      SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

      // Ensure the utcOffset is correctly applied.
      programme.stationDate = programme.utcDate.addSecs(utcOffset);
      programme.stationDate.setTimeSpec(Qt::LocalTime); // The local time of the station

      qreal bestMatch = -1.0;
      Programme best;
      foreach (const Programme &p, getProgrammes(programme.channelName,
                                                 programme.utcDate.addSecs(-30 * 60),
                                                 programme.utcDate.addSecs(30 * 60)))
      {
        const qreal match = (p.utcDate == programme.utcDate) ? 1.0 :
            (SStringParser::computeBidirMatch(SStringParser::toRawName(p.name),
                                              SStringParser::toRawName(programme.name)) -
             (qreal(qAbs(p.utcDate.secsTo(programme.utcDate) / 60)) / 120.0));

        if (match > bestMatch)
        {
          bestMatch = match;
          best = p;
        }
      }

      if (bestMatch >= 0.3)
      { // Update existing programme

        // Choose the name with the least number of words (cleanest).
        if (SStringParser::numWords(best.name) < SStringParser::numWords(programme.name))
          programme.name = best.name;

        if (programme.category.length() < best.category.length())
          programme.category = best.category;

        if (programme.description.length() < best.description.length())
          programme.description = best.description;

        if (programme.recordPriority < best.recordPriority)
          programme.recordPriority = best.recordPriority;

        QSqlQuery query(Database::database());
        query.prepare("UPDATE OR REPLACE TelevisionProgrammes "
                      "SET utcDate = :utcDate, rawName = :rawName, "
                      "name = :name, category = :category, description = :description, "
                      "stationDate = :stationDate, imdbLink = NULL, "
                      "recordPriority = :recordPriority "
                      "WHERE rawChannel = :rawChannel AND utcDate = :oldDate");
        query.bindValue(0, programme.utcDate);
        query.bindValue(1, SStringParser::toRawName(programme.name));
        query.bindValue(2, programme.name);
        query.bindValue(3, programme.category);
        query.bindValue(4, programme.description);
        query.bindValue(5, programme.stationDate);
        query.bindValue(6, programme.recordPriority);
        query.bindValue(7, SStringParser::toRawName(programme.channelName));
        query.bindValue(8, best.utcDate);
        query.exec();

        return;
      }

      // New programme
      QSqlQuery query(Database::database());
      query.prepare("INSERT INTO TelevisionProgrammes VALUES ("
                    ":rawChannel, :utcDate, :rawName, :name, :category, :description, :stationDate, NULL, :recordPriority)");
      query.bindValue(0, SStringParser::toRawName(programme.channelName));
      query.bindValue(1, programme.utcDate);
      query.bindValue(2, SStringParser::toRawName(programme.name));
      query.bindValue(3, programme.name);
      query.bindValue(4, programme.category);
      query.bindValue(5, programme.description);
      query.bindValue(6, programme.stationDate);
      query.bindValue(7, programme.recordPriority);
      query.exec();
    }
  }
}

void EpgDatabase::recordProgramme(const QString &channelName, const QDateTime &date, int priority)
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("UPDATE TelevisionProgrammes "
                "SET recordPriority = :recordPriority "
                "WHERE rawChannel = :rawChannel AND utcDate = :utcDate");
  query.bindValue(0, priority);
  query.bindValue(1, SStringParser::toRawName(channelName));
  query.bindValue(2, date.toUTC());
  query.exec();
}

QList<EpgDatabase::Programme> EpgDatabase::getProgrammesToRecord(const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Programme> result;
  QMap<QString, QString> channelNames;

  QSqlQuery query(Database::database());
  query.prepare("SELECT name, category, description, rawChannel, utcDate, stationDate, recordPriority "
                "FROM TelevisionProgrammes WHERE recordPriority > 0 "
                "AND utcDate >= :begin AND utcDate < :end ORDER BY utcDate ASC");
  query.bindValue(0, begin.toUTC());
  query.bindValue(1, end.toUTC());
  query.exec();
  while (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = getChannelName(query.value(3).toString(), &channelNames);
    programme.utcDate = query.value(4).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(5).toDateTime();
    programme.recordPriority = query.value(6).toInt();

    result << programme;
  }

  return result;
}

QList<EpgDatabase::Programme> EpgDatabase::getProgrammesByName(const QString &programmeName, const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Programme> result;
  QMap<QString, QString> channelNames;

  QSqlQuery query(Database::database());
  query.prepare("SELECT name, category, description, rawChannel, utcDate, stationDate, recordPriority "
                "FROM TelevisionProgrammes WHERE utcDate >= :begin AND utcDate < :end "
                "AND rawName LIKE '%" + SStringParser::toRawName(programmeName) + "%' "
                "ORDER BY utcDate ASC");
  query.bindValue(0, begin.toUTC());
  query.bindValue(1, end.toUTC());
  query.exec();
  while (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = getChannelName(query.value(3).toString(), &channelNames);
    programme.utcDate = query.value(4).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(5).toDateTime();
    programme.recordPriority = query.value(6).toInt();

    result << programme;
  }

  return result;
}

EpgDatabase::Record EpgDatabase::getRecord(const QString &channelName, const QDateTime &time) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT file, begin, end FROM TelevisionRecords "
                "WHERE rawChannel = :rawChannel "
                "AND begin <= :time AND end > :time");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, time.toUTC());
  query.bindValue(2, time.toUTC());
  query.exec();
  if (query.next())
  {
    Record record;
    record.fileName = query.value(0).toString();
    record.begin = query.value(1).toDateTime();
    record.begin.setTimeSpec(Qt::UTC);
    record.end = query.value(2).toDateTime();
    record.end.setTimeSpec(Qt::UTC);

    return record;
  }

  return Record();
}

EpgDatabase::Record EpgDatabase::getActiveRecord(const QString &channelName, const QDateTime &time) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT file, begin, end FROM TelevisionRecords "
                "WHERE rawChannel = :rawChannel "
                "AND begin < :time AND end = begin");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, time.toUTC());
  query.exec();
  if (query.next())
  {
    Record record;
    record.fileName = query.value(0).toString();
    record.begin = query.value(1).toDateTime();
    record.begin.setTimeSpec(Qt::UTC);
    record.end = query.value(2).toDateTime();
    record.end.setTimeSpec(Qt::UTC);

    return record;
  }

  return Record();
}

QList<EpgDatabase::Record> EpgDatabase::getRecords(const QString &channelName, const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Record> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT file, begin, end FROM TelevisionRecords "
                "WHERE rawChannel = :rawChannel "
                "AND begin <= :end AND end > :begin ORDER BY begin ASC");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, end.toUTC());
  query.bindValue(2, begin.toUTC());
  query.exec();
  while (query.next())
  {
    Record record;
    record.fileName = query.value(0).toString();
    record.begin = query.value(1).toDateTime();
    record.begin.setTimeSpec(Qt::UTC);
    record.end = query.value(2).toDateTime();
    record.end.setTimeSpec(Qt::UTC);

    result << record;
  }

  return result;
}

void EpgDatabase::storeRecord(const QString &channelName, const QString &fileName, const QDateTime &begin, const QDateTime &end)
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("INSERT INTO TelevisionRecords VALUES ("
                ":rawChannel, :file, :begin, :end)");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, fileName);
  query.bindValue(2, begin.toUTC());
  query.bindValue(3, end.toUTC());
  query.exec();
}

QList<EpgDatabase::Programme> EpgDatabase::getRecordedProgrammes(const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Programme> result;
  QMap<QString, QString> channelNames;

  QSqlQuery query(Database::database());
  query.prepare("SELECT a.name, a.category, a.description, a.rawChannel, a.utcDate, a.stationDate, a.recordPriority "
                "FROM (SELECT * FROM TelevisionProgrammes WHERE utcDate >= :begin AND utcDate < :end) AS a "
                "CROSS JOIN (SELECT * FROM TelevisionRecords WHERE begin < :end AND end >= :begin) AS b "
                "ON a.rawChannel = b.rawChannel "
                "WHERE a.utcDate >= b.begin AND a.utcDate < b.end ORDER BY a.utcDate ASC");
  query.bindValue(0, begin.toUTC());
  query.bindValue(1, end.toUTC());
  query.exec();
  while (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = getChannelName(query.value(3).toString(), &channelNames);
    programme.utcDate = query.value(4).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(5).toDateTime();
    programme.recordPriority = query.value(6).toInt();

    result << programme;
  }

  return result;
}

EpgDatabase::View EpgDatabase::getView(const QString &channelName, const QDateTime &time) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT begin, end FROM TelevisionViews "
                "WHERE rawChannel = :rawChannel "
                "AND begin <= :time AND end > :time");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, time.toUTC());
  query.exec();
  if (query.next())
  {
    View view;
    view.begin = query.value(0).toDateTime();
    view.begin.setTimeSpec(Qt::UTC);
    view.end = query.value(1).toDateTime();
    view.end.setTimeSpec(Qt::UTC);

    return view;
  }

  return View();
}

QList<EpgDatabase::View> EpgDatabase::getViews(const QString &channelName, const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::View> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT begin, end FROM TelevisionViews "
                "WHERE rawChannel = :rawChannel "
                "AND begin <= :end AND end > :begin ORDER BY begin ASC");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, end.toUTC());
  query.bindValue(2, begin.toUTC());
  query.exec();
  while (query.next())
  {
    View view;
    view.begin = query.value(0).toDateTime();
    view.begin.setTimeSpec(Qt::UTC);
    view.end = query.value(1).toDateTime();
    view.end.setTimeSpec(Qt::UTC);

    result << view;
  }

  return result;
}

void EpgDatabase::storeView(const QString &channelName, const QDateTime &begin, const QDateTime &end)
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("INSERT INTO TelevisionViews VALUES ("
                ":rawChannel, :begin, :end)");
  query.bindValue(0, SStringParser::toRawName(channelName));
  query.bindValue(1, begin.toUTC());
  query.bindValue(2, end.toUTC());
  query.exec();
}

QList<EpgDatabase::Programme> EpgDatabase::getViewedProgrammes(const QDateTime &begin, const QDateTime &end) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<EpgDatabase::Programme> result;
  QMap<QString, QString> channelNames;

  QSqlQuery query(Database::database());
  query.prepare("SELECT a.name, a.category, a.description, a.rawChannel, a.utcDate, a.stationDate, a.recordPriority "
                "FROM (SELECT * FROM TelevisionProgrammes WHERE utcDate >= :begin AND utcDate < :end) AS a "
                "CROSS JOIN (SELECT * FROM TelevisionViews WHERE begin < :end AND end >= :begin) AS b "
                "ON a.rawChannel = b.rawChannel "
                "WHERE a.utcDate >= b.begin AND a.utcDate < b.end ORDER BY a.utcDate ASC");
  query.bindValue(0, begin.toUTC());
  query.bindValue(1, end.toUTC());
  query.bindValue(2, end.toUTC());
  query.bindValue(3, begin.toUTC());
  query.exec();
  while (query.next())
  {
    Programme programme;
    programme.name = query.value(0).toString();
    programme.category = query.value(1).toString();
    programme.description = query.value(2).toString();
    programme.channelName = getChannelName(query.value(3).toString(), &channelNames);
    programme.utcDate = query.value(4).toDateTime();
    programme.utcDate.setTimeSpec(Qt::UTC);
    programme.stationDate = query.value(5).toDateTime();
    programme.recordPriority = query.value(6).toInt();

    result << programme;
  }

  return result;
}

void EpgDatabase::importExternalEpg(void)
{
  SDebug::Trace t("EpgDatabase::importExternalEpg");

  QDir extEpgDir(GlobalSettings::applicationDataDir());

  foreach (const QFileInfo &info, extEpgDir.entryInfoList(QStringList("*.xmltv"), QDir::Files | QDir::Readable, QDir::Name))
  if (info.size() > 0)
    QtConcurrent::run(this, &EpgDatabase::importFile, info.absoluteFilePath(), true);
}

void EpgDatabase::cleanDatabase(void)
{
  SDebug::Trace t("EpgDatabase::cleanDatabase");

  const QDateTime oldDate = QDateTime::currentDateTime().addDays(-daysInHistory);

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlDatabase db = Database::database();
  db.transaction();

  QSqlQuery query(db);
  query.prepare("DELETE FROM TelevisionProgrammes WHERE utcDate < :oldDate");
  query.bindValue(0, oldDate);
  query.exec();

  query.prepare("DELETE FROM TelevisionRecords WHERE end < :oldDate");
  query.bindValue(0, oldDate);
  query.exec();

  query.prepare("DELETE FROM TelevisionViews WHERE end < :oldDate");
  query.bindValue(0, oldDate);
  query.exec();

  db.commit();
}

QDate EpgDatabase::dayFor(const QString &channelName, const QDateTime &utcDateTime) const
{
  return getLocalTime(channelName, utcDateTime).addSecs(-4 * 60 * 60).date();
}

QTime EpgDatabase::isTime(const QString &timeText)
{
  bool decodeOk = false;
  QString text = timeText;
  text.replace('.', ':');

  if (text.contains(':'))
  {
    if ((text.length() == 5) && text.at(0).isNumber() && text.at(1).isNumber() && text.at(3).isNumber() && text.at(4).isNumber())
      return QTime::fromString(text, "hh:mm");
    else if ((text.length() == 4) && text.at(0).isNumber() && text.at(2).isNumber() && text.at(3).isNumber())
      return QTime::fromString(text, "hh:mm");
  }
  else if ((text.length() == 4) && (text.toInt(&decodeOk) >= 0) && (text.toInt() <= 2400))
  if (decodeOk)
    return QTime::fromString(text.left(2) + ":" + text.right(2), "hh:mm");

  return QTime();
}

bool EpgDatabase::isPage(const QString &text)
{
  if ((text.length() == 3) && (text.toInt(NULL, 16) >= 0x100) && (text.toInt(NULL, 16) < 0x900))
    return true;

  return false;
}

QString EpgDatabase::filterName(QString name)
{
  for (int i=0; i<name.length(); i++)
  if (name[i] == '.')
    name[i] = ' ';

  return name.simplified();
}

QString EpgDatabase::toDateString(const QDateTime &localDate, const QDateTime &utcDate)
{
  QDateTime ld = localDate; ld.setTimeSpec(Qt::UTC);
  QDateTime ud = utcDate; ud.setTimeSpec(Qt::UTC);
  const int minsOffUTC = (ud.secsTo(ld) + 30) / 60; // +30 to correctly round.

  QString offset = minsOffUTC < 0 ? "-" : "+";
  offset += ("0" + QString::number(qAbs(minsOffUTC) / 60)).right(2);
  offset += ("0" + QString::number(qAbs(minsOffUTC) % 60)).right(2);

  return ld.toString("yyyyMMddhhmmss") + " " + offset;
}

void EpgDatabase::fromDateString(const QString &s, QDateTime &localDate, QDateTime &utcDate)
{
  const QString st = s.simplified();
  if (st.length() == 20)
  {
    localDate = QDateTime::fromString(st.left(14), "yyyyMMddhhmmss");
    localDate.setTimeSpec(Qt::LocalTime);

    const QString offset = st.right(5);
    const int minsOffUTC = ((offset.mid(1, 2).toInt() * 60) + offset.mid(3, 2).toInt());
    utcDate = localDate.addSecs(-minsOffUTC * (offset[0] == '-' ? -60 : 60));
    utcDate.setTimeSpec(Qt::UTC);
  }
  else
  {
    localDate = QDateTime();
    utcDate = QDateTime();
  }
}

QString EpgDatabase::toDateString(const QDateTime &utcDate)
{
  return utcDate.toString("yyyyMMddhhmmss") + " +0000";
}

QDateTime EpgDatabase::fromDateString(const QString &s)
{
  QDateTime localDate, utcDate;
  fromDateString(s, localDate, utcDate);

  return utcDate;
}


QDomNode EpgDatabase::XmlTvProgramme::toXml(QDomDocument &doc) const
{
  QDomElement prg = createElement(doc, "programme");

  addElement(doc, prg, "title", SStringParser::removeControl(name));

  const QString c = SStringParser::removeControl(category);
  if (!c.isEmpty())
    addElement(doc, prg, "category", c);

  const QString d = SStringParser::removeControl(description);
  if (!d.isEmpty())
    addElement(doc, prg, "desc", d);

  prg.setAttribute("start", toDateString(stationDate, utcDate));

  return prg;
}

void EpgDatabase::XmlTvProgramme::fromXml(const QDomNode &elm)
{
  QDomElement prg = findElement(elm, "programme");

  name = element(prg, "title");
  category = element(prg, "category");
  description = element(prg, "desc");

  fromDateString(prg.attribute("start"), stationDate, utcDate);
}

} // End of namespace
