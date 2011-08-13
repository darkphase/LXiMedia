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

#ifndef MUSICSERVER_H
#define MUSICSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include "mediadatabase.h"
#include "playlist.h"
#include "playlistserver.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MusicServer : public PlaylistServer
{
Q_OBJECT
public:
                                MusicServer(MediaDatabase::Category, QObject *);
  virtual                       ~MusicServer();

protected:
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

  QStringList                   playlists(void) const;
  Playlist                    * createPlaylist(const QString &);
  void                          storePlaylist(Playlist *, const QString &);

private:
  QDir                          playlistDir;

private:
  static const char     * const htmlMusicStreams;
  static const char     * const htmlMusicPlaylist;
  static const char     * const htmlMusicStream;
  static const char     * const htmlMusic;
  static const char     * const htmlList;
  static const char     * const htmlListItem;
  static const char     * const htmlListAltItem;
  static const char     * const htmlMusicPlayer;
  static const char     * const htmlPlayListItem;
  static const char     * const htmlPlayListAltItem;
  static const char     * const htmlPlayListSelItem;
};

} } // End of namespaces

#endif
