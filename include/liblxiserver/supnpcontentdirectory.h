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
      Type_Playlist,

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
                                Stream(quint32 id = 0, const QString &lang = QString::null);
                                ~Stream();

      quint32                   id;
      QString                   lang;
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

    bool                        isDir;
    bool                        played;
    bool                        direct;
    quint8                      type;
    QUrl                        url;
    QUrl                        iconUrl;

    QString                     title;
    QString                     artist;
    QString                     album;
    int                         track;

    QList<Stream>               audioStreams;
    QList<Stream>               videoStreams;
    QList<Stream>               subtitleStreams;

    unsigned                    duration; //!< In seconds
    QList<Chapter>              chapters;
  };

  struct Callback
  {
    virtual int                 countContentDirItems(const QString &path) = 0;
    virtual QList<Item>         listContentDirItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;
  };

public:
  explicit                      SUPnPContentDirectory(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPContentDirectory();

  void                          initialize(SHttpServer *, SUPnPMediaServer *);
  void                          close(void);

  void                          setProtocols(ProtocolType, const ProtocolList &);
  void                          setQueryItems(const QString &peer, const QMap<QString, QString> &);
  QMap<QString, QString>        activeClients(void) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

  static QByteArray             toQueryID(const QByteArray &query);
  static QByteArray             fromQueryID(const QByteArray &id);

public slots:
  void                          modified(void);

protected: // From SUPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);

private:
  _lxi_internal void            handleBrowse(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestMessage &, const QHostAddress &);
  _lxi_internal QDomElement     didlDirectory(QDomDocument &, Item::Type, const QString &path, const QString &title = QString::null);
  _lxi_internal QDomElement     didlFile(QDomDocument &doc, const QString &peer, const QString &host, const Item &, const QString &path, const QString &title = QString::null);
  _lxi_internal void            emitEvent(bool dirty);

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

public:
  static const char             contentDirectoryNS[];

private:
  _lxi_internal static const unsigned seekSec;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
