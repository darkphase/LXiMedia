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

TvShowServer::TvShowServer(MediaDatabase *mediaDatabase, MediaDatabase::Category category, const char *name, Plugin *plugin, MasterServer *server)
  : PlaylistServer(mediaDatabase, category, name, plugin, server),
    seasonText(tr("Season"))
{
}

TvShowServer::~TvShowServer()
{
}

HttpServer::SocketOp TvShowServer::streamVideo(const HttpServer::RequestHeader &request, QAbstractSocket *socket)
{
  const QStringList file = request.file().split('.');

  if ((file.count() > 1) && (file[1] == "playlist"))
  {
    const QString path = QString::fromUtf8(QByteArray::fromHex(file.first().toAscii()));
    if (!mediaDatabase->hasAlbum(category, path))
    {
      const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
      if (dir.startsWith("/" + seasonText + " "))
      {
        const unsigned season = dir.mid(seasonText.length() + 1, dir.length() - seasonText.length() - 2).toUInt();

        QMap<unsigned, QVector<MediaDatabase::UniqueID> > seasons;
        bool useSeasons;
        categorizeSeasons(path.left(path.length() - dir.length() + 1), seasons, useSeasons);

        QMultiMap<QString, SMediaInfo> files;

        const QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator s = seasons.find(season);
        if (s != seasons.end())
        for (QVector<MediaDatabase::UniqueID>::ConstIterator i=s->begin(); i!=s->end(); i++)
        {
          const SMediaInfo node = mediaDatabase->readNode(*i);
          if (!node.isNull())
          {
            const QDateTime lastPlayed = mediaDatabase->lastPlayed(node.filePath());
            const QString key =
                (lastPlayed.isValid() ? lastPlayed.toString("yyyyMMddhhmmss") : QString("00000000000000")) +
                ("000000000" + QString::number(node.track())).right(10) +
                SStringParser::toRawName(node.title());

            files.insert(key, node);
          }
        }

        PlaylistStream *stream = new PlaylistStream(this, socket->peerAddress(), request.path(), files.values());
        if (stream->setup(request, socket))
        if (stream->start())
          return HttpServer::SocketOp_LeaveOpen; // The graph owns the socket now.

        delete stream;
      }

      qWarning() << "Failed to start stream" << request.path();
      socket->write(HttpServer::ResponseHeader(HttpServer::Status_NotFound));
      return HttpServer::SocketOp_Close;
    }
  }

  return PlaylistServer::streamVideo(request, socket);
}

int TvShowServer::countItems(const QString &path)
{
  const bool hasAlbum = mediaDatabase->hasAlbum(category, path);
  const int numAlbums = countAlbums(path);

  if ((numAlbums == 0) && !hasAlbum)
  {
    const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
    if (dir.startsWith("/" + seasonText + " "))
    {
      const unsigned season = dir.mid(seasonText.length() + 1, dir.length() - seasonText.length() - 2).toUInt();

      QMap<unsigned, QVector<MediaDatabase::UniqueID> > seasons;
      bool useSeasons;
      categorizeSeasons(path.left(path.length() - dir.length() + 1), seasons, useSeasons);

      const QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator s = seasons.find(season);
      if (s != seasons.end())
        return s->count() + 1;
      else
        return 0;
    }
  }

  if (hasAlbum)
  {
    QMap<unsigned, QVector<MediaDatabase::UniqueID> > seasons;
    bool useSeasons;
    categorizeSeasons(path, seasons, useSeasons);

    const QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator r = seasons.find(0);
    if (r != seasons.end())
      return numAlbums + (seasons.count() - 1) + r->count() + 1;
    else
      return numAlbums + seasons.count() + 1;
  }
  else
    return numAlbums;
}

QList<TvShowServer::Item> TvShowServer::listItems(const QString &path, unsigned start, unsigned count)
{
  const bool returnAll = count == 0;
  const bool hasAlbum = mediaDatabase->hasAlbum(category, path);
  QList<Item> result = listAlbums(path, start, count);

  if (result.isEmpty() && !hasAlbum)
  {
    const QString dir = path.mid(path.left(path.length() - 1).lastIndexOf('/'));
    if (dir.startsWith("/" + seasonText + " "))
    {
      const unsigned season = dir.mid(seasonText.length() + 1, dir.length() - seasonText.length() - 2).toUInt();

      QMap<unsigned, QVector<MediaDatabase::UniqueID> > seasons;
      bool useSeasons;
      categorizeSeasons(path.left(path.length() - dir.length() + 1), seasons, useSeasons);

      const QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator s = seasons.find(season);
      if ((s != seasons.end()) && !s->isEmpty())
      {
        QList<Item> result = listPlayAllItem(path, start, count, s->first());

        if (returnAll || (count > 0))
        for (int i=start, n=0; (i<s->count()) && (returnAll || (n<int(count))); i++, n++)
          result.append(makeSeasonItem(s->at(i)));

        return result;
      }
    }

    return QList<Item>();
  }

  if (hasAlbum && (returnAll || (count > 0)))
  {
    QMap<unsigned, QVector<MediaDatabase::UniqueID> > seasons;
    bool useSeasons;
    categorizeSeasons(path, seasons, useSeasons);

    for (QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator i=seasons.begin(); i!=seasons.end(); i++)
    if ((i.key() != 0) && !i.value().isEmpty())
    {
      if (start == 0)
      {
        if (returnAll || (count > 0))
        {
          Item item;
          item.isDir = true;
          item.title = seasonText + " " + QString::number(i.key());
          item.iconUrl = MediaDatabase::toUidString(i.value().first()) + "-thumb.jpeg?overlay=folder-video";
          result.append(item);

          if (count > 0)
            count--;
        }
      }
      else
        start--;
    }

    if (returnAll || (count > 0))
    {
      result += listPlayAllItem(path, start, count);

      if (returnAll || (count > 0))
      {
        const QMap<unsigned, QVector<MediaDatabase::UniqueID> >::ConstIterator r = seasons.find(0);
        if (r != seasons.end())
        for (int i=start, n=0; (i<r->count()) && (returnAll || (n<int(count))); i++, n++)
          result.append(useSeasons ? makeSeasonItem(r->at(i)) : makePlainItem(r->at(i)));
      }
    }
  }

  return result;
}

void TvShowServer::categorizeSeasons(const QString &path, QMap<unsigned, QVector<MediaDatabase::UniqueID> > &seasons, bool &useSeasons)
{
  const QList<MediaDatabase::File> files = mediaDatabase->getAlbumFiles(category, path);

  seasons.clear();
  useSeasons = true;
  foreach (const MediaDatabase::File &file, files)
  {
    const unsigned track = file.name.left(10).toUInt();
    if ((track > 0) && (track < SMediaInfo::tvShowSeason))
    {
      // Don't use seasons
      seasons.clear();
      useSeasons = false;
      foreach (const MediaDatabase::File &file, files)
        seasons[0].append(file.uid);

      return;
    }

    seasons[track / SMediaInfo::tvShowSeason].append(file.uid);
  }

  if (seasons.count() == 1)
  {
    // Only one season
    const QVector<MediaDatabase::UniqueID> s = *(seasons.begin());
    seasons.erase(seasons.begin());
    seasons[0] = s;
  }
}

TvShowServer::Item TvShowServer::makePlainItem(MediaDatabase::UniqueID uid)
{
  Item item = makeItem(uid);
  if (item.mediaInfo.track() > 0)
    item.title = QString::number(item.mediaInfo.track()) + " " + item.title;

  return item;
}

TvShowServer::Item TvShowServer::makeSeasonItem(MediaDatabase::UniqueID uid)
{
  Item item = makeItem(uid);
  if (item.mediaInfo.track() > 0)
    item.title = toTvShowNumber(item.mediaInfo.track()) + " " + item.title;

  return item;
}

QString TvShowServer::toTvShowNumber(unsigned episode)
{
  QString text = QString::number(episode % SMediaInfo::tvShowSeason);
  if (text.length() < 2)
    text = "0" + text;

  return QString::number(episode / SMediaInfo::tvShowSeason) + "x" + text;
}

} // End of namespace
