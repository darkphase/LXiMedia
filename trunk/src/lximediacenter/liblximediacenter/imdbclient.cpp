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

#include "imdbclient.h"

#include <cstdio>
#include <QtSql>
#include "database.h"
#include "globalsettings.h"

// Prevents dependency with zlibg-dev (as Qt already provides zlib).
extern "C"
{
  typedef void * gzFile;
  gzFile gzdopen(int fd, const char *mode);
  int gzread(gzFile file, void *buf, unsigned len);
  int gzclose(gzFile file);
}

namespace LXiMediaCenter {

const char  * const ImdbClient::sentinelItem = "#SENTINEL";
const unsigned      ImdbClient::readChunkSize = 3000;
const int           ImdbClient::maxAge = 120;
const QEvent::Type  ImdbClient::tryMirrorEventType = QEvent::Type(QEvent::registerEventType());

const char * const ImdbClient::mirrors[] =
{
  "ftp://ftp.fu-berlin.de/pub/misc/movies/database/",
  "ftp://ftp.funet.fi/pub/mirrors/ftp.imdb.com/pub/",
  "ftp://ftp.sunet.se/pub/tv+movies/imdb/"
};

ImdbClient::ImdbClient(QObject *parent)
  : QObject(parent),
    mutex(sApp),
    cacheDir(GlobalSettings::applicationDataDir() + "/imdb"),
    moviesFile(),
    plotFile(),
    ratingFile(),
    downloading(0),
    available(0),
    useMirror(qrand() % (sizeof(mirrors) / sizeof(mirrors[0]))), useAttempt(0),
    failed(false),
    manager(new QNetworkAccessManager(this)),
    reply(NULL)
{
  GlobalSettings settings;

  static bool firsttime = true;
  if (firsttime)
  {
    firsttime = false;

    SDebug::_MutexLocker<SScheduler::Dependency> l(&mutex, __FILE__, __LINE__);
    
    if (!cacheDir.exists())
    if (!cacheDir.mkpath(cacheDir.absolutePath()))
    {
      qWarning() << "Failed to create IMDB cache directory:" << cacheDir.absolutePath();
      return;
    }

    SDebug::_MutexLocker<SScheduler::Dependency> dl(Database::mutex(), __FILE__, __LINE__);
    QSqlDatabase db = Database::database();
    QSqlQuery query(db);

    // Drop all tables if the database version is outdated.
    static const int imdbDatabaseVersion = 1;
    if (settings.value("ImdbDatabaseVersion", 0).toInt() != imdbDatabaseVersion)
    {
      qDebug() << "IMDB database layout changed, recreating tables. This may take several minutes ...";

      QStringList tables;
      query.exec("SELECT name FROM sqlite_master WHERE type='table'");
      while (query.next())
      {
        const QString name = query.value(0).toString();
        if (name.startsWith("Imdb"))
          tables += name;
      }

      QStringList indices;
      query.exec("SELECT name FROM sqlite_master WHERE type='index'");
      while (query.next())
      {
        const QString name = query.value(0).toString();
        if (name.startsWith("Imdb"))
          indices += name;
      }

      query.exec("PRAGMA foreign_keys = OFF");

      db.transaction();

      foreach (const QString &name, indices)
        query.exec("DROP INDEX IF EXISTS " + name);

      foreach (const QString &name, tables)
        query.exec("DROP TABLE IF EXISTS " + name);

      db.commit();

      query.exec("PRAGMA foreign_keys = ON");
    }

    // Create tables that don't exist
    query.exec("CREATE TABLE IF NOT EXISTS ImdbEntries ("
               "rawName        TEXT PRIMARY KEY NOT NULL,"
               "title          TEXT NOT NULL,"
               "type           INTEGER NOT NULL,"
               "year           INTEGER,"
               "episodeName    TEXT,"
               "episodeNumber  INTEGER,"
               "seasonNumber   INTEGER,"
               "plot           TEXT,"
               "rating         REAL)");

    query.exec("CREATE INDEX IF NOT EXISTS ImdbEntries_rawName "
               "ON ImdbEntries(rawName)");

    settings.setValue("ImdbDatabaseVersion", imdbDatabaseVersion);
  }

  moviesFile.setFileName(cacheDir.absoluteFilePath("movies.list"));
  plotFile.setFileName(cacheDir.absoluteFilePath("plot.list"));
  ratingFile.setFileName(cacheDir.absoluteFilePath("ratings.list"));

  if (!isAvailable() &&
      moviesFile.exists() && plotFile.exists() && ratingFile.exists() &&
      (moviesFile.size() > 0) && (plotFile.size() > 0) && (ratingFile.size() > 0))
  {
    importIMDBDatabase();
  }
}

ImdbClient::~ImdbClient(void)
{
  delete reply;
  delete manager;
}

void ImdbClient::obtainIMDBFiles(void)
{
  SDebug::_MutexLocker<SScheduler::Dependency> l(&mutex, __FILE__, __LINE__);

  if (moviesFile.open(QIODevice::ReadWrite) &&
      plotFile.open(QIODevice::ReadWrite) &&
      ratingFile.open(QIODevice::ReadWrite))
  {
    obtainFiles.clear();
    obtainFiles << &moviesFile << &plotFile << &ratingFile;

    QCoreApplication::postEvent(this, new QEvent(tryMirrorEventType));
  }
}

bool ImdbClient::isDownloading(void)
{
  SDebug::_MutexLocker<SScheduler::Dependency> l(&mutex, __FILE__, __LINE__);

  return downloading > 0;
}

bool ImdbClient::isAvailable(void)
{
  if (available == 0)
  {
    SDebug::_MutexLocker<SScheduler::Dependency> dl(Database::mutex(), __FILE__, __LINE__);

    // Check for the sentinel entry indicating the last import has completed.
    QSqlQuery query(Database::database());
    query.exec("SELECT title FROM ImdbEntries WHERE rawName = '#SENTINEL'");
    if (!query.next())
    {
      available = -1;
      return false;
    }

    available = 1;
    return true;
  }

  return available == 1;
}

bool ImdbClient::needUpdate(void)
{
  const QDateTime now = QDateTime::currentDateTime();

  if (moviesFile.exists() && plotFile.exists() && ratingFile.exists())
  {
    return ((QFileInfo(moviesFile).lastModified().daysTo(now) > maxAge) ||
            (QFileInfo(plotFile).lastModified().daysTo(now) > maxAge) ||
            (QFileInfo(ratingFile).lastModified().daysTo(now) > maxAge));
  }

  return true;
}

ImdbClient::Entry ImdbClient::readEntry(const QString &rawName)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  if (rawName != sentinelItem)
  {
    QSqlQuery query(Database::database());

    query.prepare("SELECT rawName, title, type, year, episodeName, "
                  "       episodeNumber, seasonNumber, plot, rating "
                  "FROM ImdbEntries WHERE rawName = :rawName");
    query.bindValue(0, rawName);
    query.exec();
    if (query.next())
    {
      Entry entry;
      entry.rawName = query.value(0).toString();
      entry.title = query.value(1).toString();
      entry.type = Type(query.value(2).toInt());
      entry.year = query.value(3).toInt();
      entry.episodeName = query.value(4).toString();
      entry.episodeNumber = query.value(5).toInt();
      entry.seasonNumber = query.value(6).toInt();
      entry.plot = query.value(7).toString();
      entry.rating = query.value(8).toDouble();

      return entry;
    }
  }

  return Entry();
}

QStringList ImdbClient::findSimilar(const QString &title, Type type)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  static const int minWordLength = 5;
  const QStringList words = SStringParser::toCleanName(title).split(' ', QString::SkipEmptyParts);

  QString q;
  foreach (const QString &word, words)
  {
    const QString rawWord = SStringParser::toRawName(word);
    if (rawWord.length() >= minWordLength)
      q += "OR rawName LIKE '%" + rawWord + "%' ";
  }

  q = q.length() > 3 ? q.mid(3) : QString::null;

  QStringList result;
  if (!q.isEmpty())
  {
    QSqlQuery query(Database::database());
    query.exec("SELECT rawName, year FROM ImdbEntries "
               "WHERE type = " + QString::number(type) + " "
               "AND (" + q + ")");
    while (query.next())
      result += query.value(0).toString() + ("000" + QString::number(query.value(1).toInt())).right(4);
  }

  return result;
}

QString ImdbClient::findBest(const QString &title, const QStringList &matches)
{
  const QString rawName = SStringParser::toRawName(title);
  const QStringList words = SStringParser::toCleanName(title).split(' ', QString::SkipEmptyParts);

  unsigned year = 0;
  foreach (const QString &word, words)
  if (word.length() == 4)
  {
    const unsigned y = word.toUInt();
    if (y >= 1800)
    {
      year = y;
      break;
    }
  }

  qreal bestMatch = 0.0;
  QString best = "";
  foreach (const QString &indexName, matches)
  {
    const unsigned descYear = indexName.right(4).toUInt();
    const qreal match = SStringParser::computeMatch(rawName, indexName) *
                        ((((year == 0) || (descYear == year)) ? 1.0 : 0.5) +
                         (qreal(descYear) / 10000.0));

    if (match > bestMatch)
    {
      best = indexName.left(indexName.length() - 4);
      bestMatch = match;
    }
  }

  if (!best.isEmpty())
  {
    qDebug() << "Matching" << title << year << "to IMDB title" << best << "with rating" << bestMatch;

    if (bestMatch >= 0.3)
      return best;
  }

  return sentinelItem;
}

void ImdbClient::customEvent(QEvent *e)
{
  if (e->type() == tryMirrorEventType)
  {
    tryMirror();
  }
  else
    QObject::customEvent(e);
}

void ImdbClient::importIMDBDatabase(void)
{
  // Remove the sentinel entry indicating the last import has completed.
  SDebug::_MutexLocker<SScheduler::Dependency> dl(Database::mutex(), __FILE__, __LINE__);

  available = 0;

  QSqlQuery query(Database::database());
  query.exec("DELETE FROM ImdbEntries WHERE rawName = '#SENTINEL'");

  dl.unlock();

  SDebug::_MutexLocker<SScheduler::Dependency> l(&mutex, __FILE__, __LINE__);
  if (moviesFile.open(QIODevice::ReadOnly) &&
      plotFile.open(QIODevice::ReadOnly) &&
      ratingFile.open(QIODevice::ReadOnly))
  {
    l.unlock();
    qDebug() << "IMDB import: Parsing movies.list";

    sApp->schedule(this, &ImdbClient::readIMDBMoviesListLines, Q_INT64_C(0), &mutex, basePriority);
  }
}

struct ImdbClient::MoviesListLines
{
  QVariantList rawName, title, type, year, episodeName, episodeNumber,
               seasonNumber, plot, rating;
};

void ImdbClient::readIMDBMoviesListLines(qint64 pos)
{
  Q_ASSERT(!mutex.tryLock()); // Mutex should be locked by the caller.

  if (pos >= 0)
  {
    MoviesListLines lines;

    if (moviesFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = moviesFile.readLine(1024);
        if (!line.isEmpty())
        {
          if (line.contains("\t"))
          {
            const Entry entry = decodeEntry(line.left(line.length() - 5));
            if (entry.rawName.length() > 0)
            {
              lines.rawName << entry.rawName;
              lines.title << entry.title;
              lines.type << entry.type;
              lines.year << entry.year;
              lines.episodeName << entry.episodeName;
              lines.episodeNumber << entry.episodeNumber;
              lines.seasonNumber << entry.seasonNumber;
              lines.plot << entry.plot;
              lines.rating << entry.rating;
            }
          }
        }
        else // Finished
          finished = true;
      }

      if (!lines.rawName.isEmpty())
        sApp->schedule(this, &ImdbClient::insertIMDBMoviesListLines, lines, Database::mutex(), insertPriority);

      if (finished)
      {
        qDebug() << "IMDB import: Finished parsing movies.list, Parsing plot.list";
        sApp->schedule(this, &ImdbClient::readIMDBPlotListLines, Q_INT64_C(0), &mutex, basePriority);
      }
      else
        sApp->schedule(this, &ImdbClient::readIMDBMoviesListLines, moviesFile.pos(), &mutex, basePriority);
    }
  }
}

void ImdbClient::insertIMDBMoviesListLines(const MoviesListLines &lines)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  QSqlDatabase db = Database::database();
  db.transaction();

  QSqlQuery query(db);
  query.prepare("INSERT OR REPLACE INTO ImdbEntries VALUES ("
                ":rawName, :title, :type, :year, :episodeName, "
                ":episodeNumber, :seasonNumber, :plot, :rating)");
  query.bindValue(0, lines.rawName);
  query.bindValue(1, lines.title);
  query.bindValue(2, lines.type);
  query.bindValue(3, lines.year);
  query.bindValue(4, lines.episodeName);
  query.bindValue(5, lines.episodeNumber);
  query.bindValue(6, lines.seasonNumber);
  query.bindValue(7, lines.plot);
  query.bindValue(8, lines.rating);
  query.execBatch();

  db.commit();
}

struct ImdbClient::PlotListLines
{
  QVariantList rawName, plot;
};

void ImdbClient::readIMDBPlotListLines(qint64 pos)
{
  Q_ASSERT(!mutex.tryLock()); // Mutex should be locked by the caller.

  if (pos >= 0)
  {
    PlotListLines lines;

    if (plotFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = plotFile.readLine(1024);
        if (!line.isEmpty())
        {
          if (line.startsWith("MV:"))
          {
            Entry entry = decodeEntry(line.mid(4));
            if (entry.rawName.length() > 0)
            {
              for (QByteArray line = plotFile.readLine(1024);
                   !line.isEmpty();
                   line = plotFile.readLine(1024))
              {
                if (line.startsWith("PL:"))
                  entry.plot += QString::fromUtf8(line.mid(4)) + " ";
                else if (line.startsWith("BY:") || line.startsWith("----"))
                  break;
              }

              entry.plot = entry.plot.simplified();

              lines.rawName << entry.rawName;
              lines.plot << entry.plot;
            }
          }
        }
        else // Finished
          finished = true;
      }

      // Insert/update results
      if (!lines.rawName.isEmpty())
        sApp->schedule(this, &ImdbClient::insertIMDBPlotListLines, lines, Database::mutex(), insertPriority);

      if (finished)
      {
        qDebug() << "IMDB import: Finished parsing plot.list, Parsing ratings.list";
        sApp->schedule(this, &ImdbClient::readIMDBRatingListLines, Q_INT64_C(0), &mutex, basePriority);
      }
      else
        sApp->schedule(this, &ImdbClient::readIMDBPlotListLines, plotFile.pos(), &mutex, basePriority);
    }
  }
}

void ImdbClient::insertIMDBPlotListLines(const PlotListLines &lines)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  QSqlDatabase db = Database::database();
  db.transaction();

  QSqlQuery query(db);
  query.prepare("UPDATE ImdbEntries SET plot = :plot WHERE rawName = :rawName");
  query.bindValue(0, lines.plot);
  query.bindValue(1, lines.rawName);
  query.execBatch();

  db.commit();
}

struct ImdbClient::RatingListLines
{
  QVariantList rawName, rating;
};

void ImdbClient::readIMDBRatingListLines(qint64 pos)
{
  Q_ASSERT(!mutex.tryLock()); // Mutex should be locked by the caller.

  if (pos >= 0)
  {
    RatingListLines lines;

    if (ratingFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = ratingFile.readLine(1024);
        if (!line.isEmpty())
        {
          if (line.startsWith("      "))
          {
            const float r = line.mid(26, 5).trimmed().toFloat();
            if ((r > 0.9f) && (r < 10.1f))
            {
              const Entry entry = decodeEntry(line.mid(32));
              if (entry.rawName.length() > 0)
              {
                lines.rawName << entry.rawName;
                lines.rating << r;
              }
            }
          }
        }
        else // Finished
          finished = true;
      }

      // Insert/update results
      if (!lines.rawName.isEmpty())
        sApp->schedule(this, &ImdbClient::insertIMDBRatingListLines, lines, Database::mutex(), insertPriority);

      if (finished)
      {
        qDebug() << "IMDB import: Finished parsing ratings.list";
        sApp->schedule(this, &ImdbClient::insertSentinelItem, Database::mutex(), insertPriority);
      }
      else
        sApp->schedule(this, &ImdbClient::readIMDBRatingListLines, ratingFile.pos(), &mutex, basePriority);
    }
  }
}

void ImdbClient::insertIMDBRatingListLines(const RatingListLines &lines)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  QSqlDatabase db = Database::database();
  db.transaction();

  QSqlQuery query(db);
  query.prepare("UPDATE ImdbEntries SET rating = :rating WHERE rawName = :rawName");
  query.bindValue(0, lines.rating);
  query.bindValue(1, lines.rawName);
  query.execBatch();

  db.commit();
}

void ImdbClient::insertSentinelItem(void)
{
  Q_ASSERT(!Database::mutex()->tryLock()); // Mutex should be locked by the caller.

  QSqlQuery query(Database::database());
  query.exec("INSERT INTO ImdbEntries VALUES ('#SENTINEL', '', 0, 0, '', 0, 0, '', 0.0)");
}

ImdbClient::Entry ImdbClient::decodeEntry(const QByteArray &line)
{
  Entry entry;

  char episodeName[512], title[512];
  unsigned eni = 0, ti = 0;
  memset(episodeName, 0, sizeof(episodeName));
  memset(title, 0, sizeof(title));
  
  bool parsingYear = false, parsingEpisode = false, parsingEpisodeNumber = false;
  foreach (char c, line)
  {
    switch (c)
    {
    case '\t':
      entry.year = 0;
      parsingYear = true;
      parsingEpisode = false;
      parsingEpisodeNumber = false;
      break;

    case '(':
      if (!parsingEpisode)
        parsingYear = true;
      else
        parsingEpisodeNumber = true;

      break;

    case ')':
      if (!parsingEpisode)
        parsingYear = false;
      else
        parsingEpisodeNumber = false;

      break;

    case '{':
      parsingEpisode = true;
      break;

    case '}':
      parsingEpisode = false;
      break;

    default:
      if ((c != '\r') && (c != '\n'))
      {
        if (parsingYear)
        {
          if ((c >= '0') && (c <= '9'))
            entry.year = (entry.year * 10) + unsigned(c - '0');

          title[ti++] = c;
        }
        else if (parsingEpisodeNumber)
        {
          if ((c >= '0') && (c <= '9'))
            entry.episodeNumber = (entry.episodeNumber * 10) + unsigned(c - '0');
          else if (c == '.')
            qSwap(entry.episodeNumber, entry.seasonNumber);
        }
        else if (parsingEpisode)
        {
          if (eni < (sizeof(episodeName) - 1))
            episodeName[eni++] = c;
        }
        else if (c != '\"')
        {
          if (ti < (sizeof(title) - 1))
            title[ti++] = c;
        }
      }
    }
  }

  entry.title = QString::fromUtf8(title).simplified();
  entry.episodeName = QString::fromUtf8(episodeName).simplified();
  entry.rawName = SStringParser::toRawName(entry.title);

  if (line[0] == '\"')
    entry.type = Type_TvShow;
  else
    entry.type = Type_Movie;

  return entry;
}

void ImdbClient::tryMirror(void)
{
  SDebug::_MutexLocker<SScheduler::Dependency> l(&mutex, __FILE__, __LINE__);

  if (!obtainFiles.isEmpty() && (useMirror < (sizeof(mirrors) / sizeof(mirrors[0]))))
  {
    const QUrl url(mirrors[useMirror] + QDir(obtainFiles.first()->fileName()).dirName() + ".gz");
    qDebug() << "ImdbClient attempting to obtain" << url.toString();
  
    QNetworkRequest request;
    request.setUrl(url);
  
    reply = manager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(finished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error()));

    failed = false;
  }
}

void ImdbClient::finished(void)
{
  if (!failed)
  {
    qDebug() << "ImdbClient successfully obtained" << reply->url().toString();

    QFile compressedFile(obtainFiles.first()->fileName() + ".gz");
    if (compressedFile.open(QFile::WriteOnly | QFile::Truncate))
    {
      compressedFile.write(reply->readAll());
      compressedFile.close();

      FILE * const f = fopen(compressedFile.fileName().toUtf8().data(), "rb");
      if (f)
      {
        const gzFile gf = gzdopen(fileno(f), "rb");
        if (gf)
        {
          char buffer[65536];

          if (obtainFiles.first()->seek(0))
          {
            for (int n=gzread(gf, buffer, sizeof(buffer)); n>0; n=gzread(gf, buffer, sizeof(buffer)))
              obtainFiles.first()->write(buffer, n);

            obtainFiles.first()->flush();
            obtainFiles.first()->resize(obtainFiles.first()->pos());
          }

          gzclose(gf);
        }

        fclose(f);
      }

      compressedFile.remove();
    }

    if (obtainFiles.first()->size() > 0)
      obtainFiles.takeFirst()->close();
    else
      obtainFiles.takeFirst()->remove();

    // Next file or finished.
    if (obtainFiles.count() > 0)
      tryMirror();
    else
      importIMDBDatabase();
  }
  else
  {
    if (++useAttempt < (sizeof(mirrors) / sizeof(mirrors[0])))
    {
      useMirror++;
      tryMirror();
    }
  }
}

void ImdbClient::error(void)
{
  qWarning() << reply->errorString();
  failed = true;
}

} // End of namespace
