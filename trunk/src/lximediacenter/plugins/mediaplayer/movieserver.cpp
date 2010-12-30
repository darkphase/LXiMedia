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

#include "movieserver.h"

namespace LXiMediaCenter {

MovieServer::MovieServer(MediaDatabase *mediaDatabase, Plugin *plugin, MasterServer *server)
            :MediaServer(QT_TR_NOOP("Movies"), mediaDatabase, plugin, server)
{
  enableDlna();
  connect(mediaDatabase, SIGNAL(updatedMovies()), SLOT(startDlnaUpdate()));
}

MovieServer::~MovieServer()
{
}

BackendServer::SearchResultList MovieServer::search(const QStringList &query) const
{
  SearchResultList results;

  foreach (const MediaDatabase::UniqueID &uid, mediaDatabase->queryMovieFiles(query))
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      const QString title = QFileInfo(node.fileName()).completeBaseName();
      const qreal match = SStringParser::computeMatch(SStringParser::toRawName(title), query);
      if (match >= minSearchRelevance)
      {
        const QString time = QTime().addSecs(node.mediaInfo.duration().toSec()).toString(videoTimeFormat);

        SearchResult result;
        result.relevance = match;
        result.headline = title + " (" + tr("Movie") + ")";
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

void MovieServer::updateDlnaTask(void)
{
#warning lastPlayed
  QMap<QString, QMap<QString, MediaDatabase::Node> > movies;
  foreach (const QString &movie, mediaDatabase->allMovies())
  {
    const QList<MediaDatabase::UniqueID> allFiles = mediaDatabase->allMovieFiles(movie);
    if (!allFiles.isEmpty())
    {
      QString title = QString::null;

      const ImdbClient::Entry imdbEntry = mediaDatabase->getMovieImdbEntry(movie);
      if (!imdbEntry.isNull())
      {
        title = imdbEntry.title;
        if (imdbEntry.rating > 0.0)
          title += " [" + QString::number(imdbEntry.rating, 'f', 1) + "]";
      }

      bool isHD = true;
      QMap<QString, MediaDatabase::Node> nodes;
      foreach (MediaDatabase::UniqueID uid, allFiles)
      {
        const MediaDatabase::Node node = mediaDatabase->readNode(uid);

        bool hasHD = false;
        foreach (const SMediaInfo::VideoStreamInfo &info, node.mediaInfo.videoStreams())
          hasHD |= info.codec.size().width() >= 1280;

        isHD &= hasHD;

        if (title.isEmpty())
          title = QFileInfo(node.fileName()).completeBaseName();

        nodes.insert(SStringParser::toRawName(node.fileName()), node);
      }

      if (isHD)
        title += ", HD";

      QMap<QString, QMap<QString, MediaDatabase::Node> >::Iterator i = movies.find(title);
      if (i == movies.end())
        i = movies.insert(title, nodes);
      else for (QMap<QString, MediaDatabase::Node>::Iterator j=nodes.begin(); j!=nodes.end(); j++)
        i->insert(j.key(), j.value());
    }
  }

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  dlnaDir.clear();
  for (QMap<QString, QMap<QString, MediaDatabase::Node> >::Iterator i=movies.begin(); i!=movies.end(); i++)
    addVideoFile(&dlnaDir, i->values(), i.key());
}

} // End of namespace
