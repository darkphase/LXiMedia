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

const char  * const MediaDatabase::categoryNames[7] = { "none", "movies", "tvshows", "clips", "homevideos", "photos", "music" };
MediaDatabase     * MediaDatabase::self = NULL;

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
    probeSandbox(masterServer->createSandbox(SSandboxClient::Priority_Low))
{
  PluginSettings settings(Module::pluginName);

  Database::Query query;

  // Drop all tables if the database version is outdated.
  static const int databaseVersion = 4;
  //if (settings.value("DatabaseVersion", 0).toInt() != databaseVersion)
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

    //query.exec("VACUUM");
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
/*
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
             "file           INTEGER UNIQUE NOT NULL,"
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
*/
  settings.setValue("DatabaseVersion", databaseVersion);
}

MediaDatabase::~MediaDatabase()
{
  self = NULL;

  delete probeSandbox;
}

QByteArray MediaDatabase::toUidString(UniqueID uid)
{
  return QByteArray::number(uid | Q_UINT64_C(0x8000000000000000), 16);
}

MediaDatabase::UniqueID MediaDatabase::fromUidString(const QByteArray &str)
{
  UniqueID result = 0;
  if (str.length() >= 16)
    result = str.left(16).toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF);

  return result;
}

MediaDatabase::UniqueID MediaDatabase::fromUidString(const QString &str)
{
  UniqueID result = 0;
  if (str.length() >= 16)
    result = str.left(16).toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF);

  return result;
}

MediaDatabase::UniqueID MediaDatabase::fromPath(const QString &path) const
{
  Database::Query query;
  query.prepare("SELECT uid FROM MediaplayerFiles WHERE path = :path");
  query.bindValue(0, QDir::cleanPath(path));
  query.exec();
  if (query.next())
    return query.value(0).toLongLong();

  return 0;
}

FileNode MediaDatabase::readNode(UniqueID uid) const
{
  const QByteArray value = readNodeData(uid);
  if (!value.isEmpty())
    return FileNode::fromByteArray(value);

  return FileNode();
}

QByteArray MediaDatabase::readNodeData(UniqueID uid) const
{
  Database::Query query;
  query.exec("SELECT mediaInfo "
             "FROM MediaplayerFiles WHERE uid = " + QString::number(uid));
  if (query.next())
    return query.value(0).toByteArray();

  return QByteArray();
}

void MediaDatabase::setLastPlayed(UniqueID uid, const QDateTime &lastPlayed)
{
  const FileNode node = readNode(uid);
  if (!node.isNull())
    setLastPlayed(node.filePath(), lastPlayed);
}

void MediaDatabase::setLastPlayed(const QString &filePath, const QDateTime &lastPlayed)
{
  if (!filePath.isEmpty())
  {
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
  const FileNode node = readNode(uid);
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

bool MediaDatabase::hasAlbum(Category category, const QString &path) const
{
  foreach (const QString &root, rootPaths(category))
  {
    QDir dir(root + '/' + path);
    if (dir.exists())
      return true;
  }

  return false;
}

int MediaDatabase::countAlbums(Category category, const QString &path) const
{
  int result = 0;

  foreach (const QString &root, rootPaths(category))
  {
    QDir dir(root + '/' + path);
    if (dir.exists())
      result += dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count();
  }

  return result;
}

QStringList MediaDatabase::getAlbums(Category category, const QString &path, unsigned start, unsigned count) const
{
  const bool returnAll = count == 0;
  QStringList result;

  foreach (const QString &root, rootPaths(category))
  {
    QDir dir(root + '/' + path);
    if (dir.exists())
    {
      QVariant parentUid = fromPath(dir.absolutePath());
      if ((parentUid.toULongLong() == 0) && (path == "/"))
      {
        // Insert root path.
        const QFileInfo fileInfo(dir.cleanPath(dir.absolutePath()));

        Database::Query query;
        query.prepare("INSERT INTO MediaplayerFiles "
                      "VALUES (:uid, :parentDir, :path, :size, :lastModified, :mediaInfo)");
        query.bindValue(0, QVariant(QVariant::LongLong));
        query.bindValue(1, QVariant(QVariant::LongLong));
        query.bindValue(2, fileInfo.absoluteFilePath());
        query.bindValue(3, qMax(Q_INT64_C(0), fileInfo.size()));
        query.bindValue(4, fileInfo.lastModified());
        query.bindValue(5, QVariant(QVariant::ByteArray));
        query.exec();

        parentUid = query.lastInsertId();
      }

      if (!parentUid.isNull() && (parentUid.toLongLong() != 0))
      {
        // Remove old files.
        if (start == 0)
        {
          QVariantList obsoleteUids;

          Database::Query query;
          query.prepare("SELECT uid, path FROM MediaplayerFiles WHERE parentDir = :parentDir");
          query.bindValue(0, parentUid);
          query.exec();
          while (query.next())
          if (!QFileInfo(query.value(1).toString()).exists())
            obsoleteUids += query.value(0);

          query.prepare("DELETE FROM MediaplayerFiles WHERE uid = :uid");
          query.bindValue(0, obsoleteUids);
          query.execBatch();
        }

        // Add new albums.
        const QStringList albums = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (int i=start, n=0; (i<albums.count()) && (returnAll || (n<int(count))); i++, n++)
        {
          const QFileInfo fileInfo(dir.cleanPath(dir.absoluteFilePath(albums[i])));
          UniqueID uid = 0;

          result += albums[i];

          Database::Query query;
          query.prepare("SELECT uid, size, lastModified FROM MediaplayerFiles WHERE path = :path");
          query.bindValue(0, fileInfo.absoluteFilePath());
          query.exec();
          if (query.next())
          {
            if ((query.value(1).toLongLong() == fileInfo.size()) &&
                (qAbs(query.value(2).toDateTime().secsTo(fileInfo.lastModified())) <= 1))
            {
              continue;
            }
            else
              uid = query.value(0).toLongLong();
          }

          if (uid == 0)
          {
            query.prepare("INSERT INTO MediaplayerFiles "
                          "VALUES (:uid, :parentDir, :path, :size, :lastModified, :mediaInfo)");
            query.bindValue(0, QVariant(QVariant::LongLong));
            query.bindValue(1, parentUid);
            query.bindValue(2, fileInfo.absoluteFilePath());
            query.bindValue(3, qMax(Q_INT64_C(0), fileInfo.size()));
            query.bindValue(4, fileInfo.lastModified());
            query.bindValue(5, QVariant(QVariant::ByteArray));
            query.exec();

            continue;
          }
          else
          {
            query.prepare("UPDATE MediaplayerFiles "
                          "SET size = :size, lastModified = :lastModified "
                          "WHERE uid = :uid");
            query.bindValue(0, qMax(Q_INT64_C(0), fileInfo.size()));
            query.bindValue(1, fileInfo.lastModified());
            query.bindValue(2, uid);
            query.exec();

            continue;
          }
        }
      }
    }
  }

  return result;
}

int MediaDatabase::countAlbumFiles(Category category, const QString &path) const
{
  int result = 0;

  foreach (const QString &root, rootPaths(category))
  {
    QDir dir(root + '/' + path);
    if (dir.exists())
      result += dir.entryList(QDir::Files).count();
  }

  return result;
}

QVector<MediaDatabase::UniqueID> MediaDatabase::getAlbumFiles(Category category, const QString &path, unsigned start, unsigned count) const
{
  const bool returnAll = count == 0;
  QVector<MediaDatabase::UniqueID> result;

  foreach (const QString &root, rootPaths(category))
  {
    QDir dir(root + '/' + path);
    if (dir.exists())
    {
      const QVariant parentUid = fromPath(dir.absolutePath());
      if (!parentUid.isNull() && (parentUid.toLongLong() != 0))
      {
        QByteArray probeFiles;
        int numProbeFiles = 0;

        const QStringList files = dir.entryList(QDir::Files, QDir::Name);
        for (int i=start, n=0; (i<files.count()) && (returnAll || (n<int(count))); i++, n++)
        {
          const QFileInfo fileInfo(dir.cleanPath(dir.absoluteFilePath(files[i])));

          Database::Query query;
          query.prepare("SELECT uid, size, lastModified FROM MediaplayerFiles WHERE path = :path");
          query.bindValue(0, fileInfo.absoluteFilePath());
          query.exec();
          if (query.next())
          {
            if ((query.value(1).toLongLong() == fileInfo.size()) &&
                (qAbs(query.value(2).toDateTime().secsTo(fileInfo.lastModified())) <= 1))
            {
              result += query.value(0).toLongLong();
              continue;
            }
          }

          probeFiles += fileInfo.absoluteFilePath().toUtf8() + '\n';
          numProbeFiles++;
        }

        if (!probeFiles.isEmpty())
        {
          SSandboxClient::RequestMessage request(probeSandbox);
          request.setRequest("POST", QByteArray(MediaPlayerSandbox::path) + "?probe=");
          request.setContent(probeFiles);

          const SHttpEngine::ResponseMessage response = probeSandbox->blockingRequest(request, 15000);
          if (response.status() == SHttpEngine::Status_Ok)
          foreach (const QByteArray &entry, response.content().split('\n'))
          if (!entry.isEmpty())
          {
            const FileNode mediaInfo = FileNode::fromByteArray(entry);

            Database::Query query;
            query.prepare("SELECT uid FROM MediaplayerFiles WHERE path = :path");
            query.bindValue(0, mediaInfo.filePath());
            query.exec();
            if (query.next())
            {
              const UniqueID uid = query.value(0).toLongLong();

              query.prepare("UPDATE MediaplayerFiles "
                            "SET size = :size, lastModified = :lastModified, mediaInfo = :mediaInfo "
                            "WHERE uid = :uid");
              query.bindValue(0, qMax(Q_INT64_C(0), mediaInfo.size()));
              query.bindValue(1, mediaInfo.lastModified());
              query.bindValue(2, entry);
              query.bindValue(3, uid);
              query.exec();

              result += uid;
              continue;
            }
            else
            {
              query.prepare("INSERT INTO MediaplayerFiles "
                            "VALUES (:uid, :parentDir, :path, :size, :lastModified, :mediaInfo)");
              query.bindValue(0, QVariant(QVariant::LongLong));
              query.bindValue(1, parentUid);
              query.bindValue(2, mediaInfo.filePath());
              query.bindValue(3, qMax(Q_INT64_C(0), mediaInfo.size()));
              query.bindValue(4, mediaInfo.lastModified());
              query.bindValue(5, entry);
              query.exec();

              result += query.lastInsertId().toLongLong();
              continue;
            }
          }
        }
      }
    }
  }

  return result;
}

QStringList MediaDatabase::rootPaths(MediaDatabase::Category category) const
{
  return rootPaths(categoryNames[category]);
}

QStringList MediaDatabase::rootPaths(const QString &category) const
{
  PluginSettings settings(Module::pluginName);

  QStringList result;

  if (settings.childGroups().contains(category))
  {
    settings.beginGroup(category);

    foreach (const QString &root, settings.value("Paths").toStringList())
    if (!root.trimmed().isEmpty() && !ConfigServer::isHidden(root) && QDir(root).exists())
      result.append(root);

    settings.endGroup();
  }

  return result;
}

void MediaDatabase::setRootPaths(Category category, const QStringList &paths)
{
  setRootPaths(categoryNames[category], paths);
}

void MediaDatabase::setRootPaths(const QString &category, const QStringList &paths)
{
  PluginSettings settings(Module::pluginName);

  settings.beginGroup(category);
  settings.setValue("Paths", paths);
  settings.endGroup();
}

} } // End of namespaces
