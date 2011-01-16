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

#ifndef MEDIASERVER_H
#define MEDIASERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>
#include "mediadatabase.h"

namespace LXiMediaCenter {

class MediaServer : public VideoServer
{
Q_OBJECT
protected:
  class FileStream : public TranscodeStream
  {
  public:
                                FileStream(MediaServer *, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID);
    virtual                     ~FileStream();

  public:
    const QDateTime             startTime;
    const MediaDatabase::UniqueID uid;

    SFileInputNode              file;
  };

  class DiscStream : public TranscodeStream
  {
  public:
                                DiscStream(MediaServer *, const QHostAddress &peer, const QString &url, const QString &fileName, MediaDatabase::UniqueID);
    virtual                     ~DiscStream();

  public:
    const QDateTime             startTime;
    const MediaDatabase::UniqueID uid;

    SDiscInputNode              disc;
  };

  struct PlayItem
  {
    inline PlayItem(MediaDatabase::UniqueID uid, const SMediaInfo &mediaInfo, const QDateTime &lastModified) : uid(uid), mediaInfo(mediaInfo), lastModified(lastModified) { }
    inline PlayItem(const MediaDatabase::Node &node) : uid(node.uid), mediaInfo(node.mediaInfo), lastModified(node.lastModified) { }

    MediaDatabase::UniqueID     uid;
    SMediaInfo                  mediaInfo;
    QDateTime                   lastModified;
  };

public:
                                MediaServer(const char *, MediaDatabase *, Plugin *, BackendServer::MasterServer *);

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  void                          enableDlna(void);
  void                          addVideoFile(DlnaServerDir *, const PlayItem &, const QString &, int = 0) const;
  void                          addVideoFile(DlnaServerDir *, const QList<PlayItem> &, const QString &, int = 0) const;

  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *);
  virtual bool                  buildPlaylist(const QHttpRequestHeader &, QAbstractSocket *);

  virtual bool                  handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

  static QString                videoFormatString(const SMediaInfo &);
  static QByteArray             buildVideoPlayer(const MediaDatabase::Node &, const QUrl &, const QSize & = QSize(768, 432));

protected:
  static const int              seekBySecs;

  MediaDatabase         * const mediaDatabase;
};

class MediaServerFileDir : public DlnaServerDir
{
Q_OBJECT
public:
  explicit inline               MediaServerFileDir(DlnaServer *parent) : DlnaServerDir(parent) { }

public:
  QList<MediaDatabase::UniqueID> uids;
};

} // End of namespace

#endif
