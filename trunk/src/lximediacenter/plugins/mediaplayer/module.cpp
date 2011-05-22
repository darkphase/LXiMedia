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

#include "module.h"
#include "configserver.h"
#include "mediadatabase.h"
#include "mediaplayersandbox.h"
#include "mediaplayerserver.h"
#include "musicserver.h"
#include "photoserver.h"
#include "tvshowserver.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

template <class _base, MediaDatabase::Category _category, const char * _name, const char * _icon>
class Server : public _base
{
public:
  explicit Server(const QString &, QObject *parent)
    : _base(_category, parent)
  {
  }

  virtual QString serverName(void) const
  {
    return _name;
  }

  virtual QString serverIconPath(void) const
  {
    return _icon;
  }
};

const char Module::pluginName[]     = QT_TR_NOOP("Media player");

const char Module::moviesName[]     = QT_TR_NOOP("Movies"),       Module::moviesIcon[] = "/img/media-optical.png";
const char Module::tvShowsName[]    = QT_TR_NOOP("TV Shows"),     Module::tvShowsIcon[] = "/img/video-television.png";
const char Module::clipsName[]      = QT_TR_NOOP("Video clips"),  Module::clipsIcon[] = "/img/phone.png";
const char Module::homeVideosName[] = QT_TR_NOOP("Home videos"),  Module::homeVideosIcon[] = "/img/media-tape.png";
const char Module::photosName[]     = QT_TR_NOOP("Photos"),       Module::photosIcon[] = "/img/camera-photo.png";
const char Module::musicName[]      = QT_TR_NOOP("Music"),        Module::musicIcon[] = "/img/audio-headset.png";

bool Module::registerClasses(void)
{
  MediaPlayerServer::registerClass< Server<MediaPlayerServer, MediaDatabase::Category_Movies,     moviesName,     moviesIcon> >(0);
  MediaPlayerServer::registerClass< Server<TvShowServer,      MediaDatabase::Category_TVShows,    tvShowsName,    tvShowsIcon> >(-1);
  MediaPlayerServer::registerClass< Server<PlaylistServer,    MediaDatabase::Category_Clips,      clipsName,      clipsIcon> >(-2);
  MediaPlayerServer::registerClass< Server<PlaylistServer,    MediaDatabase::Category_HomeVideos, homeVideosName, homeVideosIcon> >(-3);
  MediaPlayerServer::registerClass< Server<PhotoServer,       MediaDatabase::Category_Photos,     photosName,     photosIcon> >(-4);
  MediaPlayerServer::registerClass< Server<MusicServer,       MediaDatabase::Category_Music,      musicName,      musicIcon> >(-5);
  MediaPlayerServer::registerClass<ConfigServer>(-6);

  MediaPlayerSandbox::registerClass<MediaPlayerSandbox>(0);

  return true;
}

void Module::unload(void)
{
  MediaDatabase::destroyInstance();
}

QByteArray Module::about(void)
{
  return QByteArray(pluginName) + " by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  return QByteArray();
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lximediacenter_mediaplayer, LXiMediaCenter::MediaPlayerBackend::Module);
