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
    invalidatedClips(1),
    invalidatedHomeVideos(1),
    invalidatedMovies(1),
    invalidatedMusic(1),
    invalidatedPhotos(1),
    invalidatedTvShows(1),
    matchingImdbItems(false)
{
  PluginSettings settings(plugin);

  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);
  QSqlDatabase db = Database::database();
  QSqlQuery query(db);

  // Drop all tables if the database version is outdated.
  static const int databaseVersion = 1;
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

MediaDatabase::Node MediaDatabase::readNode(UniqueID uid) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.exec("SELECT path, size, lastModified, mediaInfo "
             "FROM MediaplayerFiles WHERE uid = " + QString::number(uid));
  if (query.next())
  {
    Node node;
    node.uid = uid;
    node.path = query.value(0).toString();
    node.size = query.value(1).toLongLong();
    node.lastModified = query.value(2).toDateTime();
    node.mediaInfo.fromByteArray(query.value(3).toByteArray(), node.path);

    return node;
  }

  return Node();
}

void MediaDatabase::setLastPlayed(UniqueID uid, const QDateTime &lastPlayed)
{
  setLastPlayed(readNode(uid), lastPlayed);
}

void MediaDatabase::setLastPlayed(const Node &node, const QDateTime &lastPlayed)
{
  QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

  QString key = node.path;
  key.replace('/', '|');
  key.replace('\\', '|');

  if (lastPlayed.isValid())
    settings.setValue(key, lastPlayed);
  else
    settings.remove(key);
}

QDateTime MediaDatabase::lastPlayed(UniqueID uid) const
{
  return lastPlayed(readNode(uid));
}

QDateTime MediaDatabase::lastPlayed(const Node &node) const
{
  QSettings settings(GlobalSettings::applicationDataDir() + "/lastplayed.db", QSettings::IniFormat);

  QString key = node.path;
  key.replace('/', '|');
  key.replace('\\', '|');

  return settings.value(key, QDateTime()).toDateTime();
}

QStringList MediaDatabase::allMovies(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerMovies ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allMovieFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerMovies WHERE rawName = :rawName");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

ImdbClient::Entry MediaDatabase::getMovieImdbEntry(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT imdbLink FROM MediaplayerMovies WHERE rawName = :rawName");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
  if (!query.value(0).isNull() && (query.value(0).toString() != ImdbClient::sentinelItem))
    return ImdbClient::readEntry(query.value(0).toString());

  return ImdbClient::Entry();
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryMovieFiles(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs;
  foreach (const QString &item, q)
    qs += " AND rawName LIKE '%" + SStringParser::toRawName(item) + "%'";

  if (!qs.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerMovies "
               "WHERE" + qs.mid(4));
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

QStringList MediaDatabase::allTvShows(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerTvShows ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allTvShowEpisodes(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerTvShows WHERE rawName = :rawName");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

ImdbClient::Entry MediaDatabase::getTvShowImdbEntry(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT imdbLink FROM MediaplayerTvShows WHERE rawName = :rawName");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
  if (!query.value(0).isNull() && (query.value(0).toString() != ImdbClient::sentinelItem))
    return ImdbClient::readEntry(query.value(0).toString());

  return ImdbClient::Entry();
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryTvShows(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawName LIKE '%" + rawItem + "%'";
    qs2 += " AND rawEpisode LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerTvShows "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

QStringList MediaDatabase::allHomeVideos(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerHomeVideos ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allHomeVideoFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerHomeVideos WHERE rawName = :rawName");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryHomeVideos(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawName LIKE '%" + rawItem + "%'";
    qs2 += " AND rawFile LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerHomeVideos "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

QStringList MediaDatabase::allVideoClipAlbums(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerVideoClipAlbums ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allVideoClipFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerVideoClipAlbums WHERE rawName = :rawName ORDER BY rawFile");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryVideoClips(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawName LIKE '%" + rawItem + "%'";
    qs2 += " AND rawFile LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerVideoClipAlbums "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

QStringList MediaDatabase::allPhotoAlbums(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerPhotoAlbums ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allPhotoFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerPhotoAlbums WHERE rawName = :rawName ORDER BY rawFile");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryPhotoAlbums(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawName LIKE '%" + rawItem + "%'";
    qs2 += " AND rawFile LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerPhotoAlbums "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

unsigned MediaDatabase::numSongs(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.exec("SELECT COUNT(*) FROM MediaplayerMusic");
  if (query.next())
    return query.value(0).toUInt();

  return 0;
}

MediaDatabase::UniqueID MediaDatabase::getSong(unsigned offset) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QSqlQuery query(Database::database());
  query.exec("SELECT file FROM MediaplayerMusic LIMIT 1 OFFSET " + QString::number(offset));
  if (query.next())
    return query.value(0).toULongLong();

  return 0;
}

QList<MediaDatabase::UniqueID> MediaDatabase::latestSongs(unsigned count) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT file FROM "
             "MediaplayerMusic INNER JOIN MediaplayerFiles ON (MediaplayerMusic.file = MediaplayerFiles.uid) "
             "ORDER BY lastModified DESC LIMIT " + QString::number(count));
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QStringList MediaDatabase::allMusicArtists(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawArtist FROM MediaplayerMusic ORDER BY rawArtist");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allMusicArtistFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerMusic WHERE rawArtist = :rawArtist");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QStringList MediaDatabase::allMusicGenres(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawGenre FROM MediaplayerMusic ORDER BY rawGenre");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allMusicGenreFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerMusic WHERE rawGenre = :rawGenre");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}
QStringList MediaDatabase::allMusicVideoAlbums(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT rawArtist FROM MediaplayerMusic WHERE hasVideo = :hasVideo ORDER BY rawArtist");
  query.bindValue(0, true);
  query.exec();
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allMusicVideoFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerMusic WHERE hasVideo = :hasVideo AND rawArtist = :rawArtist ORDER BY rawTitle");
  query.bindValue(0, true);
  query.bindValue(1, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryMusic(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2, qs3;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawTitle LIKE '%" + rawItem + "%'";
    qs2 += " AND rawArtist LIKE '%" + rawItem + "%'";
    qs3 += " AND rawGenre LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty() && !qs3.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerMusic "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ") OR (" + qs3.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

QString MediaDatabase::genreName(const QString &name)
{
  QString result = "";
  foreach (QChar c, name.simplified())
  if (c.isLetter() || c.isSpace())
    result += c;

  result = result.simplified();

  if ((result.length() == 0) || (result.toUpper() == "OTHER") ||
      (result.toUpper() == "MISC") || (result.toUpper() == "SOUNDTRACK"))
  {
   result = "(Other)";
  }

  return result;
}

QStringList MediaDatabase::allMiscAudioAlbums(void) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QStringList result;

  QSqlQuery query(Database::database());
  query.exec("SELECT DISTINCT rawName FROM MediaplayerMiscAudio ORDER BY rawName");
  while (query.next())
    result << query.value(0).toString();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::allMiscAudioFiles(const QString &key) const
{
  SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

  QList<UniqueID> result;

  QSqlQuery query(Database::database());
  query.prepare("SELECT DISTINCT file FROM MediaplayerMiscAudio WHERE rawName = :rawName ORDER BY rawFile");
  query.bindValue(0, key);
  query.exec();
  while (query.next())
    result << query.value(0).toLongLong();

  return result;
}

QList<MediaDatabase::UniqueID> MediaDatabase::queryMiscAudioAlbums(const QStringList &q) const
{
  QList<UniqueID> result;

  QString qs1, qs2;
  foreach (const QString &item, q)
  {
    const QString rawItem = SStringParser::toRawName(item);
    qs1 += " AND rawName LIKE '%" + rawItem + "%'";
    qs2 += " AND rawFile LIKE '%" + rawItem + "%'";
  }

  if (!qs1.isEmpty() && !qs2.isEmpty())
  {
    SDebug::MutexLocker dl(&(Database::mutex()), __FILE__, __LINE__);

    QSqlQuery query(Database::database());
    query.exec("SELECT file FROM MediaplayerMiscAudio "
               "WHERE (" + qs1.mid(5) + ") OR (" + qs2.mid(5) + ")");
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
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


} // End of namespace
