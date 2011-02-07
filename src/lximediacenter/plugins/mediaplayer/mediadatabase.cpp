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
             "path           TEXT UNIQUE NOT NULL,"
             "parentDir      INTEGER,"
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
             "category       TEXT NOT NULL,"
             "album          TEXT NOT NULL,"
             "title          TEXT,"
             "subtitle       TEXT,"
             "file           INTEGER NOT NULL,"
             "imdbLink       TEXT,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE,"
             "FOREIGN KEY(imdbLink) REFERENCES ImdbEntries(rawName) ON DELETE SET NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerAlbums_category "
             "ON MediaplayerAlbums(category)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerAlbums_album "
             "ON MediaplayerAlbums(album)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerAlbums_title "
             "ON MediaplayerAlbums(title)");

/*
  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerMovies ("
             "rawName        TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "imdbLink       TEXT,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE,"
             "FOREIGN KEY(imdbLink) REFERENCES ImdbEntries(rawName) ON DELETE SET NULL)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerTvShows ("
             "rawName        TEXT NOT NULL,"
             "rawEpisode     TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "imdbLink       TEXT,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE,"
             "FOREIGN KEY(imdbLink) REFERENCES ImdbEntries(rawName) ON DELETE SET NULL)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerTvShows_rawName "
             "ON MediaplayerTvShows(rawName)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerHomeVideos ("
             "rawName        TEXT NOT NULL,"
             "rawFile        TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerHomeVideos_rawName "
             "ON MediaplayerHomeVideos(rawName)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerVideoClipAlbums ("
             "rawName        TEXT NOT NULL,"
             "rawFile        TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerVideoClipAlbums_rawName "
             "ON MediaplayerVideoClipAlbums(rawName)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerPhotoAlbums ("
             "rawName        TEXT NOT NULL,"
             "rawFile        TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerPhotoAlbums_rawName "
             "ON MediaplayerPhotoAlbums(rawName)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerMusic ("
             "rawTitle       TEXT NOT NULL,"
             "rawArtist      TEXT NOT NULL,"
             "rawGenre       TEXT NOT NULL,"
             "hasVideo       BIT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerMusic_rawTitle "
             "ON MediaplayerMusic(rawTitle)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerMusic_rawArtist "
             "ON MediaplayerMusic(rawArtist)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerMusic_rawGenre "
             "ON MediaplayerMusic(rawGenre)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerMusic_hasVideo "
             "ON MediaplayerMusic(hasVideo)");

  query.exec("CREATE TABLE IF NOT EXISTS MediaplayerMiscAudio ("
             "rawName        TEXT NOT NULL,"
             "rawFile        TEXT NOT NULL,"
             "file           INTEGER NOT NULL,"
             "FOREIGN KEY(file) REFERENCES MediaplayerFiles(uid) ON DELETE CASCADE)");

  query.exec("CREATE INDEX IF NOT EXISTS MediaplayerMiscAudio_rawName "
             "ON MediaplayerMiscAudio(rawName)");
*/
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
  setLastPlayed(readNode(uid), lastPlayed);
}

void MediaDatabase::setLastPlayed(const SMediaInfo &node, const QDateTime &lastPlayed)
{
  if (!node.isNull())
  {
    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = node.filePath();
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
  return lastPlayed(readNode(uid));
}

QDateTime MediaDatabase::lastPlayed(const SMediaInfo &node) const
{
  if (!node.isNull())
  {
    QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

    QString key = node.filePath();
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
  query.prepare("SELECT DISTINCT album FROM MediaplayerAlbums WHERE category = :category ORDER BY album");
  query.bindValue(0, categoryName(category));
  query.exec();
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allAlbumFiles(Category category, const QString &album) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerAlbums WHERE category = :category AND album = :album");
  query.bindValue(0, categoryName(category));
  query.bindValue(1, album);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

MediaDatabase::UniqueID MediaDatabase::getAlbumFile(Category category, const QString &album, int offset) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT file FROM MediaplayerAlbums WHERE category = :category AND album = :album LIMIT 1 OFFSET :offset");
  query.bindValue(0, categoryName(category));
  query.bindValue(1, album);
  query.bindValue(2, offset);
  query.exec();
  if (query.next())
    return query.value(0).toLongLong();

  return 0;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryAlbums(Category category, const QStringList &q) const
{
  QList<UniqueID> result;

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
    query.prepare("SELECT file FROM MediaplayerAlbums WHERE category = :category AND "
                  "((" + qs1.mid(5) + ") OR (" + qs2.mid(5) + "))");
    query.bindValue(0, categoryName(category));
    query.exec();
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

ImdbClient::Entry MediaDatabase::getImdbEntry(UniqueID uid) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.prepare("SELECT imdbLink FROM MediaplayerAlbums WHERE file = :file");
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

const char * MediaDatabase::categoryName(Category category)
{
  switch (category)
  {
  case Category_None:       break;
  case Category_Movies:     return "Movies";
  case Category_TVShows:    return "TVShows";
  case Category_Clips:      return "Clips";
  case Category_HomeVideos: return "HomeVideos";
  case Category_Photos:     return "Photos";
  case Category_Music:      return "Music";
  }

  return "";
}


} // End of namespace
