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
namespace MediaPlayerBackend {

MusicServer::MusicServer(MediaDatabase::Category category, QObject *parent)
  : PlaylistServer(category, parent),
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

SHttpServer::SocketOp MusicServer::handleHttpRequest(const SHttpServer::RequestMessage &request, QIODevice *socket)
{
  if ((request.method() == "GET") || (request.method() == "HEAD"))
  {
    const MediaServer::File file(request);
    if (file.baseName().isEmpty())
    {
      SHttpServer::ResponseHeader response(request, SHttpServer::Status_Ok);
      response.setContentType("text/html;charset=utf-8");
      response.setField("Cache-Control", "no-cache");

      DetailedListItemList detailedItems;
      foreach (const SUPnPContentDirectory::Item &item, PlaylistServer::listItems(basePath(file.url().path())))
      {
        if (item.isDir)
        {
          DetailedListItem detailedItem;
          detailedItem.columns.append(
              DetailedListItem::Column(
                  item.title,
                  QUrl("/img/directory.png"),
                  QUrl(item.title + '/')));
          detailedItem.columns.append(DetailedListItem::Column());
          detailedItem.columns.append(DetailedListItem::Column());

          detailedItems.append(detailedItem);
        }
        else if (item.duration > 0)
        {
          QUrl url = item.url;
          url.setPath(url.path() + ".html");

          DetailedListItem detailedItem;
          detailedItem.columns.append(
              DetailedListItem::Column(
                  item.title,
                  QUrl(item.videoStreams.isEmpty() ? "/img/audio-file.png" : "/img/video-file.png"),
                  url));
          detailedItem.columns.append(DetailedListItem::Column(item.artist));
          detailedItem.columns.append(DetailedListItem::Column(QTime().addSecs(item.duration).toString("mm:ss")));
          detailedItem.played = item.played;

          detailedItems.append(detailedItem);
        }
        else
        {
          QUrl url = item.url;
          url.setPath(url.path() + ".html");

          DetailedListItem detailedItem;
          detailedItem.columns.append(
              DetailedListItem::Column(
                  item.title,
                  QUrl("/img/playlist-file.png"),
                  url));
          detailedItem.columns.append(DetailedListItem::Column());
          detailedItem.columns.append(DetailedListItem::Column());

          detailedItems.append(detailedItem);
        }
      }

      QList< QPair<QString, bool> > columns;
      columns.append(qMakePair(tr("Title"), true));
      columns.append(qMakePair(tr("Artist"), true));
      columns.append(qMakePair(tr("Duration"), false));

      return sendHtmlContent(request, socket, file.url(), response, buildDetailedView(dirName(file.url().path()), columns, detailedItems));
    }
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

} } // End of namespaces
