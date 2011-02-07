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

#include "tvshowserver.h"

namespace LXiMediaCenter {

TvShowServer::TvShowServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, BackendServer::MasterServer *server)
  : MediaPlayerServer(mediaDatabase, category, name, plugin, server)
{
  setRoot(new Dir(this, "/"));
}

TvShowServer::~TvShowServer()
{
}

QString TvShowServer::toTvShowNumber(unsigned episode)
{
  QString text = QString::number(episode % SMediaInfo::tvShowSeason);
  if (text.length() < 2)
    text = "0" + text;

  return QString::number(episode / SMediaInfo::tvShowSeason) + "x" + text;
}


TvShowServer::Dir::Dir(MediaPlayerServer *parent, const QString &albumPath)
  : MediaPlayerServerDir(parent, albumPath)
{
}

QStringList TvShowServer::Dir::listDirs(void)
{
  const QString seasonText = tr("Season");

  QSet<QString> albums;
  foreach (const QString &album, server()->mediaDatabase->allAlbums(server()->category))
  if (album.startsWith(albumPath))
  {
    const QString subAlbum = album.mid(albumPath.length()).split('/').first();
    if (!subAlbum.isEmpty())
      albums.insert(subAlbum);
  }

  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  categorizeSeasons();
  for (QMap<unsigned, QMap<QString, File> >::ConstIterator i=seasons.begin(); i!=seasons.end(); i++)
  if (i.key() > 0)
    albums.insert(seasonText + " " + QString::number(i.key()));

  QStringList addAlbums = albums.toList();
  QStringList delAlbums;

  foreach (const QString &dirName, MediaServerDir::listDirs())
  {
    if (!albums.contains(dirName))
      delAlbums.append(dirName);
    else
      addAlbums.removeAll(dirName);
  }

  foreach (const QString &album, delAlbums)
    removeDir(album);

  foreach (const QString &album, addAlbums)
  {
    if (album.startsWith(seasonText))
    {
      const int season = album.mid(seasonText.length() + 1).toInt();
      SeasonDir * const dir = new SeasonDir(server(), seasons[season]);
      dir->sortOrder += season;

      addDir(album, dir);
    }
    else
      addDir(album, createDir(server(), albumPath + album));
  }

  return MediaServerDir::listDirs();
}

QStringList TvShowServer::Dir::listFiles(void)
{
  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  if (seasons.isEmpty())
  {
    QMap<QString, File> files;
    foreach (MediaDatabase::UniqueID uid, server()->mediaDatabase->allAlbumFiles(server()->category, albumPath))
    {
      const SMediaInfo node = server()->mediaDatabase->readNode(uid);
      const SMediaInfo titleNode = (!node.isNull() && node.isDisc()) ? node.titles().first() : node;

      if (!node.isNull() && !titleNode.isNull())
      {
        File file;
        if (titleNode.containsAudio() || titleNode.containsVideo())
          file.mimeType = "video/mpeg";
        else
          file.mimeType = "image/jpeg";

        if (!file.mimeType.isEmpty())
        {
          file.played = server()->mediaDatabase->lastPlayed(uid).isValid();
          file.url = server()->httpPath() + MediaDatabase::toUidString(uid) + "." + file.mimeType.right(4);
          file.iconUrl = server()->httpPath() + MediaDatabase::toUidString(uid) + "-thumb.jpeg";
          file.mediaInfo = titleNode;

          QString title = node.title();
          if (node.track() > 0)
            title = toTvShowNumber(node.track()) + " " + title;

          files.insert(title, file);
        }
      }
    }

    QStringList addFiles = files.keys();
    QStringList delFiles;

    foreach (const QString &fileName, MediaServerDir::listFiles())
    {
      if (!files.contains(fileName))
        delFiles.append(fileName);
      else
        addFiles.removeAll(fileName);
    }

    foreach (const QString &fileName, delFiles)
      removeFile(fileName);

    foreach (const QString &fileName, addFiles)
      addFile(fileName, files[fileName]);

    return MediaServerDir::listFiles();
  }
  else
  {
    QMap<unsigned, QMap<QString, File> >::Iterator nullSeason = seasons.find(0);
    if (nullSeason != seasons.end())
    {
      QStringList addFiles = nullSeason->keys();
      QStringList delFiles;

      foreach (const QString &fileName, MediaServerDir::listFiles())
      {
        if (!nullSeason->contains(fileName))
          delFiles.append(fileName);
        else
          addFiles.removeAll(fileName);
      }

      foreach (const QString &fileName, delFiles)
        removeFile(fileName);

      foreach (const QString &fileName, addFiles)
        addFile(fileName, (*nullSeason)[fileName]);

      return MediaServerDir::listFiles();
    }
    else
      return QStringList();
  }
}

void TvShowServer::Dir::categorizeSeasons(void)
{
  seasons.clear();

  foreach (MediaDatabase::UniqueID uid, server()->mediaDatabase->allAlbumFiles(server()->category, albumPath))
  {
    const SMediaInfo node = server()->mediaDatabase->readNode(uid);
    const SMediaInfo titleNode = (!node.isNull() && node.isDisc()) ? node.titles().first() : node;

    if (!node.isNull() && !titleNode.isNull())
    {
      File file;
      file.played = server()->mediaDatabase->lastPlayed(uid).isValid();
      file.mimeType = "video/mpeg";
      file.url = server()->httpPath() + MediaDatabase::toUidString(uid) + "." + file.mimeType.right(4);
      file.iconUrl = server()->httpPath() + MediaDatabase::toUidString(uid) + "-thumb.jpeg";
      file.mediaInfo = titleNode;

      QString title = node.title();
      if (node.track() > 0)
        title = toTvShowNumber(node.track()) + " " + title;

      seasons[node.track() / SMediaInfo::tvShowSeason].insert(title, file);
    }
  }
}

MediaPlayerServerDir * TvShowServer::Dir::createDir(MediaPlayerServer *parent, const QString &albumPath)
{
  return new Dir(parent, albumPath);
}


TvShowServer::SeasonDir::SeasonDir(TvShowServer *parent, const QMap<QString, File> &episodes)
  : MediaServerDir(parent),
    episodes(episodes)
{
}

QStringList TvShowServer::SeasonDir::listFiles(void)
{
  SDebug::WriteLocker l(&server()->lock, __FILE__, __LINE__);

  QStringList addFiles = episodes.keys();
  QStringList delFiles;

  foreach (const QString &fileName, MediaServerDir::listFiles())
  {
    if (!episodes.contains(fileName))
      delFiles.append(fileName);
    else
      addFiles.removeAll(fileName);
  }

  foreach (const QString &fileName, delFiles)
    removeFile(fileName);

  foreach (const QString &fileName, addFiles)
    addFile(fileName, episodes[fileName]);

  return MediaServerDir::listFiles();
}

} // End of namespace
