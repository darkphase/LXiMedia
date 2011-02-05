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

#ifndef LXMEDIACENTER_VIDEOSERVER_H
#define LXMEDIACENTER_VIDEOSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <LXiStream>
#include "backendserver.h"
#include "httpoutputnode.h"

namespace LXiMediaCenter {


class VideoServer : public BackendServer
{
Q_OBJECT
protected:
  class Stream : public SGraph
  {
  public:
    explicit                    Stream(VideoServer *, const QHostAddress &peer, const QString &url);
    virtual                     ~Stream();

    void                        setup(bool, const QString & = QString::null, const QImage & = QImage());

  public:
    const int                   id;
    VideoServer         * const parent;
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
    explicit                    TranscodeStream(VideoServer *, const QHostAddress &peer, const QString &url);

    bool                        setup(const QHttpRequestHeader &, QAbstractSocket *, SInterfaces::BufferReaderNode *, STime duration, const QString & = QString::null, const QImage & = QImage());

  public:
    SAudioDecoderNode           audioDecoder;
    SVideoDecoderNode           videoDecoder;
    SDataDecoderNode            dataDecoder;
  };

  struct ThumbnailListItem
  {
    inline ThumbnailListItem(void) : played(false) { }

    QString                     page;
    QString                     title;
    QString                     subtitle;
    QString                     iconurl;
    QString                     url;
    bool                        played;
  };

  typedef QMultiMap<QString, ThumbnailListItem> ThumbnailListItemMap;

public:
                                VideoServer(const char *, Plugin *, BackendServer::MasterServer *);
  virtual                       ~VideoServer();

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *) = 0;
  virtual bool                  buildPlaylist(const QHttpRequestHeader &, QAbstractSocket *) = 0;

  virtual void                  customEvent(QEvent *);

protected slots:
  virtual void                  cleanStreams(void);

private:
  void                          addStream(Stream *);
  void                          removeStream(Stream *);

protected:
  mutable QReadWriteLock        lock;

private:
  struct Private;
  Private               * const p;

protected:
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

  static QByteArray             buildThumbnailView(const QString &title, const ThumbnailListItemMap &, const QUrl &);
  static QByteArray             buildVideoPlayer(const QByteArray &item, const SMediaInfo &, const QUrl &, const QSize & = QSize(768, 432));
};


} // End of namespace

#endif
