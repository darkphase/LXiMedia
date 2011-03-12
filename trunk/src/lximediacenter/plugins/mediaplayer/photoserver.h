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
#include "slideshownode.h"

namespace LXiMediaCenter {

class PhotoServer : public PlaylistServer
{
Q_OBJECT
protected:
  class SlideShowStream : public Stream
  {
  public:
                                SlideShowStream(PhotoServer *, const QHostAddress &peer, const QString &url, const QList<MediaDatabase::File> &);

    bool                        setup(const HttpServer::RequestHeader &, QAbstractSocket *);

  public:
    SlideShowNode               slideShow;
  };

public:
                                PhotoServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, MasterServer *);
  virtual                       ~PhotoServer();

protected:
  virtual HttpServer::SocketOp  streamVideo(const HttpServer::RequestHeader &, QAbstractSocket *);

  virtual QList<Item>           listItems(const QString &path, unsigned start, unsigned count);
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

private:
  HttpServer::SocketOp          sendPhoto(const HttpServer::RequestHeader &, QAbstractSocket *, MediaDatabase::UniqueID, const QString &format) const;
  HttpServer::SocketOp          handleHtmlRequest(const HttpServer::RequestHeader &, QAbstractSocket *, const QString &);

private:
  static const char     * const htmlView;
};

} // End of namespace

#endif
