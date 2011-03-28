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

class PlaylistServer;


class PlaylistServer : public MediaPlayerServer
{
Q_OBJECT
public:
                                PlaylistServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, MasterServer *, const QString & = tr("Play all"));
  virtual                       ~PlaylistServer();

protected:
  virtual Stream              * streamVideo(const SHttpServer::RequestHeader &);

  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestHeader &, QIODevice *);

  QList<Item>                   listPlayAllItem(const QString &path,  unsigned &start, unsigned &count, MediaDatabase::UniqueID = 0, const QList<Item> &thumbs = QList<Item>());

private:
  const QString                 itemTitle;

private:
  static const char     * const htmlView;
};

} // End of namespace

#endif
