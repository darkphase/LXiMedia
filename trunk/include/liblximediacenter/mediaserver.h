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
#include <LXiStream>
#include "backendserver.h"
#include "httpoutputnode.h"

namespace LXiMediaCenter {

class MediaServerDir;

typedef FileServerHandle<MediaServerDir> MediaServerDirHandle;
typedef FileServerHandle<const MediaServerDir> MediaServerConstDirHandle;

class MediaServer : public BackendServer,
                    public FileServer
{
Q_OBJECT
friend class MediaServerDir;
protected:
  class Stream : public SGraph
  {
  public:
    explicit                    Stream(MediaServer *, const QHostAddress &peer, const QString &url);
    virtual                     ~Stream();

    void                        setup(bool, const QString & = QString::null, const QImage & = QImage());

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

private:
  class HttpDir : public HttpServerDir
  {
  public:
    explicit                    HttpDir(HttpServer *, MediaServer *, const QString &mediaPath);

    virtual QStringList         listDirs(void);

    virtual bool                handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

  private:
    MediaServer         * const mediaServer;
    const QString               mediaPath;
  };

  class DlnaDir : public DlnaServerDir
  {
  public:
    explicit                    DlnaDir(DlnaServer *, MediaServer *, const QString &mediaPath);

    virtual QStringList         listDirs(void);
    virtual QStringList         listFiles(void);

  private:
    MediaServer         * const mediaServer;
    const QString               mediaPath;
    QStringList                 plainFiles;
  };

public:
                                MediaServer(const char *, Plugin *, BackendServer::MasterServer *);
  virtual                       ~MediaServer();

  void                          setRoot(MediaServerDir *);

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual bool                  buildDir(const QUrl &, const QString &, QAbstractSocket *);
  virtual void                  customEvent(QEvent *);

  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *) = 0;
  virtual bool                  buildPlaylist(const QHttpRequestHeader &, QAbstractSocket *) = 0;

protected slots:
  virtual void                  cleanStreams(void);

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
  static QByteArray             buildVideoPlayer(const QByteArray &item, const QString &title, const QUrl &, const QSize & = QSize(768, 432));
};

class MediaServerDir : public FileServerDir
{
Q_OBJECT
friend class MediaServer;
public:
  struct File
  {
    inline File(void) : played(false), music(false), sortOrder(MediaServer::defaultFileSortOrder) { }

    inline bool                 isNull(void) const                              { return url.isEmpty(); }

    bool                        played;
    bool                        music;
    qint32                      sortOrder;
    QString                     mimeType;
    QString                     url;
    QString                     iconUrl;
    SMediaInfo                  mediaInfo;
  };

public:
  explicit                      MediaServerDir(MediaServer *);

  virtual void                  addFile(const QString &name, const File &);
  virtual void                  removeFile(const QString &name);
  virtual void                  clear(void);
  virtual int                   count(void) const;

  inline MediaServer          * server(void)                                    { return static_cast<MediaServer *>(FileServerDir::server()); }
  inline const MediaServer    * server(void) const                              { return static_cast<const MediaServer *>(FileServerDir::server()); }

  virtual QStringList           listFiles(void);
  virtual File                  findFile(const QString &name);

  inline QStringList            listFiles(void) const                           { return const_cast<MediaServerDir *>(this)->listFiles(); }
  inline File                   findFile(const QString &name) const             { return const_cast<MediaServerDir *>(this)->findFile(name); }

  virtual QString               getIcon(void) const;

public:
  qint32                        sortOrder;

private:
  QMap<QString, File>           files;
};


} // End of namespace

#endif
