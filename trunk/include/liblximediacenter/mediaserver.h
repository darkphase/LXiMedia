/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#ifndef LXIMEDIACENTER_MEDIASERVER_H
#define LXIMEDIACENTER_MEDIASERVER_H

#include <QtCore>
#include <LXiStream>
#include "backendserver.h"
#include "mediaprofiles.h"
#include "rootdevice.h"
#include "export.h"

namespace LXiMediaCenter {

class MediaStream;

class LXIMEDIACENTER_PUBLIC MediaServer : public BackendServer,
                                          protected UPnP::HttpCallback,
                                          private ContentDirectory::Callback
{
Q_OBJECT
friend class MediaServerDir;
public:
  struct LXIMEDIACENTER_PUBLIC Item : ContentDirectory::Item
  {
                                Item(void);
                                ~Item();

    SAudioFormat                audioFormat;
    SVideoFormat                videoFormat;
    SSize                       imageSize;
  };

  struct LXIMEDIACENTER_PUBLIC ThumbnailListItem
  {
                                ThumbnailListItem(void);
                                ~ThumbnailListItem();

    QString                     title;
    QStringList                 text;
    QUrl                        iconurl;
    QUrl                        url;
    QString                     func;
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

  struct TranscodeSize
  {
    inline TranscodeSize(void) { }
    inline TranscodeSize(const QString &name, const SSize &size) : name(name), size(size) { }

    QString                     name;
    SSize                       size;
  };

  struct TranscodeChannel
  {
    inline TranscodeChannel(void) { }
    inline TranscodeChannel(const QString &name, const SAudioFormat::Channels &channels) : name(name), channels(channels) { }

    QString                     name;
    SAudioFormat::Channels      channels;
  };

  struct SubtitleSize
  {
    inline SubtitleSize(void) { }
    inline SubtitleSize(const QString &name, float ratio) : name(name), ratio(ratio) { }

    QString                     name;
    float                       ratio;
  };

  enum ListType
  {
    ListType_Thumbnails,
    ListType_Details
  };

public:
  explicit                      MediaServer(QObject * = NULL);
  virtual                       ~MediaServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  static HttpStatus             makeThumbnail(QIODevice *&, QByteArray &, QSize, const QImage &, const QString & = QString::null);

  static QSet<QString>        & fileProtocols(void);
  static MediaProfiles        & mediaProfiles(void);
  static QSet<QByteArray>     & activeClients(void);

  static QList<TranscodeSize>   allTranscodeSizes(void);
  static QString                defaultTranscodeSizeName(void);
  static QString                defaultTranscodeCropName(void);
  static QString                defaultEncodeModeName(void);
  static QList<TranscodeChannel> allTranscodeChannels(void);
  static QString                defaultTranscodeChannelName(void);
  static QString                defaultTranscodeMusicChannelName(void);
  static bool                   defaultMusicAddBlackVideo(void);
  static QList<SubtitleSize>    allSubtitleSizes(void);
  static QString                defaultSubtitleSizeName(void);

  static int                    loadItemCount(void);

protected:
  virtual MediaStream         * streamVideo(const QUrl &request) = 0;
  virtual HttpStatus            sendPhoto(const QUrl &request, QByteArray &contentType, QIODevice *&response) = 0;

  virtual QList<Item>           listItems(const QString &path, int start, int &count) = 0;
  virtual Item                  getItem(const QString &path) = 0;
  virtual ListType              listType(const QString &path);

protected slots:
  virtual void                  cleanStreams(void);

protected: // From Server::HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response);

private: // From ContentDirectory::Callback
  virtual QList<ContentDirectory::Item> listContentDirItems(const QByteArray &client, const QString &path, int start, int &count);
  virtual ContentDirectory::Item getContentDirItem(const QByteArray &client, const QString &path);

private:
  static SAudioFormat           audioFormatFor(const QByteArray &client, const Item &item, bool &addVideo);
  static SVideoFormat           videoFormatFor(const QByteArray &client, const Item &item);
  static void                   processItem(const QByteArray &client, Item &);
  static void                   setQueryItemsFor(const QByteArray &client, QUrlQuery &query, bool isMusic);

public:
  static const qint32           defaultDirSortOrder;
  static const qint32           defaultFileSortOrder;
  static const int              seekBySecs;

private:
  struct Data;
  Data                  * const d;

private: // Implemented in mediaserver.html.cpp
  static const char             m3uPlaylist[];
  static const char             m3uPlaylistItem[];
  static const char             htmlListHead[];
  static const char             htmlListLoader[];
  static const char             htmlListItemLink[];
  static const char             htmlListItemLinkNoTitle[];
  static const char             htmlListItemFunc[];
  static const char             htmlListItemFuncNoTitle[];
  static const char             htmlListItemNoLink[];
  static const char             htmlListItemNoLinkNoTitle[];
  static const char             htmlListItemTextLine[];
  static const char             htmlAudioPlayer[];
  static const char             htmlAudioPlayerElement[];
  static const char             htmlAudioPlayerSource[];
  static const char             htmlPlayer[];
  static const char             htmlVideoPlayerElement[];
  static const char             htmlVideoPlayerSource[];

  QByteArray                    buildListLoader(const QString &path, ListType);
  QByteArray                    buildListItems(const ThumbnailListItemList &);
  HttpStatus                    buildAudioPlayer(const QUrl &, QByteArray &contentType, QIODevice *&response);
  HttpStatus                    buildPlayer(const QUrl &, QByteArray &contentType, QIODevice *&response);
};

} // End of namespace

#endif
