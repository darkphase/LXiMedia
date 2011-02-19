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

#ifndef PLAYLISTSERVER_H
#define PLAYLISTSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include "mediadatabase.h"
#include "mediaplayerserver.h"

namespace LXiMediaCenter {

class PlaylistServer : public MediaPlayerServer
{
Q_OBJECT
protected:
  class PlaylistStream : public TranscodeStream
  {
  public:
                                PlaylistStream(PlaylistServer *, const QHostAddress &peer, const QString &url, const SMediaInfoList &files);

    bool                        setup(const HttpServer::RequestHeader &, QAbstractSocket *);

  public:
    SPlaylistNode               playlistNode;
  };

public:
                                PlaylistServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, MasterServer *, const QString & = tr("Play all"));
  virtual                       ~PlaylistServer();

protected:
  virtual HttpServer::SocketOp  streamVideo(const HttpServer::RequestHeader &, QAbstractSocket *);

  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start, unsigned count);
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

  QList<Item>                   listPlayAllItem(const QString &path,  unsigned &start, unsigned &count, MediaDatabase::UniqueID = 0);

private:
  const QString                 itemTitle;

private:
  static const char     * const htmlView;
};

} // End of namespace

#endif
