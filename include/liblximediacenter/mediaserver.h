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
#include "httpoutputnode.h"

namespace LXiMediaCenter {

class MediaServer : public BackendServer,
                    protected HttpServer::Callback,
                    private UPnPContentDirectory::Callback
{
Q_OBJECT
friend class MediaServerDir;
protected:
  class Stream : public SGraph
  {
  public:
    explicit                    Stream(MediaServer *, const QHostAddress &peer, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const HttpServer::RequestHeader &, QAbstractSocket *, STime duration, SInterval frameRate, SSize size, SAudioFormat::Channels channels);
    bool                        setup(const HttpServer::RequestHeader &, QAbstractSocket *, STime duration, SAudioFormat::Channels channels);

  public:
    const int                   id;
    MediaServer         * const parent;
    const QHostAddress          peer;
    const QString               url;

    STimeStampResamplerNode     timeStampResampler;
    SAudioResampleNode          audioResampler;
    SVideoDeinterlaceNode       deinterlacer;
    SSubpictureRenderNode       subpictureRenderer;
    SVideoLetterboxDetectNode   letterboxDetectNode;
    SVideoResizeNode            videoResizer;
    SVideoBoxNode               videoBox;
    SSubtitleRenderNode         subtitleRenderer;
    STimeStampSyncNode          sync;
    SAudioEncoderNode           audioEncoder;
    SVideoEncoderNode           videoEncoder;
    HttpOutputNode              output;

  private:
    static QAtomicInt           idCounter;
  };

  class TranscodeStream : public Stream
  {
  public:
    explicit                    TranscodeStream(MediaServer *, const QHostAddress &peer, const QString &url);

    bool                        setup(const HttpServer::RequestHeader &, QAbstractSocket *, SInterfaces::BufferReaderNode *, STime duration = STime());

  public:
    SAudioDecoderNode           audioDecoder;
    SVideoDecoderNode           videoDecoder;
    SDataDecoderNode            dataDecoder;
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

  typedef UPnPContentDirectory::Item Item;

public:
                                MediaServer(const char *, Plugin *, BackendServer::MasterServer *);
  virtual                       ~MediaServer();

protected:
  virtual void                  customEvent(QEvent *);

  virtual HttpServer::SocketOp  streamVideo(const HttpServer::RequestHeader &, QAbstractSocket *) = 0;
  virtual HttpServer::SocketOp  buildPlaylist(const HttpServer::RequestHeader &, QAbstractSocket *) = 0;

  virtual int                   countItems(const QString &path) = 0;
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;

protected slots:
  virtual void                  cleanStreams(void);

protected: // From HttpServer::Callback
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

private: // From DlnaServer::Callback
  virtual int                   countDlnaItems(const QString &path);
  virtual QList<UPnPContentDirectory::Item> listDlnaItems(const QString &path, unsigned start, unsigned count);

private:
  void                          addStream(Stream *);
  void                          removeStream(Stream *);

public:
  static const qint32           defaultDirSortOrder;
  static const qint32           defaultFileSortOrder;
  static const int              seekBySecs;

private:
  struct Private;
  Private               * const p;

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
  static QByteArray             buildVideoPlayer(const QByteArray &item, const SMediaInfo &, const QUrl &, const QSize & = QSize(768, 432));
  static QByteArray             buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &, const QSize & = QSize(768, 432));
};

} // End of namespace

#endif
