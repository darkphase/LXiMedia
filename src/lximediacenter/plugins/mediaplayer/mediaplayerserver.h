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

#ifndef MEDIAPLAYERSERVER_H
#define MEDIAPLAYERSERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>
#include "mediadatabase.h"

namespace LXiMediaCenter {

class MediaPlayerServerDir;

class MediaPlayerServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(MediaPlayerServer *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request, const QByteArray &content);

  public:
    SSandboxClient            * sandbox;
  };

  struct PlayItem
  {
    inline PlayItem(MediaDatabase::UniqueID uid, const SMediaInfo &mediaInfo) : uid(uid), mediaInfo(mediaInfo) { }

    MediaDatabase::UniqueID     uid;
    SMediaInfo                  mediaInfo;
  };

public:
                                MediaPlayerServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, BackendServer::MasterServer *);

protected:
  //void                          addVideoFile(DlnaServerDir *, const PlayItem &, const QString &, int = 0) const;
  //void                          addVideoFile(DlnaServerDir *, const QList<PlayItem> &, const QString &, int = 0) const;

  virtual Stream              * streamVideo(const SHttpServer::RequestHeader &);

  bool                          isEmpty(const QString &path);
  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);

  int                           countAlbums(const QString &path);
  QList<Item>                   listAlbums(const QString &path, unsigned &start, unsigned &count);
  Item                          makeItem(MediaDatabase::UniqueID, bool recursePrograms = true);
  Item::Type                    defaultItemType(Item::Type = Item::Type_None) const;

  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestHeader &, QIODevice *);

  static QString                videoFormatString(const SMediaInfo::Program &);
  static QByteArray             buildVideoPlayer(MediaDatabase::UniqueID, const SMediaInfo::Program &, const QUrl &, const QSize & = QSize(768, 432));
  static QByteArray             buildVideoPlayer(const QByteArray &, const QString &, const QUrl &, const QSize & = QSize(768, 432));

private slots:
  void                          consoleLine(const QString &);

protected:
  MediaDatabase         * const mediaDatabase;
  const MediaDatabase::Category category;
};

} // End of namespace

#endif
