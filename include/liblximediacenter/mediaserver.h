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

#ifndef LXIMEDIACENTER_MEDIASERVER_H
#define LXIMEDIACENTER_MEDIASERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiServer>
#include <LXiStream>
#include "backendserver.h"
#include "mediaprofiles.h"
#include "export.h"

namespace LXiMediaCenter {

class MediaStream;

class LXIMEDIACENTER_PUBLIC MediaServer : public BackendServer,
                                          protected SHttpServer::Callback,
                                          private SUPnPContentDirectory::Callback
{
Q_OBJECT
friend class MediaServerDir;
public:
  struct LXIMEDIACENTER_PUBLIC Item : SUPnPContentDirectory::Item
  {
                                Item(void);
                                ~Item();

    SAudioFormat                audioFormat;
    SVideoFormat                videoFormat;
    SSize                       imageSize;
  };

  class LXIMEDIACENTER_PUBLIC Stream
  {
  public:
                                Stream(MediaServer *parent, const QString &url);
    virtual                     ~Stream();

  public:
    MediaServer         * const parent;
    const QString               url;
    SHttpStreamProxy            proxy;
  };

  struct LXIMEDIACENTER_PUBLIC ThumbnailListItem
  {
                                ThumbnailListItem(void);
                                ~ThumbnailListItem();

    QString                     title;
    QUrl                        iconurl;
    QUrl                        url;
    bool                        played;
  };

  typedef QList<ThumbnailListItem> ThumbnailListItemList;

  struct LXIMEDIACENTER_PUBLIC DetailedListItem
  {
    struct LXIMEDIACENTER_PUBLIC Column
    {
                                Column(QString = QString::null, QUrl = QUrl(), QUrl = QUrl());
                                ~Column();

      QString                   title;
      QUrl                      iconurl; 
      QUrl                      url;
    };

                                DetailedListItem(void);
                                ~DetailedListItem();

    QList<Column>               columns;
    bool                        played;
  };

  typedef QList<DetailedListItem> DetailedListItemList;

public:
  explicit                      MediaServer(QObject * = NULL);
  virtual                       ~MediaServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  static MediaProfiles        & mediaProfiles(void);
  static QSet<QString>        & activeClients(void);

protected:
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &) = 0;
  virtual SHttpServer::ResponseMessage sendPhoto(const SHttpServer::RequestMessage &) = 0;

  virtual int                   countItems(const QString &path) = 0;
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;
  virtual Item                  getItem(const QString &path) = 0;

protected slots:
  virtual void                  cleanStreams(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

private: // From UPnPContentDirectory::Callback
  virtual int                   countContentDirItems(const QString &client, const QString &path);
  virtual QList<SUPnPContentDirectory::Item> listContentDirItems(const QString &client, const QString &path, unsigned start, unsigned count);
  virtual SUPnPContentDirectory::Item getContentDirItem(const QString &client, const QString &path);

private:
  static SAudioFormat           audioFormatFor(const QString &client, const Item &item, bool &addVideo);
  static SVideoFormat           videoFormatFor(const QString &client, const Item &item);
  static void                   processItem(const QString &client, Item &);
  static void                   setQueryItemsFor(const QString &client, QUrl &, bool isMusic);
  void                          addStream(Stream *);
  void                          removeStream(Stream *);

public:
  static const qint32           defaultDirSortOrder;
  static const qint32           defaultFileSortOrder;
  static const int              seekBySecs;

private:
  struct Data;
  Data                  * const d;

protected: // Implemented in mediaserver.html.cpp
  static const char             m3uPlaylist[];
  static const char             m3uPlaylistItem[];
  static const char             htmlThumbnailLoader[];
  static const char             htmlThumbnailItem[];
  static const char             htmlThumbnailItemNoTitle[];
  static const char             htmlThumbnailItemNoLink[];
  static const char             htmlThumbnailItemNoLinkNoTitle[];
  static const char             htmlPhotoViewer[];
  static const char             htmlVideoPlayer[];
  static const char             htmlAudioPlayer[];

  QByteArray                    buildThumbnailLoader(const QString &path);
  QByteArray                    buildThumbnailItems(const ThumbnailListItemList &);
  SHttpServer::ResponseMessage  buildPhotoViewer(const SHttpServer::RequestMessage &);
  SHttpServer::ResponseMessage  buildVideoPlayer(const SHttpServer::RequestMessage &);
  SHttpServer::ResponseMessage  buildAudioPlayer(const SHttpServer::RequestMessage &);
};

} // End of namespace

#endif
