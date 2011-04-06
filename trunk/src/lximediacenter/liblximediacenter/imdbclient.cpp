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

class ImdbClient::ReadMoviesListEvent : public QEvent
{
public:
  inline ReadMoviesListEvent(qint64 pos)
    : QEvent(readMoviesListEventType), pos(pos)
  {
  }

public:
  const qint64 pos;
};

class ImdbClient::ReadPlotListEvent : public QEvent
{
public:
  inline ReadPlotListEvent(qint64 pos)
    : QEvent(readPlotListEventType), pos(pos)
  {
  }

public:
  const qint64 pos;
};

class ImdbClient::ReadRatingListEvent : public QEvent
{
public:
  inline ReadRatingListEvent(qint64 pos)
    : QEvent(readRatingListEventType), pos(pos)
  {
  }

public:
  const qint64 pos;
};

struct ImdbClient::Data
{
  static const unsigned         readChunkSize;
  static const int              maxAge;
  static const int              basePriority;

  QDir                          cacheDir;
  QFile                         moviesFile;
  QFile                         plotFile;
  QFile                         ratingFile;
  int                           downloading;
  int                           available;

  QList<QFile *>                obtainFiles;
  unsigned                      useMirror, useAttempt;
  bool                          failed;
  QNetworkAccessManager       * manager;
  QNetworkReply               * reply;
};

const char  * const ImdbClient::sentinelItem = "#SENTINEL";
const QEvent::Type  ImdbClient::tryMirrorEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  ImdbClient::readMoviesListEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  ImdbClient::readPlotListEventType = QEvent::Type(QEvent::registerEventType());
const QEvent::Type  ImdbClient::readRatingListEventType = QEvent::Type(QEvent::registerEventType());

#if defined(Q_OS_UNIX)
const unsigned      ImdbClient::Data::readChunkSize = 3000;
#else
const unsigned      ImdbClient::Data::readChunkSize = 500;
#endif
const int           ImdbClient::Data::maxAge = 120;
const int           ImdbClient::Data::basePriority = INT_MIN;

const char * const ImdbClient::mirrors[] =
{
  "ftp://ftp.fu-berlin.de/pub/misc/movies/database/",
  "ftp://ftp.funet.fi/pub/mirrors/ftp.imdb.com/pub/",
  "ftp://ftp.sunet.se/pub/tv+movies/imdb/"
};

ImdbClient::ImdbClient(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  GlobalSettings settings;

  d->cacheDir = QDir(settings.applicationDataDir() + "/imdb");
  d->downloading = 0;
  d->available = 0;
  d->useMirror = qrand() % (sizeof(mirrors) / sizeof(mirrors[0]));
  d->useAttempt = 0;
  d->failed = false;
  d->manager = new QNetworkAccessManager(this);
  d->reply = NULL;

  static bool firsttime = true;
  if (firsttime)
  {
    firsttime = false;

    if (!d->cacheDir.exists())
    if (!d->cacheDir.mkpath(d->cacheDir.absolutePath()))
    {
      qWarning() << "Failed to create IMDB cache directory:" << d->cacheDir.absolutePath();
      return;
    }

    Database::Query query;

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

      Database::transaction();

      foreach (const QString &name, indices)
        query.exec("DROP INDEX IF EXISTS " + name);

      foreach (const QString &name, tables)
        query.exec("DROP TABLE IF EXISTS " + name);

      Database::commit();

      query.exec("PRAGMA foreign_keys = ON");

      query.exec("VACUUM");
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

  d->moviesFile.setFileName(d->cacheDir.absoluteFilePath("movies.list"));
  d->plotFile.setFileName(d->cacheDir.absoluteFilePath("plot.list"));
  d->ratingFile.setFileName(d->cacheDir.absoluteFilePath("ratings.list"));

  if (!isAvailable() &&
      d->moviesFile.exists() && d->plotFile.exists() && d->ratingFile.exists() &&
      (d->moviesFile.size() > 0) && (d->plotFile.size() > 0) && (d->ratingFile.size() > 0))
  {
    importIMDBDatabase();
  }
}

ImdbClient::~ImdbClient(void)
{
  delete d->reply;
  delete d->manager;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void ImdbClient::obtainIMDBFiles(void)
{
  if (d->moviesFile.open(QIODevice::ReadWrite) &&
      d->plotFile.open(QIODevice::ReadWrite) &&
      d->ratingFile.open(QIODevice::ReadWrite))
  {
    d->obtainFiles.clear();
    d->obtainFiles << &d->moviesFile << &d->plotFile << &d->ratingFile;

    QCoreApplication::postEvent(this, new QEvent(tryMirrorEventType));
  }
}

bool ImdbClient::isDownloading(void) const
{
  return d->downloading > 0;
}

bool ImdbClient::isAvailable(void) const
{
  if (d->available == 0)
  {
    // Check for the sentinel entry indicating the last import has completed.
    Database::Query query;
    query.exec("SELECT title FROM ImdbEntries WHERE rawName = '" + QString(sentinelItem) + "'");
    if (!query.next())
    {
      d->available = -1;
      return false;
    }

    d->available = 1;
    return true;
  }

  return d->available == 1;
}

bool ImdbClient::needUpdate(void) const
{
  const QDateTime now = QDateTime::currentDateTime();

  if (d->moviesFile.exists() && d->plotFile.exists() && d->ratingFile.exists())
  {
    return ((QFileInfo(d->moviesFile).lastModified().daysTo(now) > d->maxAge) ||
            (QFileInfo(d->plotFile).lastModified().daysTo(now) > d->maxAge) ||
            (QFileInfo(d->ratingFile).lastModified().daysTo(now) > d->maxAge));
  }

  return true;
}

ImdbClient::Entry ImdbClient::readEntry(const QString &rawName)
{
  if (rawName != sentinelItem)
  {
    Database::Query query;
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
    Database::Query query;
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
  if (e->type() == readMoviesListEventType)
  {
    const ReadMoviesListEvent * const event = static_cast<const ReadMoviesListEvent *>(e);
    if (event->pos >= 0)
    {
      QVariantList rawName, title, type, year, episodeName, episodeNumber,
                   seasonNumber, plot, rating;

      if (d->moviesFile.seek(event->pos))
      {
        bool finished = false;
        for (unsigned i=0; (i<d->readChunkSize) && !finished; i++)
        {
          const QByteArray line = d->moviesFile.readLine(1024);
          if (!line.isEmpty())
          {
            if (line.contains("\t"))
            {
              const Entry entry = decodeEntry(line.left(line.length() - 5));
              if (entry.rawName.length() > 0)
              {
                rawName << entry.rawName;
                title << entry.title;
                type << entry.type;
                year << entry.year;
                episodeName << entry.episodeName;
                episodeNumber << entry.episodeNumber;
                seasonNumber << entry.seasonNumber;
                plot << entry.plot;
                rating << entry.rating;
              }
            }
          }
          else // Finished
            finished = true;
        }

        if (!rawName.isEmpty())
        {
          Database::transaction();

          Database::Query query;
          query.prepare("INSERT OR REPLACE INTO ImdbEntries VALUES ("
                        ":rawName, :title, :type, :year, :episodeName, "
                        ":episodeNumber, :seasonNumber, :plot, :rating)");
          query.bindValue(0, rawName);
          query.bindValue(1, title);
          query.bindValue(2, type);
          query.bindValue(3, year);
          query.bindValue(4, episodeName);
          query.bindValue(5, episodeNumber);
          query.bindValue(6, seasonNumber);
          query.bindValue(7, plot);
          query.bindValue(8, rating);
          query.execBatch();

          Database::commit();
        }

        if (finished)
        {
          qDebug() << "IMDB import: Finished parsing movies.list, parsing plot.list";
          qApp->postEvent(this, new ReadPlotListEvent(0), d->basePriority);
        }
        else
          qApp->postEvent(this, new ReadMoviesListEvent(d->moviesFile.pos()), d->basePriority);
      }
    }
  }
  else if (e->type() == readPlotListEventType)
  {
    const ReadPlotListEvent * const event = static_cast<const ReadPlotListEvent *>(e);
    if (event->pos >= 0)
    {
      QVariantList rawName, plot;

      if (d->plotFile.seek(event->pos))
      {
        bool finished = false;
        for (unsigned i=0; (i<d->readChunkSize) && !finished; i++)
        {
          const QByteArray line = d->plotFile.readLine(1024);
          if (!line.isEmpty())
          {
            if (line.startsWith("MV:"))
            {
              Entry entry = decodeEntry(line.mid(4));
              if (entry.rawName.length() > 0)
              {
                for (QByteArray line = d->plotFile.readLine(1024);
                     !line.isEmpty();
                     line = d->plotFile.readLine(1024))
                {
                  if (line.startsWith("PL:"))
                    entry.plot += QString::fromUtf8(line.mid(4)) + " ";
                  else if (line.startsWith("BY:") || line.startsWith("----"))
                    break;
                }

                entry.plot = entry.plot.simplified();

                rawName << entry.rawName;
                plot << entry.plot;
              }
            }
          }
          else // Finished
            finished = true;
        }

        // Insert/update results
        if (!rawName.isEmpty())
        {
          Database::transaction();

          Database::Query query;
          query.prepare("UPDATE ImdbEntries SET plot = :plot WHERE rawName = :rawName");
          query.bindValue(0, plot);
          query.bindValue(1, rawName);
          query.execBatch();

          Database::commit();
        }

        if (finished)
        {
          qDebug() << "IMDB import: Finished parsing plot.list, parsing ratings.list";
          qApp->postEvent(this, new ReadRatingListEvent(0), d->basePriority);
        }
        else
          qApp->postEvent(this, new ReadPlotListEvent(d->plotFile.pos()), d->basePriority);
      }
    }
  }
  else if (e->type() == readRatingListEventType)
  {
    const ReadRatingListEvent * const event = static_cast<const ReadRatingListEvent *>(e);
    if (event->pos >= 0)
    {
      QVariantList rawName, rating;

      if (d->ratingFile.seek(event->pos))
      {
        bool finished = false;
        for (unsigned i=0; (i<d->readChunkSize) && !finished; i++)
        {
          const QByteArray line = d->ratingFile.readLine(1024);
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
                  rawName << entry.rawName;
                  rating << r;
                }
              }
            }
          }
          else // Finished
            finished = true;
        }

        // Insert/update results
        if (!rawName.isEmpty())
        {
          Database::transaction();

          Database::Query query;
          query.prepare("UPDATE ImdbEntries SET rating = :rating WHERE rawName = :rawName");
          query.bindValue(0, rating);
          query.bindValue(1, rawName);
          query.execBatch();

          Database::commit();
        }

        if (finished)
        {
          qDebug() << "IMDB import: Finished.";
          Database::Query("INSERT OR IGNORE INTO ImdbEntries VALUES ('" + QString(sentinelItem) + "', '', 0, 0, '', 0, 0, '', 0.0)").exec();
        }
        else
          qApp->postEvent(this, new ReadRatingListEvent(d->ratingFile.pos()), d->basePriority);
      }
    }
  }
  else if (e->type() == tryMirrorEventType)
    tryMirror();
  else
    QObject::customEvent(e);
}

void ImdbClient::importIMDBDatabase(void)
{
  // Remove the sentinel entry indicating the last import has completed.
  d->available = 0;
  Database::Query("DELETE FROM ImdbEntries WHERE rawName = '" + QString(sentinelItem) + "'").exec();

  if (d->moviesFile.open(QIODevice::ReadOnly) &&
      d->plotFile.open(QIODevice::ReadOnly) &&
      d->ratingFile.open(QIODevice::ReadOnly))
  {
    qDebug() << "IMDB import: Parsing movies.list";

    qApp->postEvent(this, new ReadMoviesListEvent(0), d->basePriority);
  }
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
  if (!d->obtainFiles.isEmpty() && (d->useMirror < (sizeof(mirrors) / sizeof(mirrors[0]))))
  {
    const QUrl url(mirrors[d->useMirror] + QDir(d->obtainFiles.first()->fileName()).dirName() + ".gz");
    qDebug() << "ImdbClient attempting to obtain" << url.toString();
  
    QNetworkRequest request;
    request.setUrl(url);
  
    d->reply = d->manager->get(request);
    connect(d->reply, SIGNAL(finished()), SLOT(finished()));
    connect(d->reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(error()));

    d->failed = false;
  }
}

void ImdbClient::finished(void)
{
  if (!d->failed)
  {
    qDebug() << "ImdbClient successfully obtained" << d->reply->url().toString();

    QFile compressedFile(d->obtainFiles.first()->fileName() + ".gz");
    if (compressedFile.open(QFile::WriteOnly | QFile::Truncate))
    {
      compressedFile.write(d->reply->readAll());
      compressedFile.close();

      FILE * const f = fopen(compressedFile.fileName().toUtf8().data(), "rb");
      if (f)
      {
        const gzFile gf = gzdopen(fileno(f), "rb");
        if (gf)
        {
          char buffer[65536];

          if (d->obtainFiles.first()->seek(0))
          {
            for (int n=gzread(gf, buffer, sizeof(buffer)); n>0; n=gzread(gf, buffer, sizeof(buffer)))
              d->obtainFiles.first()->write(buffer, n);

            d->obtainFiles.first()->flush();
            d->obtainFiles.first()->resize(d->obtainFiles.first()->pos());
          }

          gzclose(gf);
        }

        fclose(f);
      }

      compressedFile.remove();
    }

    if (d->obtainFiles.first()->size() > 0)
      d->obtainFiles.takeFirst()->close();
    else
      d->obtainFiles.takeFirst()->remove();

    // Next file or finished.
    if (d->obtainFiles.count() > 0)
      tryMirror();
    else
      importIMDBDatabase();
  }
  else
  {
    if (++d->useAttempt < (sizeof(mirrors) / sizeof(mirrors[0])))
    {
      d->useMirror++;
      tryMirror();
    }
  }
}

void ImdbClient::error(void)
{
  qWarning() << d->reply->errorString();
  d->failed = true;
}

} // End of namespace
