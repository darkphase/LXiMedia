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

#ifndef LXMEDIACENTER_DLNASERVER_H
#define LXMEDIACENTER_DLNASERVER_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include <LXiStream>
#include "httpserver.h"
#include "globalsettings.h"

namespace LXiMediaCenter {

class SsdpServer;

class DlnaServer : public QObject,
                   protected HttpServer::Callback
{
Q_OBJECT
public:
  struct Item
  {
    enum Mode
    {
      Mode_Default              = 0,
      Mode_PlaySeek,
      Mode_Seek,
      Mode_Chapters,
      Mode_Direct               = 255
    };

    inline Item(void)
      : isDir(false), played(false), music(false), mode(Mode_Default)
    {
    }

    inline bool                 isNull(void) const                              { return url.isEmpty(); }

    bool                        isDir;
    bool                        played;
    bool                        music;
    quint8                      mode;
    QString                     title;
    QString                     mimeType;
    QUrl                        url;
    QUrl                        iconUrl;
    SMediaInfo                  mediaInfo;
  };

  struct Callback
  {
    virtual int                 countDlnaItems(const QString &path) = 0;
    virtual QList<Item>         listDlnaItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;
  };

private:
  typedef quint64               ItemID;

  class EventSession;
  struct StreamSettings;
  struct ItemData;

public:
  explicit                      DlnaServer(QObject * = NULL);
  virtual                       ~DlnaServer();

  void                          initialize(HttpServer *, SsdpServer *);
  void                          close(void);

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

  void                          update(const QString &path);

protected:
  virtual void                  timerEvent(QTimerEvent *);

protected: // From HttpServer::Callback
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QAbstractSocket *);

private:
  HttpServer::SocketOp          handleBrowse(const QDomElement &, const HttpServer::RequestHeader &, QAbstractSocket *);
  void                          browseDir(QDomDocument &, QDomElement &, ItemID, ItemData &, Callback *, const StreamSettings &, const QString &, unsigned, unsigned);
  void                          browseFile(QDomDocument &, QDomElement &, ItemID, ItemData &, const StreamSettings &, const QString &, unsigned, unsigned);
  ItemID                        addChildItem(ItemData &, const Item &item, bool asDir);
  static QDomElement            didlDirectory(QDomDocument &, const Item &, ItemID, ItemID = 0);
  static QDomElement            didlFile(QDomDocument &doc, const StreamSettings &, const Item &, ItemID, ItemID = 0);
  void                          sendEventMessage(const QUrl &, const QByteArray &);
  
  static QString                toIDString(ItemID id)                           { return QString::number(id | Q_UINT64_C(0x8000000000000000), 16); }
  static ItemID                 fromIDString(const QString &str)                { return str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF); }

private:
  static const unsigned         seekSec;

  struct Private;
  Private               * const p;
};


} // End of namespace

#endif
