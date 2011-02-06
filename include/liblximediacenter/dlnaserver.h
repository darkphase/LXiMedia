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
class DlnaServerDir;

typedef FileServerHandle<DlnaServerDir> DlnaServerDirHandle;
typedef FileServerHandle<const DlnaServerDir> DlnaServerConstDirHandle;

class DlnaServer : public QObject,
                   public FileServer
{
Q_OBJECT
friend class DlnaServerDir;
private:
  class HttpDir;
  class EventSession;

  struct File
  {
                                File(void);
    explicit                    File(DlnaServer *);

    qint32                      id;
    bool                        played;
    bool                        music;
    quint32                     sortOrder;
    QString                     mimeType;
    QString                     url;
    QString                     iconUrl;
  };

  struct DirCacheEntry
  {
    struct Item : File
    {
                                Item(const QString &title, const File &);
                                Item(const QString &title, DlnaServerConstDirHandle);

      QString                   title;
    };

    QMultiMap<QString, Item>    children;
    qint32                      updateId;
    QTime                       update;
  };

public:
  explicit                      DlnaServer(HttpServer *, QObject * = NULL);
  virtual                       ~DlnaServer();

  void                          initialize(SsdpServer *);
  void                          close(void);

  void                          update(qint32 dirId);
  DlnaServerDir               * getDir(qint32 dirId);
  const DlnaServerDir         * getDir(qint32 dirId) const;

protected:
  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);
  virtual void                  timerEvent(QTimerEvent *);

private:
  QMap<qint32, DirCacheEntry>::Iterator updateCacheEntry(int dirId);
  bool                          handleBrowse(const QDomElement &, const QHttpRequestHeader &, QAbstractSocket *);
  static QDomElement            didlDirectory(QDomDocument &, const DirCacheEntry::Item &, int);
  static QDomElement            didlFile(QDomDocument &doc, const QString &, const QString &, const QString &, const QString &, const QString &, const DirCacheEntry::Item &, int, bool);
  void                          sendEventMessage(const QUrl &, const QByteArray &);
  
public:
  static const int              cacheTimeout;

private:
  static const qint32           DIR_ID_MASK  = 0x40000000;
  static const qint32           FILE_ID_MASK = 0x20000000;

  struct Private;
  Private               * const p;
};


class DlnaServerDir : public FileServerDir
{
Q_OBJECT
friend class DlnaServer;
public:
  typedef DlnaServer::File      File;

public:
  explicit                      DlnaServerDir(DlnaServer *);
  virtual                       ~DlnaServerDir();

  virtual void                  addFile(const QString &name, const File &);
  virtual void                  removeFile(const QString &name);
  virtual void                  addDir(const QString &name, FileServerDir *);
  virtual void                  removeDir(const QString &name);
  virtual void                  clear(void);
  virtual int                   count(void) const;

  virtual QStringList           listFiles(void);
  virtual DlnaServer::File      findFile(const QString &name);

  inline DlnaServer           * server(void)                                    { return static_cast<DlnaServer *>(FileServerDir::server()); }
  inline const DlnaServer     * server(void) const                              { return static_cast<const DlnaServer *>(FileServerDir::server()); }
  inline QStringList            listFiles(void) const                           { return const_cast<DlnaServerDir *>(this)->listFiles(); }
  inline DlnaServer::File       findFile(const QString &name) const             { return const_cast<DlnaServerDir *>(this)->findFile(name); }

public:
  const qint32                  id;
  bool                          played;
  qint32                        sortOrder;

protected:
  volatile qint32               updateId;
  QMap<QString, File>           files;
};


} // End of namespace

#endif
