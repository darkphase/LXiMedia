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

#include <liblximediacenter/imdbclient.h>
#include <cstdio>
#include <QtSql>
#include <liblximediacenter/database.h>
#include <liblximediacenter/globalsettings.h>

// Prevents dependency with zlibg-dev (as Qt already provides zlib).
extern "C"
{
  typedef void * gzFile;
  gzFile gzdopen(int fd, const char *mode);
  int gzread(gzFile file, void *buf, unsigned len);
  int gzclose(gzFile file);
}


namespace LXiMediaCenter {

struct ImdbClient::Files
{
  QFile                         moviesFile;
  QFile                         plotFile;
  QFile                         ratingFile;
};

class ImdbClient::Task : public QRunnable
{
public:
  inline                      Task(void(* func)(qint64), qint64 pos) : func(func), pos(pos) { }

  virtual void                run(void);

private:
  void                        (* const func)(qint64);
  const qint64                pos;
};

class ImdbClient::MainThreadObject : public QObject
{
public:
  void                        createClient(void);

protected:
  virtual void                customEvent(QEvent *);

private:
  static const QEvent::Type   createClientEventType;
};

const char              * const ImdbClient::sentinelItem = "#SENTINEL";
const unsigned                  ImdbClient::readChunkSize = 3000;
const int                       ImdbClient::maxAge = 120;
QThreadPool                   * ImdbClient::threadPool = NULL;
ImdbClient::MainThreadObject  * ImdbClient::mainThreadObject = NULL;
bool                            ImdbClient::running = true;
int                             ImdbClient::downloading = 0;
int                             ImdbClient::available = 0;

const char * const ImdbClient::mirrors[] =
{
  "ftp://ftp.fu-berlin.de/pub/misc/movies/database/",
  "ftp://ftp.funet.fi/pub/mirrors/ftp.imdb.com/pub/",
  "ftp://ftp.sunet.se/pub/tv+movies/imdb/"
};

void ImdbClient::initialize(QThreadPool *pool)
{
  static bool initialized = false;
  if (!initialized)
  {
    GlobalSettings settings;

    initialized = true;
    threadPool = pool;
    running = true;
    SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);
    
    const QDir cacheDir = settings.applicationDataDir() + "/imdb";
    if (!cacheDir.exists())
    if (!cacheDir.mkpath(cacheDir.absolutePath()))
    {
      qWarning() << "Failed to create IMDB cache directory:" << cacheDir.absolutePath();
      return;
    }

    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
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

      db.transaction();

      foreach (const QString &name, indices)
        query.exec("DROP INDEX " + name);

      foreach (const QString &name, tables)
        query.exec("DROP TABLE " + name);

      db.commit();
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

    mainThreadObject = new MainThreadObject();

    files().moviesFile.setFileName(cacheDir.absoluteFilePath("movies.list"));
    files().plotFile.setFileName(cacheDir.absoluteFilePath("plot.list"));
    files().ratingFile.setFileName(cacheDir.absoluteFilePath("ratings.list"));

    if (!isAvailable() &&
        files().moviesFile.exists() &&
        files().plotFile.exists() &&
        files().ratingFile.exists() &&
        (files().moviesFile.size() > 0) &&
        (files().plotFile.size() > 0) &&
        (files().ratingFile.size() > 0))
    {
      importIMDBDatabase();
    }
  }
}

void ImdbClient::shutdown(void)
{
  running = false;

  delete mainThreadObject;
  mainThreadObject = NULL;
}

void ImdbClient::obtainIMDBFiles(void)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  if (files().moviesFile.open(QIODevice::ReadWrite) &&
      files().plotFile.open(QIODevice::ReadWrite) &&
      files().ratingFile.open(QIODevice::ReadWrite))
  {
    mainThreadObject->createClient();
  }
}

bool ImdbClient::isDownloading(void)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  return downloading > 0;
}

bool ImdbClient::isAvailable(void)
{
  if (available == 0)
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

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

  if (files().moviesFile.exists() && files().plotFile.exists() && files().ratingFile.exists())
  {
    return ((QFileInfo(files().moviesFile).lastModified().daysTo(now) > maxAge) ||
            (QFileInfo(files().plotFile).lastModified().daysTo(now) > maxAge) ||
            (QFileInfo(files().ratingFile).lastModified().daysTo(now) > maxAge));
  }

  return true;
}

QString ImdbClient::findEntry(const QString &title, Type type)
{
  static const int minWordLength = 5;

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

  QSet<QString> matches;
  foreach (const QString &word, words)
  {
    const QString rawWord = SStringParser::toRawName(word);
    if (rawWord.length() >= minWordLength)
    {
      SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

      QSqlQuery query(Database::database());
      query.exec("SELECT rawName, year FROM ImdbEntries "
                 "WHERE type = " + QString::number(type) + " "
                 "AND rawName LIKE '%" + rawWord + "%' ");

      while (query.next())
        matches.insert(query.value(0).toString() + ("000" + QString::number(query.value(1).toInt())).right(4));
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

ImdbClient::Entry ImdbClient::readEntry(const QString &rawName)
{
  if (rawName != sentinelItem)
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
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

void ImdbClient::importIMDBDatabase(void)
{
  // Remove the sentinel entry indicating the last import has completed.
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  available = 0;

  QSqlQuery query(Database::database());
  query.exec("DELETE FROM ImdbEntries WHERE rawName = '#SENTINEL'");

  dl.unlock();
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);
  if (files().moviesFile.open(QIODevice::ReadOnly) &&
      files().plotFile.open(QIODevice::ReadOnly) &&
      files().ratingFile.open(QIODevice::ReadOnly) &&
      running)
  {
    l.unlock();
    qDebug() << "IMDB import: Parsing movies.list";

    threadPool->start(new Task(&ImdbClient::readIMDBMoviesListLines, 0), -1);
  }
}

void ImdbClient::readIMDBMoviesListLines(qint64 pos)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  if ((pos >= 0) && running)
  {
    QVariantList rawName, title, type, year, episodeName, episodeNumber,
                 seasonNumber, plot, rating;

    if (files().moviesFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = files().moviesFile.readLine(1024);
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

      const qint64 nextPos = files().moviesFile.pos();
      l.unlock();

      // Insert/update results
      if (!rawName.isEmpty())
      {
        SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
        QSqlDatabase db = Database::database();
        db.transaction();

        QSqlQuery query(db);
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

        db.commit();
      }

      if (!finished)
      {
        threadPool->start(new Task(&ImdbClient::readIMDBMoviesListLines, nextPos), -1);
      }
      else
      {
        qDebug() << "IMDB import: Finished parsing movies.list, Parsing plot.list";
        threadPool->start(new Task(&ImdbClient::readIMDBPlotListLines, 0), -1);
      }
    }
  }
}

void ImdbClient::readIMDBPlotListLines(qint64 pos)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  if ((pos >= 0) && running)
  {
    QVariantList rawName, plot;

    if (files().plotFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = files().plotFile.readLine(1024);
        if (!line.isEmpty())
        {
          if (line.startsWith("MV:"))
          {
            Entry entry = decodeEntry(line.mid(4));
            if (entry.rawName.length() > 0)
            {
              for (QByteArray line = files().plotFile.readLine(1024);
                   !line.isEmpty();
                   line = files().plotFile.readLine(1024))
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

      const qint64 nextPos = files().plotFile.pos();
      l.unlock();

      // Insert/update results
      if (!rawName.isEmpty())
      {
        SDebug::MutexLocker l(&(Database::mutex()), __FILE__, __LINE__);
        QSqlDatabase db = Database::database();
        db.transaction();

        QSqlQuery query(db);
        query.prepare("UPDATE ImdbEntries SET plot = :plot WHERE rawName = :rawName");
        query.bindValue(0, plot);
        query.bindValue(1, rawName);
        query.execBatch();

        db.commit();
      }

      if (!finished)
      {
        threadPool->start(new Task(&ImdbClient::readIMDBPlotListLines, nextPos), -1);
      }
      else
      {
        qDebug() << "IMDB import: Finished parsing plot.list, Parsing ratings.list";
        threadPool->start(new Task(&ImdbClient::readIMDBRatingListLines, 0), -1);
      }
    }
  }
}

void ImdbClient::readIMDBRatingListLines(qint64 pos)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  if ((pos >= 0) && running)
  {
    QVariantList rawName, rating;

    if (files().ratingFile.seek(pos))
    {
      bool finished = false;
      for (unsigned i=0; (i<readChunkSize) && !finished; i++)
      {
        const QByteArray line = files().ratingFile.readLine(1024);
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

      const qint64 nextPos = files().ratingFile.pos();
      l.unlock();

      // Insert/update results
      if (!rawName.isEmpty())
      {
        SDebug::MutexLocker l(&(Database::mutex()), __FILE__, __LINE__);
        QSqlDatabase db = Database::database();
        db.transaction();

        QSqlQuery query(db);
        query.prepare("UPDATE ImdbEntries SET rating = :rating WHERE rawName = :rawName");
        query.bindValue(0, rating);
        query.bindValue(1, rawName);
        query.execBatch();

        db.commit();
      }

      if (!finished)
      {
        threadPool->start(new Task(&ImdbClient::readIMDBRatingListLines, nextPos), -1);
      }
      else
      {
        qDebug() << "IMDB import: Finished parsing ratings.list";

        // Add a sentinel entry indicating the last import has completed.
        SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

        available = 0;

        QSqlQuery query(Database::database());
        query.exec("INSERT INTO ImdbEntries VALUES ('#SENTINEL', '', 0, 0, '', 0, 0, '', 0.0)");

        dl.unlock();
      }
    }
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

QMutex & ImdbClient::mutex(void)
{
  static QMutex m(QMutex::Recursive);

  return m;
}

ImdbClient::Files & ImdbClient::files(void)
{
  static Files f;

  return f;
}

ImdbClient::ImdbClient(QList<QFile *> obtainFiles)
           :QObject(NULL),
            obtainFiles(obtainFiles),
            useMirror(qrand() % (sizeof(mirrors) / sizeof(mirrors[0]))),
            useAttempt(0),
            failed(false),
            manager(new QNetworkAccessManager(this)),
            reply(NULL)
{
  tryMirror();
}

ImdbClient::~ImdbClient()
{
  downloading--;

  delete reply;
  delete manager;
}

void ImdbClient::tryMirror(void)
{
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
    {
      tryMirror();
    }
    else
    {
      importIMDBDatabase();
      deleteLater();
    }
  }
  else
  {
    if (++useAttempt < (sizeof(mirrors) / sizeof(mirrors[0])))
    {
      useMirror++;
      tryMirror();
    }
    else
      deleteLater();
  }
}

void ImdbClient::error(void)
{
  qWarning() << reply->errorString();
  failed = true;
}

void ImdbClient::Task::run(void)
{
  func(pos);
}

const QEvent::Type ImdbClient::MainThreadObject::createClientEventType = QEvent::Type(QEvent::registerEventType());

void ImdbClient::MainThreadObject::createClient(void)
{
  SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

  downloading++;

  QCoreApplication::postEvent(this, new QEvent(createClientEventType));
}

void ImdbClient::MainThreadObject::customEvent(QEvent *e)
{
  if (e->type() == createClientEventType)
  {
    SDebug::MutexLocker l(&mutex(), __FILE__, __LINE__);

    new ImdbClient(QList<QFile *>() << &(files().moviesFile)
                                    << &(files().plotFile)
                                    << &(files().ratingFile));
  }
  else
    QObject::customEvent(e);
}

} // End of namespace
