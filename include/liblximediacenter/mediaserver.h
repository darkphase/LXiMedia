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

  struct LXIMEDIACENTER_PUBLIC ThumbnailListItem
  {
                                ThumbnailListItem(void);
                                ~ThumbnailListItem();

    QString                     title;
    QString                     subtitle;
    QUrl                        iconurl;
    QUrl                        url;
    bool                        played;
  };

  typedef QList<ThumbnailListItem> ThumbnailListItemList;

  struct LXIMEDIACENTER_PUBLIC DetailedListItem
  {
                                DetailedListItem(void);
                                ~DetailedListItem();

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
  virtual void                  handleHttpOptions(SHttpServer::ResponseHeader &);

private: // From UPnPContentDirectory::Callback
  virtual int                   countContentDirItems(const QString &path);
  virtual QList<SUPnPContentDirectory::Item> listContentDirItems(const QString &path, unsigned start, unsigned count);

private:
  _lxi_internal void            addStream(Stream *);
  _lxi_internal void            removeStream(Stream *);

public:
  _lxi_internal static const qint32 defaultDirSortOrder;
  _lxi_internal static const qint32 defaultFileSortOrder;
  _lxi_internal static const int seekBySecs;

private:
  struct Data;
  Data                  * const d;

public: // Implemented in mediaserver.html.cpp
  static void                   enableHtml5(bool = true);

protected: // Implemented in mediaserver.html.cpp
  static bool                   html5Enabled;
  static const unsigned         itemsPerThumbnailPage;
  static const char             audioTimeFormat[];
  static const char             videoTimeFormat[];

  static const char             m3uPlaylist[];
  static const char             m3uPlaylistItem[];
  static const char             htmlPages[];
  static const char             htmlPageItem[];
  static const char             htmlPageCurrentItem[];
  static const char             htmlThumbnails[];
  static const char             htmlThumbnailItem[];
  static const char             htmlThumbnailItemNoTitle[];
  static const char             htmlDetailedList[];
  static const char             htmlDetailedListRow[];
  static const char             htmlDetailedListHead[];
  static const char             htmlDetailedListIcon[];
  static const char             htmlDetailedListColumn[];
  static const char             htmlDetailedListColumnLink[];
  static const char             htmlPlayer[];
  static const char             htmlPlayerAudioItem[];
  static const char             htmlPlayerAudioItemHtml5[];
  static const char             htmlPlayerVideoItem[];
  static const char             htmlPlayerVideoItemHtml5[];
  static const char             htmlPlayerThumbItem[];
  static const char             htmlPlayerThumbItemOption[];
  static const char             htmlPlayerInfoItem[];
  static const char             htmlPlayerInfoActionHead[];
  static const char             htmlPlayerInfoAction[];

  static const char             headPlayer[];

  QByteArray                    buildThumbnailView(const QString &path, const ThumbnailListItemList &, int, int);
  QByteArray                    buildDetailedView(const QString &path, const QStringList &columns, const DetailedListItemList &);
  static QByteArray             buildVideoPlayer(const QByteArray &item, const QString &title, const SMediaInfo::Program &, const QUrl &, const QSize & = QSize(768, 432), SAudioFormat::Channels = SAudioFormat::Channels_Stereo);
  static QByteArray             buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &, const QSize & = QSize(768, 432), SAudioFormat::Channels = SAudioFormat::Channels_Stereo);
};

} // End of namespace

#endif
