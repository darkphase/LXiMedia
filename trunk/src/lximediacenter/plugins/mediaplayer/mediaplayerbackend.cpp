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

#include "mediaplayerbackend.h"
#include "configserver.h"
#include "mediaplayerserver.h"
#include "musicserver.h"
#include "photoserver.h"
#include "playlistserver.h"
#include "tvshowserver.h"

namespace LXiMediaCenter {

MediaPlayerBackend::MediaPlayerBackend(QObject *parent)
  : BackendPlugin(parent),
    database(NULL)
{
}

MediaPlayerBackend::~MediaPlayerBackend()
{
  delete database;
}

QString MediaPlayerBackend::pluginName(void) const
{
  return "Media Player";
}

QString MediaPlayerBackend::pluginVersion(void) const
{
  return "1.0";
}

QString MediaPlayerBackend::authorName(void) const
{
  return "A.J. Admiraal";
}

QList<BackendServer *> MediaPlayerBackend::createServers(BackendServer::MasterServer *server)
{
  if (database == NULL)
  {
    database = new MediaDatabase(this, server->imdbClient());
    connect(database, SIGNAL(modified()), server->contentDirectory(), SLOT(modified()));
  }

  QList<BackendServer *> servers;
  servers += new MediaPlayerServer(database, MediaDatabase::Category_Movies,      QT_TR_NOOP("Movies"),       this, server);
  servers += new TvShowServer     (database, MediaDatabase::Category_TVShows,     QT_TR_NOOP("TV Shows"),     this, server);
  servers += new PlaylistServer   (database, MediaDatabase::Category_Clips,       QT_TR_NOOP("Video clips"),  this, server);
  servers += new PlaylistServer   (database, MediaDatabase::Category_HomeVideos,  QT_TR_NOOP("Home videos"),  this, server);
  servers += new PhotoServer      (database, MediaDatabase::Category_Photos,      QT_TR_NOOP("Photos"),       this, server);
  servers += new MusicServer      (database, MediaDatabase::Category_Music,       QT_TR_NOOP("Music"),        this, server);

  servers += new ConfigServer(this, server);

  return servers;
}


} // End of namespace

#include <QtPlugin>
Q_EXPORT_PLUGIN2("mediaplayer", LXiMediaCenter::MediaPlayerBackend);
