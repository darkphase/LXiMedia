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

TvShowServer::TvShowServer(MediaDatabase *mediaDatabase, Plugin *plugin, MasterServer *server)
             :MediaServer(QT_TR_NOOP("TV Shows"), mediaDatabase, plugin, server)
{
  enableDlna();
  connect(mediaDatabase, SIGNAL(updatedTvShows()), SLOT(startDlnaUpdate()));
}

TvShowServer::~TvShowServer()
{
}

BackendServer::SearchResultList TvShowServer::search(const QStringList &query) const
{
  SearchResultList results;

  foreach (const MediaDatabase::UniqueID &uid, mediaDatabase->queryTvShows(query))
  {
    const MediaDatabase::Node node = mediaDatabase->readNode(uid);
    if (!node.isNull())
    {
      const QString showName = node.mediaInfo.album().length() > 0 ? node.mediaInfo.album() : node.title();
      const qreal match =
          qMin(SStringParser::computeMatch(SStringParser::toRawName(node.title()), query) +
               SStringParser::computeMatch(SStringParser::toRawName(showName), query), 1.0);

      if (match >= minSearchRelevance)
      {
        const QString time = QTime().addSecs(node.mediaInfo.duration().toSec()).toString(videoTimeFormat);

        SearchResult result;
        result.relevance = match;
        result.headline = node.title() + " [" + showName + "] (" + tr("TV show episode") + ")";
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

void TvShowServer::updateDlnaTask(void)
{
  QMultiMap<QString, DlnaServerDir *> subDirs;
  foreach (const QString &tvShow, mediaDatabase->allTvShows())
  {
    QString showName, showDirName;
    bool hasSeasons = true;
    QList<MediaDatabase::Node> nodes;
    foreach (MediaDatabase::UniqueID uid, mediaDatabase->allTvShowEpisodes(tvShow))
    {
      const MediaDatabase::Node node = mediaDatabase->readNode(uid);
      if (!node.isNull())
      {
        nodes += node;

        if (node.mediaInfo.track() > 0)
          hasSeasons = hasSeasons && (node.mediaInfo.track() > SMediaInfo::tvShowSeason);

        if (showName.isEmpty())
          showName = node.mediaInfo.album();

        if (showDirName.isEmpty())
        {
          QDir parentDir(node.path);
          parentDir.cdUp();
          showDirName = parentDir.dirName();
        }
      }
    }

    QMap<QString, QMultiMap<QString, MediaDatabase::Node> > dlnaFiles;
    foreach (const MediaDatabase::Node &node, nodes)
    {
      QString seasonName = QString::null;
      if (hasSeasons)
      if ((node.mediaInfo.track() > 0) && (node.mediaInfo.track() >= SMediaInfo::tvShowSeason))
        seasonName = tr("Season") + " " + QString::number(node.mediaInfo.track() / SMediaInfo::tvShowSeason);

      QMap<QString, QMultiMap<QString, MediaDatabase::Node> >::Iterator files;
      files = dlnaFiles.find(seasonName);
      if (files == dlnaFiles.end())
        files = dlnaFiles.insert(seasonName, QMultiMap<QString, MediaDatabase::Node>());

      if (hasSeasons)
      {
        if (node.mediaInfo.track() > 0)
          files->insert(toTvShowNumber(node.mediaInfo.track()) + " " + node.title(), node);
        else
          files->insert(node.title(), node);
      }
      else if (node.mediaInfo.track() > 0)
      {
        QString episode = QByteArray::number(node.mediaInfo.track());
        while (episode.length() < 3)
          episode = "0" + episode;

        files->insert(episode + " " + node.title(), node);
      }
      else
        files->insert(node.title(), node);
    }

    if (!dlnaFiles.isEmpty())
    {
      if (showName.isEmpty())
        showName = !showDirName.isEmpty() ? showDirName : tvShow;

      DlnaServerDir * subDir = new DlnaServerDir(dlnaDir.server());
      for (QMap<QString, QMultiMap<QString, MediaDatabase::Node> >::Iterator i=dlnaFiles.begin(); i!=dlnaFiles.end(); i++)
      {
        DlnaServerDir * seasonDir = subDir;
        if (!i.key().isEmpty())
        {
          seasonDir = new DlnaServerDir(dlnaDir.server());
          seasonDir->sortOrder = i.key().split(' ').last().toInt() + SMediaInfo::tvShowSeason;

          subDir->addDir(i.key(), seasonDir);
        }

        for (QMultiMap<QString, MediaDatabase::Node>::Iterator j=i->begin(); j!=i->end(); j++)
        if (j->mediaInfo.track() > 0)
          addVideoFile(seasonDir, *j, j.key(), j->mediaInfo.track());
        else
          addVideoFile(seasonDir, *j, j.key(), SMediaInfo::tvShowSeason * 100);
      }

      subDirs.insert(showName, subDir);
    }
  }

  SDebug::MutexLocker l(&dlnaDir.server()->mutex, __FILE__, __LINE__);

  dlnaDir.clear();
  for (QMultiMap<QString, DlnaServerDir *>::Iterator i=subDirs.begin(); i!=subDirs.end(); i++)
    dlnaDir.addDir(i.key(), i.value());
}

QString TvShowServer::toTvShowNumber(unsigned episode)
{
  QString text = QString::number(episode % SMediaInfo::tvShowSeason);
  if (text.length() < 2)
    text = "0" + text;

  return QString::number(episode / SMediaInfo::tvShowSeason) + "x" + text;
}

} // End of namespace
