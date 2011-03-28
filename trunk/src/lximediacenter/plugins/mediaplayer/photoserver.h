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

#ifndef PHOTOSERVER_H
#define PHOTOSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include "mediadatabase.h"
#include "playlistserver.h"

namespace LXiMediaCenter {

class PhotoServer : public PlaylistServer
{
Q_OBJECT
public:
                                PhotoServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, MasterServer *);
  virtual                       ~PhotoServer();

protected:
  virtual Stream              * streamVideo(const SHttpServer::RequestHeader &);

  virtual QList<Item>           listItems(const QString &path, unsigned start, unsigned count);
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestHeader &, QIODevice *);

private:
  SHttpServer::SocketOp         sendPhoto(const SHttpServer::RequestHeader &, QIODevice *, MediaDatabase::UniqueID, const QString &format) const;
  SHttpServer::SocketOp         handleHtmlRequest(const SHttpServer::RequestHeader &, QIODevice *, const QString &);

private:
  static const char     * const htmlView;
};

} // End of namespace

#endif
