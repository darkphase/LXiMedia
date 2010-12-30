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
        result.headline = node.title() + " [" + albumName + "] (" + tr("Home video") + ")";
        result.location = MediaDatabase::toUidString(node.uid) + ".html";
        result.text = node.mediaInfo.fileTypeName() + ", " +
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

void HomeVideoServer::updateDlnaTask(void)
{
  QMultiMap<QString, DlnaServerDir *> subDirs;
  foreach (const QString &homeVideo, mediaDatabase->allHomeVideos())
  {
    QString homeVideoName;

    QMultiMap<QString, DlnaServer::File> dlnaFiles;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allHomeVideoFiles(homeVideo))
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        if (homeVideoName.isEmpty())
        {
          QDir parentDir(node.path);
          parentDir.cdUp();
          homeVideoName = parentDir.dirName();
        }

        DlnaServer::File clipFile(dlnaDir.server());
        clipFile.date = node.lastModified;
        clipFile.url = httpPath() + MediaDatabase::toUidString(node.uid) + ".mpeg";
        clipFile.iconUrl = httpPath() + MediaDatabase::toUidString(node.uid) + "-thumb.jpeg";
        clipFile.mimeType = "video/mpeg";

        dlnaFiles.insert(node.title(), clipFile);
      }
    }

    if (!dlnaFiles.isEmpty())
    {
      if (homeVideoName.isEmpty())
        homeVideoName = homeVideo;

      DlnaServerDir * subDir = new DlnaServerDir(dlnaDir.server());
      for (QMultiMap<QString, DlnaServer::File>::Iterator i=dlnaFiles.begin(); i!=dlnaFiles.end(); i++)
        subDir->addFile(i.key(), i.value());

      subDirs.insert(homeVideoName, subDir);
    }
  }

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  dlnaDir.clear();
  for (QMultiMap<QString, DlnaServerDir *>::Iterator i=subDirs.begin(); i!=subDirs.end(); i++)
    dlnaDir.addDir(i.key(), i.value());
}

} // End of namespace
