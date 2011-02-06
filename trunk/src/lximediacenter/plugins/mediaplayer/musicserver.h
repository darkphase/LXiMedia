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
#include "mediaplayerserver.h"
#include "playlist.h"
#include "playlistnode.h"

namespace LXiMediaCenter {

class MusicServer : public MediaPlayerServer
{
Q_OBJECT
protected:
  class PlaylistStream : public Stream
  {
  public:
                                PlaylistStream(MusicServer *, int id, const QHostAddress &peer, const QString &url, MediaDatabase *, Playlist *, const QString &);
    virtual                     ~PlaylistStream();

    bool                        setup(const QHttpRequestHeader &, QAbstractSocket *);

  public:
    static const int            frameRate = 25;
    MusicServer         * const parent;
    const int                   id;
    const QString               name;

    PlaylistNode                playlistNode;
  };

public:
                                MusicServer(MediaDatabase *, Plugin *, MasterServer *);
  virtual                       ~MusicServer();

  virtual SearchResultList      search(const QStringList &) const;

  virtual void                  updateDlnaTask(void);

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *);

  virtual bool                  handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

  QStringList                   playlists(void) const;
  Playlist                    * createPlaylist(const QString &);
  void                          storePlaylist(Playlist *, const QString &);
  int                           createStream(const QString &, const QHostAddress &, const QString &);

private:

private:
  DlnaServerDir               * musicVideosDir;

  QDir                          playlistDir;
  QAtomicInt                    nextStreamId;
  QMap<int, PlaylistStream *>   streams;

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

} // End of namespace

#endif
