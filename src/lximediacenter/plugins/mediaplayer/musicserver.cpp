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

#include "musicserver.h"

namespace LXiMediaCenter {

MusicServer::MusicServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, MasterServer *server)
  : PlaylistServer(mediaDatabase, category, name, plugin, server),
    playlistDir(GlobalSettings::applicationDataDir() + "/playlists")
{
  if (!playlistDir.exists())
    playlistDir.mkpath(playlistDir.absolutePath());
}

MusicServer::~MusicServer()
{
}

QList<MusicServer::Item> MusicServer::listItems(const QString &path, unsigned start, unsigned count)
{
  QList<Item> items;
  foreach (Item item, PlaylistServer::listItems(path, start, count))
  {
    if (!item.isDir)
    {
      item.played = false; // Not useful for music.
      item.direct = true;

      if (!item.artist.isEmpty())
        item.title += " [" + item.artist + "]";
    }

    items += item;
  }

  return items;
}

HttpServer::SocketOp MusicServer::handleHttpRequest(const HttpServer::RequestHeader &request, QIODevice *socket)
{
  const QUrl url(request.path());
  const QString file = request.file();

  if (file.isEmpty())
  {
    HttpServer::ResponseHeader response(request, HttpServer::Status_Ok);
    response.setContentType("text/html;charset=utf-8");
    response.setField("Cache-Control", "no-cache");

    QString path = url.path().mid(httpPath().length());
    path = path.startsWith('/') ? path : ('/' + path);

    DetailedListItemList detailedItems;

    foreach (const UPnPContentDirectory::Item &item, PlaylistServer::listItems(path))
    {
      if (item.isDir)
      {
        DetailedListItem detailedItem;
        detailedItem.columns += item.title;
        detailedItem.columns += "";
        detailedItem.columns += "";
        detailedItem.url = item.title + '/';
        detailedItem.iconurl = "/img/directory.png";

        detailedItems.append(detailedItem);
      }
      else if (item.duration > 0)
      {
        DetailedListItem detailedItem;
        detailedItem.columns += item.title;
        detailedItem.columns += item.artist;
        detailedItem.columns += QTime().addSecs(item.duration).toString("mm:ss");
        detailedItem.url = item.url;
        detailedItem.url.setPath(detailedItem.url.path() + ".html");
        detailedItem.iconurl = item.videoStreams.isEmpty() ? "/img/audio-file.png" : "/img/video-file.png";
        detailedItem.played = item.played;

        detailedItems.append(detailedItem);
      }
      else
      {
        DetailedListItem detailedItem;
        detailedItem.columns += item.title;
        detailedItem.columns += "";
        detailedItem.columns += "";
        detailedItem.url = item.url;
        detailedItem.url.setPath(detailedItem.url.path() + ".html");
        detailedItem.iconurl = "/img/playlist-file.png";

        detailedItems.append(detailedItem);
      }
    }

    const QStringList columns = QStringList() << tr("Title") << tr("Artist") << tr("Duration");

    return sendHtmlContent(socket, url, response, buildDetailedView(path, columns, detailedItems), headList);
  }

  return PlaylistServer::handleHttpRequest(request, socket);
}

QStringList MusicServer::playlists(void) const
{
  QStringList result;

  foreach (const QString &name, playlistDir.entryList(QStringList() << "*.m3u"))
    result += name.left(name.length() - 4);

  return result;
}

Playlist * MusicServer::createPlaylist(const QString &name)
{
  QFile file(playlistDir.absoluteFilePath(name + ".m3u"));
  if (file.open(QFile::ReadOnly))
  {
    Playlist * const playlist = new Playlist(mediaDatabase);
    if (playlist->deserialize(file.readAll()))
      return playlist;

    delete playlist;
  }

  return NULL;
}

void MusicServer::storePlaylist(Playlist *playlist, const QString &name)
{
  QFile file(playlistDir.absoluteFilePath(name + ".m3u"));
  if (file.open(QFile::WriteOnly))
    file.write(playlist->serialize());
}

} // End of namespace
