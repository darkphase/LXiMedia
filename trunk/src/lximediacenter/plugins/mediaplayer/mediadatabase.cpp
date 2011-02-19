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

#include "mediadatabase.h"
#include "configserver.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {

const int MediaDatabase::maxSongDurationMin = 15;

const MediaDatabase::CatecoryDesc MediaDatabase::categories[] =
{
  { "movies",     MediaDatabase::Category_Movies     },
  { "tvshows",    MediaDatabase::Category_TVShows    },
  { "clips",      MediaDatabase::Category_Clips      },
  { "homevideos", MediaDatabase::Category_HomeVideos },
  { "photos",     MediaDatabase::Category_Photos     },
  { "music",      MediaDatabase::Category_Music      },
  { NULL,         MediaDatabase::Category_None       }
};

MediaDatabase::MediaDatabase(Plugin *plugin, QThreadPool *threadPool)
  : QObject(plugin),
    plugin(plugin),
    threadPool(threadPool),
    mutex(QMutex::Recursive),
    matchingImdbItems(false)
{
  PluginSettings settings(plugin);

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlDatabase db = Database::database();
  QSqlQuery query(db);

  // Drop all tables if the database version is outdated.
  static const int databaseVersion = 2;
  if (settings.value("DatabaseVersion", 0).toInt() != databaseVersion)
  {
    qDebug() << "Mediaplayer database layout changed, recreating tables.";

    QStringList tables;
    query.exec("SELECT name FROM sqlite_master WHERE type='table'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Mediaplayer"))
        tables += name;
    }

    QStringList indices;
    query.exec("SELECT name FROM sqlite_master WHERE type='index'");
    while (query.next())
    {
      const QString name = query.value(0).toString();
      if (name.startsWith("Mediaplayer"))
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
  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerFiles ("
             "uid            INTEGER PRIMARY KEY,"
             "parentDir      INTEGER,"
             "path           TEXT UNIQUE NOT NULL,"
             "size           INTEGER NOT NULL,"
             "lastModified   DATE NOT NULL,"
             "mediaInfo      TEXT,"
             "FOREIGN KEY(parentDir) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerFiles_path "
             "ON MediaplayerFiles(path)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerFiles_parentDir "
             "ON MediaplayerFiles(parentDir)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerFiles_size "
             "ON MediaplayerFiles(size)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerAlbums ("
             "id             INTEGER PRIMARY KEY,"
             "parentDir      INTEGER NOT NULL,"
             "category       INTEGER NOT NULL,"
             "name           TEXT NOT NULL,"
             "FOREIGN KEY(parentDir) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerAlbums_name "
             "ON MediaplayerAlbums(name)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerItems ("
             "file           INTEGER NOT NULL,"
             "album          INTEGER NOT NULL,"
             "title          TEXT,"
             "subtitle       TEXT,"
             "imdbLink       TEXT,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE,"
             "FOREIGN KEY(album) REFERENCES MediaplayerAlbums(id) ON DELETE CASCADE,"
             "FOREIGN KEY(imdbLink) REFERENCES ImdbEntries(rawName) ON DELETE SET NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerItems_album "
             "ON MediaplayerItems(album)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerItems_title "
             "ON MediaplayerItems(title)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerItems_subtitle "
             "ON MediaplayerItems(subtitle)");

  settings.setValue("DatabaseVersion", databaseVersion);

  dl.unlock();

  connect(&scanRootsTimer, SIGNAL(timeout()), SLOT(scanRoots()));

  scanRootsTimer.start(60000);
  QTimer::singleShot(3000, this, SLOT(scanRoots()));
}

MediaDatabase::~MediaDatabase()
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);
  dirsToScan.clear();
  filesToProbe.clear();
  imdbItemsToMatch.clear();
  l.unlock();

  QThreadPool::globalInstance()->waitForDone();
}

MediaDatabase::UniqueID MediaDatabase::fromPath(const QString &path) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT uid FROM MediaplayerFiles WHERE path = :path");
  query.bindValue(0, path);
  query.exec();
  if (query.next())
    return query.value(0).toLongLong();

  return 0;
}

SMediaInfo MediaDatabase::readNode(UniqueID uid) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.exec("SELECT mediaInfo "
             "FROM MediaplayerFiles WHERE uid = " + QString::number(uid));
  if (query.next())
  {
    SMediaInfo node;
    node.fromByteArray(query.value(0).toByteArray());

    return node;
  }

  return SMediaInfo();
}

void MediaDatabase::setLastPlayed(UniqueID uid, const QDateTime &lastPlayed)
{
  const SMediaInfo node = readNode(uid);
  if (!node.isNull())
    setLastPlayed(node.filePath(), lastPlayed);
}

void MediaDatabase::setLastPlayed(const QString &filePath, const QDateTime &lastPlayed)
{
  if (!filePath.isEmpty())
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = filePath;
    key.replace('/', '|');
    key.replace('\\', '|');

    if (lastPlayed.isValid())
      settings.setValue(key, lastPlayed);
    else
      settings.remove(key);
  }
}

QDateTime MediaDatabase::lastPlayed(UniqueID uid) const
{
  const SMediaInfo node = readNode(uid);
  if (!node.isNull())
    return lastPlayed(node.filePath());

  return QDateTime();
}

QDateTime MediaDatabase::lastPlayed(const QString &filePath) const
{
  if (!filePath.isEmpty())
  {
    SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = filePath;
    key.replace('/', '|');
    key.replace('\\', '|');

    return settings.value(key, QDateTime()).toDateTime();
  }

  return QDateTime();
}

QStringList MediaDatabase::allAlbums(Category category) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT name FROM MediaplayerAlbums "
                "WHERE category = :category "
                "ORDER BY name");
  query.bindValue(0, category);
  query.exec();
  while (query.next())
    result << query.value(0).toString();

  return result;
}

int MediaDatabase::countAlbumFiles(Category category, const QString &album) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT COUNT(*) FROM MediaplayerItems "
                "WHERE album IN ("
                  "SELECT id FROM MediaplayerAlbums "
                  "WHERE name = :name AND category = :category)");
  query.bindValue(0, album);
  query.bindValue(1, category);
  query.exec();
  if (query.next())
    return query.value(0).toInt();

  return 0;
}

bool MediaDatabase::hasAlbum(Category category, const QString &album) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT COUNT(*) FROM MediaplayerAlbums "
                "WHERE name = :name AND category = :category");
  query.bindValue(0, album);
  query.bindValue(1, category);
  query.exec();
  if (query.next())
    return query.value(0).toInt() > 0;

  return false;
}

QList<MediaDatabase::File> MediaDatabase::getAlbumFiles(Category category, const QString &album, unsigned start, unsigned count) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QString limit;
  if (count > 0)
  {
    limit += " LIMIT " + QString::number(count);
    if (start > 0)
      limit += " OFFSET " + QString::number(start);
  }

  QList<File> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT file, title FROM MediaplayerItems "
                "WHERE album IN ("
                  "SELECT id FROM MediaplayerAlbums "
                  "WHERE name = :name AND category = :category) "
                "ORDER BY title" + limit);
  query.bindValue(0, album);
  query.bindValue(1, category);
  query.exec();
  while (query.next())
    result << File(query.value(0).toLongLong(), query.value(1).toString());

  return result;
}

QList<MediaDatabase::File> MediaDatabase::queryAlbums(Category category, const QStringList &q, unsigned start, unsigned count) const
{
  QString limit;
  if (count > 0)
  {
    limit += " LIMIT " + QString::number(count);
    if (start > 0)
      limit += " OFFSET " + QString::number(start);
  }

  QList<File> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    if (!item.isEmpty())
    {
      qs1 += " AND title LIKE '%" + rawItem + "%'";
      qs2 += " AND subtitle LIKE '%" + rawItem + "%'";
    }
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.prepare("SELECT file, title FROM MediaplayerItems "
                  "WHERE album IN ("
                    "SELECT id FROM MediaplayerAlbums "
                    "WHERE category = :category) "
                  "AND ((" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")) "
                  "ORDER BY title" + limit);
    query.bindValue(0, category);
    query.exec();
    while (query.next())
      result << File(query.value(0).toLongLong(), query.value(1).toString());
  }

  return result;
}

ImdbClient::Entry MediaDatabase::getImdbEntry(UniqueID uid) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT imdbLink FROM MediaplayerItems WHERE file = :file");
  query.bindValue(0, uid);
  query.exec();
  while (query.next())
  if (!query.value(0).isNull() && (query.value(0).toString() != ImdbClient::sentinelItem))
    return ImdbClient::readEntry(query.value(0).toString());

  return ImdbClient::Entry();
}

QList<MediaDatabase::UniqueID> MediaDatabase::allFilesInDirOf(UniqueID uid) const
{
  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.exec("SELECT parentDir FROM MediaplayerFiles WHERE uid = " + QString::number(uid));
  if (query.next())
  {
    query.exec("SELECT uid FROM MediaplayerFiles WHERE parentDir = " + QString::number(query.value(0).toLongLong()));
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

void MediaDatabase::scanRoots(void)
{
  PluginSettings settings(plugin);

  QStringList allRootPaths;
  foreach (const QString &group, settings.childGroups())
  {
    settings.beginGroup(group);

    QStringList paths;
    foreach (const QString &root, settings.value("Paths").toStringList())
    if (!root.trimmed().isEmpty())
    {
      const QFileInfo info(root);
      if (info.exists())
      {
        bool hide = false;
        const QString canonicalPath =
#ifndef Q_OS_WIN
            info.canonicalFilePath();
#else
            info.canonicalFilePath().toLower();
#endif

        foreach (const QString &hidden, ConfigServer::hiddenDirs())
        if ((canonicalPath == hidden) || canonicalPath.startsWith(hidden + "/"))
        {
          hide = true;
          break;
        }

        if (!hide)
          paths.append(canonicalPath);
      }
    }

    settings.setValue("Paths", paths);
    rootPaths[group] = paths;
    allRootPaths += paths;

    settings.endGroup();
  }

  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (dirsToScan.isEmpty() && filesToProbe.isEmpty() && !matchingImdbItems)
  {
    foreach (const QString &root, allRootPaths)
    if (QFileInfo(root).exists())
      dirsToScan.insert(root);

    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
    QSqlDatabase db = Database::database();
    db.transaction();
    QSqlQuery query(db);

    // Find roots that are no longer roots and remove them
    query.exec("SELECT path FROM MediaplayerFiles WHERE parentDir ISNULL");
    QVariantList removePaths;
    while (query.next())
    if (findRoot(query.value(0).toString(), allRootPaths).isEmpty())
      removePaths << query.value(0);

    if (!removePaths.isEmpty())
    {
      query.prepare("DELETE FROM MediaplayerFiles WHERE path = :path");
      query.bindValue(0, removePaths);
      query.execBatch();
    }

    // Find files that were not probed yet.
    query.exec("SELECT path FROM MediaplayerFiles WHERE size < 0");
    while (query.next())
      filesToProbe.insert(query.value(0).toString());

    // Find IMDB items that still need to be matched.
    if (ImdbClient::isAvailable())
    {
      query.prepare("SELECT title FROM MediaplayerItems "
                    "WHERE album IN ("
                      "SELECT id FROM MediaplayerAlbums "
                      "WHERE category = :category) "
                    "AND imdbLink ISNULL");
      query.bindValue(0, Category_Movies);
      query.exec();
      while (query.next())
        imdbItemsToMatch.insert("M" + query.value(0).toString());

//      query.prepare("SELECT title FROM MediaplayerItems "
//                    "WHERE album IN ("
//                      "SELECT id FROM MediaplayerAlbums "
//                      "WHERE category = :category) "
//                    "AND imdbLink ISNULL");
//      query.bindValue(0, Category_TVShows);
//      query.exec();
//      while (query.next())
//        imdbItemsToMatch.insert("T" + query.value(0).toString());
    }

    db.commit();

    if (!dirsToScan.isEmpty())
      threadPool->start(new Task(this, &MediaDatabase::scanDirs), -1);
    else if (!filesToProbe.isEmpty())
      threadPool->start(new Task(this, &MediaDatabase::probeFiles), -1);
    else if (!imdbItemsToMatch.isEmpty())
    {
      matchingImdbItems = true;
      threadPool->start(new Task(this, &MediaDatabase::matchImdbItems), -1);
    }
  }
}

QString MediaDatabase::findRoot(const QString &path, const QStringList &allRootPaths) const
{
  const QFileInfo info(path);

#ifndef Q_OS_WIN
  const QString canonicalPath = info.exists() ? info.canonicalFilePath() : info.absoluteFilePath();
#else
  const QString canonicalPath = QDir::toNativeSeparators(info.exists() ? info.canonicalFilePath() : info.absoluteFilePath()).toLower();
#endif

  foreach (const QString &root, allRootPaths)
#ifndef Q_OS_WIN
  if ((root == canonicalPath) || canonicalPath.startsWith(root + "/"))
    return root.endsWith('/') ? root : (root + '/');
#else
  {
    const QString croot = QDir::toNativeSeparators(root).toLower();
    if ((croot == canonicalPath) || canonicalPath.startsWith(croot + "\\"))
      return root.endsWith('/') ? root : (root + '/');
  }
#endif

  return QString::null;
}

struct MediaDatabase::QuerySet
{
  inline QuerySet(const QSqlDatabase &db)
    : request(db), insert(db), update(db), children(db), remove(db)
  {
  }

  QSqlQuery request;
  QSqlQuery insert;
  QSqlQuery update;
  QSqlQuery children;
  QSqlQuery remove;
};

void MediaDatabase::scanDirs(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!dirsToScan.isEmpty())
  {
    QSet<QString>::Iterator i = dirsToScan.begin();
    QString path =
#ifndef Q_OS_WIN
        *i;
#else
        i->toLower();
#endif
    dirsToScan.erase(i);
    l.unlock();

    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
    QSqlDatabase db = Database::database();
    db.transaction();

    QuerySet q(db);
    q.request.prepare("SELECT uid, parentDir, size, lastModified "
                      "FROM MediaplayerFiles WHERE path = :path");

    q.insert.prepare("INSERT INTO MediaplayerFiles "
                     "VALUES (:uid, :parentDir, :path, :size, :lastModified, :mediaInfo)");

    q.update.prepare("UPDATE MediaplayerFiles "
                     "SET size = :size, lastModified = :lastModified, mediaInfo = :mediaInfo "
                     "WHERE path = :path");

    q.children.prepare("SELECT path FROM MediaplayerFiles "
                       "WHERE parentDir = :parentDir");

    q.remove.prepare("DELETE FROM MediaplayerFiles WHERE path = :path");

    // Ensure directories end with a '/'
    if (!path.endsWith('/'))
      path += '/';

    const QFileInfo info(path);

    l.relock(__FILE__, __LINE__);

    q.request.bindValue(0, path);
    q.request.exec();
    if (q.request.next()) // Directory is in the database
    {
      const qint64 rowId = q.request.value(0).toLongLong();
      const qint64 size = q.request.value(2).toLongLong();
      const QDateTime lastModified = q.request.value(3).toDateTime();

      const QDir dir(path);
      if ((dir.count() != unsigned(size)) || (info.lastModified() > lastModified.addSecs(2)))
      {
        //qDebug() << "Updated dir:" << path << dir.count() << size << info.lastModified() << lastModified.addSecs(2);

        // Update the dir
        q.update.bindValue(0, dir.count());
        q.update.bindValue(1, info.lastModified());
        q.update.bindValue(2, QVariant(QVariant::ByteArray));
        q.update.bindValue(3, path);
        q.update.exec();

        updateDir(path, rowId, q);
      }
      else // Dir is up-to-date, only scan child dirs
      {
        foreach (const QFileInfo &child, dir.entryInfoList(QDir::Dirs))
        if (!child.fileName().startsWith('.'))
          dirsToScan.insert(child.canonicalFilePath());
      }
    }
    else if (info.isDir())
    {
      qDebug() << "New root dir:" << path;

      // Insert the dir
      q.insert.bindValue(0, QVariant(QVariant::LongLong));
      q.insert.bindValue(1, QVariant(QVariant::LongLong));
      q.insert.bindValue(2, path);
      q.insert.bindValue(3, -1); // Size will be set when scanned
      q.insert.bindValue(4, info.lastModified());
      q.insert.bindValue(5, QVariant(QVariant::ByteArray));
      q.insert.exec();

      dirsToScan.insert(path); // Scan again.
    }

    db.commit();
  }

  if (!dirsToScan.isEmpty())
    threadPool->start(new Task(this, &MediaDatabase::scanDirs), -1);
  else if (!filesToProbe.isEmpty())
    threadPool->start(new Task(this, &MediaDatabase::probeFiles), -1);
  else if (!imdbItemsToMatch.isEmpty() && !matchingImdbItems)
  {
    matchingImdbItems = true;
    threadPool->start(new Task(this, &MediaDatabase::matchImdbItems), -1);
  }
}

void MediaDatabase::updateDir(const QString &path, qint64 parentDir, QuerySet &q)
{
  qDebug() << "Scanning:" << path;

  QSet<QString> childPaths;

  // Update existing and add new dirs.
  foreach (const QFileInfo &child, QDir(path).entryInfoList(QDir::Dirs))
  if (!child.fileName().startsWith('.'))
  {
    QString childPath =
#ifndef Q_OS_WIN
        child.canonicalFilePath();
#else
        child.canonicalFilePath().toLower();
#endif

    // Ensure directories end with a '/'
    if (!childPath.endsWith('/'))
      childPath += '/';

    childPaths.insert(childPath);

    q.request.bindValue(0, childPath);
    q.request.exec();
    if (!q.request.next())
    {
      // Insert the dir
      q.insert.bindValue(0, QVariant(QVariant::LongLong));
      q.insert.bindValue(1, parentDir);
      q.insert.bindValue(2, childPath);
      q.insert.bindValue(3, -1); // Size will be set when scanned
      q.insert.bindValue(4, child.lastModified());
      q.insert.bindValue(5, QVariant(QVariant::ByteArray));
      q.insert.exec();
    }

    dirsToScan.insert(childPath);
  }

  // Update existing and add new files.
  foreach (const QFileInfo &child, QDir(path).entryInfoList(QDir::Files))
  if (!child.fileName().startsWith('.'))
  {
    if (path.endsWith("/VIDEO_TS/", Qt::CaseInsensitive) || path.endsWith("/AUDIO_TS/", Qt::CaseInsensitive))
    if (child.fileName().compare("VIDEO_TS.IFO", Qt::CaseInsensitive) != 0)
      continue;

    const QString childPath =
#ifndef Q_OS_WIN
        child.canonicalFilePath();
#else
        child.canonicalFilePath().toLower();
#endif

    childPaths.insert(childPath);

    q.request.bindValue(0, childPath);
    q.request.exec();
    if (!q.request.next())
    {
      qDebug() << "New file:" << childPath;

      // Insert the file
      q.insert.bindValue(0, QVariant(QVariant::LongLong));
      q.insert.bindValue(1, parentDir);
      q.insert.bindValue(2, childPath);
      q.insert.bindValue(3, -1); // Size will be set when probed
      q.insert.bindValue(4, child.lastModified());
      q.insert.bindValue(5, QVariant(QVariant::ByteArray));
      q.insert.exec();

      filesToProbe.insert(childPath);
    }
    else if ((child.size() != q.request.value(2).toLongLong()) ||
             (child.lastModified() > q.request.value(3).toDateTime().addSecs(2)))
    {
      //qDebug() << "Updated file:" << childPath << child.size() << q.request.value(2).toLongLong() << child.lastModified() << q.request.value(3).toDateTime().addSecs(2);

      // Update the file
      q.update.bindValue(0, -1); // Size will be set when probed
      q.update.bindValue(1, child.lastModified());
      q.update.bindValue(2, QVariant(QVariant::ByteArray));
      q.update.bindValue(3, childPath);
      q.update.exec();

      filesToProbe.insert(childPath);
    }
  }

  // Remove deleted files
  q.children.bindValue(0, parentDir);
  q.children.exec();

  QVariantList removePaths;
  while (q.children.next())
  {
    const QString path = q.children.value(0).toString();
    if (!childPaths.contains(path))
    {
      //qDebug() << "Removed file:" << path;

      removePaths << path;
    }
  }

  if (!removePaths.isEmpty())
  {
    //foreach (const QString &c, childPaths)
    //  qDebug() << "ChildPath:" << c;

    q.remove.bindValue(0, removePaths);
    q.remove.execBatch();
  }
}

void MediaDatabase::probeFiles(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!filesToProbe.isEmpty())
  {
    QSet<QString>::Iterator i = filesToProbe.begin();
    const QString path =
#ifndef Q_OS_WIN
        *i;
#else
        i->toLower();
#endif

    filesToProbe.erase(i);
    if (!filesToProbe.isEmpty())
    {
      threadPool->start(new Task(this, &MediaDatabase::probeFiles), -1);
    }
    else if (!imdbItemsToMatch.isEmpty() && !matchingImdbItems)
    {
      matchingImdbItems = true;
      threadPool->start(new Task(this, &MediaDatabase::matchImdbItems), -1);
    }

    l.unlock();

    if (!path.isEmpty())
    {
      // Only scan files if they can be opened (to prevent scanning files that
      // are still being copied).
      if (QFile(path).open(QFile::ReadOnly))
      {
        qDebug() << "Probing:" << path;

        const SMediaInfo mediaInfo(path);
        const QByteArray mediaInfoXml = mediaInfo.toByteArray(-1);

        SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
        QSqlDatabase db = Database::database();
        db.transaction();

        QSqlQuery query(db);
        query.prepare("UPDATE MediaplayerFiles "
                      "SET size = :size, mediaInfo = :mediaInfo "
                      "WHERE path = :path");
        query.bindValue(0, qMax(Q_INT64_C(0), mediaInfo.size()));
        query.bindValue(1, mediaInfoXml);
        query.bindValue(2, path);
        query.exec();

        if (mediaInfo.isProbed() && mediaInfo.isReadable())
        {
          // Find the rowId and categorize the file
          query.prepare("SELECT uid, parentDir FROM MediaplayerFiles WHERE path = :path");
          query.bindValue(0, path);
          query.exec();

          if (query.next())
          {
            const qint64 rowId = query.value(0).toLongLong();
            const qint64 parentDirId = query.value(1).toLongLong();

            const QMap<Category, QString> categories = findCategories(path);
            for (QMap<Category, QString>::ConstIterator i=categories.begin(); i!=categories.end(); i++)
            {
              if ((i.key() == Category_Movies) || (i.key() == Category_HomeVideos) ||
                  (i.key() == Category_Clips) || (i.key() == Category_TVShows))
              if (!mediaInfo.containsAudio() || !mediaInfo.containsVideo())
                continue;

              if (i.key() == Category_Movies)
              if (mediaInfo.duration().isValid() && (mediaInfo.duration().toMin() < 5))
                continue;

              if (i.key() == Category_Music)
              if (!mediaInfo.containsAudio())
                continue;

              if (i.key() == Category_Photos)
              if (!mediaInfo.containsImage())
                continue;

              // Find or create the album id.
              qint64 albumId = -1;
              query.prepare("SELECT id FROM MediaplayerAlbums "
                            "WHERE name = :name AND category = :category");
              query.bindValue(0, i.value());
              query.bindValue(1, i.key());
              query.exec();
              if (!query.next())
              {
                query.prepare("INSERT INTO MediaplayerAlbums "
                              "VALUES (:id, :parentDir, :category, :name)");
                query.bindValue(0, QVariant(QVariant::LongLong));
                query.bindValue(1, parentDirId);
                query.bindValue(2, i.key());
                query.bindValue(3, i.value());
                query.exec();

                query.prepare("SELECT id FROM MediaplayerAlbums "
                              "WHERE name = :name AND category = :category");
                query.bindValue(0, i.value());
                query.bindValue(1, i.key());
                query.exec();
                if (query.next())
                  albumId = query.value(0).toLongLong();
              }
              else
                albumId = query.value(0).toLongLong();

              if (albumId != -1)
              {
                QString rawTitle = SStringParser::toRawName(mediaInfo.title());
                if ((i.key() == Category_TVShows) || (i.key() == Category_Music))
                  rawTitle = ("000000000" + QString::number(mediaInfo.track())).right(10) + rawTitle;

                query.prepare("INSERT INTO MediaplayerItems "
                              "VALUES (:file, :album, :title, :subtitle, :imdbLink)");
                query.bindValue(0, rowId);
                query.bindValue(1, albumId);
                query.bindValue(2, rawTitle);
                query.bindValue(3, SStringParser::toRawName(mediaInfo.album()));
                query.bindValue(4, QVariant(QVariant::String));
                query.exec();
              }
            }
          }
        }

        db.commit();
      }
      else
      {
        SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
        QSqlQuery query(Database::database());
        query.prepare("SELECT size FROM MediaplayerFiles WHERE path = :path");
        query.bindValue(0, path);
        query.exec();
        if (query.next())
        {
          const qint64 size = query.value(0).toLongLong();

          if (size == -1)
            qDebug() << "File" << path << "can not be opened, scanning later";

          if (size > -60) // Try 60 times (~1 hour), otherwise fail forever.
          {
            query.prepare("UPDATE MediaplayerFiles "
                          "SET size = :size "
                          "WHERE path = :path");
            query.bindValue(0, qMin(size - 1, Q_INT64_C(-1)));
            query.bindValue(1, path);
            query.exec();
          }
          else // Set the correct size so the file is not probed anymore.
          {
            qDebug() << "File" << path << "can not be opened, not scanning at all";

            query.prepare("UPDATE MediaplayerFiles "
                          "SET size = :size "
                          "WHERE path = :path");
            query.bindValue(0, QFileInfo(path).size());
            query.bindValue(1, path);
            query.exec();
          }
        }
      }
    }
  }
  else if (!imdbItemsToMatch.isEmpty() && !matchingImdbItems)
  {
    matchingImdbItems = true;
    threadPool->start(new Task(this, &MediaDatabase::matchImdbItems), -1);
  }
}

void MediaDatabase::matchImdbItems(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!imdbItemsToMatch.isEmpty())
  {
    QSet<QString>::Iterator i = imdbItemsToMatch.begin();
    const QString item = *i;
    imdbItemsToMatch.erase(i);
    l.unlock();

    if (ImdbClient::isAvailable() && (item.length() >= 2))
    {
      SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
      QSqlQuery query(Database::database());

      Category category = Category_None;
      if (item[0] == 'M')
        category = Category_Movies;
      else if (item[0] == 'T')
        category = Category_TVShows;

      if (category != Category_None)
      {
        const ImdbClient::Type imdbType = category == Category_Movies ? ImdbClient::Type_Movie : ImdbClient::Type_TvShow;

        query.prepare("SELECT file FROM MediaplayerItems "
                      "WHERE album IN ("
                        "SELECT id FROM MediaplayerAlbums "
                        "WHERE category = :category) "
                      "AND title = :title");
        query.bindValue(0, category);
        query.bindValue(1, item.mid(1));
        query.exec();
        if (query.next())
        {
          const SMediaInfo node = readNode(query.value(0).toULongLong());
          if (!node.isNull())
          {
            dl.unlock();
            const QString imdbLink = ImdbClient::findEntry(node.title(), imdbType);
            dl.relock(__FILE__, __LINE__);

            query.prepare("UPDATE MediaplayerItems SET imdbLink = :imdbLink "
                          "WHERE album IN ("
                            "SELECT id FROM MediaplayerAlbums "
                            "WHERE category = :category) "
                          "AND title = :title");
            query.bindValue(0, imdbLink);
            query.bindValue(1, category);
            query.bindValue(2, item.mid(1));
            query.exec();
          }
        }
      }
    }

    l.relock(__FILE__, __LINE__);
    if (!imdbItemsToMatch.isEmpty())
      threadPool->start(new Task(this, &MediaDatabase::matchImdbItems), -1);
    else
      matchingImdbItems = false;
  }
}

QMap<MediaDatabase::Category, QString> MediaDatabase::findCategories(const QString &path) const
{
  QMap<Category, QString> result;

  for (int i=0; categories[i].name; i++)
  {
    QMap<QString, QStringList>::ConstIterator rootPath = rootPaths.find(categories[i].name);
    if (rootPath != rootPaths.end())
    foreach (const QString &root, *rootPath)
    if (path.startsWith(root))
    {
      QString album = path.mid(root.length());
      if (album.endsWith("/VIDEO_TS/VIDEO_TS.IFO", Qt::CaseInsensitive))
        album = album.left(album.length() - 22);

      album = album.left(album.lastIndexOf('/') + 1);
      album = album.startsWith('/') ? album : ('/' + album);

      result[categories[i].category] = album;
      break;
    }
  }

  return result;
}

void MediaDatabase::Task::run(void)
{
  (parent->*func)();
}

} // End of namespace
