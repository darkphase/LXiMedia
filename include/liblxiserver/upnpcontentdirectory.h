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

#ifndef LXISERVER_UPNPCONTENTDIRECTORY_H
#define LXISERVER_UPNPCONTENTDIRECTORY_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include "httpserver.h"
#include "upnpbase.h"

namespace LXiServer {

class UPnPContentDirectory : public UPnPBase
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

    enum Type
    {
      Type_None                 = 0,
      Type_Audio,
      Type_Video,
      Type_Image,
      Type_FlagMusic            = 0x80
    };

    struct Stream
    {
      inline Stream(quint32 id = 0, const QString &lang = QString::null)
        : id(id), lang(lang)
      {
      }

      quint32                   id;
      QString                   lang;
    };

    struct Chapter
    {
      inline Chapter(const QString &title = QString::null, unsigned position = 0)
        : title(title), position(position)
      {
      }

      QString                   title;
      unsigned                  position;
    };

    inline Item(void)
      : isDir(false), played(false), mode(Mode_Default), type(Type_None),
        duration(0)
    {
    }

    inline bool                 isNull(void) const                              { return url.isEmpty(); }

    bool                        isDir;
    bool                        played;
    quint8                      mode;
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
    virtual int                 countDlnaItems(const QString &path) = 0;
    virtual QList<Item>         listDlnaItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;
  };

private:
  typedef quint64               ItemID;

  struct ItemData
  {
    inline ItemData(void) : itemID(Q_INT64_C(-1)), parentID(Q_INT64_C(-1)) { }

    QString                     path;
    Item                        item;
    ItemID                      itemID;
    ItemID                      parentID;
    QVector<ItemID>             children;
  };

public:
  explicit                      UPnPContentDirectory(const QString &basePath, QObject * = NULL);
  virtual                       ~UPnPContentDirectory();

  void                          initialize(HttpServer *, UPnPMediaServer *);
  void                          close(void);

  void                          setProtocols(Item::Type, const ProtocolList &);
  void                          setQueryItems(const QString &peer, const QMap<QString, QString> &);
  QMap<QString, QString>        activeClients(void) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

public slots:
  void                          modified(void);

protected: // From UPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &);

private:
  void                          handleBrowse(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &);
  void                          browseDir(QDomDocument &, QDomElement &, ItemData &, Callback *, const QString &peer, const QString &host, const QString &, unsigned, unsigned);
  void                          browseFile(QDomDocument &, QDomElement &, ItemData &, const QString &peer, const QString &host, const QString &, unsigned, unsigned);
  ItemData                      addChildItem(ItemData &, const Item &item, bool asDir);
  QDomElement                   didlDirectory(QDomDocument &, const ItemData &) const;
  QDomElement                   didlFile(QDomDocument &doc, const QString &peer, const QString &, const ItemData &) const;
  void                          emitEvent(bool dirty);

  static QString                toIDString(ItemID);
  static ItemID                 fromIDString(const QString &);

public:
  static const char     * const contentDirectoryNS;

private:
  static const unsigned         seekSec;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
