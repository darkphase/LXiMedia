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

#include "homevideoserver.h"

namespace LXiMediaCenter {

HomeVideoServer::HomeVideoServer(MediaDatabase *mediaDatabase, Plugin *plugin, MasterServer *server)
    : MediaServer(QT_TR_NOOP("Home videos"), mediaDatabase, plugin, server)
{
  enableDlna();
  connect(mediaDatabase, SIGNAL(updatedHomeVideos()), SLOT(startDlnaUpdate()));
}

HomeVideoServer::~HomeVideoServer()
{
}

BackendServer::SearchResultList HomeVideoServer::search(const QStringList &query) const
{
  SearchResultList results;

  foreach (const MediaDatabase::UniqueID &uid, mediaDatabase->queryHomeVideos(query))
  {
    const SMediaInfo node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QDir parentDir(node.filePath());
      parentDir.cdUp();

      const QString albumName = parentDir.dirName();
      const QString rawAlbumName = SStringParser::toRawName(albumName);
      const qreal match =
          qMin(SStringParser::computeMatch(SStringParser::toRawName(node.title()), query) +
               SStringParser::computeMatch(rawAlbumName, query), 1.0);

      if (match >= minSearchRelevance)
      {
        const QString time = QTime().addSecs(node.duration().toSec()).toString(videoTimeFormat);

        SearchResult result;
        result.relevance = match;
        result.headline = node.title() + " [" + albumName + "] (" + tr("Home video") + ")";
        result.location = MediaDatabase::toUidString(uid) + ".html";
        result.text = node.fileTypeName() + ", " +
                      videoFormatString(node) + ", " +
                      node.lastModified().toString(searchDateTimeFormat);

        if (!node.thumbnails().isEmpty())
          result.thumbLocation = MediaDatabase::toUidString(uid) + "-thumb.jpeg";

        results += result;
      }
    }
  }

  return results;
}

void HomeVideoServer::updateDlnaTask(void)
{
  QMultiMap<QString, QMultiMap<QString, PlayItem> > albums;
  foreach (const QString &album, mediaDatabase->allHomeVideos())
  {
    QMultiMap<QString, PlayItem> clips;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allHomeVideoFiles(album))
    {
      const SMediaInfo node = mediaDatabase->readNode(uid);
      if (!node.isNull())
        clips.insert(node.title(), PlayItem(uid, node));
    }

    if (!clips.isEmpty())
      albums.insert(album, clips);
  }

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  dlnaDir.clear();
  for (QMultiMap<QString, QMultiMap<QString, PlayItem> >::Iterator i=albums.begin(); i!=albums.end(); i++)
  {
    DlnaServerDir * const dir = getAlbumDir(i.key());
    for (QMultiMap<QString, PlayItem>::Iterator j=i->begin(); j!=i->end(); j++)
      addVideoFile(dir, *j, j.key());
  }
}

} // End of namespace
