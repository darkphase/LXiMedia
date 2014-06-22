/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef LXIMEDIACENTER_CONTENTDIRECTORY_H
#define LXIMEDIACENTER_CONTENTDIRECTORY_H

#include <QtCore>
#include "export.h"
#include "rootdevice.h"
#include "connectionmanager.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC ContentDirectory : public QObject,
                                               public RootDevice::Service,
                                               private UPnP::HttpCallback
{
Q_OBJECT
public:
  struct LXIMEDIACENTER_PUBLIC Item
  {
    enum Type
    {
      Type_None                 = 0,

      Type_Audio                = 10,
      Type_Music,
      Type_AudioBroadcast,
      Type_AudioBook,

      Type_Video                = 20,
      Type_Movie,
      Type_VideoBroadcast,
      Type_MusicVideo,

      Type_Image                = 30,
      Type_Photo
    };

    struct LXIMEDIACENTER_PUBLIC Stream
    {
                                Stream(void);
                                ~Stream();

      QString                   title;
      QList< QPair<QString, QString> > queryItems;
    };

    struct LXIMEDIACENTER_PUBLIC Chapter
    {
                                Chapter(const QString &title = QString::null, unsigned position = 0);
                                ~Chapter();

      QString                   title;
      unsigned                  position;
    };

                                Item(void);
                                ~Item();

    bool                        isNull(void) const;
    bool                        isAudio(void) const;
    bool                        isVideo(void) const;
    bool                        isImage(void) const;
    bool                        isMusic(void) const;

    bool                        isDir;
    quint8                      type;
    QUrl                        url;
    QUrl                        iconUrl;
    QString                     path;

    QString                     title;
    QString                     artist;
    QString                     album;
    int                         track;

    unsigned                    duration; //!< In seconds.
    int                         lastPosition; //!< In seconds, -1 = unknown.

    QList<Stream>               streams;
    QList<Chapter>              chapters;
    ConnectionManager::ProtocolList protocols;
  };

  struct Callback
  {
    virtual QList<Item>         listContentDirItems(const QByteArray &client, const QString &path, int start, int &count) = 0;
    virtual Item                getContentDirItem(const QByteArray &client, const QString &path) = 0;
  };

public:
  struct LXIMEDIACENTER_PUBLIC BrowseItem
  {
                                BrowseItem();
                                ~BrowseItem();

    QByteArray                  id;
    QByteArray                  parentID;
    bool                        restricted;
    QByteArray                  title;
    quint32                     duration;
    QList< QPair<QByteArray, QByteArray> > attributes;
    QList< QPair<QByteArray, ConnectionManager::Protocol> > files;
  };

  struct LXIMEDIACENTER_PUBLIC BrowseContainer
  {
                                BrowseContainer();
                                ~BrowseContainer();

    QByteArray                  id;
    QByteArray                  parentID;
    bool                        restricted;
    quint32                     childCount;
    QByteArray                  title;
    QList< QPair<QByteArray, QByteArray> > attributes;
  };

  struct LXIMEDIACENTER_PUBLIC ActionBrowse
  {
    enum BrowseFlag { BrowseMetadata, BrowseDirectChildren };

    virtual QByteArray          getObjectID() const = 0;
    virtual BrowseFlag          getBrowseFlag() const = 0;
    virtual QByteArray          getFilter() const = 0;
    virtual quint32             getStartingIndex() const = 0;
    virtual quint32             getRequestedCount() const = 0;
    virtual QByteArray          getSortCriteria() const = 0;

    virtual void                addItem(const BrowseItem &) = 0;
    virtual void                addContainer(const BrowseContainer &) = 0;
    virtual void                setResponse(quint32 totalMatches, quint32 updateID) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionSearch
  {
    virtual QByteArray          getContainerID() const = 0;
    virtual QByteArray          getSearchCriteria() const = 0;
    virtual QByteArray          getFilter() const = 0;
    virtual quint32             getStartingIndex() const = 0;
    virtual quint32             getRequestedCount() const = 0;
    virtual QByteArray          getSortCriteria() const = 0;

    virtual void                addItem(const BrowseItem &) = 0;
    virtual void                setResponse(quint32 totalMatches, quint32 updateID) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetSearchCapabilities
  {
    virtual void                setResponse(const QByteArray &) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetSortCapabilities
  {
    virtual void                setResponse(const QByteArray &) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionGetSystemUpdateID
  {
    virtual void                setResponse(quint32) = 0;
  };

  // Samsung GetFeatureList
  struct LXIMEDIACENTER_PUBLIC ActionGetFeatureList
  {
    virtual void                setResponse(const QList<QByteArray> &) = 0;
  };

public:
  explicit                      ContentDirectory(RootDevice *rootDevice, ConnectionManager *connectionManager);
  virtual                       ~ContentDirectory();

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

  void                          updateSystem();
  void                          updatePath(const QString &path);

  void                          handleAction(const UPnP::HttpRequestInfo &, ActionBrowse &);
  void                          handleAction(const UPnP::HttpRequestInfo &, ActionSearch &);
  void                          handleAction(const UPnP::HttpRequestInfo &, ActionGetSearchCapabilities &);
  void                          handleAction(const UPnP::HttpRequestInfo &, ActionGetSortCapabilities &);
  void                          handleAction(const UPnP::HttpRequestInfo &, ActionGetSystemUpdateID &);
  void                          handleAction(const UPnP::HttpRequestInfo &, ActionGetFeatureList &);

protected: // From RootDevice::Service
  virtual const char          * serviceType(void);

  virtual void                  initialize(void);
  virtual void                  close(void);

  virtual void                  writeServiceDescription(RootDevice::ServiceDescription &) const;
  virtual void                  writeEventableStateVariables(RootDevice::EventablePropertySet &) const;

protected: // From RootDevice::Service
  virtual void                  customEvent(QEvent *e);

private: // From HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response);

private:
  void                          addDirectory(ActionBrowse &, Item::Type, const QString &client, const QString &path, const QString &title = QString::null);
  void                          addContainer(ActionBrowse &, Item::Type, const QString &path, const QString &title = QString::null, int childCount = -1);
  void                          addFile(ActionBrowse &, const QString &host, const Item &, const QString &path, const QString &title = QString::null);

  static QStringList            allItems(const Item &, const QStringList &itemProps);
  static QStringList            streamItems(const Item &);
  static QStringList            playSeekItems(const Item &);
  static QStringList            seekItems(const Item &);
  static QStringList            chapterItems(const Item &);
  static QStringList            splitItemProps(const QString &);
  static Item                   makePlayItem(const Item &, const QStringList &);

  static QString                baseDir(const QString &);
  static QString                parentDir(const QString &);
  QByteArray                    toObjectID(const QString &path, bool create = true);
  QString                       fromObjectID(const QByteArray &id);
  QByteArray                    toObjectURL(const QUrl &path, const QByteArray &suffix);
  QUrl                          fromObjectURL(const QUrl &url);

private slots:
  void                          processPendingUpdates(void);

protected:
  static const char             serviceId[];
  static const unsigned         seekSec;

private:
  RootDevice            * const rootDevice;
  ConnectionManager     * const connectionManager;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
