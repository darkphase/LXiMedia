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
  class FileStream : public TranscodeStream
  {
  public:
                                FileStream(MediaPlayerServer *, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID);
    virtual                     ~FileStream();

  public:
    const QDateTime             startTime;
    const MediaDatabase::UniqueID uid;

    SFileInputNode              file;
  };

  class DiscStream : public TranscodeStream
  {
  public:
                                DiscStream(MediaPlayerServer *, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID);
    virtual                     ~DiscStream();

  public:
    const QDateTime             startTime;
    const MediaDatabase::UniqueID uid;

    SDiscInputNode              disc;
  };

  struct PlayItem
  {
    inline PlayItem(MediaDatabase::UniqueID uid, const SMediaInfo &mediaInfo) : uid(uid), mediaInfo(mediaInfo) { }

    MediaDatabase::UniqueID     uid;
    SMediaInfo                  mediaInfo;
  };

public:
                                MediaPlayerServer(MediaDatabase *, MediaDatabase::Category, const char *, Plugin *, BackendServer::MasterServer *);

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  //void                          addVideoFile(DlnaServerDir *, const PlayItem &, const QString &, int = 0) const;
  //void                          addVideoFile(DlnaServerDir *, const QList<PlayItem> &, const QString &, int = 0) const;

  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *);
  virtual bool                  buildPlaylist(const QHttpRequestHeader &, QAbstractSocket *);

  static QString                videoFormatString(const SMediaInfo &);
  static QByteArray             buildVideoPlayer(MediaDatabase::UniqueID, const SMediaInfo &, const QUrl &, const QSize & = QSize(768, 432));
  static QByteArray             buildVideoPlayer(const QByteArray &, const QString &, const QUrl &, const QSize & = QSize(768, 432));

protected:
  MediaDatabase         * const mediaDatabase;
  const MediaDatabase::Category category;
};

class MediaPlayerServerDir : public MediaServerDir
{
Q_OBJECT
friend class MediaPlayerServer;
public:
  explicit                      MediaPlayerServerDir(MediaPlayerServer *, const QString &albumPath);

  virtual QStringList           listDirs(void);
  virtual QStringList           listFiles(void);
  virtual QString               getIcon(void) const;

  inline MediaPlayerServer    * server(void)                                    { return static_cast<MediaPlayerServer *>(MediaServerDir::server()); }
  inline const MediaPlayerServer * server(void) const                           { return static_cast<const MediaPlayerServer *>(MediaServerDir::server()); }

protected:
  virtual MediaPlayerServerDir * createDir(MediaPlayerServer *, const QString &albumPath);

protected:
  const QString                 albumPath;
};

} // End of namespace

#endif
