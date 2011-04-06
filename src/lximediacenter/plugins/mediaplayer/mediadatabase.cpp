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
#include "mediaplayersandbox.h"
#include "module.h"
#include <LXiStreamGui>

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

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

MediaDatabase * MediaDatabase::self = NULL;

MediaDatabase * MediaDatabase::createInstance(BackendServer::MasterServer *masterServer)
{
  if (self)
    return self;
  else
    return self = new MediaDatabase(masterServer);
}

void MediaDatabase::destroyInstance(void)
{
  delete self;
}

MediaDatabase::MediaDatabase(BackendServer::MasterServer *masterServer, QObject *parent)
  : QObject(parent),
    imdbClient(masterServer->imdbClient()),
    probeSandbox(masterServer->createSandbox(SSandboxClient::Mode_Nice)),
    probeDependency(sApp),
    maxProbeCount(QThread::idealThreadCount() * 2),
    probeCount(0)
{
  PluginSettings settings(Module::pluginName);

  SScheduler::DependencyLocker dl(Database::dependency());
  Database::Query query;

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

  connect(probeSandbox, SIGNAL(response(SHttpEngine::ResponseMessage)), SLOT(probeFinished(SHttpEngine::ResponseMessage)));

  connect(&fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(directoryChanged(QString)));
  
  QTimer::singleShot(3000, this, SLOT(initialize()));
}

MediaDatabase::~MediaDatabase()
{
  self = NULL;

  sApp->waitForDone();
  delete probeSandbox;
}

QByteArray MediaDatabase::toUidString(UniqueID uid)
{
  return
      QByteArray::number(quint64(uid.fid) | Q_UINT64_C(0x8000000000000000), 16) +
      '-' + ("000" + QByteArray::number(uid.pid, 16)).right(4);
}

MediaDatabase::UniqueID MediaDatabase::fromUidString(const QByteArray &str)
{
  UniqueID result;
  if ((str.length() >= 21) && (str[16] == '-'))
  {
    result.fid = str.left(16).toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF);
    result.pid = str.mid(17, 4).toUShort(NULL, 16);
  }

  return result;
}

MediaDatabase::UniqueID MediaDatabase::fromUidString(const QString &str)
{
  UniqueID result;
  if ((str.length() >= 21) && (str[16] == '-'))
  {
    result.fid = str.left(16).toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF);
    result.pid = str.mid(17, 4).toUShort(NULL, 16);
  }

  return result;
}

MediaDatabase::UniqueID MediaDatabase::fromPath(const QString &path) const
{
  SScheduler::DependencyLocker dl(Database::dependency());

  Database::Query query;
  query.prepare("SELECT uid FROM MediaplayerFiles WHERE path = :path");
  query.bindValue(0, path);
  query.exec();
  if (query.next())
    return query.value(0).toLongLong();

  return 0;
}

SMediaInfo MediaDatabase::readNode(UniqueID uid) const
{
  SScheduler::DependencyLocker dl(Database::dependency());
  const QByteArray value = readNodeData(uid);
  dl.unlock();

  if (!value.isEmpty())
  {
    SMediaInfo node;
    node.fromByteArray(value);

    return node;
  }

  return SMediaInfo();
}

QByteArray MediaDatabase::readNodeData(UniqueID uid) const
{
  Database::Query query;
  query.exec("SELECT mediaInfo "
             "FROM MediaplayerFiles WHERE uid = " + QString::number(uid.fid));
  if (query.next())
    return query.value(0).toByteArray();

  return QByteArray();
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
    SScheduler::DependencyLocker dl(Database::dependency());

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
  SScheduler::DependencyLocker dl(Database::dependency());

  QStringList result;

  Database::Query query;
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
  SScheduler::DependencyLocker dl(Database::dependency());

  Database::Query query;
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
  SScheduler::DependencyLocker dl(Database::dependency());

  Database::Query query;
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
  SScheduler::DependencyLocker dl(Database::dependency());

  QString limit;
  if (count > 0)
  {
    limit += " LIMIT " + QString::number(count);
    if (start > 0)
      limit += " OFFSET " + QString::number(start);
  }

  QList<File> result;

  Database::Query query;
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
    SScheduler::DependencyLocker dl(Database::dependency());

    Database::Query query;
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
  if (imdbClient)
  {
    SScheduler::DependencyLocker dl(Database::dependency());

    Database::Query query;
    query.prepare("SELECT imdbLink FROM MediaplayerItems WHERE file = :file");
    query.bindValue(0, uid.fid);
    query.exec();
    while (query.next())
    if (!query.value(0).isNull() && (query.value(0).toString() != ImdbClient::sentinelItem))
      return imdbClient->readEntry(query.value(0).toString());
  }

  return ImdbClient::Entry();
}

QList<MediaDatabase::UniqueID> MediaDatabase::allFilesInDirOf(UniqueID uid) const
{
  QList<UniqueID> result;

  Database::Query query;
  query.exec("SELECT parentDir FROM MediaplayerFiles WHERE uid = " + QString::number(uid.fid));
  if (query.next())
  {
    query.exec("SELECT uid FROM MediaplayerFiles WHERE parentDir = " + QString::number(query.value(0).toLongLong()));
    while (query.next())
      result << query.value(0).toLongLong();
  }

  return result;
}

void MediaDatabase::initialize(void)
{
  PluginSettings settings(Module::pluginName);

  QStringList allRootPaths;
  foreach (const QString &group, settings.childGroups())
  {
    settings.beginGroup(group);

    QStringList paths;
    foreach (const QString &root, settings.value("Paths").toStringList())
    if (!root.trimmed().isEmpty() && !isHidden(root) && QFileInfo(root).exists())
      paths.append(root);

    settings.setValue("Paths", paths);
    rootPaths[group] = paths;
    allRootPaths += paths;

    settings.endGroup();
  }

  SScheduler::DependencyLocker dl(Database::dependency());

  Database::transaction();
  Database::Query query;

  // Scan all root paths recursively
  foreach (const QString &path, allRootPaths)
    sApp->schedule(this, &MediaDatabase::scanDir, path, Database::dependency(), scanDirPriority);

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
    sApp->schedule(this, &MediaDatabase::probeFile, query.value(0).toString(), &probeDependency, probeFilePriority);

  // Find IMDB items that still need to be matched.
  if (imdbClient)
  if (imdbClient->isAvailable())
  {
    query.prepare("SELECT title FROM MediaplayerItems "
                  "WHERE album IN ("
                    "SELECT id FROM MediaplayerAlbums "
                    "WHERE category = :category) "
                  "AND imdbLink ISNULL");
    query.bindValue(0, Category_Movies);
    query.exec();
    while (query.next())
      sApp->schedule(this, &MediaDatabase::queryImdbItem, query.value(0).toString(), Category_Movies, Database::dependency(), matchImdbItemPriority);

//      query.prepare("SELECT title FROM MediaplayerItems "
//                    "WHERE album IN ("
//                      "SELECT id FROM MediaplayerAlbums "
//                      "WHERE category = :category) "
//                    "AND imdbLink ISNULL");
//      query.bindValue(0, Category_TVShows);
//      query.exec();
//      while (query.next())
//        sApp->run(this, &MediaDatabase::queryImdbItem, query.value(0).toString(), Category_TVShows, Database::dependency(), matchImdbItemPriority);
  }

  Database::commit();
}

void MediaDatabase::directoryChanged(const QString &path)
{
  sApp->schedule(this, &MediaDatabase::scanDir, path, Database::dependency(), scanDirPriority);
}

void MediaDatabase::probeFinished(const SHttpEngine::ResponseMessage &message)
{
  if ((message.status() == SHttpEngine::Status_Ok) && !message.content().isEmpty())
  {
    SMediaInfo mediaInfo;
    mediaInfo.fromByteArray(message.content());

    sApp->schedule(this, &MediaDatabase::insertFile, mediaInfo, message.content(), Database::dependency(), insertFilePriority);
  }
  else
  {
    qDebug() << "probeFinished" << message.status();
  }

  SScheduler::DependencyLocker l(&probeDependency);

  if (!probeQueue.isEmpty())
  {
    qDebug() << "Probing:" << probeQueue.first();

    SSandboxClient::RequestMessage request(probeSandbox);
    request.setRequest("GET", QByteArray(MediaPlayerSandbox::path) + "?probe=" + probeQueue.takeFirst().toUtf8().toHex());
    request.setHost(probeSandbox->serverName());
    probeSandbox->sendRequest(request);
  }
  else
    probeCount--;
}

QString MediaDatabase::findRoot(const QString &path, const QStringList &allRootPaths) const
{
  const QFileInfo info(path);

#ifndef Q_OS_WIN
  const QString absoluteFilePath = info.absoluteFilePath();
#else
  const QString absoluteFilePath = QDir::toNativeSeparators(info.absoluteFilePath()).toLower();
#endif

  foreach (const QString &root, allRootPaths)
#ifndef Q_OS_WIN
  if ((root == absoluteFilePath) || absoluteFilePath.startsWith(root + "/"))
    return root.endsWith('/') ? root : (root + '/');
#else
  {
    const QString croot = QDir::toNativeSeparators(root).toLower();
    if ((croot == absoluteFilePath) || absoluteFilePath.startsWith(croot + "\\"))
      return root.endsWith('/') ? root : (root + '/');
  }
#endif

  return QString::null;
}

struct MediaDatabase::QuerySet
{
  Database::Query request;
  Database::Query insert;
  Database::Query update;
  Database::Query children;
  Database::Query remove;
};

void MediaDatabase::scanDir(const QString &_path)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  // Ensure directories end with a '/'
  const QString path =
      (_path.endsWith('/') ? _path : (_path + '/'))
#ifdef Q_OS_WIN
      .toLower()
#endif
      ;

  if (!isHidden(path))
  {
    Database::transaction();

    QuerySet q;
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

    const QFileInfo info(path);

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

        if (!fileSystemWatcher.directories().contains(path))
          fileSystemWatcher.addPath(path);

        updateDir(path, rowId, q);
      }
      else if (!fileSystemWatcher.directories().contains(path))
      {
        fileSystemWatcher.addPath(path);

        foreach (const QFileInfo &child, dir.entryInfoList(QDir::Dirs))
        if (!child.fileName().startsWith('.'))
          sApp->schedule(this, &MediaDatabase::scanDir, child.absoluteFilePath(), Database::dependency(), scanDirPriority);
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

      // Scan again.
      sApp->schedule(this, &MediaDatabase::scanDir, path, Database::dependency(), scanDirPriority);
    }

    Database::commit();
  }
}

void MediaDatabase::updateDir(const QString &path, qint64 parentDir, QuerySet &q)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  qDebug() << "Scanning:" << path;

  QSet<QString> childPaths;

  // Update existing and add new dirs.
  foreach (const QFileInfo &child, QDir(path).entryInfoList(QDir::Dirs))
  if (!child.fileName().startsWith('.'))
  {
    QString childPath = child.absoluteFilePath()
#ifdef Q_OS_WIN
        .toLower()
#endif
        ;

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

    sApp->schedule(this, &MediaDatabase::scanDir, childPath, Database::dependency(), scanDirPriority);
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
        child.absoluteFilePath();
#else
        child.absoluteFilePath().toLower();
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

      sApp->schedule(this, &MediaDatabase::probeFile, childPath, &probeDependency, probeFilePriority);
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

      sApp->schedule(this, &MediaDatabase::probeFile, childPath, &probeDependency, probeFilePriority);
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

    emit modified();
  }
}

void MediaDatabase::insertFile(const SMediaInfo &mediaInfo, const QByteArray &mediaInfoXml)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  Database::transaction();

  Database::Query query;
  query.prepare("UPDATE MediaplayerFiles "
                "SET size = :size, mediaInfo = :mediaInfo "
                "WHERE path = :path");
  query.bindValue(0, qMax(Q_INT64_C(0), mediaInfo.size()));
  query.bindValue(1, mediaInfoXml);
  query.bindValue(2, mediaInfo.filePath());
  query.exec();

  if (mediaInfo.isProbed() && mediaInfo.isReadable())
  {
    // Find the rowId and categorize the file
    query.prepare("SELECT uid, parentDir FROM MediaplayerFiles WHERE path = :path");
    query.bindValue(0, mediaInfo.filePath());
    query.exec();

    if (query.next())
    {
      const qint64 rowId = query.value(0).toLongLong();
      const qint64 parentDirId = query.value(1).toLongLong();

      const QMap<Category, QString> categories = findCategories(mediaInfo.filePath());
      for (QMap<Category, QString>::ConstIterator i=categories.begin(); i!=categories.end(); i++)
      {
        if ((i.key() == Category_Movies) || (i.key() == Category_HomeVideos) ||
            (i.key() == Category_Clips) || (i.key() == Category_TVShows))
        if (!mediaInfo.containsAudio() || !mediaInfo.containsVideo())
          continue;

        if (i.key() == Category_Movies)
        if (mediaInfo.totalDuration().isValid() && (mediaInfo.totalDuration().toMin() < 5))
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

          if (i.key() == Category_Movies)
            sApp->schedule(this, &MediaDatabase::queryImdbItem, rawTitle, i.key(), Database::dependency(), matchImdbItemPriority);

          emit modified();
        }
      }
    }
  }

  Database::commit();
}

// Ensure directories end with a '/'
void MediaDatabase::probeFile(
#ifndef Q_OS_WIN
    const QString &path)
{
#else
    const QString &_path)
{
  const QString path = _path.toLower();
#endif

  Q_ASSERT(!probeDependency.tryLock()); // Mutex should be locked by the caller.

  if (!path.isEmpty() && !isHidden(path))
  {
    // Only scan files if they can be opened (to prevent scanning files that
    // are still being copied).
    if (QFile(path).open(QFile::ReadOnly))
    {
      if (probeCount < maxProbeCount)
      {
        qDebug() << "Probing:" << path;

        SSandboxClient::RequestMessage request(probeSandbox);
        request.setRequest("GET", QByteArray(MediaPlayerSandbox::path) + "?probe=" + path.toUtf8().toHex());
        request.setHost(probeSandbox->serverName());
        probeSandbox->sendRequest(request);

        probeCount++;
      }
      else
        probeQueue.append(path);
    }
    else
      sApp->schedule(this, &MediaDatabase::delayFile, path, Database::dependency(), insertFilePriority);
  }
}

void MediaDatabase::delayFile(const QString &path)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  Database::Query query;
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

void MediaDatabase::queryImdbItem(const QString &item, Category category)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  Database::Query query;

  if (category != Category_None)
  {
    const ImdbClient::Type imdbType = category == Category_Movies ? ImdbClient::Type_Movie : ImdbClient::Type_TvShow;

    query.prepare("SELECT file FROM MediaplayerItems "
                  "WHERE album IN ("
                    "SELECT id FROM MediaplayerAlbums "
                    "WHERE category = :category) "
                  "AND title = :title");
    query.bindValue(0, category);
    query.bindValue(1, item);
    query.exec();
    if (query.next())
    {
      const QByteArray value = readNodeData(query.value(0).toULongLong());
      if (!value.isEmpty())
      {
        SMediaInfo node;
        node.fromByteArray(value);
        if (!node.isNull())
        {
          const QStringList similar = imdbClient->findSimilar(node.title(), imdbType);
          if (!similar.isEmpty())
            sApp->schedule(this, &MediaDatabase::matchImdbItem, item, node.title(), similar, category, NULL, storeImdbItemPriority);
        }
      }
    }
  }
}

void MediaDatabase::matchImdbItem(const QString &item, const QString &title, const QStringList &similar, Category category)
{
  const QString imdbLink = imdbClient->findBest(title, similar);

  sApp->schedule(this, &MediaDatabase::storeImdbItem, item, imdbLink, category, Database::dependency(), storeImdbItemPriority);
}

void MediaDatabase::storeImdbItem(const QString &item, const QString &imdbLink, Category category)
{
  Q_ASSERT(!Database::dependency()->tryLock()); // Mutex should be locked by the caller.

  Database::Query query;

  query.prepare("UPDATE MediaplayerItems SET imdbLink = :imdbLink "
                "WHERE album IN ("
                  "SELECT id FROM MediaplayerAlbums "
                  "WHERE category = :category) "
                "AND title = :title");
  query.bindValue(0, imdbLink);
  query.bindValue(1, category);
  query.bindValue(2, item);
  query.exec();

  emit modified();
}

bool MediaDatabase::isHidden(const QString &absoluteFilePath)
{
  const QFileInfo info(absoluteFilePath);
  const QString canonicalFilePath =
      (info.exists() ? info.canonicalFilePath() : absoluteFilePath)
#ifdef Q_OS_WIN
      .toLower()
#endif
      ;

  foreach (const QString &hidden, ConfigServer::hiddenDirs())
  if ((absoluteFilePath == hidden) || absoluteFilePath.startsWith(hidden + "/") ||
      (canonicalFilePath == hidden) || canonicalFilePath.startsWith(hidden + "/"))
  {
    return true;
  }

  return false;
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

} } // End of namespaces
