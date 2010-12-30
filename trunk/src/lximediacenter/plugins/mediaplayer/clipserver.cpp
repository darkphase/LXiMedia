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

#include "clipserver.h"

namespace LXiMediaCenter {

ClipServer::ClipServer(MediaDatabase *mediaDatabase, Plugin *plugin, MasterServer *server)
           :MediaServer(QT_TR_NOOP("Video clips"), mediaDatabase, plugin, server)
{
  enableDlna();
  connect(mediaDatabase, SIGNAL(updatedClips()), SLOT(startDlnaUpdate()));
}

ClipServer::~ClipServer()
{
}

BackendServer::SearchResultList ClipServer::search(const QStringList &query) const
{
  SearchResultList results;

  foreach (const MediaDatabase::UniqueID &uid, mediaDatabase->queryVideoClips(query))
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      QDir parentDir(node.path);
      parentDir.cdUp();

      const QString albumName = parentDir.dirName();
      const QString rawAlbumName = SStringParser::toRawName(albumName);
      const qreal match =
          qMin(SStringParser::computeMatch(SStringParser::toRawName(node.title()), query) +
               SStringParser::computeMatch(rawAlbumName, query), 1.0);

      if (match >= minSearchRelevance)
      {
        const QString time = QTime().addSecs(node.mediaInfo.duration().toSec()).toString(videoTimeFormat);

        SearchResult result;
        result.relevance = match;
        result.headline = node.title() + " (" + tr("Video clip") + ")";
        result.location = MediaDatabase::toUidString(node.uid) + ".html";
        result.text = time + ", " + node.mediaInfo.fileTypeName() + ", " +
                      videoFormatString(node.mediaInfo) + ", " +
                      node.lastModified.toString(searchDateTimeFormat);

        if (!node.mediaInfo.thumbnails().isEmpty())
          result.thumbLocation = MediaDatabase::toUidString(node.uid) + "-thumb.jpeg";

        results += result;
      }
    }
  }

  return results;
}

void ClipServer::updateDlnaTask(void)
{
  QMultiMap<QString, DlnaServerDir *> subDirs;
  foreach (const QString &clipAlbum, mediaDatabase->allVideoClipAlbums())
  {
    QString albumName;

    QMultiMap<QString, MediaDatabase::Node> clips;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allVideoClipFiles(clipAlbum))
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        if (albumName.isEmpty())
        {
          QDir parentDir(node.path);
          parentDir.cdUp();
          albumName = parentDir.dirName();
        }

        clips.insert(node.title(), node);
      }
    }

    if (!clips.isEmpty())
    {
      if (albumName.isEmpty())
        albumName = clipAlbum;

      DlnaServerDir * subDir = new DlnaServerDir(dlnaDir.server());
      for (QMultiMap<QString, MediaDatabase::Node>::Iterator i=clips.begin(); i!=clips.end(); i++)
        addVideoFile(subDir, *i, i.key());

      subDirs.insert(albumName, subDir);
    }
  }

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  dlnaDir.clear();
  for (QMultiMap<QString, DlnaServerDir *>::Iterator i=subDirs.begin(); i!=subDirs.end(); i++)
    dlnaDir.addDir(i.key(), i.value());
}

} // End of namespace
