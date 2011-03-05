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
      : isDir(false), played(false), music(false), mode(Mode_Default), duration(0)
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

  struct ItemData;

public:
  explicit                      UPnPContentDirectory(QObject * = NULL);
  virtual                       ~UPnPContentDirectory();

  void                          setFormats(const QByteArray &type, const QList<QByteArray> &formats);
  void                          setQueryItems(const QString &peer, const QMap<QString, QString> &);
  QMap<QString, QString>        activeClients(void) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

protected: // From UPnPBase
  virtual void                  emitEvent(void);
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &);
  virtual void                  addEventProperties(QDomDocument &, QDomElement &);

private:
  void                          handleBrowse(const QDomElement &, QDomDocument &, QDomElement &, const HttpServer::RequestHeader &, const QHostAddress &);
  void                          browseDir(QDomDocument &, QDomElement &, ItemID, ItemData &, Callback *, const QString &peer, const QString &host, const QString &, unsigned, unsigned);
  void                          browseFile(QDomDocument &, QDomElement &, ItemID, ItemData &, const QString &peer, const QString &host, const QString &, unsigned, unsigned);
  ItemID                        addChildItem(ItemData &, const Item &item, bool asDir);
  QDomElement                   didlDirectory(QDomDocument &, const Item &, ItemID, ItemID = 0) const;
  QDomElement                   didlFile(QDomDocument &doc, const QString &peer, const QString &, const Item &, ItemID, ItemID = 0) const;
  
  static QString                toIDString(ItemID id)                           { return QString::number(id | Q_UINT64_C(0x8000000000000000), 16); }
  static ItemID                 fromIDString(const QString &str)                { return str.toULongLong(NULL, 16) & Q_UINT64_C(0x7FFFFFFFFFFFFFFF); }

public:
  static const char     * const contentDirectoryNS;

private:
  static const unsigned         seekSec;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
