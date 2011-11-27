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

#ifndef LXISERVER_SUPNPCONTENTDIRECTORY_H
#define LXISERVER_SUPNPCONTENTDIRECTORY_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiCore>
#include "shttpserver.h"
#include "supnpbase.h"
#include "export.h"

namespace LXiServer {

class LXISERVER_PUBLIC SUPnPContentDirectory : public SUPnPBase
{
Q_OBJECT
public:
  struct LXISERVER_PUBLIC Item
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

    struct LXISERVER_PUBLIC Stream
    {
                                Stream(void);
                                ~Stream();

      QString                   title;
      QList< QPair<QString, QString> > queryItems;
    };

    struct LXISERVER_PUBLIC Chapter
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
    bool                        played;
    quint8                      type;
    QUrl                        url;
    QUrl                        iconUrl;
    QString                     path;

    QString                     title;
    QString                     artist;
    QString                     album;
    int                         track;

    unsigned                    duration; //!< In seconds

    QList<Stream>               streams;
    QList<Chapter>              chapters;
    ProtocolList                protocols;
  };

  struct Callback
  {
    virtual int                 countContentDirItems(const QString &client, const QString &path) = 0;
    virtual QList<Item>         listContentDirItems(const QString &client, const QString &path, unsigned start = 0, unsigned count = 0) = 0;
    virtual Item                getContentDirItem(const QString &client, const QString &path) = 0;
  };

public:
  explicit                      SUPnPContentDirectory(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPContentDirectory();

  void                          initialize(SHttpServer *, SUPnPMediaServer *);
  void                          close(void);
  void                          reset(void);

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

public slots:
  void                          modified(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

protected: // From SUPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual SHttpServer::Status   handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);

private:
  _lxi_internal bool            handleBrowse(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);
  _lxi_internal void            didlDirectory(QDomDocument &, QDomElement &, Item::Type, const QString &client, const QString &path, const QString &title = QString::null);
  _lxi_internal void            didlContainer(QDomDocument &, QDomElement &, Item::Type, const QString &path, const QString &title = QString::null, int childCount = 0);
  _lxi_internal void            didlFile(QDomDocument &, QDomElement &, const QString &host, const Item &, const QString &path, const QString &title = QString::null);
  _lxi_internal void            emitEvent(bool dirty);

  _lxi_internal static QStringList allItems(const Item &, const QStringList &itemProps);
  _lxi_internal static QStringList streamItems(const Item &);
  _lxi_internal static QStringList playSeekItems(const Item &);
  _lxi_internal static QStringList seekItems(const Item &);
  _lxi_internal static QStringList chapterItems(const Item &);
  _lxi_internal static QStringList splitItemProps(const QString &);
  _lxi_internal static Item     makePlayItem(const Item &, const QStringList &);

  _lxi_internal static QString  baseDir(const QString &);
  _lxi_internal static QString  parentDir(const QString &);
  _lxi_internal QByteArray      toObjectID(const QString &path);
  _lxi_internal QString         fromObjectID(const QByteArray &id);
  _lxi_internal QByteArray      toObjectURL(const QUrl &path, const QByteArray &suffix);
  _lxi_internal QByteArray      fromObjectURL(const QByteArray &url);

public:
  static const char             contentDirectoryNS[];

private:
  _lxi_internal static const unsigned seekSec;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
