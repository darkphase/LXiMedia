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
namespace MediaPlayerBackend {

class MediaPlayerServerDir;

class MediaPlayerServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(MediaPlayerServer *, SSandboxClient *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request, const QByteArray &content);

  public:
    SSandboxClient      * const sandbox;
  };

  struct PlayItem
  {
    inline PlayItem(MediaDatabase::UniqueID uid, const SMediaInfo &mediaInfo) : uid(uid), mediaInfo(mediaInfo) { }

    MediaDatabase::UniqueID     uid;
    SMediaInfo                  mediaInfo;
  };

public:
                                MediaPlayerServer(MediaDatabase::Category, QObject *);

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               pluginName(void) const;

  virtual SearchResultList      search(const QStringList &) const;

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);

  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);

protected:
  bool                          isEmpty(const QString &path);
  int                           countAlbums(const QString &path);
  QList<Item>                   listAlbums(const QString &path, unsigned &start, unsigned &count);
  Item                          makeItem(MediaDatabase::UniqueID, bool recursePrograms = true);
  QUrl                          findAlbumIcon(const QString &path);
  Item::Type                    defaultItemType(Item::Type = Item::Type_None) const;

  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);

  static QString                videoFormatString(const SMediaInfo::Program &);
  QByteArray                    buildVideoPlayer(const QString &dirPath, MediaDatabase::UniqueID, const QString &, const SMediaInfo::Program &, const QUrl &, const QSize & = QSize(768, 432));
  QByteArray                    buildVideoPlayer(const QString &dirPath, const QByteArray &, const QString &, const QUrl &, const QSize & = QSize(768, 432));

private slots:
  void                          consoleLine(const QString &);

protected:
  const MediaDatabase::Category category;
  MasterServer                * masterServer;
  MediaDatabase               * mediaDatabase;
};

} } // End of namespaces

#endif
