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

  struct ThumbnailListItem
  {
    inline ThumbnailListItem(void) : played(false) { }

    QString                     title;
    QString                     subtitle;
    QUrl                        iconurl;
    QUrl                        url;
    bool                        played;
  };

  typedef QList<ThumbnailListItem> ThumbnailListItemList;

  struct DetailedListItem
  {
    inline DetailedListItem(void) : played(false) { }

    QStringList                 columns;
    QUrl                        iconurl;
    QUrl                        url;
    bool                        played;
  };

  typedef QList<DetailedListItem> DetailedListItemList;

  typedef SUPnPContentDirectory::Item Item;

public:
  explicit                      MediaServer(QObject * = NULL);
  virtual                       ~MediaServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

protected:
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &) = 0;

  virtual int                   countItems(const QString &path) = 0;
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;

protected slots:
  virtual void                  cleanStreams(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);

private: // From UPnPContentDirectory::Callback
  virtual int                   countContentDirItems(const QString &path);
  virtual QList<SUPnPContentDirectory::Item> listContentDirItems(const QString &path, unsigned start, unsigned count);

private:
  __internal void               addStream(Stream *);
  __internal void               removeStream(Stream *);

public:
  __internal static const qint32 defaultDirSortOrder;
  __internal static const qint32 defaultFileSortOrder;
  __internal static const int   seekBySecs;

private:
  struct Data;
  Data                  * const d;

protected: // Implemented in mediaserver.html.cpp
  static const unsigned         itemsPerThumbnailPage;
  static const char     * const audioTimeFormat;
  static const char     * const videoTimeFormat;

  static const char     * const m3uPlaylist;
  static const char     * const m3uPlaylistItem;
  static const char     * const htmlPages;
  static const char     * const htmlPageItem;
  static const char     * const htmlPageCurrentItem;
  static const char     * const htmlThumbnails;
  static const char     * const htmlThumbnailItemRow;
  static const char     * const htmlThumbnailItem;
  static const char     * const htmlThumbnailItemNoTitle;
  static const char     * const htmlDetailedList;
  static const char     * const htmlDetailedListRow;
  static const char     * const htmlDetailedListHead;
  static const char     * const htmlDetailedListIcon;
  static const char     * const htmlDetailedListColumn;
  static const char     * const htmlDetailedListColumnLink;
  static const char     * const htmlPlayer;
  static const char     * const htmlPlayerAudioItem;
  static const char     * const htmlPlayerVideoItem;
  static const char     * const htmlPlayerThumbItem;
  static const char     * const htmlPlayerThumbItemOption;
  static const char     * const htmlPlayerInfoItem;
  static const char     * const htmlPlayerInfoActionHead;
  static const char     * const htmlPlayerInfoAction;

  static const char     * const headList;
  static const char     * const headPlayer;

  QByteArray                    buildThumbnailView(const QString &path, const ThumbnailListItemList &, int, int);
  QByteArray                    buildDetailedView(const QString &path, const QStringList &columns, const DetailedListItemList &);
  static QByteArray             buildVideoPlayer(const QByteArray &item, const SMediaInfo::Program &, const QUrl &, const QSize & = QSize(768, 432));
  static QByteArray             buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &, const QSize & = QSize(768, 432));
};

} // End of namespace

#endif
